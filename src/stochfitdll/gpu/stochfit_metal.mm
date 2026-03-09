#include "stochfit_metal.h"

#if defined(STOCHFIT_HAS_METAL)

#import <Metal/Metal.h>
#import <Foundation/Foundation.h>
#include "gpu_types.h"
#include <cstring>
#include <algorithm>
#include <atomic>
#include <vector>

// Matching struct layouts for Metal (same as gpu_types.h but ensure Metal alignment)
// The Metal shader uses its own mirror structs defined in the .metal file.

class MetalSARunner : public GpuSARunner {
public:
    ~MetalSARunner() override { cleanup(); }

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

    id<MTLDevice> m_device = nil;
    id<MTLCommandQueue> m_queue = nil;
    id<MTLLibrary> m_library = nil;

    // Compute pipelines
    id<MTLComputePipelineState> m_edp_pipeline = nil;
    id<MTLComputePipelineState> m_parratt_pipeline = nil;
    id<MTLComputePipelineState> m_qsmear_pipeline = nil;
    id<MTLComputePipelineState> m_objective_pipeline = nil;

    // Buffers (storageModeShared for CPU readback)
    id<MTLBuffer> m_chain_states = nil;
    id<MTLBuffer> m_chain_params = nil;
    id<MTLBuffer> m_chain_params_backup = nil;
    id<MTLBuffer> m_chain_edp = nil;
    id<MTLBuffer> m_chain_dedp = nil;
    id<MTLBuffer> m_chain_rho_arr = nil;
    id<MTLBuffer> m_chain_refl = nil;
    id<MTLBuffer> m_chain_qspread_refl = nil;
    id<MTLBuffer> m_chain_gof = nil;
    id<MTLBuffer> m_chain_chi2 = nil;
    id<MTLBuffer> m_edp_config_buf = nil;
    id<MTLBuffer> m_meas_config_buf = nil;
    id<MTLBuffer> m_ed_spacing = nil;
    id<MTLBuffer> m_dist_array = nil;
    id<MTLBuffer> m_sintheta = nil;
    id<MTLBuffer> m_sinsq = nil;
    id<MTLBuffer> m_qspread_sin = nil;
    id<MTLBuffer> m_qspread_sin2 = nil;
    id<MTLBuffer> m_yi = nil;
    id<MTLBuffer> m_eyi = nil;
    id<MTLBuffer> m_num_chains_buf = nil;
    id<MTLBuffer> m_num_q_buf = nil;
    id<MTLBuffer> m_num_dp_buf = nil;
    id<MTLBuffer> m_num_layers_buf = nil;

    int m_num_chains_val = 0;
    int m_num_layers = 0;
    int m_num_datapoints = 0;
    int m_num_q_per_call = 0;
    bool m_use_qspread = false;
    std::atomic<bool> m_cancelled{false};
    bool m_finished = false;

    GpuResultSummary m_result{};
};

void MetalSARunner::cleanup() {
    // ARC handles Metal object release
    m_device = nil;
    m_queue = nil;
    m_library = nil;
}

void MetalSARunner::initialize(
    const GpuSAState& sa_state,
    const GpuParams& params,
    const GpuMeasurement& measurement,
    const GpuEDPConfig& edp_config,
    int num_chains)
{
    cleanup();

    m_num_chains_val = num_chains;
    m_num_layers = edp_config.num_layers;
    m_num_datapoints = measurement.num_datapoints;
    m_use_qspread = measurement.use_qspread != 0;
    m_num_q_per_call = m_use_qspread ? m_num_datapoints * 13 : m_num_datapoints;
    m_cancelled = false;
    m_finished = false;

    m_device = MTLCreateSystemDefaultDevice();
    if (!m_device) return;

    m_queue = [m_device newCommandQueue];

    // Load Metal library from default library
    NSError* error = nil;
    m_library = [m_device newDefaultLibrary];
    if (!m_library) {
        // Try loading from compiled metallib
        NSString* libPath = [[NSBundle mainBundle] pathForResource:@"stochfit_sa" ofType:@"metallib"];
        if (libPath) {
            NSURL* url = [NSURL fileURLWithPath:libPath];
            m_library = [m_device newLibraryWithURL:url error:&error];
        }
    }
    if (!m_library) return;

    // Create compute pipelines
    auto makePipeline = [&](const char* name) -> id<MTLComputePipelineState> {
        id<MTLFunction> fn = [m_library newFunctionWithName:[NSString stringWithUTF8String:name]];
        if (!fn) return nil;
        return [m_device newComputePipelineStateWithFunction:fn error:&error];
    };

    m_edp_pipeline = makePipeline("kernel_generate_edp");
    m_parratt_pipeline = makePipeline("kernel_parratt");
    m_qsmear_pipeline = makePipeline("kernel_qsmear");
    m_objective_pipeline = makePipeline("kernel_objective");

    // Allocate buffers (storageModeShared for CPU readback)
    int nc = num_chains;
    int nl = m_num_layers;
    int nd = m_num_datapoints;
    int nq = m_num_q_per_call;

    // Per-chain state
    std::vector<GpuSAState> h_states(nc, sa_state);
    std::vector<GpuParams> h_params(nc, params);

    m_chain_states = [m_device newBufferWithBytes:h_states.data()
        length:nc * sizeof(GpuSAState) options:MTLResourceStorageModeShared];
    m_chain_params = [m_device newBufferWithBytes:h_params.data()
        length:nc * sizeof(GpuParams) options:MTLResourceStorageModeShared];
    m_chain_params_backup = [m_device newBufferWithLength:nc * sizeof(GpuParams)
        options:MTLResourceStorageModeShared];

    m_chain_edp = [m_device newBufferWithLength:nc * nl * sizeof(float)
        options:MTLResourceStorageModeShared];
    m_chain_dedp = [m_device newBufferWithLength:nc * nl * sizeof(float)
        options:MTLResourceStorageModeShared];
    m_chain_rho_arr = [m_device newBufferWithLength:nc * (GPU_MAX_BOXES + 2) * sizeof(float)
        options:MTLResourceStorageModeShared];
    m_chain_refl = [m_device newBufferWithLength:nc * nd * sizeof(float)
        options:MTLResourceStorageModeShared];
    if (m_use_qspread) {
        m_chain_qspread_refl = [m_device newBufferWithLength:nc * nq * sizeof(float)
            options:MTLResourceStorageModeShared];
    }
    m_chain_gof = [m_device newBufferWithLength:nc * sizeof(float)
        options:MTLResourceStorageModeShared];
    m_chain_chi2 = [m_device newBufferWithLength:nc * sizeof(float)
        options:MTLResourceStorageModeShared];

    // Measurement data
    m_sintheta = [m_device newBufferWithBytes:measurement.sintheta
        length:nd * sizeof(float) options:MTLResourceStorageModeShared];
    m_sinsq = [m_device newBufferWithBytes:measurement.sinsquaredtheta
        length:nd * sizeof(float) options:MTLResourceStorageModeShared];
    m_yi = [m_device newBufferWithBytes:measurement.refl_values
        length:nd * sizeof(float) options:MTLResourceStorageModeShared];
    m_eyi = [m_device newBufferWithBytes:measurement.refl_errors
        length:nd * sizeof(float) options:MTLResourceStorageModeShared];

    if (m_use_qspread) {
        m_qspread_sin = [m_device newBufferWithBytes:measurement.qspread_sintheta
            length:nd * 13 * sizeof(float) options:MTLResourceStorageModeShared];
        m_qspread_sin2 = [m_device newBufferWithBytes:measurement.qspread_sin2theta
            length:nd * 13 * sizeof(float) options:MTLResourceStorageModeShared];
    }

    // EDP config
    m_ed_spacing = [m_device newBufferWithBytes:edp_config.ed_spacing
        length:nl * sizeof(float) options:MTLResourceStorageModeShared];
    m_dist_array = [m_device newBufferWithBytes:edp_config.dist_array
        length:(params.num_boxes + 2) * sizeof(float) options:MTLResourceStorageModeShared];

    // Scalar uniforms as small buffers
    // Metal shader uses GpuEDPConfigM and GpuMeasConfigM without pointer fields
    struct MetalEDPConfig {
        float rho, dz, k0, beta, beta_sub, beta_sup;
        int num_layers, use_abs;
        float indexsup_real, indexsup2_real;
    };
    MetalEDPConfig medp = {
        edp_config.rho, edp_config.dz, edp_config.k0,
        edp_config.beta, edp_config.beta_sub, edp_config.beta_sup,
        edp_config.num_layers, edp_config.use_abs,
        edp_config.indexsup_real, edp_config.indexsup2_real
    };
    m_edp_config_buf = [m_device newBufferWithBytes:&medp
        length:sizeof(medp) options:MTLResourceStorageModeShared];

    struct MetalMeasConfig {
        int num_datapoints, objective_function, use_qspread;
        int force_norm, imp_norm, xr_only;
    };
    MetalMeasConfig mmeas = {
        measurement.num_datapoints, measurement.objective_function,
        measurement.use_qspread, measurement.force_norm,
        measurement.imp_norm, measurement.xr_only
    };
    m_meas_config_buf = [m_device newBufferWithBytes:&mmeas
        length:sizeof(mmeas) options:MTLResourceStorageModeShared];

    m_num_chains_buf = [m_device newBufferWithBytes:&nc
        length:sizeof(int) options:MTLResourceStorageModeShared];
    m_num_q_buf = [m_device newBufferWithBytes:&nq
        length:sizeof(int) options:MTLResourceStorageModeShared];
    m_num_dp_buf = [m_device newBufferWithBytes:&nd
        length:sizeof(int) options:MTLResourceStorageModeShared];
    m_num_layers_buf = [m_device newBufferWithBytes:&nl
        length:sizeof(int) options:MTLResourceStorageModeShared];

    memset(&m_result, 0, sizeof(m_result));
}

void MetalSARunner::run_batch(int iterations) {
    if (!m_device || !m_edp_pipeline) return;

    int nc = m_num_chains_val;
    int nl = m_num_layers;
    int nd = m_num_datapoints;
    int nq = m_num_q_per_call;

    // NOTE: SA mutation is done on the CPU side for Metal since Metal lacks
    // a built-in RNG and grid-wide sync. The mutation logic reads/writes
    // the shared-mode buffers directly.

    GpuSAState* states = (GpuSAState*)[m_chain_states contents];
    GpuParams* params_ptr = (GpuParams*)[m_chain_params contents];
    GpuParams* backup_ptr = (GpuParams*)[m_chain_params_backup contents];
    float* gof_ptr = (float*)[m_chain_gof contents];
    float* chi2_ptr = (float*)[m_chain_chi2 contents];

    // Simple host-side RNG for mutation
    std::mt19937 rng(42);
    auto uniform = [&](float lo, float hi) {
        return std::uniform_real_distribution<float>(lo, hi)(rng);
    };

    for (int iter = 0; iter < iterations && !m_cancelled.load(std::memory_order_relaxed); iter++) {
        // CPU: backup params and mutate
        for (int c = 0; c < nc; c++) {
            backup_ptr[c] = params_ptr[c];
            // Inline mutation (mirrors device_mutate from CUDA)
            GpuParams& p = params_ptr[c];
            GpuSAState& s = states[c];
            int ii = std::uniform_int_distribution<int>(0, p.num_boxes - 1)(rng);
            int perc = std::uniform_int_distribution<int>(1, 100)(rng);
            float step = s.stepsize;

            if (perc > s.sigmasearch + s.normsearch + s.abssearch) {
                float cur = p.sld_values[ii + 1];
                float newval = cur + uniform(-step, step);
                newval = std::clamp(newval, p.param_low, p.param_high);
                p.sld_values[ii + 1] = newval;
            } else if (perc <= s.sigmasearch) {
                if (!p.fix_roughness) {
                    float roughmult = 5.0f / 3.0f;
                    float lo = p.roughness * (1.0f - roughmult * step);
                    float hi = p.roughness * (1.0f + roughmult * step);
                    p.roughness = std::clamp(uniform(lo, hi), p.roughness_low, p.roughness_high);
                }
            } else if (perc <= s.sigmasearch + s.abssearch) {
                if (p.use_surf_abs) {
                    float lo = p.surf_abs * (1.0f - step);
                    float hi = p.surf_abs * (1.0f + step);
                    p.surf_abs = std::clamp(uniform(lo, hi), 0.0f, p.surfabs_high);
                }
            } else {
                if (p.fix_imp_norm) {
                    float lo = p.imp_norm * (1.0f - step);
                    float hi = p.imp_norm * (1.0f + step);
                    p.imp_norm = std::clamp(uniform(lo, hi), 0.0f, p.impnorm_high);
                }
            }
        }

        // GPU: EDP + Parratt + Objective in a single command buffer
        @autoreleasepool {
            id<MTLCommandBuffer> cmdBuf = [m_queue commandBuffer];
            id<MTLComputeCommandEncoder> enc = [cmdBuf computeCommandEncoder];

            // EDP kernel
            [enc setComputePipelineState:m_edp_pipeline];
            [enc setBuffer:m_edp_config_buf offset:0 atIndex:0];
            [enc setBuffer:m_chain_params offset:0 atIndex:1];
            [enc setBuffer:m_chain_edp offset:0 atIndex:2];
            [enc setBuffer:m_chain_dedp offset:0 atIndex:3];
            [enc setBuffer:m_chain_rho_arr offset:0 atIndex:4];
            [enc setBuffer:m_ed_spacing offset:0 atIndex:5];
            [enc setBuffer:m_dist_array offset:0 atIndex:6];
            [enc setBuffer:m_num_chains_buf offset:0 atIndex:7];

            MTLSize edpGrid = MTLSizeMake(nl, nc, 1);
            NSUInteger edpW = m_edp_pipeline.maxTotalThreadsPerThreadgroup;
            if (edpW > 256) edpW = 256;
            MTLSize edpTG = MTLSizeMake(edpW, 1, 1);
            [enc dispatchThreads:edpGrid threadsPerThreadgroup:edpTG];

            // Parratt kernel
            [enc setComputePipelineState:m_parratt_pipeline];
            [enc setBuffer:m_edp_config_buf offset:0 atIndex:0];
            [enc setBuffer:m_meas_config_buf offset:0 atIndex:1];
            [enc setBuffer:m_chain_params offset:0 atIndex:2];
            [enc setBuffer:m_chain_dedp offset:0 atIndex:3];

            id<MTLBuffer> sin_buf = m_use_qspread ? m_qspread_sin : m_sintheta;
            id<MTLBuffer> sin2_buf = m_use_qspread ? m_qspread_sin2 : m_sinsq;
            id<MTLBuffer> refl_buf = m_use_qspread ? m_chain_qspread_refl : m_chain_refl;

            [enc setBuffer:refl_buf offset:0 atIndex:4];
            [enc setBuffer:sin_buf offset:0 atIndex:5];
            [enc setBuffer:sin2_buf offset:0 atIndex:6];
            [enc setBuffer:m_num_chains_buf offset:0 atIndex:7];
            [enc setBuffer:m_num_q_buf offset:0 atIndex:8];

            MTLSize parrattGrid = MTLSizeMake(nq, nc, 1);
            NSUInteger parrattW = m_parratt_pipeline.maxTotalThreadsPerThreadgroup;
            if (parrattW > 256) parrattW = 256;
            MTLSize parrattTG = MTLSizeMake(parrattW, 1, 1);
            [enc dispatchThreads:parrattGrid threadsPerThreadgroup:parrattTG];

            // Q-smearing if needed
            if (m_use_qspread) {
                [enc setComputePipelineState:m_qsmear_pipeline];
                [enc setBuffer:m_chain_qspread_refl offset:0 atIndex:0];
                [enc setBuffer:m_chain_refl offset:0 atIndex:1];
                [enc setBuffer:m_num_dp_buf offset:0 atIndex:2];
                [enc setBuffer:m_num_chains_buf offset:0 atIndex:3];

                MTLSize qsGrid = MTLSizeMake(nd, nc, 1);
                NSUInteger qsW = m_qsmear_pipeline.maxTotalThreadsPerThreadgroup;
                if (qsW > 256) qsW = 256;
                MTLSize qsTG = MTLSizeMake(qsW, 1, 1);
                [enc dispatchThreads:qsGrid threadsPerThreadgroup:qsTG];
            }

            // Objective kernel (1 threadgroup per chain)
            [enc setComputePipelineState:m_objective_pipeline];
            [enc setBuffer:m_meas_config_buf offset:0 atIndex:0];
            [enc setBuffer:m_chain_params offset:0 atIndex:1];
            [enc setBuffer:m_chain_refl offset:0 atIndex:2];
            [enc setBuffer:m_chain_gof offset:0 atIndex:3];
            [enc setBuffer:m_chain_chi2 offset:0 atIndex:4];
            [enc setBuffer:m_yi offset:0 atIndex:5];
            [enc setBuffer:m_eyi offset:0 atIndex:6];
            [enc setBuffer:m_chain_edp offset:0 atIndex:7];
            [enc setBuffer:m_num_layers_buf offset:0 atIndex:8];
            [enc setBuffer:m_num_chains_buf offset:0 atIndex:9];

            [enc dispatchThreadgroups:MTLSizeMake(nc, 1, 1)
                threadsPerThreadgroup:MTLSizeMake(256, 1, 1)];

            [enc endEncoding];
            [cmdBuf commit];
            [cmdBuf waitUntilCompleted];
        }

        // CPU: accept/reject
        for (int c = 0; c < nc; c++) {
            float new_energy = gof_ptr[c];
            GpuSAState& s = states[c];

            if (new_energy < 0.0f) {
                params_ptr[c] = backup_ptr[c];
                continue;
            }

            bool accepted = false;
            if (s.algorithm == 0) {
                accepted = (new_energy < s.best_energy);
                if (accepted) { s.best_energy = new_energy; s.best_solution = new_energy; }
            } else if (s.algorithm == 1) {
                float cur = s.current_energy;
                if (std::min(new_energy, cur) < s.best_solution) {
                    s.best_solution = std::min(new_energy, cur);
                    accepted = true;
                } else {
                    float deltaE = new_energy - cur;
                    s.iteration++;
                    if (s.iteration % s.plat_time == 0 && s.temperature > 1e-30f)
                        s.temperature /= s.slope;
                    accepted = uniform(0.0f, 1.0f) < expf(-s.temperature * deltaE);
                }
            } else {
                // STUN
                float cur = s.current_energy;
                if (std::min(new_energy, cur) < s.best_solution) {
                    s.best_solution = std::min(new_energy, cur);
                    accepted = true;
                } else {
                    auto fstun = [&](float val) -> float {
                        float diff = val - s.best_solution;
                        if (s.stun_func == 0) return -expf(-s.gamma * diff);
                        if (s.stun_func == 1) return sinhf(s.gamma * diff) - 1.0f;
                        float gd = s.gamma * diff;
                        return logf(gd + sqrtf(gd * gd + 1.0f)) - 1.0f;
                    };
                    float deltaE = fstun(new_energy) - fstun(cur);
                    s.iteration++;
                    if (s.adaptive) {
                        if (s.iteration % s.temp_iter == 0) {
                            if (s.avg_fstun > s.best_energy)
                                s.temperature *= 0.5f / s.slope;
                            else
                                s.temperature *= s.slope;
                            if (s.iteration % s.stun_dec_iter == 0)
                                s.avg_fstun *= s.gammadec;
                        }
                    } else {
                        if (s.iteration % s.plat_time == 0 && s.temperature > 1e-30f)
                            s.temperature /= s.slope;
                    }
                    accepted = uniform(0.0f, 1.0f) < expf(-s.temperature * deltaE);
                }
            }

            if (accepted)
                s.current_energy = new_energy;
            else
                params_ptr[c] = backup_ptr[c];
        }
    }

    // Find best chain
    float best = 1e30f;
    int best_idx = 0;
    for (int c = 0; c < nc; c++) {
        if (states[c].best_solution < best) {
            best = states[c].best_solution;
            best_idx = c;
        }
    }

    m_result.best_energy = best;
    m_result.best_chain = best_idx;
    m_result.best_roughness = params_ptr[best_idx].roughness;
    m_result.best_gof = states[best_idx].best_solution;
    m_result.best_chi_square = chi2_ptr[best_idx];
    m_result.best_imp_norm = params_ptr[best_idx].imp_norm;
    m_result.best_surf_abs = params_ptr[best_idx].surf_abs;
    m_result.best_temperature = states[best_idx].temperature;
    m_result.best_avg_fstun = states[best_idx].avg_fstun;
    int rps = params_ptr[best_idx].real_params_size;
    for (int i = 0; i < rps; i++)
        m_result.best_params[i] = params_ptr[best_idx].sld_values[i];
}

void MetalSARunner::cancel() {
    m_cancelled.store(true, std::memory_order_relaxed);
}

GpuResultSummary MetalSARunner::get_result() const {
    return m_result;
}

void MetalSARunner::get_best_edp(float* edp_out, int count) const {
    if (m_result.best_chain < 0) return;
    float* edp = (float*)[m_chain_edp contents];
    int offset = m_result.best_chain * m_num_layers;
    int n = std::min(count, m_num_layers);
    memcpy(edp_out, edp + offset, n * sizeof(float));
}

void MetalSARunner::get_best_reflectivity(float* refl_out, int count) const {
    if (m_result.best_chain < 0) return;
    float* refl = (float*)[m_chain_refl contents];
    int offset = m_result.best_chain * m_num_datapoints;
    int n = std::min(count, m_num_datapoints);
    memcpy(refl_out, refl + offset, n * sizeof(float));
}

// ── Metal GPU detection ────────────────────────────────────────────────────

GpuInfo detect_metal_gpu() {
    GpuInfo info;
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (device) {
        info.backend = GpuBackend::Metal;
        info.device_name = [[device name] UTF8String];
        info.memory_bytes = [device recommendedMaxWorkingSetSize];
        info.max_chains = 64;  // default for Apple Silicon
    }
    return info;
}

std::unique_ptr<GpuSARunner> create_metal_runner() {
    return std::make_unique<MetalSARunner>();
}

#endif // STOCHFIT_HAS_METAL
