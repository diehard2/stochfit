#include "ReflectivityObjective.h"
#include <cmath>

double ReflectivityObjective::Evaluate(std::span<const double> model,
                                       std::span<const double> yi,
                                       std::span<const double> eyi) const {
    const int n = static_cast<int>(model.size());
    double score = 0.0;

    switch (m_type) {
    case Type::LogDiff:
        for (int i = 0; i < n; ++i) {
            double d = std::log(yi[i]) - std::log(model[i]);
            score += d * d;
        }
        break;
    case Type::InvRatio:
        for (int i = 0; i < n; ++i) {
            double r = yi[i] / model[i];
            if (r < 1.0) r = 1.0 / r;
            score += (1.0 - r) * (1.0 - r);
        }
        break;
    case Type::LogDiffErr:
        for (int i = 0; i < n; ++i) {
            double d = std::log(yi[i]) - std::log(model[i]);
            score += d * d / std::fabs(std::log(eyi[i]));
        }
        break;
    case Type::InvRatioErr:
        for (int i = 0; i < n; ++i) {
            double r = yi[i] / model[i];
            if (r < 1.0) r = 1.0 / r;
            double emap = (yi[i] / eyi[i]) * (yi[i] / eyi[i]);
            score += (1.0 - r) * (1.0 - r) * emap;
        }
        break;
    }

    return score / (n + 1);
}

double ComputeChiSquare(std::span<const double> model,
                        std::span<const double> yi,
                        std::span<const double> eyi) {
    const int n = static_cast<int>(model.size());
    double chi = 0.0;
    for (int i = 0; i < n; ++i) {
        double d = yi[i] - model[i];
        chi += (d * d) / (eyi[i] * eyi[i]);
    }
    return chi / n;
}
