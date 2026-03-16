import { create } from 'zustand';
import type { BoxRow } from '../components/shared/BoxParameterTable';
import type { LMFitResult } from '../../main/native/levmar-api';

export type { BoxRow };

const STORAGE_KEY = 'stochfit-mi-edp';

interface PersistedParams {
  boxes: number;
  subRough: number;
  zOffset: number;
  oneSigma: boolean;
  boxRows: BoxRow[];
}

function defaultRow(): BoxRow {
  return { length: 15.0, rho: 0.5, sigma: 3.0 };
}

function defaultRows(n: number): BoxRow[] {
  return Array.from({ length: n }, defaultRow);
}

function loadParams(): PersistedParams {
  try {
    const stored = localStorage.getItem(STORAGE_KEY);
    if (stored) return JSON.parse(stored) as PersistedParams;
  } catch { /* ignore */ }
  return { boxes: 2, subRough: 3.0, zOffset: 0.0, oneSigma: false, boxRows: defaultRows(3) };
}

function saveParams(p: PersistedParams) {
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(p));
  } catch { /* ignore */ }
}

interface MiEdpState {
  boxes: number;
  subRough: number;
  zOffset: number;
  oneSigma: boolean;
  boxRows: BoxRow[];
  lmResult: LMFitResult | null;

  setBoxes: (n: number) => void;
  setSubRough: (v: number) => void;
  setZOffset: (v: number) => void;
  setOneSigma: (v: boolean) => void;
  setBoxRows: (rows: BoxRow[] | ((prev: BoxRow[]) => BoxRow[])) => void;
  setLmResult: (r: LMFitResult | null) => void;
  reset: () => void;
}

const initial = loadParams();

export const useMiEdpStore = create<MiEdpState>((set, get) => ({
  ...initial,
  lmResult: null,

  setBoxes: (boxes) => {
    const s = get();
    saveParams({ boxes, subRough: s.subRough, zOffset: s.zOffset, oneSigma: s.oneSigma, boxRows: s.boxRows });
    set({ boxes });
  },
  setSubRough: (subRough) => {
    const s = get();
    saveParams({ boxes: s.boxes, subRough, zOffset: s.zOffset, oneSigma: s.oneSigma, boxRows: s.boxRows });
    set({ subRough });
  },
  setZOffset: (zOffset) => {
    const s = get();
    saveParams({ boxes: s.boxes, subRough: s.subRough, zOffset, oneSigma: s.oneSigma, boxRows: s.boxRows });
    set({ zOffset });
  },
  setOneSigma: (oneSigma) => {
    const s = get();
    saveParams({ boxes: s.boxes, subRough: s.subRough, zOffset: s.zOffset, oneSigma, boxRows: s.boxRows });
    set({ oneSigma });
  },
  setBoxRows: (rows) => set((s) => {
    const boxRows = typeof rows === 'function' ? rows(s.boxRows) : rows;
    saveParams({ boxes: s.boxes, subRough: s.subRough, zOffset: s.zOffset, oneSigma: s.oneSigma, boxRows });
    return { boxRows };
  }),
  setLmResult: (lmResult) => set({ lmResult }),
  reset: () => {
    try { localStorage.removeItem(STORAGE_KEY); } catch { /* ignore */ }
    set({ boxes: 2, subRough: 3.0, zOffset: 0.0, oneSigma: false, boxRows: defaultRows(3), lmResult: null });
  },
}));
