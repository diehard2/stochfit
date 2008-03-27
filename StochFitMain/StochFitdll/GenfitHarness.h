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
#include "genome.h"
#include "multilayer.h"
#include "SA_Dispatcher.h"

class Genfit
{
	public:
		Genfit(LPCWSTR Directory, double* Q, double* Refl, double* ReflErr, double* QError, int DatapointNum,	double rholipid,double rhoh2o, double supSLD, int parratlayers, double layerlength,double surfabs, 
			   double wavelength, double subaps, double supabs, BOOL usesurfabs, double leftoffset, double QErr, BOOL forcenorm,
			   double forcesig, bool debug, bool XRonly, double resolution,double totallength, BOOL impnorm, int objfunc);
		~Genfit();
		int Start(int iterations);
		int Cancel();
		int Priority(int priority);
		int GetData(double* Z, double* RhoOut, double* Q, double* ReflOut, double* roughness, double* chisquare, double* goodnessoffit, BOOL* isfinished);
		
	

		int m_isearchalgorithm;
		int m_irhocount;
		int m_irefldatacount;
		int m_sigmasearch;
		int objectivefunction;
		bool m_bwarmedup;
		double m_dresolution;
		double m_dtotallength;
		BOOL m_bimpnorm;
		SA_Dispatcher* SA;

	private:
		int Processing();
		void LoadFromFile(CReflCalc* ml, GARealGenome* genome, const char* filename, string fileloc);
		static DWORD WINAPI InterThread(LPVOID lParam);
		void WritetoFile(CReflCalc* ml, GARealGenome* genome, const char* filename);
		void UpdateFits(CReflCalc* ml, GARealGenome* genome, int currentiteration);
	    void Initialize(double* Q, double* Refl, double* ReflError, double* QError, int PointCount);
	

		double* Zinc;
		double* Qinc;
		double* Rho;
		double* Refl;
		
		string m_Directory;
		HANDLE m_hThread;
		HANDLE mutex;
		BOOL m_bupdated;
		BOOL m_busesurfabs;
		BOOL m_bforcenorm;
		
		bool m_bthreadstop;
		bool m_bdebugging;
		bool m_bXRonly;

		double m_dsurfabs;
		double m_dwavelength;
		double m_dsubabs;
		double m_dsupabs;
		double m_dRoughness;
		double m_dChiSquare;
		double m_dGoodnessOfFit;
		double m_dlayerlength;
		double m_dleftoffset;
		double m_dfilmSLD;
		double m_dbetalipid;
		double m_dsubSLD;
		double m_dsupSLD;
		double m_dQerr;
		double m_dforcesig;

		int m_itotaliterations;
		int m_icurrentiteration;
		int m_iparratlayers;
		int m_ipriority;

		CReflCalc ml0;
		GARealGenome* genome;
};