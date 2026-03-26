import React, { useEffect, useState } from 'react';
import Plot from './Plot';
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
  lmRefl?: number[];
  lmQ?: number[];
}

type LockedRange = {
  xaxis?: { range: [number, number]; autorange: false };
  yaxis?: { range: [number, number]; autorange: false };
} | null;

export function ReflectivityGraph({ data, fitResult, lmRefl, lmQ }: Props) {
  const graphMode = useUiStore((s) => s.graphMode);
  const normalizeByFresnel = graphMode === 'fresnel';
  const rq4Mode = graphMode === 'rq4';
  const settings = useSettingsStore((s) => s.settings);
  const [lockedRange, setLockedRange] = useState<LockedRange>(null);
  const traces: Data[] = [];

  // Reset zoom when display mode changes (axes change scale/meaning)
  useEffect(() => {
    setLockedRange(null);
  }, [graphMode]);

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

  const applyTransform = (q: number[], refl: number[], errors?: number[]) => {
    if (rq4Mode) {
      return {
        q,
        refl: refl.map((r, i) => r * Math.pow(q[i], 4)),
        errors: errors?.map((e, i) => e * Math.pow(q[i], 4)),
      };
    }
    if (normalizeByFresnel && settings) {
      const Qc = calcQc(settings.subSLD, settings.supSLD);
      if (Qc !== 0) {
        return {
          q: q.map((qi) => qi / Qc),
          refl: refl.map((ri, i) => ri / calcFresnelPoint(q[i], Qc)),
          errors: errors?.map((e, i) => e / calcFresnelPoint(q[i], Qc)),
        };
      }
    }
    return { q, refl, errors };
  };

  if (data) {
    const normalized = applyTransform(data.q, data.refl, data.reflError);
    traces.push({
      x: normalized.q,
      y: normalized.refl,
      error_y: data.reflError.some((e) => e > 0)
        ? { type: 'data', array: normalized.errors ?? [], visible: true, color: COLORS.errorBar, thickness: 1, width: 3 }
        : undefined,
      mode: 'markers',
      type: 'scatter',
      name: 'Measured',
      marker: { color: COLORS.measured, size: 3, opacity: 0.7 },
      hovertemplate: 'Q: %{x:.4e}<br>R: %{y:.4e}<extra></extra>',
    });
  }

  if (fitResult && fitResult.qRange.length > 0) {
    const normalized = applyTransform(fitResult.qRange, fitResult.refl);
    traces.push({
      x: normalized.q,
      y: normalized.refl,
      mode: 'lines',
      type: 'scatter',
      name: 'MI Fit',
      line: { color: COLORS.miFit, width: 2, shape: 'spline' },
      hovertemplate: 'Q: %{x:.4e}<br>R: %{y:.4e}<extra></extra>',
    });
  }

  if (lmRefl && lmQ && lmQ.length > 0 && lmRefl.length === lmQ.length) {
    const normalized = applyTransform(lmQ, lmRefl);
    traces.push({
      x: normalized.q,
      y: normalized.refl,
      mode: 'lines',
      type: 'scatter',
      name: 'Box Model Fit',
      line: { color: COLORS.modelFit, width: 2 },
      hovertemplate: 'Q: %{x:.4e}<br>R: %{y:.4e}<extra></extra>',
    });
  }

  const baseLayout = rq4Mode
    ? reflLayout({
        xaxis: { title: { text: 'Q (Å⁻¹)', font: { size: 12 } }, type: 'linear', gridcolor: 'rgba(100, 100, 100, 0.15)' },
        yaxis: { title: { text: 'R·Q⁴ (Å⁻⁴)', font: { size: 12 } }, type: 'log', gridcolor: 'rgba(100, 100, 100, 0.15)' },
      })
    : normalizeByFresnel && settings && calcQc(settings.subSLD, settings.supSLD) !== 0
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
      data={traces}
      layout={layout}
      config={plotlyConfig}
      style={{ width: '100%', height: '100%' }}
      useResizeHandler
      onRelayout={handleRelayout}
    />
  );
}
