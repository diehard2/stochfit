#include "ReflectivityObjective.h"
#include <cmath>

void ReflectivityObjective::FillResiduals(std::span<const double> model,
                                          std::span<const double> yi,
                                          std::span<const double> eyi,
                                          std::span<double>       residuals,
                                          int low_q_offset,
                                          int high_q_offset) const {
    const int n = static_cast<int>(model.size());
    std::fill(residuals.begin(), residuals.end(), 0.0);
    const int hi = n - high_q_offset;

    switch (m_type) {
    case Type::LogDiff:
        for (int i = low_q_offset; i < hi; ++i) {
            double r = std::log(yi[i]) - std::log(model[i]);
            residuals[i] = std::isfinite(r) ? r : (r > 0 ? 1e6 : -1e6);
        }
        break;
    case Type::InvRatio:
        for (int i = low_q_offset; i < hi; ++i) {
            double r = yi[i] / model[i];
            if (r < 1.0) r = 1.0 / r;
            double res = 1.0 - r;
            residuals[i] = std::isfinite(res) ? res : -1e6;
        }
        break;
    case Type::LogDiffErr:
        for (int i = low_q_offset; i < hi; ++i) {
            double d   = std::log(yi[i]) - std::log(model[i]);
            double res = d / std::sqrt(std::fabs(std::log(eyi[i])));
            residuals[i] = std::isfinite(res) ? res : (res > 0 ? 1e6 : -1e6);
        }
        break;
    case Type::InvRatioErr:
        for (int i = low_q_offset; i < hi; ++i) {
            double r = yi[i] / model[i];
            if (r < 1.0) r = 1.0 / r;
            double emap = yi[i] / eyi[i];
            double res  = (1.0 - r) * emap;
            residuals[i] = std::isfinite(res) ? res : -1e6;
        }
        break;
    }
}

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
