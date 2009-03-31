#pragma once

#include "ParamVector.h"

class CEDP 
{
private:
	double* m_dDistArray;
	double* m_dRhoArray;
	double* m_dImagRhoArray;
	double* m_dEDSpacingArray;
	double* m_dZ;
	double m_dRho;
    double m_dDz0;
	double m_dBeta;
	double m_dBeta_Sup;
	double m_dBeta_Sub;
	double m_dWaveConstant;
	MyComplex* m_cEDP;
	MyComplex* m_DEDP;
	int m_iLayers;
	bool m_bUseSurfAbs;

	void MakeTranparentEDP(const ParamVector* g);
	void MakeEDP(const ParamVector* g);

public:
	~CEDP();

	void Initialize(ReflSettings* InitStruct);
	void GenerateEDP(const ParamVector* g);
	double Get_LayerThickness() const;
	int Get_EDPPointCount() const;
	bool Get_UseABS() const;
	double Get_FilmAbs() const;
	MyComplex* GetDoubledEDP() const;
	double* GetZ() const;
	double Get_WaveConstant();
	void Set_FilmAbs(double absorption);
	bool CheckForNegDensity();
	void GetData(double* Z, double* EDP);

	void WriteOutputFile(wstring filename);
};