import React, { useState } from 'react';
import { useDataStore } from '../../stores/data-store';
import { useSettingsStore } from '../../stores/settings-store';
import { useFitStore } from '../../stores/fit-store';
import type { BoxReflSettingsInput, LMFitResult } from '../../../main/native/levmar-api';
import type { ReflData } from '../../lib/types';
import type { ModelSettings } from '../../lib/types';

function buildBoxInput(
  data: ReflData,
  settings: ModelSettings,
  boxes: number,
  ul: number[],
  ll: number[],
  paramPercs: number[],
  miedp?: number[],
  zIncrement?: number[],
  zLength?: number
): BoxReflSettingsInput {
  return {
    directory: data.filePath.replace(/[^/\\]+$/, ''),
    q: data.q,
    refl: data.refl,
    reflError: data.reflError,
    qError: data.qError,
    ul,
    ll,
    paramPercs,
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
    miedp,
    zIncrement,
    zLength,
  };
}

function makeDefaultParams(boxes: number): { params: number[]; ul: number[]; ll: number[]; percs: number[] } {
  // 3 params per box (rho, sigma, thickness) + 1 for substrate sigma
  const n = boxes * 3 + 1;
  const params = new Array(n).fill(0).map((_, i) => {
    if (i % 3 === 0) return 1.0;   // normalized rho
    if (i % 3 === 1) return 3.0;   // sigma
    return 15.0;                    // thickness
  });
  const ul = params.map((v) => v * 2);
  const ll = params.map((v) => Math.max(0, v * 0.1));
  const percs = params.map(() => 10);
  return { params, ul, ll, percs };
}

export function RhoModelingPanel() {
  const { data } = useDataStore();
  const { settings } = useSettingsStore();
  const fitResult = useFitStore((s) => s.result);

  const [boxes, setBoxes] = useState(settings.boxes);
  const [lmResult, setLmResult] = useState<LMFitResult | null>(null);
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const { params, ul, ll, percs } = makeDefaultParams(boxes);

  async function handleRhoFit() {
    if (!data) return;
    setBusy(true);
    setError(null);
    try {
      const miedp = fitResult?.rho ?? [];
      const zInc = fitResult?.zRange?.map((_, i, arr) => i === 0 ? arr[1] - arr[0] : arr[i] - arr[i - 1]) ?? [];
      const zLen = miedp.length;
      const input = buildBoxInput(data, settings, boxes, ul, ll, percs, miedp, zInc, zLen);
      const result = await window.api.lmRhoFit(input, params) as LMFitResult;
      setLmResult(result);
    } catch (e) {
      setError((e as Error).message);
    } finally {
      setBusy(false);
    }
  }

  async function handleRhoGenerate() {
    if (!data || !lmResult) return;
    setBusy(true);
    setError(null);
    try {
      const miedp = fitResult?.rho ?? [];
      const zInc = fitResult?.zRange?.map((_, i, arr) => i === 0 ? arr[1] - arr[0] : arr[i] - arr[i - 1]) ?? [];
      const zLen = miedp.length;
      const input = buildBoxInput(data, settings, boxes, ul, ll, percs, miedp, zInc, zLen);
      await window.api.lmRhoGenerate(input, lmResult.parameters);
    } catch (e) {
      setError((e as Error).message);
    } finally {
      setBusy(false);
    }
  }

  return (
    <div className="flex flex-col gap-4 p-4">
      <h2 className="text-sm font-semibold text-primary uppercase tracking-wider">Rho Modeling</h2>

      {!fitResult && (
        <div className="text-xs text-secondary rounded-card bg-elevated border border-border p-3">
          Run MI fitting first to populate the EDP.
        </div>
      )}

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

      <div className="flex gap-2">
        <button
          onClick={handleRhoFit}
          disabled={busy || !data || !fitResult}
          className="flex-1 py-2 text-xs font-medium bg-accent/20 hover:bg-accent/30 text-accent rounded-input disabled:opacity-40 disabled:cursor-not-allowed transition-colors"
        >
          {busy ? 'Fitting…' : 'Rho Fit'}
        </button>
        <button
          onClick={handleRhoGenerate}
          disabled={busy || !lmResult}
          className="flex-1 py-2 text-xs font-medium bg-elevated hover:bg-surface text-secondary rounded-input disabled:opacity-40 disabled:cursor-not-allowed transition-colors border border-border"
        >
          Generate
        </button>
      </div>

      {error && (
        <div className="text-xs text-destructive rounded-card bg-destructive/10 border border-destructive/20 p-2">
          {error}
        </div>
      )}

      {lmResult && (
        <div className="rounded-card bg-elevated border border-border p-3 flex flex-col gap-1.5">
          <div className="text-xs font-medium text-secondary mb-1">LM Result</div>
          <div className="flex justify-between text-xs">
            <span className="text-secondary">Iterations</span>
            <span className="font-mono text-primary">{lmResult.info[5]?.toFixed(0) ?? '—'}</span>
          </div>
          <div className="flex justify-between text-xs">
            <span className="text-secondary">‖e‖²</span>
            <span className="font-mono text-primary">{lmResult.info[1]?.toExponential(3) ?? '—'}</span>
          </div>
          <div className="text-xs text-secondary mt-1 font-medium">Parameters</div>
          <div className="font-mono text-xs text-primary grid grid-cols-2 gap-x-4 gap-y-0.5">
            {lmResult.parameters.map((p, i) => (
              <span key={i}>[{i}] {p.toFixed(4)}</span>
            ))}
          </div>
        </div>
      )}
    </div>
  );
}
