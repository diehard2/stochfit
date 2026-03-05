import Plotly from 'plotly.js';

export async function exportSVG(elementId: string, filename: string): Promise<void> {
  const el = document.getElementById(elementId);
  if (!el) return;
  const svg = await Plotly.toImage(el, { format: 'svg', width: 1200, height: 800 });
  const link = document.createElement('a');
  link.href = svg;
  link.download = filename.endsWith('.svg') ? filename : `${filename}.svg`;
  link.click();
}

export async function exportPNG(elementId: string, filename: string): Promise<void> {
  const el = document.getElementById(elementId);
  if (!el) return;
  const png = await Plotly.toImage(el, { format: 'png', width: 1200, height: 800, scale: 2 });
  const link = document.createElement('a');
  link.href = png;
  link.download = filename.endsWith('.png') ? filename : `${filename}.png`;
  link.click();
}
