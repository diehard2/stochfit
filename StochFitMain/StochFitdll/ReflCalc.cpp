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
#include "ParamVector.h"
#include "ReflCalc.h"

// Multilayer reflection and transmission
// The calculation scheme used can be found in 
// L. G. Parratt, Phys. Rev. 95(2), 359(1954)

CReflCalc::~CReflCalc()
{
		_mm_free(m_dQ);
		_mm_free(m_dRefl);
		_mm_free(m_dSinSquaredTheta);
		_mm_free(m_cWaveVec);
		_mm_free(m_dWaveVec);
		_mm_free(m_cPhaseFactor);
		_mm_free(m_cFresnelCoefs);
		_mm_free(m_dFresnelCoefs);
		_mm_free(m_cReflectivity);
		_mm_free(m_dQSpreadSinSquaredTheta);
		_mm_free(m_dQSpreadRefl);
}

CReflCalc::CReflCalc():m_dQ(NULL), m_dRefl(NULL), m_dSinSquaredTheta(NULL), m_cWaveVec(NULL), m_dWaveVec(NULL), m_cPhaseFactor(NULL),
	m_cFresnelCoefs(NULL), m_dFresnelCoefs(NULL), m_cReflectivity(NULL), m_dQSpreadSinSquaredTheta(NULL), m_dQSpreadRefl(NULL)
{}

const double CReflCalc::m_Smear[] = {1.2, 1.0, 0.8, 0.6, 0.4, 0.2};	
const double CReflCalc::m_SmearWeight[] = {0.056, 0.135, 0.278, 0.487, 0.726, 0.923};
	
void CReflCalc::Initialize(const ReflSettings* InitStruct)
{
	m_dWaveVecConst = 2.0 * PI/InitStruct->Wavelength;
    
	m_bForceNorm = InitStruct->Forcenorm;
	m_dNormFactor = 1.0;
	m_bImpNorm = InitStruct->Impnorm;
	m_bHasQError = InitStruct->QErr > 0.0 || InitStruct->QError != NULL;


	//Setup OpenMP - Don't restrict the number of processors, let OpenMP handle it
	//For Debugging Purposes force single processor usage
    #ifdef SINGLEPROCDEBUG
			m_iuseableprocessors = 1;
	#else 
			m_iUseableProcessors = omp_get_num_procs();
	#endif

	omp_set_num_threads(m_iUseableProcessors);

	SetupRefl(InitStruct);
}

void CReflCalc::SetupRefl(const ReflSettings* InitStruct)
{
	//Now create our xi,yi,dyi, and thetai
	m_iDataPoints = InitStruct->QPoints - InitStruct->HighQOffset - InitStruct->CritEdgeOffset;
	m_dQ = (double*)_mm_malloc(m_iDataPoints*sizeof(double),16);
	
	m_dSinSquaredTheta = (double*)_mm_malloc(m_iDataPoints*sizeof(double),16);
	m_dQSpreadSinSquaredTheta = (double*)_mm_malloc(m_iDataPoints*13*sizeof(double),16);
	m_dRefl = (double*)_mm_malloc(m_iDataPoints*sizeof(double),16);
	m_dQSpreadRefl = (double*)_mm_malloc(m_iDataPoints*13*sizeof(double),16);

	//and fill them up
	for(int i = 0; i< m_iDataPoints; i++)
	{
		m_dQ[i] = InitStruct->Q[i+InitStruct->CritEdgeOffset];
			
		m_dSinSquaredTheta[i] = pow(InitStruct->Q[i+InitStruct->CritEdgeOffset]*InitStruct->Wavelength/(4.0 * PI), 2.0);
	}

	//Calculate the qspread sinthetai's for resolution smearing - the first case deals with user supplied errors, the second
	//handles a constant error in q
	for(int i = 0; i < m_iDataPoints; i++)
	{
		double lambdaconstant = InitStruct->Wavelength/(4.0 * PI);
		double Q = InitStruct->Q[i + InitStruct->CritEdgeOffset];
		double qerrorholder = InitStruct->QError != NULL ? InitStruct->QError[i + InitStruct->CritEdgeOffset] : (InitStruct->QErr/100.0)*Q;

		m_dQSpreadSinSquaredTheta[13*i] = pow(lambdaconstant*Q, 2.0);
		
		for(int j = 1; j < 13; j += 2)
		{
			m_dQSpreadSinSquaredTheta[13*i+j] = pow(lambdaconstant*(Q + m_Smear[(j-1)/2] * qerrorholder), 2.0);
			m_dQSpreadSinSquaredTheta[13*i+j+1] = pow(lambdaconstant*(Q - m_Smear[(j-1)/2] * qerrorholder), 2.0);
		}
	}
}

void CReflCalc::ForceReflectivityCalc(const CEDP* EDP, CalculationEnum Calc)
{
	if(!m_bHasQError)
	{
		if(Calc == Full)
		{
			FullRF(m_dSinSquaredTheta, m_iDataPoints, m_dRefl, EDP);
		}
		else if (Calc == Opaque)
		{
			OpaqueRF(m_dSinSquaredTheta, m_iDataPoints, m_dRefl, EDP);
		}
		else
		{
			TransparentRF(m_dSinSquaredTheta, m_iDataPoints, m_dRefl, EDP);
		}
	}
	else
	{
		if(Calc == Full)
		{
			FullRF(m_dQSpreadSinSquaredTheta, 13* m_iDataPoints, m_dQSpreadRefl, EDP);
		}
		else if (Calc == Opaque)
		{
			OpaqueRF(m_dQSpreadSinSquaredTheta, 13* m_iDataPoints, m_dQSpreadRefl, EDP);
		}
		else
		{
			TransparentRF(m_dQSpreadSinSquaredTheta, 13* m_iDataPoints, m_dQSpreadRefl, EDP);
		}
	}
}

void CReflCalc::MakeReflectivity(const CEDP* EDP)
{
	if(!m_bHasQError)
	{
		if(!EDP->Get_UseABS())
			TransparentRF(m_dSinSquaredTheta, m_iDataPoints, m_dRefl, EDP);
		else
			OpaqueRF(m_dSinSquaredTheta, m_iDataPoints, m_dRefl, EDP);
	}
	else
	{
		if(!EDP->Get_UseABS())
			TransparentRF(m_dQSpreadSinSquaredTheta, 13*m_iDataPoints, m_dQSpreadRefl, EDP);
		else
			OpaqueRF(m_dQSpreadSinSquaredTheta, 13*m_iDataPoints, m_dQSpreadRefl, EDP);

		QsmearRf();
	}

	//Normalize if we let the absorption vary
	if(m_bForceNorm)
	{
		ImpNorm(m_dRefl, m_iDataPoints, false);
	}
	
	//Fix imperfect normalization
	if(m_bImpNorm)
	{
		ImpNorm(m_dRefl, m_iDataPoints, true);
	}
} 


//Perform a rudimentary normalization on the modeled reflectivity (for absorbing films)
//This is for the output reflectivity. If the normalization is imperfect for neutrons,
//this should use isimprefl = true
void CReflCalc::ImpNorm(double* Refl, int DataPoints, bool IsImpRefl)
{
	double NormFactor = IsImpRefl ? m_dNormFactor : 1.0/Refl[0];

	for(int i = 0; i< DataPoints; i++)
	{
		Refl[i] *= NormFactor;
	}
}

void CReflCalc::OpaqueRF(const double* sinsquaredtheta, int datapoints,  double* refl, const CEDP* EDP)
{
	MyComplex* DEDP = EDP->GetDoubledEDP();
	
	//In order to vectorize loops, you cannot use member variables
	int EDPoints = EDP->Get_EDPPointCount();
	int EDPointsMinOne = EDPoints - 1;
	int EDPointsMinTwo = EDPoints - 2;

	if(m_cWaveVec == NULL)
	{
		InitializeScratchArrays(EDP->Get_EDPPointCount());
	}

	//Calculate some complex constants to keep them out of the loop
	MyComplex  lengthmultiplier = -2.0 * MyComplex(0.0,1.0) * EDP->Get_LayerThickness() ;
	MyComplex  indexsupsquared = (1.0 - DEDP[0]/2.0) * (1.0 - DEDP[0]/2.0);
	MyComplex  Zero;

	int HighOffSet = 0;
	int LowOffset = 0;

	GetOffSets(HighOffSet, LowOffset, DEDP, EDPoints);

	#pragma omp parallel
	{
		//Figure out the number of the thread we're currently in
		int arrayoffset = omp_get_thread_num()*EDPoints;

		MyComplex *WaveVec = m_cWaveVec+arrayoffset;
		MyComplex *PhaseFactor = m_cPhaseFactor+arrayoffset;
		MyComplex *FresnelCoef = m_cFresnelCoefs+arrayoffset;
		MyComplex *Reflectivity= m_cReflectivity+arrayoffset;
		MyComplex  TempCalc1, TempCalc2;
		
		/********Boundary conditions********/
		//No reflection in the last layer
		Reflectivity[EDPointsMinOne] = 0.0;
		//The first layer and the last layer are assumed to be infinite e^-(0+i*(layerlength)) = 1+e^(-inf*i*beta) = 1 + 0*i*beta
		PhaseFactor[EDPointsMinOne] = 1.0;
		PhaseFactor[0] = 1.0;
		
		
		#pragma omp for schedule(runtime)
		for(int l = 0; l < datapoints;l++)
		{
			TempCalc1 = m_dWaveVecConst * compsqrt(indexsupsquared*sinsquaredtheta[l]);
			TempCalc2 = m_dWaveVecConst * compsqrt(indexsupsquared*sinsquaredtheta[l]-DEDP[EDPointsMinOne]+DEDP[0]);
			
			//Workout the wavevector k -> kk[i] = m_dWaveVecConst *compsqrt(sinsquaredthetai[l]-2.0*nk[i]);
			#pragma ivdep
			for(int i = 0; i < LowOffset;i++)
			{
				WaveVec[i] = TempCalc1;
			}

			#pragma ivdep
			for(int i = LowOffset; i < HighOffSet;i++)
			{
				WaveVec[i] = m_dWaveVecConst * compsqrt(indexsupsquared*sinsquaredtheta[l]-DEDP[i]+DEDP[0]);
			}

			#pragma ivdep
			for(int i = HighOffSet; i < EDPoints; i++)
			{
				WaveVec[i] = TempCalc2;
			}

			//Make the phase factors ak -> ak[i] = compexp(-1.0*imaginary*kk[i]*dz0/2.0);
			TempCalc1 = compexp(lengthmultiplier*WaveVec[1]);
			TempCalc2 = compexp(lengthmultiplier*WaveVec[EDPointsMinOne]);

			#pragma ivdep
			for(int i = 1; i < LowOffset;i++)
			{
				PhaseFactor[i] = TempCalc1;
			}

			#pragma ivdep
			for(int i = LowOffset; i < HighOffSet;i++)
			{
				PhaseFactor[i] = compexp(lengthmultiplier*WaveVec[i]);
			}

			#pragma ivdep
			for(int i = HighOffSet; i < EDPointsMinOne; i++)
			{
				PhaseFactor[i] = TempCalc2;
			}

			//Make the Fresnel coefficients -> rj[i] =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);
			#pragma ivdep
			for(int i = 0; i < LowOffset;i++)
			{
				FresnelCoef[i] = Zero;
			}

			#pragma ivdep
			for(int i = LowOffset; i < HighOffSet;i++)
			{
				FresnelCoef[i] =(WaveVec[i]-WaveVec[i+1])/(WaveVec[i]+WaveVec[i+1]);
			}
			
			#pragma ivdep
			for(int i = HighOffSet; i < EDPointsMinOne; i++)
			{
				FresnelCoef[i] = Zero;
			}

			//Parratt recursion of the amplitude reflectivity
			for(int i = EDPointsMinTwo; i >= 0 ;i--)
			{
				Reflectivity[i] = PhaseFactor[i]*(Reflectivity[i+1]+FresnelCoef[i])/(Reflectivity[i+1]*FresnelCoef[i]+1.0);
			}
			
			//The magnitude of the reflection at layer 0 is the measured reflectivity of the film
			refl[l] = compabs(Reflectivity[0]);
			refl[l] *= refl[l];
		}
	}
}

void CReflCalc::FullRF(const double* sinsquaredtheta, int datapoints,  double* refl, const CEDP* EDP)
{
	MyComplex* DEDP = EDP->GetDoubledEDP();

	//In order to vectorize loops, you cannot use global variables
	int EDPoints = EDP->Get_EDPPointCount();
	int EDPointsMinOne = EDPoints - 1;
	int EDPointsMinTwo = EDPoints - 2;

	if(m_cWaveVec == NULL)
	{
		InitializeScratchArrays(EDPoints);
	}

	//Calculate some complex constants to keep them out of the loop
	MyComplex  lengthmultiplier = -2.0 * MyComplex(0.0,1.0) * EDP->Get_LayerThickness() ;
	MyComplex  indexsupsquared = (1.0 - DEDP[0]/2.0) * (1.0 - DEDP[0]/2.0);

	#pragma omp parallel
	{
		//Figure out the number of the thread we're currently in
		int arrayoffset = omp_get_thread_num()*EDPoints;

		MyComplex *WaveVec = m_cWaveVec+arrayoffset;
		MyComplex *PhaseFactor = m_cPhaseFactor+arrayoffset;
		MyComplex *FresnelCoef = m_cFresnelCoefs+arrayoffset;
		MyComplex *Reflectivity = m_cReflectivity+arrayoffset;

		/********Boundary conditions********/
		//No reflection in the last layer
		Reflectivity[EDPointsMinOne] = 0.0;
		//The first layer and the last layer are assumed to be infinite e^-(0+i*(layerlength)) = 1+e^(-inf*i*beta) = 1 + 0*i*beta
		PhaseFactor[EDPointsMinOne] = 1.0;
		PhaseFactor[0] = 1.0;
		
		
		#pragma omp for schedule(runtime)
		for(int l = 0; l < datapoints;l++)
		{
			#pragma ivdep
			for(int i = 0; i < EDPoints; i++)
			{
				WaveVec[i] = m_dWaveVecConst * compsqrt(indexsupsquared*sinsquaredtheta[l]-DEDP[i]+DEDP[0]);
			}

			//Make the phase factors ak -> ak[i] = compexp(-1.0*imaginary*kk[i]*dz0/2.0);
			#pragma ivdep
			for(int i = 1; i < EDPointsMinOne; i++)
			{
				PhaseFactor[i] = compexp(lengthmultiplier*WaveVec[i]);
			}

			//Make the Fresnel coefficients -> rj[i] =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);
			#pragma ivdep
			for(int i = 0; i < EDPointsMinOne;i++)
			{
				FresnelCoef[i] =(WaveVec[i]-WaveVec[i+1])/(WaveVec[i]+WaveVec[i+1]);
			}
			
			//Parratt recursion of the amplitude reflectivity
			for(int i = EDPointsMinTwo; i >= 0 ;i--)
			{
				Reflectivity[i] = PhaseFactor[i]*(Reflectivity[i+1]+FresnelCoef[i])/(Reflectivity[i+1]*FresnelCoef[i]+1.0);
			}
			
			//The magnitude of the reflection at layer 0 is the measured reflectivity of the film
			refl[l] = compabs(Reflectivity[0]);
			refl[l] *= refl[l];
		}
	}
}

void CReflCalc::TransparentRF(const double* sinsquaredtheta, int datapoints,double* refl,const CEDP* EDP)
{
	MyComplex* DEDP = EDP->GetDoubledEDP();
	
	int EDPoints = EDP->Get_EDPPointCount();
	int EDPointsMinOne = EDPoints - 1;
	int EDPointsMinTwo = EDPoints - 2;

	if(m_cWaveVec == NULL)
	{
		InitializeScratchArrays(EDPoints);
	}

	//Calculate some complex constants to keep them out of the loop
	MyComplex  lengthmultiplier = -2.0f*MyComplex (0.0f,1.0f) * EDP->Get_LayerThickness();
	MyComplex  indexsupsquared = (1.0 - DEDP[0]/2.0) * (1.0 - DEDP[0]/2.0);
	
	int HighOffSet = EDPoints;
	int LowOffset = 0;
	int Offset = datapoints;
	
	GetOffSets(HighOffSet, LowOffset, DEDP, EDPoints);

	//Find the point at which we no longer need to use complex numbers exclusively
    for(int i = 0; i< datapoints;i++)
	{	
		int neg = 0;
		for(int k = 0; k < EDPoints;k++)
		{
			if((indexsupsquared.re*sinsquaredtheta[i]-DEDP[k].re+DEDP[0].re)< 0.0)
			{
				neg -= 1;
				break;
			}
		}
		if(neg == 0)
		{
			Offset = i;
			if(!m_bHasQError)
				break;
		}
	}

	#pragma omp parallel
	{
		//Figure out the number of the thread we're currently in
		int arrayoffset = omp_get_thread_num()*EDPoints;

		double* dWaveVec = m_dWaveVec+arrayoffset;
		MyComplex * WaveVec = m_cWaveVec+arrayoffset;
		MyComplex * PhaseFactor = m_cPhaseFactor+arrayoffset;
		double* dFresnelCoef = m_dFresnelCoefs+arrayoffset;
		MyComplex * FresnelCoef = m_cFresnelCoefs+arrayoffset;
		MyComplex * Reflectivity = m_cReflectivity+arrayoffset;
		MyComplex  TempCalc1, TempCalc2, Zero;
		double dTempCalc1, dTempCalc2;

		/********Boundary conditions********/
		//No reflection in the last layer
		Reflectivity[EDPointsMinOne] = 0.0;
		//The first layer and the last layer are assumed to be infinite
		PhaseFactor[EDPointsMinOne] = 1.0;
		PhaseFactor[0] = 1.0;
		
		#pragma omp for nowait schedule(guided)
		for(int l = 0; l< Offset;l++)
		{
			TempCalc1 = m_dWaveVecConst * compsqrt(indexsupsquared*sinsquaredtheta[l]);
			TempCalc2 = m_dWaveVecConst * compsqrt(indexsupsquared*sinsquaredtheta[l]-DEDP[EDPointsMinOne]+DEDP[0]);
	
			//Workout the wavevector k -> kk[i] = m_dWaveVecConst *compsqrt(sinsquaredthetai[l]-2.0*nk[i]);
			#pragma ivdep
			for(int i = 0; i < LowOffset;i++)
			{
				WaveVec[i] = TempCalc1;
			}

			#pragma ivdep
			for(int i = LowOffset; i < HighOffSet;i++)
			{
				WaveVec[i] = m_dWaveVecConst * compsqrt(indexsupsquared*sinsquaredtheta[l]-DEDP[i]+DEDP[0]);
			}

			#pragma ivdep
			for(int i = HighOffSet; i < EDPoints; i++)
			{
				WaveVec[i] = TempCalc2;
			}

			//Make the phase factors ak -> ak[i] = compexp(-1.0*imaginary*kk[i]*dz0/2.0);
			TempCalc1 = compexp(lengthmultiplier*WaveVec[1]);
			TempCalc2 = compexp(lengthmultiplier*WaveVec[EDPointsMinOne]);

			#pragma ivdep
			for(int i = 1; i < LowOffset;i++)
			{
				PhaseFactor[i] = TempCalc1;
			}

			#pragma ivdep
			for(int i = LowOffset; i < HighOffSet;i++)
			{
				PhaseFactor[i] = compexp(lengthmultiplier*WaveVec[i]);
			}

			#pragma ivdep
			for(int i = HighOffSet; i < EDPointsMinOne; i++)
			{
				PhaseFactor[i] = TempCalc2;
			}

			//Make the Fresnel coefficients -> rj[i] =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);
			#pragma ivdep
			for(int i = 0; i < LowOffset;i++)
			{
				FresnelCoef[i] = Zero;
			}
			
			#pragma ivdep
			for(int i = LowOffset; i < HighOffSet;i++)
			{
				FresnelCoef[i] =(WaveVec[i]-WaveVec[i+1])/(WaveVec[i]+WaveVec[i+1]);
			}
			
			#pragma ivdep
			for(int i = HighOffSet; i < EDPointsMinOne; i++)
			{
				FresnelCoef[i] = Zero;
			}
			
			//Parratt recursion of the amplitude reflectivity
			for(int i = EDPointsMinTwo; i >= 0 ;i--)
			{
				Reflectivity[i] = PhaseFactor[i]*(Reflectivity[i+1]+FresnelCoef[i])/(Reflectivity[i+1]*FresnelCoef[i]+1.0);
			}
			
			//The magnitude of the reflection at layer 0 is the measured reflectivity of the film
			refl[l] = compabs(Reflectivity[0]);
			refl[l] *= refl[l];
		}

		//Now calculate the rest using doubles
		#pragma omp for schedule(guided)
		for(int l = Offset; l < datapoints;l++)
		{
			dTempCalc1 = m_dWaveVecConst * sqrt(indexsupsquared.re*sinsquaredtheta[l]-DEDP[1].re+DEDP[0].re);
			dTempCalc2 = m_dWaveVecConst * sqrt(indexsupsquared.re*sinsquaredtheta[l]-DEDP[EDPointsMinOne].re+DEDP[0].re);
			
			//Workout the wavevector k -> kk[i] = m_dWaveVecConst *compsqrt(sinsquaredthetai[l]-2.0*nk[i]);
			#pragma ivdep
			for(int i = 0; i < LowOffset;i++)
			{
				dWaveVec[i] = dTempCalc1;
			}

			#pragma ivdep
			for(int i = LowOffset; i < HighOffSet;i++)
			{
				dWaveVec[i] = m_dWaveVecConst * sqrt(indexsupsquared.re*sinsquaredtheta[l]-DEDP[i].re+DEDP[0].re);
			}

			#pragma ivdep
			for(int i = HighOffSet; i < EDPoints; i++)
			{
				dWaveVec[i] = dTempCalc2;
			}

			//Make the phase factors ak -> ak[i] = compexp(-1.0*imaginary*kk[i]*dz0/2.0);
			TempCalc1 = compexp(lengthmultiplier*dWaveVec[1]);
			TempCalc2 = compexp(lengthmultiplier*dWaveVec[EDPointsMinOne]);

			#pragma ivdep
			for(int i = 1; i < LowOffset;i++)
			{
				PhaseFactor[i] = TempCalc1;
			}

			#pragma ivdep
			for(int i = LowOffset; i < HighOffSet;i++)
			{
				PhaseFactor[i] = compexp(lengthmultiplier*dWaveVec[i]);
			}

			#pragma ivdep
			for(int i = HighOffSet; i < EDPointsMinOne; i++)
			{
				PhaseFactor[i] = TempCalc2;
			}

			//Make the Fresnel coefficients -> rj[i] =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);
			#pragma ivdep
			for(int i = 0; i < LowOffset;i++)
			{
				dFresnelCoef[i] = 0.0;
			}

			#pragma ivdep
			for(int i = LowOffset; i < HighOffSet;i++)
			{
				dFresnelCoef[i] =(dWaveVec[i]-dWaveVec[i+1])/(dWaveVec[i]+dWaveVec[i+1]);
			}
			
			#pragma ivdep
			for(int i = HighOffSet; i < EDPointsMinOne; i++)
			{
				dFresnelCoef[i] = 0.0;
			}
			
			//Parratt recursion of the amplitude reflectivity
			for(int i = EDPointsMinTwo; i >= 0 ;i--)
			{
				Reflectivity[i] = PhaseFactor[i]*(Reflectivity[i+1]+dFresnelCoef[i])/(Reflectivity[i+1]*dFresnelCoef[i]+1.0);
			}
			
			//The magnitude of the reflection at layer 0 is the measured reflectivity of the film
			refl[l] = compabs(Reflectivity[0]);
			refl[l] *= refl[l];
		}
	}
}

//Create the scratch arrays for the reflectivity calculation
void CReflCalc::InitializeScratchArrays(int EDPoints)
{
 	m_cWaveVec = (MyComplex *)_mm_malloc(sizeof(MyComplex )*EDPoints*m_iUseableProcessors,16);
	m_dWaveVec = (double*)_mm_malloc(sizeof(double)*EDPoints*m_iUseableProcessors,16);
	m_cPhaseFactor = (MyComplex *)_mm_malloc(sizeof(MyComplex )*EDPoints*m_iUseableProcessors,16);
	m_cFresnelCoefs = (MyComplex *)_mm_malloc(sizeof(MyComplex )*EDPoints*m_iUseableProcessors,16);
	m_dFresnelCoefs = (double*)_mm_malloc(sizeof(double)*EDPoints*m_iUseableProcessors,16);
	m_cReflectivity = (MyComplex *)_mm_malloc(sizeof(MyComplex )*EDPoints*m_iUseableProcessors,16);
}

//Calculate the smeared gaussian of the reflectivity
void CReflCalc::QsmearRf()
{
	#pragma ivdep
	for(int i = 0; i < m_iDataPoints; i++)
	{
		double calcholder = m_dQSpreadRefl[13*i];

		for(int j= 1; j < 13; j += 2)
		{
			calcholder += m_SmearWeight[(j-1)/2]*m_dQSpreadRefl[13*i+j];
			calcholder += m_SmearWeight[(j-1)/2]*m_dQSpreadRefl[13*i+j+1];
		}
		
		m_dRefl[i] = calcholder/6.211;
	}
}

const double* CReflCalc::GetReflData()
{
	return m_dRefl;
}

void CReflCalc::GetOffSets(int& HighOffset, int& LowOffset, const MyComplex* EDP, int EDPoints)
{	
		//Find duplicate pts so we don't do the same calculation over and over again
		for(int i = 1; i < EDPoints; i++)
		{
			if(EDP[i].re == EDP[0].re)
				LowOffset = i;
			else
				break;
		}

		for(int i = EDPoints - 2 ; i != 0; i--)
		{
			if(EDP[i].re == EDP[EDPoints-1].re)
				HighOffset = i;
			else
				break;
		}
}

void CReflCalc::WriteOutputFile(wstring filename)
{
	ofstream reflout(filename.c_str());

	for(int i = 0;i< m_iDataPoints;i++)
	{
		reflout << m_dQ[i] << " " << m_dRefl[i] << endl;
	}

	reflout.close();
}

void CReflCalc::GetData(double* Q, double* Refl)
{
	memcpy(Refl, m_dRefl, sizeof(double)*m_iDataPoints);
	memcpy(Q, m_dQ, sizeof(double)*m_iDataPoints);
} 

void CReflCalc::SetNormFactor(double NormFactor)
{
	m_dNormFactor = NormFactor;
}

int CReflCalc::GetDataPoints()
{
	return m_iDataPoints;
}