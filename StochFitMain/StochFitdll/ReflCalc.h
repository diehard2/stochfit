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
	double totalsize;
    double lambda,k0;
	BOOL m_bforcenorm;
	BOOL m_bImpNorm;
	int m_iuseableprocessors;
	double m_dwaveconstant;
	MyComplex* m_ckk;
	double* m_dkk;
	MyComplex* m_cak;
	MyComplex* m_crj;
	double* m_drj;
	MyComplex* m_cRj;

	double *tsinsquaredthetai,*sinsquaredthetai,*qspreadsinsquaredthetai,*qspreadreflpt,*qspreadsinthetai;


	//Functions
	void impnorm(double* refl, int datapoints, bool isimprefl);
	void MyTransparentRF(double* sintheta, double* sinsquaredtheta, int datapoints, double* refl, CEDP* EDP);
	void QsmearRf(double* qspreadreflpt, double* reflpt, int datapoints);
	void MyRF(double* sintheta, double* sinsquaredtheta, int datapoints, double* refl, CEDP* EDP);
	bool CheckDensity(CEDP* EDP);
	void GetOffSets(int& HighOffset, int& LowOffset, MyComplex* EDP, int EDPoints);
	void InitializeScratchArrays(int EDPoints);
 

	bool m_bXRonly;
	BOOL m_bReflInitialized;
public:

	//Variables
	double m_dChiSquare;
	double m_dgoodnessoffit;
	float m_dQSpread;
	float m_dnormfactor;

	//read from file
    double *xi,*yi,*eyi,*exi,*sinthetai,*reflpt,*dataout,*tsinthetai,*qarray;
    int m_idatapoints, tarraysize;
	
	//Member functions
	CReflCalc();
	~CReflCalc();
	int GetDataCount();

	
    void Init(ReflSettings* InitStruct);
	void SetupRef(ReflSettings* InitStruct);
	double Objective(CEDP* EDP);
    void ParamsRF(CEDP* EDP, wstring reflfile);

	//Get/Let Functions
	double GetWaveConstant();
    
	int objectivefunction;
};