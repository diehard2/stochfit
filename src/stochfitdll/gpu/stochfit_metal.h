#pragma once

#include "gpu_sa_runner.h"
#include "gpu_detect.h"
#include <memory>

#if defined(STOCHFIT_HAS_METAL)
std::unique_ptr<GpuSARunner> create_metal_runner();
GpuInfo detect_metal_gpu();
#endif
