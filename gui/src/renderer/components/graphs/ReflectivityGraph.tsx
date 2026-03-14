import React, { useRef, useEffect, useState } from 'react';
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

type LockedRange = {
  xaxis?: { range: [number, number]; autorange: false };
  yaxis?: { range: [number, number]; autorange: false };
} | null;

export function ReflectivityGraph({ data, fitResult }: Props) {
  const plotRef = useRef<any>(null);
  const normalizeByFresnel = useUiStore((s) => s.normalizeByFresnel);
  const settings = useSettingsStore((s) => s.settings);
  const [lockedRange, setLockedRange] = useState<LockedRange>(null);
  const traces: Data[] = [];

  // Reset zoom when Fresnel normalization toggles (axes change scale/meaning)
  useEffect(() => {
    setLockedRange(null);
    if (plotRef.current?.el) {
      plotRef.current.el.Plotly?.redraw?.();
    }
  }, [normalizeByFresnel]);

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

  const baseLayout = normalizeByFresnel && settings && calcQc(settings.subSLD, settings.supSLD) !== 0
    ? reflLayout({
        xaxis: { title: { text: 'Q / Q<sub>c</sub>', font: { size: 12 } }, type: 'linear', gridcolor: 'rgba(100, 100, 100, 0.15)' },
        yaxis: { title: { text: 'Intensity / Fresnel', font: { size: 12 } }, type: 'log', gridcolor: 'rgba(100, 100, 100, 0.15)' },
      })
    : reflLayout();

  const layout = {
    ...baseLayout,
    xaxis: { ...(baseLayout.xaxis as any), ...(lockedRange?.xaxis ?? {}) },
    yaxis: { ...(baseLayout.yaxis as any), ...(lockedRange?.yaxis ?? {}) },
  };

  return (
    <Plot
      ref={plotRef}
      data={traces}
      layout={layout}
      config={plotlyConfig}
      style={{ width: '100%', height: '100%' }}
      useResizeHandler
      onRelayout={handleRelayout}
    />
  );
}
