import React from 'react';
import { useUiStore } from '../../stores/ui-store';
import { useSettingsStore } from '../../stores/settings-store';
import { useFitStore } from '../../stores/fit-store';
import { Field } from '../shared/Field';

export function SettingsDialog() {
  const { settingsOpen, setSettingsOpen } = useUiStore();
  const { reset, settings } = useSettingsStore();
  const { status } = useFitStore();
  if (!settingsOpen) return null;

  const algorithm = settings?.algorithm ?? 0;
  const adaptive = settings?.adaptive ?? false;
  const isRunning = status === 'running';

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/60" onClick={() => setSettingsOpen(false)}>
      <div
        className="bg-surface border border-border rounded-card w-[480px] max-h-[80vh] overflow-y-auto shadow-subtle"
        onClick={(e) => e.stopPropagation()}
      >
        <div className="flex items-center justify-between p-4 border-b border-border">
          <h2 className="text-sm font-semibold text-primary">Advanced Settings</h2>
          <button onClick={() => setSettingsOpen(false)} className="text-secondary hover:text-primary text-lg leading-none">×</button>
        </div>

        <div className="p-4 flex flex-col gap-3">
          {/* Always visible */}
          <Field label="Title" field="title" type="text" disabled={isRunning} />
          <Field label="Iterations" field="iterations" step={100000} tooltip="Total SA iterations. Typical: 1-10 million." disabled={isRunning} />
          <Field label="Param Temp" field="paramtemp" step={0.01} tooltip="Step size for parameter perturbation. Lower = finer search, slower convergence." disabled={isRunning} />
          <Field label="Boxes" field="boxes" step={1} disabled={isRunning} />
          <Field label="Resolution (pts/Å)" field="resolution" step={1} tooltip="EDP sampling density. Higher = smoother profile, slower computation." disabled={isRunning} />
          <Field label="Norm. Search %" field="normSearchPerc" step={1} tooltip="Search range for normalization (active with Improved Normalization)." disabled={isRunning} />
          <Field label="Abs. Search %" field="absSearchPerc" step={1} tooltip="Search range for absorption (active with Surface Absorption)." disabled={isRunning} />
          <Field label="Force σ" field="forcesig" step={0.1} tooltip="Force all interfacial roughness to this value (0 = unconstrained)." disabled={isRunning} />

          {/* SA + STUN params */}
          {algorithm >= 1 && (
            <>
              <Field label="Initial Temperature" field="inittemp" step={0.1} disabled={isRunning} />
              <Field label="Plateau Iterations" field="platiter" step={1} disabled={isRunning} />
              <Field label="Slope" field="slope" step={0.01} tooltip="Cooling factor per temperature step (must be > 1). Higher = slower cooling." disabled={isRunning} />
            </>
          )}

          {/* STUN-only params */}
          {algorithm === 2 && (
            <>
              <Field label="Gamma" field="gamma" step={0.01} tooltip="STUN tunneling parameter. Higher = more aggressive tunneling." disabled={isRunning} />
              <Field label="STUN Function (0/1)" field="stunFunc" step={1} tooltip="0: exponential, 1: logarithmic STUN." disabled={isRunning} />
              <Field label="Gamma Decrease" field="gammadec" step={0.01} tooltip="Factor by which gamma decreases at each STUN decrease interval." disabled={isRunning} />
              <Field label="STUN Dec. Iterations" field="stunDeciter" step={10} disabled={isRunning} />
            </>
          )}

          {/* STUN + Adaptive only */}
          {algorithm === 2 && adaptive && (
            <Field label="Temp. Iterations" field="tempiter" step={1} disabled={isRunning} />
          )}
        </div>

        <div className="flex justify-end gap-2 p-4 border-t border-border">
          <button onClick={reset} disabled={isRunning} className="px-3 py-1.5 text-xs text-secondary hover:text-primary border border-border rounded-input transition-colors disabled:opacity-40 disabled:cursor-not-allowed">
            Reset Defaults
          </button>
          <button onClick={() => setSettingsOpen(false)} className="px-3 py-1.5 text-xs font-medium bg-accent/20 hover:bg-accent/30 text-accent rounded-input transition-colors">
            Done
          </button>
        </div>
      </div>
    </div>
  );
}
