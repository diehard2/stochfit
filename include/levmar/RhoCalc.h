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

class RhoCalc
{
private:
	vector<double> distarray;
	vector<double> rhoarray;
	vector<double> rougharray;
	vector<double> MIRho;
	vector<double> ZIncrement;

	vector<double> m_LengthArray;
	vector<double> m_RhoArray;
	vector<double> m_SigmaArray;
	
	int pcount;
	bool onesigma;
	double SubSLD;
	double m_dSupSLD;
	int boxnumber;
    double dz0;
	int Zlength;


	void Rhocalculate(double SubRough,double Zoffset);

public:	
	//Member functions
	~RhoCalc();
    void init(const BoxReflSettings& InitStruct);
	void mkdensityboxmodel(std::span<const double> p);
    void mkdensity(std::span<const double> p);
    static void objective(double *p, double *x, int m, int n, void *data);

	vector<double> nk;
	vector<double> nkb;
};