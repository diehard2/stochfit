#include "stochfit_cuda.h"

#if defined(STOCHFIT_HAS_CUDA)

#include "gpu_types.h"
#include <cuda_runtime.h>
#include <curand_kernel.h>
#include <cooperative_groups.h>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <atomic>

namespace cg = cooperative_groups;

// ── Complex float helpers ──────────────────────────────────────────────────

struct cfloat { float re, im; };

__device__ __forceinline__ cfloat cf_make(float r, float i) { return {r, i}; }
__device__ __forceinline__ cfloat cf_add(cfloat a, cfloat b) { return {a.re+b.re, a.im+b.im}; }
__device__ __forceinline__ cfloat cf_sub(cfloat a, cfloat b) { return {a.re-b.re, a.im-b.im}; }
__device__ __forceinline__ cfloat cf_mul(cfloat a, cfloat b) {
    return {a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re};
}
__device__ __forceinline__ cfloat cf_div(cfloat a, cfloat b) {
    float denom = b.re*b.re + b.im*b.im;
    return {(a.re*b.re + a.im*b.im)/denom, (a.im*b.re - a.re*b.im)/denom};
}
__device__ __forceinline__ float cf_abs2(cfloat a) { return a.re*a.re + a.im*a.im; }
__device__ __forceinline__ cfloat cf_scale(cfloat a, float s) { return {a.re*s, a.im*s}; }

// sqrt of complex number
__device__ __forceinline__ cfloat cf_sqrt(cfloat z) {
    float r = sqrtf(cf_abs2(z));
    float t = sqrtf(0.5f * (r + z.re));
    float s = (z.im >= 0.f ? 1.f : -1.f) * sqrtf(0.5f * (r - z.re));
    return {t, s};
}

// exp of complex number: e^(a+bi) = e^a * (cos b + i sin b)
__device__ __forceinline__ cfloat cf_exp(cfloat z) {
    float ea = expf(z.re);
    float s, c;
    sincosf(z.im, &s, &c);
    return {ea*c, ea*s};
}

// ── 2x2 complex matrix helpers for transfer matrix method ──────────────────

struct cmat2 {
    cfloat a, b, c, d;  // [[a,b],[c,d]]
};

__device__ __forceinline__ cmat2 cmat2_identity() {
    return {{1,0},{0,0},{0,0},{1,0}};
}

__device__ __forceinline__ cmat2 cmat2_mul(cmat2 L, cmat2 R) {
    return {
        cf_add(cf_mul(L.a, R.a), cf_mul(L.b, R.c)),
        cf_add(cf_mul(L.a, R.b), cf_mul(L.b, R.d)),
        cf_add(cf_mul(L.c, R.a), cf_mul(L.d, R.c)),
        cf_add(cf_mul(L.c, R.b), cf_mul(L.d, R.d)),
    };
}

// ── Device-side SA helper functions ────────────────────────────────────────

__device__ float device_fstun(float val, float best, float gamma, int func) {
    float diff = val - best;
    if (func == 0)
        return -expf(-gamma * diff);
    else if (func == 1)
        return sinhf(gamma * diff) - 1.0f;
    else {
        float gd = gamma * diff;
        return logf(gd + sqrtf(gd * gd + 1.0f)) - 1.0f;
    }
}

__device__ bool device_evaluate_greedy(GpuSAState* state, float new_energy) {
    if (new_energy < state->best_energy) {
        state->best_energy = new_energy;
        state->best_solution = new_energy;
        return true;
    }
    return false;
}

__device__ bool device_evaluate_sa(GpuSAState* state, float new_energy, curandState* rng) {
    float cur = state->current_energy;
    if (fminf(new_energy, cur) < state->best_solution) {
        state->best_solution = fminf(new_energy, cur);
        return true;
    }
    float deltaE = new_energy - cur;
    state->iteration++;
    if (state->iteration % state->plat_time == 0) {
        if (state->temperature > 1e-30f)
            state->temperature /= state->slope;
    }
    float prob = expf(-state->temperature * deltaE);
    return curand_uniform(rng) < prob;
}

__device__ bool device_evaluate_stun(GpuSAState* state, float new_energy, curandState* rng) {
    float cur = state->current_energy;
    if (fminf(new_energy, cur) < state->best_solution) {
        state->best_solution = fminf(new_energy, cur);
        return true;
    }
    float deltaE = device_fstun(new_energy, state->best_solution, state->gamma, state->stun_func)
                  - device_fstun(cur, state->best_solution, state->gamma, state->stun_func);

    state->iteration++;

    if (state->adaptive) {
        if (state->iteration % state->temp_iter == 0) {
            // Simplified adaptive temperature adjustment
            if (state->avg_fstun > state->best_energy) {
                if (state->temperature < 1e30f)
                    state->temperature *= 0.5f / state->slope;
            } else {
                if (state->temperature > 1e-30f)
                    state->temperature *= state->slope;
            }
            if (state->iteration % state->stun_dec_iter == 0) {
                state->avg_fstun *= state->gammadec;
            }
        }
    } else {
        if (state->iteration % state->plat_time == 0) {
            if (state->temperature > 1e-30f)
                state->temperature /= state->slope;
        }
    }

    float prob = expf(-state->temperature * deltaE);
    return curand_uniform(rng) < prob;
}

__device__ void device_mutate(GpuParams* params, curandState* rng, GpuSAState* state) {
    int ii = curand(rng) % params->num_boxes;
    int perc = (curand(rng) % 100) + 1;
    float step = state->stepsize;

    if (perc > state->sigmasearch + state->normsearch + state->abssearch) {
        // Mutate a box SLD value
        float cur = params->sld_values[ii + 1];
        float newval = cur + (curand_uniform(rng) * 2.0f - 1.0f) * step;
        newval = fmaxf(params->param_low, fminf(params->param_high, newval));
        params->sld_values[ii + 1] = newval;
    } else if (perc <= state->sigmasearch) {
        // Mutate roughness
        if (!params->fix_roughness) {
            float roughmult = 5.0f / 3.0f;
            float cur = params->roughness;
            float lo = cur * (1.0f - roughmult * step);
            float hi = cur * (1.0f + roughmult * step);
            float newval = lo + curand_uniform(rng) * (hi - lo);
            newval = fmaxf(params->roughness_low, fminf(params->roughness_high, newval));
            params->roughness = newval;
        }
    } else if (perc <= state->sigmasearch + state->abssearch) {
        // Mutate surface absorption
        if (params->use_surf_abs) {
            float cur = params->surf_abs;
            float lo = cur * (1.0f - step);
            float hi = cur * (1.0f + step);
            float newval = lo + curand_uniform(rng) * (hi - lo);
            newval = fmaxf(0.0f, fminf(params->surfabs_high, newval));
            params->surf_abs = newval;
        }
    } else {
        // Mutate imperfect normalization
        if (params->fix_imp_norm) {
            float cur = params->imp_norm;
            float lo = cur * (1.0f - step);
            float hi = cur * (1.0f + step);
            float newval = lo + curand_uniform(rng) * (hi - lo);
            newval = fmaxf(0.0f, fminf(params->impnorm_high, newval));
            params->imp_norm = newval;
        }
    }
}

// ── EDP generation kernel ──────────────────────────────────────────────────

__global__ void kernel_generate_edp(
    const GpuEDPConfig edp,
    const GpuParams* __restrict__ chain_params,
    float* __restrict__ chain_edp,      // [num_chains * num_layers]
    float* __restrict__ chain_dedp,     // [num_chains * num_layers]
    float* __restrict__ chain_rho_arr,  // [num_chains * (GPU_MAX_BOXES+2)]
    int num_chains)
{
    int chain_id = blockIdx.y;
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (chain_id >= num_chains) return;

    const GpuParams& p = chain_params[chain_id];
    int num_layers = edp.num_layers;
    int refllayers = p.real_params_size - 1;

    // Phase 1: Compute rho array (thread 0 of each chain)
    float* rho_arr = chain_rho_arr + chain_id * (GPU_MAX_BOXES + 2);
    if (blockIdx.x == 0) {
        for (int k = threadIdx.x; k < refllayers; k += blockDim.x) {
            rho_arr[k] = edp.rho * (p.sld_values[k + 1] - p.sld_values[k]) * 0.5f;
        }
    }
    __syncthreads();

    // Phase 2: Compute EDP for each layer
    if (idx < num_layers) {
        float roughness = p.roughness;
        if (roughness < 1e-6f) roughness = 1e-6f;
        roughness = 1.0f / (roughness * sqrtf(2.0f));

        float supersld = p.sld_values[0] * edp.rho;
        float edp_val = supersld;

        for (int k = 0; k < refllayers; k++) {
            float dist = (edp.ed_spacing[idx] - edp.dist_array[k]) * roughness;
            if (dist > 6.0f)
                edp_val += rho_arr[k] * 2.0f;
            else if (dist > -6.0f)
                edp_val += rho_arr[k] * (1.0f + erff(dist));
        }

        int offset = chain_id * num_layers + idx;
        chain_edp[offset] = edp_val;
        chain_dedp[offset] = 2.0f * edp_val;
    }
}

// ── Parratt reflectivity kernel (sequential per Q-point, parallelized over Q) ──

__global__ void kernel_parratt(
    const GpuEDPConfig edp,
    const GpuMeasurement meas,
    const GpuParams* __restrict__ chain_params,
    const float* __restrict__ chain_dedp,
    float* __restrict__ chain_refl,
    int num_chains,
    int num_q_per_call)  // num_datapoints or 13*num_datapoints
{
    int chain_id = blockIdx.y;
    int q_idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (chain_id >= num_chains || q_idx >= num_q_per_call) return;

    const float* dedp = chain_dedp + chain_id * edp.num_layers;
    int num_layers = edp.num_layers;
    float k0 = edp.k0;
    float dz = edp.dz;

    // Select the appropriate sintheta/sinsquaredtheta arrays
    const float* sintheta_arr = meas.use_qspread ? meas.qspread_sintheta : meas.sintheta;
    const float* sinsq_arr = meas.use_qspread ? meas.qspread_sin2theta : meas.sinsquaredtheta;

    float sintheta_val = sintheta_arr[q_idx];
    float sinsq_val = sinsq_arr[q_idx];

    float dedp0 = dedp[0];
    float indexsup_re = 1.0f - dedp0 / 2.0f;
    float indexsup2_re = indexsup_re * indexsup_re;

    // Length multiplier for phase factor: -2*i*dz
    cfloat length_mult = cf_make(0.0f, -2.0f * dz);

    // Compute wavevectors, phase factors, and Fresnel coefficients
    // Then do Parratt recursion from bottom to top

    // Use transfer matrix approach with sequential scan
    // M_total = M_0 * M_1 * ... * M_{N-2}
    // where M_i = [[ak_i, ak_i*rj_i], [rj_i, 1]]
    // R = M_total.b / M_total.d (since R_{N-1} = 0)

    cmat2 M = cmat2_identity();

    // Find EDP offsets for optimization (skip uniform regions)
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

    // kk[0]: superphase wavevector
    cfloat kk_sup = cf_make(k0 * indexsup_re * sintheta_val, 0.0f);
    // kk for uniform low region
    cfloat kk_low_arg = cf_make(indexsup2_re * sinsq_val - dedp[1] + dedp0, 0.0f);
    cfloat kk_low = cf_scale(cf_sqrt(kk_low_arg), k0);
    // kk for uniform high region
    cfloat kk_high_arg = cf_make(indexsup2_re * sinsq_val - dedp_last + dedp0, 0.0f);
    cfloat kk_high = cf_scale(cf_sqrt(kk_high_arg), k0);

    // Phase factors for uniform regions
    cfloat ak_low = cf_exp(cf_mul(length_mult, kk_low));
    cfloat ak_high = cf_exp(cf_mul(length_mult, kk_high));

    // Process layer 0 (superphase boundary)
    cfloat kk_prev = kk_sup;
    cfloat kk_cur;

    for (int i = 0; i < num_layers - 1; i++) {
        // Determine kk[i+1]
        if (i + 1 <= low_offset) {
            kk_cur = kk_low;
        } else if (i + 1 >= high_offset) {
            kk_cur = kk_high;
        } else {
            cfloat arg = cf_make(indexsup2_re * sinsq_val - dedp[i + 1] + dedp0, 0.0f);
            kk_cur = cf_scale(cf_sqrt(arg), k0);
        }

        // Phase factor ak[i]
        cfloat ak_i;
        if (i == 0 || i == num_layers - 1) {
            ak_i = cf_make(1.0f, 0.0f);  // boundary layers are infinite
        } else if (i <= low_offset) {
            ak_i = ak_low;
        } else if (i >= high_offset) {
            ak_i = ak_high;
        } else {
            ak_i = cf_exp(cf_mul(length_mult, kk_prev));
        }

        // Fresnel coefficient rj[i]
        cfloat sum = cf_add(kk_prev, kk_cur);
        cfloat diff = cf_sub(kk_prev, kk_cur);
        cfloat rj_i;
        if (cf_abs2(sum) > 1e-30f) {
            rj_i = cf_div(diff, sum);
        } else {
            rj_i = cf_make(0.0f, 0.0f);
        }

        // Build transfer matrix for layer i: [[ak_i, ak_i*rj_i], [rj_i, 1]]
        cmat2 layer = {ak_i, cf_mul(ak_i, rj_i), rj_i, cf_make(1.0f, 0.0f)};
        M = cmat2_mul(M, layer);

        kk_prev = kk_cur;
    }

    // Extract reflectivity: R = |M.b / M.d|^2
    cfloat R0 = cf_div(M.b, M.d);
    float refl_val = cf_abs2(R0);

    chain_refl[chain_id * num_q_per_call + q_idx] = refl_val;
}

// ── Q-smearing kernel ──────────────────────────────────────────────────────

__global__ void kernel_qsmear(
    const float* __restrict__ qspread_refl,  // [num_chains * 13 * num_datapoints]
    float* __restrict__ chain_refl,          // [num_chains * num_datapoints]
    int num_datapoints,
    int num_chains)
{
    int chain_id = blockIdx.y;
    int q_idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (chain_id >= num_chains || q_idx >= num_datapoints) return;

    const float* src = qspread_refl + chain_id * 13 * num_datapoints;
    int base = 13 * q_idx;

    float val = src[base];
    val += 0.056f * src[base + 1];
    val += 0.056f * src[base + 2];
    val += 0.135f * src[base + 3];
    val += 0.135f * src[base + 4];
    val += 0.278f * src[base + 5];
    val += 0.278f * src[base + 6];
    val += 0.487f * src[base + 7];
    val += 0.487f * src[base + 8];
    val += 0.726f * src[base + 9];
    val += 0.726f * src[base + 10];
    val += 0.923f * src[base + 11];
    val += 0.923f * src[base + 12];

    chain_refl[chain_id * num_datapoints + q_idx] = val / GPU_QSPREAD_NORM;
}

// ── Objective function + normalization kernel ──────────────────────────────

__global__ void kernel_objective(
    const GpuMeasurement meas,
    const GpuParams* __restrict__ chain_params,
    float* __restrict__ chain_refl,     // [num_chains * num_datapoints] - modified in place for norm
    float* __restrict__ chain_gof,      // [num_chains]
    float* __restrict__ chain_chi2,     // [num_chains]
    const float* __restrict__ chain_edp, // [num_chains * num_layers] for XR density check
    int num_layers,
    int num_chains)
{
    __shared__ float sdata_gof[256];
    __shared__ float sdata_chi2[256];

    int chain_id = blockIdx.x;
    if (chain_id >= num_chains) return;

    int tid = threadIdx.x;
    int n = meas.num_datapoints;
    float* refl = chain_refl + chain_id * n;
    const GpuParams& p = chain_params[chain_id];

    // XR density check
    if (meas.xr_only && tid == 0) {
        const float* edp = chain_edp + chain_id * num_layers;
        for (int i = 0; i < num_layers; i++) {
            if (edp[i] < 0.0f) {
                chain_gof[chain_id] = -1.0f;
                return;
            }
        }
    }
    __syncthreads();

    // Apply normalization
    if (meas.force_norm && tid == 0) {
        float norm = 1.0f / refl[0];
        for (int i = 0; i < n; i++) refl[i] *= norm;
    }
    if (meas.imp_norm && tid == 0) {
        float norm = p.imp_norm;
        for (int i = 0; i < n; i++) refl[i] *= norm;
    }
    __syncthreads();

    // Parallel reduction for objective function
    float partial_gof = 0.0f;
    float partial_chi2 = 0.0f;
    int obj = meas.objective_function;

    for (int i = tid; i < n; i += blockDim.x) {
        float yi = meas.refl_values[i];
        float ri = refl[i];
        float eyi = meas.refl_errors[i];

        // Goodness of fit
        if (obj == 0) {
            // Log difference
            float d = logf(yi) - logf(ri);
            partial_gof += d * d;
        } else if (obj == 1) {
            // Inverse difference
            float ratio = yi / ri;
            if (ratio < 1.0f) ratio = 1.0f / ratio;
            partial_gof += (1.0f - ratio) * (1.0f - ratio);
        } else if (obj == 2) {
            // Log difference with errors
            float d = logf(yi) - logf(ri);
            partial_gof += d * d / fabsf(logf(eyi));
        } else {
            // Inverse difference with errors
            float ratio = yi / ri;
            if (ratio < 1.0f) ratio = 1.0f / ratio;
            float errormap = (yi / eyi) * (yi / eyi);
            partial_gof += (1.0f - ratio) * (1.0f - ratio) * errormap;
        }

        // Chi square
        float diff = yi - ri;
        partial_chi2 += (diff * diff) / (eyi * eyi);
    }

    sdata_gof[tid] = partial_gof;
    sdata_chi2[tid] = partial_chi2;
    __syncthreads();

    // Tree reduction
    for (int s = blockDim.x / 2; s > 0; s >>= 1) {
        if (tid < s) {
            sdata_gof[tid] += sdata_gof[tid + s];
            sdata_chi2[tid] += sdata_chi2[tid + s];
        }
        __syncthreads();
    }

    if (tid == 0) {
        chain_gof[chain_id] = sdata_gof[0] / (float)(n + 1);
        chain_chi2[chain_id] = sdata_chi2[0] / (float)n;
    }
}

// ── SA accept/reject + mutate kernel ───────────────────────────────────────

__global__ void kernel_sa_step(
    GpuSAState* __restrict__ chain_states,
    GpuParams* __restrict__ chain_params,
    GpuParams* __restrict__ chain_params_backup,
    float* __restrict__ chain_gof,
    float* __restrict__ chain_chi2,
    curandState* __restrict__ rng_states,
    int num_chains,
    int phase)  // 0 = backup+mutate, 1 = accept/reject
{
    int chain_id = blockIdx.x * blockDim.x + threadIdx.x;
    if (chain_id >= num_chains) return;

    GpuSAState* state = &chain_states[chain_id];
    GpuParams* params = &chain_params[chain_id];
    curandState* rng = &rng_states[chain_id];

    if (phase == 0) {
        // Backup params and mutate
        chain_params_backup[chain_id] = *params;
        device_mutate(params, rng, state);
    } else {
        // Accept/reject
        float new_energy = chain_gof[chain_id];

        // Check for XR failure
        if (new_energy < 0.0f) {
            *params = chain_params_backup[chain_id];
            return;
        }

        bool accepted = false;
        if (state->algorithm == 0)
            accepted = device_evaluate_greedy(state, new_energy);
        else if (state->algorithm == 1)
            accepted = device_evaluate_sa(state, new_energy, rng);
        else
            accepted = device_evaluate_stun(state, new_energy, rng);

        if (accepted) {
            state->current_energy = new_energy;
        } else {
            *params = chain_params_backup[chain_id];
        }
    }
}

// ── Find global best kernel ────────────────────────────────────────────────

__global__ void kernel_find_best(
    const GpuSAState* __restrict__ chain_states,
    const GpuParams* __restrict__ chain_params,
    const float* __restrict__ chain_chi2,
    GpuResultSummary* __restrict__ result,
    int num_chains)
{
    // Single thread finds the best chain
    if (threadIdx.x != 0) return;

    float best = 1e30f;
    int best_idx = 0;
    for (int i = 0; i < num_chains; i++) {
        if (chain_states[i].best_solution < best) {
            best = chain_states[i].best_solution;
            best_idx = i;
        }
    }

    result->best_energy = best;
    result->best_chain = best_idx;
    result->best_roughness = chain_params[best_idx].roughness;
    result->best_gof = chain_states[best_idx].best_solution;
    result->best_chi_square = chain_chi2[best_idx];
    result->best_imp_norm = chain_params[best_idx].imp_norm;
    result->best_surf_abs = chain_params[best_idx].surf_abs;
    result->best_temperature = chain_states[best_idx].temperature;
    result->best_avg_fstun = chain_states[best_idx].avg_fstun;

    int rps = chain_params[best_idx].real_params_size;
    for (int i = 0; i < rps; i++)
        result->best_params[i] = chain_params[best_idx].sld_values[i];
}

// ── RNG initialization kernel ──────────────────────────────────────────────

__global__ void kernel_init_rng(curandState* states, unsigned long long seed, int n) {
    int id = blockIdx.x * blockDim.x + threadIdx.x;
    if (id < n)
        curand_init(seed, id, 0, &states[id]);
}

// ── CUDA error checking ───────────────────────────────────────────────────

#define CUDA_CHECK(call) do { \
    cudaError_t err = (call); \
    if (err != cudaSuccess) { \
        fprintf(stderr, "CUDA error at %s:%d: %s\n", __FILE__, __LINE__, \
                cudaGetErrorString(err)); \
    } \
} while(0)

// ── CudaSARunner implementation ────────────────────────────────────────────

class CudaSARunner : public GpuSARunner {
public:
    ~CudaSARunner() override { cleanup(); }

    void initialize(
        const GpuSAState& sa_state,
        const GpuParams& params,
        const GpuMeasurement& measurement,
        const GpuEDPConfig& edp_config,
        int num_chains) override;

    void run_batch(int iterations) override;
    void cancel() override;
    GpuResultSummary get_result() const override;
    void get_best_edp(float* edp_out, int count) const override;
    void get_best_reflectivity(float* refl_out, int count) const override;
    int num_chains() const override { return m_num_chains; }
    bool is_finished() const override { return m_finished; }

private:
    void cleanup();

    int m_num_chains = 0;
    int m_num_layers = 0;
    int m_num_datapoints = 0;
    int m_num_q_per_call = 0;
    bool m_use_qspread = false;
    std::atomic<bool> m_cancelled{false};
    bool m_finished = false;

    // Device memory
    GpuSAState* d_chain_states = nullptr;
    GpuParams* d_chain_params = nullptr;
    GpuParams* d_chain_params_backup = nullptr;
    float* d_chain_edp = nullptr;
    float* d_chain_dedp = nullptr;
    float* d_chain_rho_arr = nullptr;
    float* d_chain_refl = nullptr;
    float* d_chain_qspread_refl = nullptr;
    float* d_chain_gof = nullptr;
    float* d_chain_chi2 = nullptr;
    curandState* d_rng_states = nullptr;

    // Device copies of measurement/edp config
    GpuMeasurement d_meas_struct{};
    GpuEDPConfig d_edp_struct{};

    // Host-side measurement float arrays (for device copy)
    float* d_meas_q = nullptr;
    float* d_meas_refl = nullptr;
    float* d_meas_err = nullptr;
    float* d_meas_sintheta = nullptr;
    float* d_meas_sinsq = nullptr;
    float* d_meas_qspread_sin = nullptr;
    float* d_meas_qspread_sin2 = nullptr;
    float* d_edp_spacing = nullptr;
    float* d_edp_dist = nullptr;

    // Pinned result buffer
    GpuResultSummary* h_result = nullptr;
    GpuResultSummary* d_result = nullptr;
};

void CudaSARunner::cleanup() {
    cudaFree(d_chain_states);
    cudaFree(d_chain_params);
    cudaFree(d_chain_params_backup);
    cudaFree(d_chain_edp);
    cudaFree(d_chain_dedp);
    cudaFree(d_chain_rho_arr);
    cudaFree(d_chain_refl);
    cudaFree(d_chain_qspread_refl);
    cudaFree(d_chain_gof);
    cudaFree(d_chain_chi2);
    cudaFree(d_rng_states);
    cudaFree(d_meas_q);
    cudaFree(d_meas_refl);
    cudaFree(d_meas_err);
    cudaFree(d_meas_sintheta);
    cudaFree(d_meas_sinsq);
    cudaFree(d_meas_qspread_sin);
    cudaFree(d_meas_qspread_sin2);
    cudaFree(d_edp_spacing);
    cudaFree(d_edp_dist);
    cudaFree(d_result);
    if (h_result) cudaFreeHost(h_result);

    d_chain_states = nullptr;
    d_chain_params = nullptr;
    h_result = nullptr;
}

void CudaSARunner::initialize(
    const GpuSAState& sa_state,
    const GpuParams& params,
    const GpuMeasurement& measurement,
    const GpuEDPConfig& edp_config,
    int num_chains)
{
    cleanup();

    m_num_chains = num_chains;
    m_num_layers = edp_config.num_layers;
    m_num_datapoints = measurement.num_datapoints;
    m_use_qspread = measurement.use_qspread != 0;
    m_num_q_per_call = m_use_qspread ? m_num_datapoints * 13 : m_num_datapoints;
    m_cancelled = false;
    m_finished = false;

    // Allocate per-chain state arrays
    CUDA_CHECK(cudaMalloc(&d_chain_states, num_chains * sizeof(GpuSAState)));
    CUDA_CHECK(cudaMalloc(&d_chain_params, num_chains * sizeof(GpuParams)));
    CUDA_CHECK(cudaMalloc(&d_chain_params_backup, num_chains * sizeof(GpuParams)));
    CUDA_CHECK(cudaMalloc(&d_chain_edp, num_chains * m_num_layers * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_chain_dedp, num_chains * m_num_layers * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_chain_rho_arr, num_chains * (GPU_MAX_BOXES + 2) * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_chain_refl, num_chains * m_num_datapoints * sizeof(float)));
    if (m_use_qspread)
        CUDA_CHECK(cudaMalloc(&d_chain_qspread_refl, num_chains * m_num_q_per_call * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_chain_gof, num_chains * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_chain_chi2, num_chains * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_rng_states, num_chains * sizeof(curandState)));

    // Initialize all chains with the same initial state/params
    std::vector<GpuSAState> h_states(num_chains, sa_state);
    std::vector<GpuParams> h_params(num_chains, params);
    CUDA_CHECK(cudaMemcpy(d_chain_states, h_states.data(), num_chains * sizeof(GpuSAState), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_chain_params, h_params.data(), num_chains * sizeof(GpuParams), cudaMemcpyHostToDevice));

    // Initialize RNG with different seeds per chain
    int rng_blocks = (num_chains + 255) / 256;
    kernel_init_rng<<<rng_blocks, 256>>>(d_rng_states, 42ull, num_chains);

    // Copy measurement data to device (converting double -> float happened on host)
    int n = m_num_datapoints;
    CUDA_CHECK(cudaMalloc(&d_meas_q, n * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_meas_refl, n * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_meas_err, n * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_meas_sintheta, n * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_meas_sinsq, n * sizeof(float)));
    CUDA_CHECK(cudaMemcpy(d_meas_q, measurement.q_values, n * sizeof(float), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_meas_refl, measurement.refl_values, n * sizeof(float), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_meas_err, measurement.refl_errors, n * sizeof(float), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_meas_sintheta, measurement.sintheta, n * sizeof(float), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_meas_sinsq, measurement.sinsquaredtheta, n * sizeof(float), cudaMemcpyHostToDevice));

    if (m_use_qspread) {
        CUDA_CHECK(cudaMalloc(&d_meas_qspread_sin, n * 13 * sizeof(float)));
        CUDA_CHECK(cudaMalloc(&d_meas_qspread_sin2, n * 13 * sizeof(float)));
        CUDA_CHECK(cudaMemcpy(d_meas_qspread_sin, measurement.qspread_sintheta, n * 13 * sizeof(float), cudaMemcpyHostToDevice));
        CUDA_CHECK(cudaMemcpy(d_meas_qspread_sin2, measurement.qspread_sin2theta, n * 13 * sizeof(float), cudaMemcpyHostToDevice));
    }

    // Setup device measurement struct
    d_meas_struct = measurement;
    d_meas_struct.q_values = d_meas_q;
    d_meas_struct.refl_values = d_meas_refl;
    d_meas_struct.refl_errors = d_meas_err;
    d_meas_struct.sintheta = d_meas_sintheta;
    d_meas_struct.sinsquaredtheta = d_meas_sinsq;
    d_meas_struct.qspread_sintheta = d_meas_qspread_sin;
    d_meas_struct.qspread_sin2theta = d_meas_qspread_sin2;

    // Copy EDP config arrays to device
    CUDA_CHECK(cudaMalloc(&d_edp_spacing, m_num_layers * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_edp_dist, (GPU_MAX_BOXES + 2) * sizeof(float)));
    CUDA_CHECK(cudaMemcpy(d_edp_spacing, edp_config.ed_spacing, m_num_layers * sizeof(float), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_edp_dist, edp_config.dist_array, (params.num_boxes + 2) * sizeof(float), cudaMemcpyHostToDevice));

    d_edp_struct = edp_config;
    d_edp_struct.ed_spacing = d_edp_spacing;
    d_edp_struct.dist_array = d_edp_dist;

    // Pinned result buffer for host polling
    CUDA_CHECK(cudaMallocHost(&h_result, sizeof(GpuResultSummary)));
    CUDA_CHECK(cudaMalloc(&d_result, sizeof(GpuResultSummary)));
    memset(h_result, 0, sizeof(GpuResultSummary));

    CUDA_CHECK(cudaDeviceSynchronize());
}

void CudaSARunner::run_batch(int iterations) {
    int nc = m_num_chains;
    int nl = m_num_layers;
    int nq = m_num_q_per_call;
    int nd = m_num_datapoints;

    // Kernel launch configurations
    int edp_threads = 256;
    int edp_blocks_x = (nl + edp_threads - 1) / edp_threads;
    dim3 edp_grid(edp_blocks_x, nc);

    int parratt_threads = 256;
    int parratt_blocks_x = (nq + parratt_threads - 1) / parratt_threads;
    dim3 parratt_grid(parratt_blocks_x, nc);

    int sa_blocks = (nc + 255) / 256;
    int sa_threads = std::min(nc, 256);

    dim3 obj_grid(nc);
    int obj_threads = 256;

    for (int iter = 0; iter < iterations && !m_cancelled.load(std::memory_order_relaxed); iter++) {
        // Phase 0: Backup params and mutate
        kernel_sa_step<<<sa_blocks, sa_threads>>>(
            d_chain_states, d_chain_params, d_chain_params_backup,
            d_chain_gof, d_chain_chi2, d_rng_states, nc, 0);

        // Phase 1: Generate EDP
        kernel_generate_edp<<<edp_grid, edp_threads>>>(
            d_edp_struct, d_chain_params, d_chain_edp, d_chain_dedp,
            d_chain_rho_arr, nc);

        // Phase 2: Parratt reflectivity
        if (m_use_qspread) {
            // Compute reflectivity at all 13*nd Q-points
            kernel_parratt<<<parratt_grid, parratt_threads>>>(
                d_edp_struct, d_meas_struct, d_chain_params, d_chain_dedp,
                d_chain_qspread_refl, nc, nq);

            // Q-smearing reduction
            int qsmear_blocks_x = (nd + 255) / 256;
            dim3 qsmear_grid(qsmear_blocks_x, nc);
            kernel_qsmear<<<qsmear_grid, 256>>>(
                d_chain_qspread_refl, d_chain_refl, nd, nc);
        } else {
            kernel_parratt<<<parratt_grid, parratt_threads>>>(
                d_edp_struct, d_meas_struct, d_chain_params, d_chain_dedp,
                d_chain_refl, nc, nq);
        }

        // Phase 3: Objective function
        kernel_objective<<<obj_grid, obj_threads>>>(
            d_meas_struct, d_chain_params, d_chain_refl,
            d_chain_gof, d_chain_chi2, d_chain_edp, nl, nc);

        // Phase 4: Accept/reject
        kernel_sa_step<<<sa_blocks, sa_threads>>>(
            d_chain_states, d_chain_params, d_chain_params_backup,
            d_chain_gof, d_chain_chi2, d_rng_states, nc, 1);
    }

    // Find global best across all chains
    kernel_find_best<<<1, 1>>>(d_chain_states, d_chain_params, d_chain_chi2, d_result, nc);

    // Copy result to pinned host memory
    CUDA_CHECK(cudaMemcpy(h_result, d_result, sizeof(GpuResultSummary), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaDeviceSynchronize());
}

void CudaSARunner::cancel() {
    m_cancelled.store(true, std::memory_order_relaxed);
}

GpuResultSummary CudaSARunner::get_result() const {
    return *h_result;
}

void CudaSARunner::get_best_edp(float* edp_out, int count) const {
    if (!h_result || h_result->best_chain < 0) return;
    int offset = h_result->best_chain * m_num_layers;
    int n = std::min(count, m_num_layers);
    CUDA_CHECK(cudaMemcpy(edp_out, d_chain_edp + offset, n * sizeof(float), cudaMemcpyDeviceToHost));
}

void CudaSARunner::get_best_reflectivity(float* refl_out, int count) const {
    if (!h_result || h_result->best_chain < 0) return;
    int offset = h_result->best_chain * m_num_datapoints;
    int n = std::min(count, m_num_datapoints);
    CUDA_CHECK(cudaMemcpy(refl_out, d_chain_refl + offset, n * sizeof(float), cudaMemcpyDeviceToHost));
}

std::unique_ptr<GpuSARunner> create_cuda_runner() {
    return std::make_unique<CudaSARunner>();
}

#endif // STOCHFIT_HAS_CUDA
