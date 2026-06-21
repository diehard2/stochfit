# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

StochFit is a cross-platform application for fitting specular X-ray and neutron reflectivity data using stochastic methods. It has a C++20 computation backend (with optional CUDA/Metal GPU acceleration) and an Electron + React + TypeScript GUI. These two halves communicate over an FFI bridge using FlatBuffers for serialization.

## Build Commands

Build uses CMake 3.21+ with named presets and vcpkg for dependencies.

```bash
# Configure (first time or after CMakeLists changes)
cmake --preset windows          # Windows/MSVC
cmake --preset default          # macOS/Linux

# Build the shared library (main C++ target)
cmake --build --preset windows --target stochfit_shared
cmake --build --preset default --target stochfit_shared

# Build everything (C++ + LevMar + tests)
cmake --build --preset windows --parallel

# Build specific targets
cmake --build --preset windows --target stochfit_dll
cmake --build --preset windows --target levmardll
cmake --build --preset windows --target stochfit_tests
cmake --build --preset windows --target mirefl

# Build and package the Electron GUI
cmake --build --preset windows --target gui

# Run tests
ctest --test-dir build/Release -V
./build/Release/bin/stochfit_tests.exe
```

The `gui/` directory has its own npm project; the CMake `gui` target handles `npm install` and Electron Forge packaging. For iterating on the frontend only, you can run `npm run start` inside `gui/`.

## Architecture

### Layered Design

```
Electron GUI (React/TypeScript)
        ↕  IPC (FlatBuffers)
Koffi FFI (Node.js ↔ stochfit.dll)
        ↕  C-style exports
stochfit_core (static, C++20)
        ↕  optional lazy-load
stochfit_cuda_plugin.dll (CUDA kernels)
```

### C++ Backend

All physics lives in `stochfit_core` (static lib), exposed via a thin C-style FFI layer in `stochfitdll`:

- **`StochFitHarness`** (`include/stochfit/StochFitHarness.h`) — Top-level orchestrator. Constructed from a `ReflSettings` FlatBuffer; manages the worker thread, selects CPU vs GPU path, and owns all algorithm state.
- **`Anneal<Policy>`** (`include/stochfit/Anneal.h`) — Template-based simulated annealing. Three policies: `Greedy`, `SimulatedAnnealing`, `STUN`.
- **`ParrattReflectivity`** (`include/stochfit/ReflCalc.h`) — Parratt recursive reflectivity calculation; OpenMP-parallel with optional Q-smearing (13-point Gaussian quadrature).
- **`CEDP`** (`include/stochfit/CEDP.h`) — Electron density profile generator; box-model layers with Gaussian interface broadening.
- **`ParamVector`** (`include/stochfit/ParamVector.h`) — Parameter bounds, mutation, and clamping.
- **`ParameterStepper`** — Adaptive mutation step sizing.
- **`ReflectivityObjective`** — Four objective functions (log-difference, inverse-ratio, and two error-weighted variants, selected by integer index 0–3).

The FFI exports (`src/stochfitdll/StochFitDll.cpp`) are: `Init`, `Start`, `Stop`, `GetData`, `GetRunState`, `Cancel`, `Destroy`. All inputs/outputs are FlatBuffer byte buffers.

**GPU acceleration** is entirely opt-in and zero-hard-dependency:
- `gpu_detect()` probes at runtime for CUDA ≥ compute 7.5 or Apple Metal.
- If a suitable GPU is found, `GpuSARunner` lazy-loads `stochfit_cuda_plugin.dll/.so/.dylib` on the first `ProcessingGPU()` call.
- If the plugin is absent or the GPU is insufficient, the CPU path is used silently.

`levmardll` is a separate shared library for Levenberg-Marquardt post-refinement. It exposes `levmarFastReflFit` and `levmarRhoFit` FFI functions and links against vcpkg's `levmar` and LAPACK/OpenBLAS.

### FlatBuffers Protocol

Schema is in `schema/stochfit.fbs`. Key tables:
- `ReflSettings` — All input parameters (Q data, SLD layer stack, algorithm choice, GPU flag, box model params).
- `GetDataResult` — Polling response: reflectivity curve, SLD profile, chi-square, goodness-of-fit.
- `StochRunState` — Full serialized state for session resume (SA temperature, parameter values, EDP).
- `LevMarRequest` / `ReflFitResult` — LevMar request/response.

FlatBuffer generated headers are produced at CMake configure time and end up in `build/generated/`.

### Electron GUI

Located entirely under `gui/`:
- `src/main/` — Main process: IPC handlers (`ipc-handlers.ts`), file parsers (`parsers/`), FFI bindings (`native/stochfit-api.ts`, `native/levmar-api.ts` via koffi).
- `src/renderer/` — React app (Vite + Tailwind + Plotly.js): data visualization, SLD calculator, session management.

Data flow: file loaded by parser → `ReflSettings` FlatBuffer constructed → sent via IPC to main process → `Init()` called on C++ dll → GUI polls `GetData()` periodically for live plot updates.

Parsers handle ORSO (`.ort`/`.orb`), NeXus/HDF5 (via `h5wasm`), and plain text column data.

### Key Configuration Files

- `CMakePresets.json` — Named build presets; macOS deployment target is 14.0 (required for `std::jthread`).
- `vcpkg.json` — vcpkg manifest; overlay ports for `levmar` are in `cmake/vcpkg-overlay-ports/`.
- `gui/forge.config.ts` — Electron Forge packaging; produces NSIS installer (Windows), DMG (macOS), AppImage (Linux).
- `include/stochfit/SettingsStruct.h` — Plain C structs mirroring FlatBuffer tables, used internally.

## Testing

Tests are GTest-based in `tests/test_reflectivity.cpp`. To run a single test:

```bash
./build/bin/stochfit_tests.exe --gtest_filter=TestSuiteName.TestName
```

`tests/MIRefl.cpp` builds the `mirefl` console utility, useful for manual reflectivity spot-checks.
