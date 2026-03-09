import koffi from 'koffi';
import { getStochFns } from './ffi';

export interface ReflSettingsInput {
  directory: string;
  q: number[];
  refl: number[];
  reflError: number[];
  qError: number[];
  qPoints: number;
  subSLD: number;
  filmSLD: number;
  supSLD: number;
  boxes: number;
  filmAbs: number;
  subAbs: number;
  supAbs: number;
  wavelength: number;
  useSurfAbs: boolean;
  leftoffset: number;
  qErr: number;
  forcenorm: boolean;
  forcesig: number;
  debug: boolean;
  xrOnly: boolean;
  resolution: number;
  totallength: number;
  filmLength: number;
  impnorm: boolean;
  objectivefunction: number;
  paramtemp: number;
  sigmasearch: number;
  normSearchPerc: number;
  absSearchPerc: number;
  algorithm: number;
  inittemp: number;
  platiter: number;
  slope: number;
  gamma: number;
  stunFunc: number;
  adaptive: boolean;
  tempiter: number;
  stunDeciter: number;
  gammadec: number;
  critEdgeOffset: number;
  highQOffset: number;
  iterations: number;
  title: string;
  useGpu: boolean;
}

export interface FitData {
  zRange: number[];
  rho: number[];
  qRange: number[];
  refl: number[];
  roughness: number;
  chiSquare: number;
  goodnessOfFit: number;
  isFinished: boolean;
}

export interface SAParams {
  lowestEnergy: number;
  temp: number;
  mode: number;
}

// Native allocations kept alive for the duration of the fitting session
let _qBuf: unknown = null;
let _reflBuf: unknown = null;
let _reflErrBuf: unknown = null;
let _qErrBuf: unknown = null;

function allocAndEncode(arr: number[]): unknown {
  const buf = koffi.alloc('double', arr.length);
  koffi.encode(buf, 'double', arr, arr.length);
  return buf;
}

export function stochInit(settings: ReflSettingsInput): void {
  const fns = getStochFns();
  const n = settings.qPoints;

  _qBuf = allocAndEncode(settings.q.slice(0, n));
  _reflBuf = allocAndEncode(settings.refl.slice(0, n));
  _reflErrBuf = allocAndEncode(settings.reflError.slice(0, n));
  _qErrBuf = allocAndEncode(settings.qError.slice(0, n));

  const s = {
    Directory: settings.directory,
    Q: _qBuf,
    Refl: _reflBuf,
    ReflError: _reflErrBuf,
    QError: _qErrBuf,
    QPoints: settings.qPoints,
    SubSLD: settings.subSLD,
    FilmSLD: settings.filmSLD,
    SupSLD: settings.supSLD,
    Boxes: settings.boxes,
    FilmAbs: settings.filmAbs,
    SubAbs: settings.subAbs,
    SupAbs: settings.supAbs,
    Wavelength: settings.wavelength,
    UseSurfAbs: settings.useSurfAbs ? 1 : 0,
    Leftoffset: settings.leftoffset,
    QErr: settings.qErr,
    Forcenorm: settings.forcenorm ? 1 : 0,
    Forcesig: settings.forcesig,
    Debug: settings.debug ? 1 : 0,
    XRonly: settings.xrOnly ? 1 : 0,
    Resolution: settings.resolution,
    Totallength: settings.totallength,
    FilmLength: settings.filmLength,
    Impnorm: settings.impnorm ? 1 : 0,
    Objectivefunction: settings.objectivefunction,
    Paramtemp: settings.paramtemp,
    Sigmasearch: settings.sigmasearch,
    NormalizationSearchPerc: settings.normSearchPerc,
    AbsorptionSearchPerc: settings.absSearchPerc,
    Algorithm: settings.algorithm,
    Inittemp: settings.inittemp,
    Platiter: settings.platiter,
    Slope: settings.slope,
    Gamma: settings.gamma,
    STUNfunc: settings.stunFunc,
    Adaptive: settings.adaptive ? 1 : 0,
    Tempiter: settings.tempiter,
    STUNdeciter: settings.stunDeciter,
    Gammadec: settings.gammadec,
    CritEdgeOffset: settings.critEdgeOffset,
    HighQOffset: settings.highQOffset,
    Iterations: settings.iterations,
    IterationsCompleted: 0,
    ChiSquare: 0.0,
    UseGpu: settings.useGpu ? 1 : 0,
    Title: settings.title,
  };

  fns.Init(s);
}

export function stochStart(iterations: number): void {
  getStochFns().Start(iterations);
}

export function stochCancel(): void {
  getStochFns().Cancel();
}

export function stochArraySizes(): { rhoSize: number; reflSize: number } {
  const fns = getStochFns();
  const rhoSizeBuf = koffi.alloc('int', 1);
  const reflSizeBuf = koffi.alloc('int', 1);
  fns.ArraySizes(rhoSizeBuf, reflSizeBuf);
  const rhoSize = koffi.decode('int', rhoSizeBuf, 1)[0];
  const reflSize = koffi.decode('int', reflSizeBuf, 1)[0];
  return { rhoSize, reflSize };
}

export function stochGetData(): FitData {
  const fns = getStochFns();
  const { rhoSize, reflSize } = stochArraySizes();

  // Allocate buffers that C++ can write to
  const zRangeBuf = koffi.alloc('double', rhoSize);
  const rhoBuf = koffi.alloc('double', rhoSize);
  const qRangeBuf = koffi.alloc('double', reflSize);
  const reflBuf = koffi.alloc('double', reflSize);
  const roughnessBuf = koffi.alloc('double', 1);
  const chiSquareBuf = koffi.alloc('double', 1);
  const goodnessOfFitBuf = koffi.alloc('double', 1);
  const isFinishedBuf = koffi.alloc('int', 1);

  fns.GetData(zRangeBuf, rhoBuf, qRangeBuf, reflBuf, roughnessBuf, chiSquareBuf, goodnessOfFitBuf, isFinishedBuf);

  // Decode the buffers back to JavaScript arrays
  const zRange = koffi.decode('double', zRangeBuf, rhoSize);
  const rho = koffi.decode('double', rhoBuf, rhoSize);
  const qRange = koffi.decode('double', qRangeBuf, reflSize);
  const refl = koffi.decode('double', reflBuf, reflSize);
  const roughness = koffi.decode('double', roughnessBuf, 1)[0];
  const chiSquare = koffi.decode('double', chiSquareBuf, 1)[0];
  const goodnessOfFit = koffi.decode('double', goodnessOfFitBuf, 1)[0];
  const isFinished = koffi.decode('int', isFinishedBuf, 1)[0] !== 0;

  return {
    zRange,
    rho,
    qRange,
    refl,
    roughness,
    chiSquare,
    goodnessOfFit,
    isFinished,
  };
}

export function stochWarmedUp(): boolean {
  return Boolean(getStochFns().WarmedUp());
}

export function stochSAParams(): SAParams {
  const fns = getStochFns();
  const lowestEnergyBuf = koffi.alloc('double', 1);
  const tempBuf = koffi.alloc('double', 1);
  const modeBuf = koffi.alloc('int', 1);
  fns.SAparams(lowestEnergyBuf, tempBuf, modeBuf);
  const lowestEnergy = koffi.decode('double', lowestEnergyBuf, 1)[0];
  const temp = koffi.decode('double', tempBuf, 1)[0];
  const mode = koffi.decode('int', modeBuf, 1)[0];
  return { lowestEnergy, temp, mode };
}

export function stochGpuAvailable(): boolean {
  return Boolean(getStochFns().GpuAvailable());
}
