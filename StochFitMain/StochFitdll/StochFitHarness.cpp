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
#include "ParamVector.h"
#include "ReflCalc.h"
#include "SimulatedAnnealing.h"
#include "StochFitHarness.h"

StochFit::StochFit(ReflSettings* InitStruct)
{
	m_bwarmedup = m_bthreadstop = m_bupdated = false;
	m_ipriority = 2;
	
	m_Directory = InitStruct->Directory;
    fnpop = InitStruct->Directory + wstring(L"\\pop.dat");
    fnrf = InitStruct->Directory + wstring(L"\\rf.dat");
    fnrho = InitStruct->Directory + wstring(L"\\rho.dat");

	Initialize(InitStruct);
	//Create our mutex that we use for locking purposes
	m_hStochMutex = CreateMutex(NULL, FALSE, L"StochMutex");
}

void StochFit::Initialize(ReflSettings* InitStruct)
{
	 //////////////////////////////////////////////////////////
	 /******** Setup Variables and ReflectivityClass ********/
	 ////////////////////////////////////////////////////////

	m_SA.Initialize(InitStruct);
	m_cParamVec.Initialize(InitStruct);
	m_cEDP.Initialize(InitStruct);

	//Blank the offsets for the DispArray and move the Display Q to Q
	#ifndef CHECKREFLCALC
		InitStruct->Q = InitStruct->DisplayQ;
		InitStruct->QPoints = InitStruct->DispQPoints;
	#endif

	InitStruct->HighQOffset = 0;
	InitStruct->CritEdgeOffset = 0;
	 
	m_cDispRefl.Initialize(InitStruct);

	 /////////////////////////////////////////////////////
     /******** Prepare Arrays for the Front End ********/
	////////////////////////////////////////////////////

	m_irhocount = m_SA.GetEDP()->Get_EDPPointCount();

	//We've moved the appropriate Q point count to Q points by now
	m_irefldatacount = InitStruct->QPoints;

	//Let the frontend know that we've set up the arrays
	m_bwarmedup = true;

	//If we have a population already, load it
	LoadFromFile();
}

DWORD WINAPI StochFit::InterThread(LPVOID lParam)
{
	StochFit* Internalpointer = (StochFit*)lParam;
	//Set the thread priority
	Internalpointer->Priority(Internalpointer->m_ipriority);

     //Main loop
     for(int isteps = 0;(isteps < Internalpointer->m_itotaliterations) && (!Internalpointer->m_bthreadstop);isteps++)
	 {
		 WaitForSingleObject(Internalpointer->m_hStochMutex, INFINITE);
		 
		 bool accepted = Internalpointer->m_SA.Iteration(&(Internalpointer->m_cParamVec));
		
		if(accepted || isteps == 0)
		{
			Internalpointer->m_dChiSquare = Internalpointer->m_SA.Get_ChiSquare();
			Internalpointer->m_dGoodnessOfFit = Internalpointer->m_SA.Get_ObjectiveScore();
		}
	    
		//Write the population file every 5000 iterations
		if((isteps+1)%5000 == 0 || Internalpointer->m_bthreadstop || isteps == Internalpointer->m_itotaliterations-1)
		{
			//Write out the population file for the best minimum found so far
			if(Internalpointer->m_isearchalgorithm != 0 && Internalpointer->m_SA.Get_IsIterMinimum())
				Internalpointer->WritetoFile(wstring(Internalpointer->m_Directory + L"\\BestSASolution.txt").c_str());

			Internalpointer->WritetoFile( Internalpointer->fnpop.c_str());
		}

		Internalpointer->m_icurrentiteration = isteps;

		ReleaseMutex(Internalpointer->m_hStochMutex);
	 }

	 return 0;
}

int StochFit::Start(int iterations)
{
	m_itotaliterations = iterations;
	DWORD dwDummy = 0;
	m_hThread = CreateThread(NULL, NULL, InterThread, this, NULL, &dwDummy);
	return 0;
}

int StochFit::Cancel()
{
	DWORD dwdummy = 0;
	m_bthreadstop = true;
	Sleep(500);
	if(m_hThread != NULL)
	{
		WaitForSingleObject(m_hThread, 500);
	}
	CloseHandle(m_hThread);
	m_hThread = NULL;
	return 0;
}


int StochFit::GetData(double* Z, double* RhoOut, double* Q, double* ReflOut, double* roughness, double* chisquare, double* goodnessoffit, bool* isfinished)
{
	WaitForSingleObject(m_hStochMutex, INFINITE);

	*roughness = m_cParamVec.GetRoughness();
	*chisquare = m_dChiSquare;
	*goodnessoffit = m_dGoodnessOfFit;
	*isfinished = m_bthreadstop;

	m_cEDP.GenerateEDP(&m_cParamVec);
	m_cDispRefl.SetNormFactor(m_cParamVec.getImpNorm());
	m_cDispRefl.MakeReflectivity(&m_cEDP);

	for(int i = 0; i<m_irhocount;i++)
	{
		Z[i] = m_SA.GetEDP()->GetZ()[i];
		RhoOut[i] =  m_SA.GetEDP()->GetDoubledEDP()[i].re/m_SA.GetEDP()->GetDoubledEDP()[m_SA.GetEDP()->Get_EDPPointCount()-1].re;
	}

	m_cDispRefl.GetData(Q, ReflOut);

	
	m_cEDP.WriteOutputFile(fnrho);
	m_cDispRefl.WriteOutputFile(fnrf);

	int tempiter = m_icurrentiteration;

	ReleaseMutex(m_hStochMutex);

	return tempiter;
}

int StochFit::Priority(int priority)
{
	//The higher priorities seem to sometimes cause race conditions and have
	//been removed. If the thread exists, change its priority. If we haven't started yet
	//set the base priority

	if(m_hThread != NULL)
	{
		switch(priority)
		{
			case 0:
				::SetThreadPriority(m_hThread,THREAD_PRIORITY_IDLE);
				break;
			case 1:
				::SetThreadPriority(m_hThread,THREAD_PRIORITY_LOWEST);
				break;
			case 2:
				::SetThreadPriority(m_hThread,THREAD_PRIORITY_NORMAL);
				break;
			case 3:
				::SetThreadPriority(m_hThread, THREAD_PRIORITY_ABOVE_NORMAL);
			default:
				::SetThreadPriority(m_hThread,THREAD_PRIORITY_NORMAL);
		};
	}
	else
	{
		m_ipriority = priority;
	}

	return 0;
}

void StochFit::WritetoFile(const wchar_t* filename)
{
	ofstream outfile;
	outfile.open(fnpop.c_str());
	outfile<< m_cParamVec.GetRoughness() <<' '<< m_SA.GetEDP()->Get_FilmAbs()*m_cParamVec.GetSurfAbs()/ m_SA.GetEDP()->Get_WaveConstant() <<
		' '<< m_SA.Get_Temp() <<' '<< m_cParamVec.getImpNorm() << ' ' << m_SA.Get_AveragefSTUN() << endl;

	for(int i = 0; i < m_cParamVec.RealparamsSize(); i++)
	{
		outfile<<m_cParamVec.GetRealparams(i)<< endl;
	}

	outfile.close();
}

void StochFit::LoadFromFile(wstring file)
{
   ParamVector params1 = m_cParamVec;
   ifstream infile;
   
   if(file == wstring(L""))
	 infile.open(fnpop.c_str());
   else
	 infile.open(file.c_str());

   int size = m_cParamVec.RealparamsSize();
   int counter = 0;
   bool kk = true;
   double beta = 0;
   double  avgfSTUN = 0;
   double currenttemp = 0;
   double normfactor = 0;
 
   int i = 0;
 

   if(infile.is_open())
   {
       double ED, roughness;
       infile >> roughness >> beta >> currenttemp >> normfactor >> avgfSTUN;
   
       params1.SetRoughness(roughness);

	   while(!infile.eof() && i < params1.RealparamsSize())
	   {
            if(infile>>ED)
			{
				if(i == 0)
					params1.SetSupphase(ED);
				else if (i == params1.RealparamsSize()-1)
					params1.SetSubphase(ED);
				else
				    params1.SetMutatableParameter(i-1,ED);
				
				counter++;
				i++;
			}
			else
			{
                kk = false;
                break;
            }
        }
	   infile>>ED;
        
    } 
    else 
	{
		kk = false;
	}

	if(kk && !infile.eof())
	{
		kk = false;
	}
    
    if(kk)
	{
		m_cParamVec = params1;
		m_SA.GetEDP()->Set_FilmAbs(beta);
		m_SA.Set_Temp(1.0/currenttemp);
		m_cParamVec.setImpNorm(normfactor);
		m_SA.Set_AveragefSTUN(avgfSTUN);
    }
	infile.close();

	//If we're resuming, and we were Tunneling, load the best file
	if(kk && wstring(fnpop).find(L"BestSASolution.txt") == wstring::npos)
	{
		LoadFromFile(wstring(m_Directory+L"BestSASolution.txt").c_str());
	}
}

void StochFit::GetArraySizes(int* RhoSize, int* ReflSize)
{
	*RhoSize = m_irhocount;
	*ReflSize = m_irefldatacount;
}

bool StochFit::GetWarmedUp()
{
	return m_bwarmedup;
}