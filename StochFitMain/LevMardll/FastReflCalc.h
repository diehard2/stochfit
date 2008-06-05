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

class FastReflcalc
{
private:

   double *sinthetai,*sinsquaredthetai, *qspreadsinthetai, *qspreadsinsquaredthetai, *qspreadreflpt;
   
   void ImpNorm(double* refl, int datapoints);

   BOOL m_bImpNorm;

   double m_dQSpread;

protected:
	virtual void CalcRefl(double* sintheta, double* sinsquaredtheta, int datapoints, double* refl);
     double m_dnormfactor;
public:
	
	//Variables

	double* Realrefl, *Realreflerrors; int realrefllength;
	int boxnumber, SubSLD;
	
    double dz0;
	double sdyi;
	double m_dsupsld;

    double *reflpt;
    int m_idatapoints;
 
	double totalsize;

    double lambda;
	double m_dcritedgeoff;
	
	//Member functions
	
	~FastReflcalc();
	void QsmearRf(double* qspreadrefl, double* refl, int datapoints);
    void init(double xraylambda,int boxnumber, double subSLD, double SupSLD, double* p, int pcount, double* RealRefl, double* RealReflErrors, int RealRefllength, bool onesigma, double QSpread, double normfactor, BOOL impnorm);
    void MakeTheta(double* QRange, double* QError, int QRangesize);

	double* originalparams;
	//Density calcs
	int CalculateZLength();
	void MakeZ();
	double* param; 
	int pcount;
	virtual void mkdensityonesigma(double* p, int plength);
    virtual void mkdensity(double* p, int plength);
	void myrfdispatch();
	
	double CalcQc(double dSLD);
	double CalcFresnelPoint(double Q, double Qc);
	bool onesigma;
	double subphaseSLD;

	double* LengthArray;
	double* RhoArray;
	double* SigmaArray;
	double* ImagArray;

	virtual void Rhocalculate(double Zoffset,double* ZIncrement, double* LengthArray, double* RhoArray, double* SigmaArray, double* nk, double* nkb, int counter);

	
	static void objective(double *p, double *x, int m, int n, void *data);
};