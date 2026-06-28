import path from 'path';
import type { ReflData, ReflDataMeta } from '../../renderer/lib/types';

// ORSO YAML paths we care about (dot-joined key path → display label)
const ORSO_WANTED: Record<string, string> = {
  'data_source.experiment.title':      'Title',
  'data_source.experiment.instrument': 'Instrument',
  'data_source.experiment.start_date': 'Date',
  'data_source.sample.name':           'Sample',
  'data_source.measurement.instrument_settings.wavelength.magnitude': 'Wavelength',
  'data_source.measurement.instrument_settings.wavelength.unit':      '_wavelengthUnit',
};

/**
 * Walk the `# YAML` header of an ORSO file and extract known metadata fields.
 * Each header line has the form `#<indent>key: value` — we rebuild the YAML path
 * from indentation depth and match against ORSO_WANTED.
 */
function extractOrsoMetadata(lines: string[]): ReflDataMeta[] {
  // Convert each `# ...` line into {indent, trimmed}
  const headerItems = lines
    .map(l => {
      const s = l.trimStart();
      if (!s.startsWith('#')) return null;
      const afterHash = s.slice(1);           // drop leading `#`
      const indent = afterHash.search(/\S/);  // spaces after `#`
      const trimmed = afterHash.trim();
      return { indent: indent < 0 ? 0 : indent, trimmed };
    })
    .filter((x): x is { indent: number; trimmed: string } =>
      x !== null && !!x.trimmed && !x.trimmed.startsWith('#'));

  const stack: { indent: number; key: string }[] = [];
  const collected: Record<string, string> = {};

  for (const { indent, trimmed } of headerItems) {
    const colonIdx = trimmed.indexOf(':');
    if (colonIdx < 0) continue;

    const key   = trimmed.slice(0, colonIdx).trim();
    const value = trimmed.slice(colonIdx + 1).trim();

    // Pop entries with indent >= current (same level or deeper are no longer parent)
    while (stack.length > 0 && stack[stack.length - 1].indent >= indent) stack.pop();

    const dotPath = [...stack.map(s => s.key), key].join('.');

    if (value) {
      if (dotPath in ORSO_WANTED) collected[dotPath] = value;
    } else {
      stack.push({ indent, key });
    }
  }

  // Build final list, combining wavelength magnitude+unit when both present
  const meta: ReflDataMeta[] = [];
  for (const [dotPath, label] of Object.entries(ORSO_WANTED)) {
    if (label.startsWith('_')) continue; // internal
    const v = collected[dotPath];
    if (!v) continue;
    if (dotPath.endsWith('.magnitude')) {
      const unitPath = dotPath.replace('.magnitude', '.unit');
      const unit = collected[unitPath];
      meta.push({ label, value: unit ? `${v} ${unit}` : v });
    } else {
      meta.push({ label, value: v });
    }
  }
  return meta;
}

/**
 * Parse an ORSO .ort reflectivity file.
 * Format: YAML header lines starting with '#', followed by whitespace-delimited data.
 * ORSO column order: Qz  R  sigma_R  [dQz]
 * See: https://www.reflectometry.org/advanced_and_expert_level/file_format
 */
export function parseOrsoFile(content: string, filePath: string): ReflData {
  const lines = content.split('\n');
  const q: number[] = [];
  const refl: number[] = [];
  const reflError: number[] = [];
  const qError: number[] = [];

  for (const rawLine of lines) {
    const line = rawLine.trim();
    // Skip YAML header lines and blank lines
    if (!line || line.startsWith('#')) continue;

    const parts = line.split(/\s+/);
    if (parts.length < 2) continue;

    const qv = parseFloat(parts[0]);
    const rv = parseFloat(parts[1]);
    if (!isFinite(qv) || !isFinite(rv)) continue;

    q.push(qv);
    refl.push(rv);
    reflError.push(parts.length >= 3 ? parseFloat(parts[2]) || 0 : 0);
    qError.push(parts.length >= 4 ? parseFloat(parts[3]) || 0 : 0);
  }

  if (q.length === 0) throw new Error('No valid data rows found in ORSO file');

  const metadata = extractOrsoMetadata(lines);

  return {
    q,
    refl,
    reflError,
    qError,
    filePath,
    fileName: path.basename(filePath),
    ...(metadata.length > 0 ? { metadata } : {}),
  };
}
