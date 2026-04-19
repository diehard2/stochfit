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
#include "Settings.h"
#include <span>
class FastReflcalc
{
private:

   vector<double> sinthetai, sinsquaredthetai, qspreadsinthetai, qspreadsinsquaredthetai, qspreadreflpt;
   int m_icritqoffset, m_ihighqoffset;
   void ImpNorm(std::span<double> refl);
   void MakeTheta(const BoxReflSettings& InitStruct);
   bool m_bImpNorm;
   vector<double> Realreflerrors;
   int realrefllength;
   double m_dQSpread;

protected:
	 virtual void CalcRefl(std::span<const double> sintheta, std::span<const double> sinsquaredtheta, std::span<double> refl);
     double m_dnormfactor;
public:

	//Variables

	vector<double> Realrefl;
	int boxnumber, SubSLD;

	double m_dsupsld;

    vector<double> reflpt;
    int m_idatapoints;

	double totalsize;

    double lambda;
	double m_dcritedgeoff;

	//Member functions

	~FastReflcalc();
	void QsmearRf(std::span<const double> qspreadrefl, std::span<double> refl);
    void init(const BoxReflSettings& InitStruct);
	void SetOffsets(int LowQOffset, int HighQOffset);

	//Density calcs
	int CalculateZLength();
	void MakeZ();

	virtual void mkdensityonesigma(std::span<const double> p);
    virtual void mkdensity(std::span<const double> p);
	void myrfdispatch();

	bool onesigma;
	double subphaseSLD;

	vector<double> LengthArray;
	vector<double> RhoArray;
	vector<double> SigmaArray;
	vector<double> ImagArray;

	static void objective(double *p, double *x, int m, int n, void *data);
};