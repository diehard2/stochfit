import React, { useEffect, useRef, useState, useCallback } from 'react';
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
import { Toast } from '../shared/Toast';

const GITHUB_URL = 'https://github.com/diehard2/stochfit';

function MenuDropdown({ label, children }: { label: string; children: React.ReactNode }) {
  const [open, setOpen] = useState(false);
  const ref = useRef<HTMLDivElement>(null);
  useEffect(() => {
    const handler = (e: MouseEvent) => {
      if (ref.current && !ref.current.contains(e.target as Node)) setOpen(false);
    };
    document.addEventListener('mousedown', handler);
    return () => document.removeEventListener('mousedown', handler);
  }, []);
  return (
    <div className="relative" ref={ref}>
      <button
        onClick={() => setOpen((o) => !o)}
        className="text-xs text-secondary hover:text-primary transition-colors flex items-center gap-1"
      >
        {label} <span className="opacity-50 text-[10px]">▾</span>
      </button>
      {open && (
        <div className="absolute left-0 top-full mt-1 z-50 bg-elevated border border-border rounded shadow-lg py-1 min-w-40 text-xs">
          {children}
        </div>
      )}
    </div>
  );
}

function MenuItem({
  children,
  onClick,
  disabled,
  href,
}: {
  children: React.ReactNode;
  onClick?: () => void;
  disabled?: boolean;
  href?: string;
}) {
  const cls =
    'block w-full text-left px-4 py-1.5 text-secondary hover:text-primary hover:bg-surface transition-colors disabled:opacity-40 disabled:cursor-not-allowed';
  if (href) {
    return (
      <button className={cls} onClick={() => window.api.openExternal(href)}>
        {children}
      </button>
    );
  }
  return (
    <button disabled={disabled} onClick={onClick} className={cls}>
      {children}
    </button>
  );
}

function MenuSeparator() {
  return <div className="my-1 border-t border-border" />;
}

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
  const { activePanel, setAboutOpen, graphMode, setGraphMode, darkMode, setDarkMode, setGpuAvailable, setMasterGraphOpen } = useUiStore();
  const normalizeByFresnel = graphMode === 'fresnel';
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
        graphMode,
      });
      const dir = data ? data.filePath.replace(/[^/\\]+$/, '') : '';
      const baseName = data ? data.fileName.replace(/\.[^.]+$/, '') + '-report' : 'stochfit-report';
      await window.api.openPdf(dir, baseName, pdfBytes);
    } catch (e) {
      console.error('Report generation failed:', e);
    } finally {
      setReportBusy(false);
    }
  }, [data, fitResult, settings, boxModelStore, miBoxED, graphMode]);

  useEffect(() => {
    window.api.stochGpuAvailable().then((available) => {
      console.log('[GPU] GpuAvailable() =', available);
      setGpuAvailable(available);
    }).catch((err) => {
      console.error('[GPU] stochGpuAvailable failed:', err);
    });
  }, [setGpuAvailable]);

  useEffect(() => {
    document.documentElement.setAttribute('data-theme', darkMode ? 'dark' : 'light');
  }, [darkMode]);

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
          <MenuDropdown label="Reporting">
            <MenuItem onClick={() => setMasterGraphOpen(true)}>Master Graph</MenuItem>
            <MenuItem
              onClick={handleGenerateReport}
              disabled={reportBusy || (!fitResult && !boxModelStore.lastFitReport)}
            >
              {reportBusy ? 'Generating…' : 'Generate Report'}
            </MenuItem>
          </MenuDropdown>

          <MenuDropdown label="Display">
            {(['standard', 'fresnel', 'rq4'] as const).map((mode) => (
              <MenuItem key={mode} onClick={() => setGraphMode(mode)}>
                <span className="flex items-center gap-2">
                  <span className="w-3 text-accent">{graphMode === mode ? '✓' : ''}</span>
                  {mode === 'standard' ? 'Standard' : mode === 'fresnel' ? 'Fresnel Normalized' : 'R·Q⁴'}
                </span>
              </MenuItem>
            ))}
            <MenuSeparator />
            <MenuItem onClick={() => setDarkMode(!darkMode)}>
              <span className="flex items-center gap-2">
                <span className="w-3 text-accent">{!darkMode ? '✓' : ''}</span>
                Light Mode
              </span>
            </MenuItem>
          </MenuDropdown>

          <MenuDropdown label="Help">
            <MenuItem href={`${GITHUB_URL}/discussions`}>GitHub Discussions</MenuItem>
            <MenuItem href={`${GITHUB_URL}/issues`}>Report an Issue</MenuItem>
            <MenuSeparator />
            <MenuItem
              onClick={async () => {
                resetSettings();
                if (data) {
                  await window.api.stochDeleteOutput(data.filePath + '.stochfit.json');
                }
              }}
            >
              <span className="text-destructive">Reset Saved Data</span>
            </MenuItem>
            <MenuSeparator />
            <MenuItem onClick={() => setAboutOpen(true)}>About StochFit</MenuItem>
          </MenuDropdown>
        </div>
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
      <Toast />
    </div>
  );
}
