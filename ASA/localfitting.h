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

#pragma once

 
double calcf (double (*cost_function1)
       (double *, double *, double *, double *, double *, LONG_INT , int *,
        int *, int *, USER_DEFINES *), double *xloc,
       double *parameter_lower_bound, double *parameter_upper_bound,
       double *cost_tangents, double *cost_curvature,
       LONG_INT * parameter_dimension, int *parameter_int_real,
	   int *cost_flag, int *exit_code, USER_DEFINES * OPTIONS, FILE * ptr_out);

int simplex (double (*cost_function)
         (double *, double *, double *, double *, double *, LONG_INT ,
          int *, int *, int *, USER_DEFINES *), double *x,
         double *parameter_lower_bound, double *parameter_upper_bound,
         double *cost_tangents, double *cost_curvature,
         LONG_INT * parameter_dimension, int *parameter_int_real,
         int *cost_flag, int *exit_code, USER_DEFINES * OPTIONS,
         FILE * ptr_out, double tol1, double tol2, int no_progress,
         double alpha, double beta1, double beta2, double gamma, double delta);

double fitloc (double (*cost_function)
        (double *, double *, double *, double *, double *, LONG_INT, int *,
         int *, int *, USER_DEFINES *), double *xloc,
        double *parameter_lower_bound, double *parameter_upper_bound,
        double *cost_tangents, double *cost_curvature,
        LONG_INT * parameter_dimension, int *parameter_int_real,
        int *cost_flag, int *exit_code, USER_DEFINES * OPTIONS,
        FILE * ptr_out);