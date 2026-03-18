import { getLevmarFns } from './ffi';

export interface BoxReflSettingsInput {
  directory: string;
  q: number[];
  refl: number[];
  reflError: number[];
  qError: number[];
  ul: number[];
  ll: number[];
  paramPercs: number[];
  qPoints: number;
  oneSigma: boolean;
  writeFiles: boolean;
  subSLD: number;
  supSLD: number;
  boxes: number;
  wavelength: number;
  qSpread: number;
  forcenorm: boolean;
  impNorm: boolean;
  fitFunc: number;
  lowQOffset: number;
  highQOffset: number;
  iterations: number;
  miedp?: number[];
  zIncrement?: number[];
  zLength?: number;
}

function toF64(arr: number[], len: number): Float64Array {
  return new Float64Array(arr.slice(0, len));
}

function buildBoxStruct(input: BoxReflSettingsInput) {
  const n = input.qPoints;
  const p = input.ul.length;
  const zLen = input.zLength ?? 0;

  const dummy = new Float64Array(1);
  const miedpBuf = (input.miedp && zLen > 0) ? toF64(input.miedp, zLen) : dummy;
  const zIncrBuf = (input.zIncrement && zLen > 0) ? toF64(input.zIncrement, zLen) : dummy;

  return {
    Directory: input.directory,
    Q: toF64(input.q, n),
    Refl: toF64(input.refl, n),
    ReflError: toF64(input.reflError, n),
    QError: toF64(input.qError, n),
    UL: toF64(input.ul, p),
    LL: toF64(input.ll, p),
    ParamPercs: toF64(input.paramPercs, p),
    QPoints: input.qPoints,
    OneSigma: input.oneSigma ? 1 : 0,
    WriteFiles: input.writeFiles ? 1 : 0,
    SubSLD: input.subSLD,
    SupSLD: input.supSLD,
    Boxes: input.boxes,
    Wavelength: input.wavelength,
    QSpread: input.qSpread,
    Forcenorm: input.forcenorm ? 1 : 0,
    ImpNorm: input.impNorm ? 1 : 0,
    FitFunc: input.fitFunc,
    LowQOffset: input.lowQOffset,
    HighQOffset: input.highQOffset,
    Iterations: input.iterations,
    MIEDP: miedpBuf,
    ZIncrement: zIncrBuf,
    ZLength: zLen,
  };
}

export interface LMFitResult {
  parameters: number[];
  covariance: number[];
  info: number[];
}

export function levmarFastReflFit(input: BoxReflSettingsInput, params: number[]): LMFitResult {
  const fns = getLevmarFns();
  const s = buildBoxStruct(input);
  const p = params.length;
  const parameters = new Float64Array(params);
  const covar = new Float64Array(p * p);
  const info = new Float64Array(10);
  fns.FastReflfit(s, parameters, covar, p, info);
  return {
    parameters: Array.from(parameters),
    covariance: Array.from(covar),
    info: Array.from(info),
  };
}

export function levmarFastReflGenerate(input: BoxReflSettingsInput, params: number[]): number[] {
  const fns = getLevmarFns();
  const s = buildBoxStruct(input);
  const paramBuf = new Float64Array(params);
  const refl = new Float64Array(input.qPoints);
  fns.FastReflGenerate(s, paramBuf, params.length, refl);
  return Array.from(refl);
}

export function levmarRhoFit(input: BoxReflSettingsInput, params: number[]): LMFitResult {
  const fns = getLevmarFns();
  const s = buildBoxStruct(input);
  const p = params.length;
  const parameters = new Float64Array(params);
  const covar = new Float64Array(p * p);
  const info = new Float64Array(10);
  fns.Rhofit(s, parameters, covar, p, info);
  return {
    parameters: Array.from(parameters),
    covariance: Array.from(covar),
    info: Array.from(info),
  };
}

export interface RhoGenerateResult {
  ed: number[];
  boxED: number[];
}

export function levmarRhoGenerate(input: BoxReflSettingsInput, params: number[]): RhoGenerateResult {
  const fns = getLevmarFns();
  const s = buildBoxStruct(input);
  const zLen = input.zLength ?? 0;
  const paramBuf = new Float64Array(params);
  const ed = new Float64Array(zLen);
  const boxED = new Float64Array(zLen);
  fns.RhoGenerate(s, paramBuf, params.length, ed, boxED);
  return { ed: Array.from(ed), boxED: Array.from(boxED) };
}

export interface StochFitResult extends LMFitResult {
  paramArray: number[];
  chiSquareArray: number[];
  paramArraySize: number;
}

export function levmarStochFit(input: BoxReflSettingsInput, params: number[]): StochFitResult {
  const fns = getLevmarFns();
  const s = buildBoxStruct(input);
  const p = params.length;
  const parameters = new Float64Array(params);
  const covar = new Float64Array(p * p);
  const info = new Float64Array(10);
  const maxSolutions = 1000;
  const paramArray = new Float64Array(maxSolutions * p);
  const chiSquareArray = new Float64Array(maxSolutions);
  const paramArraySize = new Int32Array(1);
  fns.StochFit(s, parameters, covar, p, info, paramArray, chiSquareArray, paramArraySize);
  const n = paramArraySize[0];
  return {
    parameters: Array.from(parameters),
    covariance: Array.from(covar),
    info: Array.from(info),
    paramArray: Array.from(paramArray.slice(0, n * p)),
    chiSquareArray: Array.from(chiSquareArray.slice(0, n)),
    paramArraySize: n,
  };
}
