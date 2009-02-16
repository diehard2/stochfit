// MIRefl.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CEDP.h"
#include "ReflCalc.h"

using namespace std;

void FillInitStruct(ReflSettings* Initstruct);

int _tmain(int argc, _TCHAR* argv[])
{

	CEDP EDPGen;
	CReflCalc Refl;
	ReflSettings InitStruct;
	int calculations = 10;
	double SLD[4];
	double Thick[4];

	SLD[0] = 0.0;
	SLD[1] = 8.911;
	SLD[2] = 13.5072;
	SLD[3] = 9.38;

	Thick[0] = 0.0;
	Thick[1] = 16.0;
	Thick[2] = 10.0;
	Thick[3] = 0.0;


	//Setup Parameters
	FillInitStruct(&InitStruct);
	

	Refl.init(&InitStruct);
	EDPGen.Init(&InitStruct, SLD, Thick, 3.15);



	int t_on = clock();
	
	
	
	for(int i = 0; i < calculations; i++)
	{
		EDPGen.GenerateEDP();
		Refl.CalculateReflectivity(&EDPGen);
	}

	int t_off = clock();

	cout << calculations << " calculations in: " << (((static_cast<float>(t_off - t_on))/(CLOCKS_PER_SEC)))*(1000000) << " microseconds\n\n";
	cout << calculations/(((static_cast<float>(t_off - t_on))/(CLOCKS_PER_SEC))) << " calcuations per second\n\n";
	cout << "1 calculation in: " << (((static_cast<float>(t_off - t_on))/(CLOCKS_PER_SEC)))*(1E6)/calculations << " microseconds\n\n";
	EDPGen.WriteOutputFile(L"EDP.txt");
	Refl.ParamsRF(&EDPGen, L"refl.txt");

	getch();

	return 0;
}

void FillInitStruct(ReflSettings* InitStruct)
{
	InitStruct->Wavelength = 1.24;
	InitStruct->Forcenorm = FALSE;
	InitStruct->QErr = 0;
	InitStruct->XRonly = FALSE;
	InitStruct->Impnorm = FALSE;
	InitStruct->Q = const_cast<double*>(qrange);
	InitStruct->CritEdgeOffset = 0;
	InitStruct->HighQOffset = 0;

	InitStruct->Totallength = 0;
	InitStruct->UseSurfAbs = FALSE;
	InitStruct->SupAbs = 0;
	InitStruct->SubAbs = 0;
	InitStruct->FilmAbs = 0;
	InitStruct->Boxes = 2;
	InitStruct->Leftoffset = 40;
	InitStruct->FilmLength = 26;
	InitStruct->Resolution = 10;
	InitStruct->FilmSLD = 9.38;
	InitStruct->QPoints = (sizeof qrange / sizeof qrange[0]);

	InitStruct->QError = NULL;
	InitStruct->Refl = NULL;
	InitStruct->ReflError = NULL;
	

}