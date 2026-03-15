import { create } from 'zustand';

export interface BoxModelSolution {
  params: number[];
  chiSquare: number;
}

interface BoxModelState {
  solutions: BoxModelSolution[];
  activeIndex: number;
  genRefl: number[] | null;
  genED: number[] | null;
  genBoxED: number[] | null;
  genZRange: number[] | null;
  paramsPerSolution: number;

  setSolutions: (solutions: BoxModelSolution[], paramsPerSolution: number) => void;
  setActiveIndex: (i: number) => void;
  setGenRefl: (r: number[] | null) => void;
  setGenEDP: (ed: number[] | null, boxED: number[] | null, zRange: number[] | null) => void;
  reset: () => void;
}

export const useBoxModelStore = create<BoxModelState>((set) => ({
  solutions: [],
  activeIndex: 0,
  genRefl: null,
  genED: null,
  genBoxED: null,
  genZRange: null,
  paramsPerSolution: 0,

  setSolutions: (solutions, paramsPerSolution) => set({ solutions, paramsPerSolution, activeIndex: 0 }),
  setActiveIndex: (activeIndex) => set({ activeIndex }),
  setGenRefl: (genRefl) => set({ genRefl }),
  setGenEDP: (genED, genBoxED, genZRange) => set({ genED, genBoxED, genZRange }),
  reset: () => set({ solutions: [], activeIndex: 0, genRefl: null, genED: null, genBoxED: null, genZRange: null, paramsPerSolution: 0 }),
}));
