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

    double Evaluate(std::span<const double> model,
                    std::span<const double> yi,
                    std::span<const double> eyi) const;

private:
    Type m_type;
};

double ComputeChiSquare(std::span<const double> model,
                        std::span<const double> yi,
                        std::span<const double> eyi);
