import { create } from 'zustand';
import type { FitResult, SAParams } from '../lib/types';

type FitStatus = 'idle' | 'running' | 'completed' | 'cancelled';

interface FitState {
  status: FitStatus;
  result: FitResult | null;
  saParams: SAParams | null;
  pollTimer: ReturnType<typeof setInterval> | null;

  setStatus: (s: FitStatus) => void;
  setResult: (r: FitResult) => void;
  setSAParams: (p: SAParams) => void;
  setPollTimer: (t: ReturnType<typeof setInterval> | null) => void;
  reset: () => void;
}

export const useFitStore = create<FitState>((set) => ({
  status: 'idle',
  result: null,
  saParams: null,
  pollTimer: null,

  setStatus: (status) => set({ status }),
  setResult: (result) => set({ result }),
  setSAParams: (saParams) => set({ saParams }),
  setPollTimer: (pollTimer) => set({ pollTimer }),
  reset: () => set({ status: 'idle', result: null, saParams: null, pollTimer: null }),
}));
