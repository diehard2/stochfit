import React from 'react';
import { useUiStore } from '../../stores/ui-store';

export function Toast() {
  const toast = useUiStore((s) => s.toast);

  if (!toast) return null;

  return (
    <div className="fixed bottom-6 left-1/2 -translate-x-1/2 z-50 pointer-events-none">
      <div className="bg-surface border border-border rounded-card px-4 py-2 shadow-subtle text-xs text-primary animate-fade-in">
        {toast}
      </div>
    </div>
  );
}
