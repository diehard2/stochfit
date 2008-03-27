/***********************************************************************
* Adaptive Simulated Annealing (ASA)
* Lester Ingber <ingber@ingber.com>
* Copyright (c) 1993-2008 Lester Ingber.  All Rights Reserved.
* The ASA-LICENSE file must be included with ASA code.
*
* This a modified version of ASA written by Stephen Danauskas
* The ASA code was split into classes,
* and several defines and options unneeded by x86 Windows were removed.
* With those exceptions, it is believed that the results for routines between the 
* official ASA code and this version are identical. The original version is
* located at http://www.ingber.com/
***********************************************************************/

#include "stdafx.h"
#include "asa_base.h"

LocalFitting::LocalFitting(ASA_Base* asaptr)
{
	m_pasa = asaptr;
}
double LocalFitting::calcf (double* ansarray)
{
  double floc;

  
  floc = m_pasa->CalcCostFunc(ansarray);

  if (m_pasa->valid_state_generated_flag == FALSE) {
    floc += m_pasa->Options.Penalty;
  }

  for (int index_v = 0; index_v < m_pasa->m_inumber_parameters; ++index_v) 
  {
	  if (m_pasa->m_dparameter_maximum[index_v] - m_pasa->parameter_initial_final[index_v] < EPS_DOUBLE)
      floc += m_pasa->Options.Penalty;
	else if (m_pasa->parameter_initial_final[index_v] - m_pasa->m_dparameter_minimum[index_v] < EPS_DOUBLE)
		floc += m_pasa->Options.Penalty;
  }

  return (floc);
}

/*
   Written by Mark Johnson <mjohnson@netcom.com>, based on 

   %A J.A. Nelder
   %A R. Mead
   %T A simplex method for function minimization
   %J Computer J. (UK)
   %V 7
   %D 1964
   %P 308-313

   with improvements from

   %A G.P. Barabino
   %A G.S. Barabino
   %A B. Bianco
   %A M. Marchesi
   %T A study on the performances of simplex methods for function minimization
   %B Proc. IEEE Int. Conf. Circuits and Computers
   %D 1980
   %P 1150-1153

   adapted for use in ASA by Lester Ingber <ingber@ingber.com>
 */

int LocalFitting::simplex(double* x, double tol1, double tol2, int no_progress,double alpha, double beta1, double beta2,
						  double gamma, double delta)
{
  double fs, fl, fh, fr, fe, fc1, fc2, ftmp, flast;
  double err1;
  double *fvals;
  double **splx;                /* the simplex of points */
  double *x0;                   /* centroid of simplex */
  double *xr;                   /* point for a reflection */
  double *xe;                   /* point for an expansion */
  double *xc1;                  /* point for a minor contraction */
  double *xc2;                  /* point for a major contraction */
  int s, l, h;
  int i, j, iters, futility;
  int lastprint;
 bool localfit_print = true;

  fvals = (double *) calloc (m_pasa->m_inumber_parameters + 1, sizeof (double));
  splx = (double **) calloc (m_pasa->m_inumber_parameters + 1, sizeof (double *));
  for (i = 0; i <= m_pasa->m_inumber_parameters; i++)
    splx[i] = (double *) calloc (m_pasa->m_inumber_parameters, sizeof (double));
  x0 = (double *) calloc (m_pasa->m_inumber_parameters, sizeof (double));
  xr = (double *) calloc (m_pasa->m_inumber_parameters, sizeof (double));
  xe = (double *) calloc (m_pasa->m_inumber_parameters, sizeof (double));
  xc1 = (double *) calloc (m_pasa->m_inumber_parameters, sizeof (double));
  xc2 = (double *) calloc (m_pasa->m_inumber_parameters, sizeof (double));

  /* build the initial simplex */
  for (i = 0; i < m_pasa->m_inumber_parameters; i++) {
    splx[0][i] = x[i];
  }
  for (i = 1; i <= m_pasa->m_inumber_parameters; i++) {
    for (j = 0; j < m_pasa->m_inumber_parameters; j++) {
      if ((j + 1) == i)
        splx[i][j] = (x[j] * 2.25) + tol2;
      else
        splx[i][j] = x[j];
      xr[j] = splx[i][j];
    }
    fvals[i] = calcf (xr);
  }

  /* and of course compute function at starting point */
  fvals[0] = calcf (x);

  /* now find the largest, 2nd largest, smallest f values */
  if (fvals[0] > fvals[1]) 
  {
    h = 0;
    s = 1;
    l = 1;
  }
  else
  {
    h = 1;
    s = 0;
    l = 0;
  }
  fh = fvals[h];
  fs = fvals[s];
  fl = fvals[l];
  for (i = 2; i <= m_pasa->m_inumber_parameters; i++) {
    if (fvals[i] <= fvals[l]) {
      l = i;
      fl = fvals[i];
    } else {
      if (fvals[i] >= fvals[h]) {
        s = h;
        fs = fh;
        h = i;
        fh = fvals[i];
      } else if (fvals[i] >= fvals[s]) {
        s = i;
        fs = fvals[i];
      }
    }
  }
if(localfit_print == true)
{
  if ((s == h) || (s == l) || (h == l))
	  fprintf (m_pasa->ptr_asa_out, "\nPANIC: s,l,h not unique %d %d %d\n", s, h, l);

  fprintf (m_pasa->ptr_asa_out, "INITIAL SIMPLEX:\n");
  for (i = 0; i <= m_pasa->m_inumber_parameters; i++) {
    for (j = 0; j < m_pasa->m_inumber_parameters; j++) {
      fprintf (m_pasa->ptr_asa_out, "   %11.4g", splx[i][j]);
    }
    fprintf (m_pasa->ptr_asa_out, "      f = %12.5g", fvals[i]);
    if (i == h)
      fprintf (m_pasa->ptr_asa_out, "  HIGHEST");
    if (i == s)
      fprintf (m_pasa->ptr_asa_out, "  SECOND HIGHEST");
    if (i == l)
      fprintf (m_pasa->ptr_asa_out, "  LOWEST");
    fprintf (m_pasa->ptr_asa_out, "\n");
  }
}

/* MAJOR LOOP */

  flast = fl;
  futility = 0;
  lastprint = 0;
  iters = 0;
  err1 = 1.1 + (1.1 * tol1);
  while ((err1 > tol1) && (iters < m_pasa->Options.Iter_Max) &&
         (futility < (m_pasa->m_inumber_parameters * no_progress))) {
    iters++;

    /* now find the largest, 2nd largest, smallest f values */
    if (fvals[0] > fvals[1]) {
      h = 0;
      s = 1;
      l = 1;
    } else {
      h = 1;
      s = 0;
      l = 0;
    }
    fh = fvals[h];
    fs = fvals[s];
    fl = fvals[l];
    for (i = 2; i <= m_pasa->m_inumber_parameters; i++) {
      if (fvals[i] <= fvals[l]) {
        l = i;
        fl = fvals[i];
      } else {
        if (fvals[i] >= fvals[h]) {
          s = h;
          fs = fh;
          h = i;
          fh = fvals[i];
        } else if (fvals[i] >= fvals[s]) {
          s = i;
          fs = fvals[i];
        }
      }
    }
if(localfit_print == true)
{
    if ((s == h) || (s == l) || (h == l))
      fprintf (m_pasa->ptr_asa_out, "\nPANIC: s,l,h not unique %d %d %d\n", s, h, l);
}
    /* compute the centroid */
    for (j = 0; j < m_pasa->m_inumber_parameters; j++) {
      x0[j] = 0.0;
      for (i = 0; i <= m_pasa->m_inumber_parameters; i++) {
        if (i != h)
          x0[j] += splx[i][j];
      }
      x0[j] /= ((double) m_pasa->m_inumber_parameters);
    }

    if (fl < flast) {
      flast = fl;
      futility = 0;
    } else
      futility += 1;

if(localfit_print == true)
{
    fprintf (m_pasa->ptr_asa_out, "Iteration %3d f(best) = %12.6g halt? = %11.5g\n",
             iters, fl, err1);
    if ((iters - lastprint) >= 100) {
      fprintf (m_pasa->ptr_asa_out, "\n     Best point seen so far:\n");
      for (i = 0; i < m_pasa->m_inumber_parameters; i++) {
        fprintf (m_pasa->ptr_asa_out, "     x[%3d] = %15.7g\n", i, splx[l][i]);
      }
      lastprint = iters;
      fprintf (m_pasa->ptr_asa_out, "\n");
    }
    fflush (m_pasa->ptr_asa_out);
}

    /* STEP 1: compute a reflected point xr */
    for (i = 0; i < m_pasa->m_inumber_parameters; i++) {
      xr[i] = ((1.0 + alpha) * x0[i]) - (alpha * splx[h][i]);
    }
    fr = calcf ( xr);

    /* typical: <2nd-biggest , >lowest .  Go again */
    if ((fr < fs) && (fr > fl)) {
      for (i = 0; i < m_pasa->m_inumber_parameters; i++) {
        splx[h][i] = xr[i];
      }
      fvals[h] = fr;
      goto more_iterations;
    }

    /* STEP 2: if reflected point is favorable, expand the simplex */
    if (fr < fl) {
      for (i = 0; i < m_pasa->m_inumber_parameters; i++) {
        xe[i] = (gamma * xr[i]) + ((1.0 - gamma) * x0[i]);
      }
      fe = calcf (xe);
      
	  if (fe < fr) {            /* win big; expansion point tiny */
        for (i = 0; i < m_pasa->m_inumber_parameters; i++) {
          splx[h][i] = xe[i];
        }
        fvals[h] = fh = fe;
      } else
        /* still ok; reflection point a winner */
      {
        for (i = 0; i < m_pasa->m_inumber_parameters; i++) {
          splx[h][i] = xr[i];
        }
        fvals[h] = fh = fr;
      }
      goto more_iterations;
    }

    /* STEP 3: if reflected point is unfavorable, contract simplex */
    if (fr > fs) {
      if (fr < fh) {            /* may as well replace highest pt */
        for (i = 0; i < m_pasa->m_inumber_parameters; i++) {
          splx[h][i] = xr[i];
        }
        fvals[h] = fh = fr;
      }
      for (i = 0; i < m_pasa->m_inumber_parameters; i++) {
        xc1[i] = (beta1 * xr[i]) + ((1.0 - beta1) * x0[i]);
      }
      fc1 = calcf (xc1);
      
	  if (fc1 < fh) {           /* slight contraction worked */
        for (i = 0; i < m_pasa->m_inumber_parameters; i++) {
          splx[h][i] = xc1[i];
        }
        fvals[h] = fh = fc1;
        goto more_iterations;
      }
      /* now have to try strong contraction */
      for (i = 0; i < m_pasa->m_inumber_parameters; i++) {
        xc2[i] = (beta2 * splx[h][i]) + ((1.0 - beta2) * x0[i]);
      }
      fc2 = calcf (xc2);
      
	  if (fc2 < fh) {           /* strong contraction worked */
        for (i = 0; i < m_pasa->m_inumber_parameters; i++) {
          splx[h][i] = xc2[i];
        }
        fvals[h] = fh = fc2;
        goto more_iterations;
      }
    }

    /* STEP 4: nothing worked.  collapse the simplex around xl */
    for (i = 0; i <= m_pasa->m_inumber_parameters; i++) {
      if (i != l) {
        for (j = 0; j < m_pasa->m_inumber_parameters; j++) {
          splx[i][j] = (splx[i][j] + splx[l][j]) / delta;
          xr[j] = splx[i][j];
        }
        fvals[i] = calcf (xr);
      }
    }

  more_iterations:

    ftmp = 0.00;
    for (i = 0; i <= m_pasa->m_inumber_parameters; i++) {
      ftmp += fvals[i];
    }
    ftmp /= ((double) (m_pasa->m_inumber_parameters + 1));

    err1 = 0.00;
    for (i = 0; i <= m_pasa->m_inumber_parameters; i++) {
      err1 += ((fvals[i] - ftmp) * (fvals[i] - ftmp));
    }
    err1 /= ((double) (m_pasa->m_inumber_parameters + 1));
    err1 = sqrt (err1);
  }                             /* end of major while loop */

  /* find the smallest f value */
  l = 0;
  fl = fvals[0];
  for (i = 1; i <= m_pasa->m_inumber_parameters; i++) {
    if (fvals[i] < fvals[l])
      l = i;
  }

  /* give it back to the user */
  for (i = 0; i < m_pasa->m_inumber_parameters; i++) {
    x[i] = splx[l][i];
  }

  free (fvals);
  for (i = 0; i <= m_pasa->m_inumber_parameters; i++)
    free (splx[i]);
  free (splx);
  free (x0);
  free (xr);
  free (xe);
  free (xc1);
  free (xc2);

  return (iters);
}

double LocalFitting::fitloc()
{

  double x;
  double *xsave;
  double tol1, tol2, alpha, beta1, beta2, gamma, delta, floc, fsave, ffinal;
  int no_progress, tot_iters, locflg, bndflg;
 
  bool localfit_print = true;

if(localfit_print == true)
{
  if (m_pasa->Options.Fit_Local >= 1) {
    fprintf (m_pasa->ptr_asa_out, "\n\nSTART LOCAL FIT\n");
  } else {
    fprintf (m_pasa->ptr_asa_out, "\n\nSTART LOCAL FIT Independent of ASA\n");
  }
  fflush (m_pasa->ptr_asa_out);
}
  xsave = (double *) calloc (m_pasa->m_inumber_parameters, sizeof (double));
  bndflg = 0;

  /* The following simplex parameters may need adjustments for your system. */
  
  tol1 = EPS_DOUBLE;
  tol2 = EPS_DOUBLE * 100.;
  no_progress = 4;
  alpha = 1.0;
  beta1 = 0.75;
  beta2 = 0.75;
  gamma = 1.25;
  delta = 2.50;

  for (int index_v = 0; index_v < m_pasa->m_inumber_parameters; ++index_v) {
	  xsave[index_v] = m_pasa->parameter_initial_final[index_v];
  }

  fsave = m_pasa->CalcCostFunc(m_pasa->parameter_initial_final);

  tot_iters = simplex (m_pasa->parameter_initial_final, tol1,tol2, no_progress, alpha, beta1, beta2, gamma, delta);
  fflush (m_pasa->ptr_asa_out);

  for (int index_v = 0; index_v < m_pasa->m_inumber_parameters; ++index_v) 
  {
    x = m_pasa->parameter_initial_final[index_v];
	if ((x < m_pasa->m_dparameter_minimum[index_v])|| (x > m_pasa->m_dparameter_maximum[index_v])) 
      bndflg = 1;
  }

  floc = m_pasa->CalcCostFunc(m_pasa->parameter_initial_final);

  if (fabs (floc - fsave) < (double) EPS_DOUBLE)
  {
    locflg = 1;
    ffinal = fsave;
	if(localfit_print == true)
		fprintf (m_pasa->ptr_asa_out, "\nsame global cost = %g\tlocal cost = %g\n\n",fsave, floc);
  }
  else 
  {
		if (floc < fsave) 
		{
		  if (m_pasa->Options.Fit_Local == 2 && bndflg == 1) 
		  {
			locflg = 1;
			ffinal = fsave;
		  }
		  else 
		  {
			locflg = 0;
			ffinal = floc;
		  }
		} 
		else
		{
		  locflg = 1;
		  ffinal = fsave;
		}
		if(localfit_print == true)
			fprintf (m_pasa->ptr_asa_out, "\nDIFF global cost = %g\tlocal cost = %g\n\n", fsave, floc);
 }

  for (int index_v = 0; index_v <  m_pasa->m_inumber_parameters; ++index_v) 
  {
	if (fabs (m_pasa->parameter_initial_final[index_v] - xsave[index_v]) < (double) EPS_DOUBLE) 
	{
		if(localfit_print == true)
		{
		  fprintf (m_pasa->ptr_asa_out, "same global param[%ld] = %g\tlocal param = %g\n",
			  index_v, xsave[index_v], m_pasa->parameter_initial_final[index_v]);
		}
    }
	else
	{
		if(localfit_print == true)
			fprintf (m_pasa->ptr_asa_out, "DIFF global param[%ld] = %g\tlocal param = %g\n", index_v, xsave[index_v],  m_pasa->parameter_initial_final[index_v]);
      if (locflg == 1) 
         m_pasa->parameter_initial_final[index_v] = xsave[index_v];
    }
  }

	if(localfit_print == true)
	{
	  fprintf (m_pasa->ptr_asa_out, "\n");
	  fflush (m_pasa->ptr_asa_out);
	}

   free (xsave);

   return (ffinal);
}



