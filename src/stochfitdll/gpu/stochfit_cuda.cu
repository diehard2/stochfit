#include "gpu_sa_runner.h"
#include "gpu_types.h"

#if defined(STOCHFIT_HAS_CUDA)

// WIN32_LEAN_AND_MEAN prevents inclusion of COM/OLE/RPC headers (rpcndr.h etc.)
// which trigger cudafe++ assertion failures with Windows SDK 10.0.26100.0+.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "gpu_types.h"
#include <cuda_runtime.h>
#include <curand_kernel.h>
#include <thrust/complex.h>
#include <cstdio>
#include <cmath>
#include <algorithm>

// Q-spread weights as device constant (GPU_QSPREAD_WEIGHTS in gpu_types.h is
// constexpr host-only; CUDA device code needs __constant__ for array access)
__constant__ float kQSpreadWeights[13] = {
    1.000f,
    0.056f, 0.056f,
    0.135f, 0.135f,
    0.278f, 0.278f,
    0.487f, 0.487f,
    0.726f, 0.726f,
    0.923f, 0.923f,
};

// ── Complex float type ─────────────────────────────────────────────────────

using cfloat = thrust::complex<float>;

// ── 2x2 complex matrix helpers for transfer matrix method ──────────────────

struct cmat2 {
    cfloat a, b, c, d;  // [[a,b],[c,d]]
};

__device__ __forceinline__ cmat2 cmat2_identity() {
    return {cfloat(1,0), cfloat(0,0), cfloat(0,0), cfloat(1,0)};
}

__device__ __forceinline__ cmat2 cmat2_mul(cmat2 L, cmat2 R) {
    return {
        L.a * R.a + L.b * R.c,
        L.a * R.b + L.b * R.d,
        L.c * R.a + L.d * R.c,
        L.c * R.b + L.d * R.d,
    };
}

// ── Single Q-point Parratt reflectivity (for use inside persistent kernel) ─

__device__ float device_parratt_single(
    const float* dedp, int num_layers,
    float sintheta_val, float sinsq_val,
    float k0, float dz)
{
    float dedp0 = dedp[0];
    float indexsup_re = 1.0f - dedp0 / 2.0f;
    float indexsup2_re = indexsup_re * indexsup_re;
    cfloat length_mult(0.0f, -2.0f * dz);

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

    cfloat kk_sup(k0 * indexsup_re * sintheta_val, 0.0f);
    cfloat kk_low_arg(indexsup2_re * sinsq_val - dedp[1] + dedp0, 0.0f);
    cfloat kk_low = thrust::sqrt(kk_low_arg) * k0;
    cfloat kk_high_arg(indexsup2_re * sinsq_val - dedp_last + dedp0, 0.0f);
    cfloat kk_high = thrust::sqrt(kk_high_arg) * k0;
    cfloat ak_low = thrust::exp(length_mult * kk_low);
    cfloat ak_high = thrust::exp(length_mult * kk_high);

    cmat2 M = cmat2_identity();
    cfloat kk_prev = kk_sup;

    for (int i = 0; i < num_layers - 1; i++) {
        cfloat kk_cur;
        if (i + 1 <= low_offset) kk_cur = kk_low;
        else if (i + 1 >= high_offset) kk_cur = kk_high;
        else {
            cfloat arg(indexsup2_re * sinsq_val - dedp[i + 1] + dedp0, 0.0f);
            kk_cur = thrust::sqrt(arg) * k0;
        }

        cfloat ak_i;
        if (i == 0 || i == num_layers - 1) ak_i = cfloat(1.0f, 0.0f);
        else if (i <= low_offset) ak_i = ak_low;
        else if (i >= high_offset) ak_i = ak_high;
        else ak_i = thrust::exp(length_mult * kk_prev);

        cfloat sum = kk_prev + kk_cur;
        cfloat diff = kk_prev - kk_cur;
        cfloat rj_i = thrust::norm(sum) > 1e-30f ? diff / sum : cfloat(0.0f, 0.0f);

        cmat2 layer = {ak_i, ak_i * rj_i, rj_i, cfloat(1.0f, 0.0f)};
        M = cmat2_mul(M, layer);
        kk_prev = kk_cur;
    }

    cfloat R0 = M.b / M.d;
    return thrust::norm(R0);
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
    cfloat length_mult(0.0f, -2.0f * dz);

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
    cfloat kk_sup(k0 * indexsup_re * sintheta_val, 0.0f);
    // kk for uniform low region
    cfloat kk_low_arg(indexsup2_re * sinsq_val - dedp[1] + dedp0, 0.0f);
    cfloat kk_low = thrust::sqrt(kk_low_arg) * k0;
    // kk for uniform high region
    cfloat kk_high_arg(indexsup2_re * sinsq_val - dedp_last + dedp0, 0.0f);
    cfloat kk_high = thrust::sqrt(kk_high_arg) * k0;

    // Phase factors for uniform regions
    cfloat ak_low = thrust::exp(length_mult * kk_low);
    cfloat ak_high = thrust::exp(length_mult * kk_high);

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
            cfloat arg(indexsup2_re * sinsq_val - dedp[i + 1] + dedp0, 0.0f);
            kk_cur = thrust::sqrt(arg) * k0;
        }

        // Phase factor ak[i]
        cfloat ak_i;
        if (i == 0 || i == num_layers - 1) {
            ak_i = cfloat(1.0f, 0.0f);  // boundary layers are infinite
        } else if (i <= low_offset) {
            ak_i = ak_low;
        } else if (i >= high_offset) {
            ak_i = ak_high;
        } else {
            ak_i = thrust::exp(length_mult * kk_prev);
        }

        // Fresnel coefficient rj[i]
        cfloat sum = kk_prev + kk_cur;
        cfloat diff = kk_prev - kk_cur;
        cfloat rj_i = thrust::norm(sum) > 1e-30f ? diff / sum : cfloat(0.0f, 0.0f);

        // Build transfer matrix for layer i: [[ak_i, ak_i*rj_i], [rj_i, 1]]
        cmat2 layer = {ak_i, ak_i * rj_i, rj_i, cfloat(1.0f, 0.0f)};
        M = cmat2_mul(M, layer);

        kk_prev = kk_cur;
    }

    // Extract reflectivity: R = |M.b / M.d|^2
    cfloat R0 = M.b / M.d;
    float refl_val = thrust::norm(R0);

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
    result->best_chi_square = 0.0f; // recomputed on CPU for double precision
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

// ── Persistent SA kernel ──────────────────────────────────────────────────
// One thread block per chain.  All SA iterations run inside the kernel,
// eliminating per-iteration kernel launch overhead and keeping the EDP
// profile in fast shared memory.

__global__ void kernel_sa_persistent(
    GpuSAState* __restrict__ chain_states,
    GpuParams* __restrict__ chain_params,
    const GpuEDPConfig edp,
    const GpuMeasurement meas,
    curandState* __restrict__ rng_states,
    volatile int* __restrict__ cancel_flag,
    int num_iterations)
{
    const int chain_id = blockIdx.x;
    const int tid = threadIdx.x;
    const int nthreads = blockDim.x;
    const int nl = edp.num_layers;
    const int nd = meas.num_datapoints;

    // Dynamic shared memory: s_dedp[nl] | s_rho[66] | s_gof[nthreads] | s_chi2[nthreads]
    extern __shared__ float smem[];
    float* s_dedp = smem;
    float* s_rho  = s_dedp + nl;
    float* s_gof  = s_rho + (GPU_MAX_BOXES + 2);
    float* s_chi2 = s_gof + nthreads;

    __shared__ GpuParams  s_backup;
    __shared__ GpuSAState s_state;
    __shared__ float  s_norm;
    __shared__ int    s_xr_fail;
    __shared__ int    s_cancel;

    if (tid == 0) {
        s_state  = chain_states[chain_id];
        s_cancel = 0;
    }
    __syncthreads();

    GpuParams* params = &chain_params[chain_id];
    const float k0 = edp.k0;
    const float dz = edp.dz;
    const float* sin_arr   = meas.use_qspread ? meas.qspread_sintheta  : meas.sintheta;
    const float* sinsq_arr = meas.use_qspread ? meas.qspread_sin2theta : meas.sinsquaredtheta;

    curandState local_rng;
    if (tid == 0) local_rng = rng_states[chain_id];

    for (int iter = 0; iter < num_iterations; iter++) {

        // ── Cancellation check (every 1024 iterations) ──────────────
        if ((iter & 0x3FF) == 0) {
            if (tid == 0) s_cancel = *cancel_flag;
            __syncthreads();
            if (s_cancel) break;
        }

        // ── Phase 0: backup + mutate (thread 0) ─────────────────────
        if (tid == 0) {
            s_backup = *params;
            device_mutate(params, &local_rng, &s_state);
        }
        __syncthreads();

        // ── Phase 1: generate EDP → shared memory ───────────────────
        float rough = params->roughness;
        if (rough < 1e-6f) rough = 1e-6f;
        float inv_rough = 1.0f / (rough * sqrtf(2.0f));
        float supersld  = params->sld_values[0] * edp.rho;
        int   refllayers = params->real_params_size - 1;

        for (int k = tid; k < refllayers; k += nthreads)
            s_rho[k] = edp.rho * (params->sld_values[k + 1] - params->sld_values[k]) * 0.5f;
        __syncthreads();

        for (int idx = tid; idx < nl; idx += nthreads) {
            float val = supersld;
            for (int k = 0; k < refllayers; k++) {
                float dist = (edp.ed_spacing[idx] - edp.dist_array[k]) * inv_rough;
                if (dist > 6.0f)       val += s_rho[k] * 2.0f;
                else if (dist > -6.0f) val += s_rho[k] * (1.0f + erff(dist));
            }
            s_dedp[idx] = 2.0f * val;
        }
        __syncthreads();

        // ── XR density check ─────────────────────────────────────────
        if (meas.xr_only) {
            if (tid == 0) {
                s_xr_fail = 0;
                for (int i = 0; i < nl; i++) {
                    if (s_dedp[i] < 0.0f) { s_xr_fail = 1; break; }
                }
                if (s_xr_fail) *params = s_backup;
            }
            __syncthreads();
            if (s_xr_fail) continue;
        }

        // ── Phase 2: normalization factor (force_norm) ───────────────
        if (meas.force_norm) {
            if (tid == 0) {
                float r0;
                if (meas.use_qspread) {
                    r0 = 0.0f;
                    for (int j = 0; j < 13; j++)
                        r0 += kQSpreadWeights[j] *
                              device_parratt_single(s_dedp, nl,
                                  sin_arr[j], sinsq_arr[j], k0, dz);
                    r0 /= GPU_QSPREAD_NORM;
                } else {
                    r0 = device_parratt_single(
                        s_dedp, nl, sin_arr[0], sinsq_arr[0], k0, dz);
                }
                s_norm = (r0 > 1e-30f) ? (1.0f / r0) : 1.0f;
            }
            __syncthreads();
        }

        float norm = meas.force_norm ? s_norm : 1.0f;
        if (meas.imp_norm) norm *= params->imp_norm;

        // ── Phase 3: Parratt + objective (fused, parallel over Q) ────
        float p_gof  = 0.0f;
        float p_chi2 = 0.0f;
        int   obj    = meas.objective_function;

        for (int qi = tid; qi < nd; qi += nthreads) {
            float ri;
            if (meas.use_qspread) {
                ri = 0.0f;
                for (int j = 0; j < 13; j++) {
                    int qidx = qi * 13 + j;
                    ri += kQSpreadWeights[j] *
                          device_parratt_single(s_dedp, nl,
                              sin_arr[qidx], sinsq_arr[qidx], k0, dz);
                }
                ri /= GPU_QSPREAD_NORM;
            } else {
                ri = device_parratt_single(
                    s_dedp, nl, sin_arr[qi], sinsq_arr[qi], k0, dz);
            }
            ri *= norm;

            float yi  = meas.refl_values[qi];
            float eyi = meas.refl_errors[qi];

            if (obj == 0) {
                float d = logf(yi) - logf(ri);
                p_gof += d * d;
            } else if (obj == 1) {
                float ratio = yi / ri;
                if (ratio < 1.0f) ratio = 1.0f / ratio;
                p_gof += (1.0f - ratio) * (1.0f - ratio);
            } else if (obj == 2) {
                float d = logf(yi) - logf(ri);
                p_gof += d * d / fabsf(logf(eyi));
            } else {
                float ratio = yi / ri;
                if (ratio < 1.0f) ratio = 1.0f / ratio;
                float em = (yi / eyi) * (yi / eyi);
                p_gof += (1.0f - ratio) * (1.0f - ratio) * em;
            }

            float diff = yi - ri;
            p_chi2 += (diff * diff) / (eyi * eyi);
        }

        // ── Parallel reduction ───────────────────────────────────────
        s_gof[tid]  = p_gof;
        s_chi2[tid] = p_chi2;
        __syncthreads();
        for (int s = nthreads / 2; s > 0; s >>= 1) {
            if (tid < s) {
                s_gof[tid]  += s_gof[tid + s];
                s_chi2[tid] += s_chi2[tid + s];
            }
            __syncthreads();
        }

        // ── Phase 4: accept / reject (thread 0) ─────────────────────
        if (tid == 0) {
            float new_energy = s_gof[0] / (float)(nd + 1);

            bool accepted = false;
            if (s_state.algorithm == 0)
                accepted = device_evaluate_greedy(&s_state, new_energy);
            else if (s_state.algorithm == 1)
                accepted = device_evaluate_sa(&s_state, new_energy, &local_rng);
            else
                accepted = device_evaluate_stun(&s_state, new_energy, &local_rng);

            if (accepted)
                s_state.current_energy = new_energy;
            else
                *params = s_backup;
        }
        __syncthreads();
    }

    // ── Write back final state ───────────────────────────────────────
    if (tid == 0) {
        chain_states[chain_id] = s_state;
        rng_states[chain_id]   = local_rng;
    }
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
    bool m_finished = false;

    // Cancel flag: managed memory visible to both host and device
    int* m_cancel_flag = nullptr;

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
    if (m_cancel_flag) cudaFree(m_cancel_flag);

    d_chain_states = nullptr;
    d_chain_params = nullptr;
    h_result = nullptr;
    m_cancel_flag = nullptr;
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
    m_finished = false;

    // Managed-memory cancel flag (visible to both host and device)
    CUDA_CHECK(cudaMallocManaged(&m_cancel_flag, sizeof(int)));
    *m_cancel_flag = 0;

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
    int nthreads = 256;

    // Dynamic shared memory: s_dedp[nl] + s_rho[66] + s_gof[256] + s_chi2[256]
    size_t smem_bytes = (nl + (GPU_MAX_BOXES + 2) + nthreads + nthreads) * sizeof(float);

    // One block per chain — all iterations run inside the kernel
    kernel_sa_persistent<<<nc, nthreads, smem_bytes>>>(
        d_chain_states, d_chain_params,
        d_edp_struct, d_meas_struct,
        d_rng_states, m_cancel_flag,
        iterations);

    // Find global best across all chains
    kernel_find_best<<<1, 1>>>(d_chain_states, d_chain_params, d_result, nc);

    // Copy result to pinned host memory
    CUDA_CHECK(cudaMemcpy(h_result, d_result, sizeof(GpuResultSummary), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaDeviceSynchronize());
}

void CudaSARunner::cancel() {
    if (m_cancel_flag) *m_cancel_flag = 1;
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

extern "C" EXPORT GpuSARunner* stochfit_cuda_create_runner() {
    return new CudaSARunner();
}

#endif // STOCHFIT_HAS_CUDA
