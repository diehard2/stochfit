#pragma once

#include "ParamVector.h"
#include <random>

class ParameterStepper {
public:
    struct Config {
        int    sigmaSearch;
        int    absSearch;
        int    normSearch;
        double stepSize;
    };

    explicit ParameterStepper(Config cfg);
    void Step(ParamVector& params);

private:
    Config      m_cfg;
    std::mt19937 m_rng;
};
