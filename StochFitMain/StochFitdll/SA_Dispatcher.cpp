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

#include "stdafx.h"
#include "SA_Dispatcher.h"

SA_Dispatcher::SA_Dispatcher()
{
}

void SA_Dispatcher::Initialize(ReflSettings* InitStruct)
{
		m_cSA.Initialize(InitStruct);
		m_cEDP.Init(InitStruct);
		m_cRefl.Init(InitStruct);
		m_cObjective.Initialize(InitStruct);
		m_cParams.Initialize(InitStruct);
}

SA_Dispatcher::~SA_Dispatcher()
{
	
}

bool SA_Dispatcher::Iteration(ParamVector *params)
{
		ParamVector OldParams = *params;
	    m_cSA.TakeStep(params);
		//We have to update the reflectivity generator with the normalization factor, the EDP has no concept of this
		m_cRefl.m_dnormfactor = params->getImpNorm();
		m_cEDP.GenerateEDP(params);
		m_cRefl.MakeReflectivity(&m_cEDP);
		
		m_dObjectiveScore = m_cObjective.GetFunction(m_cRefl.GetReflData());
		m_dChiSquare = m_cObjective.ChiSquare(m_cRefl.GetReflData());
		
		if(m_cSA.Iteration(m_dObjectiveScore))
		{
			return true;
		}
		else
		{
			*params = OldParams;
			return false;
		}
}

double SA_Dispatcher::Get_ChiSquare()
{
	return m_dChiSquare > 0.0 ? m_dChiSquare : -1.0;
}

double SA_Dispatcher::Get_ObjectiveScore()
{
	return m_dObjectiveScore > 0.0 ? m_dObjectiveScore : -1.0;
}

bool SA_Dispatcher::CheckForFailure()
{
		return false;
}


double SA_Dispatcher::Get_Temp()
{
		return m_cSA.GetTemp();
}

void SA_Dispatcher::Set_Temp(double Temp)
{
		m_cSA.SetTemp(Temp);
}

double SA_Dispatcher::Get_LowestEnergy()
{
		return m_cSA.GetLowestEnergy();
}

bool SA_Dispatcher::Get_IsIterMinimum()
{
		return m_cSA.IsIterMinimum();
}

double SA_Dispatcher::Get_AveragefSTUN()
{
		return m_cSA.m_daveragefstun;
}
	
void SA_Dispatcher::Set_AveragefSTUN(double fSTUN)
{
		m_cSA.m_daveragefstun = fSTUN;
}

CEDP* SA_Dispatcher::GetEDP()
{
	return &m_cEDP;
}

void SA_Dispatcher::GetReflData(double* Q, double* Refl)
{
	m_cRefl.GetData(Q, Refl);
}