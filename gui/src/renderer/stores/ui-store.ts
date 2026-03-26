import { create } from 'zustand';

export type ActivePanel = 'data' | 'parameters' | 'mi' | 'boxmodel';
export type GraphMode = 'standard' | 'fresnel' | 'rq4';

interface UiState {
  activePanel: ActivePanel;
  settingsOpen: boolean;
  aboutOpen: boolean;
  sldCalcOpen: boolean;
  publicationMode: boolean;
  graphMode: GraphMode;
  darkMode: boolean;
  gpuAvailable: boolean;
  masterGraphOpen: boolean;
  toast: string | null;

  setActivePanel: (p: ActivePanel) => void;
  setSettingsOpen: (v: boolean) => void;
  setAboutOpen: (v: boolean) => void;
  setSldCalcOpen: (v: boolean) => void;
  setPublicationMode: (v: boolean) => void;
  setGraphMode: (m: GraphMode) => void;
  setDarkMode: (v: boolean) => void;
  setGpuAvailable: (v: boolean) => void;
  setMasterGraphOpen: (v: boolean) => void;
  showToast: (message: string, durationMs?: number) => void;
}

export const useUiStore = create<UiState>((set) => ({
  activePanel: 'data',
  settingsOpen: false,
  aboutOpen: false,
  sldCalcOpen: false,
  publicationMode: false,
  graphMode: 'standard',
  darkMode: true,
  gpuAvailable: false,
  masterGraphOpen: false,
  toast: null,

  setActivePanel: (activePanel) => set({ activePanel }),
  setSettingsOpen: (settingsOpen) => set({ settingsOpen }),
  setAboutOpen: (aboutOpen) => set({ aboutOpen }),
  setSldCalcOpen: (sldCalcOpen) => set({ sldCalcOpen }),
  setPublicationMode: (publicationMode) => set({ publicationMode }),
  setGraphMode: (graphMode) => set({ graphMode }),
  setDarkMode: (darkMode) => set({ darkMode }),
  setGpuAvailable: (gpuAvailable) => set({ gpuAvailable }),
  setMasterGraphOpen: (masterGraphOpen) => set({ masterGraphOpen }),
  showToast: (message, durationMs = 3000) => {
    set({ toast: message });
    setTimeout(() => set({ toast: null }), durationMs);
  },
}));
