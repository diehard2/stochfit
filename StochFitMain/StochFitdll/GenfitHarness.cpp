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
#include "genome.h"
#include "multilayer.h"
#include "SimulatedAnnealing.h"
#include "GenfitHarness.h"

Genfit::Genfit(LPCWSTR Directory, double* Q, double* Reflect, double* ReflErr, double* QErrors, int DatapointNum, double rholipid,double rhoh2o,double supSLD, int parratlayers, double layerlength,double surfabs, 
			   double wavelength, double subaps, double supabs, BOOL usesurfabs, double leftoffset, double Qerr, BOOL forcenorm, 
			   double forcesig, bool debug, bool XRonly, double resolution,double totallength, BOOL impnorm, int objfunc)
{

	//Needed for the unicode strings from C#
	USES_CONVERSION;


	m_Directory = W2A(Directory);
	m_dsubSLD = rhoh2o;
	m_dfilmSLD = rholipid;
	m_iparratlayers = parratlayers;
	m_dlayerlength = layerlength;
	m_dsurfabs = surfabs;
	m_dwavelength = wavelength;
	m_dsubabs = subaps;
	m_bthreadstop = false;
	m_bupdated = FALSE;
	m_bimpnorm = FALSE;
	m_icurrentiteration = 0;
	m_ipriority = 2;
	m_busesurfabs = usesurfabs;
	m_isearchalgorithm = 0;
	m_dRoughness = 3;
	m_bwarmedup = false;
	m_dleftoffset = leftoffset;
	m_dQerr = Qerr;
	m_dsupSLD = supSLD;
	m_dsupabs = supabs;
	m_dforcesig = forcesig;
	m_bdebugging = debug;
		SA = new SA_Dispatcher(debug, false, m_Directory);
	m_bforcenorm = forcenorm;
	m_bXRonly = XRonly;
	m_dresolution = resolution;
	m_dtotallength = totallength;
	m_bimpnorm = impnorm;
	objectivefunction = objfunc;
	mutex = CreateMutex(NULL,FALSE,L"SyncObj");

	
	Initialize(Q, Reflect, ReflErr, QErrors, DatapointNum);
}

Genfit::~Genfit()
{
	if(Zinc != NULL)
	{
		delete genome;
		delete SA;
		delete[] Zinc;
		delete[] Qinc;
		delete[] Rho;
		delete[] Refl;
	}
}
	
void Genfit::Initialize(double* Q, double* Reflect, double* ReflError, double* QError, int PointCount)
{
	 //////////////////////////////////////////////////////////
	 /******** Setup Variables and ReflectivityClass ********/
	 ////////////////////////////////////////////////////////


	double resolution;
	

	// Set the densities and absorbances
	m_dfilmSLD *= 1e-6 * m_dwavelength*m_dwavelength/(2.0*M_PI);
	m_dsubSLD *= 1e-6 * m_dwavelength*m_dwavelength/(2.0*M_PI);
	m_dsupSLD *= 1e-6 * m_dwavelength*m_dwavelength/(2.0*M_PI);
	double betafilm = m_dsurfabs * m_dwavelength*m_dwavelength/(2.0*M_PI);
	double betasubphase = m_dsubabs * m_dwavelength*m_dwavelength/(2.0*M_PI);
	double betasuperphase = m_dsupabs * m_dwavelength*m_dwavelength/(2.0*M_PI);

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
		ml0.totalsize = m_dtotallength; 
	else
        ml0.totalsize = m_dleftoffset + m_dlayerlength + 7 + 40;

	//Set the film property
    ml0.rho_a = m_dfilmSLD; 

	if(m_busesurfabs == TRUE)
	{
		ml0.beta_a = betafilm;
		ml0.beta_sub = betasubphase;
		ml0.beta_sup = betasuperphase;
	}
	else
	{
		ml0.beta_a = 0;
		ml0.beta_sub = 0;
		ml0.beta_sup = 0;
	}

	//Setup the genome - We start with a slightly roughened ED curve 
	genome = new GARealGenome(m_iparratlayers,m_dforcesig,m_busesurfabs, m_bimpnorm);
	genome->SetSupphase(m_dsupSLD/m_dfilmSLD);


	int actuallipidlength = (int)ceil((m_iparratlayers)*(m_dlayerlength/(m_dlayerlength+7.0)));

	for(int i = 0 ; i <= actuallipidlength; i++)
		genome->SetMutatableParameter(i,1.0);

	for(int i = actuallipidlength+1; i < m_iparratlayers; i++)
		genome->SetMutatableParameter(i,m_dsubSLD/m_dfilmSLD);	
		
	genome->SetSubphase(m_dsubSLD/m_dfilmSLD);
	
	

	ml0.m_dboxsize = (m_dlayerlength+7.0)/(m_iparratlayers);

	//Initialize the multilayer class
	ml0.init(ml0.totalsize*resolution,m_dwavelength,dz0,m_busesurfabs,m_iparratlayers, m_dleftoffset, m_bforcenorm, m_dQerr, m_bXRonly);
	ml0.SetupRef(Q, Reflect, ReflError, QError, PointCount, genome);
	ml0.objectivefunction = objectivefunction;
	ml0.m_bImpNorm = m_bimpnorm;
	
	//Set the output file names
    ml0.fnpop = m_Directory + string("\\pop.dat");
    ml0.fnrf = m_Directory + string("\\rf.dat");
    ml0.fnrho = m_Directory + string("\\rho.dat");
 
	 /////////////////////////////////////////////////////
     /******** Prepare Arrays for the Front End ********/
	////////////////////////////////////////////////////

	m_irhocount = ml0.totalsize*resolution;

	#ifndef CHECKREFLCALC
		if(m_dQerr > 0)
			m_irefldatacount = ml0.m_idatapoints;
		else
			m_irefldatacount = ml0.tarraysize;
	#else
		m_irefldatacount = ml0.m_idatapoints;
	#endif

	Zinc = new double[m_irhocount];
	Qinc = new double[m_irefldatacount];
	Rho = new double[m_irhocount];
	Refl = new double[m_irefldatacount];

	//Let the frontend know that we've set up the arrays
	m_bwarmedup = true;

	//If we have a population already, load it
	LoadFromFile(&ml0,genome, ml0.fnpop.c_str(), m_Directory);

	// Update the constraints on the genome
	genome->UpdateBoundaries(NULL,NULL);
}

int Genfit::Processing()
{

	bool accepted = false;
	//Set the thread priority
	Priority(m_ipriority);

	
	
	//.03 works well
	double mc_step=0.03;

	SA->InitializeParameters(mc_step, genome, &ml0, m_sigmasearch, m_isearchalgorithm);
	
	if(SA->CheckForFailure() == true)
	{
		MessageBox(NULL, L"Catastrophic error in SA - please contact the author", NULL,NULL);
		return -1;
	}
	 
	//Main loop
	 for(int isteps=0;(isteps < m_itotaliterations) && (m_bthreadstop == false);isteps++)
	 {
			accepted = SA->Iteration(genome);
		   
		
			if(accepted)
			{
				m_dChiSquare = ml0.m_dChiSquare;
				m_dGoodnessOfFit = ml0.m_dgoodnessoffit;
			}
		
			UpdateFits(&ml0, genome, isteps);

			//Write the population file every 5000 iterations
			if((isteps+1)%5000 == 0 || m_bthreadstop == true || isteps == m_itotaliterations-1)
			{
				WritetoFile(&ml0, genome, ml0.fnpop.c_str());
			}

			//Write out the population file for the best minimum found so far
			if(m_isearchalgorithm != 0 && SA->Get_IsIterMinimum())
				WritetoFile(&ml0, genome, (m_Directory + string("\\BestSASolution.txt")).c_str());

	 }

	//Update the arrays one last time
	UpdateFits(&ml0, genome, m_icurrentiteration);

	return 0;
}

void Genfit::UpdateFits(CReflCalc* ml, GARealGenome* genome, int currentiteration)
{
		//Check to see if we're updating
		WaitForSingleObject(mutex,INFINITE);
		
		if(m_bupdated == TRUE)
		{
			ml->genomerf(genome);
			m_dRoughness = genome->getroughness();
			

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
	

		ReleaseMutex(mutex);
}

DWORD WINAPI Genfit::InterThread(LPVOID lParam)
{
	Genfit* Internalpointer = (Genfit*)lParam;
	Internalpointer->Processing();
	return 0;
}

int Genfit::Start(int iterations)
{
	m_itotaliterations = iterations;
	DWORD dwDummy = 0;
	m_hThread = CreateThread(NULL, NULL, InterThread, this, NULL, &dwDummy);
	return 0;
}

int Genfit::Cancel()
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

int Genfit::GetData(double* Z, double* RhoOut, double* Q, double* ReflOut, double* roughness, double* chisquare, double* goodnessoffit, BOOL* isfinished)
{
	//Sleep while we are generating our output data
	if(m_icurrentiteration != m_itotaliterations)
	{
		m_bupdated = TRUE;

		while(m_bupdated == TRUE)
		{
			Sleep(20);
		}
	}

	//Stop the other thread while we are updating so the
	//data doesn't update as we're reading it
	WaitForSingleObject(mutex,INFINITE);

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

	ReleaseMutex(mutex);

	return m_icurrentiteration;
}

int Genfit::Priority(int priority)
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

void Genfit::WritetoFile(CReflCalc* ml, GARealGenome* genome, const char* filename)
{
	

	ofstream outfile;
	outfile.open(filename);
	outfile<< genome->getroughness() <<' '<< ml->beta_a*genome->getSurfAbs()*(2.*M_PI)/m_dwavelength*m_dwavelength <<
		' '<< SA->Get_Temp() <<' '<< genome->getImpNorm() << ' ' << SA->Get_AveragefSTUN() << endl;

	for(int i = 0; i < genome->RealGenomeSize(); i++)
	{
		outfile<<genome->GetRealGenome(i)<< endl;
	}

	outfile.close();
}

void Genfit::LoadFromFile(CReflCalc* ml, GARealGenome* genome,const char* filename, string fileloc )
{
   GARealGenome genome1 = *genome;
   ifstream infile(filename);
   int size = genome->RealGenomeSize();
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
   
       genome1.setroughness(roughness);

	   while(!infile.eof() && i < genome1.RealGenomeSize())
	   {
            if(infile>>ED)
			{
				if(i == 0)
					genome1.SetSupphase(ED);
				else if (i == genome1.RealGenomeSize()-1)
					genome1.SetSubphase(ED);
				else
				    genome1.SetMutatableParameter(i-1,ED);
				
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
		*genome = genome1;
		ml->beta_a = beta*m_dwavelength*m_dwavelength/(2.0*M_PI);
		SA->Set_Temp(1.0/currenttemp);
		genome->setImpNorm(normfactor);
		SA->Set_AveragefSTUN(avgfSTUN);
    }
	infile.close();

	//If we're resuming, and we were Tunneling, load the best file
	if(kk == true && string(filename).find("BestSASolution.txt") == string::npos)
	{
		LoadFromFile(ml,genome, string(fileloc+"BestSASolution.txt").c_str(), fileloc);
	}

}
