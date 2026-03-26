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
  STOCH_STOP: 'stoch:stop',
  STOCH_DESTROY: 'stoch:destroy',
  STOCH_GET_RUN_STATE: 'stoch:getRunState',
  STOCH_LOAD_OUTPUT: 'stoch:loadOutput',
  STOCH_WRITE_OUTPUT: 'stoch:writeOutput',
  STOCH_DELETE_OUTPUT: 'stoch:deleteOutput',

  // LevMar channels
  LM_FAST_REFL_FIT: 'lm:fastReflFit',
  LM_FAST_REFL_GENERATE: 'lm:fastReflGenerate',
  LM_RHO_FIT: 'lm:rhoFit',
  LM_RHO_GENERATE: 'lm:rhoGenerate',
  LM_STOCH_FIT: 'lm:stochFit',

  // File system channels
  FS_OPEN_FILE: 'fs:openFile',
  FS_OPEN_DATA_FILE: 'fs:openDataFile',
  FS_SAVE_FILE: 'fs:saveFile',
  FS_OPEN_PDF: 'fs:openPdf',

  // Progress events (main → renderer)
  FIT_PROGRESS: 'fit:progress',
  FIT_COMPLETE: 'fit:complete',

  // Shell
  SHELL_OPEN_EXTERNAL: 'shell:openExternal',

  // Settings events (main → renderer)
  SETTINGS_RESET: 'settings:reset',
} as const;

export type IpcChannel = typeof IPC[keyof typeof IPC];
