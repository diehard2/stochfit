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

#include "stdafx.h"
#include "LevMardll.h"
#include "FastReflCalc.h"
#include "RhoCalc.h"
#include "ParameterContainer.h"
#include "randomc.h"
#include "settings.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

extern "C" LEVMARDLL_API double Rhofit(LPCWSTR directory, int boxes, double SLD, double SupSLD, double parameters[], int paramsize,
			double ZRange[], int ZSize, double ED[], int EDsize, double covariance[],
			int covarsize, double info[], int infosize, BOOL onesigma)
{
	USES_CONVERSION;
	string direc = W2A(directory);

	double ChiSquare = 0;
	double opts[LM_OPTS_SZ];
	double* xvec = new double[ZSize] ;
	double *work, *covar;
	int ret;

	opts[0]=LM_INIT_MU; opts[1]=1E-15; opts[2]=1E-15; opts[3]=1E-20;
	opts[4]=-LM_DIFF_DELTA; // relevant only if the finite difference jacobian version is used 

	RhoCalc Rho;
	Rho.init(boxes,SLD,SupSLD,ED,ZRange,ZSize,onesigma);

	//Allocate a dummy array - Our real calculation is done in Refl.objective
	for(int i = 0; i < ZSize; i++)
	{
		xvec[i] = 0;
	}

	//Allocate workspace and our covariance matrix
	work=new double[((LM_DIF_WORKSZ(paramsize, ZSize)+paramsize*ZSize))];
	covar=work+LM_DIF_WORKSZ(paramsize, ZSize);

	ret = dlevmar_dif(Rho.objective, parameters, xvec,  paramsize,ZSize, 1000, opts, info, work, covar,(void*)(&Rho)); 
	
	//Calculate the current density
	if(onesigma == TRUE)
	{
		Rho.mkdensityonesigma(parameters,paramsize);
		Rho.mkdensityboxmodel(parameters,paramsize);
	}
	else
	{
		Rho.mkdensity(parameters, paramsize);
		Rho.mkdensityboxmodel(parameters,paramsize);
	}

	//Calculate ChiSquare
	for(int i = 0; i< ZSize;i++)
	{
		if(ED[i] != 0)
			ChiSquare += (ED[i]-Rho.nk[i])*(ED[i]-Rho.nk[i])/ED[i];
	}
	//ChiSquare /= ZSize-paramsize;

	//Calculate the standard deviations in the parameters
	for(int i = 0; i< paramsize;i++)
	{
		covariance[i] = sqrt(covar[i*(paramsize+1)]);
	}
	
	//Make output files
	Rho.writefiles(string(direc + string("\\rhofit.dat")).c_str());
	
	delete xvec;
	delete work;

	return ChiSquare;
}

extern "C" LEVMARDLL_API void RhoGenerate(int boxes, double SLD, double SupSLD, double parameters[], int paramsize,
			double ZRange[], int ZSize, double ED[], double BoxED[], int EDsize, BOOL onesigma)
{
	RhoCalc Rho;
	Rho.init(boxes,SLD,SupSLD,NULL,ZRange,ZSize,onesigma);
	
	if(onesigma == TRUE)
		Rho.mkdensityonesigma(parameters, paramsize);
	else
		Rho.mkdensity(parameters,paramsize);

	Rho.mkdensityboxmodel(parameters,paramsize);

	for(int i = 0; i< ZSize; i++)
	{
		ED[i] = Rho.nk[i];
		BoxED[i] = Rho.nkb[i];
	}
}


extern "C" LEVMARDLL_API double FastReflfit(BoxReflSettings* InitStruct, double params[], double covariance[], int paramsize, 
			double info[], int infosize)
{
	USES_CONVERSION;

	//Variables
	double *work, *covar;
	double ChiSquare = 0;
	double Qc = 0;
	double calcholder = 0;

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
	Refl.SetOffsets(0,0);

	if(InitStruct->UL == NULL)
		dlevmar_dif(Refl.objective,params, xvec, paramsize,InitStruct->QPoints, 1000, opts, info, work, covar,(void*)(&Refl)); 
	else
		dlevmar_bc_dif(Refl.objective, params, xvec,  paramsize,InitStruct->QPoints, 
			InitStruct->LL,InitStruct->UL,1000, opts, info, work, covar,(void*)(&Refl)); 
	
	
	Refl.myrfdispatch();

   	
	for(int i = 0; i< InitStruct->QPoints;i++)
	{
		calcholder = (InitStruct->Refl[i]-Refl.reflpt[i]);
		ChiSquare += calcholder*calcholder/InitStruct->ReflError[i];
	}
	ChiSquare /= InitStruct->QPoints-paramsize;
	
	for(int i = 0; i< paramsize;i++)
	{
		covariance[i] = sqrt(covar[i*(paramsize+1)]);
	}

	Qc = Refl.CalcQc(InitStruct->SubSLD);

	if(InitStruct->WriteFiles == TRUE)
	{
		double ZInc[500];
		double nk[500];
		double nkb[500];
		double length = 50;

		//Calculate the length
		for(int i = 1; i <= Refl.boxnumber; i++)
		{
			length += Refl.LengthArray[i];
		}
		for(int i = 0; i<500; i++)
		{
			ZInc[i] = i*length/500.0;
		}

		Refl.Rhocalculate(30,ZInc,Refl.LengthArray,Refl.RhoArray,Refl.SigmaArray,nk,nkb,500);
		
		
		
		//Make output files

		std::ofstream outrhofile(string(W2A(InitStruct->Directory) + string("\\reflrhofit.dat")).c_str());
		for(int i = 0; i<500;i++)
		{
			outrhofile<<ZInc[i] << ' ' << nk[i]/nk[499] << ' ' << nkb[i]/nkb[499] << std::endl;
		}
		outrhofile.close();
		
		
		std::ofstream outreflfile(string(W2A(InitStruct->Directory) + string("\\reflfile.dat")).c_str());
		for(int i = 0; i<InitStruct->QPoints;i++)
		{
			outreflfile << InitStruct->Q[i] << ' ' << Refl.reflpt[i] << ' ' << InitStruct->Q[i]/Qc << ' ' <<
				Refl.reflpt[i]/Refl.CalcFresnelPoint(InitStruct->Q[i],Qc)<<std::endl;
		}
		outreflfile.close();
	}


	delete[] xvec;
	delete[] work;

	return ChiSquare;
}

//
//extern "C" LEVMARDLL_API void StochFit(int boxes, double SLD, double SupSLD, double wavelength, double parameters[], int paramsize,
//			double QRange[], double QError[], int QSize, double Reflectivity[], int reflectivitysize, double Errors[],double covariance[], int covarsize, 
//			double info[], int infosize, BOOL onesigma,BOOL writefiles, int iterations, double ParamArray[], int* paramarraysize, double parampercs[], double chisquarearray[], double covararray[],
//			double QSpread, BOOL Impnorm)
//{
//	FastReflcalc Refl;
//	Refl.init(wavelength,boxes,SLD,SupSLD,parameters,paramsize, Reflectivity, Errors, reflectivitysize, (bool)onesigma, QSpread,1.0, Impnorm, 0, 0);
//	Refl.Realreflerrors = Errors;
//	Refl.MakeTheta(QRange, QError,QSize);
//	
//	//Setup the fit
//
//	double opts[LM_OPTS_SZ];
//	opts[0]=LM_INIT_MU; opts[1]=1E-15; opts[2]=1E-15; opts[3]=1E-20;
//	opts[4]=-LM_DIFF_DELTA; // relevant only if the finite difference jacobian version is used 
//	
//	//Allocate a dummy array - Our real calculation is done in Refl.objective
//	double* xvec = new double[QSize] ;
//	for(int i = 0; i < QSize; i++)
//	{
//		xvec[i] = 0;
//	}
//	//Copy starting solution
//	double* origguess = new double[paramsize];
//	memcpy(origguess, parameters, sizeof(double)*paramsize);
//
//	//Starting solution
//	//dlevmar_dif(Refl.objective, parameters, xvec,  paramsize,QSize, 1000, opts, info, NULL, NULL,(void*)(&Refl)); 
//	if(onesigma == true)
//		Refl.mkdensityonesigma(parameters,paramsize);
//	else
//		Refl.mkdensity(parameters, paramsize);
//
//	Refl.myrfdispatch();
//
//	double bestchisquare = 0;
//	for(int i = 0; i < reflectivitysize; i++)
//	{
//		bestchisquare += (log(Refl.reflpt[i])-log(Reflectivity[i]))*(log(Refl.reflpt[i])-log(Reflectivity[i]));
//	}
//
//	double* tempcovararray = new double[paramsize*paramsize];
//	memset(tempcovararray,0.0, sizeof(double)*paramsize*paramsize);
//	ParameterContainer original(parameters, tempcovararray, paramsize,onesigma,bestchisquare,parampercs[6]);
//	delete[] tempcovararray;
//
//	vector<ParameterContainer> temp;
//	temp.reserve(6000);
//
//	omp_set_num_threads(omp_get_num_procs());
//
//#pragma omp parallel
//{
//	FastReflcalc locRefl;
//	locRefl.init(wavelength,boxes,SLD,SupSLD,parameters,paramsize, Reflectivity, Errors, reflectivitysize, (bool)onesigma, QSpread,1.0, Impnorm, 0, 0);
//	locRefl.Realreflerrors = Errors;
//	locRefl.MakeTheta(QRange, QError, QSize);
//
//	//Initialize random number generator
//	int seed = time_seed();
//	CRandomMersenne randgen(time_seed()+omp_get_thread_num());
//
//	ParameterContainer localanswer;
//	double locparameters[20];
//    double locbestchisquare = bestchisquare;
//	double bestparam[20];
//	int vecsize = 1000;
//	int veccount = 0;
//	ParameterContainer* vec = (ParameterContainer*)malloc(vecsize*sizeof(ParameterContainer));
//	
//	double locinfo[9];
//
//	//Allocate workspace - these will be private to each thread
//
//	double* work, *covar;
//	work=(double*)malloc((LM_DIF_WORKSZ(paramsize, QSize)+paramsize*QSize)*sizeof(double));
//	covar=work+LM_DIF_WORKSZ(paramsize, QSize);
//
//
//	#pragma omp for schedule(runtime)
//	for(int i = 0; i<iterations;i++) 
//	{
//		locparameters[0] = randgen.IRandom(origguess[0]*parampercs[4], origguess[0]*parampercs[5]);
//		for(int k = 0; k<boxes; k++)
//		{
//			if(onesigma == TRUE)
//			{
//				locparameters[2*k+1] = randgen.IRandom(origguess[2*k+1]*parampercs[0], origguess[2*k+1]*parampercs[1]);
//				locparameters[2*k+2] = randgen.IRandom(origguess[2*k+2]*parampercs[2], origguess[2*k+2]*parampercs[3]);
//			}
//			else
//			{
//				locparameters[3*k+1] = randgen.IRandom(origguess[3*k+1]*parampercs[0], origguess[3*k+1]*parampercs[1]);
//				locparameters[3*k+2] = randgen.IRandom(origguess[3*k+2]*parampercs[2], origguess[3*k+2]*parampercs[3]);
//				locparameters[3*k+3] = randgen.IRandom(origguess[3*k+3]*parampercs[4], origguess[3*k+3]*parampercs[5]);
//			}
//		}
//
//		locparameters[paramsize-1] = origguess[paramsize-1];
//		
//		
//		
//		dlevmar_dif(locRefl.objective, locparameters, xvec,  paramsize,QSize, 500, opts, locinfo, work,covar,(void*)(&locRefl)); 
//		
//		localanswer.SetContainer(locparameters,covar,paramsize,onesigma,locinfo[1], parampercs[6]);
//
//		if(locinfo[1] < bestchisquare && localanswer.IsReasonable() == true)
//		{
//			//Resize the private arrays if we need the space
//			if(veccount+2 == vecsize)
//			{
//						vecsize += 1000;
//						vec = (ParameterContainer*)realloc(vec,vecsize*sizeof(ParameterContainer));
//			}
//
//			bool unique = true;
//			int arraysize = veccount;
//
//			//Check if the answer already exists
//			for(int i = 0; i < arraysize; i++)
//			{
//				if(localanswer == vec[i])
//				{
//					unique = false; 
//					i = arraysize;
//				}
//			}
//			//If the answer is unique add it to our set of answers
//			if(unique == true)
//			{
//				vec[veccount] = localanswer;
//				veccount++;
//			}
//		}
//	}
//	#pragma omp critical (AddVecs)
//	{
//		for(int i = 0; i < veccount; i++)
//		{
//			temp.push_back(vec[i]);
//		}
//	}
//	free(vec);
//	free(work);
//}
//
//delete[] xvec;
//delete[] origguess;
//
////Sort the answers
////Get the total number of answers
//temp.push_back(original);
//
//vector<ParameterContainer> allsolutions;
//allsolutions.reserve(6000);
//
//int tempsize = temp.size();
//allsolutions.push_back(temp[0]);
//
//for(int i = 1; i < tempsize; i++)
//{
//	int allsolutionssize = allsolutions.size();
//	for(int j = 0; j < allsolutionssize;j++)
//		{
//			if(temp[i] == allsolutions[j])
//			{
//				break;
//			}
//			if(j == allsolutionssize-1)
//			{
//				allsolutions.push_back(temp[i]);
//			}
//		}
//}
//
//if(allsolutions.size() > 0)
//{
//	sort(allsolutions.begin(), allsolutions.end());
//}
//
//for(int i = 0; i < allsolutions.size() && i < 1000 && allsolutions.size() > 0; i++)
//{
//	for(int j = 0; j < paramsize; j++)
//	{
//		ParamArray[(i)*paramsize+j] = (allsolutions.at(i).GetParamArray())[j];
//		covararray[(i)*paramsize+j] = (allsolutions.at(i).GetCovarArray())[j];
//	}
//	chisquarearray[i] = (allsolutions.at(i).GetScore());
//}
//*paramarraysize = min(allsolutions.size(),999);
//}
//
//extern "C" LEVMARDLL_API void ConstrainedStochFit(int boxes, double SLD, double SupSLD, double wavelength, double parameters[], int paramsize,
//			double QRange[], double QError[], int QSize, double Reflectivity[], int reflectivitysize, double Errors[],double covariance[], int covarsize, 
//			double info[], int infosize, BOOL onesigma,BOOL writefiles, int iterations, double ParamArray[], int* paramarraysize, double parampercs[], double chisquarearray[], double covararray[],
//			double UL[], double LL[], double QSpread, BOOL ImpNorm)
//{
//	FastReflcalc Refl;
//	Refl.init(wavelength,boxes,SLD,SupSLD,parameters,paramsize, Reflectivity, Errors, reflectivitysize, (bool)onesigma,
//		QSpread,1.0,ImpNorm, 0, 0);
//	Refl.Realreflerrors = Errors;
//	Refl.MakeTheta(QRange,QError, QSize);
//	
//	//Setup the fit
//
//	double opts[LM_OPTS_SZ];
//	opts[0]=LM_INIT_MU; opts[1]=1E-15; opts[2]=1E-15; opts[3]=1E-20;
//	opts[4]=-LM_DIFF_DELTA; // relevant only if the finite difference jacobian version is used 
//	
//	//Allocate a dummy array - Our real calculation is done in Refl.objective
//	double* xvec = new double[QSize] ; 
//	for(int i = 0; i < QSize; i++)
//	{
//		xvec[i] = 0;
//	}
//	//Copy starting solution
//	double* origguess = new double[paramsize];
//	memcpy(origguess, parameters, sizeof(double)*paramsize);
//
//	//Starting solution
//	//dlevmar_bc_dif(Refl.objective, parameters, xvec,  paramsize,QSize, LL, UL,100,  opts, info, NULL,NULL,(void*)(&Refl)); 
//	
//	if(onesigma == true)
//		Refl.mkdensityonesigma(parameters,paramsize);
//	else
//		Refl.mkdensity(parameters, paramsize);
//
//	Refl.myrfdispatch();
//
//	double bestchisquare = 0;
//	for(int i = 0; i < reflectivitysize; i++)
//	{
//		bestchisquare += (log(Refl.reflpt[i])-log(Reflectivity[i]))*(log(Refl.reflpt[i])-log(Reflectivity[i]));
//	}
//
//	double* tempcovararray = new double[paramsize*paramsize];
//	memset(tempcovararray,0.0, sizeof(double)*paramsize*paramsize);
//	ParameterContainer original(parameters, tempcovararray, paramsize,onesigma,bestchisquare, parampercs[6]);
//	delete[] tempcovararray;
//
//	vector<ParameterContainer> temp;
//	temp.reserve(6000);
//
//	omp_set_num_threads(omp_get_num_procs());
//
//#pragma omp parallel
//{
//	FastReflcalc locRefl;
//	locRefl.init(wavelength,boxes,SLD,SupSLD,parameters,paramsize, Reflectivity, Errors, reflectivitysize, (bool)onesigma,
//		QSpread,1.0,ImpNorm, 0, 0);
//	locRefl.Realreflerrors = Errors;
//	locRefl.MakeTheta(QRange, QError, QSize);
//	
//
//	//Initialize random number generator
//	int seed = time_seed();
//	CRandomMersenne randgen(time_seed()+omp_get_thread_num());
//
//	ParameterContainer localanswer;
//	double locparameters[20];
//    double locbestchisquare = bestchisquare;
//	double bestparam[20];
//	int vecsize = 1000;
//	int veccount = 0;
//	ParameterContainer* vec = (ParameterContainer*)malloc(vecsize*sizeof(ParameterContainer)*50);
//	
//	double locinfo[9];
//
//	//Allocate workspace - these will be private to each thread
//
//	double* work, *covar;
//	work=new double[((LM_DIF_WORKSZ(paramsize, QSize)+paramsize*QSize))];
//	covar=work+LM_DIF_WORKSZ(paramsize, QSize);
//	int threadnum = omp_get_thread_num();
//
//	#pragma omp for schedule(runtime)
//	for(int i = 0; i<iterations;i++) 
//	{
//		
//		//Permute the answer
//		locparameters[0] = randgen.IRandom(origguess[0]*parampercs[4], origguess[0]*parampercs[5]);
//		for(int k = 0; k<boxes; k++)
//		{
//			if(onesigma == TRUE)
//			{
//				locparameters[2*k+1] = randgen.IRandom(origguess[2*k+1]*parampercs[0], origguess[2*k+1]*parampercs[1]);
//				locparameters[2*k+2] = randgen.IRandom(origguess[2*k+2]*parampercs[2], origguess[2*k+2]*parampercs[3]);
//			}
//			else
//			{
//				locparameters[3*k+1] = randgen.IRandom(origguess[3*k+1]*parampercs[0], origguess[3*k+1]*parampercs[1]);
//				locparameters[3*k+2] = randgen.IRandom(origguess[3*k+2]*parampercs[2], origguess[3*k+2]*parampercs[3]);
//				locparameters[3*k+3] = randgen.IRandom(origguess[3*k+3]*parampercs[4], origguess[3*k+3]*parampercs[5]);
//			}
//		}
//		locparameters[paramsize-1] = origguess[paramsize-1];
//
//		dlevmar_bc_dif(locRefl.objective, locparameters, xvec,  paramsize,QSize, LL, UL,500,  opts, locinfo, work,covar,(void*)(&locRefl)); 
//		localanswer.SetContainer(locparameters,covar,paramsize,onesigma,locinfo[1], parampercs[6]);
//
//		if(locinfo[1] < bestchisquare && localanswer.IsReasonable() == true)
//		{
//			//Resize the private arrays if we need the space
//			if(veccount+2 == vecsize)
//			{
//						vecsize += 1000;
//						vec = (ParameterContainer*)realloc(vec,vecsize*sizeof(ParameterContainer));
//			}
//
//			bool unique = true;
//			int arraysize = veccount;
//
//			//Check if the answer already exists
//			for(int i = 0; i < arraysize; i++)
//			{
//				if(localanswer == vec[i])
//				{
//					unique = false; 
//					i = arraysize;
//				}
//			}
//			//If the answer is unique add it to our set of answers
//			if(unique == true)
//			{
//				vec[veccount] = localanswer;
//				veccount++;
//			}
//		}
//	}
//#pragma omp critical
//	{
//		for(int i = 0; i < veccount; i++)
//		{
//			temp.push_back(vec[i]);
//		}
//	}
//	free(vec);
//	delete(work);
//}
//
//delete[] xvec;
//delete[] origguess;
//
////Sort the answers
////Get the total number of answers
//temp.push_back(original);
//
//vector<ParameterContainer> allsolutions;
//allsolutions.reserve(6000);
//
//int tempsize = temp.size();
//allsolutions.push_back(temp[0]);
//
//
//for(int i = 1; i < tempsize; i++)
//{
//	int allsolutionssize = allsolutions.size();
//	for(int j = 0; j < allsolutionssize;j++)
//		{
//			if(temp[i] == allsolutions[j])
//			{
//				break;
//			}
//			if(j == allsolutionssize-1)
//			{
//				allsolutions.push_back(temp[i]);
//			}
//		}
//}
//
//
//if(allsolutions.size() > 0)
//{
//	sort(allsolutions.begin(), allsolutions.end());
//}
//
//
//
//for(int i = 0; i < allsolutions.size() && i < 1000 && allsolutions.size() > 0; i++)
//{
//	for(int j = 0; j < paramsize; j++)
//	{
//		ParamArray[(i)*paramsize+j] = (allsolutions.at(i).GetParamArray())[j];
//		covararray[(i)*paramsize+j] = (allsolutions.at(i).GetCovarArray())[j];
//	}
//	chisquarearray[i] = (allsolutions.at(i).GetScore());
//
//}
//*paramarraysize = min(allsolutions.size(),999);
//}
////
extern "C" LEVMARDLL_API void FastReflGenerate(BoxReflSettings* InitStruct, double parameters[], int parametersize, double Reflectivity[])
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

