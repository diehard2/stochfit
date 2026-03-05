import React from 'react';
import Plot from 'react-plotly.js';
import type { Data } from 'plotly.js';
import { edpLayout, plotlyConfig } from './graph-config';
import { COLORS } from '../../lib/constants';
import type { FitResult } from '../../lib/types';

interface Props {
  fitResult: FitResult | null;
  boxED?: number[];
}

export function ElectronDensityGraph({ fitResult, boxED }: Props) {
  const traces: Data[] = [];

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

  return (
    <Plot
      data={traces}
      layout={edpLayout()}
      config={plotlyConfig}
      style={{ width: '100%', height: '100%' }}
      useResizeHandler
    />
  );
}
