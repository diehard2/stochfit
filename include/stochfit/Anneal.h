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
    bool impNorm = false;
};

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

    void InitEnergy(ParamVector& params);

    // Cooperative interface for persistent OMP parallel regions.
    // All threads must call these in order per SA iteration.
    // PrepareCandidate: mutates candidate (omp single) then builds EDP (omp for).
    // ComputeSharedRefl: runs cooperative Parratt (omp for); result in m_deps.reflBuf.
    // EvaluateAndAccept: pure serial — call from omp single only.
    void PrepareCandidate(ParamVector& params);
    void ComputeSharedRefl();
    bool EvaluateAndAccept(ParamVector& params);

    double GetTemperature() const    { return m_policy.GetTemperature(); }
    void   SetTemperature(double t)  { m_policy.SetTemperature(t); }
    double GetRawTemperature() const { return m_policy.GetRawTemperature(); }
    double GetAverageFSTUN() const   { return m_policy.GetAverageFSTUN(); }
    void   SetAverageFSTUN(double f) { m_policy.SetAverageFSTUN(f); }
    double GetLowestEnergy() const   { return m_bestEnergy; }
    double GetCurrentEnergy() const  { return m_currentEnergy; }
    double GetLastChiSquare() const  { return m_lastChiSquare; }

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
    [[no_unique_address]] Policy m_policy;

    void ComputeModel(ParamVector& p) {
        auto result = m_parratt->CalculateReflectivity(*m_edp);
        std::ranges::copy(result, m_deps.reflBuf.begin());
        if (m_deps.impNorm) {
            for (auto& v : m_deps.reflBuf) v *= p.GetImpNorm();
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

// ── Cooperative methods for persistent OMP parallel regions ──────────────────

template <class Policy>
void Anneal<Policy>::PrepareCandidate(ParamVector& params) {
#pragma omp single
    {
        m_tempParams = params;
        m_stepper->Step(m_tempParams);
        m_edp->FillBoxArrays(m_tempParams);
    }
    // implicit barrier: all threads see m_tempParams and m_fRhoArray

    m_edp->GenerateEDPCooperative(m_tempParams);
    // implicit barrier: all threads see completed EDP
}

template <class Policy>
void Anneal<Policy>::ComputeSharedRefl() {
    auto result = m_parratt->CalculateReflectivityCooperative(*m_edp);
#pragma omp single
    {
        std::ranges::copy(result, m_deps.reflBuf.begin());
        if (m_deps.impNorm) {
            for (auto& v : m_deps.reflBuf) v *= m_tempParams.GetImpNorm();
        }
    }
}

template <class Policy>
bool Anneal<Policy>::EvaluateAndAccept(ParamVector& params) {
    const double candE = m_objective->Evaluate(m_deps.reflBuf, m_deps.yi, m_deps.eyi);

    if (candE < m_bestEnergy) {
        m_bestEnergy = m_currentEnergy = candE;
        params = m_tempParams;
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

