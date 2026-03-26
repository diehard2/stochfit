import React, { useEffect, useRef, useState } from 'react';
import Plotly from 'plotly.js-dist-min';
import Plot from './Plot';
import type { Data } from 'plotly.js';
import { useUiStore } from '../../stores/ui-store';
import { useDataStore } from '../../stores/data-store';
import { useFitStore } from '../../stores/fit-store';
import { useBoxModelStore } from '../../stores/box-model-store';
import { useSettingsStore } from '../../stores/settings-store';
import {
  pubReflLayout, pubEdpLayout, pubColors, plotlyConfig,
  type PubOptions, defaultPubOptions,
} from './graph-config';
import { calcQc, calcFresnelPoint } from '../../lib/constants';
import { exportSVG, exportPNG } from './export-utils';

// ── Helpers ──────────────────────────────────────────────────────────────────

function applyFresnelNorm(
  q: number[], refl: number[], qc: number, errors?: number[]
): { q: number[]; refl: number[]; errors?: number[] } {
  if (qc === 0) return { q, refl, errors };
  return {
    q: q.map((qi) => qi / qc),
    refl: refl.map((ri, i) => ri / calcFresnelPoint(q[i], qc)),
    errors: errors?.map((e, i) => e / calcFresnelPoint(q[i], qc)),
  };
}

function applyRq4(
  q: number[], refl: number[], errors?: number[]
): { q: number[]; refl: number[]; errors?: number[] } {
  return {
    q,
    refl: refl.map((r, i) => r * Math.pow(q[i], 4)),
    errors: errors?.map((e, i) => e * Math.pow(q[i], 4)),
  };
}

function Empty({ label }: { label: string }) {
  return (
    <div className="w-full h-full flex items-center justify-center text-sm select-none"
      style={{ background: 'white', color: '#aaa' }}>
      {label}
    </div>
  );
}

// ── Graph 1: Reflectivity (measured + MI fit) ─────────────────────────────────

function PubReflGraph({ opts, fresnelNorm, qc, rq4Mode }: { opts: PubOptions; fresnelNorm: boolean; qc: number; rq4Mode: boolean }) {
  const data = useDataStore((s) => s.data);
  const fitResult = useFitStore((s) => s.result);
  const c = pubColors(opts);
  const traces: Data[] = [];

  const applyNorm = (q: number[], refl: number[], errors?: number[]) =>
    rq4Mode ? applyRq4(q, refl, errors) : fresnelNorm ? applyFresnelNorm(q, refl, qc, errors) : { q, refl, errors };

  if (data) {
    const norm = applyNorm(data.q, data.refl, data.reflError);
    traces.push({
      x: norm.q, y: norm.refl,
      error_y: data.reflError.some((e) => e > 0)
        ? { type: 'data', array: norm.errors ?? [], visible: true, color: c.errorBar, thickness: 1, width: 3 }
        : undefined,
      mode: 'markers', type: 'scatter', name: 'Measured',
      marker: { color: c.measured, size: 3 },
    });
  }

  if (fitResult && fitResult.qRange.length > 0) {
    const norm = applyNorm(fitResult.qRange, fitResult.refl);
    traces.push({
      x: norm.q, y: norm.refl,
      mode: 'lines', type: 'scatter', name: 'MI Fit',
      line: { color: c.miFit, width: opts.lineWidth, shape: 'spline' },
    });
  }

  if (traces.length === 0) return <Empty label="No reflectivity data" />;

  return (
    <Plot data={traces} layout={pubReflLayout(opts, fresnelNorm, qc, rq4Mode)}
      config={plotlyConfig} style={{ width: '100%', height: '100%' }} useResizeHandler />
  );
}

// ── Graph 2: MI Electron Density ──────────────────────────────────────────────

function PubEdpGraph({ opts }: { opts: PubOptions }) {
  const fitResult = useFitStore((s) => s.result);
  const miBoxED = useFitStore((s) => s.miBoxED);
  const c = pubColors(opts);
  const traces: Data[] = [];

  if (fitResult && fitResult.zRange.length > 0) {
    traces.push({
      x: fitResult.zRange, y: fitResult.rho,
      mode: 'lines', type: 'scatter', name: 'MI EDP',
      line: { color: c.miFit, width: opts.lineWidth, shape: 'spline' },
    });
  }

  if (miBoxED && fitResult && miBoxED.length === fitResult.zRange.length) {
    traces.push({
      x: fitResult.zRange, y: miBoxED,
      mode: 'lines', type: 'scatter', name: 'Box Overlay',
      line: { color: c.boxFit, width: Math.max(1, opts.lineWidth - 0.5), shape: 'hv' },
    });
  }

  if (traces.length === 0) return <Empty label="No electron density data" />;

  return (
    <Plot data={traces} layout={pubEdpLayout(opts)}
      config={plotlyConfig} style={{ width: '100%', height: '100%' }} useResizeHandler />
  );
}

// ── Graph 3: Box Model EDP ────────────────────────────────────────────────────

function PubBoxEdpGraph({ opts }: { opts: PubOptions }) {
  const genED = useBoxModelStore((s) => s.genED);
  const genBoxED = useBoxModelStore((s) => s.genBoxED);
  const genZRange = useBoxModelStore((s) => s.genZRange);
  const c = pubColors(opts);
  const traces: Data[] = [];

  if (genED && genZRange && genED.length === genZRange.length) {
    traces.push({
      x: genZRange, y: genED,
      mode: 'lines', type: 'scatter', name: 'EDP',
      line: { color: c.miFit, width: opts.lineWidth, shape: 'spline' },
    });
  }

  if (genBoxED && genZRange && genBoxED.length === genZRange.length) {
    traces.push({
      x: genZRange, y: genBoxED,
      mode: 'lines', type: 'scatter', name: 'Boxes',
      line: { color: c.boxFit, width: Math.max(1, opts.lineWidth - 0.5), shape: 'hv' },
    });
  }

  if (traces.length === 0) return <Empty label="No box model EDP — open Box Model panel" />;

  return (
    <Plot data={traces} layout={pubEdpLayout(opts)}
      config={plotlyConfig} style={{ width: '100%', height: '100%' }} useResizeHandler />
  );
}

// ── Graph 4: Box Model Reflectivity ───────────────────────────────────────────

function PubBoxReflGraph({ opts, fresnelNorm, qc, rq4Mode }: { opts: PubOptions; fresnelNorm: boolean; qc: number; rq4Mode: boolean }) {
  const data = useDataStore((s) => s.data);
  const genRefl = useBoxModelStore((s) => s.genRefl);
  const c = pubColors(opts);
  const traces: Data[] = [];

  const applyNorm = (q: number[], refl: number[], errors?: number[]) =>
    rq4Mode ? applyRq4(q, refl, errors) : fresnelNorm ? applyFresnelNorm(q, refl, qc, errors) : { q, refl, errors };

  if (data) {
    const norm = applyNorm(data.q, data.refl, data.reflError);
    traces.push({
      x: norm.q, y: norm.refl,
      error_y: data.reflError.some((e) => e > 0)
        ? { type: 'data', array: norm.errors ?? [], visible: true, color: c.errorBar, thickness: 1, width: 3 }
        : undefined,
      mode: 'markers', type: 'scatter', name: 'Measured',
      marker: { color: c.measured, size: 3 },
    });
  }

  if (data && genRefl && genRefl.length === data.q.length) {
    const norm = applyNorm(data.q, genRefl);
    traces.push({
      x: norm.q, y: norm.refl,
      mode: 'lines', type: 'scatter', name: 'Box Model Fit',
      line: { color: c.modelFit, width: opts.lineWidth },
    });
  }

  if (traces.length === 0) return <Empty label="No box model fit — open Box Model panel" />;

  return (
    <Plot data={traces} layout={pubReflLayout(opts, fresnelNorm, qc, rq4Mode)}
      config={plotlyConfig} style={{ width: '100%', height: '100%' }} useResizeHandler />
  );
}

// ── Panel wrapper (title + graph + export buttons) ────────────────────────────

const ALL_GRAPHS = [
  { id: 'master-refl-graph',     name: 'stochfit-reflectivity' },
  { id: 'master-edp-graph',      name: 'stochfit-edp' },
  { id: 'master-box-edp-graph',  name: 'stochfit-box-edp' },
  { id: 'master-box-refl-graph', name: 'stochfit-box-reflectivity' },
];

type CtxMenu = { x: number; y: number } | null;

async function copyGraphPNG(elementId: string): Promise<void> {
  const el = document.getElementById(elementId);
  if (!el) return;
  const gd = (el.querySelector('.js-plotly-plot') as HTMLElement) ?? el;
  const dataUrl = await Plotly.toImage(gd, { format: 'png', width: 1200, height: 800, scale: 2 });
  const res = await fetch(dataUrl);
  const blob = await res.blob();
  await navigator.clipboard.write([new ClipboardItem({ 'image/png': blob })]);
}

function GraphPanel({
  id, title, filename, children, btnClass,
}: {
  id: string; title: string; filename: string; children: React.ReactNode; btnClass: string;
}) {
  const [ctx, setCtx] = useState<CtxMenu>(null);
  const menuRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (!ctx) return;
    const dismiss = () => setCtx(null);
    document.addEventListener('mousedown', dismiss);
    return () => document.removeEventListener('mousedown', dismiss);
  }, [ctx]);

  return (
    <div className="flex-1 flex flex-col min-h-0 min-w-0">
      <div
        id={id}
        className="flex-1 min-h-0 rounded border border-border overflow-hidden"
        style={{ background: 'white' }}
        onContextMenu={(e) => { e.preventDefault(); setCtx({ x: e.clientX, y: e.clientY }); }}
      >
        {children}
      </div>
      <div className="flex gap-2 mt-1.5 items-center flex-shrink-0">
        <span className="text-xs text-secondary font-medium flex-1">{title}</span>
        <button onClick={() => exportSVG(id, filename)} className={btnClass}>SVG</button>
        <button onClick={() => exportPNG(id, filename)} className={btnClass}>PNG</button>
      </div>

      {ctx && (
        <div
          ref={menuRef}
          className="fixed z-[200] bg-surface border border-border rounded shadow-lg py-1 text-xs"
          style={{ left: ctx.x, top: ctx.y }}
          onMouseDown={(e) => e.stopPropagation()}
        >
          {(['Copy as PNG', 'Save SVG', 'Save PNG'] as const).map((label) => (
            <button
              key={label}
              className="w-full text-left px-4 py-1.5 text-secondary hover:bg-elevated hover:text-primary transition-colors"
              onClick={() => {
                setCtx(null);
                if (label === 'Copy as PNG') copyGraphPNG(id);
                else if (label === 'Save SVG') exportSVG(id, filename);
                else exportPNG(id, filename);
              }}
            >
              {label}
            </button>
          ))}
        </div>
      )}
    </div>
  );
}

// ── Dialog ────────────────────────────────────────────────────────────────────

export function MasterGraphDialog() {
  const { masterGraphOpen, setMasterGraphOpen, graphMode, setGraphMode } = useUiStore();
  const normalizeByFresnel = graphMode === 'fresnel';
  const rq4Mode = graphMode === 'rq4';
  const settings = useSettingsStore((s) => s.settings);
  const [opts, setOpts] = useState<PubOptions>(defaultPubOptions);

  const qc = settings ? calcQc(settings.subSLD, settings.supSLD) : 0;
  const update = (patch: Partial<PubOptions>) => setOpts((o) => ({ ...o, ...patch }));

  useEffect(() => {
    if (!masterGraphOpen) return;
    const handler = (e: KeyboardEvent) => { if (e.key === 'Escape') setMasterGraphOpen(false); };
    document.addEventListener('keydown', handler);
    return () => document.removeEventListener('keydown', handler);
  }, [masterGraphOpen, setMasterGraphOpen]);

  const handleExportAll = async (format: 'svg' | 'png') => {
    for (const g of ALL_GRAPHS) {
      if (document.getElementById(g.id)) {
        if (format === 'svg') await exportSVG(g.id, g.name);
        else await exportPNG(g.id, g.name);
        await new Promise((r) => setTimeout(r, 300));
      }
    }
  };

  if (!masterGraphOpen) return null;

  const btnClass = 'px-2 py-0.5 text-xs bg-surface border border-border rounded text-secondary hover:text-primary transition-colors';
  const sectionLabel = 'text-xs font-semibold text-secondary uppercase tracking-wider mb-2';
  const reflProps = { opts, fresnelNorm: normalizeByFresnel, qc, rq4Mode };

  return (
    <div className="fixed inset-0 z-50 flex flex-col bg-elevated">
      {/* Header */}
      <div className="h-10 flex items-center justify-between px-4 border-b border-border flex-shrink-0">
        <span className="text-sm font-semibold text-primary">Master Graph</span>
        <button onClick={() => setMasterGraphOpen(false)}
          className="text-secondary hover:text-primary text-xl leading-none px-2 transition-colors"
          aria-label="Close">×</button>
      </div>

      {/* Body */}
      <div className="flex flex-1 min-h-0 overflow-hidden">

        {/* 2×2 graph grid */}
        <div className="flex-1 flex flex-col gap-3 p-3 overflow-hidden min-w-0">

          {/* Row 1: MI Reflectivity | MI EDP */}
          <div className="flex-1 flex gap-3 min-h-0">
            <GraphPanel id="master-refl-graph" title="Reflectivity" filename="stochfit-reflectivity" btnClass={btnClass}>
              <PubReflGraph {...reflProps} />
            </GraphPanel>
            <GraphPanel id="master-edp-graph" title="Electron Density" filename="stochfit-edp" btnClass={btnClass}>
              <PubEdpGraph opts={opts} />
            </GraphPanel>
          </div>

          {/* Row 2: Box Model EDP | Box Model Reflectivity */}
          <div className="flex-1 flex gap-3 min-h-0">
            <GraphPanel id="master-box-edp-graph" title="Box Model EDP" filename="stochfit-box-edp" btnClass={btnClass}>
              <PubBoxEdpGraph opts={opts} />
            </GraphPanel>
            <GraphPanel id="master-box-refl-graph" title="Box Model Reflectivity" filename="stochfit-box-reflectivity" btnClass={btnClass}>
              <PubBoxReflGraph {...reflProps} />
            </GraphPanel>
          </div>

        </div>

        {/* Options panel */}
        <div className="w-52 flex-shrink-0 border-l border-border bg-surface p-3 flex flex-col gap-4 overflow-y-auto">

          <div>
            <p className={sectionLabel}>Color</p>
            <div className="flex flex-col gap-1.5">
              {(['bw', 'color'] as const).map((mode) => (
                <label key={mode} className="flex items-center gap-2 cursor-pointer">
                  <input type="radio" name="pubColorMode" value={mode}
                    checked={opts.colorMode === mode}
                    onChange={() => update({ colorMode: mode })}
                    className="accent-accent" />
                  <span className="text-xs text-secondary">
                    {mode === 'bw' ? 'B&W (Publication)' : 'Color'}
                  </span>
                </label>
              ))}
            </div>
          </div>

          <div>
            <p className={sectionLabel}>Font Size &nbsp;<span className="font-normal normal-case">{opts.fontSize}pt</span></p>
            <input type="range" min={10} max={18} step={1} value={opts.fontSize}
              onChange={(e) => update({ fontSize: Number(e.target.value) })}
              className="w-full accent-accent" />
          </div>

          <div>
            <p className={sectionLabel}>Line Width &nbsp;<span className="font-normal normal-case">{opts.lineWidth}px</span></p>
            <input type="range" min={1} max={4} step={0.5} value={opts.lineWidth}
              onChange={(e) => update({ lineWidth: Number(e.target.value) })}
              className="w-full accent-accent" />
          </div>

          <div>
            <p className={sectionLabel}>Display</p>
            <div className="flex flex-col gap-2">
              <label className="flex items-center gap-2 cursor-pointer">
                <input type="checkbox" checked={opts.showLegend}
                  onChange={(e) => update({ showLegend: e.target.checked })}
                  className="rounded accent-accent" />
                <span className="text-xs text-secondary">Show Legend</span>
              </label>
            </div>
          </div>

          <div>
            <p className={sectionLabel}>Graph Mode</p>
            <div className="flex flex-col gap-1.5">
              {(['standard', 'fresnel', 'rq4'] as const).map((mode) => (
                <label key={mode} className="flex items-center gap-2 cursor-pointer">
                  <input type="radio" name="masterGraphMode" value={mode}
                    checked={graphMode === mode}
                    onChange={() => setGraphMode(mode)}
                    className="accent-accent" />
                  <span className="text-xs text-secondary">
                    {mode === 'standard' ? 'Standard' : mode === 'fresnel' ? 'Fresnel Norm.' : 'R·Q⁴'}
                  </span>
                </label>
              ))}
            </div>
          </div>

          <div>
            <button onClick={() => setOpts(defaultPubOptions)}
              className="w-full px-2 py-1 text-xs bg-elevated border border-border rounded text-secondary hover:text-primary transition-colors">
              Reset Options
            </button>
          </div>

          <div className="mt-auto">
            <p className={sectionLabel}>Export All</p>
            <div className="flex flex-col gap-1.5">
              <button onClick={() => handleExportAll('svg')}
                className="px-2 py-1.5 text-xs bg-elevated border border-border rounded text-secondary hover:text-primary transition-colors">
                All → SVG
              </button>
              <button onClick={() => handleExportAll('png')}
                className="px-2 py-1.5 text-xs bg-elevated border border-border rounded text-secondary hover:text-primary transition-colors">
                All → PNG
              </button>
            </div>
          </div>

        </div>
      </div>
    </div>
  );
}
