// MIRefl.cpp : Defines the entry point for the console application.
//

#include <stochfit/common/platform.h>
#include "CEDP.h"
#include "ReflCalc.h"
#include "ParamVector.h"

void FillInitStruct(ReflSettings* Initstruct);

// Test Q-range (Angstrom^-1), 100 points from 0.02 to 0.60
static const double qrange[] = {
    0.020, 0.026, 0.032, 0.038, 0.044, 0.050, 0.056, 0.062, 0.068, 0.074,
    0.080, 0.086, 0.092, 0.098, 0.104, 0.110, 0.116, 0.122, 0.128, 0.134,
    0.140, 0.146, 0.152, 0.158, 0.164, 0.170, 0.176, 0.182, 0.188, 0.194,
    0.200, 0.206, 0.212, 0.218, 0.224, 0.230, 0.236, 0.242, 0.248, 0.254,
    0.260, 0.266, 0.272, 0.278, 0.284, 0.290, 0.296, 0.302, 0.308, 0.314,
    0.320, 0.326, 0.332, 0.338, 0.344, 0.350, 0.356, 0.362, 0.368, 0.374,
    0.380, 0.386, 0.392, 0.398, 0.404, 0.410, 0.416, 0.422, 0.428, 0.434,
    0.440, 0.446, 0.452, 0.458, 0.464, 0.470, 0.476, 0.482, 0.488, 0.494,
    0.500, 0.506, 0.512, 0.518, 0.524, 0.530, 0.536, 0.542, 0.548, 0.554,
    0.560, 0.566, 0.572, 0.578, 0.584, 0.590, 0.594, 0.596, 0.598, 0.600
};

int main(int argc, char* argv[])
{

	CEDP EDPGen;
	CReflCalc Refl;
	ReflSettings InitStruct = {};
	int calculations = 10;

	// Normalized SLD values (divided by FilmSLD = 9.38)
	// Superphase (air): 0.0, Box1: 8.911, Box2: 13.5072, Subphase: 9.38
	double FilmSLD = 9.38;
	double SLD[] = {0.0, 8.911, 13.5072, 9.38};

	//Setup Parameters
	FillInitStruct(&InitStruct);

	// Build a ParamVector and populate it with normalized SLD values
	ParamVector params(&InitStruct);
	for(int i = 0; i < InitStruct.Boxes; i++)
		params.SetMutatableParameter(i, SLD[i+1] / FilmSLD);
	params.setroughness(3.15);

	Refl.Init(&InitStruct);
	EDPGen.Init(&InitStruct);

	int t_on = clock();

	for(int i = 0; i < calculations; i++)
	{
		EDPGen.GenerateEDP(&params);
		Refl.CalculateReflectivity(&EDPGen);
	}

	int t_off = clock();

	cout << calculations << " calculations in: " << (((static_cast<float>(t_off - t_on))/(CLOCKS_PER_SEC)))*(1000000) << " microseconds\n\n";
	cout << calculations/(((static_cast<float>(t_off - t_on))/(CLOCKS_PER_SEC))) << " calcuations per second\n\n";
	cout << "1 calculation in: " << (((static_cast<float>(t_off - t_on))/(CLOCKS_PER_SEC)))*(1E6)/calculations << " microseconds\n\n";
	EDPGen.WriteOutputFile("EDP.txt");
	Refl.ParamsRF(&EDPGen, "refl.txt");

	return 0;
}

void FillInitStruct(ReflSettings* InitStruct)
{
	InitStruct->Wavelength = 1.24;
	InitStruct->Forcenorm = 0;
	InitStruct->QErr = 0;
	InitStruct->XRonly = 0;
	InitStruct->Impnorm = 0;
	InitStruct->Q = const_cast<double*>(qrange);
	InitStruct->CritEdgeOffset = 0;
	InitStruct->HighQOffset = 0;

	InitStruct->UseSurfAbs = 0;
	InitStruct->SupAbs = 0;
	InitStruct->SubAbs = 0;
	InitStruct->FilmAbs = 0;
	InitStruct->Boxes = 2;
	InitStruct->FilmLength = 26;
	InitStruct->Resolution = 10;
	InitStruct->FilmSLD = 9.38;
	InitStruct->SupSLD = 0.0;
	InitStruct->SubSLD = 9.38;
	InitStruct->QPoints = (sizeof qrange / sizeof qrange[0]);
	InitStruct->Forcesig = 0;
	InitStruct->Objectivefunction = 0;

	InitStruct->QError = NULL;
	InitStruct->Refl = NULL;
	InitStruct->ReflError = NULL;
}
