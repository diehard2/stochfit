#pragma once
#include "platform.h"

struct BoxReflSettings
{
    std::string    Directory;
    vector<double> Q;
    vector<double> Refl;
    vector<double> ReflError;
    vector<double> QError;
    vector<double> UL;
    vector<double> LL;
    vector<double> ParamPercs;
    int QPoints    = 0;
    bool OneSigma  = false;
    bool WriteFiles = false;
    double SubSLD  = 0;
    double SupSLD  = 0;
    int Boxes      = 0;
    double Wavelength = 0;
    double QSpread = 0;
    bool Forcenorm = false;
    bool ImpNorm   = false;
    int FitFunc    = 0;
    int LowQOffset = 0;
    int HighQOffset = 0;
    int Iterations = 0;
    // EDP Specific Settings
    vector<double> MIEDP;
    vector<double> ZIncrement;
    int ZLength    = 0;
};
