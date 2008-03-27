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
#include "Genome.h"
#include "SimulatedAnnealing.h"
#include "ASA.h"


class SA_Dispatcher
{
private:
	bool m_bUseASA;

	SimAnneal* m_cSA;
	ASA* m_cASA;

public: 
	SA_Dispatcher(bool debug, bool ASAonoff, std::string directory);
	~SA_Dispatcher();

	void Initialize_Subsytem(double inittemp, double tempplateautime, double gamma, double slope, bool adaptive, int tempiter, 
			int STUNfunc, int deciter, double gammadec);
	void InitializeParameters(double step, GARealGenome* genome, CReflCalc* ml0, int sigmasearch, int algorithm);
	bool Iteration(GARealGenome* genome);
	double Get_Temp();
	void Set_Temp(double Temp);
	double Get_LowestEnergy();
	bool Get_IsIterMinimum();
	double Get_AveragefSTUN();
	void Set_AveragefSTUN(double fSTUN);
	bool CheckForFailure();



};