// Graph color palette
export const COLORS = {
  measured: '#8B8CF8',      // soft indigo — data points
  miFit: '#34D399',         // emerald — MI fit curve
  modelFit: '#F472B6',      // soft pink — model fit
  boxFit: '#FB923C',        // warm amber — box stepped fit
  errorBar: 'rgba(139,140,248,0.3)',
  grid: 'rgba(255,255,255,0.06)',
  gridLight: 'rgba(0,0,0,0.06)',
  axisMuted: '#888893',
} as const;

// Publication mode colors (B&W)
export const PUB_COLORS = {
  data: '#000000',
  fit: '#333333',
  fitAlt: '#666666',
} as const;

// Font stacks
export const FONTS = {
  ui: 'Inter, system-ui, sans-serif',
  publication: 'Garamond, Georgia, serif',
} as const;

export const POLLING_INTERVAL_MS = 2000;

// Fresnel normalization helpers
export function calcQc(subSLD: number, supSLD: number): number {
  if (subSLD - supSLD > 0) {
    return 4 * Math.sqrt(Math.PI * (subSLD - supSLD) * 1e-6);
  }
  return 0;
}

export function calcFresnelPoint(Q: number, Qc: number): number {
  if (Q <= Qc) {
    return 1;
  }
  const term1 = Math.sqrt(1 - Math.pow(Qc / Q, 2));
  return Math.pow((1 - term1) / (1 + term1), 2);
}
