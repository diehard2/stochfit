#pragma once

// Settings structs for SA fitting and session restore.
// ReflSettings: passed by const reference into all C++ classes.
// StochRunState: optional resume state passed to StochFit constructor (nullptr = fresh start).

#include "platform.h"
#include <span>
#include <string_view>

struct ReflSettings
{
	std::string_view Directory;
	std::span<const double> Q;
	std::span<const double> Refl;
	std::span<const double> ReflError;
	std::span<const double> QError;
	// Q.size() replaces the old QPoints field
	double SubSLD;
	double FilmSLD;
	double SupSLD;
	int Boxes;
	double FilmAbs;
	double SubAbs;
	double SupAbs;
	double Wavelength;
	bool UseSurfAbs;
	double QErr;
	bool Forcenorm;
	double Forcesig;
	double RoughnessMax = 8.0; // upper bound for roughness search; determines EDP padding (6× this value)
	bool Debug;
	bool XRonly;
	int Resolution;
	double FilmLength;
	bool Impnorm;
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
	bool Adaptive;
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
	bool UseGpu;
	int32_t GpuChains;
	std::string_view Title;
};

// Session state for resuming a previous SA run.
// Electron writes stochfit-session.json; on resume it calls GetRunState() to read
// raw internal values then passes this struct to Init(). nullptr = fresh start.
// filmAbsInput = m_dBeta / m_dWaveConstant (inverse of Set_FilmAbs multiplication).
// temperature  = raw m_dTemp (not 1/m_dTemp — passed directly to SetTemp()).
// surfAbs saved independently so it is not baked into filmAbsInput across save/load cycles.
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
	std::span<const double> edValues;  // Boxes+2 doubles: [supphase, box1..boxN, subphase]
	// edCount removed — use edValues.size()
};
