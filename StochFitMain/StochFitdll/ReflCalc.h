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

class CReflCalc {
private:
	MyComplex *doublenk;
	float *tsinsquaredthetai,*sinsquaredthetai,*qspreadsinsquaredthetai,*qspreadreflpt,*qspreadsinthetai;


	void impnorm(float* refl, int datapoints, bool isimprefl);
	void mytransparentrf(float* sintheta, float* sinsquaredtheta, int datapoints, float* refl);
	void mkdensity(ParamVector *g);
	void mkdensitytrans(ParamVector* g);
	void QsmearRf(float* qspreadreflpt, float* reflpt, int datapoints);
	void myrf(float* sintheta, float* sinsquaredtheta, int datapoints, float* refl);
	bool CheckDensity();

	float m_dQSpread;
	float CalcQc(ParamVector g);
	float CalcFresnelPoint(float Q, float Qc);
	int Qpoints;
 
	int m_ilowEDduplicatepts;
	int m_ihighEDduplicatepts;
	bool m_bXRonly;
public:
	MyComplex *nk;
	//Variables

	//File names
	wstring fnpop;
	wstring fnrf;
	wstring fnrho;
	ReflSettings* m_InitStruct;
	double m_dwaveconstant;
    float dz0;
	float m_dboxsize;
	float m_dnormfactor;
	//read from file
    float *xi,*yi,*eyi,*exi,*sinthetai,*reflpt,*dataout,*tsinthetai,*qarray, *objarray, *fresnelcurve;
    int m_idatapoints, tarraysize;
    int nl;

	float totalsize;
    float rho_a,beta_a;
    float lambda,k0;
    float m_dChiSquare;
	float m_dgoodnessoffit;
	BOOL m_bforcenorm;
	BOOL m_bUseSurfAbs;
	BOOL m_bImpNorm;
	float* distarray;
	float* rhoarray;
	float* imagrhoarray;
	float* edspacingarray;
	int m_iuseableprocessors;

	MyComplex* m_ckk;
	float* m_dkk;
	MyComplex* m_cak;
	MyComplex* m_crj;
	float* m_drj;
	MyComplex* m_cRj;

	//Member functions
	CReflCalc();
	~CReflCalc();
	
    void init(ReflSettings* InitStruct);
	void SetupRef(double* Q, double* Refl, double* ReflError, double* QError, int PointCount, ParamVector* params);
    double objective(ParamVector  *g);
    void paramsrf(ParamVector *g);
    
	int objectivefunction;
	double beta_sub;
	double beta_sup;
};