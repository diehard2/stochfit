#pragma once

#include <span>

class ReflectivityObjective {
public:
    enum class Type : int {
        LogDiff     = 0,
        InvRatio    = 1,
        LogDiffErr  = 2,
        InvRatioErr = 3,
    };

    explicit ReflectivityObjective(Type t) : m_type(t) {}

    // Returns sum(residual[i]²) / (n+1).
    double Evaluate(std::span<const double> model,
                    std::span<const double> yi,
                    std::span<const double> eyi) const;

    // Fills residuals[i] for i in [low_q_offset, n - high_q_offset).
    // Indices outside that range are zero-filled (already set by caller or here).
    // Non-finite values are clamped to ±1e6.
    void FillResiduals(std::span<const double> model,
                       std::span<const double> yi,
                       std::span<const double> eyi,
                       std::span<double>       residuals,
                       int low_q_offset  = 0,
                       int high_q_offset = 0) const;

private:
    Type m_type;
};

double ComputeChiSquare(std::span<const double> model,
                        std::span<const double> yi,
                        std::span<const double> eyi);
