cmake_minimum_required(VERSION 3.21)

# vcpkg must be set up before project()
include(cmake/Vcpkg.cmake)

project(StochFit VERSION 1.0.0 LANGUAGES C CXX)

# ── Global settings ────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Warnings
if(MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

# ── Dependencies ───────────────────────────────────────────────────────────────
find_package(LAPACK REQUIRED)
find_package(OpenMP)

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
target_include_directories(stochfit_core PUBLIC include)
if(OpenMP_CXX_FOUND)
    target_link_libraries(stochfit_core PUBLIC OpenMP::OpenMP_CXX)
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
target_include_directories(levmar PUBLIC include)
target_link_libraries(levmar PRIVATE ${LAPACK_LIBRARIES})

# ── levmardll (static lib) ────────────────────────────────────────────────────
add_library(levmardll STATIC
    src/levmardll/FastReflCalc.cpp
    src/levmardll/LMRoutines.cpp
    src/levmardll/LevMardll.cpp
    src/levmardll/ReflCalc.cpp
    src/levmardll/RhoCalc.cpp
    src/levmardll/mersenne.cpp
)
target_include_directories(levmardll PUBLIC include)
target_link_libraries(levmardll PUBLIC stochfit_core levmar)

# ── mirefl (console executable) ───────────────────────────────────────────────
add_executable(mirefl
    src/mirefl/CEDP.cpp
    src/mirefl/MIRefl.cpp
    src/mirefl/ReflCalctest.cpp
)
# src/mirefl/ has private headers (MDParamVec.h, targetver.h)
target_include_directories(mirefl PRIVATE src/mirefl include)
target_link_libraries(mirefl PRIVATE stochfit_core)
