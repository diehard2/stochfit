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
        bool OneSigma;
        bool WriteFiles;
        double SubSLD;
        double SupSLD;
        int Boxes;
        double Wavelength;
        double QSpread;
        bool Forcenorm;
        bool ImpNorm;
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