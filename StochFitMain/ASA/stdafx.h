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

#pragma warning(disable : 4996) 
#pragma warning(disable : 4018) 
#pragma warning(disable : 4101)
#pragma warning(disable : 810)
#pragma warning(disable : 4391)
#pragma warning(disable : 4068)
#pragma warning(disable : 441)
#pragma warning(disable : 4244)
#pragma warning(disable : 4391)
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <conio.h>
#include <time.h>

#include "defines.h"
#include "random.h"

using namespace std;