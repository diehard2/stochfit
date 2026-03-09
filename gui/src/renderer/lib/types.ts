export interface ReflData {
  q: number[];
  refl: number[];
  reflError: number[];
  qError: number[];
  filePath: string;
  fileName: string;
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
  leftoffset: number;
  qErr: number;
  forcenorm: boolean;
  forcesig: number;
  xrOnly: boolean;
  resolution: number;
  totallength: number;
  filmLength: number;
  impnorm: boolean;
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

  // GPU acceleration
  useGpu: boolean;
}

export const DEFAULT_SETTINGS: ModelSettings = {
  subSLD: 2.07,
  filmSLD: 9.4,
  supSLD: 0.0,
  boxes: 5,
  filmAbs: 0.0,
  subAbs: 0.0,
  supAbs: 0.0,
  wavelength: 1.54,
  useSurfAbs: false,
  leftoffset: 0,
  qErr: 0.0,
  forcenorm: false,
  forcesig: 0.0,
  xrOnly: true,
  resolution: 0,
  totallength: 500.0,
  filmLength: 100.0,
  impnorm: false,
  objectivefunction: 0, // Log(R) cost
  paramtemp: 0.03, // Parameter temperature
  sigmasearch: 100,
  normSearchPerc: 10,
  absSearchPerc: 10,
  algorithm: 0, // Greedy (doesn't use slope parameter)
  inittemp: 10.0,
  platiter: 10,
  slope: 0.95,
  gamma: 1.0,
  stunFunc: 0,
  adaptive: true,
  tempiter: 50,
  stunDeciter: 1000,
  gammadec: 0.5,
  critEdgeOffset: 0,
  highQOffset: 0,
  iterations: 5000000,
  title: 'StochFit',
  useGpu: false, // Opt-in, not default enabled
};

export interface BoxLMSettings {
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
  oneSigma: boolean;
  writeFiles: boolean;
  parameters: number[];
  ul: number[];
  ll: number[];
  paramPercs: number[];
}
