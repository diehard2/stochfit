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
	MyComplex<double> *doublenk;
	double *tsinsquaredthetai,*sinsquaredthetai,*qspreadsinsquaredthetai,*qspreadreflpt,*qspreadsinthetai;


	void impnorm(double* refl, int datapoints, bool isimprefl);
	void mytransparentrf(double* sintheta, double* sinsquaredtheta, int datapoints, double* refl);
	void mkdensity(ParamVector *g);
	void mkdensitytrans(ParamVector* g);
	void QsmearRf(double* qspreadreflpt, double* reflpt, int datapoints);
	void myrf(double* sintheta, double* sinsquaredtheta, int datapoints, double* refl);
	bool CheckDensity();

	double m_dQSpread;
	double CalcQc(ParamVector g);
	double CalcFresnelPoint(double Q, double Qc);
	int Qpoints;
 
	int m_ilowEDduplicatepts;
	int m_ihighEDduplicatepts;
	bool m_bXRonly;
public:
	MyComplex<double> *nk;
	//Variables

	//File names
	wstring fnpop;
	wstring fnrf;
	wstring fnrho;

    double dz0;
	double m_dboxsize;
	double m_dnormfactor;
	//read from file
    double *xi,*yi,*eyi,*exi,*sinthetai,*reflpt,*dataout,*tsinthetai,*qarray, *objarray, *fresnelcurve;
    int m_idatapoints, tarraysize;
    int nl;

	double totalsize;
    float rho_a,beta_a;
    double lambda,k0;
    double m_dChiSquare;
	double m_dgoodnessoffit;
	BOOL m_bforcenorm;
	BOOL m_bUseSurfAbs;
	BOOL m_bImpNorm;
	float* distarray;
	float* rhoarray;
	float* imagrhoarray;
	float* edspacingarray;
	int m_iuseableprocessors;

	MyComplex<double>* m_ckk;
	double* m_dkk;
	MyComplex<double>* m_cak;
	MyComplex<double>* m_crj;
	double* m_drj;
	MyComplex<double>* m_cRj;

	//Member functions
	CReflCalc();
	~CReflCalc();
	
    void init(int layernumber,double xraylambda,double dz, BOOL mbusesurfabs,int parratlayers, double leftoffset, BOOL forcenorm, double qspread, bool XRonly);
	void SetupRef(double* Q, double* Refl, double* ReflError, double* QError, int PointCount, ParamVector* params);
    double objective(ParamVector  *g);
    void paramsrf(ParamVector *g);
    
	int objectivefunction;
	double beta_sub;
	double beta_sup;
};