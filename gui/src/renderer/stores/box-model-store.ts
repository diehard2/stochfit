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

  setBoxes: (n: number) => void;
  setSubRough: (v: number) => void;
  setNormFactor: (v: number) => void;
  setOneSigma: (v: boolean) => void;
  setBoxRows: (rows: BoxRow[] | ((prev: BoxRow[]) => BoxRow[])) => void;
  setSolutions: (solutions: BoxModelSolution[], paramsPerSolution: number) => void;
  setActiveIndex: (i: number) => void;
  setGenRefl: (r: number[] | null) => void;
  setGenEDP: (ed: number[] | null, boxED: number[] | null, zRange: number[] | null) => void;
  reset: () => void;
}

export const useBoxModelStore = create<BoxModelState>((set) => ({
  boxes: 2,
  subRough: 3.0,
  normFactor: 1.0,
  oneSigma: false,
  boxRows: [defaultRow(), defaultRow()],

  solutions: [],
  activeIndex: 0,
  genRefl: null,
  genED: null,
  genBoxED: null,
  genZRange: null,
  paramsPerSolution: 0,

  setBoxes: (boxes) => set({ boxes }),
  setSubRough: (subRough) => set({ subRough }),
  setNormFactor: (normFactor) => set({ normFactor }),
  setOneSigma: (oneSigma) => set({ oneSigma }),
  setBoxRows: (rows) => set((s) => ({ boxRows: typeof rows === 'function' ? rows(s.boxRows) : rows })),
  setSolutions: (solutions, paramsPerSolution) => set({ solutions, paramsPerSolution, activeIndex: 0 }),
  setActiveIndex: (activeIndex) => set({ activeIndex }),
  setGenRefl: (genRefl) => set({ genRefl }),
  setGenEDP: (genED, genBoxED, genZRange) => set({ genED, genBoxED, genZRange }),
  reset: () => set({
    boxes: 2, subRough: 3.0, normFactor: 1.0, oneSigma: false,
    boxRows: [defaultRow(), defaultRow()],
    solutions: [], activeIndex: 0, genRefl: null, genED: null, genBoxED: null, genZRange: null, paramsPerSolution: 0,
  }),
}));
