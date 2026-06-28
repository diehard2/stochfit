import Plotly from 'plotly.js-dist-min';

export async function exportSVG(elementId: string, filename: string): Promise<void> {
  const el = document.getElementById(elementId);
  if (!el) return;
  const gd = (el.querySelector('.js-plotly-plot') as HTMLElement) ?? el;
  const svg = await Plotly.toImage(gd, { format: 'svg', width: 1200, height: 800 });
  const link = document.createElement('a');
  link.href = svg;
  link.download = filename.endsWith('.svg') ? filename : `${filename}.svg`;
  link.click();
}

export async function exportPNG(elementId: string, filename: string): Promise<void> {
  const el = document.getElementById(elementId);
  if (!el) return;
  const gd = (el.querySelector('.js-plotly-plot') as HTMLElement) ?? el;
  const png = await Plotly.toImage(gd, { format: 'png', width: 1200, height: 800, scale: 2 });
  const link = document.createElement('a');
  link.href = png;
  link.download = filename.endsWith('.png') ? filename : `${filename}.png`;
  link.click();
}
