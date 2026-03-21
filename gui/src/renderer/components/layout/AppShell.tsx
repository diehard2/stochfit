import React, { useEffect, useState, useCallback } from 'react';
import { Sidebar } from './Sidebar';
import { StatusBar } from './StatusBar';
import { DataPanel } from '../panels/DataPanel';
import { ParameterPanel } from '../panels/ParameterPanel';
import { ModelIndependentPanel } from '../panels/ModelIndependentPanel';
import { BoxModelPanel } from '../panels/BoxModelPanel';
import { ReflectivityGraph } from '../graphs/ReflectivityGraph';
import { ElectronDensityGraph } from '../graphs/ElectronDensityGraph';
import { SettingsDialog } from '../dialogs/SettingsDialog';
import { AboutDialog } from '../dialogs/AboutDialog';
import { SLDCalculatorDialog } from '../dialogs/SLDCalculatorDialog';
import { useUiStore } from '../../stores/ui-store';
import { useDataStore } from '../../stores/data-store';
import { useFitStore } from '../../stores/fit-store';
import { useBoxModelStore } from '../../stores/box-model-store';
import { useSettingsStore } from '../../stores/settings-store';
import { exportSVG } from '../graphs/export-utils';
import { MasterGraphDialog } from '../graphs/MasterPane';
import { generateReport } from '../../lib/report-generator';

function PanelContent() {
  const panel = useUiStore((s) => s.activePanel);
  switch (panel) {
    case 'data':       return <DataPanel />;
    case 'parameters': return <ParameterPanel />;
    case 'mi':         return <ModelIndependentPanel />;
    case 'boxmodel':   return <BoxModelPanel />;
    default:           return null;
  }
}

export function AppShell() {
  const data = useDataStore((s) => s.data);
  const resetSettings = useSettingsStore((s) => s.reset);
  const fitResult = useFitStore((s) => s.result);
  const miBoxED = useFitStore((s) => s.miBoxED);
  const boxModelGenRefl = useBoxModelStore((s) => s.genRefl);
  const boxModelGenED = useBoxModelStore((s) => s.genED);
  const boxModelGenBoxED = useBoxModelStore((s) => s.genBoxED);
  const boxModelGenZRange = useBoxModelStore((s) => s.genZRange);
  const { activePanel, setAboutOpen, normalizeByFresnel, setNormalizeByFresnel, setGpuAvailable, setMasterGraphOpen } = useUiStore();
  const boxModelStore = useBoxModelStore();
  const settings = useSettingsStore((s) => s.settings);
  const [reportBusy, setReportBusy] = useState(false);

  const handleGenerateReport = useCallback(async () => {
    setReportBusy(true);
    try {
      const pdfBytes = await generateReport({
        data,
        fitResult,
        settings,
        boxes: boxModelStore.boxes,
        subRough: boxModelStore.subRough,
        normFactor: boxModelStore.normFactor,
        oneSigma: boxModelStore.oneSigma,
        impNorm: boxModelStore.impNorm,
        boxRows: boxModelStore.boxRows,
        solutions: boxModelStore.solutions,
        lastFitReport: boxModelStore.lastFitReport,
        genRefl: boxModelStore.genRefl,
        genED: boxModelStore.genED,
        genBoxED: boxModelStore.genBoxED,
        genZRange: boxModelStore.genZRange,
        miBoxED,
        normalizeByFresnel,
      });
      const dir = data ? data.filePath.replace(/[^/\\]+$/, '') : '';
      const baseName = data ? data.fileName.replace(/\.[^.]+$/, '') + '-report' : 'stochfit-report';
      await window.api.openPdf(dir, baseName, pdfBytes);
    } catch (e) {
      console.error('Report generation failed:', e);
    } finally {
      setReportBusy(false);
    }
  }, [data, fitResult, settings, boxModelStore, miBoxED, normalizeByFresnel]);

  useEffect(() => {
    window.api.stochGpuAvailable().then((available) => {
      console.log('[GPU] GpuAvailable() =', available);
      setGpuAvailable(available);
    }).catch((err) => {
      console.error('[GPU] stochGpuAvailable failed:', err);
    });
  }, [setGpuAvailable]);

  // Contextual graph data based on active panel
  const isBoxModel = activePanel === 'boxmodel';
  const graphFitResult = isBoxModel ? null : fitResult;
  const graphBoxED = isBoxModel ? undefined : (miBoxED ?? undefined);
  const lmRefl = isBoxModel ? (boxModelGenRefl ?? undefined) : undefined;
  const lmQ = isBoxModel ? (data?.q ?? undefined) : undefined;

  type CtxMenu = { x: number; y: number; graphId: string; filename: string } | null;
  const [ctxMenu, setCtxMenu] = useState<CtxMenu>(null);

  const onContextMenu = useCallback((e: React.MouseEvent, graphId: string, filename: string) => {
    e.preventDefault();
    setCtxMenu({ x: e.clientX, y: e.clientY, graphId, filename });
  }, []);

  const closeMenu = useCallback(() => setCtxMenu(null), []);

  return (
    <div className="flex flex-col h-screen bg-bg text-primary overflow-hidden">
      {/* Top menu bar */}
      <div className="h-9 flex items-center justify-between px-4 border-b border-border bg-elevated flex-shrink-0">
        <div className="flex items-center gap-4">
          <button
            onClick={() => setAboutOpen(true)}
            className="text-xs text-secondary hover:text-primary transition-colors"
          >
            About
          </button>
          <button
            onClick={() => setMasterGraphOpen(true)}
            className="text-xs text-secondary hover:text-primary transition-colors"
          >
            Master Graph
          </button>
          <button
            onClick={handleGenerateReport}
            disabled={reportBusy || (!fitResult && !boxModelStore.lastFitReport)}
            className="text-xs text-secondary hover:text-primary transition-colors disabled:opacity-40 disabled:cursor-not-allowed"
          >
            {reportBusy ? 'Generating…' : 'Generate Report'}
          </button>
          <button
            onClick={async () => {
              resetSettings();
              if (data) {
                const sessionPath = data.filePath.replace(/[^/\\]+$/, '') + 'stochfit-session.json';
                await window.api.stochDeleteSession(sessionPath);
              }
            }}
            className="text-xs text-secondary hover:text-destructive transition-colors"
          >
            Reset Saved Data
          </button>
        </div>
        <label className="flex items-center gap-2 cursor-pointer">
          <input
            type="checkbox"
            checked={normalizeByFresnel}
            onChange={(e) => setNormalizeByFresnel(e.target.checked)}
            className="rounded"
          />
          <span className="text-xs text-secondary hover:text-primary transition-colors">Normalize by Fresnel</span>
        </label>
      </div>

      <div className="flex flex-1 overflow-hidden">
        {/* Sidebar nav */}
        <Sidebar />

        {/* Control panel */}
        <div className="w-72 flex-shrink-0 bg-surface border-r border-border overflow-y-auto">
          <PanelContent />
        </div>

        {/* Graph area */}
        <div className="flex-1 flex flex-col overflow-hidden p-3 gap-3" onClick={closeMenu}>
          <div id="refl-graph" className="flex-1 rounded-card bg-surface border border-border overflow-hidden min-h-0"
            onContextMenu={(e) => onContextMenu(e, 'refl-graph', 'stochfit-reflectivity')}>
            <ReflectivityGraph data={data} fitResult={graphFitResult} lmRefl={lmRefl} lmQ={lmQ} />
          </div>
          <div id="edp-graph" className="flex-1 rounded-card bg-surface border border-border overflow-hidden min-h-0"
            onContextMenu={(e) => onContextMenu(e, 'edp-graph', 'stochfit-edp')}>
            <ElectronDensityGraph
              fitResult={graphFitResult}
              boxED={graphBoxED}
              lmZRange={isBoxModel ? (boxModelGenZRange ?? undefined) : undefined}
              lmED={isBoxModel ? (boxModelGenED ?? undefined) : undefined}
              lmBoxED={isBoxModel ? (boxModelGenBoxED ?? undefined) : undefined}
            />
          </div>
        </div>
      </div>

      {ctxMenu && (
        <div
          className="fixed z-50 bg-elevated border border-border rounded shadow-lg py-1 text-xs"
          style={{ left: ctxMenu.x, top: ctxMenu.y }}
        >
          <button
            className="block w-full text-left px-4 py-1.5 text-secondary hover:text-primary hover:bg-surface transition-colors"
            onClick={() => { exportSVG(ctxMenu.graphId, ctxMenu.filename); closeMenu(); }}
          >
            Export SVG
          </button>
        </div>
      )}

      <StatusBar />

      {/* Dialogs (portaled to root) */}
      <SettingsDialog />
      <AboutDialog />
      <SLDCalculatorDialog />
      <MasterGraphDialog />
    </div>
  );
}
