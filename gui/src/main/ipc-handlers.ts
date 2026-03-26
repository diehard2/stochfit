import { ipcMain, dialog, shell } from 'electron';
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
  readOutputFile,
  writeOutputFile,
  outputFilePath,
  type ReflSettingsInput,
  type StochRunStateOutput,
  type StochFitOutput,
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
import path from 'path';
import { parseDataFile } from './parsers';

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

  ipcMain.handle(IPC.STOCH_LOAD_OUTPUT, (_event, filePath: string) => {
    return wrap('STOCH_LOAD_OUTPUT', () => readOutputFile(filePath));
  });

  ipcMain.handle(IPC.STOCH_WRITE_OUTPUT, (_event, filePath: string, output: StochFitOutput) => {
    return wrap('STOCH_WRITE_OUTPUT', () => writeOutputFile(filePath, output));
  });

  ipcMain.handle(IPC.STOCH_DELETE_OUTPUT, (_event, filePath: string) => {
    return wrap('STOCH_DELETE_OUTPUT', () => {
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
  ipcMain.handle(IPC.FS_OPEN_DATA_FILE, async (_event) => {
    const result = await dialog.showOpenDialog({
      properties: ['openFile'],
      filters: [
        { name: 'All supported', extensions: ['txt', 'dat', 'csv', 'ort', 'nxs', 'h5', 'hdf5', 'hdf'] },
        { name: 'Text data', extensions: ['txt', 'dat', 'csv'] },
        { name: 'ORSO (.ort)', extensions: ['ort'] },
        { name: 'NeXus / HDF5', extensions: ['nxs', 'h5', 'hdf5', 'hdf'] },
        { name: 'All files', extensions: ['*'] },
      ],
    });
    if (result.canceled || result.filePaths.length === 0) return null;
    return parseDataFile(result.filePaths[0]);
  });

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

  ipcMain.handle(IPC.FS_OPEN_PDF, async (_event, dir: string, baseName: string, data: Uint8Array) => {
    // Auto-increment filename so we never overwrite a previous report (mirrors C# behavior)
    let filePath = path.join(dir, `${baseName}.pdf`);
    let counter = 1;
    while (fs.existsSync(filePath)) {
      filePath = path.join(dir, `${baseName}${counter++}.pdf`);
    }
    fs.writeFileSync(filePath, Buffer.from(data));
    await shell.openPath(filePath);
    return filePath;
  });

  ipcMain.handle(IPC.SHELL_OPEN_EXTERNAL, (_event, url: string) => {
    shell.openExternal(url);
  });
}
