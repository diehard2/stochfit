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

#pragma once
#pragma warning(disable : 4391)
#pragma warning(disable : 4996) 
#pragma warning(disable : 1786) 
#pragma warning(disable : 810)


#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
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
#include <omp.h>
#include "mycomplex.h"
#include <xmmintrin.h>
#include <mmintrin.h>
#include "SettingsStruct.h"
#include <ctime>
#include <conio.h>
#include <iostream>


//Define CHECKREFLCALC to see how the actual reflectivity calculation is proceeding
#define CHECKREFLCALC
//Define SINGLEPROCDEBUG for debugging when the development machine only has one processor
//#define SINGLEPROCDEBUG

//Maximum number of processors to run the reflectivity calculation on
#define MAX_OMP_THREADS 8
#define M_PI	3.1415926535897932384623233 

using namespace MyComplexNumber;
using namespace std;
 
//Use the error function from libmmd.lib (Intel math library), can't use mathimf.h due to
//compiler bug - hopefully this will be fixed...
extern "C" float __declspec(dllimport) __cdecl erff(float __x);


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