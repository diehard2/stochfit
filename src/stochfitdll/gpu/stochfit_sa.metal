#include <metal_stdlib>
using namespace metal;

// ── Complex float helpers ──────────────────────────────────────────────────

struct cfloat { float re; float im; };

inline cfloat cf_make(float r, float i) { return {r, i}; }
inline cfloat cf_add(cfloat a, cfloat b) { return {a.re+b.re, a.im+b.im}; }
inline cfloat cf_sub(cfloat a, cfloat b) { return {a.re-b.re, a.im-b.im}; }
inline cfloat cf_mul(cfloat a, cfloat b) {
    return {a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re};
}
inline cfloat cf_div(cfloat a, cfloat b) {
    float denom = b.re*b.re + b.im*b.im;
    return {(a.re*b.re + a.im*b.im)/denom, (a.im*b.re - a.re*b.im)/denom};
}
inline float cf_abs2(cfloat a) { return a.re*a.re + a.im*a.im; }
inline cfloat cf_scale(cfloat a, float s) { return {a.re*s, a.im*s}; }

inline cfloat cf_sqrt(cfloat z) {
    float r = sqrt(cf_abs2(z));
    float t = sqrt(0.5f * (r + z.re));
    float s = (z.im >= 0.f ? 1.f : -1.f) * sqrt(0.5f * (r - z.re));
    return {t, s};
}

inline cfloat cf_exp(cfloat z) {
    float ea = metal::precise::exp(z.re);
    float s = metal::precise::sin(z.im);
    float c = metal::precise::cos(z.im);
    return {ea*c, ea*s};
}

// ── 2x2 complex matrix ────────────────────────────────────────────────────

struct cmat2 { cfloat a, b, c, d; };

inline cmat2 cmat2_identity() {
    return {{1,0},{0,0},{0,0},{1,0}};
}

inline cmat2 cmat2_mul(cmat2 L, cmat2 R) {
    return {
        cf_add(cf_mul(L.a, R.a), cf_mul(L.b, R.c)),
        cf_add(cf_mul(L.a, R.b), cf_mul(L.b, R.d)),
        cf_add(cf_mul(L.c, R.a), cf_mul(L.d, R.c)),
        cf_add(cf_mul(L.c, R.b), cf_mul(L.d, R.d)),
    };
}

// ── Shared structs (must match host) ──────────────────────────────────────

struct GpuSAStateM {
    float temperature;
    float best_energy;
    float current_energy;
    float slope;
    float gamma;
    float avg_fstun;
    float gammadec;
    float stepsize;
    float best_solution;
    int algorithm;
    int iteration;
    int plat_time;
    int temp_iter;
    int stun_func;
    int stun_dec_iter;
    int adaptive;
    int sigmasearch;
    int normsearch;
    int abssearch;
};

constant int GPU_MAX_BOXES_M = 64;

struct GpuParamsM {
    float sld_values[GPU_MAX_BOXES_M + 2];
    float roughness;
    float surf_abs;
    float imp_norm;
    int num_boxes;
    int real_params_size;
    int fix_roughness;
    int use_surf_abs;
    int fix_imp_norm;
    float roughness_low;
    float roughness_high;
    float surfabs_high;
    float impnorm_high;
    float param_low;
    float param_high;
};

struct GpuEDPConfigM {
    float rho;
    float dz;
    float k0;
    float beta;
    float beta_sub;
    float beta_sup;
    int num_layers;
    int use_abs;
    float indexsup_real;
    float indexsup2_real;
};

struct GpuMeasConfigM {
    int num_datapoints;
    int objective_function;
    int use_qspread;
    int force_norm;
    int imp_norm;
    int xr_only;
};

// ── PCG random number generator ───────────────────────────────────────────

struct PcgState { uint state; uint inc; };

inline uint pcg_next(thread PcgState& rng) {
    uint old = rng.state;
    rng.state = old * 747796405u + rng.inc;
    uint xorshifted = ((old >> 18u) ^ old) >> 27u;
    uint rot = old >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31u));
}

inline float pcg_uniform(thread PcgState& rng) {
    return float(pcg_next(rng)) / 4294967295.0f;
}

// ── EDP generation kernel ──────────────────────────────────────────────────

kernel void kernel_generate_edp(
    constant GpuEDPConfigM& edp [[buffer(0)]],
    device GpuParamsM* chain_params [[buffer(1)]],
    device float* chain_edp [[buffer(2)]],
    device float* chain_dedp [[buffer(3)]],
    device float* chain_rho_arr [[buffer(4)]],
    constant float* ed_spacing [[buffer(5)]],
    constant float* dist_array [[buffer(6)]],
    constant int& num_chains [[buffer(7)]],
    uint2 gid [[thread_position_in_grid]])
{
    int chain_id = gid.y;
    int idx = gid.x;
    if (chain_id >= num_chains) return;

    device GpuParamsM& p = chain_params[chain_id];
    int num_layers = edp.num_layers;
    int refllayers = p.real_params_size - 1;

    device float* rho_arr = chain_rho_arr + chain_id * (GPU_MAX_BOXES_M + 2);

    // Compute rho array for this chain (only first threads)
    if (idx < refllayers) {
        rho_arr[idx] = edp.rho * (p.sld_values[idx + 1] - p.sld_values[idx]) * 0.5f;
    }
    threadgroup_barrier(mem_flags::mem_device);

    if (idx < num_layers) {
        float rough = p.roughness;
        if (rough < 1e-6f) rough = 1e-6f;
        rough = 1.0f / (rough * sqrt(2.0f));

        float supersld = p.sld_values[0] * edp.rho;
        float edp_val = supersld;

        for (int k = 0; k < refllayers; k++) {
            float dist = (ed_spacing[idx] - dist_array[k]) * rough;
            if (dist > 6.0f)
                edp_val += rho_arr[k] * 2.0f;
            else if (dist > -6.0f)
                edp_val += rho_arr[k] * (1.0f + metal::precise::erf(dist));
        }

        int offset = chain_id * num_layers + idx;
        chain_edp[offset] = edp_val;
        chain_dedp[offset] = 2.0f * edp_val;
    }
}

// ── Parratt reflectivity kernel ────────────────────────────────────────────

kernel void kernel_parratt(
    constant GpuEDPConfigM& edp [[buffer(0)]],
    constant GpuMeasConfigM& meas_config [[buffer(1)]],
    device GpuParamsM* chain_params [[buffer(2)]],
    device float* chain_dedp [[buffer(3)]],
    device float* chain_refl [[buffer(4)]],
    constant float* sintheta_arr [[buffer(5)]],
    constant float* sinsq_arr [[buffer(6)]],
    constant int& num_chains [[buffer(7)]],
    constant int& num_q [[buffer(8)]],
    uint2 gid [[thread_position_in_grid]])
{
    int chain_id = gid.y;
    int q_idx = gid.x;
    if (chain_id >= num_chains || q_idx >= num_q) return;

    device float* dedp = chain_dedp + chain_id * edp.num_layers;
    int num_layers = edp.num_layers;
    float k0 = edp.k0;
    float dz = edp.dz;

    float sintheta_val = sintheta_arr[q_idx];
    float sinsq_val = sinsq_arr[q_idx];

    float dedp0 = dedp[0];
    float indexsup_re = 1.0f - dedp0 / 2.0f;
    float indexsup2_re = indexsup_re * indexsup_re;
    cfloat length_mult = cf_make(0.0f, -2.0f * dz);

    // Find uniform region offsets
    int low_offset = 0;
    for (int i = 0; i < num_layers; i++) {
        if (dedp[i] == dedp0) low_offset++;
        else break;
    }
    int high_offset = num_layers;
    float dedp_last = dedp[num_layers - 1];
    for (int i = num_layers - 1; i > 0; i--) {
        if (dedp[i] == dedp_last) high_offset = i;
        else break;
    }

    cfloat kk_sup = cf_make(k0 * indexsup_re * sintheta_val, 0.0f);
    cfloat kk_low = cf_scale(cf_sqrt(cf_make(indexsup2_re * sinsq_val - dedp[1] + dedp0, 0.0f)), k0);
    cfloat kk_high = cf_scale(cf_sqrt(cf_make(indexsup2_re * sinsq_val - dedp_last + dedp0, 0.0f)), k0);
    cfloat ak_low = cf_exp(cf_mul(length_mult, kk_low));
    cfloat ak_high = cf_exp(cf_mul(length_mult, kk_high));

    cmat2 M = cmat2_identity();
    cfloat kk_prev = kk_sup;

    for (int i = 0; i < num_layers - 1; i++) {
        cfloat kk_cur;
        if (i + 1 <= low_offset) kk_cur = kk_low;
        else if (i + 1 >= high_offset) kk_cur = kk_high;
        else kk_cur = cf_scale(cf_sqrt(cf_make(indexsup2_re * sinsq_val - dedp[i + 1] + dedp0, 0.0f)), k0);

        cfloat ak_i;
        if (i == 0 || i == num_layers - 1) ak_i = cf_make(1.0f, 0.0f);
        else if (i <= low_offset) ak_i = ak_low;
        else if (i >= high_offset) ak_i = ak_high;
        else ak_i = cf_exp(cf_mul(length_mult, kk_prev));

        cfloat sum = cf_add(kk_prev, kk_cur);
        cfloat diff = cf_sub(kk_prev, kk_cur);
        cfloat rj_i = cf_abs2(sum) > 1e-30f ? cf_div(diff, sum) : cf_make(0.0f, 0.0f);

        cmat2 layer = {ak_i, cf_mul(ak_i, rj_i), rj_i, cf_make(1.0f, 0.0f)};
        M = cmat2_mul(M, layer);
        kk_prev = kk_cur;
    }

    cfloat R0 = cf_div(M.b, M.d);
    chain_refl[chain_id * num_q + q_idx] = cf_abs2(R0);
}

// ── Q-smearing kernel ──────────────────────────────────────────────────────

kernel void kernel_qsmear(
    device float* qspread_refl [[buffer(0)]],
    device float* chain_refl [[buffer(1)]],
    constant int& num_datapoints [[buffer(2)]],
    constant int& num_chains [[buffer(3)]],
    uint2 gid [[thread_position_in_grid]])
{
    int chain_id = gid.y;
    int q_idx = gid.x;
    if (chain_id >= num_chains || q_idx >= num_datapoints) return;

    device float* src = qspread_refl + chain_id * 13 * num_datapoints;
    int base = 13 * q_idx;

    float val = src[base];
    val += 0.056f * src[base + 1];  val += 0.056f * src[base + 2];
    val += 0.135f * src[base + 3];  val += 0.135f * src[base + 4];
    val += 0.278f * src[base + 5];  val += 0.278f * src[base + 6];
    val += 0.487f * src[base + 7];  val += 0.487f * src[base + 8];
    val += 0.726f * src[base + 9];  val += 0.726f * src[base + 10];
    val += 0.923f * src[base + 11]; val += 0.923f * src[base + 12];

    chain_refl[chain_id * num_datapoints + q_idx] = val / 6.211f;
}

// ── Objective function kernel ──────────────────────────────────────────────

kernel void kernel_objective(
    constant GpuMeasConfigM& meas_config [[buffer(0)]],
    device GpuParamsM* chain_params [[buffer(1)]],
    device float* chain_refl [[buffer(2)]],
    device float* chain_gof [[buffer(3)]],
    device float* chain_chi2 [[buffer(4)]],
    constant float* yi_arr [[buffer(5)]],
    constant float* eyi_arr [[buffer(6)]],
    device float* chain_edp [[buffer(7)]],
    constant int& num_layers [[buffer(8)]],
    constant int& num_chains [[buffer(9)]],
    uint tid [[thread_position_in_threadgroup]],
    uint chain_id [[threadgroup_position_in_grid]],
    uint tg_size [[threads_per_threadgroup]])
{
    if ((int)chain_id >= num_chains) return;

    int n = meas_config.num_datapoints;
    device float* refl = chain_refl + chain_id * n;

    // Apply normalization (thread 0)
    if (tid == 0) {
        if (meas_config.force_norm && refl[0] != 0.0f) {
            float norm = 1.0f / refl[0];
            for (int i = 0; i < n; i++) refl[i] *= norm;
        }
        if (meas_config.imp_norm) {
            float norm = chain_params[chain_id].imp_norm;
            for (int i = 0; i < n; i++) refl[i] *= norm;
        }
    }
    threadgroup_barrier(mem_flags::mem_device);

    // Parallel reduction
    threadgroup float sdata_gof[256];
    threadgroup float sdata_chi2[256];

    float partial_gof = 0.0f;
    float partial_chi2 = 0.0f;
    int obj = meas_config.objective_function;

    for (int i = (int)tid; i < n; i += (int)tg_size) {
        float yi = yi_arr[i];
        float ri = refl[i];
        float eyi = eyi_arr[i];

        if (obj == 0) {
            float d = log(yi) - log(ri);
            partial_gof += d * d;
        } else if (obj == 1) {
            float ratio = yi / ri;
            if (ratio < 1.0f) ratio = 1.0f / ratio;
            partial_gof += (1.0f - ratio) * (1.0f - ratio);
        } else if (obj == 2) {
            float d = log(yi) - log(ri);
            partial_gof += d * d / fabs(log(eyi));
        } else {
            float ratio = yi / ri;
            if (ratio < 1.0f) ratio = 1.0f / ratio;
            float errormap = (yi / eyi) * (yi / eyi);
            partial_gof += (1.0f - ratio) * (1.0f - ratio) * errormap;
        }

        float diff = yi - ri;
        partial_chi2 += (diff * diff) / (eyi * eyi);
    }

    sdata_gof[tid] = partial_gof;
    sdata_chi2[tid] = partial_chi2;
    threadgroup_barrier(mem_flags::mem_threadgroup);

    for (uint s = tg_size / 2; s > 0; s >>= 1) {
        if (tid < s) {
            sdata_gof[tid] += sdata_gof[tid + s];
            sdata_chi2[tid] += sdata_chi2[tid + s];
        }
        threadgroup_barrier(mem_flags::mem_threadgroup);
    }

    if (tid == 0) {
        chain_gof[chain_id] = sdata_gof[0] / float(n + 1);
        chain_chi2[chain_id] = sdata_chi2[0] / float(n);
    }
}
