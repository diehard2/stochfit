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
#include "ReflCalc.h"
#include "platform.h"

#include "gpu/gpu_detect.h"
#include "gpu/gpu_sa_runner.h"

// ── Constructor
// ───────────────────────────────────────────────────────────────

StochFit::StochFit(const ReflSettings &InitStruct,
                   const std::unique_ptr<StochRunState> &state)
    // m_initStruct FIRST — m_parratt holds a const ref to it.
    : m_initStruct(InitStruct), params(InitStruct), m_parratt(m_initStruct),
      m_objective(ReflectivityObjective::Type{InitStruct.Objectivefunction}),
      m_stepper({.sigmaSearch = InitStruct.Sigmasearch,
                 .absSearch = InitStruct.AbsorptionSearchPerc,
                 .normSearch = InitStruct.NormalizationSearchPerc,
                 .stepSize = InitStruct.Paramtemp}) {
  m_bupdated = false;
  m_stop_requested = false;

  m_Directory = InitStruct.Directory;

  // Initialize the display/LM reflectivity path (CReflCalc).
  if (auto r = m_cRefl.Init(InitStruct); !r) {
    m_initError = r;
    return;
  }
  {
    ReflSettings sa_rs = m_initStruct;
    sa_rs.Resolution = std::max(1, (int)std::ceil(m_initStruct.Q.back() * 2.0 / std::numbers::pi));
    m_cEDP.Init(sa_rs);
  }

  Qinc.resize(m_cRefl.GetDataCount());
  Refl.resize(m_cRefl.GetDataCount());

  // Apply run state if provided.
  // filmAbsInput is the pre-multiplication value: Set_FilmAbs(x) stores x*WC.
  // temperature is raw m_dTemp (β) stored directly via SetTemperature.
  // surfAbs is saved independently so it is never baked into filmAbsInput.
  if (state &&
      static_cast<int>(state->edValues.size()) == params.RealparamsSize()) {
    params.setroughness(state->roughness);
    params.SetSupphase(state->edValues[0]);
    for (int i = 1; i < static_cast<int>(state->edValues.size()) - 1; i++)
      params.SetMutatableParameter(i - 1, state->edValues[i]);
    params.SetSubphase(state->edValues[state->edValues.size() - 1]);
    m_cEDP.Set_FilmAbs(state->filmAbsInput);
    params.setSurfAbs(state->surfAbs);
    params.setImpNorm(state->impNorm);
  }
  params.UpdateBoundaries();

  // Build SA scratch buffer and deps.
  const int nd = m_cRefl.m_idatapoints;
  m_saReflBuf.resize(nd);
  AnnealDeps deps;
  deps.yi = m_cRefl.yi;
  deps.eyi = m_cRefl.eyi;
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
      static_cast<int>(state->edValues.size()) == params.RealparamsSize()) {
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
    if (m_initStruct.UseGpu) {
      auto gpu_info = detect_gpu();
      if (gpu_info.backend != GpuBackend::None) {
        m_gpuBackend = gpu_info.backend;
        return ProcessingGPU();
      }
    }

    if (!m_annealer)
      return -1;

    for (int isteps = 0;
         isteps < m_itotaliterations && !m_stop_requested.load(); ++isteps) {

      const bool accepted =
          std::visit([&](auto &a) { return a.Iteration(params); }, *m_annealer);

      if (accepted || isteps == 0) {
        m_dChiSquare = std::visit(
            [](const auto &a) { return a.GetLastChiSquare(); }, *m_annealer);
        m_dGoodnessOfFit = std::visit(
            [](const auto &a) { return a.GetCurrentEnergy(); }, *m_annealer);
      }
      UpdateFits(isteps);
    }

    UpdateFits(m_icurrentiteration);
    return 0;
  } catch (const std::exception &ex) {
    std::cerr << "[StochFit] Processing() caught exception: " << ex.what()
              << std::endl;
    m_icurrentiteration = m_itotaliterations > 0 ? m_itotaliterations - 1 : 0;
    m_bupdated = false;
    return -1;
  } catch (...) {
    std::cerr << "[StochFit] Processing() caught unknown exception"
              << std::endl;
    m_icurrentiteration = m_itotaliterations > 0 ? m_itotaliterations - 1 : 0;
    m_bupdated = false;
    return -1;
  }
}

// ── GPU support
// ───────────────────────────────────────────────────────────────

void StochFit::InitGpuData(GpuSAState &sa_state, GpuParams &gpu_params,
                           GpuMeasurement &meas, GpuEDPConfig &edp_config) {
  memset(&sa_state, 0, sizeof(sa_state));
  // GetTemperature() = 1/β; GPU wants β = 1/T.
  sa_state.temperature = 1.0f / static_cast<float>(GetTemperature());
  sa_state.best_energy = static_cast<float>(GetLowestEnergy());
  sa_state.current_energy = sa_state.best_energy;
  sa_state.best_solution = sa_state.best_energy;
  sa_state.slope = static_cast<float>(m_initStruct.Slope);
  sa_state.gamma = static_cast<float>(m_initStruct.Gamma);
  sa_state.avg_fstun = static_cast<float>(m_initStruct.Inittemp);
  sa_state.gammadec = static_cast<float>(m_initStruct.Gammadec);
  sa_state.stepsize = static_cast<float>(m_initStruct.Paramtemp);
  sa_state.algorithm = m_initStruct.Algorithm;
  sa_state.iteration = 0;
  sa_state.plat_time = m_initStruct.Platiter;
  sa_state.temp_iter = m_initStruct.Tempiter;
  sa_state.stun_func = m_initStruct.STUNfunc;
  sa_state.stun_dec_iter = m_initStruct.STUNdeciter;
  sa_state.adaptive = m_initStruct.Adaptive;
  sa_state.sigmasearch = m_initStruct.Sigmasearch;
  sa_state.normsearch = m_initStruct.NormalizationSearchPerc;
  sa_state.abssearch = m_initStruct.AbsorptionSearchPerc;

  memset(&gpu_params, 0, sizeof(gpu_params));
  int rps = params.RealparamsSize();
  gpu_params.real_params_size = rps;
  gpu_params.num_boxes = params.GetInitializationLength();
  for (int i = 0; i < rps; i++)
    gpu_params.sld_values[i] = static_cast<float>(params.GetRealparams(i));
  gpu_params.roughness = static_cast<float>(params.getroughness());
  gpu_params.surf_abs = static_cast<float>(params.getSurfAbs());
  gpu_params.imp_norm = static_cast<float>(params.getImpNorm());
  gpu_params.fix_roughness = params.Get_FixedRoughness() ? 1 : 0;
  gpu_params.use_surf_abs = params.Get_UseSurfAbs() ? 1 : 0;
  gpu_params.fix_imp_norm = params.Get_FixImpNorm() ? 1 : 0;
  gpu_params.roughness_low = 0.1f;
  gpu_params.roughness_high = 8.0f;
  gpu_params.surfabs_high = 10000.0f;
  gpu_params.impnorm_high = 10000.0f;
  gpu_params.param_low = -5.0f;
  gpu_params.param_high = 5.0f;

  const int nd = m_cRefl.m_idatapoints;
  m_fMeasQ.resize(nd);
  m_fMeasRefl.resize(nd);
  m_fMeasErr.resize(nd);
  m_fMeasSintheta.resize(nd);
  m_fMeasSinsq.resize(nd);
  for (int i = 0; i < nd; i++) {
    m_fMeasQ[i] = static_cast<float>(m_cRefl.xi[i]);
    m_fMeasRefl[i] = static_cast<float>(m_cRefl.yi[i]);
    m_fMeasErr[i] = static_cast<float>(m_cRefl.eyi[i]);
    m_fMeasSintheta[i] = static_cast<float>(m_cRefl.sinthetai[i]);
    m_fMeasSinsq[i] = static_cast<float>(m_cRefl.sinsquaredthetai[i]);
  }

  const bool use_qspread =
      (m_cRefl.m_dQSpread > 0.0f && m_cRefl.exi.has_value());
  if (use_qspread) {
    m_fQspreadSin.resize(m_cRefl.qspreadsinthetai.size());
    m_fQspreadSin2.resize(m_cRefl.qspreadsinthetai.size());
    for (int i = 0; i < static_cast<int>(m_cRefl.qspreadsinthetai.size());
         i++) {
      m_fQspreadSin[i] = static_cast<float>(m_cRefl.qspreadsinthetai[i]);
      m_fQspreadSin2[i] =
          static_cast<float>(m_cRefl.qspreadsinsquaredthetai[i]);
    }
  }

  memset(&meas, 0, sizeof(meas));
  meas.q_values = m_fMeasQ.data();
  meas.refl_values = m_fMeasRefl.data();
  meas.refl_errors = m_fMeasErr.data();
  meas.sintheta = m_fMeasSintheta.data();
  meas.sinsquaredtheta = m_fMeasSinsq.data();
  meas.qspread_sintheta = use_qspread ? m_fQspreadSin.data() : nullptr;
  meas.qspread_sin2theta = use_qspread ? m_fQspreadSin2.data() : nullptr;
  meas.num_datapoints = nd;
  meas.objective_function = m_cRefl.objectivefunction;
  meas.use_qspread = use_qspread ? 1 : 0;
  meas.imp_norm = m_initStruct.Impnorm;
  meas.xr_only = m_initStruct.XRonly;

  const int nl = m_cEDP.Get_EDPPointCount();
  m_fEdSpacing.resize(nl);
  m_fDistArray.resize(gpu_params.num_boxes + 2);

  const float leftOffset = static_cast<float>(m_cEDP.Get_LeftOffset());
  for (int i = 0; i < nl; i++)
    m_fEdSpacing[i] = i * static_cast<float>(m_cEDP.Get_Dz()) - leftOffset;

  constexpr int FilmSlack = 7;
  for (int k = 0; k < gpu_params.num_boxes + 2; k++)
    m_fDistArray[k] = k *
                      (static_cast<float>(m_initStruct.FilmLength) +
                       static_cast<float>(FilmSlack)) /
                      static_cast<float>(m_initStruct.Boxes);

  const float waveConst = static_cast<float>(m_cEDP.Get_WaveConstant());
  const float lambda = static_cast<float>(m_initStruct.Wavelength);

  memset(&edp_config, 0, sizeof(edp_config));
  edp_config.ed_spacing = m_fEdSpacing.data();
  edp_config.dist_array = m_fDistArray.data();
  edp_config.rho = static_cast<float>(m_initStruct.FilmSLD * 1e-6) * waveConst;
  edp_config.dz = static_cast<float>(m_cEDP.Get_Dz());
  edp_config.k0 = 2.0f * std::numbers::pi_v<float> / lambda;
  edp_config.num_layers = nl;
  edp_config.use_abs = m_initStruct.UseSurfAbs;
  if (m_initStruct.UseSurfAbs) {
    edp_config.beta = static_cast<float>(m_initStruct.FilmAbs) * waveConst;
    edp_config.beta_sub = static_cast<float>(m_initStruct.SubAbs) * waveConst;
    edp_config.beta_sup = static_cast<float>(m_initStruct.SupAbs) * waveConst;
  }
}

int StochFit::ProcessingGPU() {
  GpuSAState sa_state;
  GpuParams gpu_params;
  GpuMeasurement meas;
  GpuEDPConfig edp_config;
  InitGpuData(sa_state, gpu_params, meas, edp_config);

  auto gpu_info = detect_gpu();
  const int num_chains = (m_initStruct.GpuChains > 0) ? m_initStruct.GpuChains
                                                      : gpu_info.max_chains;

  m_gpuRunner = GpuSARunner::create(m_gpuBackend);
  if (!m_gpuRunner)
    return -1;

  m_gpuRunner->initialize(sa_state, gpu_params, meas, edp_config, num_chains);

  constexpr int batch_size = 5000;
  auto last_update = std::chrono::steady_clock::now();
  constexpr auto update_interval = std::chrono::seconds(2);

  const int outer_total = m_itotaliterations;
  m_itotaliterations = outer_total * num_chains;

  for (int done = 0; done < outer_total && !m_stop_requested.load();
       done += batch_size) {

    const int this_batch = std::min(batch_size, outer_total - done);
    m_gpuRunner->run_batch(this_batch);
    m_icurrentiteration = (done + this_batch) * num_chains;

    const auto now = std::chrono::steady_clock::now();
    if (m_bupdated || (now - last_update) >= update_interval) {
      GpuResultSummary result = m_gpuRunner->get_result();

      m_dRoughness = static_cast<double>(result.best_roughness);
      m_dGoodnessOfFit = static_cast<double>(result.best_gof);

      const int rps = params.RealparamsSize();
      for (int i = 0; i < rps && i < static_cast<int>(GPU_MAX_BOXES + 2); i++) {
        if (i == 0)
          params.SetSupphase(result.best_params[i]);
        else if (i == rps - 1)
          params.SetSubphase(result.best_params[i]);
        else
          params.SetMutatableParameter(i - 1, result.best_params[i]);
      }
      params.setroughness(result.best_roughness);
      if (params.Get_UseSurfAbs())
        params.setSurfAbs(result.best_surf_abs);
      if (params.Get_FixImpNorm())
        params.setImpNorm(result.best_imp_norm);

      // GPU temperature is β; SetTemperature takes β directly.
      SetTemperature(1.0 / static_cast<double>(result.best_temperature));
      SetAverageFSTUN(static_cast<double>(result.best_avg_fstun));

      m_cEDP.GenerateEDP(params);
      m_cRefl.ComputeRF(m_cEDP);
      m_dChiSquare = m_cRefl.m_dChiSquare;

      for (int i = 0; i < static_cast<int>(Refl.size()); i++) {
        if (m_cRefl.m_dQSpread > 0.0) {
          Refl[i] = m_cRefl.reflpt[i];
          Qinc[i] = m_cRefl.xi[i];
        } else {
          Refl[i] = m_cRefl.dataout[i];
          Qinc[i] = m_cRefl.qarray[i];
        }
      }

      m_bupdated = false;
      last_update = now;
    }
  }

  // Final update
  GpuResultSummary result = m_gpuRunner->get_result();
  m_dRoughness = static_cast<double>(result.best_roughness);
  m_dGoodnessOfFit = static_cast<double>(result.best_gof);

  const int rps = params.RealparamsSize();
  for (int i = 0; i < rps && i < static_cast<int>(GPU_MAX_BOXES + 2); i++) {
    if (i == 0)
      params.SetSupphase(result.best_params[i]);
    else if (i == rps - 1)
      params.SetSubphase(result.best_params[i]);
    else
      params.SetMutatableParameter(i - 1, result.best_params[i]);
  }
  params.setroughness(result.best_roughness);
  if (params.Get_UseSurfAbs())
    params.setSurfAbs(result.best_surf_abs);
  if (params.Get_FixImpNorm())
    params.setImpNorm(result.best_imp_norm);

  m_cEDP.GenerateEDP(params);
  m_cRefl.ComputeRF(m_cEDP);
  m_dChiSquare = m_cRefl.m_dChiSquare;

  for (int i = 0; i < static_cast<int>(Refl.size()); i++) {
    if (m_cRefl.m_dQSpread > 0.0) {
      Refl[i] = m_cRefl.reflpt[i];
      Qinc[i] = m_cRefl.xi[i];
    } else {
      Refl[i] = m_cRefl.dataout[i];
      Qinc[i] = m_cRefl.qarray[i];
    }
  }

  m_gpuRunner.reset();
  return 0;
}

// ── Display update
// ────────────────────────────────────────────────────────────

void StochFit::UpdateFits(int currentiteration) {
  if (m_bupdated) {
    m_cEDP.GenerateEDP(params);
    m_cRefl.ComputeRF(m_cEDP);
    m_dRoughness = params.getroughness();

    for (int i = 0; i < static_cast<int>(Refl.size()); i++) {
#ifndef CHECKREFLCALC
      if (m_cRefl.m_dQSpread > 0.0) {
        Refl[i] = m_cRefl.reflpt[i];
        Qinc[i] = m_cRefl.xi[i];
      } else {
        Refl[i] = m_cRefl.dataout[i];
        Qinc[i] = m_cRefl.qarray[i];
      }
#else
      Qinc[i] = m_cRefl.xi[i];
      Refl[i] = m_cRefl.reflpt[i];
#endif
    }
    m_bupdated = false;
  }
  m_icurrentiteration = currentiteration;
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

void StochFit::GetRunState(double *saScalars, double *edValues, int *edCount) {
  saScalars[0] = params.getroughness();
  saScalars[1] = m_cEDP.Get_FilmAbsInput();
  saScalars[2] = params.getSurfAbs();
  saScalars[3] = GetRawTemperature();
  saScalars[4] = params.getImpNorm();
  saScalars[5] = GetAverageFSTUN();
  saScalars[6] = GetLowestEnergy();
  saScalars[7] = m_dChiSquare;
  saScalars[8] = m_dGoodnessOfFit;

  const int count = params.RealparamsSize();
  for (int i = 0; i < count; i++)
    edValues[i] = params.GetRealparams(i);
  *edCount = count;
}

int StochFit::GetData(double *Q, double *ReflOut,
                      double *roughness, double *chisquare,
                      double *goodnessoffit, int32_t *isfinished) {
  const bool done = (m_icurrentiteration >= m_itotaliterations - 1);
  if (!done) {
    m_bupdated = true;
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (m_bupdated.load() && std::chrono::steady_clock::now() < deadline)
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    m_bupdated = false;
  }

  for (int i = 0; i < static_cast<int>(Refl.size()); i++) {
    Q[i] = Qinc[i];
    ReflOut[i] = Refl[i];
  }

  *roughness = m_dRoughness;
  *chisquare = m_dChiSquare;
  *goodnessoffit = m_dGoodnessOfFit;
  *isfinished = (m_icurrentiteration >= m_itotaliterations - 1) ? 1 : 0;

  return m_icurrentiteration;
}
