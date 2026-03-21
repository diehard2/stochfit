// plotly.js-dist-min ships no .d.ts — point it at the @types/plotly.js declarations.
declare module 'plotly.js-dist-min' {
  import * as PlotlyType from 'plotly.js';
  export = PlotlyType;
}
