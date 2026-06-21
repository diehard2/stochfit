#pragma once

// Electron density profile (EDP) generator for SA fitting.
// Builds the real and imaginary EDP arrays (m_EDP, m_DEDP) from a ParamVector
// using Gaussian interface broadening (Motofit-style erf convolution).
// Supports both transparent films and absorbing films via BuildEDP<Absorbing>.
// m_DEDP = 2 * m_EDP and is the input to the Parratt reflectivity calculation.

#include "LayerStack.h"
#include "ParamVector.h"

class CEDP {
private:
  vector<double> m_fDistArray;
  vector<double> m_fRhoArray;
  vector<double> m_fImagRhoArray;
  vector<double> m_fEDSpacingArray;

  // Precomputed per-layer length multiplier {0, -2*dz} for BuildLayerStack.
  vector<std::complex<double>> m_length_mult;

  double m_dRho;
  double m_dLambda;
  double m_dDz0;
  double m_dBeta;
  double m_dBeta_Sup;
  double m_dBeta_Sub;
  double m_dWaveConstant;

  int m_iLayers;

  bool m_bUseSurfAbs;

  // Cached flat-region offsets; updated at end of GenerateEDP.
  mutable int m_supOff = 0;
  mutable int m_subOff = 0;

  template <bool Absorbing>
  void BuildEDP(ParamVector &g);
  template <bool Absorbing>
  void FillBoxArraysImpl(ParamVector &g);

public:
  CEDP() = default;
  explicit CEDP(const ReflSettings &s) { Init(s); }

  void Init(const ReflSettings &InitStruct);
  int GetLayerCount() const { return m_iLayers; }

  // Standalone EDP build — includes serial setup, parallel EDP loop, and offset caching.
  void GenerateEDP(ParamVector &g);

  // Cooperative path (SA persistent-team use):
  //   1. Call FillBoxArrays(g) inside your omp single block before the parallel section.
  //   2. Call GenerateEDPCooperative(g) — runs only the parallel omp for.
  //   3. Call BuildLayerStackFull() inside the next omp single — computes offsets fresh.
  void FillBoxArrays(ParamVector &g);         // serial setup only, no omp pragma
  void GenerateEDPCooperative(ParamVector &g); // omp for only, no serial omp singles

  int Get_EDPPointCount() const;
  bool Get_UseABS() const;
  double Get_FilmAbs() const;
  double Get_FilmAbsInput()
      const; // returns m_dBeta / m_dWaveConstant — inverse of Set_FilmAbs
  double Get_Dz() const;
  double Get_LeftOffset() const; // superphase padding = 6 * RoughnessMax
  double Get_WaveConstant() const;
  void Set_FilmAbs(double absorption);
  std::pair<int, int> GetOffSets() const;

  // Returns a zero-copy view of m_DEDP suitable for ParrattReflectivity.
  // Valid only after a GenerateEDP() call. No roughness (σ=0), sup/sub offsets
  // from the most recent GenerateEDP, transparent flag from m_bUseSurfAbs.
  LayerStack BuildLayerStack() const;

  // Like BuildLayerStack but computes sup/sub offsets fresh from m_DEDP — use
  // in the cooperative SA path where GenerateEDPCooperative skipped GetOffSets.
  // Const (m_supOff/m_subOff are mutable); call from within an omp single.
  LayerStack BuildLayerStackFull() const;

  vector<std::complex<double>> m_EDP;
  vector<std::complex<double>> m_DEDP;
};
