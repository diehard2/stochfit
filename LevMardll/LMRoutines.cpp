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
#include "LevMardll.h"
#include "lm.h"

extern "C" LEVMARDLL_API int dlevmardif(
      void (*func)(double *p, double *hx, int m, int n, void *adata),
      double *p, double *x, int m, int n, int itmax, double *opts,
      double *info, double *work, double *covar, double *adata)
{
	int ret=dlevmar_dif(func, p, x, m, n, itmax, opts, info, NULL, NULL, NULL); 
	return ret;
}

extern "C" LEVMARDLL_API int dlevmarder(
      void (*func)(double *p, double *hx, int m, int n, void *adata),
      void (*jacf)(double *p, double *j, int m, int n, void *adata),
      double *p, double *x, int m, int n, int itmax, double *opts,
      double *info, double *work, double *covar, double *adata)
{
	int ret=dlevmar_der(func, jacf, p, x, m, n, itmax, opts, info, NULL, NULL, NULL); 
	return ret;
}

/* box-constrained minimization */
extern "C" LEVMARDLL_API int dlevmar_bcder(
       void (*func)(double *p, double *hx, int m, int n, void *adata),
       void (*jacf)(double *p, double *j, int m, int n, void *adata),  
       double *p, double *x, int m, int n, double *lb, double *ub,
       int itmax, double *opts, double *info, double *work, double *covar, double *adata)
{
	int ret=dlevmar_bc_der(func, jacf, p, x, m, n, lb, ub, itmax, opts, info, NULL, NULL, NULL); // with analytic jacobian
	return ret;
}

extern "C" LEVMARDLL_API int dlevmar_bcdif(
       void (*func)(double *p, double *hx, int m, int n, void *adata),
       double *p, double *x, int m, int n, double *lb, double *ub,
       int itmax, double *opts, double *info, double *work, double *covar, double *adata)
{
	int ret=dlevmar_bc_dif(func, p, x, m, n, lb, ub, itmax, opts, info, NULL, NULL, NULL); // with analytic jacobian
	return ret;
}

//Need LAPACK for compilation
/* linear equation constrained minimization */

extern "C" LEVMARDLL_API int dlevmar_lecder(
      void (*func)(double *p, double *hx, int m, int n, void *adata),
      void (*jacf)(double *p, double *j, int m, int n, void *adata),
      double *p, double *x, int m, int n, double *A, double *b, int k,
      int itmax, double *opts, double *info, double *work, double *covar, double *adata)
{
//	int ret=dlevmar_lec_der(func, jacf, p, x, m, n, A, b, k, itmax, opts, info, NULL, NULL, NULL); // lin. constraints, analytic jacobian
	return -1;
}

extern "C" LEVMARDLL_API int dlevmar_lecdif(
      void (*func)(double *p, double *hx, int m, int n, void *adata),
      double *p, double *x, int m, int n, double *A, double *b, int k,
      int itmax, double *opts, double *info, double *work, double *covar, double *adata)
{
//	int ret=dlevmar_lec_dif(func, p, x, m, n, A, b, k,itmax, opts, info, NULL, NULL, NULL); // lin. constraints, analytic jacobian
	return -1;
}



/* single precision LM, with & without jacobian */
/* unconstrained minimization */
extern "C" LEVMARDLL_API int slevmarder(
      void (*func)(float *p, float *hx, int m, int n, void *adata),
      void (*jacf)(float *p, float *j, int m, int n, void *adata),
      float *p, float *x, int m, int n, int itmax, float *opts,
      float *info, float *work, float *covar, double *adata)
{
	return 0;
}

extern "C" LEVMARDLL_API int slevmardif(
      void (*func)(float *p, float *hx, int m, int n, void *adata),
      float *p, float *x, int m, int n, int itmax, float *opts,
      float *info, float *work, float *covar, double *adata)
{
	return 0;
}

/* box-constrained minimization */
extern "C" LEVMARDLL_API int slevmar_bcder(
       void (*func)(float *p, float *hx, int m, int n, void *adata),
       void (*jacf)(float *p, float *j, int m, int n, void *adata),  
       float *p, float *x, int m, int n, float *lb, float *ub,
       int itmax, float *opts, float *info, float *work, float *covar, double *adata)
{
	return 0;
}

extern "C" LEVMARDLL_API int slevmar_bcdif(
       void (*func)(float *p, float *hx, int m, int n, void *adata),
       float *p, float *x, int m, int n, float *lb, float *ub,
       int itmax, float *opts, float *info, float *work, float *covar, double *adata)
{
	return 0;
}


/* linear equation constrained minimization */
extern "C" LEVMARDLL_API int slevmar_lecder(
      void (*func)(float *p, float *hx, int m, int n, void *adata),
      void (*jacf)(float *p, float *j, int m, int n, void *adata),
      float *p, float *x, int m, int n, float *A, float *b, int k,
      int itmax, float *opts, float *info, float *work, float *covar, double *adata)
{
	return 0;
}

extern "C" LEVMARDLL_API int slevmar_lecdif(
      void (*func)(float *p, float *hx, int m, int n, void *adata),
      float *p, float *x, int m, int n, float *A, float *b, int k,
      int itmax, float *opts, float *info, float *work, float *covar, double *adata)
{
	return 0;
}

