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

#include "stdafx.h"
#include "Rhocalc.h"
#include "settings.h"

void RhoCalc::init(BoxReflSettings* InitStruct)
{
   	onesigma = InitStruct->OneSigma;
	boxnumber = InitStruct->Boxes;
	ZIncrement = InitStruct->ZIncrement;
	Zlength = InitStruct->ZLength;
	MIRho = InitStruct->MIEDP;	
	SubSLD = InitStruct->SubSLD;
	m_dSupSLD = InitStruct->SupSLD;

	nk = (double*)_aligned_malloc(sizeof(double)*Zlength,16);
	nkb = (double*)_aligned_malloc(sizeof(double)*Zlength,16);
	
	distarray = (double*)_aligned_malloc((boxnumber+1)*sizeof(double),16);
	rhoarray = (double*)_aligned_malloc((boxnumber+1)*sizeof(double),16);
	rougharray = (double*)_aligned_malloc((boxnumber+1)*sizeof(double),16);

	m_LengthArray = new double[boxnumber];
	m_RhoArray = new double[boxnumber];
	m_SigmaArray = new double[boxnumber];
}

RhoCalc::~RhoCalc()
{
	_aligned_free(nk);
	_aligned_free(nkb);
	_aligned_free(distarray);
	_aligned_free(rhoarray);
	_aligned_free(rougharray);
	delete(m_LengthArray);
	delete(m_SigmaArray);
	delete(m_RhoArray);
}

 void RhoCalc::objective(double* par, double* x, int m, int n, void* data)
{
  RhoCalc* rhoinst = (RhoCalc*)data;
  rhoinst->mkdensity(par,m);

  //This gets squared in the LM routines
  for(int i=0; i<rhoinst->Zlength; ++i)
  {
		x[i] = rhoinst->MIRho[i]-rhoinst->nk[i];	
  }
}

void RhoCalc::Rhocalculate(double SubRough, double Zoffset)
{
	//The code for this section is based on the electron density calculation
	//in Motofit (www.sourceforge.net/motofit). It is a standard method of calculating the
	//electron density profile. We treat the profile as having a user defined number of boxes
	//The last 30% of the curve will converge to have rho/rhoinf = 1.0. Currently, it is only
	//useful for the air-lipid-substrate interfaces. In order to allow for a substrate-lipid-substrate
	//model, set the superphaseSLD variable. Currently, the absorbance is not allowed to vary, however this can
	//be changed by linking it to the density genome. For lipid and lipid protein films, the absorbance is negligible
	//For films with large roughnesses, we allow the roughness of the air-film interface to vary
	
	
	double SuperphaseSLD = m_dSupSLD;
	double deltarho = 0;
	double thick = 0;
	double roughness = 0;
	double dist = 0;
	double sqrt2 = sqrt(2.0);

	//Calculate the portions of the e-density equation that don't need to be repeated
	for (int i = 0; i <= boxnumber; i++)
    {
        if (i == 0)
        {
            deltarho = m_RhoArray[0] * SubSLD - SuperphaseSLD;
            thick = 0;
            roughness = m_SigmaArray[0];
        }
        else if (i == boxnumber)
        {
            deltarho = SubSLD - m_RhoArray[i - 1] * SubSLD;
            roughness = SubRough;
            thick = m_LengthArray[i - 1];
        }
        else
        {
            deltarho = (m_RhoArray[i + 1 - 1] - m_RhoArray[i - 1]) * SubSLD;
            thick = m_LengthArray[i - 1];
            roughness = m_SigmaArray[i + 1 - 1];
        }

		dist  += thick;

		distarray[i] = dist;
		rhoarray[i] = deltarho/2.0;
		rougharray[i] = roughness*sqrt2;
	}
	
	// This allows OpenMP to choose the appropriate number of threads
	// The algorithm should now scale with the number of processors in a system
	#ifdef _OPENMP
		omp_set_dynamic(TRUE);	
	#endif

	#pragma omp parallel for schedule(guided)
	for(int j = 0; j < Zlength;j++)
	{
		double summ = SuperphaseSLD;
		#pragma ivdep
		for (int i = 0; i <= boxnumber; i++)
        {
			summ += (rhoarray[i]) * (1.0 + erf((ZIncrement[j] - distarray[i]-Zoffset) / (rougharray[i])));
		}	
	
		if(SubRough != 1e-16)
		{
			nk[j] = summ/SubSLD;
		}
		else
		{
			nkb[j] = summ/SubSLD;
		}
	}
}

void RhoCalc::mkdensityboxmodel(double* p, int plength)
{
	double SubRough = 1e-16;
	double ZOffset = p[1];

	if(onesigma)
	{
		for(int i = 0; i< boxnumber;i++)
		{
			m_LengthArray[i] = p[2*i+2];
			m_RhoArray[i] = p[2*i+3];
			m_SigmaArray[i] = 1e-16;
		}

		Rhocalculate(SubRough, ZOffset);
	}
	else
	{
		for(int i = 0; i< boxnumber;i++)
		{
			m_LengthArray[i] = p[3*i+2];
			m_RhoArray[i] = p[3*i+3];
			m_SigmaArray[i] = 1e-16;
		}
		
		Rhocalculate(SubRough, ZOffset);
	}
}

void RhoCalc::mkdensity(double* p, int plength)
{
	//Move our parameters into individual arrays so they're easier to deal with

	double SubRough = p[0];
	double ZOffset = p[1];

	if(onesigma)
	{
		for(int i = 0; i< boxnumber;i++)
		{
			m_LengthArray[i] = p[2*i+2];
			m_RhoArray[i] = p[2*i+3];
			m_SigmaArray[i] = p[0];
		}
	}
	else
	{
		for(int i = 0; i< boxnumber;i++)
		{
			m_LengthArray[i] = p[3*i+2];
 			m_RhoArray[i] = p[3*i+3];
			m_SigmaArray[i] = p[3*i+4];
		}

	}
	
	Rhocalculate(SubRough, ZOffset);
}

void RhoCalc::writefiles(const char* filename)
{
	std::ofstream outrhofile(filename);
	for(int i = 0; i<Zlength;i++)
	{
		outrhofile<< ZIncrement[i] << ' ' << nk[i] << ' ' << nkb[i] << std::endl;
	}
	outrhofile.close();
}