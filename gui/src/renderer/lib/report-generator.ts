import Plotly from 'plotly.js-dist-min';
import jsPDF from 'jspdf';
import autoTable from 'jspdf-autotable';
import type { Data, Layout } from 'plotly.js';
import { pubReflLayout, pubEdpLayout, pubColors, defaultPubOptions } from '../components/graphs/graph-config';
import { calcQc, calcFresnelPoint } from './constants';
import type { FitResult, ModelSettings } from './types';
import type { FitReport, BoxModelSolution } from '../stores/box-model-store';
import type { ReflData } from './types';

// ── Types ─────────────────────────────────────────────────────────────────────

export interface ReportInput {
  data: ReflData | null;
  fitResult: FitResult | null;
  settings: ModelSettings;
  // Box model
  boxes: number;
  subRough: number;
  normFactor: number;
  oneSigma: boolean;
  impNorm: boolean;
  boxRows: Array<{ length: number; rho: number; sigma: number }>;
  solutions: BoxModelSolution[];
  lastFitReport: FitReport | null;
  genRefl: number[] | null;
  genED: number[] | null;
  genBoxED: number[] | null;
  genZRange: number[] | null;
  miBoxED: number[] | null;
  // Display
  normalizeByFresnel: boolean;
}

// ── Graph rendering ───────────────────────────────────────────────────────────

async function renderGraph(traces: Data[], layout: Partial<Layout>): Promise<string | null> {
  if (traces.length === 0) return null;
  const div = document.createElement('div');
  div.style.cssText = 'width:700px;height:450px;position:fixed;top:-9999px;left:-9999px;visibility:hidden;';
  document.body.appendChild(div);
  try {
    await Plotly.newPlot(div, traces, layout, { staticPlot: true, responsive: false });
    const url = await Plotly.toImage(div, { format: 'png', width: 700, height: 450, scale: 2 });
    return url;
  } finally {
    Plotly.purge(div);
    document.body.removeChild(div);
  }
}

function applyFresnelNorm(
  q: number[], refl: number[], qc: number, errors?: number[]
): { q: number[]; refl: number[]; errors?: number[] } {
  if (qc === 0) return { q, refl, errors };
  return {
    q: q.map((qi) => qi / qc),
    refl: refl.map((ri, i) => ri / calcFresnelPoint(q[i], qc)),
    errors: errors?.map((e, i) => e / calcFresnelPoint(q[i], qc)),
  };
}

// ── jsPDF helpers ─────────────────────────────────────────────────────────────

const PAGE_W = 210;   // A4 mm
const MARGIN  = 15;
const CONTENT_W = PAGE_W - MARGIN * 2;

function addSectionHeading(doc: jsPDF, text: string, y: number, size = 14): number {
  doc.setFontSize(size);
  doc.setFont('helvetica', 'bold');
  doc.text(text, MARGIN, y);
  return y + size * 0.5 + 2;
}

function addBodyLine(doc: jsPDF, text: string, y: number, size = 9): number {
  doc.setFontSize(size);
  doc.setFont('helvetica', 'normal');
  doc.text(text, MARGIN, y);
  return y + size * 0.4 + 1.5;
}

function addImage(doc: jsPDF, dataUrl: string, y: number, heightMm = 80): number {
  const availW = CONTENT_W;
  const ratio = 700 / 450; // width/height of rendered graph
  const imgW = Math.min(availW, heightMm * ratio);
  const imgH = imgW / ratio;
  const x = MARGIN + (CONTENT_W - imgW) / 2;
  const pageH = doc.internal.pageSize.getHeight();
  if (y + imgH + 5 > pageH - MARGIN) {
    doc.addPage();
    y = MARGIN;
  }
  doc.addImage(dataUrl, 'PNG', x, y, imgW, imgH);
  return y + imgH + 5;
}

function sci(v: number): string {
  if (v === 0) return '0';
  const exp = Math.floor(Math.log10(Math.abs(v)));
  const mant = v / Math.pow(10, exp);
  return `${mant.toFixed(3)} E${exp >= 0 ? '+' : ''}${exp}`;
}

// ── Main entry ────────────────────────────────────────────────────────────────

export async function generateReport(input: ReportInput): Promise<Uint8Array> {
  const { data, fitResult, settings, normalizeByFresnel } = input;
  const opts = defaultPubOptions;
  const c = pubColors(opts);
  const qc = calcQc(settings.subSLD, settings.supSLD);
  const fresnelActive = normalizeByFresnel && qc !== 0;

  // ── Render graphs ───────────────────────────────────────────────────────────

  // MI Reflectivity
  let imgMIRefl: string | null = null;
  if (fitResult && fitResult.qRange.length > 0) {
    const traces: Data[] = [];
    if (data) {
      const n = fresnelActive ? applyFresnelNorm(data.q, data.refl, qc, data.reflError) : { q: data.q, refl: data.refl, errors: data.reflError };
      traces.push({
        x: n.q, y: n.refl,
        error_y: data.reflError.some((e) => e > 0) ? { type: 'data', array: n.errors ?? [], visible: true, color: c.errorBar, thickness: 1, width: 3 } : undefined,
        mode: 'markers', type: 'scatter', name: 'Measured',
        marker: { color: c.measured, size: 3 },
      });
    }
    const nr = fresnelActive ? applyFresnelNorm(fitResult.qRange, fitResult.refl, qc) : { q: fitResult.qRange, refl: fitResult.refl };
    traces.push({ x: nr.q, y: nr.refl, mode: 'lines', type: 'scatter', name: 'MI Fit', line: { color: c.miFit, width: opts.lineWidth, shape: 'spline' } });
    imgMIRefl = await renderGraph(traces, pubReflLayout(opts, fresnelActive, qc));
  }

  // MI EDP
  let imgMIEdp: string | null = null;
  if (fitResult && fitResult.zRange.length > 0) {
    const traces: Data[] = [
      { x: fitResult.zRange, y: fitResult.rho, mode: 'lines', type: 'scatter', name: 'MI EDP', line: { color: c.miFit, width: opts.lineWidth, shape: 'spline' } },
    ];
    if (input.miBoxED && input.miBoxED.length === fitResult.zRange.length) {
      traces.push({ x: fitResult.zRange, y: input.miBoxED, mode: 'lines', type: 'scatter', name: 'Box Overlay', line: { color: c.boxFit, width: Math.max(1, opts.lineWidth - 0.5), shape: 'hv' } });
    }
    imgMIEdp = await renderGraph(traces, pubEdpLayout(opts));
  }

  // Box Model EDP
  let imgBoxEdp: string | null = null;
  if (input.genED && input.genZRange) {
    const traces: Data[] = [
      { x: input.genZRange, y: input.genED, mode: 'lines', type: 'scatter', name: 'EDP', line: { color: c.miFit, width: opts.lineWidth, shape: 'spline' } },
    ];
    if (input.genBoxED && input.genBoxED.length === input.genZRange.length) {
      traces.push({ x: input.genZRange, y: input.genBoxED, mode: 'lines', type: 'scatter', name: 'Boxes', line: { color: c.boxFit, width: Math.max(1, opts.lineWidth - 0.5), shape: 'hv' } });
    }
    imgBoxEdp = await renderGraph(traces, pubEdpLayout(opts));
  }

  // Box Model Reflectivity
  let imgBoxRefl: string | null = null;
  if (data && input.genRefl && input.genRefl.length === data.q.length) {
    const traces: Data[] = [];
    const nm = fresnelActive ? applyFresnelNorm(data.q, data.refl, qc, data.reflError) : { q: data.q, refl: data.refl, errors: data.reflError };
    traces.push({
      x: nm.q, y: nm.refl,
      error_y: data.reflError.some((e) => e > 0) ? { type: 'data', array: nm.errors ?? [], visible: true, color: c.errorBar, thickness: 1, width: 3 } : undefined,
      mode: 'markers', type: 'scatter', name: 'Measured',
      marker: { color: c.measured, size: 3 },
    });
    const nb = fresnelActive ? applyFresnelNorm(data.q, input.genRefl, qc) : { q: data.q, refl: input.genRefl };
    traces.push({ x: nb.q, y: nb.refl, mode: 'lines', type: 'scatter', name: 'Box Model Fit', line: { color: c.modelFit, width: opts.lineWidth } });
    imgBoxRefl = await renderGraph(traces, pubReflLayout(opts, fresnelActive, qc));
  }

  // ── Assemble PDF ────────────────────────────────────────────────────────────

  const doc = new jsPDF({ unit: 'mm', format: 'a4' });
  const pageH = doc.internal.pageSize.getHeight();
  let y = MARGIN;

  // Header
  doc.setFontSize(18);
  doc.setFont('helvetica', 'bold');
  doc.text('StochFit Modeling Report', MARGIN, y);
  y += 9;
  doc.setFontSize(10);
  doc.setFont('helvetica', 'normal');
  const now = new Date();
  doc.text(now.toLocaleString(), MARGIN, y);
  y += 8;

  // ── Section 1: Model Independent Parameters ────────────────────────────────
  if (fitResult) {
    y = addSectionHeading(doc, 'Model Independent Parameters', y, 14);
    y += 1;

    const lines = [
      data ? `Data file: ${data.fileName}` : '',
      `Substrate SLD: ${settings.subSLD}`,
      `Superphase SLD: ${settings.supSLD}`,
      settings.neutron ? '' : `Film SLD: ${settings.filmSLD}`,
      `Wavelength: ${settings.wavelength} Å`,
      `Q spread: ${settings.qSpread}%`,
      `Film boxes: ${settings.boxes}`,
      `Film length: ${settings.filmLength} Å`,
      `Subphase roughness: ${sci(fitResult.roughness)} Å`,
      `Chi-square: ${sci(fitResult.chiSquare)}`,
      `Goodness of Fit: ${sci(fitResult.goodnessOfFit)}`,
    ].filter(Boolean);

    for (const line of lines) y = addBodyLine(doc, line, y);
    y += 3;

    if (imgMIRefl) y = addImage(doc, imgMIRefl, y);
    if (imgMIEdp)  y = addImage(doc, imgMIEdp, y);

    if (input.lastFitReport || imgBoxRefl) {
      doc.addPage();
      y = MARGIN;
    }
  }

  // ── Section 2: Box Model Reflectivity Fit ─────────────────────────────────
  const { lastFitReport, oneSigma, impNorm, boxRows, boxes, subRough } = input;
  const hasBoxFit = !!(lastFitReport || imgBoxRefl);

  if (hasBoxFit) {
    y = addSectionHeading(doc, 'Reflectivity Non-Linear Regression Fit', y, 14);
    y += 1;

    const covar = lastFitReport?.covariance ?? [];
    const chi = lastFitReport ? lastFitReport.info[1] : null;
    const arrayconst = oneSigma ? 2 : 3;

    const infoLines = [
      oneSigma
        ? 'The reflectivity curve was fit with a single roughness parameter'
        : `The reflectivity curve was fit with ${boxes + 1} roughness parameters`,
      `Percent Error in Q: ${settings.qSpread}`,
      `Normalization Factor: ${input.normFactor.toFixed(6)}`,
      `Low Q Offset: ${settings.critEdgeOffset}`,
      `High Q Offset: ${settings.highQOffset}`,
      `Superphase SLD: ${settings.supSLD}`,
      `Subphase SLD: ${settings.subSLD}`,
      `Wavelength: ${settings.wavelength} Å`,
      chi !== null ? `Chi Square: ${sci(chi)}` : '',
      covar.length > 0 ? `Subphase roughness: ${sci(subRough)} \u00B1 ${sci(covar[0])}` : `Subphase roughness: ${sci(subRough)}`,
    ].filter(Boolean);

    for (const line of infoLines) y = addBodyLine(doc, line, y);
    y += 3;

    if (imgBoxRefl) y = addImage(doc, imgBoxRefl, y);
    if (imgBoxEdp)  y = addImage(doc, imgBoxEdp, y);

    // Layer table
    if (boxRows.length > 0) {
      if (y + 30 > pageH - MARGIN) { doc.addPage(); y = MARGIN; }

      const useSLD = settings.neutron;
      const header = ['Layer #', 'Length (Å)', useSLD ? 'SLD' : 'Rho/Rho(\u221E)', 'Sigma (Å)'];
      const rows: string[][] = boxRows.slice(0, boxes).map((row, i) => {
        const li = arrayconst * i + 1;
        const ri = arrayconst * i + 2;
        const si = oneSigma ? 0 : arrayconst * i + 3;

        const lenStr = covar.length > li ? `${sci(row.length)} \u00B1 ${sci(covar[li])}` : sci(row.length);
        const rhoVal = useSLD ? row.rho * settings.subSLD : row.rho;
        const rhoErr = (useSLD && covar.length > ri) ? covar[ri] * settings.subSLD : (covar[ri] ?? 0);
        const rhoStr = covar.length > ri ? `${sci(rhoVal)} \u00B1 ${sci(rhoErr)}` : sci(rhoVal);
        const sigErr = covar.length > si ? covar[si] : 0;
        const sigStr = covar.length > si ? `${sci(row.sigma)} \u00B1 ${sci(sigErr)}` : sci(row.sigma);

        return [`${i + 1}`, lenStr, rhoStr, sigStr];
      });

      autoTable(doc, {
        startY: y,
        head: [header],
        body: rows,
        margin: { left: MARGIN, right: MARGIN },
        styles: { fontSize: 7, cellPadding: 1.5 },
        headStyles: { fillColor: [220, 220, 220], textColor: 0, fontStyle: 'bold' },
        columnStyles: { 0: { cellWidth: 15, halign: 'center' } },
      });
      y = (doc as any).lastAutoTable.finalY + 5;
    }
  }

  return doc.output('arraybuffer') as unknown as Uint8Array;
}
