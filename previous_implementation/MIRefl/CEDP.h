#pragma once

#include "ParamVector.h"

class CEDP {
private:
	float* m_fDistArray;
	float* m_fRhoArray;
	float* m_fImagRhoArray;
	float* m_fEDSpacingArray;
	
	double m_dRho;
	double m_dLambda;
    double m_dDz0;
	double m_dBeta;
	double m_dBeta_Sup;
	double m_dBeta_Sub;
	double m_dWaveConstant;
	float* m_BoxRho;
	double m_Roughness;
	int m_iLayers;
	int m_Boxes;
	
	BOOL m_bUseSurfAbs;

	void MakeTranparentEDP();
	void MakeEDP();

public:
	~CEDP();
	CEDP();
	void Init(ReflSettings* InitStruct, double SLD[], double Thickness[], double roughness);
	void GenerateEDP();
	double Get_LayerThickness();

	int Get_EDPPointCount();
	BOOL Get_UseABS();
	double Get_Dz();
	void WriteOutputFile(wstring filename);

	MyComplex* m_EDP;
	MyComplex* m_DEDP;
};