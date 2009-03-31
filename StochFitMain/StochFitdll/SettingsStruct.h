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
        bool UseSurfAbs;
        double Leftoffset;
        double QErr;
        bool Forcenorm;
        double Forcesig;
        bool Debug;
        bool XRonly;
        int Resolution;
        double Totallength;
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
		LPCWSTR Title;

		bool IsNeutron;
        LPCWSTR Version;
        bool disposed;

		ReflSettings()
		{
			Directory = L"";
			Q = NULL;
			DisplayQ = NULL;
			Refl = NULL;
			ReflError = NULL;
			QError = NULL;
			QPoints = 0;
			DispQPoints = 0;
			SubSLD = 0.0;
			FilmSLD = 0.0;
			SupSLD = 0.0;
			Boxes = 0;
			FilmAbs = 0.0;
			SubAbs = 0.0;
			SupAbs = 0.0;
			Wavelength = 0.0;
			UseSurfAbs = false;
			Leftoffset = 0;
			QErr = 0.0;
			Forcenorm = false;
			Forcesig = false;
			Debug = false;
			XRonly = false;
			Resolution = 0;
			Totallength = 0.0;
			FilmLength = 0.0;
			Impnorm = false;
			Objectivefunction = 0;
			Paramtemp = 0.0;
			CritEdgeOffset = 0;
			HighQOffset = 0;
			//Not used
			Iterations = 0;
			IterationsCompleted = 0;
			ChiSquare = 0.0;
			Title = L"";

			IsNeutron = false;
			Version = L"";
			disposed = false;

		}
		
};
#pragma pack(pop)

