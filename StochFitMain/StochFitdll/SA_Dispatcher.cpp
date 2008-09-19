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

SA_Dispatcher::SA_Dispatcher():m_bUseASA(false)
{
}

void SA_Dispatcher::Initialize(bool debug, bool ASAonoff, wstring directory)
{
	if(ASAonoff == false)
		m_cSA = new SimAnneal(debug, directory);
	else
		m_cASA = new ASA(debug, directory + wstring(L"\\asa_out.txt"), 0);

	m_bUseASA = ASAonoff;
}

SA_Dispatcher::~SA_Dispatcher()
{
	if(m_bUseASA == false)
		delete m_cSA;
	else
		delete m_cASA;
}

void SA_Dispatcher::Initialize_Subsytem(ReflSettings* InitStruct)
{

	if(m_bUseASA == false)
		m_cSA->Initialize(InitStruct);
	
}

void SA_Dispatcher::InitializeParameters(ReflSettings* InitStruct, ParamVector *params, CReflCalc *m_cRefl, CEDP* m_cEDP)
{
	if(m_bUseASA == false)
		m_cSA->InitializeParameters(InitStruct, params, m_cRefl, m_cEDP);
	else
		m_cASA->Initialize(params, m_cRefl, m_cEDP);

}

bool SA_Dispatcher::Iteration(ParamVector *params)
{
	if(m_bUseASA == false)
		return m_cSA->Iteration(params);
	else 
		return m_cASA->Iteration(params);
}

bool SA_Dispatcher::CheckForFailure()
{
	if(m_bUseASA == true)
		return m_cASA->CheckFailure();
	else 
		return false;
}


double SA_Dispatcher::Get_Temp()
{

	if(m_bUseASA == false)
		return m_cSA->GetTemp();
	else
		return -1;
}

void SA_Dispatcher::Set_Temp(double Temp)
{
	if(m_bUseASA == false)
		m_cSA->SetTemp(Temp);
}

double SA_Dispatcher::Get_LowestEnergy()
{
	if(m_bUseASA == false)
		return m_cSA->GetLowestEnergy();
	else
		return -1;
}

bool SA_Dispatcher::Get_IsIterMinimum()
{
	if(m_bUseASA == false)
		return m_cSA->IsIterMinimum();
	else
		return false;
}

double SA_Dispatcher::Get_AveragefSTUN()
{
	if(m_bUseASA == false)
		return m_cSA->m_daveragefstun;
	else
		return -1;
}
	

void SA_Dispatcher::Set_AveragefSTUN(double fSTUN)
{
	if(m_bUseASA == false)
		m_cSA->m_daveragefstun = fSTUN;
}

