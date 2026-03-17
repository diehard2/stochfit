import React, { useCallback, useEffect, useState } from 'react';
import { useFitStore } from '../../stores/fit-store';
import { useDataStore } from '../../stores/data-store';
import { useSettingsStore } from '../../stores/settings-store';
import { useUiStore } from '../../stores/ui-store';
import { useMiEdpStore } from '../../stores/mi-edp-store';
import type { ModelSettings } from '../../lib/types';
import { POLLING_INTERVAL_MS } from '../../lib/constants';
import { BoxParameterTable, type BoxRow } from '../shared/BoxParameterTable';
import { Field } from '../shared/Field';
import type { FitResult, SAParams } from '../../lib/types';
import type { ReflSettingsInput, StochRunStateOutput, StochSessionFile } from '../../../main/native/stochfit-api';
import type { BoxReflSettingsInput, LMFitResult } from '../../../main/native/levmar-api';

// ── Param helpers ────────────────────────────────────────────────────────────

// Compute a pure step-function box EDP in the frontend.
// Avoids the erf midpoint artifact (erf(0)=0 → 0.5) that appears when z falls
// exactly on an interface in the C++ RhoGenerate calculation.
function computeBoxStepEDP(
  zRange: number[],
  rows: BoxRow[],
  zOffset: number,
  supSLD: number,
  subSLD: number,
): number[] {
  const boundaries: number[] = [zOffset];
  for (const row of rows) {
    boundaries.push(boundaries[boundaries.length - 1] + row.length);
  }
  const supNorm = subSLD !== 0 ? supSLD / subSLD : 0;
  return zRange.map(z => {
    if (z < boundaries[0]) return supNorm;
    for (let i = 0; i < rows.length; i++) {
      if (z < boundaries[i + 1]) return rows[i].rho;
    }
    return 1.0; // subphase
  });
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
    const sigma = oneSigma ? subRough : (params[idx++] ?? 0);
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
  const { settings, restore: restoreSettings } = useSettingsStore();
  const { gpuAvailable, setSettingsOpen } = useUiStore();
  const {
    status, result, saParams, pollTimer, itPerSec,
    setStatus, setResult, setSAParams, setPollTimer, setMiBoxED, setItPerSec, reset,
  } = useFitStore();

  const totalIterations = settings?.iterations ?? 0;

  // SA speed tracking
  const prevIterRef = React.useRef(0);
  const prevTimeRef = React.useRef(0);
  const autoBoxTimer = React.useRef<ReturnType<typeof setTimeout> | null>(null);

  // Resume dialog
  const [resumePending, setResumePending] = useState<{
    input: ReflSettingsInput;
    session: StochSessionFile;
  } | null>(null);

  // EDP box fitting state (persisted in store)
  const {
    boxes, subRough, zOffset, oneSigma, boxRows, lmResult,
    setBoxes: setBoxesState, setSubRough, setZOffset, setOneSigma, setBoxRows, setLmResult,
  } = useMiEdpStore();
  const [lmBusy, setLmBusy] = useState(false);
  const [lmError, setLmError] = useState<string | null>(null);

  // Sync box rows count when boxes changes
  useEffect(() => {
    setBoxRows((prev) => {
      if (prev.length === boxes) return prev;
      if (boxes > prev.length) {
        return [...prev, ...Array.from({ length: boxes - prev.length }, (): BoxRow => ({ length: 15.0, rho: 0.5, sigma: 3.0 }))];
      }
      return prev.slice(0, boxes);
    });
  }, [boxes]);

  // Start polling for fit updates. Extracted so doStart and the remount effect can share it.
  const startPolling = useCallback((runDirectory: string, runDataFile: string) => {
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
        await saveSession(runDirectory, runDataFile, fitData.iterationsCompleted);
        setStatus('completed');
      }
    }, POLLING_INTERVAL_MS);

    setPollTimer(timer);
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  // On remount: if a fit was running when we switched away, restart polling.
  useEffect(() => {
    if (status === 'running' && !pollTimer && data) {
      prevTimeRef.current = Date.now();
      startPolling(
        data.filePath.replace(/[^/\\]+$/, ''),
        data.filePath,
      );
    }
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  // Clean up poll timer on unmount and null it in the store so remount detects it's gone.
  useEffect(() => {
    return () => {
      if (pollTimer) {
        clearInterval(pollTimer);
        setPollTimer(null);
      }
    };
  }, [pollTimer, setPollTimer]);

  // ── SA Fitting ─────────────────────────────────────────────────────────────

  async function handleStart() {
    if (!data) return alert('Load a data file first.');

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

    const sessionPath = input.directory + 'stochfit-session.json';
    const session = await window.api.stochLoadSession(sessionPath) as StochSessionFile | null;
    if (session && session.saState.edCount === settings.boxes + 2) {
      setResumePending({ input, session });
      return;
    }

    await doStart(input, null);
  }

  async function doStart(input: ReflSettingsInput, runState: StochRunStateOutput | null) {
    reset();
    setStatus('running');
    setItPerSec(0);
    prevIterRef.current = 0;
    prevTimeRef.current = Date.now();

    // Capture directory for session file path (stable over the run lifetime)
    const runDirectory = input.directory;
    const runDataFile = data!.filePath;

    try {
      await window.api.stochInit(input, runState);
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

    startPolling(runDirectory, runDataFile);
  }

  async function saveSession(directory: string, dataFile: string, iteration: number) {
    try {
      await window.api.stochStop();
      const saState = await window.api.stochGetRunState(settings.boxes) as StochRunStateOutput;
      saState.iteration = iteration;
      const session: StochSessionFile = {
        version: 1,
        savedAt: new Date().toISOString(),
        dataFile,
        settings: { ...settings } as Record<string, unknown>,
        saState,
      };
      await window.api.stochWriteSession(directory + 'stochfit-session.json', session);
    } catch (e) {
      console.error('[MI] saveSession failed:', e);
    } finally {
      await window.api.stochDestroy();
    }
  }

  async function handleCancel() {
    if (pollTimer) {
      clearInterval(pollTimer);
      setPollTimer(null);
    }
    if (data) {
      const directory = data.filePath.replace(/[^/\\]+$/, '');
      await saveSession(directory, data.filePath, result?.iterationsCompleted ?? 0);
    } else {
      await window.api.stochCancel();
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

      // Generate box step EDP overlay directly in frontend (no erf midpoint artifacts)
      const boxED = computeBoxStepEDP(result.zRange, parsed.rows, parsed.zOffset, settings.supSLD, settings.subSLD);
      setMiBoxED(boxED);
    } catch (e) {
      setLmError((e as Error).message);
    } finally {
      setLmBusy(false);
    }
  }

  const generateBoxEDP = useCallback(() => {
    if (!result) return;
    const boxED = computeBoxStepEDP(result.zRange, boxRows, zOffset, settings.supSLD, settings.subSLD);
    setMiBoxED(boxED);
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [result, zOffset, boxRows, settings.supSLD, settings.subSLD]);

  // Auto-generate box EDP whenever fit result or box params change (debounced)
  useEffect(() => {
    if (!data || !result) return;
    if (autoBoxTimer.current) clearTimeout(autoBoxTimer.current);
    autoBoxTimer.current = setTimeout(generateBoxEDP, 400);
    return () => { if (autoBoxTimer.current) clearTimeout(autoBoxTimer.current); };
  }, [generateBoxEDP]);

  const isRunning = status === 'running';
  const hasFit = !!result;

  return (
    <div className="flex flex-col gap-4 p-4">
      {resumePending && (
        <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/60">
          <div className="bg-surface border border-border rounded-card w-72 shadow-subtle p-5 flex flex-col gap-4">
            <h2 className="text-sm font-semibold text-primary">Previous Run Found</h2>
            <p className="text-xs text-secondary leading-relaxed">
              A previous run was found in this directory. Resume from where it left off, or start fresh?
            </p>
            <div className="flex gap-2">
              <button
                onClick={() => {
                  const p = resumePending;
                  setResumePending(null);
                  restoreSettings(p.session.settings as unknown as Partial<ModelSettings>);
                  doStart(p.input, p.session.saState);
                }}
                className="flex-1 py-2 text-xs font-medium bg-accent/20 hover:bg-accent/30 text-accent rounded-input transition-colors"
              >
                Resume
              </button>
              <button
                onClick={() => { const p = resumePending; setResumePending(null); doStart(p.input, null); }}
                className="flex-1 py-2 text-xs font-medium bg-elevated hover:bg-surface text-secondary rounded-input border border-border transition-colors"
              >
                Fresh Start
              </button>
            </div>
          </div>
        </div>
      )}

      <h2 className="text-sm font-semibold text-primary uppercase tracking-wider">Model Independent</h2>

      {/* Fit Settings */}
      <div className="flex flex-col gap-2">
        <div className="text-xs font-semibold text-secondary uppercase tracking-wider border-b border-border pb-1">
          Fit Settings
        </div>
        <Field
          label="Algorithm"
          field="algorithm"
          type="select"
          tooltip="Greedy: fast, no uphill moves. SA: accepts worse solutions to escape local minima. STUN: stochastic tunneling."
          options={[
            { label: 'Greedy', value: 0 },
            { label: 'Simulated Annealing', value: 1 },
            { label: 'STUN', value: 2 },
          ]}
        />
        <Field
          label="Objective Function"
          field="objectivefunction"
          type="select"
          options={[
            { label: 'Log Difference', value: 0, description: <ObjFormula0 /> },
            { label: 'Inverse Difference', value: 1, description: <ObjFormula1 /> },
            { label: 'Log Difference + Errors', value: 2, description: <ObjFormula2 /> },
            { label: 'Inverse Difference + Errors', value: 3, description: <ObjFormula3 /> },
          ]}
        />
        <Field label="Roughness (σ) Search %" field="sigmasearch" step={1} />
        {settings.algorithm === 2 && (
          <Field label="Adaptive Temperature" field="adaptive" type="checkbox" tooltip="Auto-adjust temperature schedule based on acceptance rate." />
        )}
        <Field
          label="Use GPU Acceleration"
          field="useGpu"
          type="checkbox"
          disabled={!gpuAvailable}
          tooltip={gpuAvailable ? undefined : 'Requires NVIDIA RTX 20+ (compute 7.5+) or Apple Silicon Mac'}
        />
        <button
          onClick={() => setSettingsOpen(true)}
          className="mt-1 py-1.5 text-xs text-secondary hover:text-primary border border-border rounded-input transition-colors bg-elevated hover:bg-surface"
        >
          Advanced…
        </button>
      </div>

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
              <StatRow label="χ²" value={result.chiSquare.toExponential(4)} />
              <StatRow label="Goodness" value={result.goodnessOfFit.toExponential(4)} />
              <StatRow label="Roughness" value={`${result.roughness.toFixed(2)} Å`} />
            </>
          )}
          {saParams && (
            <>
              <StatRow label="Temperature" value={saParams.temp.toExponential(3)} />
              {settings.algorithm !== 0 && (
                <StatRow label="Best Goodness" value={saParams.lowestEnergy.toExponential(4)} />
              )}
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
                  <span className="text-xs text-secondary">Lock Roughness</span>
                </label>
              </div>

              {/* Box table */}
              <BoxParameterTable rows={boxRows} onRowChange={(i, row) => {
                setBoxRows((prev) => prev.map((r, idx) => idx === i ? row : r));
              }} oneSigma={oneSigma} />

              <button
                onClick={handleRhoFit}
                disabled={lmBusy}
                className="mt-1 w-full py-2 text-xs font-medium bg-accent/20 hover:bg-accent/30 text-accent rounded-input disabled:opacity-40 disabled:cursor-not-allowed transition-colors"
              >
                {lmBusy ? 'Fitting…' : 'Fit'}
              </button>
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

// ── Math helpers ─────────────────────────────────────────────────────────────
const ms: React.CSSProperties = { fontFamily: 'Georgia, "Times New Roman", serif' };

// Italic serif variable with optional subscript
const V = ({ c, sub }: { c: string; sub?: string }) => (
  <span style={{ ...ms, fontStyle: 'italic' }}>
    {c}{sub && <sub style={{ fontSize: '0.75em' }}>{sub}</sub>}
  </span>
);

// Stacked fraction: numerator / line / denominator
const Frac = ({ n, d }: { n: React.ReactNode; d: React.ReactNode }) => (
  <span style={{ display: 'inline-flex', flexDirection: 'column', alignItems: 'center', verticalAlign: 'middle', lineHeight: 1.15, margin: '0 1px' }}>
    <span style={{ borderBottom: '1px solid currentColor', padding: '0 2px 1px' }}>{n}</span>
    <span style={{ padding: '1px 2px 0' }}>{d}</span>
  </span>
);

const Sq = ({ children }: { children: React.ReactNode }) => (
  <>{children}<sup style={{ fontSize: '0.7em' }}>2</sup></>
);

const Sig = () => <span style={ms}>σ</span>;
const Sum = () => <span style={{ ...ms, fontSize: '1.15em' }}>Σ</span>;

// Formula 0:  Σ(ln Rd − ln Rm)²  /  N
function ObjFormula0() {
  return (
    <span style={{ ...ms, display: 'inline-flex', alignItems: 'center', gap: '3px', flexWrap: 'wrap' }}>
      <Frac
        n={<><Sum /> <Sq>(ln <V c="R" sub="d" /> &minus; ln <V c="R" sub="m" />)</Sq></>}
        d={<V c="N" />}
      />
      <span>&ensp;—&ensp; best for most datasets</span>
    </span>
  );
}

// Formula 1:  Σ(1 − max(Rd/Rm, Rm/Rd))²  /  N
function ObjFormula1() {
  return (
    <span style={{ ...ms, display: 'inline-flex', alignItems: 'center', gap: '3px', flexWrap: 'wrap' }}>
      <Frac
        n={<><Sum /> <Sq>(1 &minus; max(<Frac n={<V c="R" sub="d" />} d={<V c="R" sub="m" />} />, <Frac n={<V c="R" sub="m" />} d={<V c="R" sub="d" />} />))</Sq></>}
        d={<V c="N" />}
      />
      <span>&ensp;—&ensp; symmetric ratio; useful when <V c="R" /> approaches zero (avoids log singularity)</span>
    </span>
  );
}

// Formula 2:  Σ (ln Rd − ln Rm)² / |ln σᵢ|  /  N
function ObjFormula2() {
  return (
    <span style={{ ...ms, display: 'inline-flex', alignItems: 'center', gap: '3px', flexWrap: 'wrap' }}>
      <Frac
        n={<><Sum /> <Frac n={<Sq>(ln <V c="R" sub="d" /> &minus; ln <V c="R" sub="m" />)</Sq>} d={<>|ln <Sig /><sub style={{ fontSize: '0.7em' }}>i</sub>|</>} /></>}
        d={<V c="N" />}
      />
      <span>&ensp;—&ensp; error-weighted log</span>
    </span>
  );
}

// Formula 3:  Σ (1 − max(r,1/r))² · (Rd/σ)²  /  N
function ObjFormula3() {
  return (
    <span style={{ ...ms, display: 'inline-flex', alignItems: 'center', gap: '3px', flexWrap: 'wrap' }}>
      <Frac
        n={<><Sum /> <Sq>(1 &minus; max(<V c="r" />, <Frac n="1" d={<V c="r" />} />))</Sq> &middot; <Sq>(<Frac n={<V c="R" sub="d" />} d={<Sig />} />)</Sq></>}
        d={<V c="N" />}
      />
      <span>&ensp;—&ensp; error-weighted ratio</span>
    </span>
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
  const [text, setText] = React.useState(value.toFixed(4));
  const focused = React.useRef(false);

  React.useEffect(() => {
    if (!focused.current) setText(value.toFixed(4));
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
          setText(isNaN(v) ? value.toFixed(4) : v.toFixed(4));
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
