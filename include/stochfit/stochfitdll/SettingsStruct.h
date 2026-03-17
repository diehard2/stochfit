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
        int32_t UseSurfAbs;   // int32 for koffi FFI ('int' in structs.ts)
        double QErr;
        int32_t Forcenorm;
        double Forcesig;
        int32_t Debug;
        int32_t XRonly;
        int Resolution;
        double FilmLength;
        int32_t Impnorm;
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
		int32_t Adaptive;
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
	int32_t UseGpu;
	const char* Title;

};
#pragma pack(pop)

// Session state for resuming a previous SA run.
// Electron writes stochfit-session.json; on resume it calls GetRunState() to read
// raw internal values then passes this struct to Init(). nullptr = fresh start.
// filmAbsInput = m_dBeta / m_dWaveConstant (inverse of Set_FilmAbs multiplication).
// temperature  = raw m_dTemp (not 1/m_dTemp — passed directly to SetTemp()).
// surfAbs saved independently so it is not baked into filmAbsInput across save/load cycles.
#pragma pack(push, 8)
struct StochRunState {
    double roughness;
    double filmAbsInput;  // pre-multiplication value: Set_FilmAbs(filmAbsInput) → m_dBeta = filmAbsInput * WC
    double surfAbs;       // params->getSurfAbs() — saved independently
    double temperature;   // raw m_dTemp (passed directly to SetTemp())
    double impNorm;
    double avgfSTUN;
    double bestSolution;
    double chiSquare;
    double goodnessOfFit;
    int    iteration;
    double* edValues;     // Boxes+2 doubles: [supphase, box1..boxN, subphase]
    int    edCount;       // must equal Boxes+2; mismatch → fresh start
};
#pragma pack(pop)
