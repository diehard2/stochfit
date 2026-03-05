import React from 'react';
import { ReflectivityGraph } from './ReflectivityGraph';
import { ElectronDensityGraph } from './ElectronDensityGraph';
import { exportSVG } from './export-utils';
import { useDataStore } from '../../stores/data-store';
import { useFitStore } from '../../stores/fit-store';

export function MasterPane() {
  const data = useDataStore((s) => s.data);
  const fitResult = useFitStore((s) => s.result);

  return (
    <div className="flex flex-col h-full gap-3 p-3">
      <div className="flex items-center justify-between flex-shrink-0">
        <span className="text-xs font-semibold text-secondary uppercase tracking-wider">Publication View</span>
        <div className="flex gap-2">
          <button
            onClick={() => exportSVG('refl-graph', 'stochfit-reflectivity')}
            className="px-2 py-1 text-xs bg-elevated border border-border rounded-sm text-secondary hover:text-primary transition-colors"
          >
            Export Refl SVG
          </button>
          <button
            onClick={() => exportSVG('edp-graph', 'stochfit-edp')}
            className="px-2 py-1 text-xs bg-elevated border border-border rounded-sm text-secondary hover:text-primary transition-colors"
          >
            Export EDP SVG
          </button>
        </div>
      </div>

      <div className="flex gap-3 flex-1 min-h-0">
        <div id="refl-graph" className="flex-1 rounded-card bg-surface border border-border overflow-hidden">
          <ReflectivityGraph data={data} fitResult={fitResult} />
        </div>
        <div id="edp-graph" className="flex-1 rounded-card bg-surface border border-border overflow-hidden">
          <ElectronDensityGraph fitResult={fitResult} />
        </div>
      </div>
    </div>
  );
}
