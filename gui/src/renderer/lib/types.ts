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

  // GPU acceleration
  useGpu: boolean;
  gpuChains: number;  // parallel SA chains; lower = easier to compare with CPU
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
  resolution: 3,
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
  useGpu: false,
  gpuChains: 1,
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
