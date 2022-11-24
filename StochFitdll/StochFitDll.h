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

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the STOCHFIT_API
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// STOCHFIT_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef STOCHFIT_EXPORTS
#define STOCHFIT_API __declspec(dllexport)
#else
#define STOCHFIT_API __declspec(dllimport)
#endif


extern "C" STOCHFIT_API void Init(ReflSettings* initstruct);
extern "C" STOCHFIT_API void GenPriority(int priority);
extern "C" STOCHFIT_API void Start(int iterations);
extern "C" STOCHFIT_API void Cancel();
extern "C" STOCHFIT_API int GetData(double ZRange[],double Rho[],double QRange[], double Refl[] ,double* roughness, double* chisquare, double* goodnessoffit, BOOL* isfinished);
extern "C" STOCHFIT_API void ArraySizes(int* RhoSize, int* Reflsize);
extern "C" STOCHFIT_API bool WarmedUp();
extern "C" STOCHFIT_API void SAparams(double* lowestenergy, double* temp, int* mode);