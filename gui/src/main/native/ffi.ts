import koffi from 'koffi';
import path from 'path';
import { app } from 'electron';

// Ensure struct types are registered before binding functions
import './structs';

let _stochLib: ReturnType<typeof koffi.load> | null = null;
let _levmarLib: ReturnType<typeof koffi.load> | null = null;

function libPath(name: string): string {
  if (app.isPackaged) {
    return path.join(process.resourcesPath, name);
  }
  return path.join(__dirname, '../../../../build', name);
}

export function getStochLib() {
  if (!_stochLib) {
    _stochLib = koffi.load(libPath('libstochfit.so'));
  }
  return _stochLib;
}

export function getLevmarLib() {
  if (!_levmarLib) {
    _levmarLib = koffi.load(libPath('liblevmardll.so'));
  }
  return _levmarLib;
}

type KoffiFn = ReturnType<ReturnType<typeof koffi.load>['func']>;

let _fnInit: KoffiFn | null = null;
let _fnStart: KoffiFn | null = null;
let _fnCancel: KoffiFn | null = null;
let _fnGetData: KoffiFn | null = null;
let _fnArraySizes: KoffiFn | null = null;
let _fnWarmedUp: KoffiFn | null = null;
let _fnSAparams: KoffiFn | null = null;

export function getStochFns() {
  const lib = getStochLib();
  if (!_fnInit) {
    _fnInit = lib.func('void Init(ReflSettings *initstruct)');
    _fnStart = lib.func('void Start(int iterations)');
    _fnCancel = lib.func('void Cancel()');
    _fnGetData = lib.func('int GetData(double *ZRange, double *Rho, double *QRange, double *Refl, double *roughness, double *chisquare, double *goodnessoffit, int *isfinished)');
    _fnArraySizes = lib.func('void ArraySizes(int *RhoSize, int *Reflsize)');
    _fnWarmedUp = lib.func('bool WarmedUp()');
    _fnSAparams = lib.func('void SAparams(double *lowestenergy, double *temp, int *mode)');
  }
  return {
    Init: _fnInit!,
    Start: _fnStart!,
    Cancel: _fnCancel!,
    GetData: _fnGetData!,
    ArraySizes: _fnArraySizes!,
    WarmedUp: _fnWarmedUp!,
    SAparams: _fnSAparams!,
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
