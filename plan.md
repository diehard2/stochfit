# C++ Code Review — Structural & Stylistic Recommendations

## Context

You asked whether there's anything worth changing in the C++ backend now that the
big refactor (unified Parratt, retired `CReflCalc`/`FastReflcalc`, `LayerStack`,
`std::span`-based `ParamVector` getters) has landed. This file is a
prioritized list of what I'd still touch, with citations. It is **not** a
single implementation plan — pick whichever items you want and we can do them
in follow-up turns.

The codebase is in a healthy place overall: no raw `new`/`delete`, the
`Anneal<Policy>` templated core is clean, `LayerStack` and `AnnealDeps` are
exemplary little span-based aggregates, and the unified `ParrattReflectivity`
+ `ReflectivityObjective::FillResiduals` design closes a real correctness gap
(LevMar previously ignored `FitFunc`). What's left is mostly legacy edges
that didn't get carried along with the refactor.

---

## Tier 1 — Highest leverage

### 1. Modernize `ParamVector`
`include/stochfit/ParamVector.h`, `src/stochfitdll/ParamVector.cpp`

This file is the biggest stylistic outlier in the codebase. Concrete issues:

- Hungarian prefixes (`m_b…`, `m_i…`, `m_d…`) and oddly-named `gnome` vector
  (`ParamVector.h:36–50`) — every other recent file uses plain CamelCase
  members.
- `int`-returning setters used as out-of-band error codes
  (`SetMutatableParameter`, `setroughness`, `setImpNorm`, `setSurfAbs`).
  The rest of the codebase has moved to `tl::expected` / exceptions; pick one.
- Non-const getters (`getroughness`, `getImpNorm`, `getSurfAbs`,
  `GetMutatableParameter`) prevent passing `const ParamVector&` cleanly. This
  forces `Anneal::ComputeModel` to take a non-const reference.
- `m_binitialized` is set true unconditionally in the constructor, then
  guarded everywhere — the guards are dead.
- Commented-out default constructor at `ParamVector.h:56` and
  `ParamVector.cpp:83–86`.
- `length` member shadows `std::vector::size`; rename `m_boxes`.
- Inconsistent casing: `getroughness`/`getImpNorm`/`GetMutatableParameter` —
  pick one convention.

**Why it's worth it:** ParamVector is touched by every iteration of the SA
loop and by every restore path. Its API leaks into Anneal, into the harness,
and into the FFI. Cleaning it up unblocks const-correctness elsewhere.

### 2. Carve dead GPU plumbing out of `StochFit`
`include/stochfit/StochFitHarness.h:107–116`,
`src/stochfitdll/StochFitHarness.cpp:186–193`, plus `ProcessingGPU`/
`InitGpuData`.

Nine `std::vector<float>` member buffers and the entire `ProcessingGPU` /
`InitGpuData` body are commented out / unused while the GPU path is paused.
Two options:

- **Delete** the float buffers and the disabled paths until the GPU rework
  lands. Keeps `StochFit` lean. `gpu_detect.h` / `GpuSARunner` stay.
- **Extract** them into a `GpuHarness` sibling class so the CPU object stops
  carrying GPU state.

I'd lean toward delete — it's recoverable from git, and the current
half-state makes the header a chore to read.

### 3. Fix the `StochRunState` constructor signature
`include/stochfit/StochFitHarness.h:71–72`,
`src/stochfitdll/StochFitDll.cpp` (Init exports).

```cpp
StochFit(const ReflSettings&, const std::unique_ptr<StochRunState>& = {});
```

`const std::unique_ptr<T>&` borrows ownership it doesn't take, and forces the
FFI to wrap a heap-allocated `StochRunState` it doesn't otherwise need. Use
`const StochRunState*` (null = fresh start) or `std::optional<StochRunState>`.

### 4. Drop `using namespace std;` from `platform.h`
`include/platform.h:63`

This pulls `std::*` into every translation unit that includes the header,
which is essentially everything. The cost shows up in headers like
`include/levmar/Settings.h:8` (`vector<double>`) and `CEDP.h` (bare `vector`,
`string`) — they look broken without it. Qualify everything `std::` in
headers and remove the directive. `.cpp` files can keep it locally if you
want.

### 5. FFI exception safety
`src/stochfitdll/StochFitDll.cpp` (all `extern "C"` exports),
`src/levmardll/LevMardll.cpp` (`FastReflfit`, `FastReflGenerate`,
`Rhofit`, `RhoGenerate`).

C++ exceptions crossing the FFI boundary into koffi are undefined behavior
and will crash the Electron main process. Wrap each `extern "C"` body in
`try { … } catch(const std::exception&) { … } catch(...) { … }` and either
return an error code or stash the message into a thread-local `last_error`
that the JS side can query. `Processing()` already does this — extend the
pattern to the FFI itself.

---

## Tier 2 — Worth doing, smaller scope

### 6. `goodnessOfFit = GetLowestEnergy()` looks wrong
`src/stochfitdll/StochFitHarness.cpp` (`GetRunState`)

Both `bestSolution` and `goodnessOfFit` are filled from `GetLowestEnergy()`.
Either intentional (then say so in a comment) or a copy-paste from the
old code. Worth a 30-second look.

### 7. Settings duplication
`include/stochfit/SettingsStruct.h` (`ReflSettings`),
`include/levmar/Settings.h` (`BoxReflSettings`),
`src/levmardll/SettingsBridge.cpp` (only fills 3 fields).

The bridge's near-empty body suggests `BoxReflSettings` is mostly a subset of
`ReflSettings`. Options: fold into one struct, or extract a shared
`MeasurementData { Q, Refl, ReflError, QError }` aggregate that both reuse.

### 8. Deduplicate `ReflectivityObjective::Evaluate` vs `FillResiduals`
`src/stochfitdll/ReflectivityObjective.cpp`

Two near-identical four-case switches over the same residual formulas. Pull
the per-point residual into a small inline function (or a `static constexpr`
table of function pointers keyed by `Type`) and have both methods loop over
it. Replace the magic `1e6` clamp with a named `constexpr double`.

### 9. `Cancel` semantics in the FFI
`include/stochfit/StochFitHarness.h:75`,
`src/stochfitdll/StochFitDll.cpp` (Cancel export).

`Cancel` sets the stop flag without joining; the FFI then resets the
`unique_ptr`, which joins via the destructor. Works, but only by accident.
Either rename to `StopAndDestroy` or have `Cancel` join explicitly so the
contract is visible.

### 10. `LevMardll` cleanup
`src/levmardll/LevMardll.cpp`

- `StochFitBoxModel` (lines ~309–489) is 180 lines and mixes parallel
  trial setup, levmar invocation, dedup, sort, and serialization.
  Extract `RunOneTrial`, `Dedup`, `Serialize`.
- Dedup is O(N²) over up to 6000 candidates (~36M comparisons). Sort first,
  then linear-pass dedup against the previous kept entry.
- `omp_set_num_threads(omp_get_num_procs())` (~line 354) is a process-global
  side effect from inside an FFI export. Use `#pragma omp parallel
  num_threads(...)` to scope it.
- C-style casts like `(void*)(&task)` → `static_cast<void*>(&task)`.

---

## Tier 3 — Polish

- **CEDP**: `m_supOff`/`m_subOff` declared `mutable` (`CEDP.h:35–36`) but only
  written from non-const methods — drop `mutable`. Public `m_EDP`/`m_DEDP`
  exposed as raw vectors; a `Profile() -> std::span<const std::complex<double>>`
  would localize the convention. `Init()` is a pseudo-constructor; fold into
  the real constructor.
- **AnnealPolicies**: identical `ProbCalc` in three policies could share a
  small free function. STUN's `1e-200` / `1e200` magic constants → named
  `constexpr`.
- **GPL header**: present on some files (`StochFitHarness.h`, `ParamVector.h`),
  absent on others (`Anneal.h`, `CEDP.h`, `UnifiedReflectivity.h`,
  `LayerStack.h`). Pick one policy.
- **Header bloat**: `StochFitHarness.h` transitively pulls Anneal,
  AnnealPolicies, CEDP, UnifiedReflectivity, ParameterStepper, etc. The FFI
  sees all of it. A `pImpl` or forward-declared `AnnealVariant` would shrink
  the surface, but only worth it if compile times bite.

---

## What's already clean (don't touch)

- `Anneal<Policy>` with `[[no_unique_address]]` and span-based deps.
- `LayerStack`, `AnnealDeps`, `WaveScratch<T>` — small, value-typed,
  unambiguous ownership.
- `ParrattReflectivity::ReflectivityCalcCoreImpl<HasRoughness>` and
  `CEDP::BuildEDP<Absorbing>` — templates kept private, no header leak.
- `Processing()` exception handling — the exact pattern to extend to the FFI.
- `GetData` 2-second wait-with-timeout (replaces the old spin-wait).

---

## Verification (after any of these land)

For ParamVector / Harness / FFI changes:
- `cmake --build build --config Release --target stochfit_core stochfit_dll levmardll stochfit_tests`
- `ctest --test-dir build -C Release -V`
- Run the GUI through one full SA fit + LevMar refinement on the saved
  test dataset and confirm chi-square / GoF / EDP match a pre-change run.

For Settings dedup / FFI signature changes:
- Same as above plus a session save/load round-trip — Init→Start→Stop→
  GetRunState→re-Init with the saved state, confirm iteration resumes
  cleanly.

For LevMar dedup change:
- `tests/MIRefl.cpp` for a manual reflectivity sanity check.
- Run a known multi-minimum dataset and confirm the dedup'd output set
  matches the previous output (modulo ordering).
