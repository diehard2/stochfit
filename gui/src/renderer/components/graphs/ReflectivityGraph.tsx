import React, { useRef, useEffect } from 'react';
import Plot from 'react-plotly.js';
import type { Data } from 'plotly.js';
import { reflLayout, plotlyConfig } from './graph-config';
import { COLORS, calcQc, calcFresnelPoint } from '../../lib/constants';
import type { ReflData, FitResult } from '../../lib/types';
import { useUiStore } from '../../stores/ui-store';
import { useDataStore } from '../../stores/data-store';
import { useSettingsStore } from '../../stores/settings-store';

interface Props {
  data: ReflData | null;
  fitResult: FitResult | null;
}

export function ReflectivityGraph({ data, fitResult }: Props) {
  const plotRef = useRef<any>(null);
  const normalizeByFresnel = useUiStore((s) => s.normalizeByFresnel);
  const settings = useSettingsStore((s) => s.settings);
  const traces: Data[] = [];

  useEffect(() => {
    if (plotRef.current?.el) {
      plotRef.current.el.Plotly?.redraw?.();
    }
  }, [normalizeByFresnel]);

  const applyFresnelNorm = (q: number[], refl: number[], errors?: number[]) => {
    if (!normalizeByFresnel || !settings) {
      return { q, refl, errors };
    }

    const Qc = calcQc(settings.subSLD, settings.supSLD);
    if (Qc === 0) return { q, refl, errors };

    return {
      q: q.map((qi) => qi / Qc),
      refl: refl.map((ri, i) => ri / calcFresnelPoint(q[i], Qc)),
      errors: errors?.map((e, i) => e / calcFresnelPoint(q[i], Qc)),
    };
  };

  if (data) {
    const normalized = applyFresnelNorm(data.q, data.refl, data.reflError);
    traces.push({
      x: normalized.q,
      y: normalized.refl,
      error_y: data.reflError.some((e) => e > 0)
        ? { type: 'data', array: normalized.errors, visible: true, color: COLORS.errorBar, thickness: 1, width: 3 }
        : undefined,
      mode: 'markers',
      type: 'scatter',
      name: 'Measured',
      marker: { color: COLORS.measured, size: 3, opacity: 0.7 },
    });
  }

  if (fitResult && fitResult.qRange.length > 0) {
    const normalized = applyFresnelNorm(fitResult.qRange, fitResult.refl);
    traces.push({
      x: normalized.q,
      y: normalized.refl,
      mode: 'lines',
      type: 'scatter',
      name: 'MI Fit',
      line: { color: COLORS.miFit, width: 2, shape: 'spline' },
    });
  }

  const layout = normalizeByFresnel && settings && calcQc(settings.subSLD, settings.supSLD) !== 0
    ? reflLayout({
        xaxis: { title: { text: 'Q / Q<sub>c</sub>', font: { size: 12 } }, type: 'linear', gridcolor: 'rgba(100, 100, 100, 0.15)' },
        yaxis: { title: { text: 'Intensity / Fresnel', font: { size: 12 } }, type: 'log', gridcolor: 'rgba(100, 100, 100, 0.15)' },
      })
    : reflLayout();

  return (
    <Plot
      ref={plotRef}
      data={traces}
      layout={layout}
      config={plotlyConfig}
      style={{ width: '100%', height: '100%' }}
      useResizeHandler
    />
  );
}
