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
#include "SimulatedAnnealing.h"
#include <iomanip>

SimAnneal::SimAnneal(): m_bisiterminimum(false), m_dTemp(-1.0),
	m_ipoorsolutionacc(0),m_inumberpoorsol(0),m_daverageSTUNval(0), m_dState1(-1.0)
{}

void SimAnneal::Initialize(ReflSettings* InitStruct)
{
	m_sdirectory = InitStruct->Directory;
	m_bdebugging = InitStruct->Debug;
	m_iPlattime = InitStruct->Platiter;
	m_daveragefstun = InitStruct->Inittemp;

	if(m_dTemp == -1)
	{
		if(!InitStruct->Adaptive)
		{
			m_dTemp = 1.0/InitStruct->Inittemp;
		}
		else
		{
			m_dTemp = 10;
		}
	}

	//SA Parameters
	m_iIteration = 0;
	m_iTime = 0;
	m_dgamma = InitStruct->Gamma;
	m_dslope = InitStruct->Slope;
	m_itempiter = InitStruct->Tempiter;
	m_badaptive = InitStruct->Adaptive;
	m_iSTUNfunc = InitStruct->STUNfunc;
	m_iSTUNdec = InitStruct->STUNdeciter;
	m_dgammadec = InitStruct->Gammadec;

	//Pertubation parameters
	mc_stepsize = InitStruct->Paramtemp;
	m_isigmasearch = InitStruct->Sigmasearch;
	m_inormsearch = InitStruct->NormalizationSearchPerc;
	m_iabssearch = InitStruct->AbsorptionSearchPerc;
	m_ialgorithm = InitStruct->Algorithm;

	if(m_bdebugging)
	{
		debugfile.open(wstring(m_sdirectory + wstring(L"\\debug.txt")).c_str());
		rejfile.open(wstring(m_sdirectory + wstring(L"\\rejfile.txt")).c_str());
	}
	
	//Initialize the random number generator
	srand(time_seed());
}

SimAnneal::~SimAnneal()
{
	if(m_bdebugging)
	{
		debugfile.close();
		rejfile.close();
	}
}

bool SimAnneal::EvaluateGreedy(double bestval, double curval)
{
	if(curval < bestval)
	{
		m_dbestsolution = curval;
		return true;
	}
	else
		return false;
}            

bool SimAnneal::EvaluateSA(double bestval, double curval)
{
	if(min(curval,bestval) < m_dbestsolution)
	{
		m_dbestsolution = min(curval,bestval);
		return true;
	}

	double deltaE = curval-bestval;
	m_iIteration++;
	
	if((m_iIteration)%m_iPlattime == 0)
	{
		Schedule();
		m_iTime++;
	}

	return random(100.0,0.0) < ProbCalc(deltaE);
}

bool SimAnneal::EvaluateSTUN(long double bestval,long double curval)
{

	double probability = 0;
	double deltaE = fSTUN(curval)-fSTUN(bestval);

	if(min(curval,bestval) < m_dbestsolution)
	{
		m_dbestsolution = min(curval,bestval);
		if(m_bdebugging)
		{
			rejfile << "New minimimum found - " << (double)m_dbestsolution << endl;
			debugfile << "New minimimum found - " << (double)m_dbestsolution << endl;
		}
		return true;
	}

	if(m_badaptive)
	{
			if(m_bdebugging)
			{
				debugfile << "Iteration - " << m_iIteration << endl;
				debugfile << (double)m_dbestsolution << "  " << (double)bestval << "  " << (double)curval << endl;
				debugfile << "fstun(curval) = " << 1.0+fSTUN(curval) << "   fstun(bestval) = "  << 1.0+fSTUN(bestval) << endl;
				debugfile << "deltaE = " << (double)deltaE << endl;
				debugfile << "ProbCalcE = " << ProbCalc(deltaE) << endl ;
				debugfile << "Temp = " << 1.0/(double)m_dTemp << endl;
			}

			if(Q.size() < m_iPlattime)
				Q.push_front(1.0+fSTUN(curval));
			else
			{
				Q.pop_back();
				Q.push_front(1.0+fSTUN(curval));
			}

			if(Q.size() >= m_iPlattime)
			{
				m_daverageSTUNval = 0;

				for(int i = 0; i < Q.size(); i++)
				{
					m_daverageSTUNval += Q.at(i);
				}
				m_daverageSTUNval /= Q.size();
			}

			m_iIteration++;

			if(m_iIteration % m_itempiter == 0 && Q.size() >= m_iPlattime)
			{
				if(m_bdebugging)
				{
					if(m_inumberpoorsol != 0)
					{
						debugfile << "PoorSolAccPerc - " << (double)m_ipoorsolutionacc*100.0/(double)m_inumberpoorsol <<  endl << endl;
						rejfile << (double)m_ipoorsolutionacc*100.0/(double)m_inumberpoorsol << "   " << m_inumberpoorsol << endl;
					}
					else
						rejfile << "All solutions accepted" << endl;

						m_ipoorsolutionacc = 0;
						m_inumberpoorsol = 0;
				}
				AdjustTemp(m_daverageSTUNval);

				if(m_iIteration%m_iSTUNdec == 0)
				{
					m_daveragefstun *= m_dgammadec;
					rejfile << "decreasing average fstun = " << (double)m_daveragefstun << endl;
				}
			}
			probability = ProbCalc(deltaE);
	}
	else
	{
		

		if(m_bdebugging)
		{
				debugfile << "Iteration - " << m_iIteration << endl;
				debugfile << (double)m_dbestsolution << "  " << (double)bestval << "  " << (double)curval << endl;
				debugfile << "1.0+fstun(curval) = " << 1.0+fSTUN(curval) << "   fstun(bestval) = "  << 1.0+fSTUN(bestval) << endl;
				debugfile << "deltaE = " << (double)deltaE << endl;
				debugfile << "ProbCalcE = " << ProbCalc(deltaE) << endl ;
				debugfile << "Temp = " << 1.0/(double)m_dTemp << endl;
		}

		m_iIteration++;

		if(m_iIteration % m_iPlattime == 0)
			Schedule();
		
		probability = ProbCalc(deltaE);
	}
		if(m_bdebugging)
		{
			if(probability < 100.0)
				m_inumberpoorsol++;
		}

		if(random(100.0,0.0) < probability)
		{
			if(m_bdebugging)
			{
				debugfile << "Accepted\n\n";
				if(probability < 100)
					m_ipoorsolutionacc++;
			}
			return true;
		}
		else 
		{
			if(m_bdebugging)
				debugfile << "Rejected\n\n";

			return false;
		}
}

double SimAnneal::ProbCalc(double deltaE)
{
	return exp(-1*m_dTemp*deltaE)*100;
}


double SimAnneal::Schedule()
{
	//Our schedule is exponential	- also pretty slow
	//m_dTemp = m_dTemp*pow(0.999999,(double)m_iTime);
	//Just right
	if(m_dTemp > 1e-30)
		m_dTemp = m_dTemp/m_dslope;
	//Our schedule is geometrical - way too slow
	//m_dTemp = 1/(1+m_iTime);
	return 0;
}

double SimAnneal::fSTUN(double val)
{
	if(m_iSTUNfunc == 0)
	   return	-exp(-1*m_dgamma*(val-m_dbestsolution));
	else if (m_iSTUNfunc == 1)
		return sinh(m_dgamma*(val-m_dbestsolution))-1;
	else
		return log(m_dgamma*(val-m_dbestsolution)+sqrt((m_dgamma*(val-m_dbestsolution))*m_dgamma*(val-m_dbestsolution)+1))-1;
}
	
void SimAnneal::AdjustTemp(double averageSTUNval)
{
	if(averageSTUNval > m_daveragefstun)
	{
		if(m_dTemp < 1e200)
			m_dTemp *=  0.5/m_dslope;
	
		if(m_bdebugging)
			debugfile << "Reducing Temp - " << (double)m_dTemp << endl;
	}
	else
	{
		if(m_dTemp > 1e-200 )
			m_dTemp  *= m_dslope;
	
		if(m_bdebugging)
			debugfile << "Increasing Temp - " << (double)m_dTemp << endl;
	}
}

void SimAnneal::SetTemp(double currenttemp)
{
	m_dTemp = currenttemp;
}

double SimAnneal::GetTemp()
{
	return 1.0/m_dTemp;
}

double SimAnneal::GetLowestEnergy()
{
	return m_dbestsolution;
}

bool SimAnneal::Iteration(double score)
{
	bool accepted = false;
	m_bisiterminimum = false;
	

	//Don't allow for negative ED in XR case
	if(score == -1.0)
		return false;

	if(m_dState1 == -1.0)
	{
		m_dState1 = score;
		return true;
	}

	if(m_ialgorithm == 0)
	{
		accepted = EvaluateGreedy(m_dState1,score);
	}
	else if (m_ialgorithm == 1)
	{
		accepted = EvaluateSA(m_dState1, score);
	}
	else
	{
		accepted = EvaluateSTUN(m_dState1, score);
	}

	if(accepted)
	{
		m_dState1 = score;

		if(m_dState1 == m_dbestsolution)
			m_bisiterminimum = true;
	}
	return accepted;
}

bool SimAnneal::IsIterMinimum()
{
	return m_bisiterminimum;
}

void SimAnneal::TakeStep(ParamVector* m_cParamVec)
{
		double roughmult = 5.0/3.0;
		
		//Pick the box we're going to mutate
		int ii= random(m_cParamVec->GetInitializationLength()-1,0);
		int perc = random(100, 1);
		
	
		//Only mutate for the actual guessed fuzzy layer length 
		if(perc > m_isigmasearch + m_inormsearch + m_iabssearch)
		{
			m_cParamVec->SetMutatableParameter(ii, random(m_cParamVec->GetMutatableParameter(ii) + mc_stepsize,
									m_cParamVec->GetMutatableParameter(ii) - mc_stepsize));
		}
		else if(perc <= m_isigmasearch)
		{
			m_cParamVec->SetRoughness(random(m_cParamVec->GetRoughness()*(1+roughmult*mc_stepsize),m_cParamVec->GetRoughness()*(1-roughmult*mc_stepsize)));
		}
		else if(perc > m_isigmasearch && perc <= m_isigmasearch + m_iabssearch)
		{
			m_cParamVec->setSurfAbs(random(m_cParamVec->GetSurfAbs()*(1.0+mc_stepsize),m_cParamVec->GetSurfAbs()*(1.0-mc_stepsize)));
		}
		else
		{
			m_cParamVec->setImpNorm(random(m_cParamVec->getImpNorm()*(1.0+mc_stepsize),m_cParamVec->getImpNorm()*(1.0-mc_stepsize)));
		}
}
