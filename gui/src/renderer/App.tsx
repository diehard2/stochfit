import React, { useEffect } from 'react';
import { AppShell } from './components/layout/AppShell';
import { useSettingsStore } from './stores/settings-store';

export function App() {
  const reset = useSettingsStore((s) => s.reset);

  useEffect(() => {
    const cleanup = window.api.onSettingsReset(() => reset());
    return cleanup;
  }, [reset]);

  return <AppShell />;
}
