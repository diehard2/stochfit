import path from 'path';
import type { ReflData } from '../../renderer/lib/types';

/**
 * Parse a whitespace-delimited reflectivity data file.
 * Columns: Q  R  [Rerr  [Qerr]]
 * Lines starting with # or ! are treated as comments and skipped.
 */
export function parseTextReflData(content: string, filePath: string): ReflData {
  const q: number[] = [];
  const refl: number[] = [];
  const reflError: number[] = [];
  const qError: number[] = [];

  for (const rawLine of content.split('\n')) {
    const line = rawLine.trim();
    if (!line || line.startsWith('#') || line.startsWith('!')) continue;

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

  if (q.length === 0) throw new Error('No valid data rows found in file');

  return {
    q,
    refl,
    reflError,
    qError,
    filePath,
    fileName: path.basename(filePath),
  };
}
