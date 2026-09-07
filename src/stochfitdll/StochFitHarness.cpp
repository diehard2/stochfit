/*
 *	Copyright (C) 2008 Stephen Danauskas
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "StochFitHarness.h"

#include <omp.h>

#include "ParamVector.h"
#include "platform.h"

#ifdef _WIN32
    #define NOMINMAX
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

    #include <cstdio>
    #include <mutex>
    #include <vector>

static DWORD_PTR GetPCoreMask()
{
    DWORD bufLen = 0;
    GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &bufLen);
    std::vector<uint8_t> buf(bufLen);
    auto* info = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(buf.data());
    // 0 here (same sentinel as the "not hybrid" case below) means the caller
    // must leave thread count/affinity at their defaults. The query-failure
    // path used to return an all-ones mask, which is the exact bug fixed
    // below for the "not hybrid" case (see that comment) — popcount() would
    // read 64 P-cores and omp_set_num_threads(64) would write past the end
    // of per-thread scratch buffers sized for the real thread count.
    if (!GetLogicalProcessorInformationEx(RelationProcessorCore, info, &bufLen)) {
        return 0;
    }

    uint8_t maxClass = 0;
    for (auto* p = info; reinterpret_cast<uint8_t*>(p) < buf.data() + bufLen;
         p = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(reinterpret_cast<uint8_t*>(p) + p->Size)) {
        maxClass = std::max(maxClass, p->Processor.EfficiencyClass);
    }

    DWORD_PTR mask = 0;
    bool hybrid = false;
    for (auto* p = info; reinterpret_cast<uint8_t*>(p) < buf.data() + bufLen;
         p = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(reinterpret_cast<uint8_t*>(p) + p->Size)) {
        if (p->Processor.EfficiencyClass < maxClass) {
            hybrid = true;
        } else {
            // Both SMT siblings per P-core are pinned deliberately: measured
            // (stochfit_profile, 2M iterations) at ~8869 iter/s with both HT
            // threads per P-core vs. ~5242 iter/s restricted to one logical
            // processor per physical core (isolating the lowest GroupMask
            // bit) — a ~40% regression, not the win the WSL2/Linux SMT
            // oversubscription finding would suggest. That Linux result
            // doesn't transfer here: vcomp already pins every thread to an
            // explicit affinity mask (see PinOMPThreadsToPCores below), so
            // there's no scheduler bouncing/false-oversubscription for HT to
            // cause — the two threads per physical core instead get to hide
            // memory/cache-miss latency in this workload's Parratt recursion,
            // which is a net win on this hardware. Do not "fix" this again
            // without re-measuring.
            mask |= p->Processor.GroupMask[0].Mask;
        }
    }
    // 0 means "not hybrid" (or the topology query failed) — the caller must
    // leave thread count/affinity at their defaults in that case. Returning
    // an all-ones sentinel here previously made popcount() read as 64 P-cores
    // on any non-hybrid machine (i.e. any VM/most non-Intel-12th-gen+ CPUs),
    // forcing omp_set_num_threads(64) regardless of real core count. The
    // per-thread scratch buffers in UnifiedReflectivity are sized off
    // omp_get_max_threads() *before* this runs (at Init(), not Start()), so
    // the inflated thread count on Start() indexed past the end of those
    // buffers — an out-of-bounds write, not merely oversubscription.
    return hybrid ? mask : 0;
}

// Windows builds with clang-cl against LLVM's libomp, whose default wait policy
// parks threads at every barrier. This loop crosses ~5 barriers per iteration at
// ~10k iterations/s, so that costs roughly half of throughput (measured 4,441
// iter/s at only ~5.5 of 16 cores busy — the threads are asleep, not working).
//
// The opposite extreme is just as bad: spinning forever (KMP_BLOCKTIME=infinite)
// measured 6,014 iter/s, because we run one thread per *logical* CPU, so a thread
// spinning at a barrier starves the worker sharing its physical core of issue
// slots. A short spin followed by a yield is worth ~2x over either extreme:
//
//     blocktime   0ms (sleep at once)   4,769 iter/s
//     blocktime   1..50ms              ~12,200 iter/s   <-- broad optimum
//     blocktime   infinite (spin hot)   6,014 iter/s
//
// Set via the kmp_set_blocktime() API rather than the KMP_BLOCKTIME environment
// variable on purpose: the env var has to be in place before libomp's loader
// initialization, which nothing inside a library that gets dlopen()'d/
// LoadLibrary()'d can guarantee (the same ordering trap documented at length on
// the Linux path below). This call runs before the first parallel region, which
// is what libomp actually reads it at, and so it covers every consumer —
// Electron, stochfit_tests, mirefl, stochfit_profile — with no environment setup.
//
// Guarded to Windows: macOS also uses libomp, but Apple Silicon has no SMT, so
// the sibling-starvation effect that makes a short blocktime win here does not
// apply and the value is unmeasured there. Do not extend without measuring.
static void SetOMPBlockTime()
{
    #if defined(__clang__)
    kmp_set_blocktime(1);  // milliseconds
    #endif
}

static void PinOMPThreadsToPCores()
{
    static std::once_flag pinned;
    std::call_once(pinned, [] {
        SetOMPBlockTime();

        // Per-thread scratch buffers in UnifiedReflectivity were already sized
        // off omp_get_max_threads() at Init() time, before this ever runs (see
        // GetPCoreMask()'s comment for the out-of-bounds write this causes when
        // a later omp_set_num_threads() call raises the count). Capture that
        // ceiling here and never raise past it: if OMP_NUM_THREADS (or any
        // other constraint) already capped the thread count below the real
        // P-core count, respect the smaller value instead of inflating past
        // what was allocated for.
        const int initThreads = omp_get_max_threads();
        const DWORD_PTR mask = GetPCoreMask();
        if (mask == 0) {
            return;  // non-hybrid CPU (or query failed) — defaults are already correct
        }
        const int pCoreCount = std::min(static_cast<int>(__popcnt64(mask)), initThreads);
        // Resize the team to its final size *before* pinning: pinning the old
        // (larger, one-thread-per-logical-CPU) team and resizing afterward
        // left the actual hot-loop team — whichever OS threads vcomp/libomp
        // picks for the new, smaller num_threads(pCoreCount) region — only
        // partially covered by the affinity call, so some threads kept
        // running on E-cores outside the intended mask. Resizing first means
        // this #pragma omp parallel already runs on the same team the hot
        // loop will reuse.
        omp_set_num_threads(pCoreCount);
    #pragma omp parallel
        {
            SetThreadAffinityMask(GetCurrentThread(), mask);
        }
    });
}
#elif defined(__linux__)
    #include <sched.h>
    #include <unistd.h>

    #include <algorithm>
    #include <cstdio>
    #include <cstring>
    #include <mutex>
    #include <vector>

static cpu_set_t GetPCoreMask()
{
    cpu_set_t empty;
    CPU_ZERO(&empty);

    const long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    if (nprocs <= 0)
        return empty;

    // Discriminate P-cores from E-cores by max scaling frequency, read from
    // /sys/devices/system/cpu/cpuN/cpufreq/cpuinfo_max_freq: on Alder Lake and
    // later, P-cores report a materially higher max frequency than E-cores on
    // the same die. This node is populated by the intel_pstate/acpi-cpufreq
    // driver on bare-metal Linux. It does not exist under WSL2 — Hyper-V does
    // not pass per-core topology through to the guest at all (every vCPU
    // reports identical cache/topology info, so there is no signal to detect
    // hybrid cores from inside the VM). When the sysfs reads fail we fall
    // through to "not hybrid," leaving thread count/affinity at defaults —
    // the same query-failure fallback as the Windows path above.
    std::vector<long> maxFreq(static_cast<size_t>(nprocs), -1);
    long highest = -1;
    for (long cpu = 0; cpu < nprocs; ++cpu) {
        char path[128];
        std::snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%ld/cpufreq/cpuinfo_max_freq", cpu);
        FILE* f = std::fopen(path, "r");
        if (!f)
            continue;
        long freq = -1;
        const int scanned = std::fscanf(f, "%ld", &freq);
        std::fclose(f);
        if (scanned != 1)
            continue;
        maxFreq[static_cast<size_t>(cpu)] = freq;
        highest = std::max(highest, freq);
    }
    if (highest <= 0)
        return empty;  // no cpufreq signal (WSL2, or driver not loaded) — not hybrid

    bool hybrid = false;
    cpu_set_t mask;
    CPU_ZERO(&mask);
    for (long cpu = 0; cpu < nprocs; ++cpu) {
        const long freq = maxFreq[static_cast<size_t>(cpu)];
        if (freq <= 0)
            continue;
        if (freq < highest)
            hybrid = true;
        else
            CPU_SET(static_cast<int>(cpu), &mask);
    }
    return hybrid ? mask : empty;
}

// Fallback physical-core count for when GetPCoreMask() finds no hybrid signal
// (e.g. under WSL2 — see the big comment on PinOMPThreadsToPCores() below for
// why this is only a partial fix). Counts unique SMT sibling groups via
// /sys/.../topology/thread_siblings_list, which — unlike /sys/.../cpufreq —
// WSL2 does report correctly. Returns 0 if the query fails or there is no SMT
// to avoid (nothing for the caller to do).
static int DetectPhysicalCoreCountIfSMT()
{
    const long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    if (nprocs <= 0)
        return 0;

    std::vector<bool> claimed(static_cast<size_t>(nprocs), false);
    int physicalCores = 0;
    bool anySMT = false;

    for (long cpu = 0; cpu < nprocs; ++cpu) {
        if (claimed[static_cast<size_t>(cpu)])
            continue;
        ++physicalCores;

        char path[128];
        std::snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%ld/topology/thread_siblings_list", cpu);
        FILE* f = std::fopen(path, "r");
        if (!f) {
            claimed[static_cast<size_t>(cpu)] = true;
            continue;
        }
        char buf[256] = {};
        const bool readOk = std::fgets(buf, sizeof(buf), f) != nullptr;
        std::fclose(f);
        claimed[static_cast<size_t>(cpu)] = true;
        if (!readOk)
            continue;

        // Sibling list is a comma list and/or dash ranges, e.g. "0-1" or "0,2,4-5".
        char* tok = std::strtok(buf, ",\n");
        while (tok) {
            int lo, hi;
            if (std::sscanf(tok, "%d-%d", &lo, &hi) == 2) {
                for (int s = lo; s <= hi; ++s) {
                    if (s >= 0 && s < nprocs && !claimed[static_cast<size_t>(s)]) {
                        claimed[static_cast<size_t>(s)] = true;
                        anySMT = true;
                    }
                }
            } else if (std::sscanf(tok, "%d", &lo) == 1) {
                if (lo >= 0 && lo < nprocs && !claimed[static_cast<size_t>(lo)]) {
                    claimed[static_cast<size_t>(lo)] = true;
                    anySMT = true;
                }
            }
            tok = std::strtok(nullptr, ",\n");
        }
    }

    return anySMT ? physicalCores : 0;
}

// ── A note on why this can only do a partial job on WSL2 ─────────────────────
//
// GetPCoreMask() needs a live /sys/.../cpufreq signal to find the P/E-core
// split, and that does not exist under WSL2 at all — Hyper-V never exposes
// per-core topology to the guest, hybrid or not. When that lookup fails, the
// fallback below at least avoids plain SMT oversubscription (using
// DetectPhysicalCoreCountIfSMT(), which — unlike cpufreq — WSL2 does report
// correctly), via a runtime omp_set_num_threads() call here in Processing().
// Measured effect on this codebase's SA loop under WSL2: ~1580 -> ~2600 iter/s.
//
// That is real, but it is well short of what turns out to be possible. The
// full fix is setting OMP_NUM_THREADS/OMP_PLACES=cores/OMP_PROC_BIND=close as
// actual process environment variables *before this process starts* — e.g.
// `OMP_NUM_THREADS=8 OMP_PLACES=cores OMP_PROC_BIND=close ./stochfit_profile`
// — which measured ~7500-8000 iter/s on the same machine, a further ~3x on
// top of the runtime fallback. Several things were tried to reproduce that
// from inside this library and none worked:
//   - omp_set_num_threads() called as literally the first line of main(),
//     before any other OpenMP construct in the process: no better than
//     calling it here in Processing().
//   - Manual sched_setaffinity(), pinning one thread to one specific physical
//     core (verified correct via `ps -L` — clean 1:1 mapping, no doubled-up
//     cores): still only reached the ~2600 mark, not ~7500+.
//   - A global (pre-main) C++ static initializer calling setenv() for
//     OMP_PLACES/OMP_PROC_BIND/OMP_NUM_THREADS: no effect at all.
// The last result explains the others: libgomp.so is a dependency of this
// library, and ELF dynamic-linking order runs a dependency's constructors
// before the depending module's — so libgomp's own constructor (which reads
// these env vars once, from the process's real environment at exec() time)
// always runs before *any* code we control, including our own global
// initializers. By the time our code — however early — gets to run, whatever
// libgomp does internally in response to these variables (thread-pool
// pre-sizing, NUMA-aware allocation, etc.) is already locked in from
// whatever was actually in the environment at process start.
//
// Net: there is no way to get the full benefit from C++ code living inside a
// library that something else loads at runtime. The environment variables
// have to be set by whatever launches the process, before it starts. For the
// Electron GUI (which loads this via koffi, i.e. dlopen()/LoadLibrary() well
// after the host process is already running), that means setting
// process.env.OMP_NUM_THREADS et al. in the Node/Electron main process
// *before* the first require()/koffi load of the native module — see
// gui/src/main/native/ffi.ts (ensureLinuxOmpThreadTuning). Setting process.env
// there still works because it's setenv() under the hood, called before
// dlopen() triggers libgomp's constructor for the first time in that
// process's lifetime — the same ordering constraint, just satisfied instead
// of violated.
static void PinOMPThreadsToPCores()
{
    static std::once_flag pinned;
    std::call_once(pinned, [] {
        // See the identical note in the Windows PinOMPThreadsToPCores() above:
        // per-thread scratch buffers were sized off omp_get_max_threads() at
        // Init() time, so never raise past that ceiling here even if the real
        // P-core/physical-core count is larger.
        const int initThreads = omp_get_max_threads();
        cpu_set_t mask = GetPCoreMask();
        int pCoreCount = CPU_COUNT(&mask);
        if (pCoreCount > 0) {
            // Resize before pinning, not after: measured on Windows (vcomp)
            // that pinning the old, larger one-thread-per-logical-CPU team
            // and resizing afterward left the actual hot-loop team only
            // partially pinned, with straggler threads landing on E-cores
            // and stalling every barrier — fixing the ordering was a clean
            // +12% (9138 -> 10271 iter/s). Same shape of bug here.
            omp_set_num_threads(std::min(pCoreCount, initThreads));
    #pragma omp parallel
            {
                sched_setaffinity(0, sizeof(cpu_set_t), &mask);
            }
            return;
        }

        // No hybrid signal — partial SMT-oversubscription fallback; see the
        // comment above for why this is the best achievable from here.
        const int physicalCores = DetectPhysicalCoreCountIfSMT();
        if (physicalCores > 0)
            omp_set_num_threads(std::min(physicalCores, initThreads));
    });
}
#endif

// ── Constructor
// ───────────────────────────────────────────────────────────────

// Trim the measurement arrays to the fit window [CritEdgeOffset, N-HighQOffset)
// and zero the offsets. Everything downstream — including the Parratt Q grid
// built by ReflConstants — must see the same window as the sliced data, or the
// model buffer and reflBuf sizes disagree. Invalid inputs are returned
// unchanged; the constructor body reports them via m_initError.
static ReflSettings SliceToFitWindow(ReflSettings s)
{
    const int qsize = static_cast<int>(s.Q.size());
    const int n = qsize - s.CritEdgeOffset - s.HighQOffset;
    if (s.CritEdgeOffset < 0 || s.HighQOffset < 0 || n <= 0 || static_cast<int>(s.Refl.size()) < qsize ||
        static_cast<int>(s.ReflError.size()) < qsize) {
        return s;
    }

    const int off = s.CritEdgeOffset;
    auto slice = [&](std::vector<double>& v) {
        if (static_cast<int>(v.size()) >= off + n) {
            v.assign(v.begin() + off, v.begin() + off + n);
        }
    };
    slice(s.Q);
    slice(s.Refl);
    slice(s.ReflError);
    slice(s.QError);  // may be empty (no smearing); left alone in that case
    s.CritEdgeOffset = 0;
    s.HighQOffset = 0;
    return s;
}

StochFit::StochFit(const ReflSettings& InitStruct, const std::unique_ptr<StochRunState>& state)
    // m_initStruct first; m_cEDP/m_displayEDP before m_parratt (declaration order).
    : m_initStruct(SliceToFitWindow(InitStruct)),
      m_cEDP(m_initStruct),
      m_displayEDP(m_initStruct),
      params(InitStruct),
      m_displayState(InitStruct),
      m_parratt(m_initStruct, m_cEDP.GetLayerCount()),
      m_objective(ReflectivityObjective::Type{InitStruct.Objectivefunction}),
      m_stepper({.sigmaSearch = InitStruct.Sigmasearch,
                 .absSearch = InitStruct.AbsorptionSearchPerc,
                 .normSearch = InitStruct.NormalizationSearchPerc,
                 .stepSize = InitStruct.Paramtemp})
{
    m_stop_requested = false;

    m_Directory = InitStruct.Directory;

    // m_initStruct was sliced to the fit window by SliceToFitWindow; validate
    // against the caller's original settings so invalid inputs are rejected.
    const int qsize = static_cast<int>(InitStruct.Q.size());
    m_datapoints = qsize - InitStruct.HighQOffset - InitStruct.CritEdgeOffset;
    if (InitStruct.CritEdgeOffset < 0 || InitStruct.HighQOffset < 0 || m_datapoints <= 0 ||
        static_cast<int>(InitStruct.Refl.size()) < qsize || static_cast<int>(InitStruct.ReflError.size()) < qsize) {
        m_initError = tl::unexpected(std::string("Invalid Q/Refl/ReflError sizes"));
        return;
    }
    m_xi = m_initStruct.Q;
    m_yi = m_initStruct.Refl;
    m_eyi = m_initStruct.ReflError;

    // Apply run state if provided.
    // filmAbsInput is the pre-multiplication value: Set_FilmAbs(x) stores x*WC.
    // temperature is raw m_dTemp (β) stored directly via SetTemperature.
    // surfAbs is saved independently so it is never baked into filmAbsInput.
    if (state && static_cast<int>(state->edValues.size()) == params.RealParamsSize()) {
        params.SetRoughness(state->roughness);
        params.SetSupphase(state->edValues[0]);
        for (int i = 1; i < static_cast<int>(state->edValues.size()) - 1; i++) {
            params.SetMutatableParameter(i - 1, state->edValues[i]);
        }
        params.SetSubphase(state->edValues[state->edValues.size() - 1]);
        m_cEDP.Set_FilmAbs(state->filmAbsInput);
        params.SetSurfAbs(state->surfAbs);
        params.SetImpNorm(state->impNorm);
    }
    params.UpdateBoundaries();
    m_displayState.params = params;

    // Build SA scratch buffer and deps.
    const int nd = m_datapoints;
    m_saReflBuf.resize(nd);
    AnnealDeps deps;
    deps.yi = m_yi;
    deps.eyi = m_eyi;
    deps.reflBuf = m_saReflBuf;
    deps.impNorm = InitStruct.Impnorm;

    // Construct the algorithm-specific annealer variant.
    const auto algo = AlgorithmFromInt(InitStruct.Algorithm);
    switch (algo) {
        case SaAlgorithm::Greedy:
            m_annealer.emplace(std::in_place_type<Anneal<GreedyPolicy>>, m_cEDP, m_parratt, m_objective, m_stepper, params, deps);
            break;
        case SaAlgorithm::Simulated:
            m_annealer.emplace(std::in_place_type<Anneal<SimulatedPolicy>>, m_cEDP, m_parratt, m_objective, m_stepper, params, deps,
                               InitStruct.Inittemp, InitStruct.Slope, InitStruct.Platiter);
            break;
        case SaAlgorithm::Stun:
            m_annealer.emplace(std::in_place_type<Anneal<StunPolicy>>, m_cEDP, m_parratt, m_objective, m_stepper, params, deps,
                               InitStruct.Inittemp, InitStruct.Slope, InitStruct.Platiter, InitStruct.Gamma, InitStruct.STUNfunc,
                               InitStruct.Tempiter, InitStruct.Adaptive);
            break;
    }

    // Apply session temperature/avgfSTUN after the annealer is constructed.
    if (state != nullptr && static_cast<int>(state->edValues.size()) == params.RealParamsSize()) {
        std::visit(
            [&](auto& a) {
                a.SetTemperature(state->temperature);
                a.SetAverageFSTUN(state->avgfSTUN);
            },
            *m_annealer);
    }

    // Compute the initial energy to seed best/current state.
    std::visit([&](auto& a) { a.InitEnergy(params); }, *m_annealer);

    m_initError = {};
}

StochFit::~StochFit()
{
    if (m_thread.joinable()) {
        m_stop_requested = true;
        m_thread.join();
    }
}

// ── Harness accessors
// ─────────────────────────────────────────────────────────

double StochFit::GetTemperature() const
{
    if (!m_annealer) {
        return 0.0;
    }
    return std::visit([](const auto& a) { return a.GetTemperature(); }, *m_annealer);
}

double StochFit::GetRawTemperature() const
{
    if (!m_annealer) {
        return 0.0;
    }
    return std::visit([](const auto& a) { return a.GetRawTemperature(); }, *m_annealer);
}

void StochFit::SetTemperature(double t)
{
    if (m_annealer) {
        std::visit([t](auto& a) { a.SetTemperature(t); }, *m_annealer);
    }
}

double StochFit::GetLowestEnergy() const
{
    if (!m_annealer) {
        return 0.0;
    }
    return std::visit([](const auto& a) { return a.GetLowestEnergy(); }, *m_annealer);
}

double StochFit::GetAverageFSTUN() const
{
    if (!m_annealer) {
        return 0.0;
    }
    return std::visit([](const auto& a) { return a.GetAverageFSTUN(); }, *m_annealer);
}

void StochFit::SetAverageFSTUN(double f)
{
    if (m_annealer) {
        std::visit([f](auto& a) { a.SetAverageFSTUN(f); }, *m_annealer);
    }
}

// ── Main CPU loop
// ─────────────────────────────────────────────────────────────

int StochFit::Processing()
{
    try {
        if (!m_annealer) {
            return -1;
        }

#if defined(_WIN32) || defined(__linux__)
        PinOMPThreadsToPCores();
#endif

        const int nThreads = omp_get_max_threads();

#pragma omp parallel num_threads(nThreads)
        {
            for (int isteps = 0; isteps < m_itotaliterations && !m_stop_requested.load(); ++isteps) {

                // All threads: mutate candidate (omp single inside) + build EDP (omp for inside).
                std::visit([&](auto& a) { a.PrepareCandidate(params); }, *m_annealer);

                // All threads: cooperative Parratt Q-point distribution (omp for inside,
                // no trailing single/barrier — every thread gets the result span back
                // as an ordinary return value in its own stack frame).
                auto lastResult = std::visit([&](auto& a) { return a.ComputeSharedRefl(); }, *m_annealer);

                // Serial: publish result + accept/reject + display snapshot, all in one
                // single/barrier (publishing used to be its own separate single).
#pragma omp single
                {
                    bool accepted = false;
                    double gof = 0.0, chi = 0.0;

                    std::visit([&](auto& a) { a.PublishResult(lastResult); }, *m_annealer);
                    accepted = std::visit([&](auto& a) { return a.EvaluateAndAccept(params); }, *m_annealer);
                    if (accepted || isteps == 0) {
                        gof = std::visit([](const auto& a) { return a.GetCurrentEnergy(); }, *m_annealer);
                        chi = std::visit([](const auto& a) { return a.GetLastChiSquare(); }, *m_annealer);
                    }

                    if (accepted || isteps == 0) {
                        std::lock_guard lock(m_displayMutex);
                        m_displayState.params = params;
                        m_displayState.refl.assign(m_saReflBuf.begin(), m_saReflBuf.end());
                        m_displayState.chiSquare = chi;
                        m_displayState.goF = gof;
                    }
                    m_icurrentiteration.store(isteps + 1, std::memory_order_relaxed);
                }
                // implicit barrier: all threads sync before the next iteration
            }
        }

        m_icurrentiteration.store(m_itotaliterations, std::memory_order_relaxed);
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "[StochFit] Processing() caught exception: " << ex.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "[StochFit] Processing() caught unknown exception" << std::endl;
        return -1;
    }
}

// ── Display snapshot (main thread only)
// ──────────────────────────────────────

DataSnapshot StochFit::GetCurrentState()
{
    // Copy the minimum inside the lock, then compute outside.
    DisplayState snap(m_initStruct);
    {
        std::lock_guard lock(m_displayMutex);
        snap = m_displayState;
    }

    m_displayEDP.GenerateEDP(snap.params);

    DataSnapshot out;
    out.roughness = snap.params.GetRoughness();
    out.chiSquare = snap.chiSquare;
    out.goodnessOfFit = snap.goF;

    // snap.refl is the SA scoring buffer: one value per measured Q point.
    const int nd = m_datapoints;
    out.Q.assign(m_xi.begin(), m_xi.begin() + nd);
    out.refl = std::move(snap.refl);
    out.refl.resize(nd);

    const int nl = m_displayEDP.Get_EDPPointCount();
    out.z.resize(nl);
    out.rho.resize(nl);
    const double dz = m_displayEDP.Get_Dz();
    const double leftOffset = m_displayEDP.Get_LeftOffset();
    const double rhoSub = m_displayEDP.m_EDP[nl - 1].real();
    for (int i = 0; i < nl; i++) {
        out.z[i] = (i * dz) - leftOffset;
        out.rho[i] = m_displayEDP.m_EDP[i].real() / rhoSub;
    }

    return out;
}

// ── Public API
// ────────────────────────────────────────────────────────────────

int StochFit::Start(int iterations)
{
    m_itotaliterations = iterations;
    m_stop_requested = false;
    m_thread = std::thread([this] { Processing(); });
    return 0;
}

int StochFit::Cancel()
{
    if (m_thread.joinable()) {
        m_stop_requested = true;
    }
    return 0;
}

void StochFit::Stop()
{
    if (m_thread.joinable()) {
        m_stop_requested = true;
        m_thread.join();
    }
}

StochRunState StochFit::GetRunState()
{
    // Called after Stop() — thread is joined, no lock needed.
    StochRunState s;
    s.roughness = params.GetRoughness();
    s.filmAbsInput = m_cEDP.Get_FilmAbsInput();
    s.surfAbs = params.GetSurfAbs();
    s.temperature = GetRawTemperature();
    s.impNorm = params.GetImpNorm();
    s.avgfSTUN = GetAverageFSTUN();
    s.bestSolution = GetLowestEnergy();
    s.chiSquare = m_displayState.chiSquare;
    s.goodnessOfFit = m_displayState.goF;
    auto span = params.RealParams();
    s.edValues.assign(span.begin(), span.end());
    return s;
}

DataSnapshot StochFit::GetData()
{
    const int iter = m_icurrentiteration.load(std::memory_order_relaxed);
    DataSnapshot snap = GetCurrentState();
    snap.iteration = iter;
    snap.isFinished = (iter >= m_itotaliterations);
    return snap;
}
