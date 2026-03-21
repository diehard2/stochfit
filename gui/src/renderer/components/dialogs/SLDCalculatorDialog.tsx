import React, { useState, useRef, useLayoutEffect } from 'react';
import { useUiStore } from '../../stores/ui-store';
import { useSettingsStore } from '../../stores/settings-store';
import {
  parseFormula,
  processInput,
  molecularWeight,
  electronCount,
  bSum,
  mwvolFromDensity,
  mwvolFromBox,
  xraySLD,
  neutronSLD,
  type AtomCounts,
} from '../../lib/sld-calculator';

type DensityMode = 'bulk' | 'box';

function ResultRow({ label, value }: { label: string; value: number | null }) {
  const update = useSettingsStore((s) => s.update);

  function apply(field: 'subSLD' | 'filmSLD' | 'supSLD') {
    if (value === null) return;
    update({ [field]: parseFloat(value.toFixed(4)) });
  }

  const btnClass =
    'text-xs px-2 py-0.5 rounded bg-surface border border-border hover:border-accent/50 hover:text-accent transition-colors disabled:opacity-40 disabled:cursor-not-allowed';

  return (
    <div className="flex flex-col gap-1.5">
      <div className="flex items-baseline gap-2">
        <span className="text-secondary text-xs w-14 shrink-0 text-right">{label}</span>
        <span className="font-mono text-sm text-primary whitespace-nowrap">
          {value !== null ? `${value.toFixed(4)} ×10⁻⁶ Å⁻²` : '—'}
        </span>
      </div>
      <div className="flex gap-1.5 pl-[calc(3.5rem+0.5rem)]">
        <button className={btnClass} disabled={value === null} onClick={() => apply('subSLD')}>→ Sub</button>
        <button className={btnClass} disabled={value === null} onClick={() => apply('filmSLD')}>→ Film</button>
        <button className={btnClass} disabled={value === null} onClick={() => apply('supSLD')}>→ Sup</button>
      </div>
    </div>
  );
}

export function SLDCalculatorDialog() {
  const { sldCalcOpen, setSldCalcOpen } = useUiStore();
  const isNeutron = useSettingsStore((s) => s.settings.neutron);

  const [formula, setFormula] = useState('');
  const [densityMode, setDensityMode] = useState<DensityMode>('bulk');
  const [density, setDensity] = useState(1.0);
  const [area, setArea] = useState(30.0);
  const [thickness, setThickness] = useState(10.0);

  const inputRef = useRef<HTMLInputElement>(null);
  const cursorRef = useRef<number>(0);

  // Restore cursor position after every formula state update.
  // Without this, React's controlled-input rerender resets the cursor to the end.
  useLayoutEffect(() => {
    const el = inputRef.current;
    if (el && document.activeElement === el) {
      el.setSelectionRange(cursorRef.current, cursorRef.current);
    }
  }, [formula]);

  if (!sldCalcOpen) return null;

  // --- Derived ---
  const parsed = formula.trim() ? parseFormula(formula) : null;
  const counts: AtomCounts | null = parsed?.counts ?? null;
  const hasUnknown = (parsed?.unknown?.length ?? 0) > 0;
  const validFormula = parsed !== null && !hasUnknown;

  const mw = counts ? molecularWeight(counts) : null;
  const Z = counts ? electronCount(counts) : null;
  const bs = counts ? bSum(counts) : null;

  const mwvol =
    validFormula && counts
      ? densityMode === 'bulk'
        ? mwvolFromDensity(counts, density)
        : mwvolFromBox(area, thickness)
      : null;

  const xr = mwvol !== null && counts ? xraySLD(mwvol, counts) : null;
  const ns = mwvol !== null && counts ? neutronSLD(mwvol, counts) : null;

  // Atom tag pills
  const tags = counts
    ? Object.entries(counts).map(([el, n]) => {
        const unknown = parsed?.unknown?.includes(el) ?? false;
        return (
          <span
            key={el}
            className={`inline-flex items-baseline px-1.5 py-0.5 rounded text-xs font-mono border ${
              unknown
                ? 'border-red-500/60 text-red-400 bg-red-500/10'
                : 'border-border text-secondary bg-elevated'
            }`}
          >
            {el}{n > 1 && <sub className="text-[0.65em] leading-none">{n}</sub>}
          </span>
        );
      })
    : null;

  const inputClass =
    'h-7 px-2 text-xs font-mono bg-elevated border border-border rounded-input text-primary focus:outline-none focus:border-accent/50 w-full';
  const radioLabelClass = 'flex items-center gap-2 cursor-pointer text-xs text-secondary hover:text-primary';

  return (
    <div
      className="fixed inset-0 z-50 flex items-center justify-center bg-black/60"
      onClick={() => setSldCalcOpen(false)}
    >
      <div
        className="bg-surface border border-border rounded-card w-[420px] shadow-subtle"
        onClick={(e) => e.stopPropagation()}
      >
        {/* Header */}
        <div className="flex items-center justify-between px-4 py-3 border-b border-border">
          <h2 className="text-sm font-semibold text-primary">SLD Calculator</h2>
          <button
            onClick={() => setSldCalcOpen(false)}
            className="text-secondary hover:text-primary text-lg leading-none"
          >
            ×
          </button>
        </div>

        <div className="p-4 flex flex-col gap-4">
          {/* Formula */}
          <div className="flex flex-col gap-1.5">
            <label className="text-xs text-secondary">Formula</label>
            <input
              className={`${inputClass} ${formula && !validFormula ? 'border-red-500/60' : ''}`}
              placeholder="e.g. H2O, SiO2, Ca(OH)2, C16H32O2"
              value={formula}
              ref={inputRef}
              onChange={(e) => {
                cursorRef.current = e.target.selectionStart ?? e.target.value.length;
                setFormula(processInput(e.target.value));
              }}
              spellCheck={false}
            />
            {/* Tags row */}
            {tags && (
              <div className="flex flex-wrap gap-1 mt-0.5">
                {tags}
              </div>
            )}
            {/* Molecular stats */}
            {validFormula && mw !== null && Z !== null && bs !== null && (
              <div className="text-xs text-secondary font-mono flex gap-4 mt-0.5">
                <span>MW: {mw.toFixed(3)} g/mol</span>
                <span>Z: {Z} e⁻</span>
                <span>Σb: {bs.toFixed(3)} fm</span>
              </div>
            )}
            {hasUnknown && (
              <span className="text-xs text-red-400">
                Unknown element(s): {parsed?.unknown?.join(', ')}
              </span>
            )}
          </div>

          {/* Volume / Density */}
          <div className="flex flex-col gap-2">
            <span className="text-xs font-medium text-secondary uppercase tracking-wider">Volume</span>
            <label className={radioLabelClass}>
              <input
                type="radio"
                checked={densityMode === 'bulk'}
                onChange={() => setDensityMode('bulk')}
                className="accent-accent"
              />
              Bulk Density
              {densityMode === 'bulk' && (
                <div className="flex items-center gap-1.5 ml-2">
                  <input
                    type="number"
                    className="h-7 w-24 px-2 text-xs font-mono bg-elevated border border-border rounded-input text-primary focus:outline-none focus:border-accent/50"
                    value={density}
                    step={0.001}
                    min={0}
                    onChange={(e) => setDensity(parseFloat(e.target.value) || 0)}
                  />
                  <span className="text-secondary text-xs">g/cm³</span>
                </div>
              )}
            </label>
            <label className={radioLabelClass}>
              <input
                type="radio"
                checked={densityMode === 'box'}
                onChange={() => setDensityMode('box')}
                className="accent-accent"
              />
              Area + Thickness
              {densityMode === 'box' && (
                <div className="flex items-center gap-1.5 ml-2">
                  <input
                    type="number"
                    className="h-7 w-20 px-2 text-xs font-mono bg-elevated border border-border rounded-input text-primary focus:outline-none focus:border-accent/50"
                    value={area}
                    step={0.1}
                    min={0}
                    onChange={(e) => setArea(parseFloat(e.target.value) || 0)}
                  />
                  <span className="text-secondary text-xs">Å²</span>
                  <span className="text-secondary text-xs mx-0.5">×</span>
                  <input
                    type="number"
                    className="h-7 w-20 px-2 text-xs font-mono bg-elevated border border-border rounded-input text-primary focus:outline-none focus:border-accent/50"
                    value={thickness}
                    step={0.1}
                    min={0}
                    onChange={(e) => setThickness(parseFloat(e.target.value) || 0)}
                  />
                  <span className="text-secondary text-xs">Å</span>
                </div>
              )}
            </label>
          </div>

          {/* Results */}
          <div className="flex flex-col gap-2">
            <span className="text-xs font-medium text-secondary uppercase tracking-wider">Results</span>
            <div className="bg-elevated border border-border rounded-card p-3">
              {isNeutron
                ? <ResultRow label="Neutron" value={ns} />
                : <ResultRow label="X-ray" value={xr} />}
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
