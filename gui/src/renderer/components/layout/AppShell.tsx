import React from 'react';
import { Sidebar } from './Sidebar';
import { StatusBar } from './StatusBar';
import { DataPanel } from '../panels/DataPanel';
import { ParameterPanel } from '../panels/ParameterPanel';
import { FittingPanel } from '../panels/FittingPanel';
import { RhoModelingPanel } from '../panels/RhoModelingPanel';
import { ReflModelingPanel } from '../panels/ReflModelingPanel';
import { StochResultsPanel } from '../panels/StochResultsPanel';
import { ReflectivityGraph } from '../graphs/ReflectivityGraph';
import { ElectronDensityGraph } from '../graphs/ElectronDensityGraph';
import { SettingsDialog } from '../dialogs/SettingsDialog';
import { AboutDialog } from '../dialogs/AboutDialog';
import { useUiStore } from '../../stores/ui-store';
import { useDataStore } from '../../stores/data-store';
import { useFitStore } from '../../stores/fit-store';

function PanelContent() {
  const panel = useUiStore((s) => s.activePanel);
  switch (panel) {
    case 'data':       return <DataPanel />;
    case 'parameters': return <ParameterPanel />;
    case 'fitting':    return <FittingPanel />;
    case 'rho':        return <RhoModelingPanel />;
    case 'refl':       return <ReflModelingPanel />;
    case 'stoch':      return <StochResultsPanel />;
    default:           return null;
  }
}

export function AppShell() {
  const data = useDataStore((s) => s.data);
  const fitResult = useFitStore((s) => s.result);
  const { setSettingsOpen, setAboutOpen } = useUiStore();

  return (
    <div className="flex flex-col h-screen bg-bg text-primary overflow-hidden">
      {/* Top menu bar */}
      <div className="h-9 flex items-center gap-4 px-4 border-b border-border bg-elevated flex-shrink-0">
        <span className="text-xs font-bold text-primary tracking-tight mr-2">StochFit</span>
        <button
          onClick={() => setSettingsOpen(true)}
          className="text-xs text-secondary hover:text-primary transition-colors"
        >
          Settings
        </button>
        <button
          onClick={() => setAboutOpen(true)}
          className="text-xs text-secondary hover:text-primary transition-colors"
        >
          About
        </button>
      </div>

      <div className="flex flex-1 overflow-hidden">
        {/* Sidebar nav */}
        <Sidebar />

        {/* Control panel */}
        <div className="w-72 flex-shrink-0 bg-surface border-r border-border overflow-y-auto">
          <PanelContent />
        </div>

        {/* Graph area */}
        <div className="flex-1 flex flex-col overflow-hidden p-3 gap-3">
          <div id="refl-graph" className="flex-1 rounded-card bg-surface border border-border overflow-hidden min-h-0">
            <ReflectivityGraph data={data} fitResult={fitResult} />
          </div>
          <div id="edp-graph" className="flex-1 rounded-card bg-surface border border-border overflow-hidden min-h-0">
            <ElectronDensityGraph fitResult={fitResult} />
          </div>
        </div>
      </div>

      <StatusBar />

      {/* Dialogs (portaled to root) */}
      <SettingsDialog />
      <AboutDialog />
    </div>
  );
}
