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
// from a DLL simpler. All files within this DLL are compiled with the LEVMARDLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// LEVMARDLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef LEVMARDLL_EXPORTS
#define LEVMARDLL_API __declspec(dllexport)
#else
#define LEVMARDLL_API __declspec(dllimport)
#endif

extern "C" LEVMARDLL_API double FastReflfit(LPCWSTR directory, int boxes, double SLD, double SupSLD, double wavelength, double parameters[], int paramsize,
			double QRange[],double QError[], int QSize, double Reflectivity[], int reflectivitysize, double Errors[],double covar[], int covarsize, 
			double info[], int infosize, BOOL onesigma,BOOL writefiles, double QSpread, BOOL ImpNorm);

extern "C" LEVMARDLL_API double ConstrainedFastReflfit(LPCWSTR directory, int boxes, double SLD,double SupSLD, double wavelength, double parameters[], int paramsize,
			double QRange[],double QError[], int QSize, double Reflectivity[], int reflectivitysize, double Errors[],double covar[], int covarsize, 
			double info[], int infosize, BOOL onesigma,BOOL writefiles, double UL[], double LL[], double QSpread, BOOL ImpNorm);

extern "C" LEVMARDLL_API double FastReflGenerate(int boxes, double SLD, double SupSLD, double wavelength, double parameters[], int paramsize,
			double QRange[], double QError[], int QSize, double Reflectivity[], int reflectivitysize, double QSpread, BOOL impnorm);

extern "C" LEVMARDLL_API double Rhofit(LPCWSTR directory, int boxes, double SLD, double SupSLD, double parameters[], int paramsize,
			double ZRange[], int ZSize, double ED[], int EDsize, double covariance[],
			int covarsize, double info[], int infosize, BOOL onesigma);

extern "C" LEVMARDLL_API double RhoGenerate(int boxes, double SLD, double SupSLD, double parameters[], int paramsize,
			double ZRange[], int ZSize, double ED[], double BoxED[], int EDsize);

extern "C" LEVMARDLL_API double StochFit(int boxes, double SLD, double SupSLD, double wavelength, double parameters[], int paramsize,
			double QRange[], double QError[], int QSize, double Reflectivity[], int reflectivitysize, double Errors[],double covar[], int covarsize, 
			double info[], int infosize, BOOL onesigma,BOOL writefiles, int iterations,double ParamArray[], int* paramarraysize, double paramperc[], double chisquarearray[], double covararray[],
			double QSpread, BOOL ImpNorm);

extern "C" LEVMARDLL_API double ConstrainedStochFit(int boxes, double SLD,double SupSLD, double wavelength, double parameters[], int paramsize,
			double QRange[],double QError[],  int QSize, double Reflectivity[], int reflectivitysize, double Errors[],double covar[], int covarsize, 
			double info[], int infosize, BOOL onesigma,BOOL writefiles, int iterations,double ParamArray[], int* paramarraysize, double parampercs[], double chisquarearray[], double covararray[],
			double UL[], double LL[], double QSpread, BOOL ImpNorm);

/* Leave these in so the dll can also be used as a general purpose LS minimizer*/

extern "C" LEVMARDLL_API int dlevmarder(
      void (*func)(double *p, double *hx, int m, int n, void *adata),
      void (*jacf)(double *p, double *j, int m, int n, void *adata),
      double *p, double *x, int m, int n, int itmax, double *opts,
      double *info, double *work, double *covar, double *adata);

extern "C" LEVMARDLL_API int dlevmardif(
      void (*func)(double *p, double *hx, int m, int n, void *adata),
      double *p, double *x, int m, int n, int itmax, double *opts,
      double *info, double *work, double *covar, double *adata);

/* box-constrained minimization */
extern "C" LEVMARDLL_API int dlevmar_bcder(
       void (*func)(double *p, double *hx, int m, int n, void *adata),
       void (*jacf)(double *p, double *j, int m, int n, void *adata),  
       double *p, double *x, int m, int n, double *lb, double *ub,
       int itmax, double *opts, double *info, double *work, double *covar,double *adata);

extern "C" LEVMARDLL_API int dlevmar_bcdif(
       void (*func)(double *p, double *hx, int m, int n, void *adata),
       double *p, double *x, int m, int n, double *lb, double *ub,
       int itmax, double *opts, double *info, double *work, double *covar, double *adata);

//Need LAPACK for compilation
/* linear equation constrained minimization */

extern "C" LEVMARDLL_API int dlevmar_lecder(void (*func)(double *p, double *hx, int m, int n, void *adata),
      void (*jacf)(double *p, double *j, int m, int n, void *adata),
      double *p, double *x, int m, int n, double *A, double *b, int k,
      int itmax, double *opts, double *info, double *work, double *covar, double *adata);

extern "C" LEVMARDLL_API int dlevmar_lecdif(
      void (*func)(double *p, double *hx, int m, int n, void *adata),
      double *p, double *x, int m, int n, double *A, double *b, int k,
      int itmax, double *opts, double *info, double *work, double *covar, double *adata);



/* single precision LM, with & without jacobian */
/* unconstrained minimization */
extern "C" LEVMARDLL_API int slevmarder(
      void (*func)(float *p, float *hx, int m, int n, void *adata),
      void (*jacf)(float *p, float *j, int m, int n, void *adata),
      float *p, float *x, int m, int n, int itmax, float *opts,
      float *info, float *work, float *covar, double *adata);

extern "C" LEVMARDLL_API int slevmardif(
      void (*func)(float *p, float *hx, int m, int n, void *adata),
      float *p, float *x, int m, int n, int itmax, float *opts,
      float *info, float *work, float *covar, double *adata);

/* box-constrained minimization */
extern "C" LEVMARDLL_API int slevmar_bcder(
       void (*func)(float *p, float *hx, int m, int n, void *adata),
       void (*jacf)(float *p, float *j, int m, int n, void *adata),  
       float *p, float *x, int m, int n, float *lb, float *ub,
       int itmax, float *opts, float *info, float *work, float *covar, double *adata);

extern "C" LEVMARDLL_API int slevmar_bcdif(
       void (*func)(float *p, float *hx, int m, int n, void *adata),
       float *p, float *x, int m, int n, float *lb, float *ub,
       int itmax, float *opts, float *info, float *work, float *covar, double *adata);


/* linear equation constrained minimization */
extern "C" LEVMARDLL_API int slevmar_lecder(
      void (*func)(float *p, float *hx, int m, int n, void *adata),
      void (*jacf)(float *p, float *j, int m, int n, void *adata),
      float *p, float *x, int m, int n, float *A, float *b, int k,
      int itmax, float *opts, float *info, float *work, float *covar, double *adata);

extern "C" LEVMARDLL_API int slevmar_lecdif(
      void (*func)(float *p, float *hx, int m, int n, void *adata),
      float *p, float *x, int m, int n, float *A, float *b, int k,
      int itmax, float *opts, float *info, float *work, float *covar, double *adata);
