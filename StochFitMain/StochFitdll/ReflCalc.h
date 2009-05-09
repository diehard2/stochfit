/* 
 *	Copyright (C) 2008 Stephen Danauskas
 *	
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once
#include "ParamVector.h"
#include "CEDP.h"

class CReflCalc {
private:

	//Variables
	double m_dWaveVecConst;
	double m_dNormFactor;
	int m_iDataPoints;
	int m_iUseableProcessors;
	
	double* m_dWaveVec;
	MyComplex* m_cWaveVec;
	MyComplex* m_cPhaseFactor;
	MyComplex* m_cFresnelCoefs;
	double* m_dFresnelCoefs;
	MyComplex* m_cReflectivity;

	bool m_bHasQError;
	bool m_bForceNorm;
	bool m_bImpNorm;
	
    double *m_dQ;
	double *m_dRefl;
	double *m_dSinSquaredTheta;
	double *m_dQSpreadSinSquaredTheta;
	double *m_dQSpreadRefl;

	static const double m_Smear[];
	static const double m_SmearWeight[];
	
	//Functions
	void SetupRefl(const ReflSettings* InitStruct);
	void ImpNorm(double* refl, const int datapoints, bool isimprefl);
	void FullRF(const double* sinsquaredtheta, int datapoints, double* refl, const CEDP* EDP);
	void TransparentRF(const double* sinsquaredtheta, int datapoints, double* refl, const CEDP* EDP);
	void OpaqueRF(const double* sinsquaredtheta, int datapoints, double* refl, const CEDP* EDP);
	void QsmearRf();
	void GetOffSets(int& HighOffset, int& LowOffset,const MyComplex* EDP, int EDPoints);
	void InitializeScratchArrays(int EDPoints);
	

public:

	CReflCalc();
	~CReflCalc();

    void Initialize(const ReflSettings* InitStruct);
	void MakeReflectivity(const CEDP* EDP);
	void ForceReflectivityCalc(const CEDP* EDP, CalculationEnum x);
	//Get/Set Functions
	void GetData(double* Q, double* Refl);
	const double* GetReflData();
	void WriteOutputFile(wstring filename);
	void SetNormFactor(double NormFactor);
	int GetDataPoints();
};