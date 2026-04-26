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

#pragma once

// Top-level SA orchestrator.
// Manages the worker thread, GPU dispatch (CUDA/Metal when UseGpu=true),
// and data marshaling between C++ and the FFI layer.
// Session file I/O is handled by Electron: Init() accepts an optional
// StochRunState* with pre-parsed state (null = fresh start).
// On stop: call Stop() to block until worker exits, then GetRunState() to read
// the raw internal values, then Destroy() to clean up the object.

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <variant>

#include "Algorithm.h"
#include "Anneal.h"
#include "AnnealPolicies.h"
#include "CEDP.h"
#include "ParamVector.h"
#include "ParameterStepper.h"
#include "ReflCalc.h"
#include "ReflectivityObjective.h"
#include "UnifiedReflectivity.h"
#include <stochfit/SettingsStruct.h>

class GpuSARunner;
struct GpuSAState;
struct GpuParams;
struct GpuMeasurement;
struct GpuEDPConfig;

using AnnealVariant = std::variant<Anneal<GreedyPolicy>,
                                   Anneal<SimulatedPolicy>, Anneal<StunPolicy>>;

struct DataSnapshot {
  std::vector<double> Q;
  std::vector<double> refl;
  std::vector<double> z;
  std::vector<double> rho;
  double roughness     = 0.0;
  double chiSquare     = 0.0;
  double goodnessOfFit = 0.0;
  bool   isFinished    = false;
  int    iteration     = 0;
};

class StochFit {
public:
  StochFit(const ReflSettings &InitStruct,
           const std::unique_ptr<StochRunState> &state = {});
  ~StochFit();
  int Start(int iterations);
  int Cancel();
  void Stop();
  DataSnapshot GetData();
  StochRunState GetRunState();
  tl::expected<void, std::string> GetInitError() const { return m_initError; }

  const ReflSettings& Settings()      const { return m_initStruct; }
  int                 GetDataCount()  const { return m_cRefl.GetDataCount(); }

  // Harness-level accessors for the annealer (used by GPU seam and FFI).
  // GetTemperature()    = 1/β  (display value, same as old Get_Temp())
  // GetRawTemperature() = β    (for session save, same as old Get_RawTemp())
  // SetTemperature(β)   = set β directly (session restore)
  double GetTemperature() const;
  double GetRawTemperature() const;
  void SetTemperature(double t);
  double GetLowestEnergy() const;
  double GetAverageFSTUN() const;
  void SetAverageFSTUN(double f);

private:
  DataSnapshot GetCurrentState();
  int Processing();
  int ProcessingGPU();
  void InitGpuData(GpuSAState &sa_state, GpuParams &gpu_params,
                   GpuMeasurement &meas, GpuEDPConfig &edp_config);

  tl::expected<void, std::string> m_initError;

  std::unique_ptr<GpuSARunner> m_gpuRunner;
  GpuBackend m_gpuBackend = GpuBackend::None;

  // Float buffers for GPU data transfer
  std::vector<float> m_fMeasSintheta;
  std::vector<float> m_fMeasSinsq;
  std::vector<float> m_fMeasQ;
  std::vector<float> m_fMeasRefl;
  std::vector<float> m_fMeasErr;
  std::vector<float> m_fQspreadSin;
  std::vector<float> m_fQspreadSin2;
  std::vector<float> m_fEdSpacing;
  std::vector<float> m_fDistArray;

  std::thread m_thread;
  std::atomic<bool> m_stop_requested;
  std::atomic<int>  m_icurrentiteration{0};

  string m_Directory;
  int m_itotaliterations = 0;
  int m_iparratlayers    = 0;

  // m_initStruct must be declared before m_parratt, which holds a const ref to it.
  ReflSettings m_initStruct;

  CReflCalc m_cRefl;      // display/LM path — main thread only
  CEDP m_cEDP;            // SA scoring     — worker thread only (via annealer)
  CEDP m_displayEDP;      // display EDP    — main thread only (GetCurrentState)
  ParamVector params;     // SA state       — worker thread only

  // Snapshot of the latest accepted solution, readable from the main thread.
  // Guarded by m_displayMutex; copy only the minimum inside the lock.
  struct DisplayState {
    explicit DisplayState(const ReflSettings &s) : params(s) {}
    ParamVector params;
    std::vector<double> refl; // SA-computed reflectivity at accepted solution
    double chiSquare = 0.0;
    double goF       = 0.0;
  };
  mutable std::mutex m_displayMutex;
  DisplayState m_displayState;

  ParrattReflectivity m_parratt;
  ReflectivityObjective m_objective;
  ParameterStepper m_stepper;

  std::optional<AnnealVariant> m_annealer;

  // SA scratch buffer (sized by m_cRefl.m_idatapoints at init)
  std::vector<double> m_saReflBuf;
};
