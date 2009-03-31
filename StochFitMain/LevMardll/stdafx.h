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
#pragma warning(disable:11505)

#include <math.h>
#include <fstream>
#include <omp.h>
#include <ctime>
#include <vector>
#include <algorithm>
#include <atlbase.h>
#include <string>
#include "..\StochFitDll\random.h"
#include "..\StochFitDll\mycomplex.h"
#include ".\Levmar\lm.h"

using namespace MyComplexNumber;
using namespace Random;
using namespace std;

//Use the error function from libmmd.lib (Intel math library), can't use mathimf.h due to
//compiler bug
extern "C" double __declspec(dllimport) __cdecl erf(double __x);

#define M_PI	3.1415926535897932384626433 




