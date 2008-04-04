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
#include "multilayer.h"
#include "ParamVector.h"

class SimAnneal
{
	private:

		//Variables
		ParamVector temp_params;
		CReflCalc* multi;

		int m_iIteration;
		int m_iPlattime;
		long double m_dTemp;
		wstring m_sdirectory;

		//Variables for tracking acceptance and rejection
		int m_ipoorsolutionacc;
		int m_inumberpoorsol;
		double m_daverageSTUNval;

		double m_iTime;
		long double m_dgamma;
		double m_dslope;
		bool m_badaptive;
		bool m_bdebugging;
		int m_iSTUNdec;
		double m_dgammadec;
		bool m_bisiterminimum;
		int m_iSTUNfunc;
		int m_itempiter;
		double mc_stepsize;
		wofstream debugfile;
		wofstream rejfile;
		std::deque<double> Q;
		
		//Functions
		double ProbCalc(double deltaE);
		double fSTUN(double val);
		double fSTUNExp(double val);
		double Schedule();
		void AdjustTemp(double AverageSTUNval);

		int m_isigmasearch;
		int m_ialgorithm;
		double m_dState1, m_dState2;
		double TakeStep(ParamVector* params);
	
public:
	SimAnneal(bool debug, std::wstring directory);
		~SimAnneal();
		void Initialize(double inittemp, double tempplateautime, double gamma, double slope, bool adaptive, int tempiter, 
			int STUNfunc, int deciter, double gammadec);
		bool Iteration(ParamVector* params);
		void InitializeParameters(double step, ParamVector* params, CReflCalc* m_cRefl, int simgasearch, int algorithm);
		bool EvaluateGreedy(double bestval, double curval);
		bool EvaluateSA(double bestval, double curval);
		bool EvaluateSTUN(long double bestval,long double curval);
		void SetGamma(double gamma);
		void SetTemp(double currenttemp);
		double GetTemp();
		double GetLowestEnergy();
		bool IsIterMinimum();
		double m_daveragefstun;
		long double m_dbestsolution;


};