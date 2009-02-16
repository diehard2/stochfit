#pragma once

#pragma pack(push, 8)
struct BoxReflSettings
{
        LPCWSTR Directory;
        double* Q;
        double* Refl;
        double* ReflError;
        double* QError;
        double* UL;
        double* LL;
        double* ParamPercs;
        int QPoints;
        BOOL OneSigma;
        BOOL WriteFiles;
        double SubSLD;
        double SupSLD;
        int Boxes;
        double Wavelength;
        double QSpread;
        BOOL Forcenorm;
        BOOL ImpNorm;
        int FitFunc;
        int LowQOffset;
        int HighQOffset;
		int Iterations;
		//EDP Specific Settings
		double* MIEDP;
		double* ZIncrement;
		int ZLength;

};
#pragma pack(pop)