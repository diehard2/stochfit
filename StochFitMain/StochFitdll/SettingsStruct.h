#pragma once

//This struct must be kept in exact agreement with the struct in the C# StochFit project. It is the main method
//of communication between the GUI and the numeric routines


#pragma pack(push, 8)
struct ReflSettings
{
		LPCWSTR Directory;
        double* Q;
		double* DisplayQ;
        double* Refl;
        double* ReflError;
        double* QError;
        int QPoints;
		int DispQPoints;
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
		LPCWSTR Title;

		BOOL IsNeutron;
        LPCWSTR Version;
        bool disposed;
		
};
#pragma pack(pop)

