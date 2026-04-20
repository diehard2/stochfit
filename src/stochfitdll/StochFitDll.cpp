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

#include "StochFitDll.h"
#include "SettingsStruct.h"
#include "StochFitHarness.h"
#include "generated/stochfit_generated.h"
#include "gpu/gpu_detect.h"
#include "platform.h"
#include <flatbuffers/flatbuffers.h>

#include <cstring>

// ── Global state
// ──────────────────────────────────────────────────────────────

static std::unique_ptr<StochFit> stochfit;
static std::string g_last_init_error;

// ── Helper: finish a FlatBuffer and copy into caller-supplied buffer
// ────────── Returns bytes written, or -1 if the buffer is too small.
template <typename T>
static int finish_into(flatbuffers::FlatBufferBuilder &fbb,
                       flatbuffers::Offset<T> root, uint8_t *outBuf,
                       int maxLen) {
  fbb.Finish(root);
  int written = static_cast<int>(fbb.GetSize());
  if (written > maxLen)
    return -1;
  std::memcpy(outBuf, fbb.GetBufferPointer(), written);
  return written;
}

// ── Lifecycle
// ─────────────────────────────────────────────────────────────────

extern "C" EXPORT void Init(const uint8_t *buf, int len) {
  g_last_init_error.clear();

  const auto *req = StochFitProto::GetInitRequest(buf);
  ReflSettings rs(req->settings());

  std::unique_ptr<StochRunState> state;
  if (req->state() != nullptr) {
    state = std::make_unique<StochRunState>(req->state());
  }

  stochfit = std::make_unique<StochFit>(rs, state);

  auto result = stochfit->GetInitError();
  if (!result) {
    g_last_init_error = result.error();
    stochfit.reset();
  }
}

extern "C" EXPORT const char *GetInitError() {
  return g_last_init_error.c_str();
}

extern "C" EXPORT void Start(int iterations) {
  if (stochfit) {
    stochfit->Start(iterations);
  }
}

extern "C" EXPORT void Stop() {
  if (stochfit) {
    stochfit->Stop();
  }
}

extern "C" EXPORT void Destroy() { stochfit.reset(); }

extern "C" EXPORT void Cancel() {
  if (stochfit) {
    stochfit->Stop();
  }
  stochfit.reset();
}

// ── SA polling
// ────────────────────────────────────────────────────────────────

extern "C" EXPORT int GetData(uint8_t *outBuf, int maxLen) {
  if (!stochfit)
    return -1;

  const int nd = stochfit->GetDataCount();
  std::vector<double> qRange(nd), refl(nd);
  double roughness = 0, chiSquare = 0, goodnessOfFit = 0;
  int32_t isFinished = 0;

  int iter = stochfit->GetData(qRange.data(), refl.data(),
                               &roughness, &chiSquare, &goodnessOfFit, &isFinished);

  // Generate display EDP at full resolution from current best genome
  CEDP display_edp;
  display_edp.Init(stochfit->Settings());
  ParamVector genome = stochfit->GetParams();
  display_edp.GenerateEDP(genome);

  const int n = display_edp.Get_EDPPointCount();
  std::vector<double> zRange(n), rho(n);
  for (int i = 0; i < n; i++) {
    zRange[i] = i * display_edp.Get_Dz() - display_edp.Get_LeftOffset();
    rho[i]    = display_edp.m_EDP[i].real() / display_edp.m_EDP[n - 1].real();
  }

  flatbuffers::FlatBufferBuilder fbb(static_cast<size_t>(maxLen));
  auto zVec   = fbb.CreateVector(zRange);
  auto rhoVec = fbb.CreateVector(rho);
  auto qVec   = fbb.CreateVector(qRange);
  auto rfVec  = fbb.CreateVector(refl);

  auto result = StochFitProto::CreateGetDataResult(
      fbb, zVec, rhoVec, qVec, rfVec, roughness, chiSquare, goodnessOfFit,
      isFinished != 0, iter);

  return finish_into(fbb, result, outBuf, maxLen);
}

extern "C" EXPORT int GetRunState(uint8_t *outBuf, int maxLen) {
  if (!stochfit)
    return -1;

  double saScalars[9] = {};
  // size the ed buffer generously; GetRunState fills edCount with actual count
  std::vector<double> edValues(256, 0.0);
  int edCount = 0;
  stochfit->GetRunState(saScalars, edValues.data(), &edCount);
  edValues.resize(static_cast<size_t>(edCount));

  flatbuffers::FlatBufferBuilder fbb(4096);
  auto edVec = fbb.CreateVector(edValues);
  auto result = StochFitProto::CreateGetRunStateResult(
      fbb, saScalars[0], saScalars[1], saScalars[2], saScalars[3], saScalars[4],
      saScalars[5], saScalars[6], saScalars[7], saScalars[8], edVec,
      0 /*iteration filled by caller*/);

  return finish_into(fbb, result, outBuf, maxLen);
}

extern "C" EXPORT int SAParams(uint8_t *outBuf, int maxLen) {
  if (!stochfit)
    return -1;

  double lowestEnergy = 0, temp = 0;
  int mode = 0;
  temp = stochfit->GetTemperature();
  lowestEnergy = stochfit->GetLowestEnergy();
  mode = (temp < 1e-20) ? -1 : 1;

  flatbuffers::FlatBufferBuilder fbb(64);
  auto result =
      StochFitProto::CreateSaParamsResult(fbb, lowestEnergy, temp, mode);
  return finish_into(fbb, result, outBuf, maxLen);
}

extern "C" EXPORT bool GpuAvailable() { return is_gpu_available(); }
