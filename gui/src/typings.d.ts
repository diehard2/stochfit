// plotly.js-dist-min ships no .d.ts — point it at the @types/plotly.js declarations.
declare module 'plotly.js-dist-min' {
  import * as PlotlyType from 'plotly.js';
  export = PlotlyType;
}

import type { ReflData, StochFitOutput, FitResult, SAParams, SARunState, LMResult, RhoEDPResult, StochFitResult } from './renderer/lib/types';
import type { ReflSettingsInput } from './main/native/stochfit-api';
import type { BoxReflSettingsInput } from './main/native/levmar-api';

declare const __APP_VERSION__: string;

declare global {
  interface Window {
    api: {
      // Data file
      openDataFile: () => Promise<{ data: ReflData; savedOutput?: StochFitOutput } | null>;
      openFile: (filters?: Electron.FileFilter[]) => Promise<{ filePath: string; content: string } | null>;
      saveFile: (defaultPath: string, content: string) => Promise<boolean>;
      openPdf: (dir: string, baseName: string, data: Uint8Array) => Promise<string | null>;
      openExternal: (url: string) => Promise<void>;

      // StochFit SA lifecycle
      stochInit: (s: ReflSettingsInput, runState: SARunState | null) => Promise<void>;
      stochStart: (n: number) => Promise<void>;
      stochStop: () => Promise<void>;
      stochDestroy: () => Promise<void>;
      stochCancel: () => Promise<void>;

      // StochFit polling / state
      stochGetData: () => Promise<FitResult>;
      stochSAParams: () => Promise<SAParams>;
      stochGetRunState: (boxes: number) => Promise<SARunState>;

      // StochFit session I/O
      stochLoadOutput: (filePath: string) => Promise<StochFitOutput | null>;
      stochWriteOutput: (filePath: string, output: StochFitOutput) => Promise<void>;
      stochDeleteOutput: (filePath: string) => Promise<void>;

      // LevMar
      lmFastReflFit: (i: BoxReflSettingsInput, p: number[]) => Promise<LMResult>;
      lmFastReflGenerate: (i: BoxReflSettingsInput, p: number[]) => Promise<number[]>;
      lmRhoFit: (i: BoxReflSettingsInput, p: number[]) => Promise<LMResult>;
      lmRhoGenerate: (i: BoxReflSettingsInput, p: number[]) => Promise<RhoEDPResult>;
      lmStochFit: (i: BoxReflSettingsInput, p: number[]) => Promise<StochFitResult>;
    };
  }
}

export {};
