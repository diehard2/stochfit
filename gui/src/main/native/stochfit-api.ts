import koffi from 'koffi';
import fs from 'fs';
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
  qErr: number;
  forcenorm: boolean;
  forcesig: number;
  debug: boolean;
  xrOnly: boolean;
  resolution: number;
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
  iterationsCompleted: number;
}

export interface SAParams {
  lowestEnergy: number;
  temp: number;
  mode: number;
}

// JS representation of StochRunState returned from GetRunState() FFI.
export interface StochRunStateOutput {
  roughness: number;
  filmAbsInput: number;
  surfAbs: number;
  temperature: number;
  impNorm: number;
  avgfSTUN: number;
  bestSolution: number;
  chiSquare: number;
  goodnessOfFit: number;
  iteration: number;    // filled by renderer from last iterationsCompleted
  edValues: number[];   // length == boxes+2
  edCount: number;
}

// Session file written to {dataDirectory}/stochfit-session.json by Electron.
export interface StochSessionFile {
  version: number;
  savedAt: string;
  dataFile: string;
  settings: Record<string, unknown>;
  saState: StochRunStateOutput;
}

// Retrieve current SA state from C++. Only safe to call after stochStop().
// boxes: the current settings.boxes value (used to size the edValues buffer).
export function stochGetRunState(boxes: number): StochRunStateOutput {
  const fns = getStochFns();
  const saScalars = new Float64Array(9);
  const edValues = new Float64Array(boxes + 2);
  const edCount = new Int32Array(1);
  fns.GetRunState(saScalars, edValues, edCount);
  const count = edCount[0];
  return {
    roughness: saScalars[0],
    filmAbsInput: saScalars[1],
    surfAbs: saScalars[2],
    temperature: saScalars[3],
    impNorm: saScalars[4],
    avgfSTUN: saScalars[5],
    bestSolution: saScalars[6],
    chiSquare: saScalars[7],
    goodnessOfFit: saScalars[8],
    iteration: 0, // filled by caller from last iterationsCompleted
    edValues: Array.from(edValues.slice(0, count)),
    edCount: count,
  };
}

// Write a session JSON file. Attempts atomic rename; falls back to direct write
// if rename fails (e.g. OneDrive holds a lock on the file during sync).
export function writeSessionFile(filePath: string, session: StochSessionFile): void {
  const json = JSON.stringify(session, null, 2);
  const tmpPath = filePath + '.tmp';
  fs.writeFileSync(tmpPath, json, 'utf-8');
  try {
    fs.renameSync(tmpPath, filePath);
  } catch (e: unknown) {
    // OneDrive (and some antivirus) can lock files, causing EPERM on rename.
    // Fall back to direct write so the session is still saved.
    try { fs.unlinkSync(tmpPath); } catch { /* ignore */ }
    fs.writeFileSync(filePath, json, 'utf-8');
  }
}

// Read and validate a session JSON file. Returns null if missing or invalid.
export function readSessionFile(filePath: string): StochSessionFile | null {
  try {
    const content = fs.readFileSync(filePath, 'utf-8');
    const data = JSON.parse(content) as StochSessionFile;
    if (data.version !== 1) return null;
    if (!data.saState || !Array.isArray(data.saState.edValues)) return null;
    return data;
  } catch {
    return null;
  }
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

export function stochInit(settings: ReflSettingsInput, runState: StochRunStateOutput | null = null): void {
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
    QErr: settings.qErr,
    Forcenorm: settings.forcenorm ? 1 : 0,
    Forcesig: settings.forcesig,
    Debug: settings.debug ? 1 : 0,
    XRonly: settings.xrOnly ? 1 : 0,
    Resolution: settings.resolution,
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

  // Build StochRunState struct if provided; otherwise pass null (fresh start)
  let stateStruct: unknown = null;
  if (runState !== null) {
    const edBuf = koffi.alloc('double', runState.edValues.length);
    koffi.encode(edBuf, 'double', runState.edValues, runState.edValues.length);
    stateStruct = {
      roughness: runState.roughness,
      filmAbsInput: runState.filmAbsInput,
      surfAbs: runState.surfAbs,
      temperature: runState.temperature,
      impNorm: runState.impNorm,
      avgfSTUN: runState.avgfSTUN,
      bestSolution: runState.bestSolution,
      chiSquare: runState.chiSquare,
      goodnessOfFit: runState.goodnessOfFit,
      iteration: runState.iteration ?? 0,
      edValues: edBuf,
      edCount: runState.edValues.length,
    };
  }

  fns.Init(s, stateStruct);
  const errMsg: string = fns.GetInitError();
  if (errMsg) {
    throw new Error(`StochFit init failed: ${errMsg}`);
  }
}

export function stochStart(iterations: number): void {
  getStochFns().Start(iterations);
}

export function stochStop(): void {
  getStochFns().Stop();
}

export function stochDestroy(): void {
  getStochFns().Destroy();
}

export function stochCancel(): void {
  getStochFns().Cancel();
}

export function stochArraySizes(): { rhoSize: number; reflSize: number } {
  const fns = getStochFns();
  const rhoSize = new Int32Array(1);
  const reflSize = new Int32Array(1);
  fns.ArraySizes(rhoSize, reflSize);
  return { rhoSize: rhoSize[0], reflSize: reflSize[0] };
}

export function stochGetData(): FitData {
  const fns = getStochFns();
  const { rhoSize, reflSize } = stochArraySizes();

  // Use TypedArrays for output parameters - koffi will fill them
  const zRange = new Float64Array(rhoSize);
  const rho = new Float64Array(rhoSize);
  const qRange = new Float64Array(reflSize);
  const refl = new Float64Array(reflSize);
  const roughness = new Float64Array(1);
  const chiSquare = new Float64Array(1);
  const goodnessOfFit = new Float64Array(1);
  const isFinished = new Int32Array(1);

  // GetData returns the current iteration count
  const iterationsCompleted = fns.GetData(zRange, rho, qRange, refl, roughness, chiSquare, goodnessOfFit, isFinished);

  return {
    zRange: Array.from(zRange),
    rho: Array.from(rho),
    qRange: Array.from(qRange),
    refl: Array.from(refl),
    roughness: roughness[0],
    chiSquare: chiSquare[0],
    goodnessOfFit: goodnessOfFit[0],
    isFinished: isFinished[0] !== 0,
    iterationsCompleted,
  };
}

export function stochWarmedUp(): boolean {
  return Boolean(getStochFns().WarmedUp());
}

export function stochSAParams(): SAParams {
  const fns = getStochFns();
  const lowestEnergy = new Float64Array(1);
  const temp = new Float64Array(1);
  const mode = new Int32Array(1);
  fns.SAparams(lowestEnergy, temp, mode);
  return { lowestEnergy: lowestEnergy[0], temp: temp[0], mode: mode[0] };
}

export function stochGpuAvailable(): boolean {
  return Boolean(getStochFns().GpuAvailable());
}
