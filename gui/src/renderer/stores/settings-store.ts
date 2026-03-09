import { create } from 'zustand';
import { DEFAULT_SETTINGS, type ModelSettings } from '../lib/types';

interface SettingsState {
  settings: ModelSettings;
  update: (patch: Partial<ModelSettings>) => void;
  reset: () => void;
}

const STORAGE_KEY = 'stochfit-settings';

const loadSettings = (): ModelSettings => {
  try {
    const stored = localStorage.getItem(STORAGE_KEY);
    if (stored) {
      return { ...DEFAULT_SETTINGS, ...JSON.parse(stored) };
    }
  } catch (e) {
    console.warn('Failed to load settings from localStorage', e);
  }
  return { ...DEFAULT_SETTINGS };
};

export const useSettingsStore = create<SettingsState>((set) => ({
  settings: loadSettings(),
  update: (patch) => set((s) => {
    const updated = { ...s.settings, ...patch };
    try {
      localStorage.setItem(STORAGE_KEY, JSON.stringify(updated));
    } catch (e) {
      console.warn('Failed to save settings to localStorage', e);
    }
    return { settings: updated };
  }),
  reset: () => {
    try {
      localStorage.removeItem(STORAGE_KEY);
    } catch (e) {
      console.warn('Failed to clear settings from localStorage', e);
    }
    return set({ settings: { ...DEFAULT_SETTINGS } });
  },
}));
