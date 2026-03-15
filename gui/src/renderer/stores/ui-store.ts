import { create } from 'zustand';

export type ActivePanel = 'data' | 'parameters' | 'mi' | 'boxmodel';

interface UiState {
  activePanel: ActivePanel;
  settingsOpen: boolean;
  aboutOpen: boolean;
  publicationMode: boolean;
  normalizeByFresnel: boolean;
  gpuAvailable: boolean;

  setActivePanel: (p: ActivePanel) => void;
  setSettingsOpen: (v: boolean) => void;
  setAboutOpen: (v: boolean) => void;
  setPublicationMode: (v: boolean) => void;
  setNormalizeByFresnel: (v: boolean) => void;
  setGpuAvailable: (v: boolean) => void;
}

export const useUiStore = create<UiState>((set) => ({
  activePanel: 'data',
  settingsOpen: false,
  aboutOpen: false,
  publicationMode: false,
  normalizeByFresnel: false,
  gpuAvailable: false,

  setActivePanel: (activePanel) => set({ activePanel }),
  setSettingsOpen: (settingsOpen) => set({ settingsOpen }),
  setAboutOpen: (aboutOpen) => set({ aboutOpen }),
  setPublicationMode: (publicationMode) => set({ publicationMode }),
  setNormalizeByFresnel: (normalizeByFresnel) => set({ normalizeByFresnel }),
  setGpuAvailable: (gpuAvailable) => set({ gpuAvailable }),
}));
