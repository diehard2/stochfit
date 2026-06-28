import type { BoxRow } from '../components/shared/BoxParameterTable';

// Compute a pure step-function box EDP in the frontend.
// Avoids the erf midpoint artifact (erf(0)=0 → 0.5) that appears when z falls
// exactly on an interface in the C++ RhoGenerate calculation.
export function computeBoxStepEDP(
  zRange: number[],
  rows: BoxRow[],
  zOffset: number,
  supSLD: number,
  subSLD: number,
): number[] {
  const boundaries: number[] = [zOffset];
  for (const row of rows) {
    boundaries.push(boundaries[boundaries.length - 1] + row.length);
  }
  const supNorm = subSLD !== 0 ? supSLD / subSLD : 0;
  return zRange.map(z => {
    if (z < boundaries[0]) return supNorm;
    for (let i = 0; i < rows.length; i++) {
      if (z < boundaries[i + 1]) return rows[i].rho;
    }
    return 1.0;
  });
}
