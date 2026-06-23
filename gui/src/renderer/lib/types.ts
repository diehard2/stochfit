export interface ReflDataMeta {
  label: string;
  value: string;
}

export interface ReflData {
  q: number[];
  refl: number[];
  reflError: number[];
  qError: number[];
  filePath: string;
  fileName: string;
  /** Optional metadata extracted from the file header (ORSO YAML, NeXus attrs, etc.) */
  metadata?: ReflDataMeta[];
}

export interface FitResult {
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

export interface ModelSettings {
  // Sample
  subSLD: number;
  filmSLD: number;
  supSLD: number;
  boxes: number;
  filmAbs: number;
  subAbs: number;
  supAbs: number;
  wavelength: number;
  useSurfAbs: boolean;
  qSpread: number;
  forcenorm: boolean;
  forcesig: number;
  neutron: boolean;
  resolution: number;
  filmLength: number;
  objectivefunction: number;

  // SA algorithm
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
}

export const DEFAULT_SETTINGS: ModelSettings = {
  subSLD: 9.38,
  filmSLD: 9.38,
  supSLD: 0.0,
  boxes: 40,
  filmAbs: 1e-14,
  subAbs: 2e-8,
  supAbs: 0.0,
  wavelength: 1.24,
  useSurfAbs: false,
  qSpread: 0.0,
  forcenorm: false,
  forcesig: 0.0,
  neutron: false,
  resolution: 10,
  filmLength: 25.0,
  objectivefunction: 0, // Log(R) cost
  paramtemp: 0.03, // Parameter temperature
  sigmasearch: 10,
  normSearchPerc: 0,
  absSearchPerc: 0,
  algorithm: 0, // Greedy
  inittemp: 10.0,
  platiter: 4000,
  slope: 0.95,
  gamma: 0.05,
  stunFunc: 0,
  adaptive: false,
  tempiter: 100,
  stunDeciter: 200000,
  gammadec: 0.85,
  critEdgeOffset: 0,
  highQOffset: 0,
  iterations: 5000000,
  title: 'StochFit',
};

// Inlined from stochfit-api / levmar-api to avoid main-process import in renderer
export interface SARunState {
  roughness: number;
  filmAbsInput: number;
  surfAbs: number;
  temperature: number;
  impNorm: number;
  avgfSTUN: number;
  bestSolution: number;
  chiSquare: number;
  goodnessOfFit: number;
  iteration: number;
  edValues: number[];
  edCount: number;
}

// Re-exported from BoxParameterTable for use in StochFitOutput
import type { BoxRow } from '../components/shared/BoxParameterTable';
export type { BoxRow };

export interface LMResult {
  parameters: number[];
  covariance: number[];
  info: number[];
}

export interface StochFitOutput {
  version: 2;
  savedAt: string;
  dataFile: string;
  settings: ModelSettings;
  saState: SARunState;
  fitResult: Omit<FitResult, 'isFinished'>;
  boxModel?: {
    boxes: number;
    subRough: number;
    zOffset: number;
    oneSigma: boolean;
    boxRows: BoxRow[];
    lmResult?: LMResult;
  };
}

export interface RhoEDPResult {
  ed: number[];
  boxED: number[];
}

export interface StochFitResult extends LMResult {
  paramArray: number[];
  chiSquareArray: number[];
  paramArraySize: number;
}

export interface BoxLMSettings {
  subSLD: number;
  supSLD: number;
  boxes: number;
  wavelength: number;
  qSpread: number;
  impNorm: boolean;
  fitFunc: number;
  lowQOffset: number;
  highQOffset: number;
  iterations: number;
  oneSigma: boolean;
  parameters: number[];
  ul: number[];
  ll: number[];
  paramPercs: number[];
}
