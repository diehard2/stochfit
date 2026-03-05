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

#include <stochfit/common/platform.h>
#include "ReflCalc.h"


/* ----------------------------------------------------------------------------
  Dongxu Li and Stephen Danauskas
  Copyright (c) 2007 University of Chicago

---------------------------------------------------------------------------- */



// Multilayer reflection and transmission
// Parrett scheme, L. G. Parrett, Phys. Rev. 95(2), 359(1954)
// formulea to use
//      n \cos(\theta) = \cos(\theta_0)
//
//      (E_{k,+} + E_{k,-} ) \cos(\theta_k) = ( E_{k+1,+}/a + E_{k+1,-} a) \cos( \theta_{k+1})
//      (E_{k,+} - E_{k,-} ) \sin(\theta_k) = ( E_{k+1,+}/a - E_{k+1,-} a) \sin( \theta_{k+1})
//      a = exp( i k_{k+1} d_{k+1})
//      k_{k+1} = sqrt( n^2 - \cos^2(\theta_0)) k_0
//

//
// Due to restriction with OpenMP, and vectorization all of the arrays must be dynamically
// declared.

Reflcalc::~Reflcalc()
{
	platform_aligned_free(sinsquaredthetai);
	platform_aligned_free(doublenk);
	platform_aligned_free(sinthetai);
	platform_aligned_free(nk);
	platform_aligned_free(reflpt);
	platform_aligned_free(nkb);
}

void Reflcalc::init(double xraylambda,int boxes, double subSLD, double* parameters, int paramcount, double* refldata, int refldatacount, bool onesig)
{
    lambda=xraylambda;
	onesigma = onesig;
	Realrefl = refldata;
	realrefllength = refldatacount;
	param = parameters;
	pcount = paramcount;
	boxnumber = boxes;

	MakeZ();

    nk = (MyComplex*)platform_aligned_alloc(sizeof(MyComplex)*nl,16);
	doublenk = (MyComplex*)platform_aligned_alloc(sizeof(MyComplex)*nl,16);
	nkb = (MyComplex*)platform_aligned_alloc(sizeof(MyComplex)*nl,16);

	//Neglect absorption
	setbulk(subSLD,0);
}

void Reflcalc::MakeTheta(double* QRange, int QRangesize)
{
	m_idatapoints = QRangesize;
	double holder;

	sinthetai = (double*)platform_aligned_alloc(QRangesize*sizeof(double),16);
	sinsquaredthetai = (double*)platform_aligned_alloc(QRangesize*sizeof(double),16);
	reflpt = (double*)platform_aligned_alloc(QRangesize*sizeof(double),16);

	for(int i = 0; i< m_idatapoints; i++)
	{
		sinthetai[i] = QRange[i]*lambda/(4*M_PI);
	}

	for (int l = 0; l < m_idatapoints; l++)
    {
        sinsquaredthetai[l] = sinthetai[l]*sinthetai[l];
    }
}

int Reflcalc::CalculateZLength()
{
			double totallength = 0;

			if(onesigma == true)
			{
				for (int i = 0; i < boxnumber; i++)
				{
					totallength += param[(2*i+1)];
				}
			}
			else
			{
				for (int i = 0; i < boxnumber; i++)
				{
					totallength += param[(3*i+1)];
				}
			}

			totallength = 100 + (int)totallength + 100;
            return totallength;
}

 void Reflcalc::objective(double* par, double* x, int m, int n, void* data)
{
  Reflcalc* reflinst = (Reflcalc*)data;

  if(reflinst->onesigma == true)
	  reflinst->mkdensityonesigma(par,m);
  else
	  reflinst->mkdensity(par,m);

  reflinst->myrf();

  for(int i=0; i<n; ++i)
	  x[i] = log(reflinst->reflpt[i]/reflinst->Realrefl[i])/fabs(log(reflinst->Realreflerrors[i]));

}

void Reflcalc::MakeZ()
{
			//5 points per Angstrom
			double dx = 1.0/5.0;
			dz0 = dx;
            double Length = (double)CalculateZLength();
			Zlength = (int)Length*5;
			nl = Zlength;
			ZIncrement = (double*)platform_aligned_alloc(Zlength*sizeof(double),16);
			for (int i = 0; i < Zlength; i++)
            {
                ZIncrement[i] = i*dx;
            }
}

void Reflcalc::Rhocalc(double SubRough, double* LengthArray, double* RhoArray, double* SigmaArray)
{
	int leftoffset = 100;
	int refllayers = boxnumber;
	int reflpoints = nl;
	double SuperphaseSLD = 0;
	double deltarho = 0;
	double thick = 0;
	double roughness = 0;
	double dist = 0;

	//Create arrays so we don't have to redo this calculation for every data point

	double* distarray = (double*)platform_aligned_alloc((refllayers+1)*sizeof(double),16);
	double* rhoarray = (double*)platform_aligned_alloc((refllayers+1)*sizeof(double),16);
	double* rougharray = (double*)platform_aligned_alloc((refllayers+1)*sizeof(double),16);

	//Calculate the portions of the e-density equation that don't need to be repeated
	double SubSLD = nk[nl-1].real();
	for (int i = 0; i <= refllayers; i++)
    {
        if (i == 0)
        {
            deltarho = RhoArray[0] * SubSLD - SuperphaseSLD;
            thick = 0;
            roughness = SigmaArray[0];
        }
        else if (i == refllayers)
        {
            deltarho = SubSLD - RhoArray[i - 1] * SubSLD;
            roughness = SubRough;
            thick = LengthArray[i - 1];
        }
        else
        {
            deltarho = (RhoArray[i + 1 - 1] - RhoArray[i - 1]) * SubSLD;
            thick = LengthArray[i - 1];
            roughness = SigmaArray[i + 1 - 1];
        }

		dist  += thick;

		distarray[i] = dist;
		rhoarray[i] = deltarho;
		rougharray[i] = roughness;
	}

	double sqrt2 = sqrt(2.0);
	int loopcount = nl-1;

	// This allows OpenMP to choose the appropriate number of threads
	// The algorithm should now scale with the number of processors in a system

	#ifdef OMP_PARALLEL
		omp_set_dynamic(TRUE);
	#endif

	#pragma omp parallel for schedule(guided)
	for(int j = 0; j < loopcount;j++)
	{
		double summ = 0;
		#pragma ivdep
		for (int i = 0; i <= refllayers; i++)
        {
            summ += (rhoarray[i] / 2.0) * (1.0 + erf((j*dz0 - distarray[i]-leftoffset) / (rougharray[i] * sqrt2)));
		}

		if(SubRough != 1e-16)
		{
			nk[j] = summ;
		}
		else
		{
			nkb[j] = summ;
		}
	}

	//Make double array for the reflectivity calculation
	#pragma ivdep
	for(int i = 0; i<reflpoints;i++)
	{
		doublenk[i] = 2.0*nk[i].real();
	}

	//Free arrays

	platform_aligned_free(distarray);
	platform_aligned_free(rhoarray);
	platform_aligned_free(rougharray);
}

void Reflcalc::mkdensityboxmodel(double* p, int plength, bool onesigma)
{
	double SubRough = 1e-16;
	double LengthArray[6];
	double RhoArray[6];
	double SigmaArray[6];

	if(onesigma == true)
	{
		for(int i = 0; i< boxnumber;i++)
		{
			LengthArray[i] = p[2*i+1];
			RhoArray[i] = p[2*i+2];
			SigmaArray[i] = 1e-16;
		}

		Rhocalc(SubRough, LengthArray, RhoArray, SigmaArray);
	}
	else
	{
		for(int i = 0; i< boxnumber;i++)
		{
			LengthArray[i] = p[3*i+1];
			RhoArray[i] = p[3*i+2];
			SigmaArray[i] = 1e-16;
		}

		Rhocalc(SubRough, LengthArray, RhoArray, SigmaArray);
	}
}

void Reflcalc::mkdensityonesigma(double* p, int plength)
{
	//Dump our parameters into individual arrays so they're easier to deal with

	double SubRough = p[0];
	double LengthArray[6];
	double RhoArray[6];
	double SigmaArray[6];

	for(int i = 0; i< boxnumber;i++)
	{
		LengthArray[i] = p[2*i+1];
		RhoArray[i] = p[2*i+2];
		SigmaArray[i] = p[0];
	}

	Rhocalc(SubRough, LengthArray, RhoArray, SigmaArray);
}

void Reflcalc::mkdensity(double* p, int plength)
{
	//Move our parameters into individual arrays so they're easier to deal with

	double SubRough = p[0];
	double LengthArray[6];
	double RhoArray[6];
	double SigmaArray[6];

	for(int i = 0; i< boxnumber;i++)
	{
		LengthArray[i] = p[3*i+1];
		RhoArray[i] = p[3*i+2];
		SigmaArray[i] = p[3*i+3];
	}

	Rhocalc(SubRough, LengthArray, RhoArray, SigmaArray);
}

double Reflcalc::myrf()
{
	int counter = m_idatapoints;
	//Complex constants

	MyComplex k0((2.0*M_PI/lambda),0.0);



	// This allows OpenMP to choose the appropriate number of threads
	// The algorithm should now scale with the number of processors in a system

	#ifdef OMP_PARALLEL
	omp_set_dynamic(TRUE);
	#endif

	#pragma omp parallel for schedule(guided)
	for(int l = 0; l<counter;l++)
	{
		//
		//Arrays and temporary variables for the loop - these must be declared inside the
		//loop for OpenMP
		//
		MyComplex lengthmultiplier = -1.0*MyComplex(0.0,1.0)*dz0/2.0 ;
		MyComplex* kk = (MyComplex*)platform_aligned_alloc(sizeof(MyComplex)*nl,16);
		MyComplex* ak = (MyComplex*)platform_aligned_alloc(sizeof(MyComplex)*nl,16);
		MyComplex* rj = (MyComplex*)platform_aligned_alloc(sizeof(MyComplex)*nl,16);
		MyComplex* Rj = (MyComplex*)platform_aligned_alloc(sizeof(MyComplex)*nl,16);
		//In order to vectorize loops, you cannot use global variables
		int nlminone = nl-1;
		int numlay =  nl;

		//Variables that hold a value in the loop - once again, declared in the loop
		//to simplify OpenMP operations

		double holder;
		MyComplex cholder;

		/********Boundary conditions********/
		//No reflection in the last layer
		Rj[nlminone] = 0.0;
		//The first layer and the last layer are assumed to be infinite
		ak[0] = 1.0;
		ak[nlminone] = 1.0;
		//The refractive index for air is 1, so there is no refractive index term for kk[0]
		kk[0] = k0*sinthetai[l];

		//Workout the wavevector k -> kk[i] = k0 *std::sqrt(sinsquaredthetai[l]-2.0*nk[i]);

		for(int i = 1; i<numlay;i++)
		{
			kk[i] = k0 * std::sqrt(sinsquaredthetai[l]-doublenk[i]);
		}

		//Make the ak -> ak[i] = std::exp(-1.0*imaginary*kk[i]*dz0/2.0);

		for(int i = 1; i<nlminone;i++)
		{
			ak[i] = std::exp(kk[i] * lengthmultiplier);
		}

		//Make the Fresnel coefficients -> rj[i] =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);

		for(int i = 0; i < nlminone;i++)
		{
			rj[i] =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);
		}


		#pragma omp critical (reflcalc)
		{
			//Parratt recursion
			for(int i = nl-2; i>=0;i--)
			{
				cholder = ak[i]*ak[i];
				Rj[i] = cholder*cholder*(Rj[i+1]+rj[i])/(Rj[i+1]*rj[i]+1.0);
			}

			//The magnitude of the reflection at layer 0 is the reflectivity of the film
			holder = std::abs(Rj[0]);
			reflpt[l] = holder*holder;
		}

	platform_aligned_free(kk);
	platform_aligned_free(ak);
	platform_aligned_free(rj);
	platform_aligned_free(Rj);

  }

    return(0);
}

double Reflcalc::CalcQc(double dSLD)
{
    return 4 * sqrt(M_PI * SubSLD * 1e-6);
}

double Reflcalc::CalcFresnelPoint(double Q, double Qc)
{
    if (Q <= Qc)
        return 1;
    else
    {
        double term1 = sqrt(1 - (Qc / Q)*(Qc/Q));
        return ((1.0 - term1) / (1.0 + term1))*((1.0 - term1) / (1.0 + term1));
    }
}
