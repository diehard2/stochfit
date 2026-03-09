import React, { useEffect } from 'react';
import { useFitStore } from '../../stores/fit-store';
import { useDataStore } from '../../stores/data-store';
import { useSettingsStore } from '../../stores/settings-store';
import { POLLING_INTERVAL_MS } from '../../lib/constants';
import type { FitResult, SAParams } from '../../lib/types';
import type { ReflSettingsInput } from '../../../main/native/stochfit-api';

export function FittingPanel() {
  const { data } = useDataStore();
  const { settings } = useSettingsStore();
  const { status, result, saParams, pollTimer, setStatus, setResult, setSAParams, setPollTimer, reset } = useFitStore();

  // Clean up poll timer on unmount
  useEffect(() => {
    return () => {
      if (pollTimer) clearInterval(pollTimer);
    };
  }, [pollTimer]);

  async function handleStart() {
    if (!data) return alert('Load a data file first.');
    reset();
    setStatus('running');

    const input: ReflSettingsInput = {
      directory: data.filePath.replace(/[^/\\]+$/, ''),
      q: data.q,
      refl: data.refl,
      reflError: data.reflError,
      qError: data.qError,
      qPoints: data.q.length,
      debug: false,
      ...settings,
    };

    await window.api.stochInit(input);
    await window.api.stochStart(settings.iterations);

    const timer = setInterval(async () => {
      const [fitData, saData] = await Promise.all([
        window.api.stochGetData() as Promise<FitResult>,
        window.api.stochSAParams() as Promise<SAParams>,
      ]);
      setResult(fitData);
      setSAParams(saData);
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

  const isRunning = status === 'running';

  return (
    <div className="flex flex-col gap-4 p-4">
      <h2 className="text-sm font-semibold text-primary uppercase tracking-wider">Fitting</h2>

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

      {/* Status badge */}
      <div className="flex items-center gap-2">
        <div className={`w-2 h-2 rounded-full ${
          status === 'running' ? 'bg-success animate-pulse' :
          status === 'completed' ? 'bg-accent' :
          status === 'cancelled' ? 'bg-warning' : 'bg-border'
        }`} />
        <span className="text-xs text-secondary capitalize">{status}</span>
      </div>

      {/* Live stats */}
      {(result || saParams) && (
        <div className="rounded-card bg-elevated border border-border p-3 flex flex-col gap-2">
          {result && (
            <>
              <StatRow label="Iterations" value={`${result.iterationsCompleted.toLocaleString()} / ${result.iterationsCompleted > 0 ? '∞' : '0'}`} />
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
