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

#include "platform.h"
#include <levmar/Settings.h>

extern "C" EXPORT void FastReflfit(BoxReflSettings* InitStruct, double parameters[], double covar[], int paramsize, double info[]);

extern "C" EXPORT void FastReflGenerate(BoxReflSettings* InitStruct, double parameters[], int parametersize, double Reflectivity[]);

extern "C" EXPORT void Rhofit(BoxReflSettings* InitStruct, double parameters[], double covariance[], int parametersize, double info[]);

extern "C" EXPORT void RhoGenerate(BoxReflSettings* InitStruct, double parameters[], int paramsize, double ED[], double BoxED[]);

extern "C" EXPORT void StochFit(BoxReflSettings* InitStruct, double parameters[], double covararray[], int paramsize, 
			double info[], double ParamArray[], double chisquarearray[], int* paramarraysize);


