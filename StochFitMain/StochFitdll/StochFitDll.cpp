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

//The global stochfit class pointer
StochFit* stochfit = NULL;

extern "C" STOCHFIT_API void Init(ReflSettings* initstruct)
{
	if(stochfit == NULL)
		stochfit = new StochFit(*initstruct);
	else
	{
		delete stochfit;
		stochfit = new StochFit(*initstruct);
	}
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
	int iter = -1;
	if(stochfit != NULL)
		iter = stochfit->GetData(ZRange,Rho,QRange,Refl,roughness, chisquare, goodnessoffit, isfinished);
	
	return iter;
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