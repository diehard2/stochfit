#pragma once

#include "AnnealPolicies.h"
#include "CEDP.h"
#include "ParameterStepper.h"
#include "ReflectivityObjective.h"
#include "UnifiedReflectivity.h"

#include <limits>
#include <random>
#include <ranges>
#include <span>

struct AnnealDeps {
    std::span<const double> yi, eyi;
    std::span<double>       reflBuf;
    bool xrOnly    = false;
    bool forceNorm = false;
    bool impNorm   = false;
};

inline bool HasNegativeDensity(const CEDP& edp) {
    return std::ranges::any_of(
        std::span{edp.m_EDP}.first(static_cast<size_t>(edp.Get_EDPPointCount())),
        [](const auto& v) { return v.real() < 0.0; });
}

template <class Policy>
class Anneal {
public:
    template <class... PolicyArgs>
    Anneal(CEDP& edp,
           ParrattReflectivity& parratt,
           const ReflectivityObjective& objective,
           ParameterStepper& stepper,
           const ParamVector& initParams,
           AnnealDeps deps,
           PolicyArgs&&... policyArgs)
        : m_edp(&edp), m_parratt(&parratt), m_objective(&objective),
          m_stepper(&stepper), m_tempParams(initParams), m_deps(std::move(deps)),
          m_policy(std::forward<PolicyArgs>(policyArgs)...),
          m_rng(std::random_device{}()) {}

    bool Iteration(ParamVector& params);
    void InitEnergy(ParamVector& params);

    double GetTemperature() const    { return m_policy.GetTemperature(); }
    void   SetTemperature(double t)  { m_policy.SetTemperature(t); }
    double GetRawTemperature() const { return m_policy.GetRawTemperature(); }
    double GetAverageFSTUN() const   { return m_policy.GetAverageFSTUN(); }
    void   SetAverageFSTUN(double f) { m_policy.SetAverageFSTUN(f); }
    double GetLowestEnergy() const   { return m_bestEnergy; }
    double GetCurrentEnergy() const  { return m_currentEnergy; }
    double GetLastChiSquare() const  { return m_lastChiSquare; }
    bool   IsIterMinimum() const     { return m_isIterMinimum; }

private:
    CEDP*                        m_edp;
    ParrattReflectivity*         m_parratt;
    const ReflectivityObjective* m_objective;
    ParameterStepper*            m_stepper;
    ParamVector                  m_tempParams;
    AnnealDeps                   m_deps;
    std::mt19937                 m_rng;
    double m_bestEnergy    = std::numeric_limits<double>::max();
    double m_currentEnergy = std::numeric_limits<double>::max();
    double m_lastChiSquare = 0.0;
    bool   m_isIterMinimum = false;
    [[no_unique_address]] Policy m_policy;

    void ComputeModel(ParamVector& p) {
        auto result = m_parratt->CalculateReflectivity(*m_edp);
        std::ranges::copy(result, m_deps.reflBuf.begin());
        if (m_deps.impNorm) {
            for (auto& v : m_deps.reflBuf) v *= p.getImpNorm();
        } else if (m_deps.forceNorm) {
            const double f = 1.0 / m_deps.reflBuf[0];
            for (auto& v : m_deps.reflBuf) v *= f;
        }
    }
};

template <class Policy>
void Anneal<Policy>::InitEnergy(ParamVector& params) {
    m_edp->GenerateEDP(params);
    ComputeModel(params);
    m_bestEnergy = m_currentEnergy =
        m_objective->Evaluate(m_deps.reflBuf, m_deps.yi, m_deps.eyi);
    m_lastChiSquare = ComputeChiSquare(m_deps.reflBuf, m_deps.yi, m_deps.eyi);
}

template <class Policy>
bool Anneal<Policy>::Iteration(ParamVector& params) {
    m_isIterMinimum = false;
    m_tempParams = params;
    m_stepper->Step(m_tempParams);

    m_edp->GenerateEDP(m_tempParams);

    if (m_deps.xrOnly && HasNegativeDensity(*m_edp))
        return false;

    ComputeModel(m_tempParams);

    const double candE = m_objective->Evaluate(m_deps.reflBuf, m_deps.yi, m_deps.eyi);

    // Fast path: unconditionally accept a new global best.
    if (candE < m_bestEnergy) {
        m_bestEnergy = m_currentEnergy = candE;
        params = m_tempParams;
        m_isIterMinimum = true;
        m_lastChiSquare = ComputeChiSquare(m_deps.reflBuf, m_deps.yi, m_deps.eyi);
        return true;
    }

    if (m_policy.Accept(m_currentEnergy, candE, m_bestEnergy, m_rng)) {
        m_currentEnergy = candE;
        params = m_tempParams;
        m_lastChiSquare = ComputeChiSquare(m_deps.reflBuf, m_deps.yi, m_deps.eyi);
        return true;
    }

    return false;
}
