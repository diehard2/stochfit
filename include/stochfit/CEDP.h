#pragma once

// Electron density profile (EDP) generator for SA fitting.
// Builds the real and imaginary EDP arrays (m_EDP, m_DEDP) from a ParamVector
// using Gaussian interface broadening (Motofit-style erf convolution).
// Supports both transparent films (MakeTranparentEDP) and absorbing films
// (MakeEDP). All internal scratch arrays are double-precision vectors. m_DEDP =
// 2 * m_EDP and is the input to the Parratt reflectivity calculation.

#include "ParamVector.h"

class CEDP {
private:
  vector<double> m_fDistArray;
  vector<double> m_fRhoArray;
  vector<double> m_fImagRhoArray;
  vector<double> m_fEDSpacingArray;

  double m_dRho;
  double m_dLambda;
  double m_dDz0;
  double m_dBeta;
  double m_dBeta_Sup;
  double m_dBeta_Sub;
  double m_dWaveConstant;

  int m_iLayers;

  bool m_bUseSurfAbs;

  void MakeTranparentEDP(ParamVector &g);
  void MakeEDP(ParamVector &g);

public:
  void Init(const ReflSettings &InitStruct);
  void GenerateEDP(ParamVector &g);
  int Get_EDPPointCount() const;
  bool Get_UseABS() const;
  double Get_FilmAbs() const;
  double Get_FilmAbsInput()
      const; // returns m_dBeta / m_dWaveConstant — inverse of Set_FilmAbs
  double Get_Dz() const;
  double Get_WaveConstant() const;
  void Set_FilmAbs(double absorption);
  std::pair<int, int> GetOffSets() const;
  vector<std::complex<double>> m_EDP;
  vector<std::complex<double>> m_DEDP;
};
