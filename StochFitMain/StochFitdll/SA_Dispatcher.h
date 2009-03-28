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
#include "ReflCalc.h"
#include "ParamVector.h"
#include "SimulatedAnnealing.h"
#include "CEDP.h"
#include "CObjectiveFunc.h"



class SA_Dispatcher
{
private:

	SimAnneal m_cSA;
    CReflCalc m_cRefl;
	CEDP m_cEDP;
	ParamVector m_cParams;
	CObjective m_cObjective;
	double m_dChiSquare;
	double m_dObjectiveScore;
public: 
	SA_Dispatcher();
	~SA_Dispatcher();

	void Initialize(ReflSettings* InitStruct);
	bool Iteration(ParamVector* params);
	double Get_Temp();
	void Set_Temp(double Temp);
	double Get_LowestEnergy();
	bool Get_IsIterMinimum();
	double Get_AveragefSTUN();
	void Set_AveragefSTUN(double fSTUN);
	bool CheckForFailure();
	double Get_ChiSquare();
	double Get_ObjectiveScore();
	CEDP* GetEDP();
	void GetReflData(double* Q, double* Refl);
};
