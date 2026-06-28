import type { Layout, Config } from 'plotly.js';
import { COLORS, FONTS } from '../../lib/constants';

// ---- Publication / master graph options ----

export interface PubOptions {
  colorMode: 'bw' | 'color';
  fontSize: number;
  lineWidth: number;
  showLegend: boolean;
}

export const defaultPubOptions: PubOptions = {
  colorMode: 'bw',
  fontSize: 13,
  lineWidth: 2,
  showLegend: true,
};

export function pubColors(opts: PubOptions) {
  if (opts.colorMode === 'bw') {
    return {
      measured: '#000000',
      miFit: '#333333',
      modelFit: '#666666',
      boxFit: '#999999',
      errorBar: 'rgba(0,0,0,0.4)',
    };
  }
  return {
    measured: COLORS.measured,
    miFit: COLORS.miFit,
    modelFit: COLORS.modelFit,
    boxFit: COLORS.boxFit,
    errorBar: COLORS.errorBar,
  };
}

function pubBaseLayout(opts: PubOptions): Partial<Layout> {
  const axisBase = {
    gridcolor: 'rgba(0,0,0,0.12)',
    linecolor: '#000000',
    linewidth: 1.5,
    showline: true,
    ticks: 'outside' as const,
    tickcolor: '#000000',
    tickwidth: 1,
    mirror: true as const,
    zeroline: false,
  };
  return {
    paper_bgcolor: 'white',
    plot_bgcolor: 'white',
    font: { family: FONTS.publication, size: opts.fontSize, color: '#000000' },
    margin: { l: 70, r: 30, t: 30, b: 65 },
    showlegend: opts.showLegend,
    legend: {
      x: 0.5, y: -0.2,
      xanchor: 'center',
      orientation: 'h',
      font: { size: opts.fontSize - 1 },
      bgcolor: 'transparent',
    },
    xaxis: axisBase,
    yaxis: axisBase,
  };
}

export function pubReflLayout(opts: PubOptions, fresnelNorm: boolean, qc: number, rq4Mode = false): Partial<Layout> {
  const base = pubBaseLayout(opts);
  let xText = 'Q (Å⁻¹)';
  let yText = 'Intensity';
  let xType: 'linear' | 'log' = 'linear';
  let yType: 'linear' | 'log' = 'log';
  if (rq4Mode) {
    xText = 'Q (Å⁻¹)';
    yText = 'R·Q⁴ (Å⁻⁴)';
    yType = 'log';
  } else if (fresnelNorm && qc !== 0) {
    xText = 'Q / Q<sub>c</sub>';
    yText = 'Intensity / Fresnel';
  }
  return {
    ...base,
    xaxis: {
      ...(base.xaxis as any),
      title: { text: xText, font: { size: opts.fontSize + 1 } },
      type: xType,
    },
    yaxis: {
      ...(base.yaxis as any),
      title: { text: yText, font: { size: opts.fontSize + 1 } },
      type: yType,
    },
  };
}

export function pubEdpLayout(opts: PubOptions): Partial<Layout> {
  const base = pubBaseLayout(opts);
  return {
    ...base,
    xaxis: {
      ...(base.xaxis as any),
      title: { text: 'Z (Å)', font: { size: opts.fontSize + 1 } },
    },
    yaxis: {
      ...(base.yaxis as any),
      title: { text: 'Electron Density (norm.)', font: { size: opts.fontSize + 1 } },
    },
  };
}

export const darkTemplate = {
  layout: {
    paper_bgcolor: 'transparent',
    plot_bgcolor: 'transparent',
    font: { family: FONTS.ui, size: 11, color: COLORS.axisMuted },
    xaxis: {
      gridcolor: COLORS.grid,
      linecolor: 'transparent',
      tickcolor: 'transparent',
      zerolinecolor: COLORS.grid,
    },
    yaxis: {
      gridcolor: COLORS.grid,
      linecolor: 'transparent',
      tickcolor: 'transparent',
      zerolinecolor: COLORS.grid,
    },
    legend: {
      bgcolor: 'transparent',
      borderwidth: 0,
      font: { size: 10 },
    },
  },
};

export function baseLayout(overrides: Partial<Layout> = {}): Partial<Layout> {
  return {
    paper_bgcolor: 'transparent',
    plot_bgcolor: 'transparent',
    font: { family: FONTS.ui, size: 11, color: COLORS.axisMuted },
    margin: { l: 60, r: 20, t: 20, b: 50 },
    showlegend: true,
    legend: { x: 0.5, y: -0.15, xanchor: 'center', orientation: 'h', font: { size: 10 } },
    xaxis: {
      gridcolor: 'rgba(100, 100, 100, 0.15)',
      linecolor: 'transparent',
      zerolinecolor: 'transparent',
      tickcolor: '#444',
    },
    yaxis: {
      gridcolor: 'rgba(100, 100, 100, 0.15)',
      linecolor: 'transparent',
      zerolinecolor: 'transparent',
      tickcolor: '#444',
    },
    ...overrides,
  };
}

export const plotlyConfig: Partial<Config> = {
  displayModeBar: false,
  responsive: true,
};

export function reflLayout(overrides: Partial<Layout> = {}): Partial<Layout> {
  const base = baseLayout();
  return {
    ...base,
    xaxis: {
      ...(base.xaxis as any),
      title: { text: 'Q (Å⁻¹)', font: { size: 12 } },
      type: 'linear',
    },
    yaxis: {
      ...(base.yaxis as any),
      title: { text: 'Intensity', font: { size: 12 } },
      type: 'log',
    },
    ...overrides,
  };
}

export function edpLayout(): Partial<Layout> {
  const base = baseLayout();
  return {
    ...base,
    xaxis: {
      ...(base.xaxis as any),
      title: { text: 'Z (Å)', font: { size: 12 } },
    },
    yaxis: {
      ...(base.yaxis as any),
      title: { text: 'Electron Density (norm.)', font: { size: 12 } },
    },
  };
}
