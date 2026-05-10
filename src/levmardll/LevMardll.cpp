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
 */

#include "LevMardll.h"
#include "BoxLayerBuild.h"
#include "RhoCalc.h"
#include "Settings.h"
#include "SettingsBridge.h"
#include "generated/stochfit_generated.h"
#include "platform.h"
#include "stochfit/ReflectivityObjective.h"
#include "stochfit/UnifiedReflectivity.h"
#include <flatbuffers/flatbuffers.h>

#include <cmath>
#include <cstring>
#include <levmar/levmar.h>
#include <random>
#include <span>
#include <vector>

// ── Helpers
// ───────────────────────────────────────────────────────────────────

static std::vector<double> copy_fbs_vec(const flatbuffers::Vector<double> *v) {
  if (!v || v->size() == 0)
    return {};
  return std::vector<double>(v->data(), v->data() + v->size());
}

static void fill_box_settings(const StochFitProto::BoxReflSettings *s,
                              BoxReflSettings &rs) {
  rs.Directory = s->directory() ? s->directory()->str() : "";
  rs.Q = copy_fbs_vec(s->q());
  rs.Refl = copy_fbs_vec(s->refl());
  rs.ReflError = copy_fbs_vec(s->refl_error());
  rs.QError = copy_fbs_vec(s->q_error());
  rs.UL = copy_fbs_vec(s->ul());
  rs.LL = copy_fbs_vec(s->ll());
  rs.ParamPercs = copy_fbs_vec(s->param_percs());
  rs.MIEDP = copy_fbs_vec(s->miedp());
  rs.ZIncrement = copy_fbs_vec(s->z_increment());
  rs.QPoints = s->q_points();
  rs.OneSigma = s->one_sigma();
  rs.SubSLD = s->sub_sld();
  rs.SupSLD = s->sup_sld();
  rs.Boxes = s->boxes();
  rs.Wavelength = s->wavelength();
  rs.QSpread = s->q_spread();
  rs.ImpNorm = s->imp_norm();
  rs.FitFunc = s->fit_func();
  rs.LowQOffset = s->low_q_offset();
  rs.HighQOffset = s->high_q_offset();
  rs.Iterations = s->iterations();
  rs.ZLength = s->z_length();
}

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

// ── LevmarReflTask
// ──────────────────────────────────────────────────────────── All state needed
// by the levmar residual callback. Stack-allocated per FFI call.

struct LevmarReflTask {
  ParrattReflectivity parratt;
  ReflectivityObjective objective;
  BoxLayers layers;
  std::span<const double> Realrefl;
  std::span<const double> Realreflerrors;
  const BoxReflSettings *rs = nullptr;
};

// Static C-style callback required by levmar's void* interface.
static void LevmarReflResidual(double *p, double *x, int m, int n, void *data) {
  auto *self = static_cast<LevmarReflTask *>(data);
  BuildBoxLayers(*self->rs, std::span<const double>(p, m), self->rs->OneSigma,
                 self->layers);
  auto refl =
      self->parratt.CalculateReflectivity(self->layers.View(self->rs->Boxes));
  if (self->rs->ImpNorm) {
    const double nf = self->layers.normfactor;
    for (auto &r : refl)
      r *= nf;
  }
  self->objective.FillResiduals(refl, self->Realrefl, self->Realreflerrors,
                                std::span<double>(x, n), self->rs->LowQOffset,
                                self->rs->HighQOffset);
}

// ── FastReflfit
// ───────────────────────────────────────────────────────────────

extern "C" EXPORT int FastReflfit(const uint8_t *inBuf, int /*inLen*/,
                                  uint8_t *outBuf, int maxLen) {
  auto *req = flatbuffers::GetRoot<StochFitProto::LevMarRequest>(inBuf);

  BoxReflSettings rs{};
  fill_box_settings(req->settings(), rs);

  std::vector<double> params = copy_fbs_vec(req->parameters());
  int paramsize = static_cast<int>(params.size());

  double opts[LM_OPTS_SZ] = {LM_INIT_MU, 1e-15, 1e-15, 1e-20, -LM_DIFF_DELTA};

  LevmarReflTask task{
      ParrattReflectivity(ToReflSettings(rs)),
      ReflectivityObjective(ReflectivityObjective::Type{rs.FitFunc}),
      {},
      rs.Refl,
      rs.ReflError,
      &rs};

  std::vector<double> xvec(rs.QPoints, 0.0);
  std::vector<double> work(LM_DIF_WORKSZ(paramsize, rs.QPoints) +
                           paramsize * rs.QPoints);
  auto covar = std::span(work).subspan(LM_DIF_WORKSZ(paramsize, rs.QPoints));
  std::vector<double> info(LM_INFO_SZ, 0.0);

  if (rs.UL.empty())
    dlevmar_dif(LevmarReflResidual, params.data(), xvec.data(), paramsize,
                rs.QPoints, 1000, opts, info.data(), work.data(), covar.data(),
                (void *)(&task));
  else
    dlevmar_bc_dif(LevmarReflResidual, params.data(), xvec.data(), paramsize,
                   rs.QPoints, rs.LL.data(), rs.UL.data(), nullptr, 1000, opts,
                   info.data(), work.data(), covar.data(), (void *)(&task));

  std::vector<double> covarOut(paramsize);
  for (int i = 0; i < paramsize; i++)
    covarOut[i] = std::sqrt(covar[i * (paramsize + 1)]);

  flatbuffers::FlatBufferBuilder fbb(4096);
  auto result = StochFitProto::CreateReflFitResult(
      fbb, fbb.CreateVector(params), fbb.CreateVector(covarOut),
      fbb.CreateVector(info));
  return finish_into(fbb, result, outBuf, maxLen);
}

// ── FastReflGenerate
// ──────────────────────────────────────────────────────────

extern "C" EXPORT int FastReflGenerate(const uint8_t *inBuf, int /*inLen*/,
                                       uint8_t *outBuf, int maxLen) {
  auto *req = flatbuffers::GetRoot<StochFitProto::LevMarRequest>(inBuf);

  BoxReflSettings rs{};
  fill_box_settings(req->settings(), rs);

  std::vector<double> params = copy_fbs_vec(req->parameters());

  ParrattReflectivity parratt(ToReflSettings(rs));
  BoxLayers layers;
  BuildBoxLayers(rs, params, rs.OneSigma, layers);
  auto refl = parratt.CalculateReflectivity(layers.View(rs.Boxes));
  if (rs.ImpNorm) {
    const double nf = layers.normfactor;
    for (auto &r : refl)
      r *= nf;
  }

  std::vector<double> reflOut(refl.begin(), refl.begin() + rs.QPoints);

  flatbuffers::FlatBufferBuilder fbb(4096);
  auto result =
      StochFitProto::CreateReflGenerateResult(fbb, fbb.CreateVector(reflOut));
  return finish_into(fbb, result, outBuf, maxLen);
}

// ── Rhofit ───────────────────────────────────────────────────────────────────

extern "C" EXPORT int Rhofit(const uint8_t *inBuf, int /*inLen*/,
                             uint8_t *outBuf, int maxLen) {
  auto *req = flatbuffers::GetRoot<StochFitProto::LevMarRequest>(inBuf);

  BoxReflSettings rs{};
  fill_box_settings(req->settings(), rs);

  std::vector<double> params = copy_fbs_vec(req->parameters());
  int paramsize = static_cast<int>(params.size());

  double opts[LM_OPTS_SZ] = {LM_INIT_MU, 1e-15, 1e-15, 1e-20, -LM_DIFF_DELTA};

  RhoCalc Rho;
  Rho.init(rs);

  std::vector<double> xvec(rs.ZLength, 0.0);
  std::vector<double> work(LM_DIF_WORKSZ(paramsize, rs.ZLength) +
                           paramsize * rs.ZLength);
  auto covar = std::span(work).subspan(LM_DIF_WORKSZ(paramsize, rs.ZLength));
  std::vector<double> info(LM_INFO_SZ, 0.0);

  if (rs.UL.empty())
    dlevmar_dif(RhoCalc::objective, params.data(), xvec.data(), paramsize,
                rs.ZLength, 1000, opts, info.data(), work.data(), covar.data(),
                (void *)(&Rho));
  else
    dlevmar_bc_dif(RhoCalc::objective, params.data(), xvec.data(), paramsize,
                   rs.ZLength, rs.LL.data(), rs.UL.data(), nullptr, 1000, opts,
                   info.data(), work.data(), covar.data(), (void *)(&Rho));

  std::vector<double> covarOut(paramsize);
  for (int i = 0; i < paramsize; i++)
    covarOut[i] = std::sqrt(covar[i * (paramsize + 1)]);

  flatbuffers::FlatBufferBuilder fbb(4096);
  auto result = StochFitProto::CreateRhoFitResult(fbb, fbb.CreateVector(params),
                                                  fbb.CreateVector(covarOut),
                                                  fbb.CreateVector(info));
  return finish_into(fbb, result, outBuf, maxLen);
}

// ── RhoGenerate
// ───────────────────────────────────────────────────────────────

extern "C" EXPORT int RhoGenerate(const uint8_t *inBuf, int /*inLen*/,
                                  uint8_t *outBuf, int maxLen) {
  auto *req = flatbuffers::GetRoot<StochFitProto::LevMarRequest>(inBuf);

  BoxReflSettings rs{};
  fill_box_settings(req->settings(), rs);

  std::vector<double> params = copy_fbs_vec(req->parameters());

  RhoCalc Rho;
  Rho.init(rs);
  Rho.mkdensity(params);
  Rho.mkdensityboxmodel(params);

  std::vector<double> ed(Rho.nk.begin(), Rho.nk.begin() + rs.ZLength);
  std::vector<double> boxED(Rho.nkb.begin(), Rho.nkb.begin() + rs.ZLength);

  flatbuffers::FlatBufferBuilder fbb(8192);
  auto result = StochFitProto::CreateRhoGenerateResult(
      fbb, fbb.CreateVector(ed), fbb.CreateVector(boxED));
  return finish_into(fbb, result, outBuf, maxLen);
}

// ── StochFitBoxModel helpers
// ──────────────────────────────────────────────────

struct BoxSolution {
  std::vector<double> params;
  std::vector<double> covar; // per-parameter sigma: sqrt(|diag(covariance)|)
  std::array<double, LM_INFO_SZ> info{};
  double score = 1e300;

  bool operator<(const BoxSolution &o) const { return score < o.score; }
};

static bool IsReasonable(const BoxSolution &sol, const BoxReflSettings &rs,
                         double cutoff) {
  BoxLayers layers;
  BuildBoxLayers(rs, sol.params, rs.OneSigma, layers);
  for (int i = 1; i <= rs.Boxes; ++i) {
    if (layers.length_mult[i].imag() > 0.0)
      return false;
    if (layers.rho[i].real() < 0.0)
      return false;
  }
  if (rs.OneSigma && cutoff > 0.0) {
    for (int i = 0; i < (int)sol.params.size(); ++i) {
      if (sol.covar[i] > cutoff * std::fabs(sol.params[i]))
        return false;
    }
  }
  return true;
}

static bool ApproxEqual(const BoxSolution &a, const BoxSolution &b,
                        double tol = 0.005) {
  if (std::fabs(a.score / b.score - 1.0) > tol)
    return false;
  for (size_t i = 0; i < a.params.size(); ++i) {
    if (b.params[i] != 0.0 && std::fabs(a.params[i] / b.params[i] - 1.0) > tol)
      return false;
  }
  return true;
}

// ── StochFitBoxModel
// ──────────────────────────────────────────────────────────

extern "C" EXPORT int StochFitBoxModel(const uint8_t *inBuf, int /*inLen*/,
                                       uint8_t *outBuf, int maxLen) {
  auto *req = flatbuffers::GetRoot<StochFitProto::LevMarRequest>(inBuf);

  BoxReflSettings rs{};
  fill_box_settings(req->settings(), rs);

  std::vector<double> params = copy_fbs_vec(req->parameters());
  int paramsize = static_cast<int>(params.size());
  int QSize = rs.QPoints;

  double opts[LM_OPTS_SZ] = {LM_INIT_MU, 1e-15, 1e-15, 1e-20, -LM_DIFF_DELTA};

  // Compute initial chi-square using the selected FitFunc (same metric as the
  // fits).
  double bestchisquare = 0;
  {
    LevmarReflTask tmp{.parratt = ParrattReflectivity(ToReflSettings(rs)),
                       .objective = ReflectivityObjective(
                           ReflectivityObjective::Type{rs.FitFunc}),
                       .layers = {},
                       .Realrefl = rs.Refl,
                       .Realreflerrors = rs.ReflError,
                       .rs = &rs};
    BuildBoxLayers(rs, params, rs.OneSigma, tmp.layers);
    auto r = tmp.parratt.CalculateReflectivity(tmp.layers.View(rs.Boxes));
    if (rs.ImpNorm)
      for (auto &rv : r)
        rv *= tmp.layers.normfactor;
    std::vector<double> residuals(QSize, 0.0);
    tmp.objective.FillResiduals(r, rs.Refl, rs.ReflError, residuals,
                                rs.LowQOffset, rs.HighQOffset);
    for (double res : residuals)
      bestchisquare += res * res;
  }

  BoxSolution original;
  original.params = params;
  original.covar.assign(paramsize, 0.0);
  original.info[1] = bestchisquare;
  original.score = bestchisquare;

  std::vector<BoxSolution> temp;
  temp.reserve(6000);

  omp_set_num_threads(omp_get_num_procs());

#pragma omp parallel
  {
    LevmarReflTask task{.parratt = ParrattReflectivity(ToReflSettings(rs)),
                        .objective = ReflectivityObjective(
                            ReflectivityObjective::Type{rs.FitFunc}),
                        .layers = {},
                        .Realrefl = rs.Refl,
                        .Realreflerrors = rs.ReflError,
                        .rs = &rs};

    std::mt19937 randgen(std::random_device{}() + omp_get_thread_num());
    auto IRandom = [&](double max, double min) {
      return std::uniform_real_distribution<double>(min, max)(randgen);
    };

    BoxSolution localanswer;
    localanswer.params.resize(paramsize);
    localanswer.covar.resize(paramsize);
    std::vector<double> locparameters(paramsize);
    std::vector<BoxSolution> localvec;
    localvec.reserve(1000);

    std::vector<double> locinfo(LM_INFO_SZ);
    std::vector<double> work(LM_DIF_WORKSZ(paramsize, QSize) +
                             paramsize * QSize);
    auto covar = std::span(work).subspan(LM_DIF_WORKSZ(paramsize, QSize));
    std::vector<double> xvec(QSize, 0.0);

#pragma omp for schedule(runtime)
    for (int i = 0; i < rs.Iterations; i++) {
      locparameters[0] =
          IRandom(params[0] * rs.ParamPercs[4], params[0] * rs.ParamPercs[5]);
      for (int k = 0; k < rs.Boxes; k++) {
        if (rs.OneSigma) {
          locparameters[2 * k + 1] =
              IRandom(params[2 * k + 1] * rs.ParamPercs[0],
                      params[2 * k + 1] * rs.ParamPercs[1]);
          locparameters[2 * k + 2] =
              IRandom(params[2 * k + 2] * rs.ParamPercs[2],
                      params[2 * k + 2] * rs.ParamPercs[3]);
        } else {
          locparameters[3 * k + 1] =
              IRandom(params[3 * k + 1] * rs.ParamPercs[0],
                      params[3 * k + 1] * rs.ParamPercs[1]);
          locparameters[3 * k + 2] =
              IRandom(params[3 * k + 2] * rs.ParamPercs[2],
                      params[3 * k + 2] * rs.ParamPercs[3]);
          locparameters[3 * k + 3] =
              IRandom(params[3 * k + 3] * rs.ParamPercs[4],
                      params[3 * k + 3] * rs.ParamPercs[5]);
        }
      }
      locparameters[paramsize - 1] = params[paramsize - 1];

      if (rs.UL.empty())
        dlevmar_dif(LevmarReflResidual, locparameters.data(), xvec.data(),
                    paramsize, QSize, 500, opts, locinfo.data(), work.data(),
                    covar.data(), (void *)(&task));
      else
        dlevmar_bc_dif(LevmarReflResidual, locparameters.data(), xvec.data(),
                       paramsize, QSize, rs.LL.data(), rs.UL.data(), nullptr,
                       500, opts, locinfo.data(), work.data(), covar.data(),
                       (void *)(&task));

      localanswer.params = locparameters;
      for (int j = 0; j < paramsize; ++j)
        localanswer.covar[j] = std::sqrt(std::fabs(covar[j * (paramsize + 1)]));
      std::copy(locinfo.begin(), locinfo.end(), localanswer.info.begin());
      localanswer.score = locinfo[1];

      if (locinfo[1] < bestchisquare &&
          IsReasonable(localanswer, rs, rs.ParamPercs[6])) {
        bool unique = true;
        for (const auto &v : localvec) {
          if (ApproxEqual(localanswer, v)) {
            unique = false;
            break;
          }
        }
        if (unique)
          localvec.push_back(localanswer);
      }
    }

#pragma omp critical(AddVecs)
    {
      for (const auto &v : localvec)
        temp.push_back(v);
    }
  }

  temp.push_back(original);

  std::vector<BoxSolution> allsolutions;
  allsolutions.reserve(6000);
  int tempsize = static_cast<int>(temp.size());
  allsolutions.push_back(temp[0]);
  for (int i = 1; i < tempsize; i++) {
    int sz = static_cast<int>(allsolutions.size());
    for (int j = 0; j < sz; j++) {
      if (ApproxEqual(temp[i], allsolutions[j]))
        break;
      if (j == sz - 1)
        allsolutions.push_back(temp[i]);
    }
  }

  if (!allsolutions.empty())
    std::sort(allsolutions.begin(), allsolutions.end());

  int n = static_cast<int>(std::min<size_t>(allsolutions.size(), 999));
  std::vector<double> outParams(params);
  std::vector<double> covarArray(static_cast<size_t>(n * paramsize));
  std::vector<double> infoOut(static_cast<size_t>(n * LM_INFO_SZ));
  std::vector<double> paramArray(static_cast<size_t>(n * paramsize));
  std::vector<double> chiSquareArray(n);

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < paramsize; j++) {
      paramArray[i * paramsize + j] = allsolutions[i].params[j];
      covarArray[i * paramsize + j] = allsolutions[i].covar[j];
    }
    std::copy(allsolutions[i].info.begin(), allsolutions[i].info.end(),
              infoOut.begin() + i * LM_INFO_SZ);
    chiSquareArray[i] = allsolutions[i].score;
  }

  flatbuffers::FlatBufferBuilder fbb(static_cast<size_t>(maxLen));
  auto result = StochFitProto::CreateBoxStochFitResult(
      fbb, fbb.CreateVector(outParams), fbb.CreateVector(covarArray),
      fbb.CreateVector(infoOut), fbb.CreateVector(paramArray),
      fbb.CreateVector(chiSquareArray), n);
  return finish_into(fbb, result, outBuf, maxLen);
}
