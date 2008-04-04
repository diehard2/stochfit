#pragma once

#pragma pack(push, 8)
struct ReflSettings
{
		LPCWSTR Directory;
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
};
#pragma pack(pop)

