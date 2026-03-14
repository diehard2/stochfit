#pragma once

// FFI-compatible settings struct for SA fitting.
// Uses #pragma pack(push, 8) for koffi interop — the JS side must explicitly
// add padding fields between misaligned members (e.g., 4-byte UseGpu before
// 8-byte pointer Title). Algorithm: 0=Greedy, 1=SA, 2=STUN. UseGpu enables
// GPU-accelerated SA when a compatible device is available.

#include <stochfit/common/platform.h>

#pragma pack(push, 8)
struct ReflSettings
{
		const char* Directory;
        double* Q;
        double* Refl;
        double* ReflError;
        double* QError;
        int QPoints;
        double SubSLD;
        double FilmSLD;
        double SupSLD;
        int Boxes;
        double FilmAbs;
        double SubAbs;
        double SupAbs;
        double Wavelength;
        BOOL UseSurfAbs;
        double Leftoffset;
        double QErr;
        BOOL Forcenorm;
        double Forcesig;
        BOOL Debug;
        BOOL XRonly;
        int Resolution;
        double Totallength;
        double FilmLength;
        BOOL Impnorm;
        int Objectivefunction;
		double Paramtemp;

		//Annealing parameters
		int Sigmasearch;
		int NormalizationSearchPerc;
        int AbsorptionSearchPerc;
		int Algorithm;
		double Inittemp;
		int Platiter;
		double Slope;
		double Gamma;
		int STUNfunc;
		BOOL Adaptive;
		int Tempiter;
		int STUNdeciter;
		double Gammadec;

		int CritEdgeOffset;
		int HighQOffset;
		//Not used
		int Iterations;
		int IterationsCompleted;
		double ChiSquare;

	// GPU acceleration control
	BOOL UseGpu;
	const char* Title;

};
#pragma pack(pop)
