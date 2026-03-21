import koffi from 'koffi';
import fs from 'fs';
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

export function getStochLib() {
  if (!_stochLib) _stochLib = koffi.load(libPath('stochfit'));
  return _stochLib;
}

export function getLevmarLib() {
  if (!_levmarLib) _levmarLib = koffi.load(libPath('levmardll'));
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
      ArraySizes:   lib.func('int ArraySizes(uint8_t *outBuf, int maxLen)'),
      SAParams:     lib.func('int SAParams(uint8_t *outBuf, int maxLen)'),
      WarmedUp:     lib.func('bool WarmedUp()'),
      GpuAvailable: lib.func('bool GpuAvailable()'),
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
