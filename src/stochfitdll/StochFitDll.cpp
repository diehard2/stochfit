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

  const DataSnapshot snap = stochfit->GetData();

  flatbuffers::FlatBufferBuilder fbb(static_cast<size_t>(maxLen));
  auto zVec   = fbb.CreateVector(snap.z);
  auto rhoVec = fbb.CreateVector(snap.rho);
  auto qVec   = fbb.CreateVector(snap.Q);
  auto rfVec  = fbb.CreateVector(snap.refl);

  auto result = StochFitProto::CreateGetDataResult(
      fbb, zVec, rhoVec, qVec, rfVec,
      snap.roughness, snap.chiSquare, snap.goodnessOfFit,
      snap.isFinished, snap.iteration);

  return finish_into(fbb, result, outBuf, maxLen);
}

extern "C" EXPORT int GetRunState(uint8_t *outBuf, int maxLen) {
  if (!stochfit)
    return -1;

  const StochRunState s = stochfit->GetRunState();
  flatbuffers::FlatBufferBuilder fbb(4096);
  auto edVec = fbb.CreateVector(s.edValues);
  auto result = StochFitProto::CreateGetRunStateResult(
      fbb, s.roughness, s.filmAbsInput, s.surfAbs, s.temperature, s.impNorm,
      s.avgfSTUN, s.bestSolution, s.chiSquare, s.goodnessOfFit, edVec,
      0 /*iteration filled by caller*/);

  return finish_into(fbb, result, outBuf, maxLen);
}

extern "C" EXPORT int SAParams(uint8_t *outBuf, int maxLen) {
  if (!stochfit)
    return -1;

  int mode = 0;
  double temp = stochfit->GetTemperature();
  double lowestEnergy = stochfit->GetLowestEnergy();
  mode = (temp < 1e-20) ? -1 : 1;

  flatbuffers::FlatBufferBuilder fbb(64);
  auto result =
      StochFitProto::CreateSaParamsResult(fbb, lowestEnergy, temp, mode);
  return finish_into(fbb, result, outBuf, maxLen);
}

extern "C" EXPORT bool GpuAvailable() { return is_gpu_available(); }
