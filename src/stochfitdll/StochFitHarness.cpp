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


#include <stochfit/common/platform.h>
#include "ParamVector.h"
#include "ReflCalc.h"
#include "SimulatedAnnealing.h"
#include "StochFitHarness.h"

StochFit::StochFit(ReflSettings* InitStruct)
{
	m_bthreadstop = false;
	m_bupdated = false;
	m_bwarmedup = false;
	m_ipriority = 2;

	m_Directory = string(InitStruct->Directory);

    fnpop = m_Directory + "/pop.dat";
    fnrf  = m_Directory + "/rf.dat";
    fnrho = m_Directory + "/rho.dat";

	m_SA = new SA_Dispatcher();

	InitializeSA(InitStruct, m_SA);
	Initialize(InitStruct);
}

StochFit::~StochFit()
{
	if(Zinc != NULL)
	{
		delete params;

		if(m_SA != NULL)
			delete m_SA;

		delete[] Zinc;
		delete[] Qinc;
		delete[] Rho;
		delete[] Refl;
	}
}

void StochFit::Initialize(ReflSettings* InitStruct)
{
	 //////////////////////////////////////////////////////////
	 /******** Setup Variables and ReflectivityClass ********/
	 ////////////////////////////////////////////////////////

	m_cRefl.Init(InitStruct);
	m_cEDP.Init(InitStruct);


	//Setup the params - We start with a slightly roughened ED curve
	params = new ParamVector(InitStruct);

	 /////////////////////////////////////////////////////
     /******** Prepare Arrays for the Front End ********/
	////////////////////////////////////////////////////

	m_irhocount = m_cEDP.Get_EDPPointCount();
	m_irefldatacount = m_cRefl.GetDataCount();

	Zinc = new double[m_irhocount];
	Qinc = new double[m_irefldatacount];
	Rho = new double[m_irhocount];
	Refl = new double[m_irefldatacount];

	//Let the frontend know that we've set up the arrays
	m_bwarmedup = true;

	//If we have a population already, load it
	LoadFromFile();

	// Update the constraints on the params
	params->UpdateBoundaries(NULL,NULL);

	m_SA->InitializeParameters(InitStruct, params, &m_cRefl, &m_cEDP);

	if(m_SA->CheckForFailure() == true)
		platform_error("Catastrophic error in SA - please contact the author");
}

int StochFit::Processing()
{
	bool accepted = false;

	//Main loop
     for(int isteps=0;(isteps < m_itotaliterations) && (m_bthreadstop == false);isteps++)
	 {
			accepted = m_SA->Iteration(params);

			if(accepted || isteps == 0)
			{
				m_dChiSquare = m_cRefl.m_dChiSquare;
				m_dGoodnessOfFit = m_cRefl.m_dgoodnessoffit;
			}
		    UpdateFits(isteps);

			//Write the population file every 5000 iterations
			if((isteps+1)%5000 == 0 || m_bthreadstop == true || isteps == m_itotaliterations-1)
			{
				//Write out the population file for the best minimum found so far
				if(m_isearchalgorithm != 0 && m_SA->Get_IsIterMinimum())
					WritetoFile((m_Directory + "/BestSASolution.txt").c_str());

				WritetoFile(fnpop.c_str());
			}
	 }

	//Update the arrays one last time
	UpdateFits(m_icurrentiteration);

	return 0;
}

void StochFit::UpdateFits(int currentiteration)
{
		if(m_bupdated == true)
		{
			//Check to see if we're updating
			m_cEDP.GenerateEDP(params);
			m_cEDP.WriteOutputFile(fnrho);
			m_cRefl.ParamsRF(&m_cEDP, fnrf);
			m_dRoughness = params->getroughness();


			for(int i = 0; i<m_irhocount;i++)
			{
				Zinc[i] = i*m_cEDP.Get_Dz();
				Rho[i] =  m_cEDP.m_EDP[i].re/m_cEDP.m_EDP[m_cEDP.Get_EDPPointCount()-1].re;
			}

			for(int i = 0; i < m_irefldatacount;i++)
			{
				#ifndef CHECKREFLCALC

					if(m_cRefl.m_dQSpread > 0.0)
					{
						Refl[i] = m_cRefl.reflpt[i];
						Qinc[i] = m_cRefl.xi[i];
					}
					else
					{
						Refl[i] = m_cRefl.dataout[i];
						Qinc[i] = m_cRefl.qarray[i];
					}
				#else
					Qinc[i] = m_cRefl.xi[i];
					Refl[i] = m_cRefl.reflpt[i];
				#endif
			}
			m_bupdated = false;
		}
		m_icurrentiteration = currentiteration;
}

int StochFit::Start(int iterations)
{
	m_itotaliterations = iterations;
	m_thread = std::jthread([this]{ Processing(); });
	return 0;
}

int StochFit::Cancel()
{
	m_bthreadstop = true;
	if(m_thread.joinable())
	{
		m_thread.request_stop();
		m_thread.join();
	}
	return 0;
}

void StochFit::InitializeSA(ReflSettings* InitStruct, SA_Dispatcher* SA)
{
	SA->Initialize(InitStruct->Debug, m_Directory);
	SA->Initialize_Subsytem(InitStruct);
}

int StochFit::GetData(double* Z, double* RhoOut, double* Q, double* ReflOut, double* roughness, double* chisquare, double* goodnessoffit, BOOL* isfinished)
{
	//Sleep while we are generating our output data
	if(m_icurrentiteration != m_itotaliterations-1)
	{
		m_bupdated = true;

		while(m_bupdated == true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	else
	{
		m_bupdated = false;
	}
	//We only have one thread, and we're controlling access to it, so no need for fancy synchronization here

	for(int i = 0; i < m_irhocount; i++)
	{
		Z[i] = Zinc[i];
		RhoOut[i] = Rho[i];
	}

	for(int i = 0; i < m_irefldatacount; i++)
	{
		Q[i] = Qinc[i];
		ReflOut[i] = Refl[i];
	}

	*roughness = m_dRoughness;
	*chisquare = m_dChiSquare;
	*goodnessoffit = m_dGoodnessOfFit;

	if(m_bthreadstop == true)
		*isfinished = TRUE;
	else
		*isfinished = FALSE;

	return m_icurrentiteration;
}

int StochFit::Priority(int priority)
{
	// Thread priority is not portable; store for potential future use.
	m_ipriority = priority;
	return 0;
}

void StochFit::WritetoFile(const char* filename)
{
	ofstream outfile;
	outfile.open(filename);
	outfile<< params->getroughness() <<' '<< m_cEDP.Get_FilmAbs()*params->getSurfAbs()/m_cEDP.Get_WaveConstant() <<
		' '<< m_SA->Get_Temp() <<' '<< params->getImpNorm() << ' ' << m_SA->Get_AveragefSTUN() << endl;

	for(int i = 0; i < params->RealparamsSize(); i++)
	{
		outfile<<params->GetRealparams(i)<< endl;
	}

	outfile.close();
}

void StochFit::LoadFromFile(string file)
{
   ParamVector params1 = *params;
   ifstream infile;

   if(file.empty())
	 infile.open(fnpop.c_str());
   else
	 infile.open(file.c_str());

   int size = params->RealparamsSize();
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
		*params = params1;
		m_cEDP.Set_FilmAbs(beta);
		m_SA->Set_Temp(1.0/currenttemp);
		params->setImpNorm(normfactor);
		m_SA->Set_AveragefSTUN(avgfSTUN);
    }
	infile.close();

	//If we're resuming, and we were Tunneling, load the best file
	if(kk == true && fnpop.find("BestSASolution.txt") == string::npos)
	{
		LoadFromFile(m_Directory + "BestSASolution.txt");
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
