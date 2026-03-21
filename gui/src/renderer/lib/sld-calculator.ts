// Element database: atomic weight (g/mol), electron count Z, bound coherent scattering length b (fm, NIST)
const ELEMENTS: Record<string, { weight: number; Z: number; b: number }> = {
  H:  { weight:  1.008,  Z:  1, b: -3.7390 },
  D:  { weight:  2.014,  Z:  1, b:  6.671  },
  C:  { weight: 12.011,  Z:  6, b:  6.646  },
  N:  { weight: 14.007,  Z:  7, b:  9.36   },
  O:  { weight: 15.999,  Z:  8, b:  5.803  },
  F:  { weight: 18.998,  Z:  9, b:  5.654  },
  Na: { weight: 22.990,  Z: 11, b:  3.63   },
  Mg: { weight: 24.305,  Z: 12, b:  5.375  },
  Al: { weight: 26.982,  Z: 13, b:  3.449  },
  Si: { weight: 28.086,  Z: 14, b:  4.1491 },
  P:  { weight: 30.974,  Z: 15, b:  5.13   },
  S:  { weight: 32.06,   Z: 16, b:  2.847  },
  Cl: { weight: 35.45,   Z: 17, b:  9.577  },
  K:  { weight: 39.098,  Z: 19, b:  3.67   },
  Ca: { weight: 40.078,  Z: 20, b:  4.70   },
  Ti: { weight: 47.867,  Z: 22, b: -3.438  },
  Cr: { weight: 51.996,  Z: 24, b:  3.635  },
  Fe: { weight: 55.845,  Z: 26, b:  9.45   },
  Co: { weight: 58.933,  Z: 27, b:  2.49   },
  Ni: { weight: 58.693,  Z: 28, b: 10.3    },
  Cu: { weight: 63.546,  Z: 29, b:  7.718  },
  Zn: { weight: 65.38,   Z: 30, b:  5.680  },
  Ge: { weight: 72.630,  Z: 32, b:  8.185  },
  As: { weight: 74.922,  Z: 33, b:  6.58   },
};

// Subscript digit map for display (₀₁₂…₉) and reverse for parsing
const SUB: Record<string, string> = {
  '0':'₀','1':'₁','2':'₂','3':'₃','4':'₄',
  '5':'₅','6':'₆','7':'₇','8':'₈','9':'₉',
};
const UNSUB: Record<string, string> = Object.fromEntries(
  Object.entries(SUB).map(([k, v]) => [v, k])
);

// Strip subscript digits back to ASCII so the parser can handle display strings
function desubscript(s: string): string {
  return Array.from(s).map(c => UNSUB[c] ?? c).join('');
}

// Process a raw input string into display format (1:1 char replacement so cursor is preserved):
//   • ASCII digits → subscript equivalents
//   • Letters → proper element case (capitalise first, try 2-char known element)
//   • Parens / unknowns → pass through unchanged
export function processInput(raw: string): string {
  // normalise any subscripts already present (e.g. from paste)
  const s = desubscript(raw);
  let out = '';
  let i = 0;
  while (i < s.length) {
    const ch = s[i];
    if (ch >= '0' && ch <= '9') {
      out += SUB[ch];
      i++;
    } else if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
      const sym1 = ch.toUpperCase();
      // try 2-char element (next char must be a letter)
      if (i + 1 < s.length) {
        const next = s[i + 1];
        if ((next >= 'a' && next <= 'z') || (next >= 'A' && next <= 'Z')) {
          const candidate = sym1 + next.toLowerCase();
          if (candidate in ELEMENTS) {
            out += candidate;
            i += 2;
            continue;
          }
        }
      }
      out += sym1;
      i++;
    } else {
      out += ch; // parens, spaces, etc.
      i++;
    }
  }
  return out;
}

export type AtomCounts = Record<string, number>;

export interface ParseResult {
  counts: AtomCounts;
  unknown: string[];   // elements not in the database
}

// Parse a chemical formula string into atom counts.
// Supports: "H2O", "Ca(OH)2", "C16H32O2", nested parens.
// Returns counts for all tokens including unknowns; caller checks unknown[].
export function parseFormula(input: string): ParseResult | null {
  if (!input.trim()) return null;
  const formula = desubscript(input); // accept display strings with subscript digits

  let pos = 0;

  function isLetter(ch: string) {
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
  }

  // Consume an element symbol at the current position.
  // Capitalises first char, tries 2-char match (known element) before falling back to 1-char.
  function consumeSymbol(): string {
    const sym1 = formula[pos].toUpperCase();
    pos++;
    if (pos < formula.length && formula[pos] >= 'a' && formula[pos] <= 'z') {
      const candidate = sym1 + formula[pos];
      if (candidate in ELEMENTS) {
        pos++;
        return candidate;
      }
    }
    return sym1;
  }

  function parseNumber(): number {
    let s = '';
    while (pos < formula.length && formula[pos] >= '0' && formula[pos] <= '9') {
      s += formula[pos++];
    }
    return s ? parseInt(s, 10) : 1;
  }

  function parseGroup(): AtomCounts | null {
    const result: AtomCounts = {};

    while (pos < formula.length) {
      const ch = formula[pos];

      if (ch === '(') {
        pos++;
        const inner = parseGroup();
        if (!inner) return null;
        if (pos >= formula.length || formula[pos] !== ')') return null;
        pos++;
        const mult = parseNumber();
        for (const [el, n] of Object.entries(inner)) {
          result[el] = (result[el] ?? 0) + n * mult;
        }
      } else if (ch === ')') {
        break;
      } else if (isLetter(ch)) {
        const sym = consumeSymbol();
        const count = parseNumber();
        result[sym] = (result[sym] ?? 0) + count;
      } else {
        return null;
      }
    }

    return result;
  }

  const counts = parseGroup();
  if (!counts || pos !== formula.length) return null;

  const unknown = Object.keys(counts).filter((el) => !(el in ELEMENTS));
  return { counts, unknown };
}

// Molecular weight in g/mol
export function molecularWeight(counts: AtomCounts): number {
  return Object.entries(counts).reduce((sum, [el, n]) => {
    return sum + (ELEMENTS[el]?.weight ?? 0) * n;
  }, 0);
}

// Total electron count
export function electronCount(counts: AtomCounts): number {
  return Object.entries(counts).reduce((sum, [el, n]) => {
    return sum + (ELEMENTS[el]?.Z ?? 0) * n;
  }, 0);
}

// Bound coherent scattering length sum in fm
export function bSum(counts: AtomCounts): number {
  return Object.entries(counts).reduce((sum, [el, n]) => {
    return sum + (ELEMENTS[el]?.b ?? 0) * n;
  }, 0);
}

// Number density from bulk density: molecules / ų
// = density(g/cm³) × N_A(6.022e23) × 1e-24(cm³→ų) / MW(g/mol)
export function mwvolFromDensity(counts: AtomCounts, density: number): number {
  const mw = molecularWeight(counts);
  if (mw <= 0) return 0;
  return (density * 6.022e23 * 1e-24) / mw;
}

// Number density from box dimensions: molecules / ų
// = 1 / (area(Ų) × thickness(Å))
export function mwvolFromBox(area: number, thickness: number): number {
  if (area <= 0 || thickness <= 0) return 0;
  return 1 / (area * thickness);
}

// X-ray SLD in ×10⁻⁶ Å⁻²
// = mwvol × ΣZ × r_e(2.8179×10⁻⁵ Å) × 10⁶
export function xraySLD(mwvol: number, counts: AtomCounts): number {
  return mwvol * electronCount(counts) * 0.000028179 * 1e6;
}

// Neutron SLD in ×10⁻⁶ Å⁻²
// = mwvol(molec/ų) × Σb(fm=10⁻⁵Å) × 10⁻⁵ × 10⁶ = mwvol × Σb × 10
export function neutronSLD(mwvol: number, counts: AtomCounts): number {
  return mwvol * bSum(counts) * 10;
}
