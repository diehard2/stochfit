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
  int resolution = InitStruct.Resolution <= 0 ? 3 : InitStruct.Resolution;
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

  // Arrays for the electron density profile and twice the electron density
  // profile
  m_EDP.resize(m_iLayers);
  m_DEDP.resize(m_iLayers);
  m_fEDSpacingArray.resize(m_iLayers);

  // Create scratch arrays for the electron density calculation
  m_fDistArray.resize(InitStruct.Boxes + 2);
  m_fRhoArray.resize(InitStruct.Boxes + 2);
  m_fImagRhoArray.resize(InitStruct.Boxes + 2);

  for (int i = 0; i < m_iLayers; i++) {
    m_fEDSpacingArray[i] = i * m_dDz0 - leftOffset;
  }

  for (int k = 0; k < InitStruct.Boxes + 2; k++) {
    m_fDistArray[k] =
        k * (InitStruct.FilmLength + (double)FilmSlack) / InitStruct.Boxes;
    ;
  }
}

void CEDP::GenerateEDP(ParamVector &g) {
  if (!m_bUseSurfAbs)
    MakeTranparentEDP(g);
  else
    MakeEDP(g);
}

// The code for the ED calculation section is loosely based on the electron
// density calculation in Motofit (www.sourceforge.net/motofit). It is a
// standard method of calculating the electron density profile. We treat the
// profile as having a user defined number of boxes The last 30% or so of the
// curve will converge to have rho/rhoinf = 1.0. For lipid and lipid protein
// films, the absorbance is negligible

void CEDP::MakeTranparentEDP(ParamVector &g) {
  double dist;
  int reflpoints = m_iLayers;
  int refllayers = g.RealparamsSize() - 1;
  double roughness = g.getroughness();
  double supersld = g.GetRealparams(0) * m_dRho;

  if (g.getroughness() < 1e-6)
    roughness = 1e-6;

  roughness = 1.0 / (roughness * sqrt(2.0));

  // Don't delete this, otherwise the reflectivity calculation won't work
  // sometimes
  m_EDP[0].imag(0.0);

  for (int k = 0; k < refllayers; k++) {
    m_fRhoArray[k] =
        m_dRho * (g.GetRealparams(k + 1) - g.GetRealparams(k)) * 0.5;
  }

#pragma omp parallel for private(dist)
  for (int i = 0; i < reflpoints; i++) {
    m_EDP[i].real(supersld);

    for (int k = 0; k < refllayers; k++) {
      dist = (m_fEDSpacingArray[i] - m_fDistArray[k]) * roughness;

      if (dist > 6.0)
        m_EDP[i] += (m_fRhoArray[k]) * (2.0);
      else if (dist > -6.0)
        m_EDP[i] += (m_fRhoArray[k]) * (1.0 + erf(dist));

      // Make double array for the reflectivity calculation
      m_DEDP[i] = 2.0 * m_EDP[i].real();
    }
  }
}

void CEDP::MakeEDP(ParamVector &g) {
  double dist;
  int reflpoints = m_iLayers;
  int refllayers = g.RealparamsSize() - 1;
  double roughness = g.getroughness();
  double supersld = g.GetRealparams(0) * m_dRho;

  if (g.getroughness() < 1e-6)
    roughness = 1e-6;

  roughness = 1.0 / (roughness * sqrt(2.0));

#pragma omp parallel
  {
#pragma omp for
    for (int k = 0; k < refllayers; k++) {
      m_fRhoArray[k] =
          m_dRho * (g.GetRealparams(k + 1) - g.GetRealparams(k)) / 2.0;

      // Imag calculation
      if (k == 0) {
        m_fImagRhoArray[k] =
            (m_dBeta * g.getSurfAbs() * g.GetRealparams(k + 1) /
                 g.GetRealparams(refllayers) -
             m_dBeta_Sup) /
            2.0;
      } else if (k == refllayers - 1) {
        m_fImagRhoArray[k] =
            (m_dBeta_Sub - m_dBeta * g.getSurfAbs() * g.GetRealparams(k) /
                               g.GetRealparams(refllayers)) /
            2.0;
      } else {
        m_fImagRhoArray[k] =
            (m_dBeta * g.getSurfAbs() * g.GetRealparams(k + 1) /
                 g.GetRealparams(refllayers) -
             (m_dBeta * g.getSurfAbs() * g.GetRealparams(k) /
              g.GetRealparams(refllayers)) /
                 2.0);
      }
    }

#pragma omp for private(dist)
    for (int i = 0; i < reflpoints; i++) {
      m_EDP[i] = std::complex<double>(supersld, m_dBeta);

      for (int k = 0; k < refllayers; k++) {
        dist = (m_fEDSpacingArray[i] - m_fDistArray[k]) * roughness;

        if (dist > 6) {
          m_EDP[i] += std::complex<double>((m_fRhoArray[k]) * (2.0),
                                           (m_fImagRhoArray[k]) * (2.0));
        } else if (dist > -6) {
          m_EDP[i] +=
              std::complex<double>((m_fRhoArray[k]) * (1.0 + erf(dist)),
                                   (m_fImagRhoArray[k]) * (1.0 + erf(dist)));
        }
      }

      m_DEDP[i] = 2.0 * m_EDP[i];
    }
  }
}

int CEDP::Get_EDPPointCount() const { return m_iLayers; }

bool CEDP::Get_UseABS() const { return m_bUseSurfAbs; }

double CEDP::Get_Dz() const { return m_dDz0; }

double CEDP::Get_FilmAbs() const { return m_dBeta; }

double CEDP::Get_FilmAbsInput() const {
  // Returns the value that, when passed to Set_FilmAbs(), reproduces m_dBeta.
  // Set_FilmAbs(x) stores x * m_dWaveConstant, so x = m_dBeta /
  // m_dWaveConstant.
  return (m_dWaveConstant > 0.0) ? m_dBeta / m_dWaveConstant : 0.0;
}

double CEDP::Get_WaveConstant() const { return m_dWaveConstant; }

void CEDP::Set_FilmAbs(double abs) { m_dBeta = abs * m_dWaveConstant; }
