# FindCUDAOptional.cmake
# Optional CUDA detection — does not fail if CUDA is unavailable.
#
# Sets:
#   CUDA_OPTIONAL_FOUND   - TRUE if a usable CUDA toolkit was found
#   CUDA_OPTIONAL_VERSION - CUDA toolkit version (e.g. "12.4")
#
# This module is used by CMakeLists.txt via check_language(CUDA) instead,
# but is provided as a convenience for downstream projects.

include(CheckLanguage)
check_language(CUDA)

if(CMAKE_CUDA_COMPILER)
    set(CUDA_OPTIONAL_FOUND TRUE)
    enable_language(CUDA)
    find_package(CUDAToolkit QUIET)
    if(CUDAToolkit_FOUND)
        set(CUDA_OPTIONAL_VERSION "${CUDAToolkit_VERSION}")
    endif()
else()
    set(CUDA_OPTIONAL_FOUND FALSE)
endif()
