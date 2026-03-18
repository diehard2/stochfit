#pragma once

// Abstract GPU SA runner interface.
// GpuSARunner::create(backend) returns a CUDA or Metal runner depending on
// the detected backend. initialize() uploads all problem data to the GPU.
// run_batch(N) runs N SA iterations synchronously across all chains.
// get_result() / get_best_edp() / get_best_reflectivity() retrieve results.

#include "gpu_detect.h"
#include "gpu_types.h"
#include <memory>
#include <vector>

class GpuSARunner {
public:
    virtual ~GpuSARunner() = default;

    static std::unique_ptr<GpuSARunner> create(GpuBackend backend);

    // Initialize with problem data. All arrays are copied to GPU.
    virtual void initialize(
        const GpuSAState& sa_state,
        const GpuParams& params,
        const GpuMeasurement& measurement,
        const GpuEDPConfig& edp_config,
        int num_chains
    ) = 0;

    // Run a batch of SA iterations on the GPU (synchronous - blocks until done)
    virtual void run_batch(int iterations) = 0;

    // Cancel current batch (called from different thread)
    virtual void cancel() = 0;

    // Read results (no GPU sync needed - reads from pinned/shared memory)
    virtual GpuResultSummary get_result() const = 0;

    // Full data retrieval (triggers GPU->CPU copy)
    virtual void get_best_edp(float* edp_out, int count) const = 0;
    virtual void get_best_reflectivity(float* refl_out, int count) const = 0;

    virtual int num_chains() const = 0;
    virtual bool is_finished() const = 0;
};
