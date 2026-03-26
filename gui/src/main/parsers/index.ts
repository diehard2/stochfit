import fs from 'fs';
import path from 'path';
import type { ReflData } from '../../renderer/lib/types';
import type { StochFitOutput } from '../native/stochfit-api';
import { readOutputFile, outputFilePath } from '../native/stochfit-api';
import { parseNexusFile } from './nexus-parser';
import { parseOrsoFile } from './orso-parser';
import { parseTextReflData } from './text-parser';

export interface OpenDataResult {
  data: ReflData;
  savedOutput?: StochFitOutput;
}

/**
 * Parse a reflectivity data file, selecting the parser based on file extension.
 * Also checks for a corresponding .stochfit.json output file alongside the data.
 *
 *   .nxs / .h5 / .hdf5 / .hdf  → NXcanSAS HDF5 parser
 *   .ort                        → ORSO text parser
 *   everything else             → whitespace-delimited text parser
 */
export async function parseDataFile(filePath: string): Promise<OpenDataResult> {
  const ext = path.extname(filePath).toLowerCase();
  let data: ReflData;

  if (ext === '.nxs' || ext === '.h5' || ext === '.hdf5' || ext === '.hdf') {
    data = await parseNexusFile(filePath);
  } else {
    const content = fs.readFileSync(filePath, 'utf-8');
    data = ext === '.ort'
      ? parseOrsoFile(content, filePath)
      : parseTextReflData(content, filePath);
  }

  const savedOutput = readOutputFile(outputFilePath(filePath)) ?? undefined;
  return { data, savedOutput };
}
