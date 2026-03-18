import { create } from 'zustand';
import type { BoxRow } from '../components/shared/BoxParameterTable';

export type { BoxRow };

export interface BoxModelSolution {
  params: number[];
  chiSquare: number;
}

function defaultRow(): BoxRow {
  return { length: 15.0, rho: 0.5, sigma: 3.0 };
}

export interface FitReport {
  info: number[];
  covariance: number[];
  paramCount: number;
  paramNames: string[];
}

const STORAGE_KEY = 'stochfit-box-model';

interface PersistedParams {
  boxes: number;
  subRough: number;
  normFactor: number;
  oneSigma: boolean;
  boxRows: BoxRow[];
}

function loadParams(): PersistedParams {
  try {
    const stored = localStorage.getItem(STORAGE_KEY);
    if (stored) return JSON.parse(stored) as PersistedParams;
  } catch { /* ignore */ }
  return { boxes: 2, subRough: 3.0, normFactor: 1.0, oneSigma: false, boxRows: [defaultRow(), defaultRow()] };
}

function saveParams(p: PersistedParams) {
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(p));
  } catch { /* ignore */ }
}

interface BoxModelState {
  // Input params (persisted across panel switches)
  boxes: number;
  subRough: number;
  normFactor: number;
  oneSigma: boolean;
  boxRows: BoxRow[];

  // Fit results
  solutions: BoxModelSolution[];
  activeIndex: number;
  genRefl: number[] | null;
  genED: number[] | null;
  genBoxED: number[] | null;
  genZRange: number[] | null;
  paramsPerSolution: number;
  lastFitReport: FitReport | null;

  setBoxes: (n: number) => void;
  setSubRough: (v: number) => void;
  setNormFactor: (v: number) => void;
  setOneSigma: (v: boolean) => void;
  setBoxRows: (rows: BoxRow[] | ((prev: BoxRow[]) => BoxRow[])) => void;
  setSolutions: (solutions: BoxModelSolution[], paramsPerSolution: number) => void;
  setActiveIndex: (i: number) => void;
  setGenRefl: (r: number[] | null) => void;
  setGenEDP: (ed: number[] | null, boxED: number[] | null, zRange: number[] | null) => void;
  setLastFitReport: (report: FitReport | null) => void;
  reset: () => void;
}

const initial = loadParams();

export const useBoxModelStore = create<BoxModelState>((set, get) => ({
  ...initial,

  solutions: [],
  activeIndex: 0,
  genRefl: null,
  genED: null,
  genBoxED: null,
  genZRange: null,
  paramsPerSolution: 0,
  lastFitReport: null,

  setBoxes: (boxes) => {
    const s = get();
    const p = { boxes, subRough: s.subRough, normFactor: s.normFactor, oneSigma: s.oneSigma, boxRows: s.boxRows };
    saveParams(p);
    set({ boxes });
  },
  setSubRough: (subRough) => {
    const s = get();
    saveParams({ boxes: s.boxes, subRough, normFactor: s.normFactor, oneSigma: s.oneSigma, boxRows: s.boxRows });
    set({ subRough });
  },
  setNormFactor: (normFactor) => {
    const s = get();
    saveParams({ boxes: s.boxes, subRough: s.subRough, normFactor, oneSigma: s.oneSigma, boxRows: s.boxRows });
    set({ normFactor });
  },
  setOneSigma: (oneSigma) => {
    const s = get();
    saveParams({ boxes: s.boxes, subRough: s.subRough, normFactor: s.normFactor, oneSigma, boxRows: s.boxRows });
    set({ oneSigma });
  },
  setBoxRows: (rows) => set((s) => {
    const boxRows = typeof rows === 'function' ? rows(s.boxRows) : rows;
    saveParams({ boxes: s.boxes, subRough: s.subRough, normFactor: s.normFactor, oneSigma: s.oneSigma, boxRows });
    return { boxRows };
  }),
  setSolutions: (solutions, paramsPerSolution) => set({ solutions, paramsPerSolution, activeIndex: 0 }),
  setActiveIndex: (activeIndex) => set({ activeIndex }),
  setGenRefl: (genRefl) => set({ genRefl }),
  setGenEDP: (genED, genBoxED, genZRange) => set({ genED, genBoxED, genZRange }),
  setLastFitReport: (lastFitReport) => set({ lastFitReport }),
  reset: () => {
    try { localStorage.removeItem(STORAGE_KEY); } catch { /* ignore */ }
    set({
      boxes: 2, subRough: 3.0, normFactor: 1.0, oneSigma: false,
      boxRows: [defaultRow(), defaultRow()],
      solutions: [], activeIndex: 0, genRefl: null, genED: null, genBoxED: null, genZRange: null, paramsPerSolution: 0, lastFitReport: null,
    });
  },
}));
