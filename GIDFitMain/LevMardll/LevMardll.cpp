// LevMardll.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "LevMardll.h"
#include "GIDCalc.h"


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

//Perform a LS fit of the GID data
extern "C" LEVMARDLL_API double GIDFit(int numberofGID, double parameters[], int paramsize, double QRange[], int QSize, double GIDpoints[], int GIDsize,
										double IndividGraphs[], int IndividGraphslength, double covariance[], int covarsize, double GIDRealPoints[], double GIDErrors[], double info[])
{
	double ChiSquare = 0;
	double opts[LM_OPTS_SZ];
	double* xvec = new double[QSize] ;
	double *work, *covar;
	int ret;

	opts[0]=LM_INIT_MU; opts[1]=1E-15; opts[2]=1E-15; opts[3]=1E-20;
	opts[4]=-LM_DIFF_DELTA; // relevant only if the finite difference jacobian version is used 

	GIDCalc GID;
	GID.Init(numberofGID,QRange,QSize,GIDRealPoints,GIDErrors,GIDpoints,IndividGraphs,parameters);

	//Allocate a dummy array - Our real calculation is done in Refl.objective
	for(int i = 0; i < QSize; i++)
	{
		xvec[i] = 0;
	}

	//Allocate workspace and our covariance matrix
	work=new double[((LM_DIF_WORKSZ(paramsize, QSize)+paramsize*QSize))];
	covar=work+LM_DIF_WORKSZ(paramsize, QSize);

	//Perform the fit
	ret = dlevmar_dif(GID.objective, parameters, xvec,  paramsize,QSize, 500, opts, info, work, covar,(void*)(&GID)); 
	
	//Make sure the true parameters are used to update our arrays
	GID.MakeGID(parameters, paramsize);

	//Calculate ChiSquare
	ChiSquare = GID.CalcChiSquare();
	ChiSquare /= (double)(QSize-paramsize);

	//Calculate the standard deviations in the parameters
	for(int i = 0; i< paramsize;i++)
	{
		covariance[i] = sqrt(covar[i*(paramsize+1)]);
	}
	
	//Make output files
	GID.writefiles("rhofit.dat");
	
	//Cleanup the arrays
	delete[] xvec;
	delete[] work;

	return ChiSquare;
}


//Calculate the GID Profile from Gaussian, Lorentzian, or Voigt distributions
extern "C" LEVMARDLL_API void GIDGenerate(int numberofGID, double parameters[], int paramsize, double QRange[], int QSize, double GIDpoints[], int GIDsize, double IndividGraphs[], int IndividGraphslength)
{
	GIDCalc GID;
	GID.Init(numberofGID, QRange, QSize, NULL, NULL,GIDpoints,IndividGraphs,parameters);
	GID.MakeGID(parameters, paramsize);
}
