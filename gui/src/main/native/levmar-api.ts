import koffi from 'koffi';
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

function allocAndEncode(arr: number[]): unknown {
  const buf = koffi.alloc('double', arr.length);
  koffi.encode(buf, 'double', arr);
  return buf;
}

function buildBoxStruct(input: BoxReflSettingsInput) {
  const n = input.qPoints;
  const p = input.ul.length;
  const zLen = input.zLength ?? 0;

  const qBuf = allocAndEncode(input.q.slice(0, n));
  const reflBuf = allocAndEncode(input.refl.slice(0, n));
  const reflErrBuf = allocAndEncode(input.reflError.slice(0, n));
  const qErrBuf = allocAndEncode(input.qError.slice(0, n));
  const ulBuf = allocAndEncode(input.ul.slice(0, p));
  const llBuf = allocAndEncode(input.ll.slice(0, p));
  const paramPercsBuf = allocAndEncode(input.paramPercs.slice(0, p));

  let miedpBuf: unknown = null;
  let zIncrBuf: unknown = null;
  if (input.miedp && input.zIncrement && zLen > 0) {
    miedpBuf = allocAndEncode(input.miedp.slice(0, zLen));
    zIncrBuf = allocAndEncode(input.zIncrement.slice(0, zLen));
  }

  return {
    Directory: input.directory,
    Q: qBuf,
    Refl: reflBuf,
    ReflError: reflErrBuf,
    QError: qErrBuf,
    UL: ulBuf,
    LL: llBuf,
    ParamPercs: paramPercsBuf,
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
  const parameters = [...params];
  const covar = new Array<number>(p * p).fill(0);
  const info = new Array<number>(10).fill(0);
  fns.FastReflfit(s, parameters, covar, p, info);
  return { parameters, covariance: covar, info };
}

export function levmarFastReflGenerate(input: BoxReflSettingsInput, params: number[]): number[] {
  const fns = getLevmarFns();
  const s = buildBoxStruct(input);
  const refl = new Array<number>(input.qPoints).fill(0);
  fns.FastReflGenerate(s, [...params], params.length, refl);
  return refl;
}

export function levmarRhoFit(input: BoxReflSettingsInput, params: number[]): LMFitResult {
  const fns = getLevmarFns();
  const s = buildBoxStruct(input);
  const p = params.length;
  const parameters = [...params];
  const covar = new Array<number>(p * p).fill(0);
  const info = new Array<number>(10).fill(0);
  fns.Rhofit(s, parameters, covar, p, info);
  return { parameters, covariance: covar, info };
}

export interface RhoGenerateResult {
  ed: number[];
  boxED: number[];
}

export function levmarRhoGenerate(input: BoxReflSettingsInput, params: number[]): RhoGenerateResult {
  const fns = getLevmarFns();
  const s = buildBoxStruct(input);
  const zLen = input.zLength ?? 0;
  const ed = new Array<number>(zLen).fill(0);
  const boxED = new Array<number>(zLen).fill(0);
  fns.RhoGenerate(s, [...params], params.length, ed, boxED);
  return { ed, boxED };
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
  const parameters = [...params];
  const covar = new Array<number>(p * p).fill(0);
  const info = new Array<number>(10).fill(0);
  const maxSolutions = 1000;
  const paramArray = new Array<number>(maxSolutions * p).fill(0);
  const chiSquareArray = new Array<number>(maxSolutions).fill(0);
  const paramArraySize = [0];
  fns.StochFit(s, parameters, covar, p, info, paramArray, chiSquareArray, paramArraySize);
  const n = paramArraySize[0];
  return {
    parameters,
    covariance: covar,
    info,
    paramArray: paramArray.slice(0, n * p),
    chiSquareArray: chiSquareArray.slice(0, n),
    paramArraySize: n,
  };
}
