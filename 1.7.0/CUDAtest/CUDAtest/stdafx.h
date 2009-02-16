// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>
#include <tchar.h>



#pragma once
#pragma warning(disable : 4391)
#pragma warning(disable : 4996) 
#pragma warning(disable : 1786) 
#pragma warning(disable : 810)

#include <windows.h>
#include <math.h>
#include <string>
#include <algorithm>
#include <queue>
#include <vector>
#include <fstream>
#include <atlconv.h>
#include <malloc.h>
#include <time.h>
//#include <omp.h>
#include <iostream>
#include <conio.h>
using namespace std;

//Define CHECKREFLCALC to see how the actual reflectivity calculation is proceeding
//#define CHECKREFLCALC
//Define DEBUGFILE to monitor the fit for each iteration (large file)
#define DEBUGFILE
//Define SINGLEPROCDEBUG for debugging when the development machine only has one processor
//#define SINGLEPROCDEBUG

//Maximum number of processors to run the reflectivity calculation on
#define MAX_OMP_THREADS 6


using namespace std;
 
//Use the error function from libmmd.lib (Intel math library), can't use mathimf.h due to
//compiler bug - hopefully this will be fixed...
extern "C" float __declspec(dllimport) __cdecl erff(float __x);
//Useful defines
#include "defines.h"


	


// QRange and token errors
const double qrange[] = {0.016,
	0.0165,
	0.017,
	0.0175,
	0.018,
	0.0185,
	0.019,
	0.0195,
	0.02,
	0.0205,
	0.021,
	0.0215,
	0.022,
	0.0225,
	0.023,
	0.0235,
	0.024,
	0.0245,
	0.025,
	0.0255,
	0.026,
	0.0265,
	0.027,
	0.0275,
	0.028,
	0.0285,
	0.029,
	0.0295,
	0.03,
	0.0305,
	0.031,
	0.0315,
	0.032,
	0.036,
	0.04,
	0.044,
	0.048,
	0.052,
	0.056,
	0.066,
	0.076,
	0.086,
	0.096,
	0.106,
	0.116,
	0.126,
	0.13,
	0.14,
	0.15,
	0.16,
	0.17,
	0.173333,
	0.176667,
	0.18,
	0.183333,
	0.186667,
	0.19,
	0.193333,
	0.196667,
	0.2,
	0.203333,
	0.206667,
	0.21,
	0.213333,
	0.216667,
	0.22,
	0.225,
	0.23,
	0.235,
	0.24,
	0.245,
	0.25,
	0.255,
	0.26,
	0.265,
	0.27,
	0.275,
	0.28,
	0.285,
	0.29,
	0.295,
	0.3,
	0.305,
	0.31,
	0.315,
	0.32,
	0.325,
	0.33,
	0.335,
	0.34,
	0.345,
	0.35,
	0.355,
	0.36,
	0.365,
	0.37,
	0.375,
	0.38,
	0.385,
	0.39,
	0.395,
	0.4,
	0.41,
	0.42,
	0.43,
	0.44,
	0.45,
	0.46,
	0.47,
	0.48,
	0.49,
	0.5,
	0.51,
	0.52,
	0.53,
	0.54,
	0.55,
	0.56,
	0.57,
	0.59,
	0.61,
	0.63,
	0.65,
	0.67,
	0.69,
	0.71,
	0.73,
	0.75};

	const double errorrange[] =
	{0.00289046,
0.00289017,
0.00289672,
0.00291206,
0.00290489,
0.0029168,
0.00292866,
0.00293795,
0.00294215,
0.00293332,
0.00291849,
0.00284216,
0.00220311,
0.00180512,
0.00157996,
0.00139979,
0.00117874,
0.001086,
0.000992484,
0.000919236,
0.000856943,
0.000786971,
0.000731759,
0.000669215,
0.000609067,
0.000574721,
0.0005373,
0.0005061,
0.000480729,
0.000452309,
0.000425547,
0.000393674,
0.000369928,
0.000316611,
0.000208927,
0.000147209,
8.11E-05,
6.21E-05,
4.62E-05,
3.39E-05,
2.10E-05,
1.33E-05,
2.65E-06,
1.79E-06,
1.22E-06,
8.43E-07,
8.33E-07,
5.92E-07,
4.40E-07,
2.83E-07,
1.92E-07,
3.07E-07,
2.53E-07,
2.08E-07,
1.68E-07,
1.35E-07,
1.05E-07,
8.14E-08,
6.21E-08,
2.52E-08,
1.84E-08,
1.29E-08,
8.43E-09,
5.59E-09,
3.37E-09,
1.93E-09,
1.19E-09,
1.58E-09,
2.92E-09,
4.96E-09,
7.46E-09,
1.01E-08,
1.29E-08,
1.55E-08,
1.80E-08,
2.01E-08,
2.22E-08,
2.37E-08,
2.51E-08,
2.60E-08,
2.67E-08,
2.71E-08,
2.74E-08,
2.71E-08,
2.68E-08,
2.61E-08,
2.54E-08,
2.44E-08,
2.34E-08,
2.22E-08,
2.12E-08,
2.00E-08,
1.85E-08,
1.72E-08,
1.60E-08,
1.48E-08,
1.35E-08,
3.57E-09,
3.12E-09,
2.94E-09,
2.53E-09,
2.38E-09,
2.00E-09,
1.60E-09,
1.26E-09,
1.01E-09,
7.60E-10,
6.15E-10,
4.99E-10,
7.02E-10,
4.64E-10,
3.45E-10,
2.80E-10,
2.37E-10,
1.12E-10,
1.09E-10,
1.18E-10,
1.26E-10,
1.24E-10,
1.63E-10,
1.39E-10,
1.05E-10,
8.74E-11,
9.22E-11,
8.33E-11,
7.29E-11,
7.70E-11,
9.35E-11
	};