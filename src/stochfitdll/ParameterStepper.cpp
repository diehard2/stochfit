#include "ParameterStepper.h"

ParameterStepper::ParameterStepper(Config cfg)
    : m_cfg(cfg), m_rng(std::random_device{}()) {}

void ParameterStepper::Step(ParamVector& params) {
    constexpr double kRoughMult = 5.0 / 3.0;

    const int ii   = std::uniform_int_distribution<int>(0, params.BoxCount() - 1)(m_rng);
    const int perc = std::uniform_int_distribution<int>(1, 100)(m_rng);

    const int sigmaTop = m_cfg.sigmaSearch;
    const int absTop   = sigmaTop + m_cfg.absSearch;
    const int normTop  = absTop + m_cfg.normSearch;

    if (perc > normTop) {
        params.SetMutatableParameter(ii,
            std::uniform_real_distribution<double>(
                params.GetMutatableParameter(ii) - m_cfg.stepSize,
                params.GetMutatableParameter(ii) + m_cfg.stepSize)(m_rng));
    } else if (perc <= sigmaTop) {
        params.SetRoughness(
            std::uniform_real_distribution<double>(
                params.GetRoughness() * (1.0 - kRoughMult * m_cfg.stepSize),
                params.GetRoughness() * (1.0 + kRoughMult * m_cfg.stepSize))(m_rng));
    } else if (perc <= absTop) {
        params.SetSurfAbs(
            std::uniform_real_distribution<double>(
                params.GetSurfAbs() * (1.0 - m_cfg.stepSize),
                params.GetSurfAbs() * (1.0 + m_cfg.stepSize))(m_rng));
    } else {
        params.SetImpNorm(
            std::uniform_real_distribution<double>(
                params.GetImpNorm() * (1.0 - m_cfg.stepSize),
                params.GetImpNorm() * (1.0 + m_cfg.stepSize))(m_rng));
    }
}
