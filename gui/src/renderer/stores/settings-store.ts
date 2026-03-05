import { create } from 'zustand';
import { DEFAULT_SETTINGS, type ModelSettings } from '../lib/types';

interface SettingsState {
  settings: ModelSettings;
  update: (patch: Partial<ModelSettings>) => void;
  reset: () => void;
}

export const useSettingsStore = create<SettingsState>((set) => ({
  settings: { ...DEFAULT_SETTINGS },
  update: (patch) => set((s) => ({ settings: { ...s.settings, ...patch } })),
  reset: () => set({ settings: { ...DEFAULT_SETTINGS } }),
}));
