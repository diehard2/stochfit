import React, { useState } from 'react';
import Plot from 'react-plotly.js';
import type { Data } from 'plotly.js';
import { edpLayout, plotlyConfig } from './graph-config';
import { COLORS } from '../../lib/constants';
import type { FitResult } from '../../lib/types';

interface Props {
  fitResult: FitResult | null;
  boxED?: number[];
}

type LockedRange = {
  xaxis?: { range: [number, number]; autorange: false };
  yaxis?: { range: [number, number]; autorange: false };
} | null;

export function ElectronDensityGraph({ fitResult, boxED }: Props) {
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

  if (fitResult && fitResult.zRange.length > 0) {
    traces.push({
      x: fitResult.zRange,
      y: fitResult.rho,
      mode: 'lines',
      type: 'scatter',
      name: 'EDP',
      line: { color: COLORS.miFit, width: 2, shape: 'spline' },
    });
  }

  if (boxED && fitResult && boxED.length === fitResult.zRange.length) {
    traces.push({
      x: fitResult.zRange,
      y: boxED,
      mode: 'lines',
      type: 'scatter',
      name: 'Box Model',
      line: { color: COLORS.boxFit, width: 1.5, shape: 'hv' },
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
