export const IPC = {
  // StochFit (SA) channels
  STOCH_INIT: 'stoch:init',
  STOCH_START: 'stoch:start',
  STOCH_CANCEL: 'stoch:cancel',
  STOCH_GET_DATA: 'stoch:getData',
  STOCH_ARRAY_SIZES: 'stoch:arraySizes',
  STOCH_WARMED_UP: 'stoch:warmedUp',
  STOCH_SA_PARAMS: 'stoch:saParams',
  STOCH_GPU_AVAILABLE: 'stoch:gpuAvailable',

  // LevMar channels
  LM_FAST_REFL_FIT: 'lm:fastReflFit',
  LM_FAST_REFL_GENERATE: 'lm:fastReflGenerate',
  LM_RHO_FIT: 'lm:rhoFit',
  LM_RHO_GENERATE: 'lm:rhoGenerate',
  LM_STOCH_FIT: 'lm:stochFit',

  // File system channels
  FS_OPEN_FILE: 'fs:openFile',
  FS_SAVE_FILE: 'fs:saveFile',

  // Progress events (main → renderer)
  FIT_PROGRESS: 'fit:progress',
  FIT_COMPLETE: 'fit:complete',

  // Settings events (main → renderer)
  SETTINGS_RESET: 'settings:reset',
} as const;

export type IpcChannel = typeof IPC[keyof typeof IPC];
