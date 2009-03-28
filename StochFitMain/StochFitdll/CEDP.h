#pragma once

#include "ParamVector.h"

class CEDP 
{
private:
	float* m_fDistArray;
	float* m_fRhoArray;
	float* m_fImagRhoArray;
	float* m_fEDSpacingArray;
	float* m_fZ;
	double m_dRho;
	double m_dLambda;
    double m_dDz0;
	double m_dBeta;
	double m_dBeta_Sup;
	double m_dBeta_Sub;
	double m_dWaveConstant;
	MyComplex* m_EDP;
	MyComplex* m_DEDP;
	int m_iLayers;
	
	BOOL m_bUseSurfAbs;

	void MakeTranparentEDP(ParamVector* g);
	void MakeEDP(ParamVector* g);

public:
	~CEDP();

	void Init(ReflSettings* InitStruct);
	void GenerateEDP(ParamVector* g);
	double Get_LayerThickness();
	int Get_EDPPointCount();
	BOOL Get_UseABS();
	float Get_FilmAbs();
	MyComplex* GetDoubledEDP();
	float* GetZ();
	float Get_WaveConstant();
	void Set_FilmAbs(float absorption);
	bool CheckForNegDensity();
	void GetData(double* Z, double* EDP);

	void WriteOutputFile(wstring filename);
};