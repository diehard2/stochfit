#include "UnifiedReflectivity.h"
#include <cassert>

namespace {
void InitScratchArrays(WaveScratch<std::complex<double>> &complex_scratch,
                       WaveScratch<double> &real_scratch, int num_threads,
                       int n_layers) {
  complex_scratch.resize(n_layers, num_threads);
  real_scratch.resize(n_layers, num_threads);
}

// Returns the first Q index where no EDP layer is evanescent. Exploits the fact
// that sinsquaredthetai is monotone increasing (Q sorted ascending), so a
// single max-density precompute + binary search replaces the original O(Q·EDP)
// scan. Only valid for the measurement Q grid (monotone); never call for
// qspread arrays.
auto FindComplexToRealOffset(
    std::span<const double> sinsquaredthetai,
    std::span<const std::complex<double>> density_profile,
    double indexsupsquared_real, double sup_sld) -> int {
  const double max_density = std::ranges::max(
      density_profile |
      std::views::transform([](const auto &dp) { return dp.real(); }));

  const auto it = std::ranges::lower_bound(
      sinsquaredthetai, max_density, [&](double sin2, double target) {
        return indexsupsquared_real * sin2 + sup_sld < target;
      });
  return static_cast<int>(it - sinsquaredthetai.begin());
}
} // namespace

ReflConstants::ReflConstants(const ReflSettings &s) {
  k0 = 2.0 * std::numbers::pi / s.Wavelength;
  sup_sld = s.SupSLD;
  indexsup = 1.0 - s.SupSLD / 2.0;
  indexsupsquared = indexsup * indexsup;

  const double qspread_frac = s.QErr / 100.0;
  qsmear_enabled = qspread_frac >= 0.005 && !s.QError.empty();

  const int meas_n = static_cast<int>(s.Q.size());
  const int grid_n = qsmear_enabled ? meas_n * QSmear::Points : meas_n;
  sinthetai.resize(grid_n);
  sinsquaredthetai.resize(grid_n);

  if (qsmear_enabled) {
    std::vector<double> meas_sin(meas_n);
    for (int i = 0; i < meas_n; ++i)
      meas_sin[i] = s.Q[i] * s.Wavelength / (4.0 * std::numbers::pi);
    QSmear::BuildArrays(s.Wavelength, qspread_frac, meas_sin, s.QError,
                        sinthetai, sinsquaredthetai);
  } else {
    for (int i = 0; i < meas_n; ++i) {
      sinthetai[i] = s.Q[i] * s.Wavelength / (4.0 * std::numbers::pi);
      sinsquaredthetai[i] = sinthetai[i] * sinthetai[i];
    }
  }
}

ParrattReflectivity::ParrattReflectivity(const ReflSettings &settings)
    : m_consts(settings),
      m_qsmear_enabled(m_consts.qsmear_enabled) {

  const int meas_n = static_cast<int>(settings.Q.size());
  m_refl_out.resize(m_consts.sinthetai.size());
  if (m_qsmear_enabled)
    m_refl_smeared.resize(meas_n);
}

ParrattReflectivity::ParrattReflectivity(const ReflSettings &settings,
                                         int n_layers)
    : ParrattReflectivity(settings) {
  InitScratchArrays(m_complex, m_real, omp_get_max_threads(), n_layers);
}

// Standalone: creates its own OMP team. Safe for levmar/InitEnergy/any serial context.
auto ParrattReflectivity::CalculateReflectivity(const LayerStack &ls)
    -> std::span<double> {
  const int n_layers = static_cast<int>(ls.rho.size());
  InitScratchArrays(m_complex, m_real, omp_get_max_threads(), n_layers);

#pragma omp parallel
  {
    if (m_qsmear_enabled) {
      if (ls.has_roughness)
        ReflectivityCalcCoreImpl<true>(ls, m_consts.sinthetai,
                                       m_consts.sinsquaredthetai, m_refl_out);
      else
        ReflectivityCalcCoreImpl<false>(ls, m_consts.sinthetai,
                                        m_consts.sinsquaredthetai, m_refl_out);
    } else if (ls.transparent && !ls.has_roughness) {
      TransparentReflectivityCalc(ls);
    } else {
      ReflectivityCalc(ls);
    }
  }

  if (m_qsmear_enabled) {
    QSmear::Apply(m_refl_out, m_refl_smeared);
    return m_refl_smeared;
  }
  return m_refl_out;
}

// Cooperative: must be called by ALL threads of an enclosing OMP parallel region.
// BuildLayerStack and scratch sizing happen in omp single; Q-point work is shared.
auto ParrattReflectivity::CalculateReflectivityCooperative(const CEDP &EDP)
    -> std::span<double> {
#pragma omp single
  {
    // BuildLayerStackFull computes GetOffSets inline — merges 2 barriers into 1.
    m_cooperative_ls = EDP.BuildLayerStackFull();
  }
  // implicit barrier: all threads see m_cooperative_ls

  if (m_qsmear_enabled) {
    if (m_cooperative_ls.has_roughness)
      ReflectivityCalcCoreImpl<true>(m_cooperative_ls, m_consts.sinthetai,
                                     m_consts.sinsquaredthetai, m_refl_out);
    else
      ReflectivityCalcCoreImpl<false>(m_cooperative_ls, m_consts.sinthetai,
                                      m_consts.sinsquaredthetai, m_refl_out);
#pragma omp single
    { QSmear::Apply(m_refl_out, m_refl_smeared); }
    return m_refl_smeared;
  }

  if (m_cooperative_ls.transparent && !m_cooperative_ls.has_roughness)
    TransparentReflectivityCalc(m_cooperative_ls);
  else
    ReflectivityCalc(m_cooperative_ls);

  return m_refl_out;
}

auto ParrattReflectivity::CalculateReflectivity(const CEDP &EDP)
    -> std::span<double> {
  return CalculateReflectivity(EDP.BuildLayerStack());
}

// Complex Parratt loop over Q indices [0, q_end).
//
// LayerStack::sup_offset = first index where density != front,
// sub_offset = last index where density != back. The varying region is the
// inclusive interval [sup_offset, sub_offset]. The flat regions are
// [0, sup_offset-1] and [sub_offset+1, n_layers-1].
//
// Optimisations:
//  - Flat substrate [sub_offset+1, n-1]: Rj=0 (seed=0, rj=0).
//    Start back-recursion at sub_offset.
//  - Flat superstrate [0, sup_offset-2]: rj=0, ak=ak1 uniformly.
//    Collapse to Rj[0] = ak1^(sup_offset-2) * Rj[sup_offset-1] via pow.
//  - HasRoughness=true: Nevot-Croce factor exp(sigma_sq[i]*kk[i]*kk[i+1])
//    is multiplied into each Fresnel rj[i]. When false, the compiler omits it.
template <bool HasRoughness>
void ParrattReflectivity::ReflectivityCalcCoreImpl(
    const LayerStack &ls, std::span<const double> sinthetai,
    std::span<const double> sin2thetai, std::span<double> out, int q_end) {
  const auto &density_profile = ls.rho;
  const int n_layers = static_cast<int>(density_profile.size());
  const int n_q = std::min(q_end, static_cast<int>(sinthetai.size()));

  const int sup_offset = ls.sup_offset;
  const int sub_offset = ls.sub_offset;

  // Per-thread scratch setup — each thread in the enclosing parallel team computes
  // its own slice. No inner omp parallel needed; caller provides the team.
  const int threadnum = omp_get_thread_num();
  const int arrayoffset = threadnum * n_layers;

  auto *kk_slice = m_complex.kk.data() + arrayoffset;
  auto *ak_slice = m_complex.ak.data() + arrayoffset;
  auto *rj_slice = m_complex.rj.data() + arrayoffset;
  auto *Rj_slice = m_complex.Rj.data() + arrayoffset;

  ak_slice[0] = 1.0;
  Rj_slice[sub_offset + 1] = 0.0;

#pragma omp for schedule(runtime)
  for (int l = 0; l < n_q; l++) {
      kk_slice[0] = m_consts.k0 * m_consts.indexsup * sinthetai[l];

      // Flat-front wavevector — density uniform at [1, sup_offset-1]
      const auto kk1 =
          m_consts.k0 * std::sqrt(m_consts.indexsupsquared * sin2thetai[l] -
                                  density_profile[1] + m_consts.sup_sld);
      const auto ak1 = std::exp(ls.length_mult[1] * kk1);

      // Flat-back wavevector
      const auto kk_sub =
          m_consts.k0 *
          std::sqrt(m_consts.indexsupsquared * sin2thetai[l] -
                    density_profile[n_layers - 1] + m_consts.sup_sld);

      // Varying region: kk and ak for [sup_offset, sub_offset]
      for (int i = sup_offset; i <= sub_offset; i++) {
        kk_slice[i] =
            m_consts.k0 * std::sqrt(m_consts.indexsupsquared * sin2thetai[l] -
                                    density_profile[i] + m_consts.sup_sld);
        ak_slice[i] = std::exp(ls.length_mult[i] * kk_slice[i]);
      }

      // Boundary entries for rj at the flat/varying interfaces.
      const int rj_start = sup_offset >= 2 ? sup_offset - 1 : 0;
      if (sup_offset >= 2) {
        kk_slice[sup_offset - 1] = kk1;
        ak_slice[sup_offset - 1] = ak1;
      }
      kk_slice[sub_offset + 1] = kk_sub;

      for (int i = rj_start; i <= sub_offset; i++) {
        rj_slice[i] =
            (kk_slice[i] - kk_slice[i + 1]) / (kk_slice[i] + kk_slice[i + 1]);
        if constexpr (HasRoughness)
          rj_slice[i] *= std::exp(ls.sigma_sq[i] * kk_slice[i] * kk_slice[i + 1]);
      }

      for (int i = sub_offset; i >= rj_start; i--)
        Rj_slice[i] = ak_slice[i] * (Rj_slice[i + 1] + rj_slice[i]) /
                      (Rj_slice[i + 1] * rj_slice[i] + 1.0);

      if (sup_offset >= 2) {
        // Replace std::pow(ak1, n) with an explicit multiply loop — pow calls
        // exp(n*log(z)) which is ~30x slower than n multiplications for small n.
        // sup_offset is typically 1-3 so this loop runs 0-1 times in practice.
        auto ak_pow = std::complex<double>{1.0, 0.0};
        for (int p = 0; p < sup_offset - 2; ++p) ak_pow *= ak1;
        out[l] = std::norm(ak_pow * Rj_slice[sup_offset - 1]);
      } else {
        out[l] = std::norm(Rj_slice[0]);
      }
  }
}

void ParrattReflectivity::ReflectivityCalc(const LayerStack &ls, int q_end) {
  assert(!m_qsmear_enabled);
  if (ls.has_roughness)
    ReflectivityCalcCoreImpl<true>(ls, m_consts.sinthetai,
                                    m_consts.sinsquaredthetai, m_refl_out,
                                    q_end);
  else
    ReflectivityCalcCoreImpl<false>(ls, m_consts.sinthetai,
                                     m_consts.sinsquaredthetai, m_refl_out,
                                     q_end);
}

void ParrattReflectivity::TransparentReflectivityCalc(const LayerStack &ls) {
  assert(!m_qsmear_enabled);
  const auto &density_profile = ls.rho;
  const int n_layers = static_cast<int>(density_profile.size());
  const int n_q = static_cast<int>(m_refl_out.size());

  const int sup_offset = ls.sup_offset;
  const int sub_offset = ls.sub_offset;
  const int rj_start = sup_offset >= 2 ? sup_offset - 1 : 0;

  const int complex_to_real_offset =
      FindComplexToRealOffset(m_consts.sinsquaredthetai, density_profile,
                              m_consts.indexsupsquared, m_consts.sup_sld);

  // ls.length_mult[i].imag() = -2*length[i]; uniform for CEDP path.
  const double angle_mult_flat = ls.length_mult[1].imag();

  // Both loops share the caller's parallel team so there is only one fork+join
  // per SA run instead of two per SA iteration. nowait on the complex loop lets
  // threads proceed to the real loop without an extra barrier between them.
  const int threadnum = omp_get_thread_num();
  const int arrayoffset = threadnum * n_layers;

  auto *kk_slice  = m_complex.kk.data() + arrayoffset;
  auto *ak_slice  = m_complex.ak.data() + arrayoffset;
  auto *rj_slice  = m_complex.rj.data() + arrayoffset;
  auto *Rj_slice  = m_complex.Rj.data() + arrayoffset;
  auto *dkk_slice = m_real.kk.data() + arrayoffset;
  auto *drj_slice = m_real.rj.data() + arrayoffset;

  ak_slice[0] = 1.0;
  Rj_slice[sub_offset + 1] = 0.0;

    // Complex Q-points: evanescent layers present; use full complex arithmetic.
#pragma omp for nowait schedule(runtime)
    for (int l = 0; l < complex_to_real_offset; l++) {
      kk_slice[0] = m_consts.k0 * m_consts.indexsup * m_consts.sinthetai[l];

      const auto kk1 =
          m_consts.k0 * std::sqrt(m_consts.indexsupsquared * m_consts.sinsquaredthetai[l] -
                                  density_profile[1] + m_consts.sup_sld);
      const auto ak1 = std::exp(ls.length_mult[1] * kk1);
      const auto kk_sub =
          m_consts.k0 * std::sqrt(m_consts.indexsupsquared * m_consts.sinsquaredthetai[l] -
                                  density_profile[n_layers - 1] + m_consts.sup_sld);

      for (int i = sup_offset; i <= sub_offset; i++) {
        kk_slice[i] =
            m_consts.k0 * std::sqrt(m_consts.indexsupsquared * m_consts.sinsquaredthetai[l] -
                                    density_profile[i] + m_consts.sup_sld);
        ak_slice[i] = std::exp(ls.length_mult[i] * kk_slice[i]);
      }

      if (sup_offset >= 2) {
        kk_slice[sup_offset - 1] = kk1;
        ak_slice[sup_offset - 1] = ak1;
      }
      kk_slice[sub_offset + 1] = kk_sub;

      for (int i = rj_start; i <= sub_offset; i++)
        rj_slice[i] = (kk_slice[i] - kk_slice[i + 1]) / (kk_slice[i] + kk_slice[i + 1]);

      for (int i = sub_offset; i >= rj_start; i--)
        Rj_slice[i] = ak_slice[i] * (Rj_slice[i + 1] + rj_slice[i]) /
                      (Rj_slice[i + 1] * rj_slice[i] + 1.0);

      // kk1 is always real for the flat superstrate region, so |ak1| == 1 and
      // the pow(ak1, n) phase factor drops out of the norm.
      m_refl_out[l] = (sup_offset >= 2) ? std::norm(Rj_slice[sup_offset - 1])
                                        : std::norm(Rj_slice[0]);
    }

    // Real Q-points: all wavevectors real; exp({0,-2dz}*real_kk) is a pure
    // unit-magnitude complex number — use polar() to avoid the wasted exp()
    // call that the general complex overload would perform.
#pragma omp for schedule(runtime)
    for (int l = complex_to_real_offset; l < n_q; l++) {
      dkk_slice[0] = m_consts.k0 * m_consts.indexsup * m_consts.sinthetai[l];

      const auto dkk1 =
          m_consts.k0 *
          sqrt((m_consts.indexsupsquared * m_consts.sinsquaredthetai[l]) -
               density_profile[1].real() + m_consts.sup_sld);
      const double angle0 = angle_mult_flat * dkk1;
      const auto dak1 = std::complex<double>(std::cos(angle0), std::sin(angle0));

      const auto dkk_sub =
          m_consts.k0 *
          sqrt((m_consts.indexsupsquared * m_consts.sinsquaredthetai[l]) -
               density_profile.back().real() + m_consts.sup_sld);

      for (int i = sup_offset; i <= sub_offset; i++) {
        dkk_slice[i] =
            m_consts.k0 *
            sqrt((m_consts.indexsupsquared * m_consts.sinsquaredthetai[l]) -
                 density_profile[i].real() + m_consts.sup_sld);
        const double angle_i = ls.length_mult[i].imag() * dkk_slice[i];
        ak_slice[i] = {std::cos(angle_i), std::sin(angle_i)};
      }

      if (sup_offset >= 2) {
        dkk_slice[sup_offset - 1] = dkk1;
        ak_slice[sup_offset - 1] = dak1;
      }
      dkk_slice[sub_offset + 1] = dkk_sub;

      for (int i = rj_start; i <= sub_offset; i++)
        drj_slice[i] = (dkk_slice[i] - dkk_slice[i + 1]) /
                       (dkk_slice[i] + dkk_slice[i + 1]);

      for (int i = sub_offset; i >= rj_start; i--)
        Rj_slice[i] = ak_slice[i] * (Rj_slice[i + 1] + drj_slice[i]) /
                      (Rj_slice[i + 1] * drj_slice[i] + 1.0);

      // |dak1| == 1 (pure phase), so the flat-front propagation factor drops
      // out of the norm.
      m_refl_out[l] = (sup_offset >= 2) ? std::norm(Rj_slice[sup_offset - 1])
                                        : std::norm(Rj_slice[0]);
  }
}
