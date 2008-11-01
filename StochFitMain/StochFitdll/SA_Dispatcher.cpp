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

void SA_Dispatcher::Initialize(bool debug, wstring directory)
{
		m_cSA = new SimAnneal(debug, directory);
}

SA_Dispatcher::~SA_Dispatcher()
{
		delete m_cSA;
}

void SA_Dispatcher::Initialize_Subsytem(ReflSettings* InitStruct)
{
		m_cSA->Initialize(InitStruct);
}

void SA_Dispatcher::InitializeParameters(ReflSettings* InitStruct, ParamVector *params, CReflCalc *m_cRefl, CEDP* m_cEDP)
{
		m_cSA->InitializeParameters(InitStruct, params, m_cRefl, m_cEDP);
}

bool SA_Dispatcher::Iteration(ParamVector *params)
{
		return m_cSA->Iteration(params);
}

bool SA_Dispatcher::CheckForFailure()
{
		return false;
}


double SA_Dispatcher::Get_Temp()
{
		return m_cSA->GetTemp();
}

void SA_Dispatcher::Set_Temp(double Temp)
{
		m_cSA->SetTemp(Temp);
}

double SA_Dispatcher::Get_LowestEnergy()
{
		return m_cSA->GetLowestEnergy();
}

bool SA_Dispatcher::Get_IsIterMinimum()
{
		return m_cSA->IsIterMinimum();
}

double SA_Dispatcher::Get_AveragefSTUN()
{
		return m_cSA->m_daveragefstun;
}
	
void SA_Dispatcher::Set_AveragefSTUN(double fSTUN)
{
		m_cSA->m_daveragefstun = fSTUN;
}

