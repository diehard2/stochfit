#pragma once

// GPU data structures for SA fitting. All fields use float32 for
// consumer GPU performance (NVIDIA 4xxx/5xxx have 1:64 FP64:FP32 ratio).
// GpuSAState — per-chain SA algorithm state (temperature, gamma, etc.)
// GpuParams — box-model parameter vector with bounds for GPU mutation
// GpuMeasurement — Q/R/dR measurement data and objective function selection
// GpuEDPConfig — precomputed EDP geometry (spacing, distances, wave constant)
// GpuResultSummary — best-chain result after a run_batch() call

// All GPU computation uses float (32-bit) for performance.
// Consumer NVIDIA GPUs (4xxx/5xxx) have 1:64 FP64:FP32 ratio.

#include <cstdint>

#ifdef __CUDACC__
#define GPU_HOST_DEVICE __host__ __device__
#else
#define GPU_HOST_DEVICE
#endif

// Maximum number of SLD boxes supported on GPU
constexpr int GPU_MAX_BOXES = 64;

// Q-smearing uses 13 points per Q-value
constexpr int GPU_QSPREAD_POINTS = 13;

// Q-smearing weights (Gaussian quadrature)
constexpr float GPU_QSPREAD_WEIGHTS[GPU_QSPREAD_POINTS] = {
    1.000f,  // center
    0.056f, 0.056f,  // +/- 1.2 sigma
    0.135f, 0.135f,  // +/- 1.0 sigma
    0.278f, 0.278f,  // +/- 0.8 sigma
    0.487f, 0.487f,  // +/- 0.6 sigma
    0.726f, 0.726f,  // +/- 0.4 sigma
    0.923f, 0.923f,  // +/- 0.2 sigma
};
constexpr float GPU_QSPREAD_NORM = 6.211f;

struct GpuSAState {
    float temperature;
    float best_energy;
    float current_energy;
    float slope;
    float gamma;
    float avg_fstun;
    float gammadec;
    float stepsize;
    float best_solution;
    int algorithm;       // 0=greedy, 1=SA, 2=STUN
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

struct GpuParams {
    float sld_values[GPU_MAX_BOXES + 2];   // gnome: [supphase, box0..boxN-1, subphase]
    float roughness;
    float surf_abs;
    float imp_norm;
    int num_boxes;                          // number of mutable boxes (length)
    int real_params_size;                   // length + 2
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

struct GpuMeasurement {
    float* q_values;           // [num_datapoints]
    float* refl_values;        // [num_datapoints] yi
    float* refl_errors;        // [num_datapoints] eyi
    float* sintheta;           // [num_datapoints]
    float* sinsquaredtheta;    // [num_datapoints]
    float* qspread_sintheta;   // [num_datapoints*13] if smearing
    float* qspread_sin2theta;  // [num_datapoints*13] if smearing
    int num_datapoints;
    int objective_function;    // 0-3
    int use_qspread;
    int force_norm;
    int imp_norm;
    int xr_only;
};

struct GpuEDPConfig {
    float* ed_spacing;    // [num_layers] precomputed
    float* dist_array;    // [num_boxes+2] precomputed
    float rho;            // wave constant * film SLD
    float dz;             // layer thickness
    float k0;             // 2*pi/lambda
    float beta;           // film absorption * wave constant
    float beta_sub;       // substrate absorption
    float beta_sup;       // superphase absorption
    int num_layers;
    int use_abs;
    float indexsup_real;  // precomputed: 1 - DEDP[0]/2
    float indexsup2_real; // indexsup^2
};

struct GpuResultSummary {
    float best_energy;
    float best_roughness;
    float best_chi_square;
    float best_gof;
    int best_chain;
    int total_iterations;
    int finished;
    float best_params[GPU_MAX_BOXES + 2];
    float best_imp_norm;
    float best_surf_abs;
    float best_temperature;
    float best_avg_fstun;
};
