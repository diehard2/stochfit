// ReflStress.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <iomanip>
#include <tchar.h>
#include <conio.h>
#include "localstdafx.h"
#include "..\..\StochFitMain\StochFitdll\stdafx.h"
#include "..\..\StochFitMain\StochFitdll\CEDP.h"
#include "..\..\StochFitMain\StochFitdll\ReflCalc.h"
#include "..\..\StochFitMain\StochFitdll\ParamVector.h"
#include "..\..\StochFitMain\StochFitdll\SettingsStruct.h"


void SpeedTest(const CEDP* EDP, CReflCalc* Refl);
void AccuracyTest(const CEDP* EDP, CReflCalc* Refl);
void SetInitialStructure(ReflSettings* reflset);



int _tmain(int argc, _TCHAR* argv[])
{

	ReflSettings InitStruct;
	ParamVector Vec;
	CEDP EDP;
	CReflCalc Refl;

	SetInitialStructure(&InitStruct);
	Vec.Initialize(&InitStruct);
	EDP.Initialize(&InitStruct);
	Refl.Initialize(&InitStruct);

	EDP.GenerateEDP(&Vec);
	
	//AccuracyTest(&EDP, &Refl);
	SpeedTest(&EDP, &Refl);
	
	
	cout << "Press any key to exit" << endl;
	getch();

	return 0;
}

void SetInitialStructure(ReflSettings* reflset)
{
	reflset->Boxes = 40;
	reflset->Q = (double*)qrange;
	reflset->QPoints = sizeof(qrange)/sizeof(qrange[0]);
	reflset->Resolution = 3;
	reflset->SubSLD = 9.38;
	reflset->SupSLD = 0;
	reflset->FilmSLD = 14.2;
	reflset->CritEdgeOffset = 0;
	reflset->HighQOffset = 0;
	reflset->Debug = false;
	reflset->Forcenorm = false;
	reflset->Impnorm = false;
	reflset->Leftoffset = 40;
	reflset->FilmLength = 25;
	reflset->UseSurfAbs = false;
	
	reflset->QErr = 0.0;
	reflset->QError = NULL;
	reflset->DisplayQ = NULL;
	reflset->Wavelength = 1.25;
}

void SpeedTest(const CEDP* EDP, CReflCalc* Refl)
{
	__int64 start, stop, freq;
	int iterations;

	cout << "Enter the number of iterations" << endl;
	cin >> iterations;
	cin.ignore();
	cout << "\n\n*****Calculating for Full*****\n" << endl;

	QueryPerformanceCounter((LARGE_INTEGER*) &start);
	
	for(int i = 0; i < iterations ; i++)
	{
		Refl->ForceReflectivityCalc(EDP, Full);
	}

	QueryPerformanceCounter((LARGE_INTEGER*) &stop);
	QueryPerformanceFrequency((LARGE_INTEGER*) &freq);

	cout << "Iterations/sec for the full calculation =  " << iterations/((float)((stop-start)/freq)) << endl;

	cout << "\n\n*****Calculating for Opqaue*****\n" << endl;

	QueryPerformanceCounter((LARGE_INTEGER*) &start);
	
	for(int i = 0; i < iterations ; i++)
	{
		Refl->ForceReflectivityCalc(EDP, Opaque);
	}

	QueryPerformanceCounter((LARGE_INTEGER*) &stop);
	QueryPerformanceFrequency((LARGE_INTEGER*) &freq);

	cout << "Iterations/sec for the opaque calculation =  " << iterations/((float)((stop-start)/freq)) << endl;

	cout << "\n\n*****Calculating for Transparent*****\n" << endl;

	QueryPerformanceCounter((LARGE_INTEGER*) &start);
	
	for(int i = 0; i < iterations ; i++)
	{
		Refl->ForceReflectivityCalc(EDP, Transparent);
	}

	QueryPerformanceCounter((LARGE_INTEGER*) &stop);
	QueryPerformanceFrequency((LARGE_INTEGER*) &freq);

	cout << "Iterations/sec for the transparent calculation =  " << iterations/((float)((stop-start)/freq)) << endl;
}

void AccuracyTest(const CEDP* EDP, CReflCalc* Refl)
{
	bool printflag = false;
	double* FullCalc = new double[Refl->GetDataPoints()];
	double* OpaqueCalc = new double[Refl->GetDataPoints()];
	double* TransparentCalc = new double[Refl->GetDataPoints()];

	cout << "Performing accuracy test" << endl;
	cout << "Stage 1 - Checking internal consistency" << endl;
	Refl->ForceReflectivityCalc(EDP, Full);
	memcpy(FullCalc, Refl->GetReflData(), sizeof(double)*(Refl->GetDataPoints()));
	
	
	Refl->ForceReflectivityCalc(EDP, Opaque);
	memcpy(OpaqueCalc, Refl->GetReflData(), sizeof(double)*(Refl->GetDataPoints()));

	Refl->ForceReflectivityCalc(EDP, Transparent);
	memcpy(TransparentCalc, Refl->GetReflData(), sizeof(double)*(Refl->GetDataPoints()));

	for(int i = 0; i < Refl->GetDataPoints(); i++)
	{
		cout << setprecision (9) << FullCalc[i] << "  " << OpaqueCalc[i] << "  " << TransparentCalc[i] << endl;

		if(((FullCalc[i] - OpaqueCalc[i]) != 0) || ((FullCalc[i] - TransparentCalc[i])!= 0))
			printflag = true; 
	}

	if(printflag)
	{
		cout << "There was a descrepancy in the reflectivities, the reflectivities were printed out to ReflComparison.txt in the following order. Full, Opaque, Transparent" << endl;
		ofstream reflout(L"ReflComparison.xls");

		for(int i = 0;i< Refl->GetDataPoints(); i++)
		{
			reflout <<  setprecision (9) << FullCalc[i] << "  " << OpaqueCalc[i] << "  " << TransparentCalc[i] << endl;
		}
		reflout.close();
	}
	else
	{
		cout << "The reflectivities were equivalent" << endl;
	}

	//To do - Stage 2 - Check against previous data


	//To do - Stage 3 - Check against Motofit

	delete[] FullCalc;
	delete[] OpaqueCalc;
	delete[] TransparentCalc;
}
