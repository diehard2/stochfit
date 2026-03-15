import React, { useCallback, useEffect, useState } from 'react';
import { useFitStore } from '../../stores/fit-store';
import { useDataStore } from '../../stores/data-store';
import { useSettingsStore } from '../../stores/settings-store';
import { POLLING_INTERVAL_MS } from '../../lib/constants';
import { BoxParameterTable, type BoxRow } from '../shared/BoxParameterTable';
import type { FitResult, SAParams } from '../../lib/types';
import type { ReflSettingsInput } from '../../../main/native/stochfit-api';
import type { BoxReflSettingsInput, LMFitResult, RhoGenerateResult } from '../../../main/native/levmar-api';

// ── Param helpers ────────────────────────────────────────────────────────────

function defaultRow(): BoxRow {
  return { length: 15.0, rho: 0.5, sigma: 3.0 };
}

function defaultRows(n: number): BoxRow[] {
  return Array.from({ length: n }, defaultRow);
}

function buildRhoParams(subRough: number, zOffset: number, rows: BoxRow[], oneSigma: boolean): number[] {
  const result = [subRough, zOffset];
  for (const row of rows) {
    result.push(row.length, row.rho);
    if (!oneSigma) result.push(row.sigma);
  }
  return result;
}

function parseRhoParams(
  params: number[],
  boxes: number,
  oneSigma: boolean,
): { subRough: number; zOffset: number; rows: BoxRow[] } {
  const subRough = params[0];
  const zOffset = params[1];
  const rows: BoxRow[] = [];
  let idx = 2;
  for (let i = 0; i < boxes; i++) {
    const length = params[idx++];
    const rho = params[idx++];
    const sigma = oneSigma ? 0 : (params[idx++] ?? 0);
    rows.push({ length, rho, sigma });
  }
  return { subRough, zOffset, rows };
}

function makeBounds(params: number[]) {
  const ul = params.map((v) => (Math.abs(v) > 0 ? Math.abs(v) * 3 : 10));
  const ll = params.map((v) => (v >= 0 ? 0 : v * 3));
  const percs = params.map(() => 10);
  return { ul, ll, percs };
}

// ── Component ────────────────────────────────────────────────────────────────

export function ModelIndependentPanel() {
  const { data } = useDataStore();
  const { settings } = useSettingsStore();
  const {
    status, result, saParams, pollTimer,
    setStatus, setResult, setSAParams, setPollTimer, setMiBoxED, reset,
  } = useFitStore();

  const totalIterations = settings?.iterations ?? 0;

  // SA speed tracking
  const [itPerSec, setItPerSec] = useState(0);
  const prevIterRef = React.useRef(0);
  const prevTimeRef = React.useRef(0);
  const autoBoxTimer = React.useRef<ReturnType<typeof setTimeout> | null>(null);

  // EDP box fitting state
  const [boxes, setBoxesState] = useState(3);
  const [subRough, setSubRough] = useState(3.0);
  const [zOffset, setZOffset] = useState(() => settings?.leftoffset ?? 0.0);
  const [oneSigma, setOneSigma] = useState(false);
  const [boxRows, setBoxRows] = useState<BoxRow[]>(defaultRows(3));
  const [lmResult, setLmResult] = useState<LMFitResult | null>(null);
  const [lmBusy, setLmBusy] = useState(false);
  const [lmError, setLmError] = useState<string | null>(null);

  // Sync box rows count when boxes changes
  useEffect(() => {
    setBoxRows((prev) => {
      if (prev.length === boxes) return prev;
      if (boxes > prev.length) {
        return [...prev, ...Array.from({ length: boxes - prev.length }, defaultRow)];
      }
      return prev.slice(0, boxes);
    });
  }, [boxes]);

  // Clean up poll timer on unmount
  useEffect(() => {
    return () => { if (pollTimer) clearInterval(pollTimer); };
  }, [pollTimer]);

  // ── SA Fitting ─────────────────────────────────────────────────────────────

  async function handleStart() {
    if (!data) return alert('Load a data file first.');
    reset();
    setStatus('running');
    setItPerSec(0);
    prevIterRef.current = 0;
    prevTimeRef.current = Date.now();

    const input: ReflSettingsInput = {
      directory: data.filePath.replace(/[^/\\]+$/, ''),
      q: data.q,
      refl: data.refl,
      reflError: data.reflError,
      qError: data.qError,
      qPoints: data.q.length,
      debug: false,
      ...settings,
      normSearchPerc: settings.impnorm ? settings.normSearchPerc : 0,
      absSearchPerc: settings.useSurfAbs ? settings.absSearchPerc : 0,
    };

    try {
      await window.api.stochInit(input);
    } catch (e) {
      setStatus('idle');
      alert(String(e));
      return;
    }

    try {
      await window.api.stochStart(settings.iterations);
    } catch (e) {
      setStatus('idle');
      alert(String(e));
      return;
    }

    const timer = setInterval(async () => {
      let fitData: FitResult;
      let saData: SAParams;
      try {
        [fitData, saData] = await Promise.all([
          window.api.stochGetData() as Promise<FitResult>,
          window.api.stochSAParams() as Promise<SAParams>,
        ]);
      } catch (e) {
        console.error('[MI] poll error:', e);
        return;
      }
      setResult(fitData);
      setSAParams(saData);

      const now = Date.now();
      const elapsed = (now - prevTimeRef.current) / 1000;
      if (elapsed > 0) {
        setItPerSec(Math.round((fitData.iterationsCompleted - prevIterRef.current) / elapsed));
      }
      prevIterRef.current = fitData.iterationsCompleted;
      prevTimeRef.current = now;

      if (fitData.isFinished) {
        clearInterval(timer);
        setPollTimer(null);
        setStatus('completed');
      }
    }, POLLING_INTERVAL_MS);

    setPollTimer(timer);
  }

  async function handleCancel() {
    await window.api.stochCancel();
    if (pollTimer) {
      clearInterval(pollTimer);
      setPollTimer(null);
    }
    setStatus('cancelled');
  }

  // ── EDP Box Fitting ────────────────────────────────────────────────────────

  function buildRhoInput(): BoxReflSettingsInput {
    const zLen = result!.zRange.length;
    const params = buildRhoParams(subRough, zOffset, boxRows, oneSigma);
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
      lowQOffset: 0,
      highQOffset: settings.highQOffset,
      iterations: 1000,
      miedp: result!.rho,
      zIncrement: result!.zRange,   // pass z-positions directly (not differences)
      zLength: zLen,
    };
  }

  async function handleRhoFit() {
    if (!data || !result) return;
    setLmBusy(true);
    setLmError(null);
    try {
      const params = buildRhoParams(subRough, zOffset, boxRows, oneSigma);
      const input = buildRhoInput();
      // Update bounds in input to match current params
      const { ul, ll, percs } = makeBounds(params);
      input.ul = ul;
      input.ll = ll;
      input.paramPercs = percs;

      const fitRes = await window.api.lmRhoFit(input, params) as LMFitResult;
      setLmResult(fitRes);

      // Parse result back into editable fields
      const parsed = parseRhoParams(fitRes.parameters, boxes, oneSigma);
      setSubRough(parsed.subRough);
      setZOffset(parsed.zOffset);
      setBoxRows(parsed.rows);

      // Generate EDP overlay
      const genInput = { ...input, ul: makeBounds(fitRes.parameters).ul, ll: makeBounds(fitRes.parameters).ll };
      const genRes = await window.api.lmRhoGenerate(genInput, fitRes.parameters) as RhoGenerateResult;
      setMiBoxED(genRes.boxED);
    } catch (e) {
      setLmError((e as Error).message);
    } finally {
      setLmBusy(false);
    }
  }

  const generateBoxEDP = useCallback(async () => {
    if (!data || !result) return;
    try {
      const params = buildRhoParams(subRough, zOffset, boxRows, oneSigma);
      const input = buildRhoInput();
      const genRes = await window.api.lmRhoGenerate(input, params) as RhoGenerateResult;
      setMiBoxED(genRes.boxED);
    } catch {
      // silently ignore auto-gen errors
    }
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [data, result, subRough, zOffset, boxRows, oneSigma, boxes, settings]);

  // Auto-generate box EDP whenever fit result or box params change (debounced)
  useEffect(() => {
    if (!data || !result) return;
    if (autoBoxTimer.current) clearTimeout(autoBoxTimer.current);
    autoBoxTimer.current = setTimeout(generateBoxEDP, 400);
    return () => { if (autoBoxTimer.current) clearTimeout(autoBoxTimer.current); };
  }, [generateBoxEDP]);

  async function handleRhoGenerate() {
    if (!data || !result) return;
    setLmBusy(true);
    setLmError(null);
    try {
      await generateBoxEDP();
    } catch (e) {
      setLmError((e as Error).message);
    } finally {
      setLmBusy(false);
    }
  }

  const isRunning = status === 'running';
  const hasFit = !!result;

  return (
    <div className="flex flex-col gap-4 p-4">
      <h2 className="text-sm font-semibold text-primary uppercase tracking-wider">Model Independent</h2>

      {/* SA controls */}
      <div className="flex gap-2">
        <button
          onClick={handleStart}
          disabled={isRunning || !data}
          className="flex-1 py-2 text-xs font-medium bg-success/20 hover:bg-success/30 text-success rounded-input disabled:opacity-40 disabled:cursor-not-allowed transition-colors"
        >
          {isRunning ? 'Running…' : 'Start Fit'}
        </button>
        <button
          onClick={handleCancel}
          disabled={!isRunning}
          className="flex-1 py-2 text-xs font-medium bg-destructive/20 hover:bg-destructive/30 text-destructive rounded-input disabled:opacity-40 disabled:cursor-not-allowed transition-colors"
        >
          Cancel
        </button>
      </div>

      <div className="flex items-center gap-2">
        <div className={`w-2 h-2 rounded-full ${
          status === 'running' ? 'bg-success animate-pulse' :
          status === 'completed' ? 'bg-accent' :
          status === 'cancelled' ? 'bg-warning' : 'bg-border'
        }`} />
        <span className="text-xs text-secondary capitalize">{status}</span>
      </div>

      {(result || saParams) && (
        <div className="rounded-card bg-elevated border border-border p-3 flex flex-col gap-2">
          {result && (
            <>
              <StatRow label="Iterations" value={`${result.iterationsCompleted.toLocaleString()} / ${totalIterations.toLocaleString()}`} />
              <StatRow label="Speed" value={`${itPerSec.toLocaleString()} it/s`} />
              <StatRow label="χ²" value={result.chiSquare.toExponential(4)} />
              <StatRow label="Goodness" value={result.goodnessOfFit.toFixed(4)} />
              <StatRow label="Roughness" value={`${result.roughness.toFixed(2)} Å`} />
            </>
          )}
          {saParams && (
            <>
              <StatRow label="Temperature" value={saParams.temp.toExponential(3)} />
              <StatRow label="Lowest E" value={saParams.lowestEnergy.toExponential(4)} />
              <StatRow label="Mode" value={String(saParams.mode)} />
            </>
          )}
        </div>
      )}

      {/* EDP box fitting — shown after MI completes */}
      {hasFit && (
        <>
          <div className="border-t border-border pt-4">
            <h3 className="text-xs font-semibold text-secondary uppercase tracking-wider mb-3">Fit Boxes to EDP</h3>

            <div className="flex flex-col gap-2">
              {/* Global params */}
              <div className="grid grid-cols-2 gap-2">
                <LabeledInput label="Sub Rough (Å)" value={subRough} onChange={setSubRough} />
                <LabeledInput label="Z Offset (Å)" value={zOffset} onChange={setZOffset} />
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
              <BoxParameterTable rows={boxRows} onRowChange={(i, row) => {
                setBoxRows((prev) => prev.map((r, idx) => idx === i ? row : r));
              }} oneSigma={oneSigma} />

              <div className="flex gap-2 mt-1">
                <button
                  onClick={handleRhoFit}
                  disabled={lmBusy}
                  className="flex-1 py-2 text-xs font-medium bg-accent/20 hover:bg-accent/30 text-accent rounded-input disabled:opacity-40 disabled:cursor-not-allowed transition-colors"
                >
                  {lmBusy ? 'Fitting…' : 'Fit'}
                </button>
                <button
                  onClick={handleRhoGenerate}
                  disabled={lmBusy}
                  className="flex-1 py-2 text-xs font-medium bg-elevated hover:bg-surface text-secondary rounded-input disabled:opacity-40 disabled:cursor-not-allowed transition-colors border border-border"
                >
                  Generate
                </button>
              </div>
            </div>
          </div>

          {lmError && (
            <div className="text-xs text-destructive rounded-card bg-destructive/10 border border-destructive/20 p-2">
              {lmError}
            </div>
          )}

          {lmResult && (
            <div className="rounded-card bg-elevated border border-border p-3 flex flex-col gap-1.5">
              <div className="text-xs font-medium text-secondary mb-1">Rho Fit Result</div>
              <div className="flex justify-between text-xs">
                <span className="text-secondary">‖e‖²</span>
                <span className="font-mono text-primary">{lmResult.info[1]?.toExponential(3) ?? '—'}</span>
              </div>
              <div className="flex justify-between text-xs">
                <span className="text-secondary">Iterations</span>
                <span className="font-mono text-primary">{lmResult.info[5]?.toFixed(0) ?? '—'}</span>
              </div>
            </div>
          )}
        </>
      )}
    </div>
  );
}

function StatRow({ label, value }: { label: string; value: string }) {
  return (
    <div className="flex justify-between items-center text-xs">
      <span className="text-secondary">{label}</span>
      <span className="text-primary font-mono">{value}</span>
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
