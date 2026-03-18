#pragma once
#include "platform.h"

#pragma pack(push, 8)
struct BoxReflSettings
{
        const char* Directory;
        double* Q;
        double* Refl;
        double* ReflError;
        double* QError;
        double* UL;
        double* LL;
        double* ParamPercs;
        int QPoints;
        int32_t OneSigma;
        int32_t WriteFiles;
        double SubSLD;
        double SupSLD;
        int Boxes;
        double Wavelength;
        double QSpread;
        int32_t Forcenorm;
        int32_t ImpNorm;
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
