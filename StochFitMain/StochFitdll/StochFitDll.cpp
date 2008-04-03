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

// Genfitdll.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "StochFitDll.h"
#include "StochFitHarness.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

//The global genfit
StochFit* stochfit;

extern "C" STOCHFIT_API void Init(LPCWSTR Directory, double Q[], double Refl[], double ReflError[], double QError[], int Qpoints,double rholipid,double rhoh2o, double supSLD, int parratlayers, double layerlength,
							 double surfabs, double wavelength, double subabs, double supabs, BOOL UseSurfAbs, double leftoffset, double QErr, 
							 BOOL forcenorm, double forcesig, BOOL debug, BOOL XRonly, double resolution,double totallength, BOOL impnorm, int objectivefunction)
{
	stochfit = new StochFit(Directory, Q, Refl, ReflError, QError, Qpoints, rholipid, rhoh2o, supSLD, parratlayers, layerlength, surfabs, wavelength, subabs, supabs, UseSurfAbs,leftoffset, QErr, forcenorm,
				forcesig, (bool)debug, XRonly, resolution, totallength, impnorm,objectivefunction);
}

extern "C" STOCHFIT_API void GenPriority(int priority)
{
	if(stochfit != NULL)
		stochfit->Priority(priority);
}

extern "C" STOCHFIT_API void Start(int iterations)
{
	if(stochfit != NULL)
		stochfit->Start(iterations);
}

extern "C" STOCHFIT_API void Cancel()
{
	if(stochfit != NULL)
	{
		stochfit->Cancel();
		delete stochfit;
		stochfit = NULL;
	}
}



extern "C" STOCHFIT_API int GetData(double ZRange[],double Rho[],double QRange[], double Refl[] ,double* roughness, double* chisquare, double* goodnessoffit, BOOL* isfinished)
{
	int iter = 0;
	if(stochfit != NULL)
	{
		iter = stochfit->GetData(ZRange,Rho,QRange,Refl,roughness, chisquare, goodnessoffit, isfinished);
	}
	return iter;
}

extern "C" STOCHFIT_API  void SetSAParameters(int sigmasearch, int algorithm, double inittemp, int platiter, double slope, double gamma, int STUNfunc, BOOL adaptive, int tempiter, int STUNdeciter, double gammadec)
{
		if(stochfit != NULL)
		{
			stochfit->SA->Initialize_Subsytem(inittemp,platiter,gamma ,slope, adaptive, tempiter,STUNfunc, STUNdeciter,gammadec);
			stochfit->m_sigmasearch = sigmasearch;
			stochfit->m_isearchalgorithm = algorithm;
		}
}

extern "C" STOCHFIT_API void ArraySizes(int* RhoSize, int* Reflsize)
{
		if(stochfit != NULL)
		{
			*RhoSize = stochfit->m_irhocount;
			*Reflsize = stochfit->m_irefldatacount;
		}
}

extern "C" STOCHFIT_API bool WarmedUp()
{
		if(stochfit != NULL)
		{
			return stochfit->m_bwarmedup;
		}
		else
			return false;
}

extern "C" STOCHFIT_API void SAparams(double* lowestenergy, double* temp, int* mode)
{
		if(stochfit != NULL)
		{
			*temp = stochfit->SA->Get_Temp();
			*lowestenergy = stochfit->SA->Get_LowestEnergy();

			if(*temp < 1e-20)
				*mode = -1;
			else
				*mode = 1;
		}
		else
			return ;
}