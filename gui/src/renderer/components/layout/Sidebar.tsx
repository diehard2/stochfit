import React from 'react';
import { useUiStore, type ActivePanel } from '../../stores/ui-store';
import { useDataStore } from '../../stores/data-store';
import { useFitStore } from '../../stores/fit-store';

interface NavItem {
  id: ActivePanel;
  label: string;
  icon: string;
  requiresData?: boolean;
  requiresFit?: boolean;
}

const NAV_ITEMS: NavItem[] = [
  { id: 'data', label: 'Data', icon: '📊' },
  { id: 'parameters', label: 'Parameters', icon: '⚙️', requiresData: true },
  { id: 'fitting', label: 'Fitting', icon: '▶️', requiresData: true },
  { id: 'rho', label: 'Rho Model', icon: '🔲', requiresFit: true },
  { id: 'refl', label: 'Refl Model', icon: '〰️', requiresFit: true },
  { id: 'stoch', label: 'Solutions', icon: '📋', requiresFit: true },
];

export function Sidebar() {
  const { activePanel, setActivePanel } = useUiStore();
  const hasData = !!useDataStore((s) => s.data);
  const hasFit = !!useFitStore((s) => s.result);

  return (
    <aside className="w-48 flex-shrink-0 bg-elevated border-r border-border flex flex-col">
      {/* App title */}
      <div className="h-12 flex items-center px-4 border-b border-border">
        <span className="text-sm font-bold text-primary tracking-tight">StochFit</span>
      </div>

      {/* Nav */}
      <nav className="flex-1 py-2 overflow-y-auto">
        {NAV_ITEMS.map((item) => {
          const disabled = (item.requiresData && !hasData) || (item.requiresFit && !hasFit);
          const active = activePanel === item.id;
          return (
            <button
              key={item.id}
              onClick={() => !disabled && setActivePanel(item.id)}
              disabled={disabled}
              className={`w-full flex items-center gap-2.5 px-4 py-2 text-xs transition-colors text-left ${
                active
                  ? 'bg-accent/15 text-accent border-r-2 border-accent'
                  : disabled
                  ? 'text-secondary/40 cursor-not-allowed'
                  : 'text-secondary hover:bg-surface hover:text-primary'
              }`}
            >
              <span className="text-sm leading-none">{item.icon}</span>
              <span>{item.label}</span>
            </button>
          );
        })}
      </nav>

      {/* Footer */}
      <div className="h-8 flex items-center px-4 border-t border-border">
        <span className="text-xs text-secondary/40">v1.0.0</span>
      </div>
    </aside>
  );
}
