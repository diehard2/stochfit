import React from 'react';
import { useUiStore } from '../../stores/ui-store';
import { useSettingsStore } from '../../stores/settings-store';

export function SettingsDialog() {
  const { settingsOpen, setSettingsOpen } = useUiStore();
  const { settings, update, reset } = useSettingsStore();
  if (!settingsOpen) return null;

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/60" onClick={() => setSettingsOpen(false)}>
      <div
        className="bg-surface border border-border rounded-card w-[480px] max-h-[80vh] overflow-y-auto shadow-subtle"
        onClick={(e) => e.stopPropagation()}
      >
        <div className="flex items-center justify-between p-4 border-b border-border">
          <h2 className="text-sm font-semibold text-primary">Algorithm Settings</h2>
          <button onClick={() => setSettingsOpen(false)} className="text-secondary hover:text-primary text-lg leading-none">×</button>
        </div>

        <div className="p-4 flex flex-col gap-3">
          {([
            ['Initial Temperature', 'inittemp', 0.1],
            ['Plateau Iterations', 'platiter', 1],
            ['Temp Iterations', 'tempiter', 1],
            ['Slope', 'slope', 0.01],
            ['Gamma', 'gamma', 0.01],
            ['Gamma Decrease', 'gammadec', 0.01],
            ['STUN Function (0/1)', 'stunFunc', 1],
            ['STUN Dec. Iters', 'stunDeciter', 10],
            ['Algorithm (0=CSA)', 'algorithm', 1],
            ['Param Temp', 'paramtemp', 0.01],
            ['Sigma Search %', 'sigmasearch', 1],
            ['Norm Search %', 'normSearchPerc', 1],
            ['Abs Search %', 'absSearchPerc', 1],
          ] as [string, keyof typeof settings, number][]).map(([label, field, step]) => (
            <div key={field} className="flex items-center justify-between gap-4">
              <label className="text-xs text-secondary flex-1">{label}</label>
              <input
                type="number"
                value={String(settings[field])}
                step={step}
                onChange={(e) => update({ [field]: parseFloat(e.target.value) })}
                className="w-28 h-7 px-2 text-xs bg-elevated border border-border rounded-input text-primary focus:outline-none focus:border-accent/50"
              />
            </div>
          ))}
          <label className="flex items-center gap-2 text-xs mt-1">
            <input
              type="checkbox"
              checked={settings.adaptive}
              onChange={(e) => update({ adaptive: e.target.checked })}
              className="accent-accent"
            />
            <span className="text-secondary">Adaptive Temperature</span>
          </label>
        </div>

        <div className="flex justify-end gap-2 p-4 border-t border-border">
          <button onClick={reset} className="px-3 py-1.5 text-xs text-secondary hover:text-primary border border-border rounded-input transition-colors">
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
