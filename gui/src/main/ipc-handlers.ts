import { ipcMain, dialog } from 'electron';
import { IPC } from '../shared/ipc-channels';
import {
  stochInit,
  stochStart,
  stochCancel,
  stochGetData,
  stochArraySizes,
  stochWarmedUp,
  stochSAParams,
  stochGpuAvailable,
  type ReflSettingsInput,
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
  ipcMain.handle(IPC.STOCH_INIT, (_event, settings: ReflSettingsInput) => {
    return wrap('STOCH_INIT', () => stochInit(settings));
  });

  ipcMain.handle(IPC.STOCH_START, (_event, iterations: number) => {
    return wrap('STOCH_START', () => stochStart(iterations));
  });

  ipcMain.handle(IPC.STOCH_CANCEL, () => {
    return wrap('STOCH_CANCEL', () => stochCancel());
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
