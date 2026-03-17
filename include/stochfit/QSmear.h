#pragma once

#include "platform.h"
#include <numbers>

namespace QSmear {

// Fills qspreadsinthetai[13*i .. 13*i+12] and qspreadsinsquaredthetai for
// each data point. sinthetai must already be filled (size: datapoints).
// Pass qerror != nullptr for per-point Q errors; otherwise a constant
// fractional spread (qspread) is used.
inline void BuildArrays(
    int datapoints, double lambda, double qspread,
    const double* sinthetai, const double* qerror,
    double* qspreadsinthetai, double* qspreadsinsquaredthetai)
{
    if (qerror != nullptr) {
        const double holder = lambda / (4.0 * std::numbers::pi);
        for (int i = 0; i < datapoints; i++) {
            const double s  = sinthetai[i];
            const double ds = holder * qerror[i];
            qspreadsinthetai[13*i]    = s;
            qspreadsinthetai[13*i+1]  = s + 1.2*ds;
            qspreadsinthetai[13*i+2]  = s - 1.2*ds;
            qspreadsinthetai[13*i+3]  = s + 1.0*ds;
            qspreadsinthetai[13*i+4]  = s - 1.0*ds;
            qspreadsinthetai[13*i+5]  = s + 0.8*ds;
            qspreadsinthetai[13*i+6]  = s - 0.8*ds;
            qspreadsinthetai[13*i+7]  = s + 0.6*ds;
            qspreadsinthetai[13*i+8]  = s - 0.6*ds;
            qspreadsinthetai[13*i+9]  = s + 0.4*ds;
            qspreadsinthetai[13*i+10] = s - 0.4*ds;
            qspreadsinthetai[13*i+11] = s + 0.2*ds;
            qspreadsinthetai[13*i+12] = s - 0.2*ds;
            if (qspreadsinthetai[13*i+1] < 0.0)
                platform_error("Error in QSpread please contact the author - the program will now crash :(");
        }
    } else {
        for (int i = 0; i < datapoints; i++) {
            const double s = sinthetai[i];
            qspreadsinthetai[13*i]    = s;
            qspreadsinthetai[13*i+1]  = s * (1 + 1.2*qspread);
            qspreadsinthetai[13*i+2]  = s * (1 - 1.2*qspread);
            qspreadsinthetai[13*i+3]  = s * (1 + 1.0*qspread);
            qspreadsinthetai[13*i+4]  = s * (1 - 1.0*qspread);
            qspreadsinthetai[13*i+5]  = s * (1 + 0.8*qspread);
            qspreadsinthetai[13*i+6]  = s * (1 - 0.8*qspread);
            qspreadsinthetai[13*i+7]  = s * (1 + 0.6*qspread);
            qspreadsinthetai[13*i+8]  = s * (1 - 0.6*qspread);
            qspreadsinthetai[13*i+9]  = s * (1 + 0.4*qspread);
            qspreadsinthetai[13*i+10] = s * (1 - 0.4*qspread);
            qspreadsinthetai[13*i+11] = s * (1 + 0.2*qspread);
            qspreadsinthetai[13*i+12] = s * (1 - 0.2*qspread);
            if (qspreadsinthetai[13*i+1] < 0.0)
                platform_error("Error in QSpread please contact the author - the program will now crash :(");
        }
    }

    for (int l = 0; l < 13 * datapoints; l++)
        qspreadsinsquaredthetai[l] = qspreadsinthetai[l] * qspreadsinthetai[l];
}

// Weighted 13-point Gaussian quadrature average over the spread reflectivity.
// qspreadreflpt: size datapoints*13 (input)
// refl:          size datapoints    (output)
inline void Apply(const double* qspreadreflpt, double* refl, int datapoints)
{
    for (int i = 0; i < datapoints; i++) {
        double c = qspreadreflpt[13*i];
        c += 0.056 * qspreadreflpt[13*i+1];
        c += 0.056 * qspreadreflpt[13*i+2];
        c += 0.135 * qspreadreflpt[13*i+3];
        c += 0.135 * qspreadreflpt[13*i+4];
        c += 0.278 * qspreadreflpt[13*i+5];
        c += 0.278 * qspreadreflpt[13*i+6];
        c += 0.487 * qspreadreflpt[13*i+7];
        c += 0.487 * qspreadreflpt[13*i+8];
        c += 0.726 * qspreadreflpt[13*i+9];
        c += 0.726 * qspreadreflpt[13*i+10];
        c += 0.923 * qspreadreflpt[13*i+11];
        c += 0.923 * qspreadreflpt[13*i+12];
        refl[i] = c / 6.211;
    }
}

} // namespace QSmear
