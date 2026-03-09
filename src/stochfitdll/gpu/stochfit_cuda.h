#pragma once

#include "gpu_sa_runner.h"
#include <memory>

#if defined(STOCHFIT_HAS_CUDA)
std::unique_ptr<GpuSARunner> create_cuda_runner();
#endif
