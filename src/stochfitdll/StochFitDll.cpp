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

#include "platform.h"
#include "StochFitDll.h"
#include "StochFitHarness.h"
#include "gpu/gpu_detect.h"
#include "generated/stochfit_generated.h"
#include <flatbuffers/flatbuffers.h>

#include <cstring>
#include <vector>
#include <string>

// ── Global state ──────────────────────────────────────────────────────────────

static StochFit* stochfit = nullptr;
static std::string g_last_init_error;

// Owned copies of the Q/Refl/ReflError/QError arrays. The FlatBuffer passed to
// Init() is only valid for the duration of that call, so we copy the data here
// and keep it alive until Destroy().
static std::vector<double> g_q;
static std::vector<double> g_refl;
static std::vector<double> g_refl_error;
static std::vector<double> g_q_error;
static std::vector<double> g_ed_values;  // resume state ed array
static std::string         g_directory;
static std::string         g_title;

// ── Helper: copy FlatBuffers vector into a std::vector<double> ────────────────
static std::vector<double> copy_fbs_vec(const flatbuffers::Vector<double>* v)
{
    if (!v || v->size() == 0) return {};
    return std::vector<double>(v->data(), v->data() + v->size());
}

// ── Helper: finish a FlatBuffer and copy into caller-supplied buffer ──────────
// Returns bytes written, or -1 if the buffer is too small.
template<typename T>
static int finish_into(flatbuffers::FlatBufferBuilder& fbb,
                       flatbuffers::Offset<T> root,
                       uint8_t* outBuf, int maxLen)
{
    fbb.Finish(root);
    int written = static_cast<int>(fbb.GetSize());
    if (written > maxLen) return -1;
    std::memcpy(outBuf, fbb.GetBufferPointer(), written);
    return written;
}

// ── Lifecycle ─────────────────────────────────────────────────────────────────

extern "C" EXPORT void Init(const uint8_t* buf, int len)
{
    g_last_init_error.clear();

    auto* req = StochFitProto::GetInitRequest(buf);
    auto* s   = req->settings();

    // Copy owned array data from the FlatBuffer
    g_q          = copy_fbs_vec(s->q());
    g_refl       = copy_fbs_vec(s->refl());
    g_refl_error = copy_fbs_vec(s->refl_error());
    g_q_error    = copy_fbs_vec(s->q_error());
    g_directory  = s->directory() ? s->directory()->str() : "";
    g_title      = s->title() ? s->title()->str() : "";

    // Populate the C++ internal struct (still used by StochFitHarness)
    ReflSettings rs{};
    rs.Directory            = g_directory.c_str();
    rs.Q                    = g_q.data();
    rs.Refl                 = g_refl.data();
    rs.ReflError            = g_refl_error.data();
    rs.QError               = g_q_error.empty() ? nullptr : g_q_error.data();
    rs.QPoints              = s->q_points();
    rs.SubSLD               = s->sub_sld();
    rs.FilmSLD              = s->film_sld();
    rs.SupSLD               = s->sup_sld();
    rs.Boxes                = s->boxes();
    rs.FilmAbs              = s->film_abs();
    rs.SubAbs               = s->sub_abs();
    rs.SupAbs               = s->sup_abs();
    rs.Wavelength           = s->wavelength();
    rs.UseSurfAbs           = s->use_surf_abs();
    rs.QErr                 = s->q_err();
    rs.Forcenorm            = s->forcenorm();
    rs.Forcesig             = s->forcesig();
    rs.Debug                = s->debug();
    rs.XRonly               = s->xr_only();
    rs.Resolution           = s->resolution();
    rs.FilmLength           = s->film_length();
    rs.Impnorm              = s->impnorm();
    rs.Objectivefunction    = s->objectivefunction();
    rs.Paramtemp            = s->paramtemp();
    rs.Sigmasearch          = s->sigmasearch();
    rs.NormalizationSearchPerc = s->normalization_search_perc();
    rs.AbsorptionSearchPerc = s->absorption_search_perc();
    rs.Algorithm            = s->algorithm();
    rs.Inittemp             = s->inittemp();
    rs.Platiter             = s->platiter();
    rs.Slope                = s->slope();
    rs.Gamma                = s->gamma();
    rs.STUNfunc             = s->stun_func();
    rs.Adaptive             = s->adaptive();
    rs.Tempiter             = s->tempiter();
    rs.STUNdeciter          = s->stun_dec_iter();
    rs.Gammadec             = s->gammadec();
    rs.CritEdgeOffset       = s->crit_edge_offset();
    rs.HighQOffset          = s->high_q_offset();
    rs.Iterations           = s->iterations();
    rs.IterationsCompleted  = s->iterations_completed();
    rs.ChiSquare            = s->chi_square();
    rs.UseGpu               = s->use_gpu();
    rs.GpuChains            = s->gpu_chains();
    rs.Title                = g_title.c_str();

    StochRunState  state{};
    StochRunState* statePtr = nullptr;
    if (req->state())
    {
        auto* st             = req->state();
        state.roughness      = st->roughness();
        state.filmAbsInput   = st->film_abs_input();
        state.surfAbs        = st->surf_abs();
        state.temperature    = st->temperature();
        state.impNorm        = st->imp_norm();
        state.avgfSTUN       = st->avg_f_stun();
        state.bestSolution   = st->best_solution();
        state.chiSquare      = st->chi_square();
        state.goodnessOfFit  = st->goodness_of_fit();
        state.iteration      = st->iteration();
        g_ed_values          = copy_fbs_vec(st->ed_values());
        state.edValues       = g_ed_values.data();
        state.edCount        = static_cast<int>(g_ed_values.size());
        statePtr             = &state;
    }

    delete stochfit;
    stochfit = new StochFit(&rs, statePtr);

    if (auto r = stochfit->GetInitError(); !r)
    {
        g_last_init_error = r.error();
        delete stochfit;
        stochfit = nullptr;
    }
}

extern "C" EXPORT const char* GetInitError()
{
    return g_last_init_error.c_str();
}

extern "C" EXPORT void Start(int iterations)
{
    if (stochfit) stochfit->Start(iterations);
}

extern "C" EXPORT void Stop()
{
    if (stochfit) stochfit->Stop();
}

extern "C" EXPORT void Destroy()
{
    delete stochfit;
    stochfit = nullptr;
    g_q.clear(); g_refl.clear(); g_refl_error.clear(); g_q_error.clear();
    g_ed_values.clear(); g_directory.clear(); g_title.clear();
}

extern "C" EXPORT void Cancel()
{
    if (stochfit)
    {
        stochfit->Stop();
        delete stochfit;
        stochfit = nullptr;
    }
    g_q.clear(); g_refl.clear(); g_refl_error.clear(); g_q_error.clear();
    g_ed_values.clear(); g_directory.clear(); g_title.clear();
}

// ── SA polling ────────────────────────────────────────────────────────────────

extern "C" EXPORT int GetData(uint8_t* outBuf, int maxLen)
{
    if (!stochfit) return -1;

    // Reuse the same temporaries the harness fills
    int rhoSize = 0, reflSize = 0;
    stochfit->GetArraySizes(&rhoSize, &reflSize);

    std::vector<double> zRange(rhoSize), rho(rhoSize);
    std::vector<double> qRange(reflSize), refl(reflSize);
    double roughness = 0, chiSquare = 0, goodnessOfFit = 0;
    int32_t isFinished = 0;

    int iter = stochfit->GetData(
        zRange.data(), rho.data(), qRange.data(), refl.data(),
        &roughness, &chiSquare, &goodnessOfFit, &isFinished);

    flatbuffers::FlatBufferBuilder fbb(static_cast<size_t>(maxLen));
    auto zVec   = fbb.CreateVector(zRange);
    auto rhoVec = fbb.CreateVector(rho);
    auto qVec   = fbb.CreateVector(qRange);
    auto rfVec  = fbb.CreateVector(refl);

    auto result = StochFitProto::CreateGetDataResult(fbb,
        zVec, rhoVec, qVec, rfVec,
        roughness, chiSquare, goodnessOfFit,
        isFinished != 0,
        iter);

    return finish_into(fbb, result, outBuf, maxLen);
}

extern "C" EXPORT int GetRunState(uint8_t* outBuf, int maxLen)
{
    if (!stochfit) return -1;

    double saScalars[9] = {};
    // size the ed buffer generously; GetRunState fills edCount with actual count
    std::vector<double> edValues(256, 0.0);
    int edCount = 0;
    stochfit->GetRunState(saScalars, edValues.data(), &edCount);
    edValues.resize(static_cast<size_t>(edCount));

    flatbuffers::FlatBufferBuilder fbb(4096);
    auto edVec = fbb.CreateVector(edValues);
    auto result = StochFitProto::CreateGetRunStateResult(fbb,
        saScalars[0], saScalars[1], saScalars[2], saScalars[3],
        saScalars[4], saScalars[5], saScalars[6], saScalars[7],
        saScalars[8], edVec, 0 /*iteration filled by caller*/);

    return finish_into(fbb, result, outBuf, maxLen);
}

extern "C" EXPORT int ArraySizes(uint8_t* outBuf, int maxLen)
{
    int rhoSize = 0, reflSize = 0;
    if (stochfit) stochfit->GetArraySizes(&rhoSize, &reflSize);

    flatbuffers::FlatBufferBuilder fbb(64);
    auto result = StochFitProto::CreateArraySizesResult(fbb, rhoSize, reflSize);
    return finish_into(fbb, result, outBuf, maxLen);
}

extern "C" EXPORT int SAParams(uint8_t* outBuf, int maxLen)
{
    if (!stochfit) return -1;

    double lowestEnergy = 0, temp = 0;
    int mode = 0;
    temp          = stochfit->m_SA->Get_Temp();
    lowestEnergy  = stochfit->m_SA->Get_LowestEnergy();
    mode          = (temp < 1e-20) ? -1 : 1;

    flatbuffers::FlatBufferBuilder fbb(64);
    auto result = StochFitProto::CreateSaParamsResult(fbb, lowestEnergy, temp, mode);
    return finish_into(fbb, result, outBuf, maxLen);
}

extern "C" EXPORT bool WarmedUp()
{
    return stochfit ? stochfit->GetWarmedUp() : false;
}

extern "C" EXPORT bool GpuAvailable()
{
    bool avail = is_gpu_available();
    fprintf(stderr, "[GPU] GpuAvailable() = %s\n", avail ? "true" : "false");
    return avail;
}
