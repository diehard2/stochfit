import { create } from 'zustand';

export type ActivePanel = 'data' | 'parameters' | 'fitting' | 'rho' | 'refl' | 'stoch';

interface UiState {
  activePanel: ActivePanel;
  settingsOpen: boolean;
  aboutOpen: boolean;
  publicationMode: boolean;

  setActivePanel: (p: ActivePanel) => void;
  setSettingsOpen: (v: boolean) => void;
  setAboutOpen: (v: boolean) => void;
  setPublicationMode: (v: boolean) => void;
}

export const useUiStore = create<UiState>((set) => ({
  activePanel: 'data',
  settingsOpen: false,
  aboutOpen: false,
  publicationMode: false,

  setActivePanel: (activePanel) => set({ activePanel }),
  setSettingsOpen: (settingsOpen) => set({ settingsOpen }),
  setAboutOpen: (aboutOpen) => set({ aboutOpen }),
  setPublicationMode: (publicationMode) => set({ publicationMode }),
}));
