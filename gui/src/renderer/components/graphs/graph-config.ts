import type { Layout, Config } from 'plotly.js';
import { COLORS, FONTS } from '../../lib/constants';

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
  displayModeBar: true,
  modeBarButtonsToRemove: ['select2d', 'lasso2d'],
  displaylogo: false,
  responsive: true,
  toImageButtonOptions: { format: 'svg', width: 1200, height: 800 },
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
