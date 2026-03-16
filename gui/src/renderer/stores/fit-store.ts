import { create } from 'zustand';
import type { FitResult, SAParams } from '../lib/types';

type FitStatus = 'idle' | 'running' | 'completed' | 'cancelled';

interface FitState {
  status: FitStatus;
  result: FitResult | null;
  saParams: SAParams | null;
  pollTimer: ReturnType<typeof setInterval> | null;
  miBoxED: number[] | null;
  itPerSec: number;

  setStatus: (s: FitStatus) => void;
  setResult: (r: FitResult) => void;
  setSAParams: (p: SAParams) => void;
  setPollTimer: (t: ReturnType<typeof setInterval> | null) => void;
  setMiBoxED: (ed: number[] | null) => void;
  setItPerSec: (v: number) => void;
  reset: () => void;
}

export const useFitStore = create<FitState>((set) => ({
  status: 'idle',
  result: null,
  saParams: null,
  pollTimer: null,
  miBoxED: null,
  itPerSec: 0,

  setStatus: (status) => set({ status }),
  setResult: (result) => set({ result }),
  setSAParams: (saParams) => set({ saParams }),
  setPollTimer: (pollTimer) => set({ pollTimer }),
  setMiBoxED: (miBoxED) => set({ miBoxED }),
  setItPerSec: (itPerSec) => set({ itPerSec }),
  reset: () => set({ status: 'idle', result: null, saParams: null, pollTimer: null, miBoxED: null, itPerSec: 0 }),
}));
