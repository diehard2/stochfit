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
#include "ParamVector.h"
#include "ReflCalc.h"
#include "SA_Dispatcher.h"


class StochFit
{
	public:
		StochFit(ReflSettings* InitStruct);
		~StochFit();
		int Start(int iterations);
		int Cancel();
		int Priority(int priority);
		int GetData(double* Z, double* RhoOut, double* Q, double* ReflOut, double* roughness, double* chisquare, double* goodnessoffit, BOOL* isfinished);
		void InitializeSA(ReflSettings* InitStruct, SA_Dispatcher* SA);
		void GetArraySizes(int* RhoSize, int* ReflSize);
		bool GetWarmedUp();
		
		
		
		SA_Dispatcher* m_SA;
		bool m_bwarmedup;

	private:
		int Processing();
		void LoadFromFile(CReflCalc* ml, ParamVector* params, wstring File = wstring(L""));
		static DWORD WINAPI InterThread(LPVOID lParam);
		void WritetoFile(CReflCalc* ml, ParamVector* params, const wchar_t* filename);
		void UpdateFits(CReflCalc* ml, ParamVector* params, int currentiteration);
	    void Initialize(ReflSettings* InitStruct);
	

		double* Zinc;
		double* Qinc;
		double* Rho;
		double* Refl;

		HANDLE m_hThread;
		HANDLE mutex;
		BOOL m_bupdated;
		
		bool m_bthreadstop;
		
		//Set the output file names
		wstring m_Directory;
		wstring fnpop;
		wstring fnrf;
		wstring fnrho;
		double m_dRoughness;
		double m_dChiSquare;
		double m_dGoodnessOfFit;
		int m_itotaliterations;
		int m_icurrentiteration;
		int m_iparratlayers;
		int m_ipriority;
		int m_irhocount;
		int m_irefldatacount;
		int m_isearchalgorithm;
		CReflCalc m_cRefl;
		ParamVector* params;
};