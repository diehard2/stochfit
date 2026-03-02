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

class Reflcalc
{
private:
   MyComplex *doublenk;
   double *sinthetai,*sinsquaredthetai;
public:
	
	MyComplex *nk, *nkb;
	double* Realrefl, *Realreflerrors; int realrefllength;
	//Variables

	int boxnumber;
	int SubSLD;
	//File names
	

    double dz0;
	double sdyi;

	//read from file
    double *reflpt;
    int m_idatapoints;
    int nl;
	double totalsize;

    double lambda;
	double m_dcritedgeoff;
	
	//Member functions
	
	Reflcalc()
	{
    }

	~Reflcalc();
 
    void init(double xraylambda,int boxnumber, double subSLD, double* p, int pcount, double* RealRefl, int RealRefllength, bool onesigma);
    void MakeTheta(double* QRange, int QRangesize);
   
	//Density calcs
	int CalculateZLength();
	void MakeZ();
	double* ZIncrement;
	int Zlength;
	double* param; 
	int pcount;
	void Rhocalc(double SubRough, double* LengthArray, double* RhoArray, double* SigmaArray);
	void mkdensityboxmodel(double* p, int plenght, bool onesigma);
	void mkdensityonesigma(double* p, int plength);
    void mkdensity(double* p, int plength);
	double myrf();
	double CalcQc(double dSLD);
	double CalcFresnelPoint(double Q, double Qc);
	bool onesigma;
	void setbulk(double a, double b)
	{
        nk[nl-1]= MyComplex(a*1e-6*lambda*lambda/(2.*M_PI),-b);
		nkb[nl-1] = MyComplex(a*1e-6*lambda*lambda/(2.*M_PI),-b);
    };

	
	static void objective(double *p, double *x, int m, int n, void *data);
};