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

inline double resettable_randflt (LONG_INT * rand_seed, int reset);

/***********************************************************************
* double myrand - returns random number between 0 and 1
*	This routine returns the random number generator between 0 and 1
***********************************************************************/


  /* returns random number in {0,1} */
inline double myrand (LONG_INT * rand_seed)
{

  /* See "Random Number Generators: Good Ones Are Hard To Find,"
     Park & Miller, CACM 31 (10) (October 1988) pp. 1192-1201.
     ***********************************************************
     THIS IMPLEMENTATION REQUIRES AT LEAST 32 BIT INTEGERS
     *********************************************************** */
#define _A_MULTIPLIER  16807L
#define _M_MODULUS     2147483647L      /* (2**31)-1 */
#define _Q_QUOTIENT    127773L  /* 2147483647 / 16807 */
#define _R_REMAINDER   2836L    /* 2147483647 % 16807 */
  long lo;
  long hi;
  long test;

  hi = *rand_seed / _Q_QUOTIENT;
  lo = *rand_seed % _Q_QUOTIENT;
  test = _A_MULTIPLIER * lo - _R_REMAINDER * hi;
  if (test > 0) {
    *rand_seed = test;
  } else {
    *rand_seed = test + _M_MODULUS;
  }
  return ((double) *rand_seed / _M_MODULUS);

}

/***********************************************************************
* double randflt
***********************************************************************/


inline double randflt (LONG_INT * rand_seed)
{
  return (resettable_randflt (rand_seed, 0));
}

/***********************************************************************
* double resettable_randflt
***********************************************************************/
 /* shuffles random numbers in random_array[SHUFFLE] array */
inline double resettable_randflt (LONG_INT * rand_seed, int reset)
{

  /* This RNG is a modified algorithm of that presented in
   * %A K. Binder
   * %A D. Stauffer
   * %T A simple introduction to Monte Carlo simulations and some
   *    specialized topics
   * %B Applications of the Monte Carlo Method in statistical physics
   * %E K. Binder
   * %I Springer-Verlag
   * %C Berlin
   * %D 1985
   * %P 1-36
   * where it is stated that such algorithms have been found to be
   * quite satisfactory in many statistical physics applications. */

  double rranf;
  unsigned kranf;
  int n;
  static int initial_flag = 0;
  LONG_INT initial_seed;

  static double random_array[SHUFFLE];  /* random variables */

  if (*rand_seed < 0)
    *rand_seed = -*rand_seed;

  if ((initial_flag == 0) || reset) {
    initial_seed = *rand_seed;

    for (n = 0; n < SHUFFLE; ++n)
      random_array[n] = myrand (&initial_seed);

    initial_flag = 1;

    for (n = 0; n < 1000; ++n)  /* warm up random generator */
      rranf = randflt (&initial_seed);

    rranf = randflt (rand_seed);

    return (rranf);
  }

  kranf = (unsigned) (myrand (rand_seed) * SHUFFLE) % SHUFFLE;
  rranf = *(random_array + kranf);
  *(random_array + kranf) = myrand (rand_seed);

  return (rranf);
}


