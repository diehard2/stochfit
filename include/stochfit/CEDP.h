#pragma once

// Electron density profile (EDP) generator for SA fitting.
// Builds the real and imaginary EDP arrays (m_EDP, m_DEDP) from a ParamVector
// using Gaussian interface broadening (Motofit-style erf convolution).
// Supports both transparent films (MakeTranparentEDP) and absorbing films (MakeEDP).
// All internal scratch arrays are double-precision vectors.
// m_DEDP = 2 * m_EDP and is the input to the Parratt reflectivity calculation.

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

	void MakeTranparentEDP(ParamVector* g);
	void MakeEDP(ParamVector* g);

public:
	void Init(ReflSettings* InitStruct);
	void GenerateEDP(ParamVector* g);
	int Get_EDPPointCount();
	bool Get_UseABS();
	double Get_FilmAbs();
	double Get_FilmAbsInput(); // returns m_dBeta / m_dWaveConstant — inverse of Set_FilmAbs
	double Get_Dz();
	double Get_WaveConstant();
	void Set_FilmAbs(double absorption);
	void WriteOutputFile(string filename);

	vector<std::complex<double>> m_EDP;
	vector<std::complex<double>> m_DEDP;
};
