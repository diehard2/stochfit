#include "UnifiedReflectivity.h"

#include <cassert>

namespace {
void InitScratchArrays(WaveScratch<std::complex<double>> &complex_scratch,
                       WaveScratch<double> &real_scratch,
                       int num_threads, int edp_points) {
  complex_scratch.resize(edp_points, num_threads);
  real_scratch.resize(edp_points, num_threads);
}

// Returns the first Q index where no EDP layer is evanescent. Exploits the fact
// that sinsquaredthetai is monotone increasing (Q sorted ascending), so a single
// max-density precompute + binary search replaces the original O(Q·EDP) scan.
// Only valid for the measurement Q grid (monotone); never call for qspread arrays.
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
    : m_settings(settings), m_consts(settings),
      m_qsmear_enabled(m_consts.qsmear_enabled) {

  const int meas_n = static_cast<int>(settings.Q.size());
  m_refl_out.resize(m_consts.sinthetai.size());
  if (m_qsmear_enabled)
    m_refl_smeared.resize(meas_n);

  if constexpr (!kSingleProcDebug)
    m_num_threads = std::min(MAX_OMP_THREADS, omp_get_num_procs());
}

auto ParrattReflectivity::CalculateReflectivity(const CEDP &EDP)
    -> std::span<double> {
  InitScratchArrays(m_complex, m_real, m_num_threads, EDP.Get_EDPPointCount());

  if (m_qsmear_enabled) {
    ReflectivityCalcCore(EDP, m_consts.sinthetai, m_consts.sinsquaredthetai,
                         m_refl_out);
    QSmear::Apply(m_refl_out, m_refl_smeared);
    return m_refl_smeared;
  }

  if (!EDP.Get_UseABS())
    TransparentReflectivityCalc(EDP);
  else
    ReflectivityCalc(EDP);

  return m_refl_out;
}

// Complex Parratt loop over Q indices [0, q_end).
//
// CEDP::GetOffSets() returns sup_offset = first index where density != front,
// sub_offset = first index (from the back) where density != back. The varying
// region is the inclusive interval [sup_offset, sub_offset]. The flat regions
// are [0, sup_offset-1] and [sub_offset+1, edp_points-1].
//
// Optimisations vs original CReflCalc:
//  - Flat substrate [sub_offset+1, edp_points-1]: Rj=0 (seed=0, rj=0).
//    Start back-recursion at sub_offset.
//  - Flat superstrate [0, sup_offset-2]: rj=0, ak=ak1 uniformly.
//    Collapse to Rj[0] = ak1^(sup_offset-2) * Rj[sup_offset-1] via pow.
void ParrattReflectivity::ReflectivityCalcCore(
    const CEDP &EDP,
    std::span<const double> sinthetai,
    std::span<const double> sin2thetai,
    std::span<double> out,
    int q_end) {
  const auto &density_profile = EDP.m_DEDP;
  const int edp_points = EDP.Get_EDPPointCount();
  const int n_q = std::min(q_end, static_cast<int>(sinthetai.size()));

  const auto length_multiplier = std::complex<double>{0.0, -2.0 * EDP.Get_Dz()};
  const auto [sup_offset, sub_offset] = EDP.GetOffSets();

  omp_set_num_threads(m_num_threads);
#pragma omp parallel
  {
    const int threadnum = omp_get_thread_num();
    const int arrayoffset = threadnum * edp_points;

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
          m_consts.k0 *
          std::sqrt(m_consts.indexsupsquared * sin2thetai[l] -
                    density_profile[1] + m_consts.sup_sld);
      const auto ak1 = std::exp(length_multiplier * kk1);

      // Flat-back wavevector
      const auto kk_sub =
          m_consts.k0 *
          std::sqrt(m_consts.indexsupsquared * sin2thetai[l] -
                    density_profile[edp_points - 1] + m_consts.sup_sld);

      // Varying region: kk and ak for [sup_offset, sub_offset]
      for (int i = sup_offset; i <= sub_offset; i++) {
        kk_slice[i] =
            m_consts.k0 *
            std::sqrt(m_consts.indexsupsquared * sin2thetai[l] -
                      density_profile[i] + m_consts.sup_sld);
        ak_slice[i] = std::exp(length_multiplier * kk_slice[i]);
      }

      // Boundary entries for rj at the flat/varying interfaces.
      // Guard: sup_offset==1 means no flat region left of it; kk_slice[0] is
      // the vacuum wavevector and must not be overwritten.
      const int rj_start = sup_offset >= 2 ? sup_offset - 1 : 0;
      if (sup_offset >= 2) {
        kk_slice[sup_offset - 1] = kk1;
        ak_slice[sup_offset - 1] = ak1;
      }
      kk_slice[sub_offset + 1] = kk_sub;

      for (int i = rj_start; i <= sub_offset; i++)
        rj_slice[i] =
            (kk_slice[i] - kk_slice[i + 1]) / (kk_slice[i] + kk_slice[i + 1]);

      for (int i = sub_offset; i >= rj_start; i--)
        Rj_slice[i] = ak_slice[i] * (Rj_slice[i + 1] + rj_slice[i]) /
                      (Rj_slice[i + 1] * rj_slice[i] + 1.0);

      out[l] = (sup_offset >= 2)
                   ? std::norm(std::pow(ak1, sup_offset - 2) *
                               Rj_slice[sup_offset - 1])
                   : std::norm(Rj_slice[0]);
    }
  }
}

void ParrattReflectivity::ReflectivityCalc(const CEDP &EDP, int q_end) {
  assert(!m_qsmear_enabled);
  ReflectivityCalcCore(EDP, m_consts.sinthetai, m_consts.sinsquaredthetai,
                       m_refl_out, q_end);
}

void ParrattReflectivity::TransparentReflectivityCalc(const CEDP &EDP) {
  assert(!m_qsmear_enabled);
  const auto &density_profile = EDP.m_DEDP;
  const int edp_points = EDP.Get_EDPPointCount();

  const auto [sup_offset, sub_offset] = EDP.GetOffSets();
  const int complex_to_real_offset = FindComplexToRealOffset(
      m_consts.sinsquaredthetai, density_profile, m_consts.indexsupsquared,
      m_consts.sup_sld);

  ReflectivityCalc(EDP, complex_to_real_offset);

  omp_set_num_threads(m_num_threads);
  // For the transparent path all wavevectors are real, so exp({0,-2dz}*real_kk)
  // is a pure unit-magnitude complex number — use polar() to avoid the wasted
  // exp() call that the general complex overload would perform.
  const double angle_mult = -2.0 * EDP.Get_Dz();

#pragma omp parallel
  {
    const int threadnum = omp_get_thread_num();
    const int arrayoffset = threadnum * edp_points;

    auto *dkk_slice = m_real.kk.data() + arrayoffset;
    auto *ak_slice = m_complex.ak.data() + arrayoffset;
    auto *drj_slice = m_real.rj.data() + arrayoffset;
    auto *Rj_slice = m_complex.Rj.data() + arrayoffset;

    ak_slice[0] = 1.0;
    Rj_slice[sub_offset + 1] = 0.0;

#pragma omp for schedule(runtime)
    for (int l = complex_to_real_offset;
         l < static_cast<int>(m_refl_out.size()); l++) {
      dkk_slice[0] = m_consts.k0 * m_consts.indexsup * m_consts.sinthetai[l];

      // Flat-front wavevector
      const auto dkk1 =
          m_consts.k0 *
          sqrt((m_consts.indexsupsquared * m_consts.sinsquaredthetai[l]) -
               density_profile[1].real() + m_consts.sup_sld);
      const auto dak1 = std::polar(1.0, angle_mult * dkk1);

      // Flat-back wavevector
      const auto dkk_sub =
          m_consts.k0 *
          sqrt((m_consts.indexsupsquared * m_consts.sinsquaredthetai[l]) -
               density_profile.back().real() + m_consts.sup_sld);

      // Varying region: kk and ak for [sup_offset, sub_offset]
      for (int i = sup_offset; i <= sub_offset; i++) {
        dkk_slice[i] =
            m_consts.k0 *
            sqrt((m_consts.indexsupsquared * m_consts.sinsquaredthetai[l]) -
                 density_profile[i].real() + m_consts.sup_sld);
        ak_slice[i] = std::polar(1.0, angle_mult * dkk_slice[i]);
      }

      const int rj_start = sup_offset >= 2 ? sup_offset - 1 : 0;
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

      m_refl_out[l] =
          (sup_offset >= 2)
              ? std::norm(std::polar(1.0, static_cast<double>(sup_offset - 2) *
                                             angle_mult * dkk1) *
                          Rj_slice[sup_offset - 1])
              : std::norm(Rj_slice[0]);
    }
  }
}
