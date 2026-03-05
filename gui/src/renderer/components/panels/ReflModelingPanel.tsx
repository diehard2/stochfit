import React, { useState } from 'react';
import { useDataStore } from '../../stores/data-store';
import { useSettingsStore } from '../../stores/settings-store';
import type { BoxReflSettingsInput, LMFitResult } from '../../../main/native/levmar-api';

function makeDefaultReflParams(boxes: number) {
  // SLD + sigma + thickness per box, plus substrate sigma
  const n = boxes * 3 + 1;
  const params = Array.from({ length: n }, (_, i) => {
    if (i % 3 === 0) return 5.0;   // SLD
    if (i % 3 === 1) return 3.0;   // sigma
    return 20.0;                    // thickness
  });
  const ul = params.map((v) => v * 3);
  const ll = params.map((v) => Math.max(0, v * 0.1));
  const percs = params.map(() => 10);
  return { params, ul, ll, percs };
}

export function ReflModelingPanel() {
  const { data } = useDataStore();
  const { settings } = useSettingsStore();

  const [boxes, setBoxes] = useState(settings.boxes);
  const [lmResult, setLmResult] = useState<LMFitResult | null>(null);
  const [genRefl, setGenRefl] = useState<number[] | null>(null);
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const { params, ul, ll, percs } = makeDefaultReflParams(boxes);

  function buildInput(): BoxReflSettingsInput {
    return {
      directory: data!.filePath.replace(/[^/\\]+$/, ''),
      q: data!.q,
      refl: data!.refl,
      reflError: data!.reflError,
      qError: data!.qError,
      ul,
      ll,
      paramPercs: percs,
      qPoints: data!.q.length,
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
      lowQOffset: settings.critEdgeOffset,
      highQOffset: settings.highQOffset,
      iterations: 1000,
    };
  }

  async function handleReflFit() {
    if (!data) return;
    setBusy(true);
    setError(null);
    try {
      const result = await window.api.lmFastReflFit(buildInput(), params) as LMFitResult;
      setLmResult(result);
    } catch (e) {
      setError((e as Error).message);
    } finally {
      setBusy(false);
    }
  }

  async function handleGenerate() {
    if (!data) return;
    setBusy(true);
    setError(null);
    try {
      const p = lmResult?.parameters ?? params;
      const refl = await window.api.lmFastReflGenerate(buildInput(), p) as number[];
      setGenRefl(refl);
    } catch (e) {
      setError((e as Error).message);
    } finally {
      setBusy(false);
    }
  }

  return (
    <div className="flex flex-col gap-4 p-4">
      <h2 className="text-sm font-semibold text-primary uppercase tracking-wider">Refl Modeling</h2>

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
          onClick={handleReflFit}
          disabled={busy || !data}
          className="flex-1 py-2 text-xs font-medium bg-accent/20 hover:bg-accent/30 text-accent rounded-input disabled:opacity-40 disabled:cursor-not-allowed transition-colors"
        >
          {busy ? 'Fitting…' : 'Refl Fit'}
        </button>
        <button
          onClick={handleGenerate}
          disabled={busy || !data}
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
            <span className="text-secondary">‖e‖² final</span>
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

      {genRefl && (
        <div className="text-xs text-secondary">
          Generated {genRefl.length} reflectivity points.
        </div>
      )}
    </div>
  );
}
