import React from 'react';
import { useFitStore } from '../../stores/fit-store';
import { useDataStore } from '../../stores/data-store';

export function StatusBar() {
  const { result, saParams, status } = useFitStore();
  const data = useDataStore((s) => s.data);

  return (
    <div className="h-8 flex items-center gap-6 px-4 border-t border-border bg-elevated text-xs text-secondary font-mono flex-shrink-0">
      {data && (
        <span>
          <span className="text-secondary/60">pts </span>
          {data.q.length}
        </span>
      )}
      {result && (
        <>
          <span>
            <span className="text-secondary/60">χ² </span>
            <span className="text-accent">{result.chiSquare.toExponential(3)}</span>
          </span>
          <span>
            <span className="text-secondary/60">σ </span>
            {result.roughness.toFixed(2)} Å
          </span>
        </>
      )}
      {saParams && (
        <>
          <span>
            <span className="text-secondary/60">T </span>
            {saParams.temp.toExponential(2)}
          </span>
        </>
      )}
      <span className="ml-auto capitalize">{status === 'idle' ? '' : status}</span>
    </div>
  );
}
