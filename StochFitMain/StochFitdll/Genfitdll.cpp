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
#include "Genfitdll.h"
#include "GenfitHarness.h"

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
Genfit* genfit;

extern "C" GENFIT_API int Init(LPCWSTR Directory, double Q[], double Refl[], double ReflError[], double QError[], int Qpoints,double rholipid,double rhoh2o, double supSLD, int parratlayers, double layerlength,
							 double surfabs, double wavelength, double subabs, double supabs, BOOL UseSurfAbs, double leftoffset, double QErr, 
							 BOOL forcenorm, double forcesig, BOOL debug, BOOL XRonly, double resolution,double totallength, BOOL impnorm, int objectivefunction)
{
	genfit = new Genfit(Directory, Q, Refl, ReflError, QError, Qpoints, rholipid, rhoh2o, supSLD, parratlayers, layerlength, surfabs, wavelength, subabs, supabs, UseSurfAbs,leftoffset, QErr, forcenorm,
				forcesig, (bool)debug, XRonly, resolution, totallength, impnorm,objectivefunction);

	
	return 0;
}

extern "C" GENFIT_API int GenPriority(int priority)
{
	if(genfit != NULL)
		genfit->Priority(priority);
	return 0;
}
extern "C" GENFIT_API int Start(int iterations)
{
	if(genfit != NULL)
		genfit->Start(iterations);
	return 0;
}

extern "C" GENFIT_API int Cancel()
{
	if(genfit != NULL)
	{
		genfit->Cancel();
		delete genfit;
		genfit = NULL;
	}
	return 0;
}



extern "C" GENFIT_API int GetData(double ZRange[],double Rho[],double QRange[], double Refl[] ,double* roughness, double* chisquare, double* goodnessoffit, BOOL* isfinished)
{
	int iter = 0;
	if(genfit != NULL)
	{
		iter = genfit->GetData(ZRange,Rho,QRange,Refl,roughness, chisquare, goodnessoffit, isfinished);
	}
	return iter;
}

extern "C" GENFIT_API  void SetSAParameters(int sigmasearch, int algorithm, double inittemp, int platiter, double slope, double gamma, int STUNfunc, BOOL adaptive, int tempiter, int STUNdeciter, double gammadec)
{
		if(genfit != NULL)
		{
			genfit->SA->Initialize_Subsytem(inittemp,platiter,gamma ,slope, adaptive, tempiter,STUNfunc, STUNdeciter,gammadec);
			genfit->m_sigmasearch = sigmasearch;
			genfit->m_isearchalgorithm = algorithm;
		}
}

extern "C" GENFIT_API void ArraySizes(int* RhoSize, int* Reflsize)
{
		if(genfit != NULL)
		{
			*RhoSize = genfit->m_irhocount;
			*Reflsize = genfit->m_irefldatacount;
		}
}

extern "C" GENFIT_API bool WarmedUp()
{
		if(genfit != NULL)
		{
			return genfit->m_bwarmedup;
		}
		else
			return false;
}

extern "C" GENFIT_API void SAparams(double* lowestenergy, double* temp, int* mode)
{
		if(genfit != NULL)
		{
			*temp = genfit->SA->Get_Temp();
			*lowestenergy = genfit->SA->Get_LowestEnergy();

			if(*temp < 1e-20)
				*mode = -1;
			else
				*mode = 1;
		}
		else
			return ;
}