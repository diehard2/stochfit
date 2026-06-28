#pragma once

#include "platform.h"
#include <numbers>
#include <span>

namespace QSmear {

inline constexpr int    Points     = 13;
inline constexpr double WeightsSum = 6.211;

// Fills qspreadsinthetai[Points*i .. Points*i+12] and qspreadsinsquaredthetai
// for each data point. sinthetai must already be filled (size: datapoints).
// Pass a non-empty qerror span for per-point Q errors; empty span uses a
// constant fractional spread (qspread). Center point is at index 6.
inline void BuildArrays(
    double lambda, double qspread,
    std::span<const double> sinthetai,
    std::span<const double> qerror,
    std::span<double> qspreadsinthetai,
    std::span<double> qspreadsinsquaredthetai)
{
    for (int i = 0; i < (int)sinthetai.size(); i++) {
        const double s  = sinthetai[i];
        const double ds = qerror.empty() ? s * qspread : (lambda / (4.0 * std::numbers::pi)) * qerror[i];
        qspreadsinthetai[13*i]    = s - 1.2*ds;
        qspreadsinthetai[13*i+1]  = s - 1.0*ds;
        qspreadsinthetai[13*i+2]  = s - 0.8*ds;
        qspreadsinthetai[13*i+3]  = s - 0.6*ds;
        qspreadsinthetai[13*i+4]  = s - 0.4*ds;
        qspreadsinthetai[13*i+5]  = s - 0.2*ds;
        qspreadsinthetai[13*i+6]  = s;
        qspreadsinthetai[13*i+7]  = s + 0.2*ds;
        qspreadsinthetai[13*i+8]  = s + 0.4*ds;
        qspreadsinthetai[13*i+9]  = s + 0.6*ds;
        qspreadsinthetai[13*i+10] = s + 0.8*ds;
        qspreadsinthetai[13*i+11] = s + 1.0*ds;
        qspreadsinthetai[13*i+12] = s + 1.2*ds;
         if (qspreadsinthetai[13*i] < 0.0)
            platform_error("Error in QSpread please contact the author - the program will now crash :(");
    }

    for (int l = 0; l < 13 * (int)sinthetai.size(); l++)
        qspreadsinsquaredthetai[l] = qspreadsinthetai[l] * qspreadsinthetai[l];
}

// Weighted 13-point Gaussian quadrature average over the spread reflectivity.
// qspreadreflpt: size datapoints*13 (input), center point at index 6
// refl:          size datapoints    (output)
inline void Apply(std::span<const double> qspreadreflpt, std::span<double> refl)
{
    for (int i = 0; i < (int)refl.size(); i++) {
        double c = 0.056 * qspreadreflpt[13*i];
        c += 0.135 * qspreadreflpt[13*i+1];
        c += 0.278 * qspreadreflpt[13*i+2];
        c += 0.487 * qspreadreflpt[13*i+3];
        c += 0.726 * qspreadreflpt[13*i+4];
        c += 0.923 * qspreadreflpt[13*i+5];
        c +=         qspreadreflpt[13*i+6];
        c += 0.923 * qspreadreflpt[13*i+7];
        c += 0.726 * qspreadreflpt[13*i+8];
        c += 0.487 * qspreadreflpt[13*i+9];
        c += 0.278 * qspreadreflpt[13*i+10];
        c += 0.135 * qspreadreflpt[13*i+11];
        c += 0.056 * qspreadreflpt[13*i+12];
        refl[i] = c / WeightsSum;
    }
}

} // namespace QSmear
