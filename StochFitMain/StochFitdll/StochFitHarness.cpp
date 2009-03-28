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
	m_bthreadstop = false;
	m_bupdated = FALSE;
	m_bwarmedup = false;
	m_ipriority = 2;
	
	m_Directory = InitStruct->Directory;
    fnpop = InitStruct->Directory + wstring(L"\\pop.dat");
    fnrf = InitStruct->Directory + wstring(L"\\rf.dat");
    fnrho = InitStruct->Directory + wstring(L"\\rho.dat");

	Initialize(InitStruct);
	//Create our mutex
	m_hStochMutex = CreateMutex(NULL, FALSE, L"StochMutex");
}

StochFit::~StochFit()
{}
	
void StochFit::Initialize(ReflSettings* InitStruct)
{
	 //////////////////////////////////////////////////////////
	 /******** Setup Variables and ReflectivityClass ********/
	 ////////////////////////////////////////////////////////

	m_SA.Initialize(InitStruct);
	params.Initialize(InitStruct);
 
	//Blank the offsets for the DispArray and move the Display Q to Q
	ReflSettings temp = *InitStruct;

#ifdef CHECKREFLCALC
	temp.Q = InitStruct->DisplayQ;
	temp.QPoints = InitStruct->DispQPoints;
#endif
	temp.HighQOffset = 0;
	temp.CritEdgeOffset = 0;
	m_cDispRefl.Init(&temp);
	m_EDP.Init(&temp);
	 /////////////////////////////////////////////////////
     /******** Prepare Arrays for the Front End ********/
	////////////////////////////////////////////////////

	m_irhocount = m_SA.GetEDP()->Get_EDPPointCount();
	m_irefldatacount = m_cDispRefl.GetDataCount();

	//Let the frontend know that we've set up the arrays
	m_bwarmedup = true;

	//If we have a population already, load it
	LoadFromFile();

	// Update the constraints on the params
	params.UpdateBoundaries(NULL,NULL);
}

int StochFit::Processing()
{
	//Set the thread priority
	Priority(m_ipriority);

	bool accepted = false;

	//Main loop
     for(int isteps=0;(isteps < m_itotaliterations) && (m_bthreadstop == false);isteps++)
	 {

		 WaitForSingleObject(m_hStochMutex, INFINITE);
		 accepted = m_SA.Iteration(&params);
		
			if(accepted || isteps == 0)
			{
				m_dChiSquare = m_SA.Get_ChiSquare();
				m_dGoodnessOfFit = m_SA.Get_ObjectiveScore();
			}
		    
			//Write the population file every 5000 iterations
			if((isteps+1)%5000 == 0 || m_bthreadstop == true || isteps == m_itotaliterations-1)
			{
				//Write out the population file for the best minimum found so far
				if(m_isearchalgorithm != 0 && m_SA.Get_IsIterMinimum())
					WritetoFile(wstring(m_Directory + L"\\BestSASolution.txt").c_str());

				WritetoFile( fnpop.c_str());
			}

			m_icurrentiteration = isteps;

			ReleaseMutex(m_hStochMutex);
	 }

	 return 0;
}

DWORD WINAPI StochFit::InterThread(LPVOID lParam)
{
	StochFit* Internalpointer = (StochFit*)lParam;
	Internalpointer->Processing();
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


int StochFit::GetData(double* Z, double* RhoOut, double* Q, double* ReflOut, double* roughness, double* chisquare, double* goodnessoffit, BOOL* isfinished)
{
	WaitForSingleObject(m_hStochMutex, INFINITE);

	*roughness = params.getroughness();
	*chisquare = m_SA.Get_ChiSquare();
	*goodnessoffit = m_SA.Get_ObjectiveScore();
	*isfinished = m_bthreadstop == true ? TRUE : FALSE;


	for(int i = 0; i<m_irhocount;i++)
	{
		Z[i] = m_SA.GetEDP()->GetZ()[i];
		RhoOut[i] =  m_SA.GetEDP()->GetDoubledEDP()[i].re/m_SA.GetEDP()->GetDoubledEDP()[m_SA.GetEDP()->Get_EDPPointCount()-1].re;
	}
	
	m_EDP.GenerateEDP(&params);
	m_cDispRefl.m_dnormfactor = params.getImpNorm();
	m_cDispRefl.MakeReflectivity(&m_EDP);
	m_cDispRefl.GetData(Q, ReflOut);

	
	m_EDP.WriteOutputFile(fnrho);
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
			default:
				::SetThreadPriority(m_hThread,THREAD_PRIORITY_NORMAL);
		};
	}
	else
		m_ipriority = priority;

	return 0;
}

void StochFit::WritetoFile(const wchar_t* filename)
{
	ofstream outfile;
	outfile.open(fnpop.c_str());
	outfile<< params.getroughness() <<' '<< m_SA.GetEDP()->Get_FilmAbs()*params.getSurfAbs()/ m_SA.GetEDP()->Get_WaveConstant() <<
		' '<< m_SA.Get_Temp() <<' '<< params.getImpNorm() << ' ' << m_SA.Get_AveragefSTUN() << endl;

	for(int i = 0; i < params.RealparamsSize(); i++)
	{
		outfile<<params.GetRealparams(i)<< endl;
	}

	outfile.close();
}

void StochFit::LoadFromFile(wstring file)
{
   ParamVector params1 = params;
   ifstream infile;
   
   if(file == wstring(L""))
	 infile.open(fnpop.c_str());
   else
	 infile.open(file.c_str());

   int size = params.RealparamsSize();
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
   
       params1.setroughness(roughness);

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

	if(kk == true && infile.eof() == false)
	{
		kk = false;
	}
    
    if(kk == true)
	{
		params = params1;
		m_SA.GetEDP()->Set_FilmAbs(beta);
		m_SA.Set_Temp(1.0/currenttemp);
		params.setImpNorm(normfactor);
		m_SA.Set_AveragefSTUN(avgfSTUN);
    }
	infile.close();

	//If we're resuming, and we were Tunneling, load the best file
	if(kk == true && wstring(fnpop).find(L"BestSASolution.txt") == wstring::npos)
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