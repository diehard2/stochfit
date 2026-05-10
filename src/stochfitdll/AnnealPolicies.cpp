#include "AnnealPolicies.h"
#include <algorithm>

// ── SimulatedPolicy ───────────────────────────────────────────────────────────

SimulatedPolicy::SimulatedPolicy(double initTemp, double slope, int platIter)
    : m_dTemp(1.0 / initTemp), m_slope(slope), m_platIter(platIter) {}

bool SimulatedPolicy::Accept(double curE, double candE, double, std::mt19937& rng) {
    const double deltaE = candE - curE;
    ++m_iter;
    if (m_iter % m_platIter == 0)
        Schedule();
    return std::uniform_real_distribution<double>(0.0, 100.0)(rng) < ProbCalc(deltaE);
}

void SimulatedPolicy::Schedule() {
    if (m_dTemp > 1e-30)
        m_dTemp /= m_slope;
}

// ── StunPolicy ────────────────────────────────────────────────────────────────

StunPolicy::StunPolicy(double initTemp, double slope, int platIter,
                       double gamma, double gammaDec,
                       int stunFunc, int stunDecIter, int tempIter, bool adaptive)
    : m_dTemp(adaptive ? 10.0 : 1.0 / initTemp),
      m_slope(slope), m_platIter(platIter),
      m_gamma(gamma), m_gammaDec(gammaDec),
      m_averageFSTUN(initTemp),
      m_stunFunc(stunFunc), m_stunDecIter(stunDecIter),
      m_tempIter(tempIter), m_adaptive(adaptive) {}

bool StunPolicy::Accept(double curE, double candE, double bestE, std::mt19937& rng) {
    const double deltaE = fSTUN(candE, bestE) - fSTUN(curE, bestE);

    if (m_adaptive) {
        // Maintain a sliding window of fSTUN values (capacity = platIter).
        if (static_cast<int>(m_qWindow.size()) >= m_platIter)
            m_qWindow.pop_back();
        m_qWindow.push_front(1.0 + fSTUN(candE, bestE));

        // Recompute rolling average once the window is full.
        double windowAvg = 0.0;
        if (static_cast<int>(m_qWindow.size()) >= m_platIter) {
            for (double v : m_qWindow) windowAvg += v;
            windowAvg /= static_cast<double>(m_qWindow.size());
        }

        ++m_iter;

        if (m_iter % m_tempIter == 0 && static_cast<int>(m_qWindow.size()) >= m_platIter)
            AdjustTemp(windowAvg);
    } else {
        ++m_iter;
        if (m_iter % m_platIter == 0)
            Schedule();
    }

    return std::uniform_real_distribution<double>(0.0, 100.0)(rng) < ProbCalc(deltaE);
}

double StunPolicy::fSTUN(double val, double bestE) const {
    const double x = m_gamma * (val - bestE);
    switch (m_stunFunc) {
    case 0:  return -std::exp(-x);
    case 1:  return std::sinh(x) - 1.0;
    default: return std::log(x + std::sqrt(x * x + 1.0)) - 1.0;  // asinh - 1
    }
}

void StunPolicy::AdjustTemp(double averageSTUN) {
    // Wenzel & Hamacher (PRL 82:3003, 1999): reduce β when avg fSTUN exceeds
    // threshold (tunneling phase), increase β otherwise (local-search phase).
    if (averageSTUN > m_averageFSTUN) {
        if (m_dTemp > 1e-200)
            m_dTemp *= m_slope;      // β↓, T↑ — keep tunneling
    } else {
        if (m_dTemp < 1e200)
            m_dTemp /= m_slope;      // β↑, T↓ — settle into basin
    }
}

void StunPolicy::Schedule() {
    if (m_dTemp > 1e-30)
        m_dTemp /= m_slope;
}
