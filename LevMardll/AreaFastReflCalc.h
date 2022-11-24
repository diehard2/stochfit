
#pragma once

class AreaFastReflCalc: FastReflcalc 
{
	void CalcRefl(double* sintheta, double* sinsquaredtheta, int datapoints, double* refl);
	void mkdensityonesigma(double* p, int plength);
    void mkdensity(double* p, int plength);
	void Rhocalculate(double Zoffset,double* ZIncrement, double* LengthArray, double* RhoArray, double* SigmaArray, double* nk, double* nkb, int loopcounter);
};