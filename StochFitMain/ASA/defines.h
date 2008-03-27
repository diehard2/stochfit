#pragma once

#define ASA_ID "/* $Id: asa.c,v 26.24 2007/12/26 17:16:39 ingber Exp ingber $ */"

#ifndef G_FIELD
#define G_FIELD 12
#endif
#ifndef G_PRECISION
#define G_PRECISION 7
#endif

#define INTEGER_TYPE		((int) 1)
#define REAL_TYPE		((int) -1)
#define INTEGER_NO_REANNEAL	((int) 2)
#define REAL_NO_REANNEAL	((int) -2)


  /* You can define SMALL_FLOAT to better correlate to your machine's
     precision, i.e., as used in asa */
#ifndef SMALL_FLOAT
#define SMALL_FLOAT 1.0E-200
#endif

  /* You can define your machine's maximum and minimum doubles here */
//Leave these in place

#ifndef MIN_DOUBLE
#define MIN_DOUBLE ((double) SMALL_FLOAT)
#endif

#ifndef MAX_DOUBLE
#define MAX_DOUBLE ((double) 1.0 / (double) SMALL_FLOAT)
#endif

#ifndef EPS_DOUBLE
#define EPS_DOUBLE ((double) SMALL_FLOAT)
#endif


#ifndef CHECK_EXPONENT
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

#define CHECK_EXPONENT FALSE
#endif


#define NORMAL_EXIT			((int) 0)
#define P_TEMP_TOO_SMALL		((int) 1)
#define C_TEMP_TOO_SMALL		((int) 2)
#define COST_REPEATING			((int) 3)
#define TOO_MANY_INVALID_STATES		((int) 4)
#define IMMEDIATE_EXIT			((int) 5)
#define INVALID_USER_INPUT		((int) 7)
#define INVALID_COST_FUNCTION		((int) 8)
#define INVALID_COST_FUNCTION_DERIV	((int) 9)
#define CALLOC_FAILED			((int) -1)

#define LONG_INT long int

#define SHUFFLE 256      

  /*  NO_REANNEAL(x)  can determine whether to calculate derivatives. */
#define NO_REANNEAL(x)	(IABS(parameter_type[x]) == 2)

#if CHECK_EXPONENT
  /* EXPONENT_CHECK
     checks that an exponent x is within a valid range and,
     if not, adjusts its magnitude to fit in the range. */
#define MIN_EXPONENT (0.9 * log ((double) MIN_DOUBLE))
#define MAX_EXPONENT (0.9 * log ((double) MAX_DOUBLE))
#define EXPONENTCHECK(x) ((x) < MIN_EXPONENT ? MIN_EXPONENT : ((x) > MAX_EXPONENT ? MAX_EXPONENT : (x)))
#else
#define EXPONENT_CHECK(x) (x)
#endif                          /* CHECK_EXPONENT */



  /* INTEGER_PARAMETER(x) determines if the parameter is an integer type. */
#define INTEGER_PARAMETER(x) (parameter_type[x] > 0)

  /* ROW_COL_INDEX(i, j) converts from row i, column j to an index. */
#define MIN(x,y)	((x) < (y) ? (x) : (y))
#define MAX(x,y)	((x) > (y) ? (x) : (y))