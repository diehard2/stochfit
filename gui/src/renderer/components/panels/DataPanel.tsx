import React from 'react';
import { useDataStore } from '../../stores/data-store';
import { useSettingsStore } from '../../stores/settings-store';
import { useFitStore } from '../../stores/fit-store';
import { useMiEdpStore } from '../../stores/mi-edp-store';
import { useUiStore } from '../../stores/ui-store';
import type { ReflData, StochFitOutput, ModelSettings } from '../../lib/types';

interface OpenDataResult {
  data: ReflData;
  savedOutput?: StochFitOutput;
}

declare global {
  interface Window {
    api: {
      openDataFile: () => Promise<OpenDataResult | null>;
      openFile: (filters?: Electron.FileFilter[]) => Promise<{ filePath: string; content: string } | null>;
      saveFile: (defaultPath: string, content: string) => Promise<boolean>;
      stochInit: (s: unknown, runState: unknown) => Promise<void>;
      stochStart: (n: number) => Promise<void>;
      stochStop: () => Promise<void>;
      stochDestroy: () => Promise<void>;
      stochCancel: () => Promise<void>;
      stochGetData: () => Promise<unknown>;
      stochGetRunState: (boxes: number) => Promise<unknown>;
      stochLoadOutput: (filePath: string) => Promise<unknown>;
      stochWriteOutput: (filePath: string, output: unknown) => Promise<void>;
      stochDeleteOutput: (filePath: string) => Promise<void>;
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
      openPdf: (dir: string, baseName: string, data: Uint8Array) => Promise<string | null>;
    };
  }
}

export function DataPanel() {
  const { data, setData } = useDataStore();
  const { restore: restoreSettings } = useSettingsStore();
  const { setResult } = useFitStore();
  const { setBoxes, setSubRough, setZOffset, setOneSigma, setBoxRows, setLmResult } = useMiEdpStore();
  const { showToast } = useUiStore();

  async function handleOpen() {
    try {
      const result = await window.api.openDataFile();
      if (!result) return;
      setData(result.data);

      if (result.savedOutput) {
        const out = result.savedOutput;
        // Restore settings
        restoreSettings(out.settings as unknown as Partial<ModelSettings>);
        // Restore fit result for chart display
        setResult({ ...out.fitResult, isFinished: true });
        // Restore box model
        if (out.boxModel) {
          setBoxes(out.boxModel.boxes);
          setSubRough(out.boxModel.subRough);
          setZOffset(out.boxModel.zOffset);
          setOneSigma(out.boxModel.oneSigma);
          setBoxRows(out.boxModel.boxRows);
          if (out.boxModel.lmResult) {
            setLmResult(out.boxModel.lmResult);
          }
        }
        showToast('Loaded previous fit results');
      }
    } catch (e) {
      alert(`Failed to load file: ${(e as Error).message}`);
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
          <div className="text-xs text-secondary">{data.q.length} points</div>

          {/* File metadata (ORSO / NeXus headers) */}
          {data.metadata && data.metadata.length > 0 && (
            <div className="rounded-card bg-elevated border border-border p-2 flex flex-col gap-0.5">
              {data.metadata.map(({ label, value }) => (
                <div key={label} className="flex gap-2 text-xs leading-relaxed">
                  <span className="text-secondary shrink-0 w-20">{label}</span>
                  <span className="text-primary font-mono truncate" title={value}>{value}</span>
                </div>
              ))}
            </div>
          )}

          {/* Scrollable data table — all rows */}
          {(() => {
            const hasQErr = data.qError.some(v => v !== 0);
            const fmt = (v: number) => v.toExponential(hasQErr ? 2 : 3);
            return (
              <div className="mt-1 rounded-card bg-elevated border border-border overflow-hidden">
                <div className="overflow-y-auto max-h-52">
                  <table className="w-full text-xs font-mono">
                    <thead className="sticky top-0 bg-elevated z-10">
                      <tr className="border-b border-border text-secondary">
                        <th className="px-2 py-1 text-left">Q</th>
                        <th className="px-2 py-1 text-left">R</th>
                        <th className="px-2 py-1 text-left">σR</th>
                        {hasQErr && <th className="px-2 py-1 text-left">δQ</th>}
                      </tr>
                    </thead>
                    <tbody>
                      {data.q.map((q, i) => (
                        <tr key={i} className="border-b border-border/50 hover:bg-surface/50">
                          <td className="px-2 py-0.5">{fmt(q)}</td>
                          <td className="px-2 py-0.5">{fmt(data.refl[i])}</td>
                          <td className="px-2 py-0.5">{fmt(data.reflError[i])}</td>
                          {hasQErr && <td className="px-2 py-0.5">{fmt(data.qError[i])}</td>}
                        </tr>
                      ))}
                    </tbody>
                  </table>
                </div>
              </div>
            );
          })()}
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
