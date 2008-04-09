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

StochFit::StochFit(ReflSettings initstruct)
{
	m_Directory = initstruct.Directory;
	m_dsubSLD = initstruct.SubSLD;
	m_dfilmSLD = initstruct.FilmSLD;
	m_iparratlayers = initstruct.Boxes;
	m_dlayerlength = initstruct.FilmLength;
	m_dsurfabs = initstruct.FilmAbs;
	m_dwavelength = initstruct.Wavelength;
	m_dsubabs = initstruct.SubAbs;
	m_bthreadstop = false;
	m_bupdated = FALSE;
	m_bimpnorm = initstruct.Impnorm;
	m_icurrentiteration = 0;
	m_ipriority = 2;
	m_busesurfabs = initstruct.UseSurfAbs;
	m_isearchalgorithm = 0;
	m_dRoughness = 3;
	m_bwarmedup = false;
	m_dleftoffset = initstruct.Leftoffset;
	m_dQerr = initstruct.QErr;
	m_dsupSLD = initstruct.SupSLD;
	m_dsupabs = initstruct.SupAbs;
	m_dforcesig = initstruct.Forcesig;
	m_bdebugging = initstruct.Debug;
	SA = new SA_Dispatcher();
	m_bforcenorm = initstruct.Forcenorm;
	m_bXRonly = initstruct.XRonly;
	m_dresolution = initstruct.Resolution;
	m_dtotallength = initstruct.Totallength;
	m_bimpnorm = initstruct.Impnorm;
	objectivefunction = initstruct.Objectivefunction;
	m_dparamtemp = initstruct.Paramtemp;

	InitializeSA(initstruct.Sigmasearch, initstruct.Algorithm, initstruct.Inittemp, initstruct.Platiter, 
		initstruct.Slope, initstruct.Gamma, initstruct.STUNfunc, initstruct.Adaptive, initstruct.Tempiter,
		initstruct.STUNdeciter, initstruct.Gammadec);

	Initialize(initstruct.Q, initstruct.Refl, initstruct.ReflError, initstruct.QError, initstruct.QPoints);
}

StochFit::~StochFit()
{
	if(Zinc != NULL)
	{
		delete params;
		
		if(SA != NULL)
			delete SA;

		delete[] Zinc;
		delete[] Qinc;
		delete[] Rho;
		delete[] Refl;
	}
}
	
void StochFit::Initialize(double* Q, double* Reflect, double* ReflError, double* QError, int PointCount)
{
	 //////////////////////////////////////////////////////////
	 /******** Setup Variables and ReflectivityClass ********/
	 ////////////////////////////////////////////////////////
	double resolution;
	double waveconst =  m_dwavelength*m_dwavelength/(2.0*M_PI);

	// Set the densities and absorbances
	m_dfilmSLD *= 1e-6 * waveconst;
	m_dsubSLD *= 1e-6 * waveconst;
	m_dsupSLD *= 1e-6 * waveconst;
	double betafilm = m_dsurfabs * waveconst;
	double betasubphase = m_dsubabs * waveconst;
	double betasuperphase = m_dsupabs * waveconst;

	//Default of 3 points per Angstrom - Seems to be a good number
	//The computational degree of difficulty scales linearly with the number of
	//data points
	if(m_dresolution == 0)
		resolution = 3.0;
	else
		resolution = m_dresolution;

    double dz0=1.0/resolution;

	//Set the total length of our surface layer - default 80 Angstroms of superphase,
	//7 extra Angstroms of file, and 40 Angstroms of subphase

	if(m_dtotallength > 0)
		m_cRefl.totalsize = m_dtotallength; 
	else
        m_cRefl.totalsize = m_dleftoffset + m_dlayerlength + 7 + 40;

	//Set the film property
    m_cRefl.rho_a = m_dfilmSLD; 

	if(m_busesurfabs == TRUE)
	{
		m_cRefl.beta_a = betafilm;
		m_cRefl.beta_sub = betasubphase;
		m_cRefl.beta_sup = betasuperphase;
	}
	else
	{
		m_cRefl.beta_a = 0;
		m_cRefl.beta_sub = 0;
		m_cRefl.beta_sup = 0;
	}

	//Setup the params - We start with a slightly roughened ED curve 
	params = new ParamVector(m_iparratlayers,m_dforcesig,m_busesurfabs, m_bimpnorm);
	params->SetSupphase(m_dsupSLD/m_dfilmSLD);


	int actuallipidlength = (int)ceil((m_iparratlayers)*(m_dlayerlength/(m_dlayerlength+7.0)));

	for(int i = 0 ; i <= actuallipidlength; i++)
		params->SetMutatableParameter(i,1.0);

	for(int i = actuallipidlength+1; i < m_iparratlayers; i++)
		params->SetMutatableParameter(i,m_dsubSLD/m_dfilmSLD);	
		
	params->SetSubphase(m_dsubSLD/m_dfilmSLD);

	m_cRefl.m_dboxsize = (m_dlayerlength+7.0)/(m_iparratlayers);

	//Initialize the multilayer class
	m_cRefl.init(m_cRefl.totalsize*resolution,m_dwavelength,dz0,m_busesurfabs,m_iparratlayers, m_dleftoffset, m_bforcenorm, m_dQerr, m_bXRonly);
	m_cRefl.SetupRef(Q, Reflect, ReflError, QError, PointCount, params);
	m_cRefl.objectivefunction = objectivefunction;
	m_cRefl.m_bImpNorm = m_bimpnorm;
	
	//Set the output file names
    m_cRefl.fnpop = m_Directory + wstring(L"\\pop.dat");
    m_cRefl.fnrf = m_Directory + wstring(L"\\rf.dat");
    m_cRefl.fnrho = m_Directory + wstring(L"\\rho.dat");
 
	 /////////////////////////////////////////////////////
     /******** Prepare Arrays for the Front End ********/
	////////////////////////////////////////////////////

	m_irhocount = m_cRefl.totalsize*resolution;

	#ifndef CHECKREFLCALC
		if(m_dQerr > 0)
			m_irefldatacount = m_cRefl.m_idatapoints;
		else
			m_irefldatacount = m_cRefl.tarraysize;
	#else
		m_irefldatacount = m_cRefl.m_idatapoints;
	#endif

	Zinc = new double[m_irhocount];
	Qinc = new double[m_irefldatacount];
	Rho = new double[m_irhocount];
	Refl = new double[m_irefldatacount];

	//Let the frontend know that we've set up the arrays
	m_bwarmedup = true;

	//If we have a population already, load it
	LoadFromFile(&m_cRefl,params, m_cRefl.fnpop.c_str(), m_Directory);

	// Update the constraints on the params
	params->UpdateBoundaries(NULL,NULL);
}

int StochFit::Processing()
{

	bool accepted = false;
	//Set the thread priority
	Priority(m_ipriority);
	
	SA->InitializeParameters(m_dparamtemp, params, &m_cRefl, m_sigmasearch, m_isearchalgorithm);
	
	if(SA->CheckForFailure() == true)
	{
		MessageBox(NULL, L"Catastrophic error in SA - please contact the author", NULL,NULL);
		return -1;
	}
	 
	//Main loop
	 for(int isteps=0;(isteps < m_itotaliterations) && (m_bthreadstop == false);isteps++)
	 {
			accepted = SA->Iteration(params);
		   
		
			if(accepted)
			{
				m_dChiSquare = m_cRefl.m_dChiSquare;
				m_dGoodnessOfFit = m_cRefl.m_dgoodnessoffit;
			}
		
			UpdateFits(&m_cRefl, params, isteps);

			//Write the population file every 5000 iterations
			if((isteps+1)%5000 == 0 || m_bthreadstop == true || isteps == m_itotaliterations-1)
			{
				//Write out the population file for the best minimum found so far
				if(m_isearchalgorithm != 0 && SA->Get_IsIterMinimum())
					WritetoFile(&m_cRefl, params, wstring(m_Directory + L"\\BestSASolution.txt").c_str());

				WritetoFile(&m_cRefl, params, m_cRefl.fnpop.c_str());
			}

		
	 }

	//Update the arrays one last time
	UpdateFits(&m_cRefl, params, m_icurrentiteration);

	return 0;
}

void StochFit::UpdateFits(CReflCalc* ml, ParamVector* params, int currentiteration)
{
		
		
		if(m_bupdated == TRUE)
		{
			//Check to see if we're updating

			ml->paramsrf(params);
			m_dRoughness = params->getroughness();
			

			for(int i = 0; i<m_irhocount;i++)
			{
				Zinc[i] = i*ml->dz0;
				Rho[i] =  ml->nk[i].re/ml->nk[m_irhocount-1].re;
			}
			
			for(int i = 0; i<m_irefldatacount;i++)
			{
				#ifndef CHECKREFLCALC
					
					if(m_dQerr > 0)
					{				
						Refl[i] = ml->reflpt[i];
						Qinc[i] = ml->xi[i];
					}
					else
					{
						Refl[i] = ml->dataout[i];
						Qinc[i] = ml->qarray[i];
					}
				#else
					Qinc[i] = ml->xi[i];
					Refl[i] = 1.0;//ml->reflpt[i];
				#endif
			}
			m_bupdated = FALSE;
		}
		m_icurrentiteration = currentiteration;
	

		
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

void StochFit::InitializeSA(int sigmasearch, int algorithm, double inittemp, int platiter, double slope, double gamma, int STUNfunc, BOOL adaptive, int tempiter, int STUNdeciter, double gammadec)
{
	if(algorithm == 3)
		SA->Initialize(m_bdebugging, true, m_Directory);
	else
		SA->Initialize(m_bdebugging, false, m_Directory);

	SA->Initialize_Subsytem(inittemp,platiter,gamma ,slope, adaptive, tempiter,STUNfunc, STUNdeciter,gammadec);
	m_sigmasearch = sigmasearch;
	m_isearchalgorithm = algorithm;


}

int StochFit::GetData(double* Z, double* RhoOut, double* Q, double* ReflOut, double* roughness, double* chisquare, double* goodnessoffit, BOOL* isfinished)
{
	//Sleep while we are generating our output data
	if(m_icurrentiteration != m_itotaliterations)
	{
		m_bupdated = TRUE;

		while(m_bupdated == TRUE)
		{
			Sleep(100);
		}
	}

	//Stop the other thread while we are updating so the
	//data doesn't update as we're reading it

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
	//The higher priorities seem to sometimes cause race conditions and have
	//been removed

	//If the thread exists, change its priority. If we haven't started yet
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

void StochFit::WritetoFile(CReflCalc* ml, ParamVector* params, const wchar_t* filename)
{
	

	ofstream outfile;
	outfile.open(filename);
	outfile<< params->getroughness() <<' '<< ml->beta_a*params->getSurfAbs()*(2.*M_PI)/m_dwavelength*m_dwavelength <<
		' '<< SA->Get_Temp() <<' '<< params->getImpNorm() << ' ' << SA->Get_AveragefSTUN() << endl;

	for(int i = 0; i < params->RealparamsSize(); i++)
	{
		outfile<<params->GetRealparams(i)<< endl;
	}

	outfile.close();
}

void StochFit::LoadFromFile(CReflCalc* ml, ParamVector* params,const wchar_t* filename, wstring fileloc )
{
   ParamVector params1 = *params;
   ifstream infile(filename);
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
		ml->beta_a = beta*m_dwavelength*m_dwavelength/(2.0*M_PI);
		SA->Set_Temp(1.0/currenttemp);
		params->setImpNorm(normfactor);
		SA->Set_AveragefSTUN(avgfSTUN);
    }
	infile.close();

	//If we're resuming, and we were Tunneling, load the best file
	if(kk == true && wstring(filename).find(L"BestSASolution.txt") == wstring::npos)
	{
		LoadFromFile(ml,params, wstring(fileloc+L"BestSASolution.txt").c_str(), fileloc);
	}

}
