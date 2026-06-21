#include "CEDP.h"
#include "platform.h"

std::pair<int, int> CEDP::GetOffSets() const {
  const double front = m_DEDP.front().real();
  const double back = m_DEDP.back().real();

  const int sup_offset = static_cast<int>(
      std::ranges::find_if(m_DEDP, [&](const auto &v) {
        return v.real() != front;
      }) - m_DEDP.begin());

  const int sub_offset = static_cast<int>(m_DEDP.size()) - 1 -
      static_cast<int>(
          std::ranges::find_if(m_DEDP | std::views::reverse, [&](const auto &v) {
            return v.real() != back;
          }) - m_DEDP.rbegin());

  return {sup_offset, sub_offset};
}

void CEDP::Init(const ReflSettings &InitStruct) {
  int resolution = InitStruct.Resolution > 0 ? InitStruct.Resolution : 3;
  m_dDz0 = 1.0 / resolution;
  m_dLambda = InitStruct.Wavelength;
  m_bUseSurfAbs = InitStruct.UseSurfAbs;
  m_dWaveConstant = m_dLambda * m_dLambda / (2.0 * std::numbers::pi);
  m_dRho = InitStruct.FilmSLD * 1e-6 * m_dWaveConstant;
  // Padding sized to 6× the roughness upper bound: the erf tail falls below
  // 1e-9 at 6σ. Scales automatically with RoughnessMax so high-roughness
  // films (e.g. gold at 15 Å roughness → 90 Å padding) work without changes.
  // FilmSlack adds 7 Å past the last box so the substrate erf tail converges.
  const double leftOffset = 6.0 * InitStruct.RoughnessMax;
  const double substrateOffset = 6.0 * InitStruct.RoughnessMax;
  const int FilmSlack = 7;

  m_iLayers = static_cast<int>(leftOffset + InitStruct.FilmLength + FilmSlack +
                               substrateOffset);
  m_iLayers *= resolution;

  if (InitStruct.UseSurfAbs != 0) {
    m_dBeta = InitStruct.FilmAbs * m_dWaveConstant;
    m_dBeta_Sub = InitStruct.SubAbs * m_dWaveConstant;
    m_dBeta_Sup = InitStruct.SupAbs * m_dWaveConstant;
  } else {
    m_dBeta = m_dBeta_Sub = m_dBeta_Sup = 0;
  }

  m_EDP.resize(m_iLayers);
  m_DEDP.resize(m_iLayers);
  m_fEDSpacingArray.resize(m_iLayers);

  m_fDistArray.resize(InitStruct.Boxes + 2);
  m_fRhoArray.resize(InitStruct.Boxes + 2);
  m_fImagRhoArray.resize(InitStruct.Boxes + 2);

  // Precompute per-layer length multiplier for BuildLayerStack.
  m_length_mult.assign(m_iLayers, std::complex<double>{0.0, -2.0 * m_dDz0});

  for (int i = 0; i < m_iLayers; i++) {
    m_fEDSpacingArray[i] = i * m_dDz0 - leftOffset;
  }

  for (int k = 0; k < InitStruct.Boxes + 2; k++) {
    m_fDistArray[k] =
        k * (InitStruct.FilmLength + (double)FilmSlack) / InitStruct.Boxes;
  }
}

void CEDP::GenerateEDP(ParamVector &g) {
  // Standalone path: wrap serial setup in its own single so standalone callers
  // (display, InitEnergy, LevMar) work correctly outside a persistent OMP team.
#pragma omp single
  { FillBoxArrays(g); }

  if (!m_bUseSurfAbs) BuildEDP<false>(g);
  else BuildEDP<true>(g);

#pragma omp single
  {
    auto [sup, sub] = GetOffSets();
    m_supOff = sup;
    m_subOff = sub;
  }
}

// Cooperative path: FillBoxArrays was already called in the caller's omp single
// (merged with Step). GetOffSets is called later in BuildLayerStackFull.
void CEDP::GenerateEDPCooperative(ParamVector &g) {
  if (!m_bUseSurfAbs) BuildEDP<false>(g);
  else BuildEDP<true>(g);
}

// The code for the ED calculation section is loosely based on the electron
// density calculation in Motofit (www.sourceforge.net/motofit). It is a
// standard method of calculating the electron density profile. We treat the
// profile as having a user defined number of boxes. The last 30% or so of the
// curve will converge to have rho/rhoinf = 1.0. For lipid and lipid protein
// films, the absorbance is negligible.
// Fills m_fRhoArray (and m_fImagRhoArray for absorbing films). No omp pragma —
// designed to be called from within the caller's omp single block.
void CEDP::FillBoxArrays(ParamVector &g) {
  if (!m_bUseSurfAbs) FillBoxArraysImpl<false>(g);
  else FillBoxArraysImpl<true>(g);
}

template <bool Absorbing>
void CEDP::FillBoxArraysImpl(ParamVector &g) {
  const int refllayers = g.RealParamsSize() - 1;

  if constexpr (!Absorbing)
    m_EDP[0].imag(0.0);

  for (int k = 0; k < refllayers; k++) {
    m_fRhoArray[k] =
        m_dRho * (g.GetRealParams(k + 1) - g.GetRealParams(k)) * 0.5;

    if constexpr (Absorbing) {
      if (k == 0) {
        m_fImagRhoArray[k] =
            (m_dBeta * g.GetSurfAbs() * g.GetRealParams(k + 1) /
                 g.GetRealParams(refllayers) -
             m_dBeta_Sup) /
            2.0;
      } else if (k == refllayers - 1) {
        m_fImagRhoArray[k] =
            (m_dBeta_Sub - m_dBeta * g.GetSurfAbs() * g.GetRealParams(k) /
                               g.GetRealParams(refllayers)) /
            2.0;
      } else {
        m_fImagRhoArray[k] =
            (m_dBeta * g.GetSurfAbs() * g.GetRealParams(k + 1) /
                 g.GetRealParams(refllayers) -
             (m_dBeta * g.GetSurfAbs() * g.GetRealParams(k) /
              g.GetRealParams(refllayers)) /
                 2.0);
      }
    }
  }
}

template <bool Absorbing>
void CEDP::BuildEDP(ParamVector &g) {
  const int reflpoints = m_iLayers;
  const int refllayers = g.RealParamsSize() - 1;
  const double supersld = g.GetRealParams(0) * m_dRho;

  double roughness = g.GetRoughness();
  if (roughness < 1e-6) roughness = 1e-6;
  roughness = 1.0 / (roughness * std::sqrt(2.0));

  // m_fRhoArray was filled by FillBoxArrays (in an omp single with implicit barrier)
  // before this function is called. No serial setup here.

  double dist;
#pragma omp for private(dist)
  for (int i = 0; i < reflpoints; i++) {
    if constexpr (Absorbing)
      m_EDP[i] = std::complex<double>(supersld, m_dBeta);
    else
      m_EDP[i].real(supersld);

    for (int k = 0; k < refllayers; k++) {
      dist = (m_fEDSpacingArray[i] - m_fDistArray[k]) * roughness;

      if (dist > 6.0) {
        if constexpr (Absorbing)
          m_EDP[i] += std::complex<double>(m_fRhoArray[k] * 2.0,
                                           m_fImagRhoArray[k] * 2.0);
        else
          m_EDP[i] += m_fRhoArray[k] * 2.0;
      } else if (dist > -6.0) {
        const double erf_val = 1.0 + std::erf(dist);
        if constexpr (Absorbing)
          m_EDP[i] += std::complex<double>(m_fRhoArray[k] * erf_val,
                                           m_fImagRhoArray[k] * erf_val);
        else
          m_EDP[i] += m_fRhoArray[k] * erf_val;
      }
    }

    m_DEDP[i] = 2.0 * m_EDP[i];
  }
}

LayerStack CEDP::BuildLayerStack() const {
  LayerStack ls;
  ls.rho        = m_DEDP;
  ls.length_mult = m_length_mult;
  ls.sup_offset   = m_supOff;
  ls.sub_offset   = m_subOff;
  ls.transparent  = !m_bUseSurfAbs;
  ls.has_roughness = false;
  return ls;
}

// Cooperative variant: computes sup/sub offsets fresh (skips the cached values
// that GenerateEDPCooperative doesn't update). m_supOff/m_subOff are mutable so
// this can be called on a const CEDP& and still keeps the cache in sync.
LayerStack CEDP::BuildLayerStackFull() const {
  auto [sup, sub] = GetOffSets();
  m_supOff = sup;
  m_subOff = sub;
  LayerStack ls;
  ls.rho        = m_DEDP;
  ls.length_mult = m_length_mult;
  ls.sup_offset   = sup;
  ls.sub_offset   = sub;
  ls.transparent  = !m_bUseSurfAbs;
  ls.has_roughness = false;
  return ls;
}

int CEDP::Get_EDPPointCount() const { return m_iLayers; }

bool CEDP::Get_UseABS() const { return m_bUseSurfAbs; }

double CEDP::Get_Dz() const { return m_dDz0; }

double CEDP::Get_LeftOffset() const {
  return m_fEDSpacingArray.empty() ? 0.0 : -m_fEDSpacingArray[0];
}

double CEDP::Get_FilmAbs() const { return m_dBeta; }

double CEDP::Get_FilmAbsInput() const {
  // Returns the value that, when passed to Set_FilmAbs(), reproduces m_dBeta.
  // Set_FilmAbs(x) stores x * m_dWaveConstant, so x = m_dBeta / m_dWaveConstant.
  return (m_dWaveConstant > 0.0) ? m_dBeta / m_dWaveConstant : 0.0;
}

double CEDP::Get_WaveConstant() const { return m_dWaveConstant; }

void CEDP::Set_FilmAbs(double abs) { m_dBeta = abs * m_dWaveConstant; }
