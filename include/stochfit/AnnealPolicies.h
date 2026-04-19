#pragma once

#include <cmath>
#include <deque>
#include <random>

// ── Greedy ────────────────────────────────────────────────────────────────────
// Accepts any move that lowers the energy. No temperature schedule.
struct GreedyPolicy {
    bool Accept(double curE, double candE, double /*bestE*/, std::mt19937& /*rng*/) const {
        return candE < curE;
    }
    double GetTemperature() const    { return 0.0; }
    void   SetTemperature(double)    {}
    double GetRawTemperature() const { return 0.0; }
    double GetAverageFSTUN() const   { return 0.0; }
    void   SetAverageFSTUN(double)   {}
};

// ── Simulated Annealing ───────────────────────────────────────────────────────
// Standard Metropolis acceptance with geometric cooling.
// m_dTemp stores β = 1/T. ProbCalc = exp(-β·ΔE)·100.
struct SimulatedPolicy {
    SimulatedPolicy(double initTemp, double slope, int platIter);

    bool Accept(double curE, double candE, double /*bestE*/, std::mt19937& rng);

    double GetTemperature() const    { return 1.0 / m_dTemp; }
    void   SetTemperature(double t)  { m_dTemp = t; }       // t is raw β (from session)
    double GetRawTemperature() const { return m_dTemp; }    // β, for session save
    double GetAverageFSTUN() const   { return 0.0; }
    void   SetAverageFSTUN(double)   {}

private:
    double m_dTemp;
    double m_slope;
    int    m_platIter;
    int    m_iter = 0;

    void   Schedule();
    double ProbCalc(double deltaE) const { return std::exp(-m_dTemp * deltaE) * 100.0; }
};

// ── STUN (Stochastic Tunneling) ───────────────────────────────────────────────
// Transforms the energy landscape via fSTUN to tunnel through barriers.
// Supports adaptive temperature control via a sliding window average.
// m_dTemp stores β = 1/T, same convention as SimulatedPolicy.
struct StunPolicy {
    StunPolicy(double initTemp, double slope, int platIter,
               double gamma, double gammaDec,
               int stunFunc, int stunDecIter, int tempIter, bool adaptive);

    bool Accept(double curE, double candE, double bestE, std::mt19937& rng);

    double GetTemperature() const    { return 1.0 / m_dTemp; }
    void   SetTemperature(double t)  { m_dTemp = t; }
    double GetRawTemperature() const { return m_dTemp; }
    double GetAverageFSTUN() const   { return m_averageFSTUN; }
    void   SetAverageFSTUN(double f) { m_averageFSTUN = f; }

private:
    double m_dTemp;
    double m_slope;
    int    m_platIter;
    int    m_iter = 0;
    double m_gamma;
    double m_gammaDec;
    double m_averageFSTUN;
    int    m_stunFunc;
    int    m_stunDecIter;
    int    m_tempIter;
    bool   m_adaptive;
    std::deque<double> m_qWindow;

    double fSTUN(double val, double bestE) const;
    void   AdjustTemp(double averageSTUN);
    void   Schedule();
    double ProbCalc(double deltaE) const { return std::exp(-m_dTemp * deltaE) * 100.0; }
};
