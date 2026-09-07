import koffi from 'koffi';
import fs from 'fs';
import os from 'os';
import path from 'path';
import { app } from 'electron';

let _stochLib: ReturnType<typeof koffi.load> | null = null;
let _levmarLib: ReturnType<typeof koffi.load> | null = null;

function getLibExt(): string {
  switch (process.platform) {
    case 'win32':  return '.dll';
    case 'darwin': return '.dylib';
    default:       return '.so';
  }
}

function getLibPrefix(): string {
  return process.platform === 'win32' ? '' : 'lib';
}

function devLibPath(projectRoot: string, fileName: string): string {
  const override = process.env.STOCHFIT_BUILD_TYPE;
  if (override) {
    return path.join(projectRoot, 'build', override, 'bin', fileName);
  }
  const debugPath   = path.join(projectRoot, 'build', 'Debug',   'bin', fileName);
  const releasePath = path.join(projectRoot, 'build', 'Release', 'bin', fileName);
  let debugMtime = 0, releaseMtime = 0;
  try { debugMtime   = fs.statSync(debugPath).mtimeMs;   } catch { /* not built */ }
  try { releaseMtime = fs.statSync(releasePath).mtimeMs; } catch { /* not built */ }
  return debugMtime > releaseMtime ? debugPath : releasePath;
}

function libPath(name: string): string {
  const fileName = `${getLibPrefix()}${name}${getLibExt()}`;
  if (app.isPackaged) {
    return path.join(process.resourcesPath, fileName);
  }
  return devLibPath(path.join(__dirname, '../../../../'), fileName);
}

// libgomp reads OMP_WAIT_POLICY/GOMP_SPINCOUNT in its own loader constructor,
// which — because libgomp.so is a NEEDED dependency of libstochfit.so — always
// runs before any constructor defined inside libstochfit.so itself. That makes
// the equivalent setenv() in platform.h a no-op: by the time it runs, libgomp
// already cached the passive wait policy. Setting the vars here, before the
// koffi.load() that dlopen()s libstochfit.so (and pulls in libgomp.so as a
// dependency at that point), lands them before libgomp's constructor fires.
function ensureOmpWaitPolicy(): void {
  if (process.platform === 'win32') return;
  if (!process.env.OMP_WAIT_POLICY) process.env.OMP_WAIT_POLICY = 'active';
  if (!process.env.GOMP_SPINCOUNT) process.env.GOMP_SPINCOUNT = '30000000';
}

// Windows builds with clang-cl against LLVM's libomp (not MSVC's vcomp), whose
// default parks threads at every barrier — worth ~2x on this workload. The
// authoritative fix is the kmp_set_blocktime(1) call in SetOMPBlockTime()
// (src/stochfitdll/StochFitHarness.cpp), which carries the full measurements and
// covers every consumer including the standalone tools.
//
// This is belt-and-braces for the Electron path, and it is not purely redundant:
// it lands before *any* native code runs, so it also covers levmardll's own
// OpenMP regions, which never go through StochFit::Processing() and so would
// otherwise never reach that call. Same value, so the two cannot disagree.
function ensureWindowsOmpBlockTime(): void {
  if (process.platform !== 'win32') return;
  if (!process.env.KMP_BLOCKTIME) process.env.KMP_BLOCKTIME = '1';
}

// Same ordering constraint as ensureOmpWaitPolicy() above, for a much bigger
// effect: on WSL2, running the SA loop with the default thread count (one
// OpenMP thread per *logical* CPU) measured ~1080-1600 iter/s. Restricting to
// one thread per *physical* core (avoiding SMT/hyperthread-sibling
// oversubscription) via OMP_NUM_THREADS + OMP_PLACES=cores + OMP_PROC_BIND=close
// measured ~7500-8000 iter/s on the same machine — a ~5x difference that
// nothing in the C++ side can reproduce on its own. See the long comment on
// PinOMPThreadsToPCores() in StochFitHarness.cpp for the full story of why —
// short version: libgomp.so is a dependency of libstochfit.so, so its own
// constructor (which reads these env vars once, from the real process
// environment at exec() time) always runs before anything defined inside
// libstochfit.so, including any static initializer placed there. This is the
// one place in the whole process that reliably runs *before* koffi's
// dlopen()/LoadLibrary() triggers that constructor for the first time, so
// it's the only place these variables can still be set with any effect.
//
// Linux-only: on Windows the equivalent P-core pinning is done directly via
// GetLogicalProcessorInformationEx + SetThreadAffinityMask in C++ (no such
// dlopen-ordering problem there — vcomp isn't a separate loaded dependency in
// the same way). Deliberately not applied on macOS: this machine's numbers
// don't say anything about Apple Silicon's P/E-core scheduling, and macOS's
// sysfs-free topology means this Linux-specific detection wouldn't find
// anything there anyway.
function ensureLinuxOmpThreadTuning(): void {
  if (process.platform !== 'linux') return;
  if (process.env.OMP_NUM_THREADS || process.env.OMP_PLACES || process.env.OMP_PROC_BIND) return;

  const nprocs = os.cpus().length;
  if (nprocs <= 0) return;

  const claimed = new Array<boolean>(nprocs).fill(false);
  let physicalCores = 0;
  let anySmt = false;

  for (let cpu = 0; cpu < nprocs; cpu++) {
    if (claimed[cpu]) continue;
    physicalCores++;
    claimed[cpu] = true;

    let siblings: string;
    try {
      siblings = fs.readFileSync(`/sys/devices/system/cpu/cpu${cpu}/topology/thread_siblings_list`, 'utf8').trim();
    } catch {
      continue; // topology query unavailable — leave this cpu as its own group
    }

    // Sibling list is a comma list and/or dash ranges, e.g. "0-1" or "0,2,4-5".
    for (const tok of siblings.split(',')) {
      const range = tok.split('-').map(Number);
      const [lo, hi] = range.length === 2 ? range : [range[0], range[0]];
      for (let s = lo; s <= hi; s++) {
        if (s >= 0 && s < nprocs && !claimed[s]) {
          claimed[s] = true;
          anySmt = true;
        }
      }
    }
  }

  if (!anySmt || physicalCores <= 0) return; // nothing to avoid

  process.env.OMP_PLACES = 'cores';
  process.env.OMP_PROC_BIND = 'close';
  process.env.OMP_NUM_THREADS = String(physicalCores);
}

export function getStochLib() {
  if (!_stochLib) {
    ensureOmpWaitPolicy();
    ensureLinuxOmpThreadTuning();
    ensureWindowsOmpBlockTime();
    _stochLib = koffi.load(libPath('stochfit'));
  }
  return _stochLib;
}

export function getLevmarLib() {
  if (!_levmarLib) {
    // In case this is loaded before getStochLib() ever runs — same reasoning,
    // levmardll links LAPACK/OpenBLAS which can also be OpenMP-threaded.
    ensureLinuxOmpThreadTuning();
    ensureWindowsOmpBlockTime();
    _levmarLib = koffi.load(libPath('levmardll'));
  }
  return _levmarLib;
}

type KoffiFn = ReturnType<ReturnType<typeof koffi.load>['func']>;

// ── stochfit_shared bindings ──────────────────────────────────────────────────
// All data travels as FlatBuffers byte buffers (uint8_t*).
// Input-only:  void fn(uint8_t *buf, int len)
// Output-only: int  fn(uint8_t *out, int maxLen)  → bytes written

let _fnsStoch: Record<string, KoffiFn> | null = null;

export function getStochFns() {
  if (!_fnsStoch) {
    const lib = getStochLib();
    _fnsStoch = {
      Init:         lib.func('void Init(uint8_t *buf, int len)'),
      GetInitError: lib.func('str GetInitError()'),
      Start:        lib.func('void Start(int iterations)'),
      Stop:         lib.func('void Stop()'),
      Destroy:      lib.func('void Destroy()'),
      Cancel:       lib.func('void Cancel()'),
      GetData:      lib.func('int GetData(uint8_t *outBuf, int maxLen)'),
      GetRunState:  lib.func('int GetRunState(uint8_t *outBuf, int maxLen)'),
      SAParams:     lib.func('int SAParams(uint8_t *outBuf, int maxLen)'),
    };
  }
  return _fnsStoch;
}

// ── levmardll bindings ────────────────────────────────────────────────────────
// Each function: int fn(uint8_t *inBuf, int inLen, uint8_t *outBuf, int maxLen)

let _fnsLevmar: Record<string, KoffiFn> | null = null;

export function getLevmarFns() {
  if (!_fnsLevmar) {
    const lib = getLevmarLib();
    _fnsLevmar = {
      FastReflfit:      lib.func('int FastReflfit(uint8_t *inBuf, int inLen, uint8_t *outBuf, int maxLen)'),
      FastReflGenerate: lib.func('int FastReflGenerate(uint8_t *inBuf, int inLen, uint8_t *outBuf, int maxLen)'),
      Rhofit:           lib.func('int Rhofit(uint8_t *inBuf, int inLen, uint8_t *outBuf, int maxLen)'),
      RhoGenerate:      lib.func('int RhoGenerate(uint8_t *inBuf, int inLen, uint8_t *outBuf, int maxLen)'),
      StochFitBoxModel: lib.func('int StochFitBoxModel(uint8_t *inBuf, int inLen, uint8_t *outBuf, int maxLen)'),
    };
  }
  return _fnsLevmar;
}
