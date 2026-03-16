import koffi from 'koffi';
import fs from 'fs';
import path from 'path';
import { app } from 'electron';

// Ensure struct types are registered before binding functions
import './structs';

let _stochLib: ReturnType<typeof koffi.load> | null = null;
let _levmarLib: ReturnType<typeof koffi.load> | null = null;

function getLibExt(): string {
  switch (process.platform) {
    case 'win32':
      return '.dll';
    case 'darwin':
      return '.dylib';
    default:
      return '.so';
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

  let debugMtime   = 0;
  let releaseMtime = 0;
  try { debugMtime   = fs.statSync(debugPath).mtimeMs;   } catch { /* not built */ }
  try { releaseMtime = fs.statSync(releasePath).mtimeMs; } catch { /* not built */ }

  return debugMtime > releaseMtime ? debugPath : releasePath;
}

function libPath(name: string): string {
  const ext = getLibExt();
  const prefix = getLibPrefix();
  const fileName = `${prefix}${name}${ext}`;

  if (app.isPackaged) {
    return path.join(process.resourcesPath, fileName);
  }
  return devLibPath(path.join(__dirname, '../../../../'), fileName);
}

export function getStochLib() {
  if (!_stochLib) {
    _stochLib = koffi.load(libPath('stochfit'));
  }
  return _stochLib;
}

export function getLevmarLib() {
  if (!_levmarLib) {
    _levmarLib = koffi.load(libPath('levmardll'));
  }
  return _levmarLib;
}

type KoffiFn = ReturnType<ReturnType<typeof koffi.load>['func']>;

let _fnInit: KoffiFn | null = null;
let _fnStart: KoffiFn | null = null;
let _fnStop: KoffiFn | null = null;
let _fnDestroy: KoffiFn | null = null;
let _fnCancel: KoffiFn | null = null;
let _fnGetData: KoffiFn | null = null;
let _fnGetRunState: KoffiFn | null = null;
let _fnArraySizes: KoffiFn | null = null;
let _fnWarmedUp: KoffiFn | null = null;
let _fnSAparams: KoffiFn | null = null;
let _fnGetInitError: KoffiFn | null = null;
let _fnGpuAvailable: KoffiFn | null = null;

export function getStochFns() {
  const lib = getStochLib();
  if (!_fnInit) {
    _fnInit = lib.func('void Init(ReflSettings *, StochRunState *)');
    _fnGetInitError = lib.func('str GetInitError()');
    _fnStart = lib.func('void Start(int iterations)');
    _fnStop = lib.func('void Stop()');
    _fnDestroy = lib.func('void Destroy()');
    _fnCancel = lib.func('void Cancel()');
    _fnGetData = lib.func('int GetData(double *ZRange, double *Rho, double *QRange, double *Refl, double *roughness, double *chisquare, double *goodnessoffit, int *isfinished)');
    _fnGetRunState = lib.func('void GetRunState(double *saScalars, double *edValues, int *edCount)');
    _fnArraySizes = lib.func('void ArraySizes(int *RhoSize, int *Reflsize)');
    _fnWarmedUp = lib.func('bool WarmedUp()');
    _fnSAparams = lib.func('void SAparams(double *lowestenergy, double *temp, int *mode)');
    _fnGpuAvailable = lib.func('bool GpuAvailable()');
  }
  return {
    Init: _fnInit!,
    GetInitError: _fnGetInitError!,
    Start: _fnStart!,
    Stop: _fnStop!,
    Destroy: _fnDestroy!,
    Cancel: _fnCancel!,
    GetData: _fnGetData!,
    GetRunState: _fnGetRunState!,
    ArraySizes: _fnArraySizes!,
    WarmedUp: _fnWarmedUp!,
    SAparams: _fnSAparams!,
    GpuAvailable: _fnGpuAvailable!,
  };
}

let _fnFastReflfit: KoffiFn | null = null;
let _fnFastReflGenerate: KoffiFn | null = null;
let _fnRhofit: KoffiFn | null = null;
let _fnRhoGenerate: KoffiFn | null = null;
let _fnStochFit: KoffiFn | null = null;

export function getLevmarFns() {
  const lib = getLevmarLib();
  if (!_fnFastReflfit) {
    _fnFastReflfit = lib.func('void FastReflfit(BoxReflSettings *InitStruct, double *parameters, double *covar, int paramsize, double *info)');
    _fnFastReflGenerate = lib.func('void FastReflGenerate(BoxReflSettings *InitStruct, double *parameters, int parametersize, double *Reflectivity)');
    _fnRhofit = lib.func('void Rhofit(BoxReflSettings *InitStruct, double *parameters, double *covariance, int parametersize, double *info)');
    _fnRhoGenerate = lib.func('void RhoGenerate(BoxReflSettings *InitStruct, double *parameters, int paramsize, double *ED, double *BoxED)');
    _fnStochFit = lib.func('void StochFit(BoxReflSettings *InitStruct, double *parameters, double *covararray, int paramsize, double *info, double *ParamArray, double *chisquarearray, int *paramarraysize)');
  }
  return {
    FastReflfit: _fnFastReflfit!,
    FastReflGenerate: _fnFastReflGenerate!,
    Rhofit: _fnRhofit!,
    RhoGenerate: _fnRhoGenerate!,
    StochFit: _fnStochFit!,
  };
}
