import React, { useState } from 'react';
import { useDataStore } from '../../stores/data-store';
import { useSettingsStore } from '../../stores/settings-store';
import type { BoxReflSettingsInput, StochFitResult } from '../../../main/native/levmar-api';

export function StochResultsPanel() {
  const { data } = useDataStore();
  const { settings } = useSettingsStore();
  const [boxes, setBoxes] = useState(settings.boxes);
  const [result, setResult] = useState<StochFitResult | null>(null);
  const [selectedIdx, setSelectedIdx] = useState(0);
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const defaultParams = Array.from({ length: boxes * 3 + 1 }, (_, i) =>
    i % 3 === 0 ? 1.0 : i % 3 === 1 ? 3.0 : 15.0
  );

  async function handleStochFit() {
    if (!data) return;
    setBusy(true);
    setError(null);
    try {
      const p = defaultParams.length;
      const ul = defaultParams.map((v) => v * 3);
      const ll = defaultParams.map((v) => Math.max(0, v * 0.1));
      const percs = defaultParams.map(() => 10);

      const input: BoxReflSettingsInput = {
        directory: data.filePath.replace(/[^/\\]+$/, ''),
        q: data.q,
        refl: data.refl,
        reflError: data.reflError,
        qError: data.qError,
        ul,
        ll,
        paramPercs: percs,
        qPoints: data.q.length,
        oneSigma: false,
        writeFiles: false,
        subSLD: settings.subSLD,
        supSLD: settings.supSLD,
        boxes,
        wavelength: settings.wavelength,
        qSpread: settings.qErr,
        forcenorm: settings.forcenorm,
        impNorm: settings.impnorm,
        fitFunc: 0,
        lowQOffset: 0,
        highQOffset: settings.highQOffset,
        iterations: 1000,
      };

      const res = await window.api.lmStochFit(input, defaultParams) as StochFitResult;
      setResult(res);
      setSelectedIdx(0);
    } catch (e) {
      setError((e as Error).message);
    } finally {
      setBusy(false);
    }
  }

  const selectedParams = result
    ? result.paramArray.slice(selectedIdx * (boxes * 3 + 1), (selectedIdx + 1) * (boxes * 3 + 1))
    : [];

  return (
    <div className="flex flex-col gap-4 p-4">
      <h2 className="text-sm font-semibold text-primary uppercase tracking-wider">Stochastic Solutions</h2>

      <div className="flex flex-col gap-1.5">
        <label className="text-xs text-secondary">Boxes</label>
        <input
          type="number"
          value={boxes}
          min={1}
          max={20}
          onChange={(e) => setBoxes(parseInt(e.target.value) || 1)}
          className="h-7 px-2 text-xs bg-elevated border border-border rounded-input text-primary focus:outline-none focus:border-accent/50"
        />
      </div>

      <button
        onClick={handleStochFit}
        disabled={busy || !data}
        className="w-full py-2 text-xs font-medium bg-accent/20 hover:bg-accent/30 text-accent rounded-input disabled:opacity-40 disabled:cursor-not-allowed transition-colors"
      >
        {busy ? 'Running…' : 'Run Stochastic Fit'}
      </button>

      {error && (
        <div className="text-xs text-destructive rounded-card bg-destructive/10 border border-destructive/20 p-2">
          {error}
        </div>
      )}

      {result && result.paramArraySize > 0 && (
        <div className="flex flex-col gap-2">
          <div className="text-xs text-secondary">{result.paramArraySize} solutions found</div>

          {/* Solution list */}
          <div className="rounded-card bg-elevated border border-border overflow-hidden max-h-48 overflow-y-auto">
            {result.chiSquareArray.map((chi, i) => (
              <button
                key={i}
                onClick={() => setSelectedIdx(i)}
                className={`w-full flex justify-between px-3 py-1.5 text-xs transition-colors ${
                  i === selectedIdx
                    ? 'bg-accent/15 text-accent'
                    : 'text-secondary hover:bg-surface'
                }`}
              >
                <span>Solution {i + 1}</span>
                <span className="font-mono">{chi.toExponential(3)}</span>
              </button>
            ))}
          </div>

          {/* Selected solution params */}
          {selectedParams.length > 0 && (
            <div className="rounded-card bg-elevated border border-border p-3">
              <div className="text-xs font-medium text-secondary mb-1.5">
                Solution {selectedIdx + 1} — χ² {result.chiSquareArray[selectedIdx]?.toExponential(3)}
              </div>
              <div className="font-mono text-xs text-primary grid grid-cols-2 gap-x-4 gap-y-0.5">
                {selectedParams.map((p, i) => (
                  <span key={i}>[{i}] {p.toFixed(4)}</span>
                ))}
              </div>
            </div>
          )}
        </div>
      )}
    </div>
  );
}
