import React, { useState } from 'react';
import Plot from './Plot';
import type { Data } from 'plotly.js';
import { edpLayout, plotlyConfig } from './graph-config';
import { COLORS } from '../../lib/constants';
import type { FitResult } from '../../lib/types';

interface Props {
  fitResult: FitResult | null;
  boxED?: number[];
  // Independent EDP (box model tab — no fitResult needed)
  lmZRange?: number[];
  lmED?: number[];
  lmBoxED?: number[];
}

type LockedRange = {
  xaxis?: { range: [number, number]; autorange: false };
  yaxis?: { range: [number, number]; autorange: false };
} | null;

export function ElectronDensityGraph({ fitResult, boxED, lmZRange, lmED, lmBoxED }: Props) {
  const [lockedRange, setLockedRange] = useState<LockedRange>(null);
  const traces: Data[] = [];

  const handleRelayout = (e: Readonly<Plotly.PlotRelayoutEvent>) => {
    if (e['xaxis.autorange'] === true) {
      setLockedRange(null);
      return;
    }
    const x0 = e['xaxis.range[0]'] as number | undefined;
    const x1 = e['xaxis.range[1]'] as number | undefined;
    const y0 = e['yaxis.range[0]'] as number | undefined;
    const y1 = e['yaxis.range[1]'] as number | undefined;
    if (x0 !== undefined && x1 !== undefined) {
      setLockedRange({
        xaxis: { range: [x0, x1], autorange: false },
        ...(y0 !== undefined && y1 !== undefined
          ? { yaxis: { range: [y0, y1], autorange: false } }
          : {}),
      });
    }
  };

  // MI fit EDP
  if (fitResult && fitResult.zRange.length > 0) {
    traces.push({
      x: fitResult.zRange,
      y: fitResult.rho,
      mode: 'lines',
      type: 'scatter',
      name: 'EDP',
      line: { color: COLORS.miFit, width: 2, shape: 'spline' },
      hovertemplate: 'Z: %{x:.2f} Å<br>ρ: %{y:.4f}<extra></extra>',
    });
  }

  // MI box overlay (aligned to fitResult.zRange)
  if (boxED && fitResult && boxED.length === fitResult.zRange.length) {
    traces.push({
      x: fitResult.zRange,
      y: boxED,
      mode: 'lines',
      type: 'scatter',
      name: 'Box Model',
      line: { color: COLORS.boxFit, width: 1.5, shape: 'hv' },
      hovertemplate: 'Z: %{x:.2f} Å<br>ρ: %{y:.4f}<extra></extra>',
    });
  }

  // Box model independent EDP (smoothed)
  if (lmED && lmZRange && lmED.length === lmZRange.length) {
    traces.push({
      x: lmZRange,
      y: lmED,
      mode: 'lines',
      type: 'scatter',
      name: 'EDP',
      line: { color: COLORS.miFit, width: 2, shape: 'spline' },
      hovertemplate: 'Z: %{x:.2f} Å<br>ρ: %{y:.4f}<extra></extra>',
    });
  }

  // Box model independent EDP (stepped boxes)
  if (lmBoxED && lmZRange && lmBoxED.length === lmZRange.length) {
    traces.push({
      x: lmZRange,
      y: lmBoxED,
      mode: 'lines',
      type: 'scatter',
      name: 'Boxes',
      line: { color: COLORS.boxFit, width: 1.5, shape: 'hv' },
      hovertemplate: 'Z: %{x:.2f} Å<br>ρ: %{y:.4f}<extra></extra>',
    });
  }

  const base = edpLayout();
  const layout = {
    ...base,
    xaxis: { ...(base.xaxis as any), ...(lockedRange?.xaxis ?? {}) },
    yaxis: { ...(base.yaxis as any), ...(lockedRange?.yaxis ?? {}) },
  };

  return (
    <Plot
      data={traces}
      layout={layout}
      config={plotlyConfig}
      style={{ width: '100%', height: '100%' }}
      useResizeHandler
      onRelayout={handleRelayout}
    />
  );
}
