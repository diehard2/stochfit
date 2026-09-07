#pragma once

#include <limits>
#include <random>
#include <span>

#include "CEDP.h"
#include "ParameterStepper.h"
#include "ReflectivityObjective.h"
#include "UnifiedReflectivity.h"

// clang-cl targets the MSVC ABI, where plain [[no_unique_address]] has no
// effect (and warns as an unknown attribute); clang instead spells it
// [[msvc::no_unique_address]] there. MSVC itself only understands that
// spelling too (added in 19.35 / VS 17.5).
#if defined(_MSC_VER)
    #define STOCHFIT_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
    #define STOCHFIT_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

struct AnnealDeps
{
    std::span<const double> yi, eyi;
    std::span<double> reflBuf;
    bool impNorm = false;
};

template <class Policy>
class Anneal
{
  public:
    template <class... PolicyArgs>
    Anneal(CEDP& edp, ParrattReflectivity& parratt, const ReflectivityObjective& objective, ParameterStepper& stepper,
           ParamVector initParams, AnnealDeps deps, PolicyArgs&&... policyArgs)
        : m_edp(&edp),
          m_parratt(&parratt),
          m_objective(&objective),
          m_stepper(&stepper),
          m_tempParams(std::move(initParams)),
          m_deps(deps),
          m_rng(std::random_device{}()),
          m_policy(std::forward<PolicyArgs>(policyArgs)...)
    {
    }

    void InitEnergy(ParamVector& params);

    // Cooperative interface for persistent OMP parallel regions.
    // All threads must call these in order per SA iteration.
    // PrepareCandidate: mutates candidate (omp single) then builds EDP (omp for).
    // ComputeSharedRefl: runs cooperative Parratt (omp for); no barrier — returns
    //   a span into the Parratt object's own scratch buffer (stable until the next
    //   ComputeSharedRefl call). Every thread gets the identical span back as an
    //   ordinary return value (each in its own stack frame — no shared write, so
    //   no data race), so the caller can hold onto it until the next single.
    // PublishResult: serial only — call from the same omp single as EvaluateAndAccept,
    //   passing the span ComputeSharedRefl returned. Copies/scales it into m_deps.reflBuf.
    // EvaluateAndAccept: pure serial — call from omp single only, after PublishResult.
    void PrepareCandidate(ParamVector& params);
    std::span<double> ComputeSharedRefl();
    void PublishResult(std::span<const double> result);
    bool EvaluateAndAccept(ParamVector& params);

    double GetTemperature() const
    {
        return m_policy.GetTemperature();
    }
    void SetTemperature(double t)
    {
        m_policy.SetTemperature(t);
    }
    double GetRawTemperature() const
    {
        return m_policy.GetRawTemperature();
    }
    double GetAverageFSTUN() const
    {
        return m_policy.GetAverageFSTUN();
    }
    void SetAverageFSTUN(double f)
    {
        m_policy.SetAverageFSTUN(f);
    }
    double GetLowestEnergy() const
    {
        return m_bestEnergy;
    }
    double GetCurrentEnergy() const
    {
        return m_currentEnergy;
    }
    double GetLastChiSquare() const
    {
        return m_lastChiSquare;
    }

  private:
    CEDP* m_edp;
    ParrattReflectivity* m_parratt;
    const ReflectivityObjective* m_objective;
    ParameterStepper* m_stepper;
    ParamVector m_tempParams;
    AnnealDeps m_deps;
    std::mt19937 m_rng;
    double m_bestEnergy = std::numeric_limits<double>::max();
    double m_currentEnergy = std::numeric_limits<double>::max();
    double m_lastChiSquare = 0.0;
    STOCHFIT_NO_UNIQUE_ADDRESS Policy m_policy;

    void ComputeModel(ParamVector& p)
    {
        auto result = m_parratt->CalculateReflectivity(*m_edp);
        std::ranges::copy(result, m_deps.reflBuf.begin());
        if (m_deps.impNorm) {
            for (auto& v : m_deps.reflBuf) {
                v *= p.GetImpNorm();
            }
        }
    }
};

template <class Policy>
void Anneal<Policy>::InitEnergy(ParamVector& params)
{
    m_edp->GenerateEDP(params);
    ComputeModel(params);
    m_bestEnergy = m_currentEnergy = m_objective->Evaluate(m_deps.reflBuf, m_deps.yi, m_deps.eyi);
    m_lastChiSquare = ComputeChiSquare(m_deps.reflBuf, m_deps.yi, m_deps.eyi);
}

// ── Cooperative methods for persistent OMP parallel regions ──────────────────

template <class Policy>
void Anneal<Policy>::PrepareCandidate(ParamVector& params)
{
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
std::span<double> Anneal<Policy>::ComputeSharedRefl()
{
    // No single/barrier here: the result stays valid in the Parratt object's own
    // scratch buffer until the next ComputeSharedRefl call, so publishing it can
    // wait until the caller's existing serial section (see PublishResult).
    return m_parratt->CalculateReflectivityCooperative(*m_edp);
}

template <class Policy>
void Anneal<Policy>::PublishResult(std::span<const double> result)
{
    std::ranges::copy(result, m_deps.reflBuf.begin());
    if (m_deps.impNorm) {
        for (auto& v : m_deps.reflBuf) {
            v *= m_tempParams.GetImpNorm();
        }
    }
}

template <class Policy>
bool Anneal<Policy>::EvaluateAndAccept(ParamVector& params)
{
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
