// Factory-based Plot component: uses plotly.js-dist-min (browser-safe UMD) instead
// of the full plotly.js CJS entry, which pulls in Node.js built-ins incompatible with Vite.
import Plotly from 'plotly.js-dist-min';
import createPlotlyComponent from 'react-plotly.js/factory';

export default createPlotlyComponent(Plotly);
