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

#include "platform.h"
#include "FastReflCalc.h"
#include "Settings.h"
#include "stochfit/QSmear.h"

FastReflcalc::~FastReflcalc()
{
}

void FastReflcalc::init(BoxReflSettings* InitStruct)
{
    lambda=InitStruct->Wavelength;
	onesigma = InitStruct->OneSigma;
	Realrefl = InitStruct->Refl;
	Realreflerrors = InitStruct->ReflError;
	realrefllength = InitStruct->QPoints;
	subphaseSLD = InitStruct->SubSLD;
	m_dsupsld = InitStruct->SupSLD;
	boxnumber = InitStruct->Boxes;;
	m_dQSpread = InitStruct->QSpread/100.0;
	m_bImpNorm = InitStruct->ImpNorm;;
	m_dnormfactor = 1.0;
	m_icritqoffset = InitStruct->LowQOffset;
	m_ihighqoffset = InitStruct->HighQOffset;
	m_idatapoints = InitStruct->QPoints;

	RhoArray.resize(boxnumber+2);
	SigmaArray.resize(boxnumber+2);
	LengthArray.resize(boxnumber+2);
	ImagArray.resize(boxnumber+2);

	MakeTheta(InitStruct);
}

void FastReflcalc::MakeTheta(BoxReflSettings* InitStruct)
{

	double* QRange = InitStruct->Q;
	double* QError = InitStruct->QError;

	sinthetai.resize(m_idatapoints);
	sinsquaredthetai.resize(m_idatapoints);
	qspreadsinthetai.resize(m_idatapoints*20);
	qspreadsinsquaredthetai.resize(m_idatapoints*20);
	qspreadreflpt.resize(m_idatapoints*20);

	reflpt.resize(m_idatapoints);

	for(int i = 0; i< m_idatapoints; i++)
	{
		sinthetai[i] = QRange[i]*lambda/(4.0*std::numbers::pi);
	}


	for (int l = 0; l < m_idatapoints; l++)
		sinsquaredthetai[l] = sinthetai[l]*sinthetai[l];

	// Calculate the qspread sinthetai's for resolution smearing
	QSmear::BuildArrays(m_idatapoints, lambda, m_dQSpread,
	                    sinthetai.data(), QError,
	                    qspreadsinthetai.data(), qspreadsinsquaredthetai.data());
}


 void FastReflcalc::objective(double* par, double* x, int m, int n, void* data)
{
  FastReflcalc* reflinst = (FastReflcalc*)data;


  if(reflinst->onesigma)
	  reflinst->mkdensityonesigma(par,m);
  else
	  reflinst->mkdensity(par,m);

  reflinst->myrfdispatch();

  memset(x, 0, n*sizeof(double));
  for(int i = reflinst->m_icritqoffset; i < n - reflinst->m_ihighqoffset; i++)
  {
		x[i] = (log(reflinst->reflpt[i])-log(reflinst->Realrefl[i]));

		// NaN guard: clang -ffast-math/-ffinite-math-only eliminates "x != x" checks,
		// letting NaN propagate into levmar's JtJ matrix and crash dsytf2.
		// Use a bit-level check that is immune to finite-math-only optimizations.
		std::uint64_t bits;
		std::memcpy(&bits, &x[i], sizeof(bits));
		if ((bits & 0x7FFFFFFFFFFFFFFFULL) > 0x7FF0000000000000ULL)
		{
			x[i] = 1e6;
		}
  }
}

void FastReflcalc::mkdensityonesigma(double* p, int plength)
{
	//Move our parameters into individual arrays so they're easier to deal with
	double rhofactor = 1e-6*lambda*lambda/(2.0*std::numbers::pi);
	double SubRough = p[0];

	RhoArray[0] = m_dsupsld*rhofactor;
	LengthArray[0] = 0.0;
	for(int i = 1; i< boxnumber+1;i++)
	{
		LengthArray[i] = p[2*(i-1)+1];
		RhoArray[i] = p[2*(i-1)+2]*subphaseSLD*rhofactor;
		SigmaArray[i-1] = p[0];
	}

	RhoArray[boxnumber+1] = subphaseSLD*rhofactor;
	SigmaArray[boxnumber] = p[0];
	LengthArray[boxnumber+1] = 0;
	m_dnormfactor = p[plength-1];
}

void FastReflcalc::mkdensity(double* p, int plength)
{
	//Move our parameters into individual arrays so they're easier to deal with
    double rhofactor = 1e-6*lambda*lambda/(2.*std::numbers::pi);

	RhoArray[0] = m_dsupsld*rhofactor;
	LengthArray[0] = 0.0;
	for(int i = 1; i< boxnumber+1;i++)
	{
		LengthArray[i] = p[3*(i-1)+1];
		RhoArray[i] = p[3*(i-1)+2]*subphaseSLD*rhofactor;

		if(fabs(p[3*(i-1)+3]) < 1e-8)
			SigmaArray[i-1] = 1e-8;
		else
			SigmaArray[i-1] = p[3*(i-1)+3];

	}

	RhoArray[boxnumber+1] = subphaseSLD*rhofactor;
	SigmaArray[boxnumber] = p[0];
	LengthArray[boxnumber+1] = 0;

	m_dnormfactor = p[plength-1];
}

void FastReflcalc::CalcRefl(double* sintheta, double* sinsquaredtheta, int datapoints, double* refl)
{
	//Generate the Parratt reflectivity layers
	int nl = boxnumber+2;
	double k0 = 2.0*std::numbers::pi/lambda;
	std::complex<double> imaginary(0.0,1.0);
	int offset = 0;

	double suprefindex = 1-RhoArray[0];
	double suprefindexsquared = suprefindex*suprefindex;
    double sigmacalc[20];

	int neg = 0;
    for(int i =0; i<datapoints;i++)
	{

		for(int k = 1; k<nl;k++)
		{
			if((suprefindexsquared*sinsquaredtheta[i]-2.0*RhoArray[k]+2.0*RhoArray[0])<0)
			{
				neg -= 1;
				break;
			}
		}
		if(neg ==0)
		{
			/*if(m_dQSpread < 0.005 && neg == -5)
				break;*/
		}
		else
		{
			offset = i;
		}
	}

	std::complex<double>  lengthmultiplier = -2.0 * std::complex<double> (0.0,1.0) ;
	std::complex<double>  kk[20];
	double dkk[20];
	std::complex<double>  ak[20];
	double drj[20];
	std::complex<double>  rj[20];
	std::complex<double>  Rj[20];
	double dQj[20];
	std::complex<double>  Qj[20];

	//Boundary conditions
	Rj[nl-1] = 0.0;
	ak[0] = 1.0;
	ak[nl-1] = 1.0;

	std::complex<double>  doublenk[20];
	std::complex<double>  lengthcalc[20];


	//Move some calcs out of the loop
	for(int i = 0; i<nl;i++)
	{
		doublenk[i]=-2.0*std::complex<double> (RhoArray[i],0);
		lengthcalc[i] = lengthmultiplier*LengthArray[i];
		sigmacalc[i] = -2.0*SigmaArray[i]*SigmaArray[i];
	}

	for(int l = 0; l <= offset ;l++)
	{
		int nlminone = nl-1;
		kk[0] = k0*suprefindex* sintheta[l];

		//Workout the wavevector k
		for(int i = 1; i<nl;i++)
		{
			kk[i] = k0 *std::sqrt(suprefindexsquared * sinsquaredtheta[l]+ doublenk[i] - doublenk[0]);
		}

		//Make the aj
		for(int i = 1; i<nlminone;i++)
		{
			ak[i] = std::exp(kk[i]*lengthcalc[i]);
		}

		//Make the roughness correction term (Nevot-Croce)
		for(int i = 0; i< nlminone; i++)
		{
			Qj[i] = std::exp(sigmacalc[i]*kk[i]*kk[i+1]);
		}

		//Make the Fresnel coefficients
		for(int i = 0; i <nlminone;i++)
		{
			rj[i] =Qj[i]* (kk[i]-kk[i+1])/(kk[i]+kk[i+1]);
		}


		for(int i = nl-2; i>=0;i--)
		{
			Rj[i] = ak[i]*(Rj[i+1]+rj[i])/(Rj[i+1]*rj[i]+1.0);
		}
		refl[l] = std::abs(Rj[0]);
		refl[l] *= refl[l];
	}
	for(int l = offset+1; l< datapoints ;l++)
	{
		//Leave for vectorization
		int nlminone = nl-1;

		//Boundary conditions
		dkk[0] = k0*suprefindex*sintheta[l];

		//Workout the wavevector k
		for(int i = 1; i<nl;i++)
		{
			dkk[i] = k0 *sqrt(suprefindexsquared * sinsquaredtheta[l]+ doublenk[i].real() - doublenk[0].real());

		}

		//Make the aj
		for(int i = 1; i<nlminone;i++)
		{
			ak[i] = std::exp(lengthcalc[i]*dkk[i]);
		}

		//Make the roughness correction term (Nevot-Croce)
		for(int i = 0; i< nlminone; i++)
		{
			dQj[i] = exp(sigmacalc[i]*dkk[i]*dkk[i+1]);
		}

		//Make the Fresnel coefficients
		for(int i = 0; i < nlminone;i++)
		{
			drj[i] =dQj[i]* (dkk[i]-dkk[i+1])/(dkk[i]+dkk[i+1]);
		}


		for(int i = nl-2; i>=0;i--)
		{
			Rj[i] = ak[i]*(Rj[i+1]+drj[i])/(Rj[i+1]*drj[i]+1.0);
		}

		refl[l] = std::abs(Rj[0]);
		refl[l] *= refl[l];

	}
}

void FastReflcalc::Rhocalculate(double Zoffset,double* ZIncrement, double* LengthArray, double* RhoArray, double* SigmaArray, double* nk, double* nkb, int loopcounter)
{
	int refllayers = boxnumber;
	double SuperphaseSLD = RhoArray[0];
	double deltarho = 0;
	double thick = 0;
	double roughness = 0;
	double dist = 0;
	double SubSLD = RhoArray[boxnumber+1];

	//Create arrays so we don't have to redo this calculation for every data point

	vector<double> distarray(refllayers+1);
	vector<double> rhoarray(refllayers+1);
	vector<double> rougharray(refllayers+1);

	//Calculate the portions of the e-density equation that don't need to be repeated

	for (int i = 0; i <= boxnumber; i++)
    {
          if (i == 0)
        {
            deltarho = RhoArray[1] - SuperphaseSLD;
            thick = 0;
            roughness = SigmaArray[0];
        }
        else if (i == refllayers)
        {
            deltarho = SubSLD - RhoArray[i];
			roughness = SigmaArray[i];
            thick = LengthArray[i];
        }
        else
        {
            deltarho = (RhoArray[i + 1] - RhoArray[i]);
            thick = LengthArray[i];
            roughness = SigmaArray[i];
        }

		dist  += thick;

		distarray[i] = dist;
		rhoarray[i] = deltarho;
		rougharray[i] = roughness;
	}

	double sqrt2 = sqrt(2.0);

	// This allows OpenMP to choose the appropriate number of threads
	// The algorithm should now scale with the number of processors in a system

	omp_set_num_threads(omp_get_num_threads());

	#pragma omp parallel for schedule(guided)
	for(int j = 0; j < loopcounter;j++)
	{
		double summ = RhoArray[0];
		double bsumm = RhoArray[0];

		for (int i = 0; i <= boxnumber; i++)
        {
			summ += (rhoarray[i] / 2.0) * (1.0 + erf((ZIncrement[j] - distarray[i]-Zoffset) / (rougharray[i] * sqrt2)));
		}

		for (int i = 0; i <= boxnumber; i++)
        {
			bsumm += (rhoarray[i] / 2.0) * (1.0 + erf((ZIncrement[j] - distarray[i]-Zoffset) / (1e-22 * sqrt2)));
		}

		nk[j] = summ/SubSLD;

		nkb[j] = bsumm/SubSLD;
	}

}

double FastReflcalc::CalcQc(double dSLD)
{
    return 4 * sqrt(std::numbers::pi * (subphaseSLD - m_dsupsld) * 1e-6);
}

double FastReflcalc::CalcFresnelPoint(double Q, double Qc)
{
    if (Q <= Qc)
        return 1;
    else
    {
        double term1 = sqrt(1 - (Qc / Q)*(Qc/Q));
        return ((1.0 - term1) / (1.0 + term1))*((1.0 - term1) / (1.0 + term1));
    }
}


void FastReflcalc::QsmearRf(double* qspreadrefl, double* refl, int datapoints)
{
	QSmear::Apply(qspreadrefl, refl, datapoints);
}

void FastReflcalc::myrfdispatch()
{
	if(m_dQSpread < 0.005)
	{
		CalcRefl(sinthetai.data(), sinsquaredthetai.data(), m_idatapoints, reflpt.data());
	}
	else
	{
		CalcRefl(qspreadsinthetai.data(), qspreadsinsquaredthetai.data(), m_idatapoints*13, qspreadreflpt.data());
		QsmearRf(qspreadreflpt.data(), reflpt.data(), m_idatapoints);
	}

	if(m_bImpNorm)
	{
		ImpNorm(reflpt.data(), m_idatapoints);
	}
}

void FastReflcalc::ImpNorm(double* refl, int datapoints)
{
		for(int i = 0; i < datapoints; i++)
		{
			refl[i] = refl[i] * m_dnormfactor;
		}
}

void FastReflcalc::SetOffsets(int LowQOffset, int HighQOffset)
{
	m_icritqoffset = LowQOffset;
	m_ihighqoffset = HighQOffset;
}
