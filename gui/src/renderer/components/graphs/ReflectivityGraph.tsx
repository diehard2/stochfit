import React from 'react';
import Plot from 'react-plotly.js';
import type { Data } from 'plotly.js';
import { reflLayout, plotlyConfig } from './graph-config';
import { COLORS } from '../../lib/constants';
import type { ReflData, FitResult } from '../../lib/types';

interface Props {
  data: ReflData | null;
  fitResult: FitResult | null;
}

export function ReflectivityGraph({ data, fitResult }: Props) {
  const traces: Data[] = [];

  if (data) {
    traces.push({
      x: data.q,
      y: data.refl,
      error_y: data.reflError.some((e) => e > 0)
        ? { type: 'data', array: data.reflError, visible: true, color: COLORS.errorBar, thickness: 1, width: 3 }
        : undefined,
      mode: 'markers',
      type: 'scatter',
      name: 'Measured',
      marker: { color: COLORS.measured, size: 3, opacity: 0.7 },
    });
  }

  if (fitResult && fitResult.qRange.length > 0) {
    traces.push({
      x: fitResult.qRange,
      y: fitResult.refl,
      mode: 'lines',
      type: 'scatter',
      name: 'MI Fit',
      line: { color: COLORS.miFit, width: 2, shape: 'spline' },
    });
  }

  return (
    <Plot
      data={traces}
      layout={reflLayout()}
      config={plotlyConfig}
      style={{ width: '100%', height: '100%' }}
      useResizeHandler
    />
  );
}
