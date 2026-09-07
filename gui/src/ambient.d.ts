// Ambient module declarations. This file must stay a global script (no
// top-level import/export) — TS 6 only applies these globally when the
// declaring file itself isn't an ES module. See typings.d.ts for the
// module-scoped Window/global augmentations.

// plotly.js-dist-min ships no .d.ts — point it at the @types/plotly.js declarations.
declare module 'plotly.js-dist-min' {
  import * as PlotlyType from 'plotly.js';
  export = PlotlyType;
}

declare module '*.css';
