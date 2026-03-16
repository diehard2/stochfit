import { ipcMain, dialog } from 'electron';
import { IPC } from '../shared/ipc-channels';
import {
  stochInit,
  stochStart,
  stochStop,
  stochDestroy,
  stochCancel,
  stochGetData,
  stochGetRunState,
  stochArraySizes,
  stochWarmedUp,
  stochSAParams,
  stochGpuAvailable,
  readSessionFile,
  writeSessionFile,
  type ReflSettingsInput,
  type StochRunStateOutput,
  type StochSessionFile,
} from './native/stochfit-api';
import {
  levmarFastReflFit,
  levmarFastReflGenerate,
  levmarRhoFit,
  levmarRhoGenerate,
  levmarStochFit,
  type BoxReflSettingsInput,
} from './native/levmar-api';
import fs from 'fs';

function wrap<T>(name: string, fn: () => T): T {
  try {
    return fn();
  } catch (err) {
    console.error(`[IPC] ${name} threw:`, err);
    throw err;
  }
}

export function registerIpcHandlers(): void {
  // ── StochFit ────────────────────────────────────────────────────────────
  ipcMain.handle(IPC.STOCH_INIT, (_event, settings: ReflSettingsInput, runState: StochRunStateOutput | null) => {
    return wrap('STOCH_INIT', () => stochInit(settings, runState));
  });

  ipcMain.handle(IPC.STOCH_START, (_event, iterations: number) => {
    return wrap('STOCH_START', () => stochStart(iterations));
  });

  ipcMain.handle(IPC.STOCH_STOP, () => {
    return wrap('STOCH_STOP', () => stochStop());
  });

  ipcMain.handle(IPC.STOCH_DESTROY, () => {
    return wrap('STOCH_DESTROY', () => stochDestroy());
  });

  ipcMain.handle(IPC.STOCH_CANCEL, () => {
    return wrap('STOCH_CANCEL', () => stochCancel());
  });

  ipcMain.handle(IPC.STOCH_GET_RUN_STATE, (_event, boxes: number) => {
    return wrap('STOCH_GET_RUN_STATE', () => stochGetRunState(boxes));
  });

  ipcMain.handle(IPC.STOCH_LOAD_SESSION, (_event, filePath: string) => {
    return wrap('STOCH_LOAD_SESSION', () => readSessionFile(filePath));
  });

  ipcMain.handle(IPC.STOCH_WRITE_SESSION, (_event, filePath: string, session: StochSessionFile) => {
    return wrap('STOCH_WRITE_SESSION', () => writeSessionFile(filePath, session));
  });

  ipcMain.handle(IPC.STOCH_DELETE_SESSION, (_event, filePath: string) => {
    return wrap('STOCH_DELETE_SESSION', () => {
      try { fs.unlinkSync(filePath); } catch { /* already gone */ }
    });
  });

  ipcMain.handle(IPC.STOCH_GET_DATA, () => {
    return wrap('STOCH_GET_DATA', () => stochGetData());
  });

  ipcMain.handle(IPC.STOCH_ARRAY_SIZES, () => {
    return wrap('STOCH_ARRAY_SIZES', () => stochArraySizes());
  });

  ipcMain.handle(IPC.STOCH_WARMED_UP, () => {
    return wrap('STOCH_WARMED_UP', () => stochWarmedUp());
  });

  ipcMain.handle(IPC.STOCH_SA_PARAMS, () => {
    return wrap('STOCH_SA_PARAMS', () => stochSAParams());
  });

  ipcMain.handle(IPC.STOCH_GPU_AVAILABLE, () => {
    return wrap('STOCH_GPU_AVAILABLE', () => stochGpuAvailable());
  });

  // ── LevMar ──────────────────────────────────────────────────────────────
  ipcMain.handle(IPC.LM_FAST_REFL_FIT, (_event, input: BoxReflSettingsInput, params: number[]) => {
    return levmarFastReflFit(input, params);
  });

  ipcMain.handle(IPC.LM_FAST_REFL_GENERATE, (_event, input: BoxReflSettingsInput, params: number[]) => {
    return levmarFastReflGenerate(input, params);
  });

  ipcMain.handle(IPC.LM_RHO_FIT, (_event, input: BoxReflSettingsInput, params: number[]) => {
    return levmarRhoFit(input, params);
  });

  ipcMain.handle(IPC.LM_RHO_GENERATE, (_event, input: BoxReflSettingsInput, params: number[]) => {
    return levmarRhoGenerate(input, params);
  });

  ipcMain.handle(IPC.LM_STOCH_FIT, (_event, input: BoxReflSettingsInput, params: number[]) => {
    return levmarStochFit(input, params);
  });

  // ── File System ──────────────────────────────────────────────────────────
  ipcMain.handle(IPC.FS_OPEN_FILE, async (_event, filters?: Electron.FileFilter[]) => {
    const result = await dialog.showOpenDialog({
      properties: ['openFile'],
      filters: filters ?? [{ name: 'Data files', extensions: ['txt', 'dat', 'csv', '*'] }],
    });
    if (result.canceled || result.filePaths.length === 0) return null;
    const filePath = result.filePaths[0];
    const content = fs.readFileSync(filePath, 'utf-8');
    return { filePath, content };
  });

  ipcMain.handle(IPC.FS_SAVE_FILE, async (_event, defaultPath: string, content: string) => {
    const result = await dialog.showSaveDialog({ defaultPath });
    if (result.canceled || !result.filePath) return false;
    fs.writeFileSync(result.filePath, content, 'utf-8');
    return true;
  });
}
