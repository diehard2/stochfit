import { contextBridge, ipcRenderer } from 'electron';
import { IPC } from '../shared/ipc-channels';
import type { ReflSettingsInput, StochRunStateOutput, StochSessionFile } from '../main/native/stochfit-api';
import type { BoxReflSettingsInput } from '../main/native/levmar-api';

contextBridge.exposeInMainWorld('api', {
  // StochFit
  stochInit: (settings: ReflSettingsInput, runState: StochRunStateOutput | null) =>
    ipcRenderer.invoke(IPC.STOCH_INIT, settings, runState),
  stochStart: (iterations: number) => ipcRenderer.invoke(IPC.STOCH_START, iterations),
  stochStop: () => ipcRenderer.invoke(IPC.STOCH_STOP),
  stochDestroy: () => ipcRenderer.invoke(IPC.STOCH_DESTROY),
  stochCancel: () => ipcRenderer.invoke(IPC.STOCH_CANCEL),
  stochGetData: () => ipcRenderer.invoke(IPC.STOCH_GET_DATA),
  stochGetRunState: (boxes: number) => ipcRenderer.invoke(IPC.STOCH_GET_RUN_STATE, boxes),
  stochLoadSession: (filePath: string) => ipcRenderer.invoke(IPC.STOCH_LOAD_SESSION, filePath),
  stochWriteSession: (filePath: string, session: StochSessionFile) =>
    ipcRenderer.invoke(IPC.STOCH_WRITE_SESSION, filePath, session),
  stochDeleteSession: (filePath: string) =>
    ipcRenderer.invoke(IPC.STOCH_DELETE_SESSION, filePath),
  stochArraySizes: () => ipcRenderer.invoke(IPC.STOCH_ARRAY_SIZES),
  stochWarmedUp: () => ipcRenderer.invoke(IPC.STOCH_WARMED_UP),
  stochSAParams: () => ipcRenderer.invoke(IPC.STOCH_SA_PARAMS),
  stochGpuAvailable: () => ipcRenderer.invoke(IPC.STOCH_GPU_AVAILABLE),

  // LevMar
  lmFastReflFit: (input: BoxReflSettingsInput, params: number[]) =>
    ipcRenderer.invoke(IPC.LM_FAST_REFL_FIT, input, params),
  lmFastReflGenerate: (input: BoxReflSettingsInput, params: number[]) =>
    ipcRenderer.invoke(IPC.LM_FAST_REFL_GENERATE, input, params),
  lmRhoFit: (input: BoxReflSettingsInput, params: number[]) =>
    ipcRenderer.invoke(IPC.LM_RHO_FIT, input, params),
  lmRhoGenerate: (input: BoxReflSettingsInput, params: number[]) =>
    ipcRenderer.invoke(IPC.LM_RHO_GENERATE, input, params),
  lmStochFit: (input: BoxReflSettingsInput, params: number[]) =>
    ipcRenderer.invoke(IPC.LM_STOCH_FIT, input, params),

  // File system
  openFile: (filters?: Electron.FileFilter[]) => ipcRenderer.invoke(IPC.FS_OPEN_FILE, filters),
  saveFile: (defaultPath: string, content: string) =>
    ipcRenderer.invoke(IPC.FS_SAVE_FILE, defaultPath, content),

  // Progress events (main → renderer)
  onFitProgress: (callback: (data: unknown) => void) => {
    ipcRenderer.on(IPC.FIT_PROGRESS, (_event, data) => callback(data));
    return () => ipcRenderer.removeAllListeners(IPC.FIT_PROGRESS);
  },
  onFitComplete: (callback: (data: unknown) => void) => {
    ipcRenderer.on(IPC.FIT_COMPLETE, (_event, data) => callback(data));
    return () => ipcRenderer.removeAllListeners(IPC.FIT_COMPLETE);
  },

  // Settings events (main → renderer)
  onSettingsReset: (callback: () => void) => {
    ipcRenderer.on(IPC.SETTINGS_RESET, () => callback());
    return () => ipcRenderer.removeAllListeners(IPC.SETTINGS_RESET);
  },
});
