import type { ReflData } from './types';

export function applyForceNormalization(d: ReflData, on: boolean): ReflData {
  if (!on || d.refl.length === 0) return d;
  const s = 1.0 / d.refl[0];
  return {
    ...d,
    refl: d.refl.map(v => v * s),
    reflError: d.reflError.map(v => v * s),
  };
}
