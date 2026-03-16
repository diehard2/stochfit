cmake_minimum_required(VERSION 3.21)

# ── macOS deployment target (required for std::jthread) ──────────────────────
# Must be set before project() to correctly initialize compiler flags
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "macOS Deployment Target")
endif()

# vcpkg must be set up before project()
include(env/cmake/Vcpkg.cmake)

project(StochFit VERSION 1.0.0 LANGUAGES C CXX)

# ── Global settings ────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output all binaries to build/bin/Debug or build/bin/Release
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


# Warnings
if(MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

# ── Release optimizations ──────────────────────────────────────────────────────
# Target: CPUs from ~2020+ (AVX2+FMA on x86, Apple Silicon on ARM)
if(MSVC)
    # /Oi: expand intrinsics; /fp:fast: aggressive FP reordering; /GL: whole-program opt
    # /arch:AVX2: AVX2 + FMA (Haswell 2013+, covers all modern chips)
    add_compile_options(
        $<$<CONFIG:Release>:/Oi>
        $<$<CONFIG:Release>:/fp:fast>
        $<$<CONFIG:Release>:/GL>
        $<$<CONFIG:Release>:/arch:AVX2>
    )
    add_link_options($<$<CONFIG:Release>:/LTCG>)
else()
    # -O3: full auto-vectorization; -ffast-math: aggressive FP; -flto: link-time opt
    add_compile_options(
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-ffast-math>
        $<$<CONFIG:Release>:-flto>
    )
    add_link_options($<$<CONFIG:Release>:-flto>)
    # Architecture tuning — x86-64-v3 = AVX2+FMA (Haswell/Zen2+, ~2019+)
    if(APPLE AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        add_compile_options($<$<CONFIG:Release>:-mcpu=apple-m1>)
    else()
        add_compile_options($<$<CONFIG:Release>:-march=x86-64-v3>)
    endif()
endif()

# ── Dependencies ───────────────────────────────────────────────────────────────
# CMake 4.x FindLAPACK changed cheev_ detection; use clapack CONFIG directly on Windows
if(WIN32 OR (DEFINED VCPKG_TARGET_TRIPLET))
    find_package(clapack CONFIG REQUIRED)
    # clapack exports: lapack (STATIC IMPORTED) and f2c (STATIC IMPORTED)
    # BLAS comes from openblas; use imported targets directly
    set(LAPACK_LIBRARIES lapack f2c)
else()
    find_package(LAPACK REQUIRED)
endif()

if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Homebrew libomp path
    set(OpenMP_ROOT "/opt/homebrew/opt/libomp")
endif()

find_package(OpenMP)
find_package(GTest REQUIRED)
find_package(tl-expected CONFIG REQUIRED)
enable_testing()

# ── Optional GPU support ──────────────────────────────────────────────────────
option(STOCHFIT_USE_CUDA "Enable CUDA GPU acceleration" ON)
option(STOCHFIT_USE_METAL "Enable Metal GPU acceleration" ON)

if(STOCHFIT_USE_CUDA AND NOT APPLE)
    # Check for nvcc first — this result is cached, so subsequent configures are fast
    include(CheckLanguage)
    check_language(CUDA)

    if(CMAKE_CUDA_COMPILER)
        # CUDA 13.1 maxes out at C++20, so set it before enabling CUDA language
        set(CMAKE_CUDA_STANDARD 20)
        set(CMAKE_CUDA_STANDARD_REQUIRED ON)

        if(MSVC)
            set(CMAKE_CUDA_FLAGS "-allow-unsupported-compiler" CACHE STRING "CUDA flags")
        endif()

        find_package(CUDAToolkit REQUIRED)
        enable_language(CUDA)
        set(STOCHFIT_HAS_CUDA TRUE)
        set(CMAKE_CUDA_RUNTIME_LIBRARY "Static")
        message(STATUS "CUDA GPU acceleration enabled (CUDA ${CUDAToolkit_VERSION})")
    else()
        message(STATUS "CUDA compiler not found — GPU acceleration disabled")
    endif()
endif()

if(STOCHFIT_USE_METAL AND APPLE)
    find_library(METAL_FRAMEWORK Metal)
    find_library(FOUNDATION_FRAMEWORK Foundation)
    if(METAL_FRAMEWORK)
        set(STOCHFIT_HAS_METAL TRUE)
        message(STATUS "Metal GPU acceleration enabled")
    else()
        message(STATUS "Metal not found — GPU acceleration disabled")
    endif()
endif()

# ── stochfit_core (static lib — StochFitdll sources) ──────────────────────────
add_library(stochfit_core STATIC
    src/stochfitdll/CEDP.cpp
    src/stochfitdll/ParamVector.cpp
    src/stochfitdll/ReflCalc.cpp
    src/stochfitdll/SA_Dispatcher.cpp
    src/stochfitdll/SimulatedAnnealing.cpp
    src/stochfitdll/StochFitDll.cpp
    src/stochfitdll/StochFitHarness.cpp
)
target_include_directories(stochfit_core
    PUBLIC include
    PRIVATE include/stochfit/stochfitdll src/stochfitdll)
set_target_properties(stochfit_core PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)
if(OpenMP_CXX_FOUND)
    target_link_libraries(stochfit_core PUBLIC OpenMP::OpenMP_CXX)
    
    # On macOS, we need to redistribute libomp.dylib
    if(APPLE)
        get_target_property(OMP_LIBRARY OpenMP::OpenMP_CXX IMPORTED_LOCATION_RELEASE)
        if(NOT OMP_LIBRARY)
             get_target_property(OMP_LIBRARY OpenMP::OpenMP_CXX IMPORTED_LOCATION_DEBUG)
        endif()
        if(NOT OMP_LIBRARY)
             get_target_property(OMP_LIBRARY OpenMP::OpenMP_CXX IMPORTED_LOCATION)
        endif()

        if(OMP_LIBRARY)
            add_custom_command(TARGET stochfit_core POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${OMP_LIBRARY}"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libomp.dylib"
                COMMENT "Copying libomp.dylib to output directory"
            )
        endif()
    endif()
endif()
target_link_libraries(stochfit_core PUBLIC tl::expected)

# GPU sources and libraries
set(GPU_SOURCES
    src/stochfitdll/gpu/gpu_detect.cpp
    src/stochfitdll/gpu/gpu_sa_runner.cpp
)
if(STOCHFIT_HAS_CUDA)
    list(APPEND GPU_SOURCES src/stochfitdll/gpu/stochfit_cuda.cu)
endif()
if(STOCHFIT_HAS_METAL)
    list(APPEND GPU_SOURCES src/stochfitdll/gpu/stochfit_metal.mm)
endif()

if(STOCHFIT_HAS_CUDA OR STOCHFIT_HAS_METAL)
    target_sources(stochfit_core PRIVATE ${GPU_SOURCES})
endif()

if(STOCHFIT_HAS_CUDA)
    target_compile_definitions(stochfit_core PUBLIC STOCHFIT_HAS_CUDA)
    target_link_libraries(stochfit_core PUBLIC CUDA::cudart)
    set_source_files_properties(src/stochfitdll/gpu/stochfit_cuda.cu
        PROPERTIES CUDA_ARCHITECTURES "75;86;89;100")
endif()
if(STOCHFIT_HAS_METAL)
    target_compile_definitions(stochfit_core PUBLIC STOCHFIT_HAS_METAL)
    target_link_libraries(stochfit_core PUBLIC ${METAL_FRAMEWORK} ${FOUNDATION_FRAMEWORK})
endif()

# ── levmar (static lib — pure C Lourakis library) ─────────────────────────────
# Axb_core.c, lm_core.c, etc. are #include'd by the non-_core files — do NOT
# add them as compilation units.
add_library(levmar STATIC
    src/levmar/Axb.c
    src/levmar/lm.c
    src/levmar/lmbc.c
    src/levmar/lmblec.c
    src/levmar/lmlec.c
    src/levmar/misc.c
)
target_compile_definitions(levmar PRIVATE HAVE_LAPACK)
target_include_directories(levmar
    PUBLIC include
    PRIVATE include/stochfit/levmar)
target_link_libraries(levmar PRIVATE ${LAPACK_LIBRARIES})

# ── levmardll (static lib) ────────────────────────────────────────────────────
add_library(levmardll STATIC
    src/levmardll/FastReflCalc.cpp
    src/levmardll/LMRoutines.cpp
    src/levmardll/LevMardll.cpp
    src/levmardll/ReflCalc.cpp
    src/levmardll/RhoCalc.cpp
)
target_include_directories(levmardll
    PUBLIC include
    PRIVATE include/stochfit/levmardll include/stochfit/levmar)
set_target_properties(levmardll PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    ARCHIVE_OUTPUT_NAME levmardll_core)
target_link_libraries(levmardll PUBLIC stochfit_core levmar)

# ── mirefl (console executable) ───────────────────────────────────────────────
add_executable(mirefl
    src/mirefl/MIRefl.cpp
)
# CEDP and CReflCalc come from stochfit_core (which transitively brings OpenMP).
# src/mirefl is on the include path for MDParamVec.h / targetver.h.
# include/stochfit/stochfitdll resolves the bare "CEDP.h" / "ReflCalc.h" includes.
target_include_directories(mirefl PRIVATE src/mirefl include include/stochfit/stochfitdll)
target_link_libraries(mirefl PRIVATE stochfit_core)

# ── Shared library shims ───────────────────────────────────────────────────────
add_library(stochfit_shared SHARED src/shim/stochfit_shim.cpp)
set_target_properties(stochfit_shared PROPERTIES
    OUTPUT_NAME stochfit
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)
target_include_directories(stochfit_shared PRIVATE include)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(stochfit_shared PRIVATE
        -Wl,--whole-archive $<TARGET_FILE:stochfit_core> -Wl,--no-whole-archive)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_options(stochfit_shared PRIVATE
        -Wl,-force_load,$<TARGET_FILE:stochfit_core>)
elseif(MSVC)
    target_link_options(stochfit_shared PRIVATE
        /WHOLEARCHIVE:$<TARGET_FILE:stochfit_core>)
endif()
target_link_libraries(stochfit_shared PRIVATE stochfit_core)

add_executable(stochfit_tests
    tests/test_reflectivity.cpp)
target_include_directories(stochfit_tests PRIVATE include include/stochfit/stochfitdll)
target_link_libraries(stochfit_tests PRIVATE stochfit_core GTest::gtest GTest::gtest_main)
target_compile_features(stochfit_tests PRIVATE cxx_std_20)
add_test(NAME stochfit_tests COMMAND stochfit_tests)

add_library(levmardll_shared SHARED src/shim/levmardll_shim.cpp)
set_target_properties(levmardll_shared PROPERTIES
    OUTPUT_NAME levmardll
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)
target_include_directories(levmardll_shared PRIVATE include)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(levmardll_shared PRIVATE
        -Wl,--whole-archive $<TARGET_FILE:levmardll> -Wl,--no-whole-archive)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_options(levmardll_shared PRIVATE
        -Wl,-force_load,$<TARGET_FILE:levmardll>)
elseif(MSVC)
    target_link_options(levmardll_shared PRIVATE
        /WHOLEARCHIVE:$<TARGET_FILE:levmardll>)
endif()
target_link_libraries(levmardll_shared PRIVATE levmardll)

# ── compile_commands.json ─────────────────────────────────────────────────────
# Copy to build/ so clangd always finds it regardless of active config (Debug/Release)
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    add_custom_target(copy_compile_commands ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_BINARY_DIR}/../compile_commands.json"
        COMMENT "Copying compile_commands.json to build/"
    )
endif()

# ── Electron GUI ──────────────────────────────────────────────────────────────
# cmake --build build            → C++ only (fast, default)
# cmake --build build --target gui → npm install + forge make → build/electron/
find_program(NPM npm.cmd npm
    HINTS
        "C:/Program Files/nodejs"
        "C:/Program Files (x86)/nodejs"
        "/usr/local/bin"
        "/opt/homebrew/bin"
    NO_CACHE)
if(NPM)
    # Tracked install step: only re-runs when package.json / package-lock.json change
    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/gui/node_modules/.package-lock.json
        COMMAND ${NPM} install
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/gui
        DEPENDS
            ${CMAKE_SOURCE_DIR}/gui/package.json
            ${CMAKE_SOURCE_DIR}/gui/package-lock.json
        COMMENT "Installing npm dependencies"
        VERBATIM
    )

    add_custom_target(gui
        COMMAND ${NPM} run make
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/gui
        DEPENDS
            ${CMAKE_SOURCE_DIR}/gui/node_modules/.package-lock.json
            stochfit_shared
            levmardll_shared
        COMMENT "Packaging Electron GUI → build/electron/"
        VERBATIM
    )
else()
    message(STATUS "npm not found — 'gui' target unavailable")
endif()
