#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <math.h>
#include <fstream>
#include "lm.h"

using namespace std;

//Use the error function from libmmd.lib (Intel math library), can't use mathimf.h due to
//compiler bug. Need to define before including mycomplex class
extern "C" double __declspec(dllimport) __cdecl erf(double __x);
extern "C" double __declspec(dllimport) __cdecl erfc(double __x);

#include "myComplex.h"
using namespace MyComplexNumber;

#define M_PI	3.1415926535897932384626433 




