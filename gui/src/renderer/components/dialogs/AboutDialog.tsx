import React from 'react';
import { useUiStore } from '../../stores/ui-store';

export function AboutDialog() {
  const { aboutOpen, setAboutOpen } = useUiStore();
  if (!aboutOpen) return null;

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/60" onClick={() => setAboutOpen(false)}>
      <div
        className="bg-surface border border-border rounded-card w-80 shadow-subtle"
        onClick={(e) => e.stopPropagation()}
      >
        <div className="p-6 flex flex-col items-center gap-3 text-center">
          <div className="text-4xl">🔬</div>
          <div>
            <h1 className="text-lg font-bold text-primary">StochFit</h1>
            <p className="text-xs text-secondary mt-1">X-Ray Reflectometry Fitting Software</p>
          </div>
          <div className="text-xs text-secondary leading-relaxed">
            Simulated annealing and Levenberg–Marquardt fitting for X-ray reflectometry data.
            Cross-platform Electron GUI.
          </div>
          <div className="text-xs text-secondary/50">
            Original author: Stephen Danauskas<br />
            Licensed under GPL v2
          </div>
          <button
            onClick={() => setAboutOpen(false)}
            className="mt-2 px-4 py-1.5 text-xs font-medium bg-accent/20 hover:bg-accent/30 text-accent rounded-input transition-colors"
          >
            Close
          </button>
        </div>
      </div>
    </div>
  );
}
