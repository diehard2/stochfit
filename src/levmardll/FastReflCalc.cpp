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

FastReflcalc::~FastReflcalc() = default;

void FastReflcalc::init(const BoxReflSettings& InitStruct)
{
    lambda        = InitStruct.Wavelength;
	onesigma      = InitStruct.OneSigma;
	Realrefl      = InitStruct.Refl;
	Realreflerrors= InitStruct.ReflError;
	realrefllength= InitStruct.QPoints;
	subphaseSLD   = InitStruct.SubSLD;
	m_dsupsld     = InitStruct.SupSLD;
	boxnumber     = InitStruct.Boxes;
	m_dQSpread    = InitStruct.QSpread / 100.0;
	m_bImpNorm    = InitStruct.ImpNorm;
	m_dnormfactor = 1.0;
	m_icritqoffset= InitStruct.LowQOffset;
	m_ihighqoffset= InitStruct.HighQOffset;
	m_idatapoints = InitStruct.QPoints;

	RhoArray.resize(boxnumber+2);
	SigmaArray.resize(boxnumber+2);
	LengthArray.resize(boxnumber+2);
	ImagArray.resize(boxnumber+2);

	MakeTheta(InitStruct);
}

void FastReflcalc::MakeTheta(const BoxReflSettings& InitStruct)
{
	sinthetai.resize(m_idatapoints);
	sinsquaredthetai.resize(m_idatapoints);
	qspreadsinthetai.resize(m_idatapoints * QSmear::Points);
	qspreadsinsquaredthetai.resize(m_idatapoints * QSmear::Points);
	qspreadreflpt.resize(m_idatapoints * QSmear::Points);
	reflpt.resize(m_idatapoints);

	for (int i = 0; i < m_idatapoints; i++)
		sinthetai[i] = InitStruct.Q[i] * lambda / (4.0 * std::numbers::pi);

	for (int l = 0; l < m_idatapoints; l++)
		sinsquaredthetai[l] = sinthetai[l] * sinthetai[l];

	QSmear::BuildArrays(lambda, m_dQSpread,
	                    sinthetai,
	                    InitStruct.QError,
	                    qspreadsinthetai,
	                    qspreadsinsquaredthetai);
}


void FastReflcalc::objective(double* par, double* x, int m, int n, void* data)
{
    FastReflcalc* reflinst = static_cast<FastReflcalc*>(data);
    std::span<const double> params(par, m);
    std::span<double> residuals(x, n);

    if (reflinst->onesigma)
        reflinst->mkdensityonesigma(params);
    else
        reflinst->mkdensity(params);

    reflinst->myrfdispatch();

    std::fill(residuals.begin(), residuals.end(), 0.0);
    for (int i = reflinst->m_icritqoffset; i < n - reflinst->m_ihighqoffset; i++)
    {
        residuals[i] = log(reflinst->reflpt[i]) - log(reflinst->Realrefl[i]);

        //Sometimes we get NAN. We make that solution unpalatable until I can find a workaround
        if (residuals[i] != residuals[i])
            residuals[i] = 1e6;
    }
}

void FastReflcalc::mkdensityonesigma(std::span<const double> p)
{
	double rhofactor = 1e-6*lambda*lambda/(2.0*std::numbers::pi);

	RhoArray[0] = m_dsupsld*rhofactor;
	LengthArray[0] = 0.0;
	for (int i = 1; i < boxnumber+1; i++)
	{
		LengthArray[i] = p[2*(i-1)+1];
		RhoArray[i] = p[2*(i-1)+2]*subphaseSLD*rhofactor;
		SigmaArray[i-1] = p[0];
	}

	RhoArray[boxnumber+1] = subphaseSLD*rhofactor;
	SigmaArray[boxnumber] = p[0];
	LengthArray[boxnumber+1] = 0;
	m_dnormfactor = p[p.size()-1];
}

void FastReflcalc::mkdensity(std::span<const double> p)
{
    double rhofactor = 1e-6*lambda*lambda/(2.0*std::numbers::pi);

	RhoArray[0] = m_dsupsld*rhofactor;
	LengthArray[0] = 0.0;
	for (int i = 1; i < boxnumber+1; i++)
	{
		LengthArray[i] = p[3*(i-1)+1];
		RhoArray[i] = p[3*(i-1)+2]*subphaseSLD*rhofactor;
		SigmaArray[i-1] = fabs(p[3*(i-1)+3]) < 1e-8 ? 1e-8 : p[3*(i-1)+3];
	}

	RhoArray[boxnumber+1] = subphaseSLD*rhofactor;
	SigmaArray[boxnumber] = p[0];
	LengthArray[boxnumber+1] = 0;
	m_dnormfactor = p[p.size()-1];
}

void FastReflcalc::CalcRefl(std::span<const double> sintheta, std::span<const double> sinsquaredtheta, std::span<double> refl)
{
	//Generate the Parratt reflectivity layers
	int nl = boxnumber+2;
	double k0 = 2.0*std::numbers::pi/lambda;
	int offset = 0;

	double suprefindex = 1-RhoArray[0];
	double suprefindexsquared = suprefindex*suprefindex;
    double sigmacalc[20];

	bool offset_sentinel = false;
    for(int i = 0; i < (int)refl.size(); i++)
	{
		for(int k = 1; k<nl;k++)
		{
			if((suprefindexsquared*sinsquaredtheta[i]-2.0*RhoArray[k]+2.0*RhoArray[0])<0)
			{
				offset_sentinel = true;
				break;
			}
		}

		if(offset_sentinel)
		{
			offset = i;
			break;
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

	for(int l = offset+1; l < (int)refl.size(); l++)
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

void FastReflcalc::QsmearRf(std::span<const double> qspreadrefl, std::span<double> refl)
{
	QSmear::Apply(qspreadrefl, refl);
}

void FastReflcalc::myrfdispatch()
{
	if (m_dQSpread < 0.005)
	{
		CalcRefl(sinthetai, sinsquaredthetai, reflpt);
	}
	else
	{
		CalcRefl(qspreadsinthetai, qspreadsinsquaredthetai, qspreadreflpt);
		QsmearRf(qspreadreflpt, reflpt);
	}

	if (m_bImpNorm)
		ImpNorm(reflpt);
}

void FastReflcalc::ImpNorm(std::span<double> refl)
{
	for (auto& r : refl)
		r *= m_dnormfactor;
}

void FastReflcalc::SetOffsets(int LowQOffset, int HighQOffset)
{
	m_icritqoffset = LowQOffset;
	m_ihighqoffset = HighQOffset;
}
