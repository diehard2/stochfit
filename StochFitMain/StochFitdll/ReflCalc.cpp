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
		_mm_free(xi);
		_mm_free(reflpt);
		_mm_free(dataout);
		_mm_free(sinsquaredthetai);
		_mm_free(m_ckk);
		_mm_free(m_dkk);
		_mm_free(m_cak);
		_mm_free(m_crj);
		_mm_free(m_drj);
		_mm_free(m_cRj);
		_mm_free(qspreadsinsquaredthetai);
		_mm_free(qspreadreflpt);

		if(exi != NULL)
			_mm_free(exi);

}

CReflCalc::CReflCalc():exi(NULL),xi(NULL), m_bReflInitialized(FALSE)
{}

const double CReflCalc::smear[] = {1.2, 1.0, 0.8, 0.6, 0.4, 0.2};	
const double CReflCalc::smearweight[] = {0.056, 0.135, 0.278, 0.487, 0.726, 0.923};
	

void CReflCalc::Init(ReflSettings* InitStruct)
{
	lambda = InitStruct->Wavelength;
	k0 = 2.0*M_PI/lambda;
    
	m_bforcenorm = InitStruct->Forcenorm;
	m_dnormfactor = 1.0;
	m_dQSpread = InitStruct->QErr/100;
	m_bImpNorm = InitStruct->Impnorm;
	
	//Setup OpenMP - Don't restrict the number of processors, let OpenMP handle it
	//For Debugging Purposes force single processor usage
    #ifdef SINGLEPROCDEBUG
			m_iuseableprocessors = 1;
	#else 
	m_iuseableprocessors = omp_get_num_procs();
	#endif

	omp_set_num_threads(m_iuseableprocessors);
	SetupRef(InitStruct);
}

void CReflCalc::SetupRef(ReflSettings* InitStruct)
{
	//Now create our xi,yi,dyi, and thetai
	m_idatapoints = InitStruct->QPoints - InitStruct->HighQOffset - InitStruct->CritEdgeOffset;
	xi = (double*)_mm_malloc(m_idatapoints*sizeof(double),16);
	
	if(InitStruct->QError != NULL)
		exi = (double*)_mm_malloc(m_idatapoints*sizeof(double),16);
	
	sinsquaredthetai = (double*)_mm_malloc(m_idatapoints*sizeof(double),16);
	qspreadsinsquaredthetai = (double*)_mm_malloc(m_idatapoints*13*sizeof(double),16);
	reflpt = (double*)_mm_malloc(m_idatapoints*sizeof(double),16);
	qspreadreflpt = (double*)_mm_malloc(m_idatapoints*13*sizeof(double),16);

	//and fill them up
	for(int i = 0; i< m_idatapoints; i++)
	{
		xi[i] = InitStruct->Q[i+InitStruct->CritEdgeOffset];
			
		if(InitStruct->QError != NULL)
			exi[i] = InitStruct->QError[i+InitStruct->CritEdgeOffset];
		
		sinsquaredthetai[i] = pow(InitStruct->Q[i+InitStruct->CritEdgeOffset]*lambda/(4*M_PI), 2.0);
	}

	if(InitStruct->QError != NULL)
	{
		

	}
	else
	{


	}
	//Calculate the qspread sinthetai's for resolution smearing - the first case deals with user supplied errors, the second
	//handles a constant error in q
	///*if(InitStruct->QError != NULL)
	//{
	//	double holder = lambda/(4.0*M_PI);
	//	float qholder;
	//	float qerrorholder;
	//	for(int i = 0; i < m_idatapoints; i++)
	//	{
	//		qholder = InitStruct->Q[i];
	//		qerrorholder = InitStruct->QError[i];

	//		qspreadsinthetai[13*i] = sinthetai[i];
	//		qspreadsinthetai[13*i+1] = holder*(qholder+1.2*qerrorholder);
	//		qspreadsintheta		i[13*i+2] = holder*(qholder-1.2*qerrorholder);
	//		qspreadsinthetai[13*i+3] = holder*(qholder+1.0*qerrorholder);
	//		qspreadsinthetai[13*i+4] = holder*(qholder-1.0*qerrorholder);
	//		qspreadsinthetai[13*i+5] = holder*(qholder+0.8*qerrorholder);
	//		qspreadsinthetai[13*i+6] = holder*(qholder-0.8*qerrorholder);
	//		qspreadsinthetai[13*i+7] = holder*(qholder+0.6*qerrorholder);
	//		qspreadsinthetai[13*i+8] = holder*(qholder-0.6*qerrorholder);
	//		qspreadsinthetai[13*i+9] = holder*(qholder+0.4*qerrorholder);
	//		qspreadsinthetai[13*i+10] = holder*(qholder-0.4*qerrorholder);
	//		qspreadsinthetai[13*i+11] = holder*(qholder+0.2*qerrorholder);
	//		qspreadsinthetai[13*i+12] = holder*(qholder-0.2*qerrorholder);

	//		if(qspreadsinthetai[13*i+1] < 0.0)
	//			MessageBox(NULL, L"Error in QSpread please contact the author - the program will now crash :(", NULL,NULL);
	//	}
	//}
	//else
	//{
	//	for(int i = 0; i < m_idatapoints; i++)
	//	{
	//		qspreadsinthetai[13*i] = sinthetai[i];
	//		qspreadsinthetai[13*i+1] = sinthetai[i]*(1+1.2*m_dQSpread);
	//		qspreadsinthetai[13*i+2] = sinthetai[i]*(1-1.2*m_dQSpread);
	//		qspreadsinthetai[13*i+3] = sinthetai[i]*(1+1.0*m_dQSpread);	
	//		qspreadsinthetai[13*i+4] = sinthetai[i]*(1-1.0*m_dQSpread);
	//		qspreadsinthetai[13*i+5] = sinthetai[i]*(1+0.8*m_dQSpread);
	//		qspreadsinthetai[13*i+6] = sinthetai[i]*(1-0.8*m_dQSpread);
	//		qspreadsinthetai[13*i+7] = sinthetai[i]*(1+0.6*m_dQSpread);
	//		qspreadsinthetai[13*i+8] = sinthetai[i]*(1-0.6*m_dQSpread);
	//		qspreadsinthetai[13*i+9] = sinthetai[i]*(1+0.4*m_dQSpread);
	//		qspreadsinthetai[13*i+10] = sinthetai[i]*(1-0.4*m_dQSpread);
	//		qspreadsinthetai[13*i+11] = sinthetai[i]*(1+0.2*m_dQSpread);
	//		qspreadsinthetai[13*i+12] = sinthetai[i]*(1-0.2*m_dQSpread);
	//	}
	//}*/
}

void CReflCalc::MakeReflectivity(CEDP* EDP)
{
	if(m_dQSpread == 0.0 || exi == NULL)
	{
		if(EDP->Get_UseABS() == FALSE)
			MyTransparentRF(sinsquaredthetai, m_idatapoints, reflpt, EDP);
		else
			MyRF(sinsquaredthetai, m_idatapoints, reflpt, EDP);
	}
	else
	{
		if(EDP->Get_UseABS())
			MyTransparentRF(qspreadsinsquaredthetai, 13*m_idatapoints, qspreadreflpt, EDP);
		else
			MyRF(qspreadsinsquaredthetai, 13*m_idatapoints, qspreadreflpt, EDP);

		QsmearRf(qspreadreflpt, reflpt, m_idatapoints);
	}

	//Normalize if we let the absorption vary
	if(m_bforcenorm == TRUE)
	{
		impnorm(reflpt, m_idatapoints, false);
	}
	
	//Fix imperfect normalization
	if(m_bImpNorm == TRUE)
	{
		impnorm(reflpt, m_idatapoints, true);
	}
} 


//Perform a rudimentary normalization on the modeled reflectivity (for absorbing films)
//This is for the output reflectivity. If the normalization is imperfect for neutrons,
//this should use isimprefl = true
void CReflCalc::impnorm(double* refl, int datapoints, bool isimprefl)
{
	float normfactor;

	if(isimprefl == true)
		normfactor = m_dnormfactor;
	else
		normfactor = 1.0f/refl[0];

	for(int i = 0; i< datapoints; i++)
	{
		refl[i] *= normfactor;
	}
}

void CReflCalc::MyRF(double* sinsquaredtheta, int datapoints,  double* refl, CEDP* EDP)
{
	MyComplex* DEDP = EDP->m_DEDP;
	int EDPoints = EDP->Get_EDPPointCount();

	if(m_bReflInitialized == FALSE)
	{
		InitializeScratchArrays(EDP->Get_EDPPointCount());
	}

	//Calculate some complex constants to keep them out of the loop
	MyComplex  lengthmultiplier = -2.0*MyComplex (0.0,1.0)*EDP->Get_Dz() ;
	MyComplex  indexsup = 1.0 - DEDP[0]/2.0;
	MyComplex  indexsupsquared = indexsup * indexsup;
	MyComplex  zero;

	
	int HighOffSet = 0;
	int LowOffset = 0;

	GetOffSets(HighOffSet, LowOffset, DEDP, EDPoints);

	#pragma omp parallel
	{
		//Figure out the number of the thread we're currently in
		int threadnum = omp_get_thread_num();
		int arrayoffset = threadnum*EDPoints;

		double* dkk = m_dkk+arrayoffset;
		MyComplex * kk = m_ckk+arrayoffset;
		MyComplex * ak = m_cak+arrayoffset;
		double* drj = m_drj+arrayoffset;
		MyComplex * rj= m_crj+arrayoffset;
		MyComplex * Rj= m_cRj+arrayoffset;
		MyComplex  cholder, tempk1, tempk2;
		double holder;
		
		/********Boundary conditions********/
		//No reflection in the last layer
		Rj[EDPoints-1] = 0.0f;
		//The first layer and the last layer are assumed to be infinite e^-(0+i*(layerlength)) = 1+e^(-inf*i*beta) = 1 + 0*i*beta
		ak[EDPoints-1] = 1.0f;
		ak[0] = 1.0f;
		
		//In order to vectorize loops, you cannot use global variables
		int nlminone = EDPoints-1;
		int numlay =  EDPoints;

		#pragma omp for schedule(runtime)
		for(int l = 0; l < datapoints;l++)
		{
			//The refractive index for air is 1, so there is no refractive index term for kk[0]
			kk[0] = k0 * indexsup * sqrt(sinsquaredtheta[l]);

			tempk1 = k0 * compsqrt(indexsupsquared*sinsquaredtheta[l]-DEDP[1]+DEDP[0]);
			tempk2 = k0 * compsqrt(indexsupsquared*sinsquaredtheta[l]-DEDP[numlay-1]+DEDP[0]);
			//Workout the wavevector k -> kk[i] = k0 *compsqrt(sinsquaredthetai[l]-2.0*nk[i]);
			#pragma ivdep
			for(int i = 1; i <= LowOffset;i++)
			{
				kk[i] = tempk1;
			}

			#pragma ivdep
			for(int i = LowOffset+1; i < HighOffSet;i++)
			{
				kk[i] = k0 * compsqrt(indexsupsquared*sinsquaredtheta[l]-DEDP[i]+DEDP[0]);
			}

			#pragma ivdep
			for(int i = HighOffSet; i < numlay; i++)
			{
				kk[i] = tempk2;
			}

			//Make the phase factors ak -> ak[i] = compexp(-1.0*imaginary*kk[i]*dz0/2.0);

			tempk1 = compexp(lengthmultiplier*kk[1]);
			tempk2 = compexp(lengthmultiplier*kk[numlay-1]);

			#pragma ivdep
			for(int i = 1; i <= LowOffset;i++)
			{
				ak[i] = tempk1;
			}

			#pragma ivdep
			for(int i = LowOffset+1; i < HighOffSet;i++)
			{
				ak[i] = compexp(lengthmultiplier*kk[i]);
			}

			#pragma ivdep
			for(int i = HighOffSet; i < numlay-1; i++)
			{
				ak[i] = tempk2;
			}

			//Make the Fresnel coefficients -> rj[i] =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);
			
			#pragma ivdep
			for(int i = 0; i <= LowOffset;i++)
			{
				rj[i] = zero;
			}

			#pragma ivdep
			for(int i = LowOffset+1; i < HighOffSet;i++)
			{
				rj[i] =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);
			}
			
			#pragma ivdep
			for(int i = HighOffSet; i < numlay-1; i++)
			{
				rj[i] = zero;
			}
			
			
			//Parratt recursion of the amplitude reflectivity
			for(int i = EDPoints-2; i >= 0 ;i--)
			{
				Rj[i] = ak[i]*(Rj[i+1]+rj[i])/(Rj[i+1]*rj[i]+1.0);
			}
			
			//The magnitude of the reflection at layer 0 is the measured reflectivity of the film
			holder = compabs(Rj[0]);
			refl[l] = holder*holder;
		}
	}
}

void CReflCalc::MyTransparentRF(double* sinsquaredtheta, int datapoints,double* refl, CEDP* EDP)
{
	MyComplex* DEDP = EDP->m_DEDP;
	int EDPoints = EDP->Get_EDPPointCount();

	if(m_bReflInitialized == FALSE)
	{
		InitializeScratchArrays(EDP->Get_EDPPointCount());
	}

	//Calculate some complex constants to keep them out of the loop
	MyComplex  lengthmultiplier = -2.0f*MyComplex (0.0f,1.0f)*EDP->Get_Dz();
	MyComplex  indexsup = 1.0 - DEDP[0]/2.0;
	MyComplex  indexsupsquared = indexsup * indexsup;
	int HighOffSet = EDPoints;
	int LowOffset = 0;
	int offset = datapoints;
	
	GetOffSets(HighOffSet, LowOffset, DEDP, EDPoints);

	//Find the point at which we no longer need to use complex numbers exclusively
    for(int i = 0; i< datapoints;i++)
	{	
		int neg = 0;
		for(int k = 0; k<EDPoints;k++)
		{
			if((indexsupsquared.re*sinsquaredtheta[i]-DEDP[k].re+DEDP[0].re)< 0.0)
			{
				neg -= 1;
				break;
			}
		}
		if(neg == 0)
		{
			offset = i;
			if(m_dQSpread == 0)
				break;
		}
	
	}

	//In order to vectorize loops, you cannot use global variables
	int EDPointsMinOne = EDPoints-1;

	#pragma omp parallel
	{
		//Figure out the number of the thread we're currently in
		int threadnum = omp_get_thread_num();
		int arrayoffset = threadnum*EDPoints;

		double* dkk = m_dkk+arrayoffset;
		MyComplex * kk = m_ckk+arrayoffset;
		MyComplex * ak = m_cak+arrayoffset;
		double* drj = m_drj+arrayoffset;
		MyComplex * rj= m_crj+arrayoffset;
		MyComplex * Rj= m_cRj+arrayoffset;
		MyComplex  cholder, tempk1, tempk2, zero;
		double holder, dtempk1, dtempk2;

		/********Boundary conditions********/
		//No reflection in the last layer
		Rj[EDPointsMinOne] = 0.0f;
		//The first layer and the last layer are assumed to be infinite
		ak[EDPointsMinOne] = 1.0f;
		ak[0] = 1.0f;
		

		#pragma omp for nowait schedule(guided)
		for(int l = 0; l< offset;l++)
		{

			//The refractive index for air is 1, so there is no refractive index term for kk[0]
			kk[0] = k0 * indexsup * sqrt(sinsquaredtheta[l]);

			tempk1 = k0 * compsqrt(indexsupsquared*sinsquaredtheta[l]-DEDP[1]+DEDP[0]);
			tempk2 = k0 * compsqrt(indexsupsquared*sinsquaredtheta[l]-DEDP[EDPointsMinOne]+DEDP[0]);
			//Workout the wavevector k -> kk[i] = k0 *compsqrt(sinsquaredthetai[l]-2.0*nk[i]);
			#pragma ivdep
			for(int i = 1; i <= LowOffset;i++)
			{
				kk[i] = tempk1;
			}

			#pragma ivdep
			for(int i = LowOffset+1; i < HighOffSet;i++)
			{
				kk[i] = k0 * compsqrt(indexsupsquared*sinsquaredtheta[l]-DEDP[i]+DEDP[0]);
			}

			#pragma ivdep
			for(int i = HighOffSet; i < EDPoints; i++)
			{
				kk[i] = tempk2;
			}

			//Make the phase factors ak -> ak[i] = compexp(-1.0*imaginary*kk[i]*dz0/2.0);

			tempk1 = compexp(lengthmultiplier*kk[1]);
			tempk2 = compexp(lengthmultiplier*kk[EDPointsMinOne]);

			#pragma ivdep
			for(int i = 1; i <= LowOffset;i++)
			{
				ak[i] = tempk1;
			}

			#pragma ivdep
			for(int i = LowOffset+1; i < HighOffSet;i++)
			{
				ak[i] = compexp(lengthmultiplier*kk[i]);
			}

			#pragma ivdep
			for(int i = HighOffSet; i < EDPointsMinOne; i++)
			{
				ak[i] = tempk2;
			}

			//Make the Fresnel coefficients -> rj[i] =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);
			
			#pragma ivdep
			for(int i = 0; i < LowOffset;i++)
			{
				rj[i] = zero;
			}

			#pragma ivdep
			for(int i = LowOffset+1; i < HighOffSet;i++)
			{
				rj[i] =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);
			}
			
			#pragma ivdep
			for(int i = HighOffSet; i < EDPointsMinOne; i++)
			{
				rj[i] = zero;
			}
			
			
			//Parratt recursion of the amplitude reflectivity
			for(int i = EDPoints-2; i >= 0 ;i--)
			{
				Rj[i] = ak[i]*(Rj[i+1]+rj[i])/(Rj[i+1]*rj[i]+1.0);
			}
			
			//The magnitude of the reflection at layer 0 is the measured reflectivity of the film
			holder = compabs(Rj[0]);
			refl[l] = holder*holder;
		}

		//Now calculate the rest using doubles
		#pragma omp for schedule(guided)
		for(int l = offset; l < datapoints;l++)
		{
				//The refractive index for air is 1, so there is no refractive index term for kk[0]
			dkk[0] = k0 * indexsup.re * sqrt(sinsquaredtheta[l]);

			dtempk1 = k0 * sqrtf(indexsupsquared.re*sinsquaredtheta[l]-DEDP[1].re+DEDP[0].re);
			dtempk2 = k0 * sqrtf(indexsupsquared.re*sinsquaredtheta[l]-DEDP[EDPointsMinOne].re+DEDP[0].re);
			//Workout the wavevector k -> kk[i] = k0 *compsqrt(sinsquaredthetai[l]-2.0*nk[i]);
			#pragma ivdep
			for(int i = 1; i <= LowOffset;i++)
			{
				dkk[i] = dtempk1;
			}

			#pragma ivdep
			for(int i = LowOffset+1; i < HighOffSet;i++)
			{
				dkk[i] = k0 * sqrtf(indexsupsquared.re*sinsquaredtheta[l]-DEDP[i].re+DEDP[0].re);
			}

			#pragma ivdep
			for(int i = HighOffSet; i < EDPoints; i++)
			{
				dkk[i] = dtempk2;
			}

			//Make the phase factors ak -> ak[i] = compexp(-1.0*imaginary*kk[i]*dz0/2.0);

			tempk1 = compexp(lengthmultiplier*dkk[1]);
			tempk2 = compexp(lengthmultiplier*dkk[EDPointsMinOne]);

			#pragma ivdep
			for(int i = 1; i <= LowOffset;i++)
			{
				ak[i] = tempk1;
			}

			#pragma ivdep
			for(int i = LowOffset+1; i < HighOffSet;i++)
			{
				ak[i] = compexp(lengthmultiplier*dkk[i]);
			}

			#pragma ivdep
			for(int i = HighOffSet; i < EDPointsMinOne; i++)
			{
				ak[i] = tempk2;
			}

			//Make the Fresnel coefficients -> rj[i] =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);
			
			#pragma ivdep
			for(int i = 0; i <= LowOffset;i++)
			{
				drj[i] = 0.0f;
			}

			#pragma ivdep
			for(int i = LowOffset+1; i < HighOffSet;i++)
			{
				drj[i] =(dkk[i]-dkk[i+1])/(dkk[i]+dkk[i+1]);
			}
			
			#pragma ivdep
			for(int i = HighOffSet; i < EDPointsMinOne; i++)
			{
				drj[i] = 0.0f;
			}
			
			
			//Parratt recursion of the amplitude reflectivity
			for(int i = EDPoints-2; i >= 0 ;i--)
			{
				Rj[i] = ak[i]*(Rj[i+1]+drj[i])/(Rj[i+1]*drj[i]+1.0);
			}
			
			
			//The magnitude of the reflection at layer 0 is the measured reflectivity of the film
			holder = compabs(Rj[0]);
			refl[l] = holder*holder;
		}
	}
}

void CReflCalc::InitializeScratchArrays(int EDPoints)
{
	//Create the scratch arrays for the reflectivity calculation
 	m_ckk = (MyComplex *)_mm_malloc(sizeof(MyComplex )*EDPoints*m_iuseableprocessors,16);
	m_dkk = (double*)_mm_malloc(sizeof(double)*EDPoints*m_iuseableprocessors,16);
	m_cak = (MyComplex *)_mm_malloc(sizeof(MyComplex )*EDPoints*m_iuseableprocessors,16);
	m_crj = (MyComplex *)_mm_malloc(sizeof(MyComplex )*EDPoints*m_iuseableprocessors,16);
	m_drj = (double*)_mm_malloc(sizeof(double)*EDPoints*m_iuseableprocessors,16);
	m_cRj = (MyComplex *)_mm_malloc(sizeof(MyComplex )*EDPoints*m_iuseableprocessors,16);

	m_bReflInitialized = TRUE;
}

void CReflCalc::QsmearRf(double* qspreadreflpt, double* refl, int datapoints)
{
	float calcholder;
	#pragma ivdep
	for(int i = 0; i < datapoints; i++)
	{
		calcholder = 0.0f;
		calcholder = qspreadreflpt[13*i];
		calcholder += 0.056f*qspreadreflpt[13*i+1];
		calcholder += 0.056f*qspreadreflpt[13*i+2];
		calcholder += 0.135f*qspreadreflpt[13*i+3];
		calcholder += 0.135f*qspreadreflpt[13*i+4];
		calcholder += 0.278f*qspreadreflpt[13*i+5];
		calcholder += 0.278f*qspreadreflpt[13*i+6];
		calcholder += 0.487f*qspreadreflpt[13*i+7];
		calcholder += 0.487f*qspreadreflpt[13*i+8];
		calcholder += 0.726f*qspreadreflpt[13*i+9];
		calcholder += 0.726f*qspreadreflpt[13*i+10];
		calcholder += 0.923f*qspreadreflpt[13*i+11];
		calcholder += 0.923f*qspreadreflpt[13*i+12];

		refl[i] = calcholder/6.211f;
	}
}


int CReflCalc::GetDataCount()
{
		return m_idatapoints;
}

double CReflCalc::GetWaveConstant()
{
	return m_dwaveconstant;
}

void CReflCalc::GetOffSets(int& HighOffset, int& LowOffset, MyComplex* EDP, int EDPoints)
{
		//Find duplicate pts so we don't do the same calculation over and over again
		for(int i = 0; i < EDPoints; i++)
		{
			if(EDP[i].re == EDP[0].re)
				LowOffset++;
			else
				break;
		}

		for(int i = EDPoints - 1 ; i != 0; i--)
		{
			if(EDP[i].re == EDP[EDPoints-1].re)
				HighOffset = i;
			else
				break;
		}
}

void CReflCalc::WriteOutputFile(string filename)
{
	ofstream reflout(filename.c_str());

	for(int i = 0;i< m_idatapoints;i++)
	{
		reflout << xi[i] << " " << reflpt[i] << endl;
	}

	reflout.close();

}

void CReflCalc::GetData(double* Q, double* Refl)
{
	memcpy(Refl,reflpt, sizeof(double)*m_idatapoints);
	memcpy(Q, xi, sizeof(double)*m_idatapoints);
} 