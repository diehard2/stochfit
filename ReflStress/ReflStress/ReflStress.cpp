// ReflStress.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <tchar.h>
#include <conio.h>
#include "localstdafx.h"
#include "..\..\StochFitMain\StochFitdll\stdafx.h"
#include "..\..\StochFitMain\StochFitdll\CEDP.h"
#include "..\..\StochFitMain\StochFitdll\ReflCalc.h"
#include "..\..\StochFitMain\StochFitdll\ParamVector.h"
#include "..\..\StochFitMain\StochFitdll\SettingsStruct.h"

void SetInitialStructure(ReflSettings* reflset);
void SetParamVector(ParamVector* vec);


int _tmain(int argc, _TCHAR* argv[])
{
	MyComplex test;
	test.re = 1.0;
	test.im = 1.0;
	MyComplex temp = compsqrt(test);

	__int64 start, stop, freq;
	ReflSettings InitStruct;
	ParamVector Vec;
	CEDP EDP;
	CReflCalc Refl;

	SetInitialStructure(&InitStruct);
	Vec.Initialize(&InitStruct);
	EDP.Initialize(&InitStruct);
	Refl.Initialize(&InitStruct);


	EDP.GenerateEDP(&Vec);
	

	int iterations;
	char ch = '0';
	cout << "Enter the number of iterations" << endl;
	cin >> iterations;
	cin.ignore();
	cout << "Calculating" << endl;
	EDP.GenerateEDP(&Vec);
	QueryPerformanceCounter((LARGE_INTEGER*) &start);
	for(int i = 0; i < iterations ; i++)
	{
		
		Refl.MakeReflectivity(&EDP);
	}
	QueryPerformanceCounter((LARGE_INTEGER*) &stop);
	QueryPerformanceFrequency((LARGE_INTEGER*) &freq);

	cout << "Iterations/sec =  " << iterations/((float)((stop-start)/freq)) << endl;
	Refl.WriteOutputFile(L"Refl.txt");
	cout << "\b \b";
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

void SetParamVector(ParamVector* vec)
{

}