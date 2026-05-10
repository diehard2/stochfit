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

#include "gpu/gpu_detect.h"
#include "gpu/gpu_sa_runner.h"

// ── Constructor
// ───────────────────────────────────────────────────────────────

StochFit::StochFit(const ReflSettings &InitStruct,
                   const std::unique_ptr<StochRunState> &state)
    // m_initStruct first: m_parratt holds a const ref to it.
    : m_initStruct(InitStruct), params(InitStruct), m_displayState(InitStruct),
      m_parratt(m_initStruct),
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

  m_cEDP.Init(m_initStruct);
  m_displayEDP.Init(m_initStruct); // SLD profile display — no absorption needed

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
  deps.xrOnly = InitStruct.XRonly;
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
    // GPU path disabled pending rework — see ProcessingGPU() below.
    // if (m_initStruct.UseGpu) {
    //   auto gpu_info = detect_gpu();
    //   if (gpu_info.backend != GpuBackend::None) {
    //     m_gpuBackend = gpu_info.backend;
    //     return ProcessingGPU();
    //   }
    // }

    if (!m_annealer)
      return -1;

    for (int isteps = 0;
         isteps < m_itotaliterations && !m_stop_requested.load(); ++isteps) {

      const bool accepted =
          std::visit([&](auto &a) { return a.Iteration(params); }, *m_annealer);

      if (accepted || isteps == 0) {
        const double gof = std::visit(
            [](const auto &a) { return a.GetCurrentEnergy(); }, *m_annealer);
        const double chi = std::visit(
            [](const auto &a) { return a.GetLastChiSquare(); }, *m_annealer);
        std::lock_guard lock(m_displayMutex);
        m_displayState.params   = params;
        m_displayState.refl.assign(m_saReflBuf.begin(), m_saReflBuf.end());
        m_displayState.chiSquare = chi;
        m_displayState.goF       = gof;
      }
      m_icurrentiteration.store(isteps + 1, std::memory_order_relaxed);
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

// ── GPU support (disabled pending rework)
// ───────────────────────────────────────

void StochFit::InitGpuData(GpuSAState &, GpuParams &,
                           GpuMeasurement &, GpuEDPConfig &) {
  // TODO: rewrite to use DisplayState snapshot model.
  // Original implementation preserved below.
  //
  // memset(&sa_state, 0, sizeof(sa_state));
  // sa_state.temperature = 1.0f / static_cast<float>(GetTemperature());
  // sa_state.best_energy = static_cast<float>(GetLowestEnergy());
  // sa_state.current_energy = sa_state.best_energy;
  // sa_state.best_solution = sa_state.best_energy;
  // sa_state.slope = static_cast<float>(m_initStruct.Slope);
  // sa_state.gamma = static_cast<float>(m_initStruct.Gamma);
  // sa_state.avg_fstun = static_cast<float>(m_initStruct.Inittemp);
  // sa_state.gammadec = static_cast<float>(m_initStruct.Gammadec);
  // sa_state.stepsize = static_cast<float>(m_initStruct.Paramtemp);
  // sa_state.algorithm = m_initStruct.Algorithm;
  // sa_state.iteration = 0;
  // sa_state.plat_time = m_initStruct.Platiter;
  // sa_state.temp_iter = m_initStruct.Tempiter;
  // sa_state.stun_func = m_initStruct.STUNfunc;
  // sa_state.stun_dec_iter = m_initStruct.STUNdeciter;
  // sa_state.adaptive = m_initStruct.Adaptive;
  // sa_state.sigmasearch = m_initStruct.Sigmasearch;
  // sa_state.normsearch = m_initStruct.NormalizationSearchPerc;
  // sa_state.abssearch = m_initStruct.AbsorptionSearchPerc;
  //
  // memset(&gpu_params, 0, sizeof(gpu_params));
  // int rps = params.RealParamsSize();
  // gpu_params.real_params_size = rps;
  // gpu_params.num_boxes = params.BoxCount();
  // for (int i = 0; i < rps; i++)
  //   gpu_params.sld_values[i] = static_cast<float>(params.GetRealParams(i));
  // gpu_params.roughness = static_cast<float>(params.GetRoughness());
  // gpu_params.surf_abs = static_cast<float>(params.GetSurfAbs());
  // gpu_params.imp_norm = static_cast<float>(params.GetImpNorm());
  // gpu_params.fix_roughness = params.IsRoughnessFixed() ? 1 : 0;
  // gpu_params.use_surf_abs = params.UsesSurfAbs() ? 1 : 0;
  // gpu_params.fix_imp_norm = params.IsImpNormFixed() ? 1 : 0;
  // gpu_params.roughness_low = 0.1f;
  // gpu_params.roughness_high = 8.0f;
  // gpu_params.surfabs_high = 10000.0f;
  // gpu_params.impnorm_high = 10000.0f;
  // gpu_params.param_low = -5.0f;
  // gpu_params.param_high = 5.0f;
  //
  // const int nd = m_datapoints;
  // m_fMeasQ.resize(nd); m_fMeasRefl.resize(nd); m_fMeasErr.resize(nd);
  // m_fMeasSintheta.resize(nd); m_fMeasSinsq.resize(nd);
  // const auto& consts = m_parratt.GetConsts();  // TODO: expose accessor
  // for (int i = 0; i < nd; i++) {
  //   m_fMeasQ[i]       = static_cast<float>(m_xi[i]);
  //   m_fMeasRefl[i]    = static_cast<float>(m_yi[i]);
  //   m_fMeasErr[i]     = static_cast<float>(m_eyi[i]);
  //   m_fMeasSintheta[i]= static_cast<float>(consts.sinthetai[i]);
  //   m_fMeasSinsq[i]   = static_cast<float>(consts.sinsquaredthetai[i]);
  // }
  // const bool use_qspread = consts.qsmear_enabled;
  // ...
  // memset(&meas, 0, sizeof(meas));
  // meas.num_datapoints = nd; meas.objective_function = m_initStruct.Objectivefunction;
  // meas.imp_norm = m_initStruct.Impnorm; meas.xr_only = m_initStruct.XRonly;
  //
  // const int nl = m_cEDP.Get_EDPPointCount();
  // m_fEdSpacing.resize(nl); m_fDistArray.resize(gpu_params.num_boxes + 2);
  // const float leftOffset = static_cast<float>(m_cEDP.Get_LeftOffset());
  // for (int i = 0; i < nl; i++)
  //   m_fEdSpacing[i] = i * static_cast<float>(m_cEDP.Get_Dz()) - leftOffset;
  // constexpr int FilmSlack = 7;
  // for (int k = 0; k < gpu_params.num_boxes + 2; k++)
  //   m_fDistArray[k] = k * (static_cast<float>(m_initStruct.FilmLength) +
  //                          FilmSlack) / static_cast<float>(m_initStruct.Boxes);
  // const float waveConst = static_cast<float>(m_cEDP.Get_WaveConstant());
  // const float lambda    = static_cast<float>(m_initStruct.Wavelength);
  // memset(&edp_config, 0, sizeof(edp_config));
  // edp_config.ed_spacing = m_fEdSpacing.data(); edp_config.dist_array = m_fDistArray.data();
  // edp_config.rho = static_cast<float>(m_initStruct.FilmSLD * 1e-6) * waveConst;
  // edp_config.dz = static_cast<float>(m_cEDP.Get_Dz());
  // edp_config.k0 = 2.0f * std::numbers::pi_v<float> / lambda;
  // edp_config.num_layers = nl; edp_config.use_abs = m_initStruct.UseSurfAbs;
  // if (m_initStruct.UseSurfAbs) {
  //   edp_config.beta     = static_cast<float>(m_initStruct.FilmAbs) * waveConst;
  //   edp_config.beta_sub = static_cast<float>(m_initStruct.SubAbs) * waveConst;
  //   edp_config.beta_sup = static_cast<float>(m_initStruct.SupAbs) * waveConst;
  // }
}

int StochFit::ProcessingGPU() {
  // TODO: rewrite GPU loop to use DisplayState snapshot model.
  // Original implementation preserved below.
  //
  // GpuSAState sa_state; GpuParams gpu_params; GpuMeasurement meas; GpuEDPConfig edp_config;
  // InitGpuData(sa_state, gpu_params, meas, edp_config);
  // auto gpu_info = detect_gpu();
  // const int num_chains = (m_initStruct.GpuChains > 0) ? m_initStruct.GpuChains
  //                                                      : gpu_info.max_chains;
  // m_gpuRunner = GpuSARunner::create(m_gpuBackend);
  // if (!m_gpuRunner) return -1;
  // m_gpuRunner->initialize(sa_state, gpu_params, meas, edp_config, num_chains);
  // constexpr int batch_size = 5000;
  // auto last_update = std::chrono::steady_clock::now();
  // constexpr auto update_interval = std::chrono::seconds(2);
  // const int outer_total = m_itotaliterations;
  // m_itotaliterations = outer_total * num_chains;
  // for (int done = 0; done < outer_total && !m_stop_requested.load(); done += batch_size) {
  //   const int this_batch = std::min(batch_size, outer_total - done);
  //   m_gpuRunner->run_batch(this_batch);
  //   m_icurrentiteration.store((done + this_batch) * num_chains);
  //   const auto now = std::chrono::steady_clock::now();
  //   if ((now - last_update) >= update_interval) {
  //     GpuResultSummary result = m_gpuRunner->get_result();
  //     // TODO: update params + store into DisplayState under m_displayMutex
  //     last_update = now;
  //   }
  // }
  // GpuResultSummary result = m_gpuRunner->get_result();
  // // TODO: final params update + DisplayState snapshot
  // m_gpuRunner.reset();
  // return 0;
  return -1;
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
  s.goodnessOfFit = GetLowestEnergy();
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
