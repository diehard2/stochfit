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
// from a DLL simpler. All files within this DLL are compiled with the GENFIT_API
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GENFIT_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef GENFIT_EXPORTS
#define GENFIT_API __declspec(dllexport)
#else
#define GENFIT_API __declspec(dllimport)
#endif

extern "C" GENFIT_API int Init(LPCWSTR Directory, double Q[], double Refl[], double ReflError[], double QError[], int Qpoints, double rholipid,double rhoh2o,double supSLD, int parratlayers, double layerlength,
							 double surfabs, double wavelength, double subabs, double supabs, BOOL UseSurfAbs, double leftoffset, double QErr, BOOL forcenorm, 
							 double forcesig, BOOL debug, BOOL XRonly, double resolution,double totallength, BOOL impnorm, int objfunction);
extern "C" GENFIT_API int GenPriority(int priority);
extern "C" GENFIT_API int Start(int iterations);
extern "C" GENFIT_API int Cancel();
extern "C" GENFIT_API int GetData(double ZRange[],double Rho[],double QRange[], double Refl[] ,double* roughness, double* chisquare, double* goodnessoffit, BOOL* isfinished);
extern "C" GENFIT_API void SetSAParameters(int sigmasearch,int algorithm, double inittemp, int platiter, double slope, double gamma, int STUNfunc, BOOL adaptive, int tempiter, int deciter, double gammadec);

extern "C" GENFIT_API void ArraySizes(int* RhoSize, int* Reflsize);
extern "C" GENFIT_API bool WarmedUp();
extern "C" GENFIT_API void SAparams(double* lowestenergy, double* temp, int* mode);