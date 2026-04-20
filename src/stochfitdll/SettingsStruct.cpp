#include "platform.h"
#include "SettingsStruct.h"
#include "generated/stochfit_generated.h"

static std::vector<double> copy_fbs_vec(const flatbuffers::Vector<double>* v)
{
    if (!v || v->size() == 0) return {};
    return std::vector<double>(v->data(), v->data() + v->size());
}

ReflSettings::ReflSettings(const StochFitProto::ReflSettings* s)
{
    Directory            = s->directory() ? s->directory()->str() : "";
    Q                    = copy_fbs_vec(s->q());
    Refl                 = copy_fbs_vec(s->refl());
    ReflError            = copy_fbs_vec(s->refl_error());
    QError               = copy_fbs_vec(s->q_error());
    SubSLD               = s->sub_sld();
    FilmSLD              = s->film_sld();
    SupSLD               = s->sup_sld();
    Boxes                = s->boxes();
    FilmAbs              = s->film_abs();
    SubAbs               = s->sub_abs();
    SupAbs               = s->sup_abs();
    Wavelength           = s->wavelength();
    UseSurfAbs           = s->use_surf_abs();
    QErr                 = s->q_err();
    Forcesig             = s->forcesig();
    Debug                = s->debug();
    XRonly               = s->xr_only();
    Resolution           = s->resolution();
    FilmLength           = s->film_length();
    Impnorm              = s->impnorm();
    Objectivefunction    = s->objectivefunction();
    Paramtemp            = s->paramtemp();
    Sigmasearch          = s->sigmasearch();
    NormalizationSearchPerc = s->normalization_search_perc();
    AbsorptionSearchPerc = s->absorption_search_perc();
    Algorithm            = s->algorithm();
    Inittemp             = s->inittemp();
    Platiter             = s->platiter();
    Slope                = s->slope();
    Gamma                = s->gamma();
    STUNfunc             = s->stun_func();
    Adaptive             = s->adaptive();
    Tempiter             = s->tempiter();
    STUNdeciter          = s->stun_dec_iter();
    Gammadec             = s->gammadec();
    CritEdgeOffset       = s->crit_edge_offset();
    HighQOffset          = s->high_q_offset();
    Iterations           = s->iterations();
    IterationsCompleted  = s->iterations_completed();
    ChiSquare            = s->chi_square();
    UseGpu               = s->use_gpu();
    GpuChains            = s->gpu_chains();
    Title                = s->title() ? s->title()->str() : "";
}

StochRunState::StochRunState(const StochFitProto::StochRunState* st)
{
    roughness     = st->roughness();
    filmAbsInput  = st->film_abs_input();
    surfAbs       = st->surf_abs();
    temperature   = st->temperature();
    impNorm       = st->imp_norm();
    avgfSTUN      = st->avg_f_stun();
    bestSolution  = st->best_solution();
    chiSquare     = st->chi_square();
    goodnessOfFit = st->goodness_of_fit();
    iteration     = st->iteration();
    edValues      = copy_fbs_vec(st->ed_values());
}
