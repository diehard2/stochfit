import React, { useEffect, useState } from 'react';
import { useDataStore } from '../../stores/data-store';
import { useSettingsStore } from '../../stores/settings-store';
import { useBoxModelStore } from '../../stores/box-model-store';
import { BoxParameterTable, type BoxRow } from '../shared/BoxParameterTable';
import type { BoxReflSettingsInput, RhoGenerateResult, StochFitResult } from '../../../main/native/levmar-api';

// ── Param helpers ────────────────────────────────────────────────────────────

function defaultRow(): BoxRow {
  return { length: 15.0, rho: 0.5, sigma: 3.0 };
}

function defaultRows(n: number): BoxRow[] {
  return Array.from({ length: n }, defaultRow);
}

function buildFastReflParams(subRough: number, rows: BoxRow[], normFactor: number, oneSigma: boolean): number[] {
  const result = [subRough];
  for (const row of rows) {
    result.push(row.length, row.rho);
    if (!oneSigma) result.push(row.sigma);
  }
  result.push(normFactor);
  return result;
}

function parseFastReflParams(
  params: number[],
  boxes: number,
  oneSigma: boolean,
): { subRough: number; rows: BoxRow[]; normFactor: number } {
  const subRough = params[0];
  const rows: BoxRow[] = [];
  let idx = 1;
  for (let i = 0; i < boxes; i++) {
    const length = params[idx++];
    const rho = params[idx++];
    const sigma = oneSigma ? 0 : (params[idx++] ?? 0);
    rows.push({ length, rho, sigma });
  }
  const normFactor = params[idx] ?? 1.0;
  return { subRough, rows, normFactor };
}

function makeBounds(params: number[]) {
  const ul = params.map((v) => (Math.abs(v) > 0 ? Math.abs(v) * 3 : 10));
  const ll = params.map((v) => (v >= 0 ? 0 : v * 3));
  const percs = params.map(() => 10);
  return { ul, ll, percs };
}

// Convert FastReflFit params to RhoFit layout for EDP generation:
// FastRefl: [subRough, len, rho, sig, ..., normFactor]
// Rho:      [subRough, zOffset=0, len, rho, sig, ...]
function buildRhoParamsFromBoxRows(subRough: number, rows: BoxRow[], oneSigma: boolean): number[] {
  const result = [subRough, 0];
  for (const row of rows) {
    result.push(row.length, row.rho);
    if (!oneSigma) result.push(row.sigma);
  }
  return result;
}

// Build a z-range covering all boxes plus padding on each end
function makeZRange(rows: BoxRow[]): number[] {
  const totalLen = rows.reduce((s, r) => s + Math.max(0, r.length), 0);
  const padding = 30;
  const step = 1;
  const count = Math.ceil((totalLen + 2 * padding) / step) + 1;
  return Array.from({ length: count }, (_, i) => -padding + i * step);
}

// ── Component ────────────────────────────────────────────────────────────────

export function BoxModelPanel() {
  const { data } = useDataStore();
  const { settings } = useSettingsStore();
  const { solutions, activeIndex, setSolutions, setActiveIndex, setGenRefl, setGenEDP } = useBoxModelStore();

  const [boxes, setBoxesState] = useState(3);
  const [subRough, setSubRough] = useState(3.0);
  const [normFactor, setNormFactor] = useState(1.0);
  const [oneSigma, setOneSigma] = useState(false);
  const [boxRows, setBoxRows] = useState<BoxRow[]>(defaultRows(3));
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const autoGenTimer = React.useRef<ReturnType<typeof setTimeout> | null>(null);

  // Sync row count when boxes changes
  useEffect(() => {
    setBoxRows((prev) => {
      if (prev.length === boxes) return prev;
      if (boxes > prev.length) {
        return [...prev, ...Array.from({ length: boxes - prev.length }, defaultRow)];
      }
      return prev.slice(0, boxes);
    });
  }, [boxes]);

  // Auto-generate reflectivity + EDP whenever params change (debounced)
  useEffect(() => {
    if (!data) return;
    if (autoGenTimer.current) clearTimeout(autoGenTimer.current);
    autoGenTimer.current = setTimeout(async () => {
      try {
        const reflParams = buildFastReflParams(subRough, boxRows, normFactor, oneSigma);
        const { ul, ll, percs } = makeBounds(reflParams);
        const baseInput: BoxReflSettingsInput = {
          directory: data.filePath.replace(/[^/\\]+$/, ''),
          q: data.q,
          refl: data.refl,
          reflError: data.reflError,
          qError: data.qError,
          ul,
          ll,
          paramPercs: percs,
          qPoints: data.q.length,
          oneSigma,
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

        // Generate reflectivity
        const refl = await window.api.lmFastReflGenerate(baseInput, reflParams) as number[];
        setGenRefl(refl);

        // Generate EDP: convert to RhoFit param layout and use a synthetic z-range
        const zRange = makeZRange(boxRows);
        const rhoParams = buildRhoParamsFromBoxRows(subRough, boxRows, oneSigma);
        const rhoInput: BoxReflSettingsInput = {
          ...baseInput,
          miedp: new Array(zRange.length).fill(0),
          zIncrement: zRange,
          zLength: zRange.length,
        };
        const { ul: ru, ll: rl, percs: rp } = makeBounds(rhoParams);
        rhoInput.ul = ru;
        rhoInput.ll = rl;
        rhoInput.paramPercs = rp;
        const edpRes = await window.api.lmRhoGenerate(rhoInput, rhoParams) as RhoGenerateResult;
        setGenEDP(edpRes.ed, edpRes.boxED, zRange);
      } catch {
        // silently ignore auto-gen errors
      }
    }, 500);
    return () => { if (autoGenTimer.current) clearTimeout(autoGenTimer.current); };
  }, [data, subRough, normFactor, boxRows, oneSigma, boxes, settings]);

  function buildInput(): BoxReflSettingsInput {
    const params = buildFastReflParams(subRough, boxRows, normFactor, oneSigma);
    const { ul, ll, percs } = makeBounds(params);
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
      oneSigma,
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

  async function handleFit() {
    if (!data) return;
    setBusy(true);
    setError(null);
    try {
      const params = buildFastReflParams(subRough, boxRows, normFactor, oneSigma);
      const input = buildInput();
      const res = await window.api.lmStochFit(input, params) as StochFitResult;

      const p = params.length;
      const sols = res.chiSquareArray.map((chi, i) => ({
        params: res.paramArray.slice(i * p, (i + 1) * p),
        chiSquare: chi,
      }));
      // Sort by chi-square ascending
      sols.sort((a, b) => a.chiSquare - b.chiSquare);
      setSolutions(sols, p);
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
      const reflParams = buildFastReflParams(subRough, boxRows, normFactor, oneSigma);
      const input = buildInput();
      const refl = await window.api.lmFastReflGenerate(input, reflParams) as number[];
      setGenRefl(refl);

      const zRange = makeZRange(boxRows);
      const rhoParams = buildRhoParamsFromBoxRows(subRough, boxRows, oneSigma);
      const { ul, ll, percs } = makeBounds(rhoParams);
      const rhoInput: BoxReflSettingsInput = {
        ...input,
        ul, ll, paramPercs: percs,
        miedp: new Array(zRange.length).fill(0),
        zIncrement: zRange,
        zLength: zRange.length,
      };
      const edpRes = await window.api.lmRhoGenerate(rhoInput, rhoParams) as RhoGenerateResult;
      setGenEDP(edpRes.ed, edpRes.boxED, zRange);
    } catch (e) {
      setError((e as Error).message);
    } finally {
      setBusy(false);
    }
  }

  async function handleApply(solutionParams: number[]) {
    const parsed = parseFastReflParams(solutionParams, boxes, oneSigma);
    setSubRough(parsed.subRough);
    setNormFactor(parsed.normFactor);
    setBoxRows(parsed.rows);

    // Auto-generate reflectivity + EDP for this solution
    if (!data) return;
    setBusy(true);
    setError(null);
    try {
      const reflParams = buildFastReflParams(parsed.subRough, parsed.rows, parsed.normFactor, oneSigma);
      const { ul, ll, percs } = makeBounds(reflParams);
      const input: BoxReflSettingsInput = { ...buildInput(), ul, ll, paramPercs: percs };
      const refl = await window.api.lmFastReflGenerate(input, reflParams) as number[];
      setGenRefl(refl);

      const zRange = makeZRange(parsed.rows);
      const rhoParams = buildRhoParamsFromBoxRows(parsed.subRough, parsed.rows, oneSigma);
      const { ul: ru, ll: rl, percs: rp } = makeBounds(rhoParams);
      const rhoInput: BoxReflSettingsInput = {
        ...input, ul: ru, ll: rl, paramPercs: rp,
        miedp: new Array(zRange.length).fill(0),
        zIncrement: zRange,
        zLength: zRange.length,
      };
      const edpRes = await window.api.lmRhoGenerate(rhoInput, rhoParams) as RhoGenerateResult;
      setGenEDP(edpRes.ed, edpRes.boxED, zRange);
    } catch (e) {
      setError((e as Error).message);
    } finally {
      setBusy(false);
    }
  }

  return (
    <div className="flex flex-col gap-4 p-4">
      <h2 className="text-sm font-semibold text-primary uppercase tracking-wider">Box Model</h2>

      {/* Global params */}
      <div className="grid grid-cols-2 gap-2">
        <LabeledInput label="Sub Rough (Å)" value={subRough} onChange={setSubRough} />
        <LabeledInput label="Norm Factor" value={normFactor} onChange={setNormFactor} />
      </div>

      <div className="grid grid-cols-2 gap-2">
        <div className="flex flex-col gap-1">
          <label className="text-xs text-secondary">Boxes</label>
          <input
            type="number"
            value={boxes}
            min={1}
            max={20}
            step={1}
            onChange={(e) => setBoxesState(parseInt(e.target.value) || 1)}
            className="h-7 px-2 text-xs bg-elevated border border-border rounded-input text-primary focus:outline-none focus:border-accent/50"
          />
        </div>
        <label className="flex items-center gap-1.5 mt-4 cursor-pointer">
          <input
            type="checkbox"
            checked={oneSigma}
            onChange={(e) => setOneSigma(e.target.checked)}
            className="rounded"
          />
          <span className="text-xs text-secondary">One σ</span>
        </label>
      </div>

      {/* Box table */}
      <BoxParameterTable
        rows={boxRows}
        onRowChange={(i, row) => setBoxRows((prev) => prev.map((r, idx) => idx === i ? row : r))}
        oneSigma={oneSigma}
      />

      {/* Action buttons */}
      <div className="flex gap-2">
        <button
          onClick={handleFit}
          disabled={busy || !data}
          className="flex-1 py-2 text-xs font-medium bg-accent/20 hover:bg-accent/30 text-accent rounded-input disabled:opacity-40 disabled:cursor-not-allowed transition-colors"
        >
          {busy ? 'Running…' : 'Fit'}
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

      {/* Solutions list */}
      {solutions.length > 0 && (
        <div className="flex flex-col gap-2">
          <div className="text-xs text-secondary">{solutions.length} solutions (ranked by χ²)</div>
          <div className="rounded-card bg-elevated border border-border overflow-hidden max-h-48 overflow-y-auto">
            {solutions.map((sol, i) => (
              <div
                key={i}
                className={`flex items-center justify-between px-3 py-1.5 text-xs border-b border-border/40 last:border-0 ${
                  i === activeIndex ? 'bg-accent/10' : ''
                }`}
              >
                <button
                  onClick={() => setActiveIndex(i)}
                  className={`flex-1 text-left ${i === activeIndex ? 'text-accent' : 'text-secondary hover:text-primary'}`}
                >
                  <span>{i + 1}</span>
                  <span className="font-mono ml-2">{sol.chiSquare.toExponential(3)}</span>
                </button>
                <button
                  onClick={() => { setActiveIndex(i); handleApply(sol.params); }}
                  disabled={busy}
                  className="ml-2 px-2 py-0.5 text-xs bg-accent/20 hover:bg-accent/30 text-accent rounded disabled:opacity-40 transition-colors"
                >
                  Apply
                </button>
              </div>
            ))}
          </div>

          {/* Selected solution params */}
          {solutions[activeIndex] && (
            <div className="rounded-card bg-elevated border border-border p-3">
              <div className="text-xs font-medium text-secondary mb-1.5">
                Solution {activeIndex + 1} — χ² {solutions[activeIndex].chiSquare.toExponential(3)}
              </div>
              <div className="font-mono text-xs text-primary grid grid-cols-2 gap-x-4 gap-y-0.5">
                {solutions[activeIndex].params.map((p, i) => (
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

function LabeledInput({ label, value, onChange }: { label: string; value: number; onChange: (v: number) => void }) {
  const [text, setText] = React.useState(String(value));
  const focused = React.useRef(false);

  React.useEffect(() => {
    if (!focused.current) setText(String(value));
  }, [value]);

  return (
    <div className="flex flex-col gap-1">
      <label className="text-xs text-secondary">{label}</label>
      <input
        type="text"
        inputMode="decimal"
        value={text}
        onFocus={() => { focused.current = true; }}
        onBlur={() => {
          focused.current = false;
          const v = parseFloat(text);
          setText(isNaN(v) ? String(value) : String(v));
        }}
        onChange={(e) => {
          setText(e.target.value);
          const v = parseFloat(e.target.value);
          if (!isNaN(v)) onChange(v);
        }}
        className="h-7 px-2 text-xs bg-elevated border border-border rounded-input text-primary focus:outline-none focus:border-accent/50"
      />
    </div>
  );
}
