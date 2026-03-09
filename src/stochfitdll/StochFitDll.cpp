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
#include "StochFitDll.h"
#include "StochFitHarness.h"

#if STOCHFIT_HAS_GPU
#include "gpu/gpu_detect.h"
#endif

//The global stochfit class pointer
StochFit* stochfit = NULL;

extern "C" EXPORT void Init(ReflSettings* initstruct)
{
	if(stochfit == NULL)
		stochfit = new StochFit(initstruct);
	else
	{
		delete stochfit;
		stochfit = new StochFit(initstruct);
	}
}

extern "C" EXPORT void GenPriority(int priority)
{
	if(stochfit != NULL)
		stochfit->Priority(priority);
}

extern "C" EXPORT void Start(int iterations)
{
	if(stochfit != NULL)
		stochfit->Start(iterations);
}

extern "C" EXPORT void Cancel()
{
	if(stochfit != NULL)
	{
		stochfit->Cancel();
		delete stochfit;
		stochfit = NULL;
	}
}



extern "C" EXPORT int GetData(double ZRange[],double Rho[],double QRange[], double Refl[] ,double* roughness, double* chisquare, double* goodnessoffit, BOOL* isfinished)
{
	int iter = -1;
	if(stochfit != NULL)
		iter = stochfit->GetData(ZRange,Rho,QRange,Refl,roughness, chisquare, goodnessoffit, isfinished);

	return iter;
}

extern "C" EXPORT void ArraySizes(int* RhoSize, int* Reflsize)
{
		if(stochfit != NULL)
		{
			stochfit->GetArraySizes(RhoSize, Reflsize);
		}
}

extern "C" EXPORT bool WarmedUp()
{
		if(stochfit != NULL)
		{
			return stochfit->GetWarmedUp();
		}
		else
			return false;
}

extern "C" EXPORT void SAparams(double* lowestenergy, double* temp, int* mode)
{
		if(stochfit != NULL)
		{
			*temp = stochfit->m_SA->Get_Temp();
			*lowestenergy = stochfit->m_SA->Get_LowestEnergy();

			if(*temp < 1e-20)
				*mode = -1;
			else
				*mode = 1;
		}
		else
			return ;
}

extern "C" EXPORT bool GpuAvailable()
{
#if STOCHFIT_HAS_GPU
	return is_gpu_available();
#else
	return false;
#endif
}
