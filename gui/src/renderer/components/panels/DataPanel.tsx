import React from 'react';
import { useDataStore } from '../../stores/data-store';
import { parseReflData } from '../../lib/data-parser';

declare global {
  interface Window {
    api: {
      openFile: (filters?: Electron.FileFilter[]) => Promise<{ filePath: string; content: string } | null>;
      saveFile: (defaultPath: string, content: string) => Promise<boolean>;
      stochInit: (s: unknown, runState: unknown) => Promise<void>;
      stochStart: (n: number) => Promise<void>;
      stochStop: () => Promise<void>;
      stochDestroy: () => Promise<void>;
      stochCancel: () => Promise<void>;
      stochGetData: () => Promise<unknown>;
      stochGetRunState: (boxes: number) => Promise<unknown>;
      stochLoadSession: (filePath: string) => Promise<unknown>;
      stochWriteSession: (filePath: string, session: unknown) => Promise<void>;
      stochDeleteSession: (filePath: string) => Promise<void>;
      stochArraySizes: () => Promise<{ rhoSize: number; reflSize: number }>;
      stochWarmedUp: () => Promise<boolean>;
      stochSAParams: () => Promise<unknown>;
      lmFastReflFit: (i: unknown, p: number[]) => Promise<unknown>;
      lmFastReflGenerate: (i: unknown, p: number[]) => Promise<number[]>;
      lmRhoFit: (i: unknown, p: number[]) => Promise<unknown>;
      lmRhoGenerate: (i: unknown, p: number[]) => Promise<unknown>;
      lmStochFit: (i: unknown, p: number[]) => Promise<unknown>;
      onFitProgress: (cb: (d: unknown) => void) => () => void;
      onFitComplete: (cb: (d: unknown) => void) => () => void;
      onSettingsReset: (cb: () => void) => () => void;
      stochGpuAvailable: () => Promise<boolean>;
    };
  }
}

export function DataPanel() {
  const { data, setData } = useDataStore();

  async function handleOpen() {
    const result = await window.api.openFile([
      { name: 'Data files', extensions: ['txt', 'dat', 'csv'] },
      { name: 'All files', extensions: ['*'] },
    ]);
    if (!result) return;
    try {
      const parsed = parseReflData(result.content, result.filePath);
      setData(parsed);
    } catch (e) {
      alert(`Failed to parse file: ${(e as Error).message}`);
    }
  }

  return (
    <div className="flex flex-col gap-4 p-4">
      <div className="flex items-center justify-between">
        <h2 className="text-sm font-semibold text-primary uppercase tracking-wider">Data File</h2>
        <button
          onClick={handleOpen}
          className="px-3 py-1.5 text-xs font-medium bg-accent/20 hover:bg-accent/30 text-accent rounded-input transition-colors"
        >
          Open File
        </button>
      </div>

      {data ? (
        <div className="flex flex-col gap-2">
          <div className="text-xs text-secondary font-mono truncate" title={data.filePath}>
            {data.fileName}
          </div>
          <div className="text-xs text-secondary">
            {data.q.length} points &middot; Q: [{data.q[0].toExponential(3)} &hellip; {data.q[data.q.length - 1].toExponential(3)}] Å⁻¹
          </div>

          {/* Mini data table — first 8 rows */}
          <div className="mt-2 rounded-card bg-elevated border border-border overflow-hidden">
            <table className="w-full text-xs font-mono">
              <thead>
                <tr className="border-b border-border text-secondary">
                  <th className="px-2 py-1 text-left">Q</th>
                  <th className="px-2 py-1 text-left">R</th>
                  <th className="px-2 py-1 text-left">σR</th>
                </tr>
              </thead>
              <tbody>
                {data.q.slice(0, 8).map((q, i) => (
                  <tr key={i} className="border-b border-border/50 hover:bg-surface/50">
                    <td className="px-2 py-0.5">{q.toExponential(3)}</td>
                    <td className="px-2 py-0.5">{data.refl[i].toExponential(3)}</td>
                    <td className="px-2 py-0.5">{data.reflError[i].toExponential(3)}</td>
                  </tr>
                ))}
                {data.q.length > 8 && (
                  <tr>
                    <td colSpan={3} className="px-2 py-1 text-secondary text-center">
                      + {data.q.length - 8} more rows
                    </td>
                  </tr>
                )}
              </tbody>
            </table>
          </div>
        </div>
      ) : (
        <div
          onClick={handleOpen}
          className="flex flex-col items-center justify-center gap-2 h-32 rounded-card border-2 border-dashed border-border hover:border-accent/50 cursor-pointer transition-colors"
        >
          <span className="text-2xl opacity-30">📂</span>
          <span className="text-xs text-secondary">Click to open a reflectivity data file</span>
        </div>
      )}
    </div>
  );
}
