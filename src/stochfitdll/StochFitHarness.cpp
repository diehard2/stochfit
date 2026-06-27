/*
 *	Copyright (C) 2008 Stephen Danauskas
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "StochFitHarness.h"
#include "ParamVector.h"
#include "platform.h"
#include <omp.h>

#ifdef _WIN32
#  define NOMINMAX
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <vector>
#  include <mutex>

static DWORD_PTR GetPCoreMask() {
    DWORD bufLen = 0;
    GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &bufLen);
    std::vector<uint8_t> buf(bufLen);
    auto* info = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(buf.data());
    if (!GetLogicalProcessorInformationEx(RelationProcessorCore, info, &bufLen))
        return ~(DWORD_PTR)0;

    uint8_t maxClass = 0;
    for (auto* p = info; reinterpret_cast<uint8_t*>(p) < buf.data() + bufLen;
         p = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(
             reinterpret_cast<uint8_t*>(p) + p->Size))
        maxClass = std::max(maxClass, p->Processor.EfficiencyClass);

    DWORD_PTR mask = 0;
    bool hybrid = false;
    for (auto* p = info; reinterpret_cast<uint8_t*>(p) < buf.data() + bufLen;
         p = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(
             reinterpret_cast<uint8_t*>(p) + p->Size)) {
        if (p->Processor.EfficiencyClass < maxClass) hybrid = true;
        else mask |= p->Processor.GroupMask[0].Mask;
    }
    return hybrid ? mask : ~(DWORD_PTR)0;
}

static void PinOMPThreadsToPCores() {
    static std::once_flag pinned;
    std::call_once(pinned, [] {
        const DWORD_PTR mask = GetPCoreMask();
        const int pCoreCount = static_cast<int>(__popcnt64(mask));
        #pragma omp parallel
        { SetThreadAffinityMask(GetCurrentThread(), mask); }
        omp_set_num_threads(pCoreCount);
    });
}
#endif

// ── Constructor
// ───────────────────────────────────────────────────────────────

StochFit::StochFit(const ReflSettings &InitStruct,
                   const std::unique_ptr<StochRunState> &state)
    // m_initStruct first; m_cEDP/m_displayEDP before m_parratt (declaration order).
    : m_initStruct(InitStruct), params(InitStruct), m_displayState(InitStruct),
      m_cEDP(m_initStruct), m_displayEDP(m_initStruct),
      m_parratt(m_initStruct, m_cEDP.GetLayerCount()),
      m_objective(ReflectivityObjective::Type{InitStruct.Objectivefunction}),
      m_stepper({.sigmaSearch = InitStruct.Sigmasearch,
                 .absSearch = InitStruct.AbsorptionSearchPerc,
                 .normSearch = InitStruct.NormalizationSearchPerc,
                 .stepSize = InitStruct.Paramtemp}) {
  m_stop_requested = false;

  m_Directory = InitStruct.Directory;

  // Slice measurement data by CritEdgeOffset / HighQOffset.
  const int qsize  = static_cast<int>(InitStruct.Q.size());
  m_datapoints = qsize - InitStruct.HighQOffset - InitStruct.CritEdgeOffset;
  if (m_datapoints <= 0 ||
      static_cast<int>(InitStruct.Refl.size()) < qsize ||
      static_cast<int>(InitStruct.ReflError.size()) < qsize) {
    m_initError = tl::unexpected(std::string("Invalid Q/Refl/ReflError sizes"));
    return;
  }
  const int off = InitStruct.CritEdgeOffset;
  m_xi.assign(InitStruct.Q.begin() + off,
               InitStruct.Q.begin() + off + m_datapoints);
  m_yi.assign(InitStruct.Refl.begin() + off,
               InitStruct.Refl.begin() + off + m_datapoints);
  m_eyi.assign(InitStruct.ReflError.begin() + off,
                InitStruct.ReflError.begin() + off + m_datapoints);


  // Apply run state if provided.
  // filmAbsInput is the pre-multiplication value: Set_FilmAbs(x) stores x*WC.
  // temperature is raw m_dTemp (β) stored directly via SetTemperature.
  // surfAbs is saved independently so it is never baked into filmAbsInput.
  if (state &&
      static_cast<int>(state->edValues.size()) == params.RealParamsSize()) {
    params.SetRoughness(state->roughness);
    params.SetSupphase(state->edValues[0]);
    for (int i = 1; i < static_cast<int>(state->edValues.size()) - 1; i++)
      params.SetMutatableParameter(i - 1, state->edValues[i]);
    params.SetSubphase(state->edValues[state->edValues.size() - 1]);
    m_cEDP.Set_FilmAbs(state->filmAbsInput);
    params.SetSurfAbs(state->surfAbs);
    params.SetImpNorm(state->impNorm);
  }
  params.UpdateBoundaries();
  m_displayState.params = params;

  // Build SA scratch buffer and deps.
  const int nd = m_datapoints;
  m_saReflBuf.resize(nd);
  AnnealDeps deps;
  deps.yi = m_yi;
  deps.eyi = m_eyi;
  deps.reflBuf = m_saReflBuf;
  deps.impNorm = InitStruct.Impnorm;

  // Construct the algorithm-specific annealer variant.
  const auto algo = AlgorithmFromInt(InitStruct.Algorithm);
  switch (algo) {
  case SaAlgorithm::Greedy:
    m_annealer.emplace(std::in_place_type<Anneal<GreedyPolicy>>, m_cEDP,
                       m_parratt, m_objective, m_stepper, params, deps);
    break;
  case SaAlgorithm::Simulated:
    m_annealer.emplace(std::in_place_type<Anneal<SimulatedPolicy>>, m_cEDP,
                       m_parratt, m_objective, m_stepper, params, deps,
                       InitStruct.Inittemp, InitStruct.Slope,
                       InitStruct.Platiter);
    break;
  case SaAlgorithm::Stun:
    m_annealer.emplace(
        std::in_place_type<Anneal<StunPolicy>>, m_cEDP, m_parratt, m_objective,
        m_stepper, params, deps, InitStruct.Inittemp, InitStruct.Slope,
        InitStruct.Platiter, InitStruct.Gamma, InitStruct.Gammadec,
        InitStruct.STUNfunc, InitStruct.STUNdeciter, InitStruct.Tempiter,
        InitStruct.Adaptive);
    break;
  }

  // Apply session temperature/avgfSTUN after the annealer is constructed.
  if (state != nullptr &&
      static_cast<int>(state->edValues.size()) == params.RealParamsSize()) {
    std::visit(
        [&](auto &a) {
          a.SetTemperature(state->temperature);
          a.SetAverageFSTUN(state->avgfSTUN);
        },
        *m_annealer);
  }

  // Compute the initial energy to seed best/current state.
  std::visit([&](auto &a) { a.InitEnergy(params); }, *m_annealer);

  m_initError = {};
}

StochFit::~StochFit() {
  if (m_thread.joinable()) {
    m_stop_requested = true;
    m_thread.join();
  }
}

// ── Harness accessors
// ─────────────────────────────────────────────────────────

double StochFit::GetTemperature() const {
  if (!m_annealer)
    return 0.0;
  return std::visit([](const auto &a) { return a.GetTemperature(); },
                    *m_annealer);
}

double StochFit::GetRawTemperature() const {
  if (!m_annealer)
    return 0.0;
  return std::visit([](const auto &a) { return a.GetRawTemperature(); },
                    *m_annealer);
}

void StochFit::SetTemperature(double t) {
  if (m_annealer)
    std::visit([t](auto &a) { a.SetTemperature(t); }, *m_annealer);
}

double StochFit::GetLowestEnergy() const {
  if (!m_annealer)
    return 0.0;
  return std::visit([](const auto &a) { return a.GetLowestEnergy(); },
                    *m_annealer);
}

double StochFit::GetAverageFSTUN() const {
  if (!m_annealer)
    return 0.0;
  return std::visit([](const auto &a) { return a.GetAverageFSTUN(); },
                    *m_annealer);
}

void StochFit::SetAverageFSTUN(double f) {
  if (m_annealer)
    std::visit([f](auto &a) { a.SetAverageFSTUN(f); }, *m_annealer);
}

// ── Main CPU loop
// ─────────────────────────────────────────────────────────────

int StochFit::Processing() {
  try {
    if (!m_annealer)
      return -1;

#ifdef _WIN32
    PinOMPThreadsToPCores();
#endif

    const int nThreads = omp_get_max_threads();

#pragma omp parallel num_threads(nThreads)
    {
      for (int isteps = 0;
           isteps < m_itotaliterations && !m_stop_requested.load(); ++isteps) {

        // All threads: mutate candidate (omp single inside) + build EDP (omp for inside).
        // PrepareCandidate retries internally until a valid EDP is produced.
        std::visit([&](auto &a) { a.PrepareCandidate(params); }, *m_annealer);

        // All threads: cooperative Parratt Q-point distribution (omp for inside).
        std::visit([&](auto &a) { a.ComputeSharedRefl(); }, *m_annealer);

        // Serial: accept/reject + display snapshot.
#pragma omp single
        {
          bool accepted = false;
          double gof = 0.0, chi = 0.0;

          accepted = std::visit(
              [&](auto &a) { return a.EvaluateAndAccept(params); }, *m_annealer);
          if (accepted || isteps == 0) {
            gof = std::visit([](const auto &a) { return a.GetCurrentEnergy(); },
                             *m_annealer);
            chi = std::visit([](const auto &a) { return a.GetLastChiSquare(); },
                             *m_annealer);
          }

          if (accepted || isteps == 0) {
            std::lock_guard lock(m_displayMutex);
            m_displayState.params = params;
            m_displayState.refl.assign(m_saReflBuf.begin(), m_saReflBuf.end());
            m_displayState.chiSquare = chi;
            m_displayState.goF       = gof;
          }
          m_icurrentiteration.store(isteps + 1, std::memory_order_relaxed);
        }
        // implicit barrier: all threads sync before the next iteration
      }
    }

    m_icurrentiteration.store(m_itotaliterations, std::memory_order_relaxed);
    return 0;
  } catch (const std::exception &ex) {
    std::cerr << "[StochFit] Processing() caught exception: " << ex.what()
              << std::endl;
    return -1;
  } catch (...) {
    std::cerr << "[StochFit] Processing() caught unknown exception" << std::endl;
    return -1;
  }
}

// ── Display snapshot (main thread only)
// ──────────────────────────────────────

DataSnapshot StochFit::GetCurrentState() {
  // Copy the minimum inside the lock, then compute outside.
  DisplayState snap(m_initStruct);
  {
    std::lock_guard lock(m_displayMutex);
    snap = m_displayState;
  }

  m_displayEDP.GenerateEDP(snap.params);

  DataSnapshot out;
  out.roughness     = snap.params.GetRoughness();
  out.chiSquare     = snap.chiSquare;
  out.goodnessOfFit = snap.goF;

  // snap.refl is the SA scoring buffer: one value per measured Q point.
  const int nd = m_datapoints;
  out.Q.assign(m_xi.begin(), m_xi.begin() + nd);
  out.refl = std::move(snap.refl);
  out.refl.resize(nd);

  const int nl = m_displayEDP.Get_EDPPointCount();
  out.z.resize(nl);
  out.rho.resize(nl);
  const double dz         = m_displayEDP.Get_Dz();
  const double leftOffset = m_displayEDP.Get_LeftOffset();
  const double rhoSub     = m_displayEDP.m_EDP[nl - 1].real();
  for (int i = 0; i < nl; i++) {
    out.z[i]   = i * dz - leftOffset;
    out.rho[i] = m_displayEDP.m_EDP[i].real() / rhoSub;
  }

  return out;
}

// ── Public API
// ────────────────────────────────────────────────────────────────

int StochFit::Start(int iterations) {
  m_itotaliterations = iterations;
  m_stop_requested = false;
  m_thread = std::thread([this] { Processing(); });
  return 0;
}

int StochFit::Cancel() {
  if (m_thread.joinable())
    m_stop_requested = true;
  return 0;
}

void StochFit::Stop() {
  if (m_thread.joinable()) {
    m_stop_requested = true;
    m_thread.join();
  }
}

StochRunState StochFit::GetRunState() {
  // Called after Stop() — thread is joined, no lock needed.
  StochRunState s;
  s.roughness     = params.GetRoughness();
  s.filmAbsInput  = m_cEDP.Get_FilmAbsInput();
  s.surfAbs       = params.GetSurfAbs();
  s.temperature   = GetRawTemperature();
  s.impNorm       = params.GetImpNorm();
  s.avgfSTUN      = GetAverageFSTUN();
  s.bestSolution  = GetLowestEnergy();
  s.chiSquare     = m_displayState.chiSquare;
  s.goodnessOfFit = m_displayState.goF;
  auto span = params.RealParams();
  s.edValues.assign(span.begin(), span.end());
  return s;
}

DataSnapshot StochFit::GetData() {
  const int iter = m_icurrentiteration.load(std::memory_order_relaxed);
  DataSnapshot snap = GetCurrentState();
  snap.iteration  = iter;
  snap.isFinished = (iter >= m_itotaliterations);
  return snap;
}
