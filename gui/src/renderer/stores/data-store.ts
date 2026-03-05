import { create } from 'zustand';
import type { ReflData } from '../lib/types';

interface DataState {
  data: ReflData | null;
  setData: (data: ReflData) => void;
  clearData: () => void;
}

export const useDataStore = create<DataState>((set) => ({
  data: null,
  setData: (data) => set({ data }),
  clearData: () => set({ data: null }),
}));
