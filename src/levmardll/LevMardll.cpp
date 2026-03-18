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

//
// LevMardll.cpp : Defines the entry point for the DLL application.
//

#include "platform.h"
#include "LevMardll.h"
#include "FastReflCalc.h"
#include "RhoCalc.h"
#include "ParameterContainer.h"
#include <random>
#include "Settings.h"
#include <levmar/levmar.h>

extern "C" EXPORT void Rhofit(BoxReflSettings* InitStruct, double parameters[], double covariance[], int parametersize, double info[])
{
	double opts[LM_OPTS_SZ];
	double* xvec = new double[InitStruct->ZLength] ;
	double *work, *covar;

	opts[0]=LM_INIT_MU; opts[1]=1E-15; opts[2]=1E-15; opts[3]=1E-20;
	opts[4]=-LM_DIFF_DELTA; // relevant only if the finite difference jacobian version is used

	RhoCalc Rho;
	Rho.init(InitStruct);

	//Allocate a dummy array - Our real calculation is done in Refl.objective
	memset(xvec, 0, InitStruct->ZLength*sizeof(double));

	//Allocate workspace and our covariance matrix
	work=new double[((LM_DIF_WORKSZ(parametersize, InitStruct->ZLength)+parametersize*InitStruct->ZLength))];
	covar=work+LM_DIF_WORKSZ(parametersize, InitStruct->ZLength);

	if(InitStruct->UL == NULL)
		dlevmar_dif(Rho.objective, parameters, xvec, parametersize, InitStruct->ZLength, 1000, opts, info, work, covar, (void*)(&Rho));
	else
		dlevmar_bc_dif(Rho.objective, parameters, xvec, parametersize, InitStruct->ZLength, InitStruct->LL, InitStruct->UL, nullptr, 1000, opts, info, work, covar, (void*)(&Rho));


	//Calculate the standard deviations in the parameters
	for(int i = 0; i< parametersize;i++)
	{
		covariance[i] = sqrt(covar[i*(parametersize+1)]);
	}

	delete[] xvec;
	delete[] work;
}

extern "C" EXPORT void RhoGenerate(BoxReflSettings* InitStruct, double parameters[], int paramsize, double ED[], double BoxED[])
{
	RhoCalc Rho;
	Rho.init(InitStruct);
	Rho.mkdensity(parameters,paramsize);
	Rho.mkdensityboxmodel(parameters,paramsize);

	for(int i = 0; i< InitStruct->ZLength; i++)
	{
		ED[i] = Rho.nk[i];
		BoxED[i] = Rho.nkb[i];
	}
}


extern "C" EXPORT void FastReflfit(BoxReflSettings* InitStruct, double params[], double covariance[], int paramsize, double info[])
{
	//Variables
	double *work, *covar;

	FastReflcalc Refl;
	Refl.init(InitStruct);

	//Setup the fit
	//opts[4] is relevant only if the finite difference jacobian version is used
	double opts[LM_OPTS_SZ];
	opts[0]=LM_INIT_MU; opts[1]=1E-15; opts[2]=1E-15; opts[3]=1E-20; opts[4]=-LM_DIFF_DELTA;

	//Allocate a dummy array - Our real calculation is done in Refl.objective
	double* xvec = new double[InitStruct->QPoints] ;
	memset(xvec, 0, InitStruct->QPoints*sizeof(double));

	//Allocate workspace and our covariance matrix

	work=new double[((LM_DIF_WORKSZ(paramsize, InitStruct->QPoints)+paramsize*InitStruct->QPoints))];
	covar=work+LM_DIF_WORKSZ(paramsize, InitStruct->QPoints);

	if(InitStruct->UL == NULL)
		dlevmar_dif(Refl.objective,params, xvec, paramsize,InitStruct->QPoints, 1000, opts, info, work, covar,(void*)(&Refl));
	else
		dlevmar_bc_dif(Refl.objective, params, xvec,  paramsize,InitStruct->QPoints, InitStruct->LL,InitStruct->UL, nullptr, 1000, opts, info, work, covar,(void*)(&Refl));

	for(int i = 0; i< paramsize;i++)
	{
		covariance[i] = sqrt(covar[i*(paramsize+1)]);
	}

	delete[] xvec;
	delete[] work;
}


extern "C" EXPORT void StochFit(BoxReflSettings* InitStruct, double parameters[], double covararray[], int paramsize,
			double info[], double ParamArray[], double chisquarearray[], int* paramarraysize)
{
	FastReflcalc Refl;
	Refl.init(InitStruct);
	double* Reflectivity = InitStruct->Refl;
	int QSize = InitStruct->QPoints;
	double* parampercs = InitStruct->ParamPercs;

	//Setup the fit
	double opts[LM_OPTS_SZ];
	opts[0]=LM_INIT_MU; opts[1]=1E-15; opts[2]=1E-15; opts[3]=1E-20;
	opts[4]=-LM_DIFF_DELTA; // relevant only if the finite difference jacobian version is used

	//Allocate a dummy array - Our real calculation is done in Refl.objective
	double* xvec = new double[InitStruct->QPoints] ;
	for(int i = 0; i < InitStruct->QPoints; i++)
	{
		xvec[i] = 0;
	}

	//Copy starting solution
	double* origguess = new double[paramsize];
	memcpy(origguess, parameters, sizeof(double)*paramsize);

	if(InitStruct->OneSigma != 0)
		Refl.mkdensityonesigma(parameters, paramsize);
	else
		Refl.mkdensity(parameters, paramsize);

	Refl.myrfdispatch();

	double bestchisquare = 0;
	for(int i = 0; i < InitStruct->QPoints; i++)
	{
		bestchisquare += (log(Refl.reflpt[i])-log(Reflectivity[i]))*(log(Refl.reflpt[i])-log(Reflectivity[i]));
	}

	double tempinfoarray[LM_INFO_SZ];
	tempinfoarray[1] = bestchisquare;
	double* tempcovararray = new double[paramsize*paramsize];
	memset(tempcovararray, 0, sizeof(double)*paramsize*paramsize);
	ParameterContainer original(parameters, tempcovararray, paramsize,InitStruct->OneSigma,
		tempinfoarray, parampercs[6]);
	delete[] tempcovararray;

	vector<ParameterContainer> temp;
	temp.reserve(6000);

	omp_set_num_threads(omp_get_num_procs());

	#pragma omp parallel
	{
		FastReflcalc locRefl;
		locRefl.init(InitStruct);

		//Initialize random number generator
		std::mt19937 randgen(std::random_device{}() + omp_get_thread_num());
		// IRandom(max, min) mirrors the old CRandomMersenne::IRandom(double max, double min) signature
		auto IRandom = [&](double max, double min) {
			return std::uniform_real_distribution<double>(min, max)(randgen);
		};

		ParameterContainer localanswer;
		double locparameters[20];
		double locbestchisquare = bestchisquare;
		int vecsize = 1000;
		int veccount = 0;
		ParameterContainer* vec = (ParameterContainer*)malloc(vecsize*sizeof(ParameterContainer));

		double locinfo[LM_INFO_SZ];

		//Allocate workspace - these will be private to each thread

		double* work, *covar;
		work=(double*)malloc((LM_DIF_WORKSZ(paramsize, QSize)+paramsize*QSize)*sizeof(double));
		covar=work+LM_DIF_WORKSZ(paramsize, QSize);


		#pragma omp for schedule(runtime)
		for(int i = 0; i < InitStruct->Iterations;i++)
		{
			locparameters[0] = IRandom(origguess[0]*parampercs[4], origguess[0]*parampercs[5]);
			for(int k = 0; k< InitStruct->Boxes; k++)
			{
				if(InitStruct->OneSigma != 0)
				{
					locparameters[2*k+1] = IRandom(origguess[2*k+1]*parampercs[0], origguess[2*k+1]*parampercs[1]);
					locparameters[2*k+2] = IRandom(origguess[2*k+2]*parampercs[2], origguess[2*k+2]*parampercs[3]);
				}
				else
				{
					locparameters[3*k+1] = IRandom(origguess[3*k+1]*parampercs[0], origguess[3*k+1]*parampercs[1]);
					locparameters[3*k+2] = IRandom(origguess[3*k+2]*parampercs[2], origguess[3*k+2]*parampercs[3]);
					locparameters[3*k+3] = IRandom(origguess[3*k+3]*parampercs[4], origguess[3*k+3]*parampercs[5]);
				}
			}

			locparameters[paramsize-1] = origguess[paramsize-1];


			if(InitStruct->UL == NULL)
				dlevmar_dif(locRefl.objective, locparameters, xvec,  paramsize, InitStruct->QPoints, 500, opts, locinfo, work,covar,(void*)(&locRefl));
			else
				dlevmar_bc_dif(locRefl.objective, locparameters, xvec, paramsize, InitStruct->QPoints, InitStruct->LL, InitStruct->UL, nullptr,
					500, opts, locinfo, work,covar,(void*)(&locRefl));

			localanswer.SetContainer(locparameters,covar,paramsize,InitStruct->OneSigma,locinfo, parampercs[6]);

			if(locinfo[1] < bestchisquare && localanswer.IsReasonable() == true)
			{
				//Resize the private arrays if we need the space
				if(veccount+2 == vecsize)
				{
							vecsize += 1000;
							vec = (ParameterContainer*)realloc(vec,vecsize*sizeof(ParameterContainer));
				}

				bool unique = true;
				int arraysize = veccount;

				//Check if the answer already exists
				for(int i = 0; i < arraysize; i++)
				{
					if(localanswer == vec[i])
					{
						unique = false;
						i = arraysize;
					}
				}
				//If the answer is unique add it to our set of answers
				if(unique == true)
				{
					vec[veccount] = localanswer;
					veccount++;
				}
			}
		}
		#pragma omp critical (AddVecs)
		{
			for(int i = 0; i < veccount; i++)
			{
				temp.push_back(vec[i]);
			}
		}
		free(vec);
		free(work);
	}
	//
	delete[] xvec;
	delete[] origguess;

	//Sort the answers
	//Get the total number of answers
	temp.push_back(original);

	vector<ParameterContainer> allsolutions;
	allsolutions.reserve(6000);

	int tempsize = static_cast<int>(temp.size());
	allsolutions.push_back(temp[0]);

	for(int i = 1; i < tempsize; i++)
	{
		int allsolutionssize = static_cast<int>(allsolutions.size());
		for(int j = 0; j < allsolutionssize;j++)
			{
				if(temp[i] == allsolutions[j])
				{
					break;
				}
				if(j == allsolutionssize-1)
				{
					allsolutions.push_back(temp[i]);
				}
			}
	}

	if(allsolutions.size() > 0)
	{
		sort(allsolutions.begin(), allsolutions.end());
	}

	for(int i = 0; i < (int)allsolutions.size() && i < 1000 && allsolutions.size() > 0; i++)
	{
		for(int j = 0; j < paramsize; j++)
		{
			ParamArray[(i)*paramsize+j] = (allsolutions.at(i).GetParamArray())[j];
			covararray[(i)*paramsize+j] = (allsolutions.at(i).GetCovarArray())[j];
		}

		memcpy(info, allsolutions.at(i).GetInfoArray(), LM_INFO_SZ * sizeof(double));
		info += LM_INFO_SZ;

		chisquarearray[i] = (allsolutions.at(i).GetScore());
	}
	*paramarraysize = min((int)allsolutions.size(),999);
}

extern "C" EXPORT void FastReflGenerate(BoxReflSettings* InitStruct, double parameters[], int parametersize, double Reflectivity[])
{
	FastReflcalc FastRefl;
	FastRefl.init(InitStruct);

	if(InitStruct->OneSigma)
		FastRefl.mkdensityonesigma(parameters, parametersize);
	else
		FastRefl.mkdensity(parameters,parametersize);

	FastRefl.SetOffsets(0,0);
	FastRefl.myrfdispatch();

	for(int i = 0; i< InitStruct->QPoints; i++)
	{
		Reflectivity[i] = FastRefl.reflpt[i];
	}
}
