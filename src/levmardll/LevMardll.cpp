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
#include "LevMardll.h"
#include "FastReflCalc.h"
#include "RhoCalc.h"
#include "ParameterContainer.h"
#include "Settings.h"
#include "generated/stochfit_generated.h"
#include <flatbuffers/flatbuffers.h>

#include <random>
#include <levmar/levmar.h>
#include <cstring>
#include <cmath>
#include <vector>

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::vector<double> copy_fbs_vec(const flatbuffers::Vector<double>* v)
{
    if (!v || v->size() == 0) return {};
    return std::vector<double>(v->data(), v->data() + v->size());
}

// Populate a BoxReflSettings from a FlatBuffer table.
// Pointer fields are set to data() of the caller-owned vectors.
static void fill_box_settings(const StochFitProto::BoxReflSettings* s,
                               BoxReflSettings& rs,
                               std::vector<double>& q,
                               std::vector<double>& refl,
                               std::vector<double>& reflErr,
                               std::vector<double>& qErr,
                               std::vector<double>& ul,
                               std::vector<double>& ll,
                               std::vector<double>& paramPercs,
                               std::vector<double>& miedp,
                               std::vector<double>& zInc,
                               std::string& dir)
{
    q         = copy_fbs_vec(s->q());
    refl      = copy_fbs_vec(s->refl());
    reflErr   = copy_fbs_vec(s->refl_error());
    qErr      = copy_fbs_vec(s->q_error());
    ul        = copy_fbs_vec(s->ul());
    ll        = copy_fbs_vec(s->ll());
    paramPercs= copy_fbs_vec(s->param_percs());
    miedp     = copy_fbs_vec(s->miedp());
    zInc      = copy_fbs_vec(s->z_increment());
    dir       = s->directory() ? s->directory()->str() : "";

    rs.Directory  = dir.c_str();
    rs.Q          = q.empty()        ? nullptr : q.data();
    rs.Refl       = refl.empty()     ? nullptr : refl.data();
    rs.ReflError  = reflErr.empty()  ? nullptr : reflErr.data();
    rs.QError     = qErr.empty()     ? nullptr : qErr.data();
    rs.UL         = ul.empty()       ? nullptr : ul.data();
    rs.LL         = ll.empty()       ? nullptr : ll.data();
    rs.ParamPercs = paramPercs.empty()? nullptr : paramPercs.data();
    rs.MIEDP      = miedp.empty()    ? nullptr : miedp.data();
    rs.ZIncrement = zInc.empty()     ? nullptr : zInc.data();
    rs.QPoints    = s->q_points();
    rs.OneSigma   = s->one_sigma();
    rs.WriteFiles = s->write_files();
    rs.SubSLD     = s->sub_sld();
    rs.SupSLD     = s->sup_sld();
    rs.Boxes      = s->boxes();
    rs.Wavelength = s->wavelength();
    rs.QSpread    = s->q_spread();
    rs.Forcenorm  = s->forcenorm();
    rs.ImpNorm    = s->imp_norm();
    rs.FitFunc    = s->fit_func();
    rs.LowQOffset = s->low_q_offset();
    rs.HighQOffset= s->high_q_offset();
    rs.Iterations = s->iterations();
    rs.ZLength    = s->z_length();
}

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

// ── FastReflfit ───────────────────────────────────────────────────────────────

extern "C" EXPORT int FastReflfit(const uint8_t* inBuf, int /*inLen*/, uint8_t* outBuf, int maxLen)
{
    auto* req = flatbuffers::GetRoot<StochFitProto::LevMarRequest>(inBuf);

    std::vector<double> q, refl, reflErr, qErr, ul, ll, paramPercs, miedp, zInc;
    std::string dir;
    BoxReflSettings rs{};
    fill_box_settings(req->settings(), rs, q, refl, reflErr, qErr, ul, ll, paramPercs, miedp, zInc, dir);

    std::vector<double> params = copy_fbs_vec(req->parameters());
    int paramsize = static_cast<int>(params.size());

    double opts[LM_OPTS_SZ] = { LM_INIT_MU, 1e-15, 1e-15, 1e-20, -LM_DIFF_DELTA };

    FastReflcalc Refl;
    Refl.init(&rs);

    std::vector<double> xvec(rs.QPoints, 0.0);
    std::vector<double> work(LM_DIF_WORKSZ(paramsize, rs.QPoints) + paramsize * rs.QPoints);
    double* covarRaw = work.data() + LM_DIF_WORKSZ(paramsize, rs.QPoints);
    std::vector<double> info(LM_INFO_SZ, 0.0);

    if (rs.UL == nullptr)
        dlevmar_dif(Refl.objective, params.data(), xvec.data(), paramsize, rs.QPoints,
                    1000, opts, info.data(), work.data(), covarRaw, (void*)(&Refl));
    else
        dlevmar_bc_dif(Refl.objective, params.data(), xvec.data(), paramsize, rs.QPoints,
                       rs.LL, rs.UL, nullptr, 1000, opts, info.data(), work.data(), covarRaw,
                       (void*)(&Refl));

    std::vector<double> covar(paramsize);
    for (int i = 0; i < paramsize; i++)
        covar[i] = std::sqrt(covarRaw[i * (paramsize + 1)]);

    flatbuffers::FlatBufferBuilder fbb(4096);
    auto result = StochFitProto::CreateReflFitResult(fbb,
        fbb.CreateVector(params),
        fbb.CreateVector(covar),
        fbb.CreateVector(info));
    return finish_into(fbb, result, outBuf, maxLen);
}

// ── FastReflGenerate ──────────────────────────────────────────────────────────

extern "C" EXPORT int FastReflGenerate(const uint8_t* inBuf, int /*inLen*/, uint8_t* outBuf, int maxLen)
{
    auto* req = flatbuffers::GetRoot<StochFitProto::LevMarRequest>(inBuf);

    std::vector<double> q, refl, reflErr, qErr, ul, ll, paramPercs, miedp, zInc;
    std::string dir;
    BoxReflSettings rs{};
    fill_box_settings(req->settings(), rs, q, refl, reflErr, qErr, ul, ll, paramPercs, miedp, zInc, dir);

    std::vector<double> params = copy_fbs_vec(req->parameters());

    FastReflcalc FastRefl;
    FastRefl.init(&rs);

    if (rs.OneSigma)
        FastRefl.mkdensityonesigma(params.data(), static_cast<int>(params.size()));
    else
        FastRefl.mkdensity(params.data(), static_cast<int>(params.size()));

    FastRefl.SetOffsets(0, 0);
    FastRefl.myrfdispatch();

    std::vector<double> reflOut(FastRefl.reflpt.begin(), FastRefl.reflpt.begin() + rs.QPoints);

    flatbuffers::FlatBufferBuilder fbb(4096);
    auto result = StochFitProto::CreateReflGenerateResult(fbb, fbb.CreateVector(reflOut));
    return finish_into(fbb, result, outBuf, maxLen);
}

// ── Rhofit ───────────────────────────────────────────────────────────────────

extern "C" EXPORT int Rhofit(const uint8_t* inBuf, int /*inLen*/, uint8_t* outBuf, int maxLen)
{
    auto* req = flatbuffers::GetRoot<StochFitProto::LevMarRequest>(inBuf);

    std::vector<double> q, refl, reflErr, qErr, ul, ll, paramPercs, miedp, zInc;
    std::string dir;
    BoxReflSettings rs{};
    fill_box_settings(req->settings(), rs, q, refl, reflErr, qErr, ul, ll, paramPercs, miedp, zInc, dir);

    std::vector<double> params = copy_fbs_vec(req->parameters());
    int paramsize = static_cast<int>(params.size());

    double opts[LM_OPTS_SZ] = { LM_INIT_MU, 1e-15, 1e-15, 1e-20, -LM_DIFF_DELTA };

    RhoCalc Rho;
    Rho.init(&rs);

    std::vector<double> xvec(rs.ZLength, 0.0);
    std::vector<double> work(LM_DIF_WORKSZ(paramsize, rs.ZLength) + paramsize * rs.ZLength);
    double* covarRaw = work.data() + LM_DIF_WORKSZ(paramsize, rs.ZLength);
    std::vector<double> info(LM_INFO_SZ, 0.0);

    if (rs.UL == nullptr)
        dlevmar_dif(Rho.objective, params.data(), xvec.data(), paramsize, rs.ZLength,
                    1000, opts, info.data(), work.data(), covarRaw, (void*)(&Rho));
    else
        dlevmar_bc_dif(Rho.objective, params.data(), xvec.data(), paramsize, rs.ZLength,
                       rs.LL, rs.UL, nullptr, 1000, opts, info.data(), work.data(), covarRaw,
                       (void*)(&Rho));

    std::vector<double> covar(paramsize);
    for (int i = 0; i < paramsize; i++)
        covar[i] = std::sqrt(covarRaw[i * (paramsize + 1)]);

    flatbuffers::FlatBufferBuilder fbb(4096);
    auto result = StochFitProto::CreateRhoFitResult(fbb,
        fbb.CreateVector(params),
        fbb.CreateVector(covar),
        fbb.CreateVector(info));
    return finish_into(fbb, result, outBuf, maxLen);
}

// ── RhoGenerate ───────────────────────────────────────────────────────────────

extern "C" EXPORT int RhoGenerate(const uint8_t* inBuf, int /*inLen*/, uint8_t* outBuf, int maxLen)
{
    auto* req = flatbuffers::GetRoot<StochFitProto::LevMarRequest>(inBuf);

    std::vector<double> q, refl, reflErr, qErr, ul, ll, paramPercs, miedp, zInc;
    std::string dir;
    BoxReflSettings rs{};
    fill_box_settings(req->settings(), rs, q, refl, reflErr, qErr, ul, ll, paramPercs, miedp, zInc, dir);

    std::vector<double> params = copy_fbs_vec(req->parameters());

    RhoCalc Rho;
    Rho.init(&rs);
    Rho.mkdensity(params.data(), static_cast<int>(params.size()));
    Rho.mkdensityboxmodel(params.data(), static_cast<int>(params.size()));

    std::vector<double> ed(Rho.nk.begin(), Rho.nk.begin() + rs.ZLength);
    std::vector<double> boxED(Rho.nkb.begin(), Rho.nkb.begin() + rs.ZLength);

    flatbuffers::FlatBufferBuilder fbb(8192);
    auto result = StochFitProto::CreateRhoGenerateResult(fbb,
        fbb.CreateVector(ed),
        fbb.CreateVector(boxED));
    return finish_into(fbb, result, outBuf, maxLen);
}

// ── StochFitBoxModel ──────────────────────────────────────────────────────────

extern "C" EXPORT int StochFitBoxModel(const uint8_t* inBuf, int /*inLen*/, uint8_t* outBuf, int maxLen)
{
    auto* req = flatbuffers::GetRoot<StochFitProto::LevMarRequest>(inBuf);

    std::vector<double> q, refl, reflErr, qErr, ul, ll, paramPercs, miedp, zInc;
    std::string dir;
    BoxReflSettings rs{};
    fill_box_settings(req->settings(), rs, q, refl, reflErr, qErr, ul, ll, paramPercs, miedp, zInc, dir);

    std::vector<double> params = copy_fbs_vec(req->parameters());
    int paramsize = static_cast<int>(params.size());
    int QSize = rs.QPoints;
    double* parampercs = rs.ParamPercs;

    double opts[LM_OPTS_SZ] = { LM_INIT_MU, 1e-15, 1e-15, 1e-20, -LM_DIFF_DELTA };

    FastReflcalc Refl;
    Refl.init(&rs);

    std::vector<double> xvec(QSize, 0.0);

    std::vector<double> origguess(params);

    if (rs.OneSigma)
        Refl.mkdensityonesigma(params.data(), paramsize);
    else
        Refl.mkdensity(params.data(), paramsize);
    Refl.myrfdispatch();

    double bestchisquare = 0;
    for (int i = 0; i < QSize; i++)
    {
        double diff = std::log(Refl.reflpt[i]) - std::log(refl[i]);
        bestchisquare += diff * diff;
    }

    std::vector<double> tempinfoarray(LM_INFO_SZ, 0.0);
    tempinfoarray[1] = bestchisquare;
    std::vector<double> tempcovar(static_cast<size_t>(paramsize * paramsize), 0.0);
    ParameterContainer original(params.data(), tempcovar.data(), paramsize,
                                rs.OneSigma, tempinfoarray.data(), parampercs[6]);

    std::vector<ParameterContainer> temp;
    temp.reserve(6000);

    omp_set_num_threads(omp_get_num_procs());

    #pragma omp parallel
    {
        FastReflcalc locRefl;
        locRefl.init(&rs);

        std::mt19937 randgen(std::random_device{}() + omp_get_thread_num());
        auto IRandom = [&](double max, double min) {
            return std::uniform_real_distribution<double>(min, max)(randgen);
        };

        ParameterContainer localanswer;
        std::vector<double> locparameters(paramsize);
        double locbestchisquare = bestchisquare;
        int vecsize = 1000;
        int veccount = 0;
        ParameterContainer* vec = (ParameterContainer*)malloc(vecsize * sizeof(ParameterContainer));

        std::vector<double> locinfo(LM_INFO_SZ);
        std::vector<double> work(LM_DIF_WORKSZ(paramsize, QSize) + paramsize * QSize);
        double* covarRaw = work.data() + LM_DIF_WORKSZ(paramsize, QSize);

        #pragma omp for schedule(runtime)
        for (int i = 0; i < rs.Iterations; i++)
        {
            locparameters[0] = IRandom(origguess[0] * parampercs[4], origguess[0] * parampercs[5]);
            for (int k = 0; k < rs.Boxes; k++)
            {
                if (rs.OneSigma)
                {
                    locparameters[2*k+1] = IRandom(origguess[2*k+1]*parampercs[0], origguess[2*k+1]*parampercs[1]);
                    locparameters[2*k+2] = IRandom(origguess[2*k+2]*parampercs[2], origguess[2*k+2]*parampercs[3]);
                }
                else
                {
                    locparameters[3*k+1] = IRandom(origguess[3*k+1]*parampercs[0], origguess[3*k+1]*parampercs[1]);
                    locparameters[3*k+2] = IRandom(origguess[3*k+2]*parampercs[2], origguess[3*k+2]*parampercs[3]);
                    locparameters[3*k+3] = IRandom(origguess[3*k+3]*parampercs[4], origguess[3*k+3]*parampercs[5]);
                }
            }
            locparameters[paramsize-1] = origguess[paramsize-1];

            if (rs.UL == nullptr)
                dlevmar_dif(locRefl.objective, locparameters.data(), xvec.data(), paramsize, QSize,
                            500, opts, locinfo.data(), work.data(), covarRaw, (void*)(&locRefl));
            else
                dlevmar_bc_dif(locRefl.objective, locparameters.data(), xvec.data(), paramsize, QSize,
                               rs.LL, rs.UL, nullptr, 500, opts, locinfo.data(), work.data(), covarRaw,
                               (void*)(&locRefl));

            localanswer.SetContainer(locparameters.data(), covarRaw, paramsize,
                                     rs.OneSigma, locinfo.data(), parampercs[6]);

            if (locinfo[1] < bestchisquare && localanswer.IsReasonable())
            {
                if (veccount + 2 == vecsize)
                {
                    vecsize += 1000;
                    vec = (ParameterContainer*)realloc(vec, vecsize * sizeof(ParameterContainer));
                }
                bool unique = true;
                for (int j = 0; j < veccount; j++)
                {
                    if (localanswer == vec[j]) { unique = false; break; }
                }
                if (unique) vec[veccount++] = localanswer;
            }
        }

        #pragma omp critical (AddVecs)
        {
            for (int i = 0; i < veccount; i++)
                temp.push_back(vec[i]);
        }
        free(vec);
    }

    temp.push_back(original);

    std::vector<ParameterContainer> allsolutions;
    allsolutions.reserve(6000);
    int tempsize = static_cast<int>(temp.size());
    allsolutions.push_back(temp[0]);
    for (int i = 1; i < tempsize; i++)
    {
        int sz = static_cast<int>(allsolutions.size());
        for (int j = 0; j < sz; j++)
        {
            if (temp[i] == allsolutions[j]) break;
            if (j == sz - 1) allsolutions.push_back(temp[i]);
        }
    }

    if (!allsolutions.empty())
        std::sort(allsolutions.begin(), allsolutions.end());

    int n = static_cast<int>(std::min<size_t>(allsolutions.size(), 999));
    std::vector<double> outParams(params);         // best fit params (from last levmar run via original)
    std::vector<double> covarArray(static_cast<size_t>(n * paramsize));
    std::vector<double> infoOut(static_cast<size_t>(n * LM_INFO_SZ));
    std::vector<double> paramArray(static_cast<size_t>(n * paramsize));
    std::vector<double> chiSquareArray(n);

    for (int i = 0; i < n; i++)
    {
        const double* pa = allsolutions[i].GetParamArray();
        const double* ca = allsolutions[i].GetCovarArray();
        const double* ia = allsolutions[i].GetInfoArray();
        for (int j = 0; j < paramsize; j++)
        {
            paramArray [i * paramsize + j] = pa[j];
            covarArray [i * paramsize + j] = ca[j];
        }
        std::memcpy(infoOut.data() + i * LM_INFO_SZ, ia, LM_INFO_SZ * sizeof(double));
        chiSquareArray[i] = allsolutions[i].GetScore();
    }

    flatbuffers::FlatBufferBuilder fbb(static_cast<size_t>(maxLen));
    auto result = StochFitProto::CreateBoxStochFitResult(fbb,
        fbb.CreateVector(outParams),
        fbb.CreateVector(covarArray),
        fbb.CreateVector(infoOut),
        fbb.CreateVector(paramArray),
        fbb.CreateVector(chiSquareArray),
        n);
    return finish_into(fbb, result, outBuf, maxLen);
}
