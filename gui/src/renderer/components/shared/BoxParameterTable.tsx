import React from 'react';

export interface BoxRow {
  length: number;
  rho: number;
  sigma: number;
}

interface Props {
  rows: BoxRow[];
  onRowChange: (index: number, row: BoxRow) => void;
  oneSigma: boolean;
}

export function BoxParameterTable({ rows, onRowChange, oneSigma }: Props) {
  return (
    <div className="overflow-x-auto">
      <table className="w-full text-xs">
        <thead>
          <tr className="text-secondary border-b border-border">
            <th className="text-left pb-1 pr-2 font-medium w-6">#</th>
            <th className="text-right pb-1 pr-1 font-medium">Length (Å)</th>
            <th className="text-right pb-1 pr-1 font-medium">ρ (norm)</th>
            <th className="text-right pb-1 font-medium">σ (Å){oneSigma && ' [locked]'}</th>
          </tr>
        </thead>
        <tbody>
          {rows.map((row, i) => (
            <tr key={i} className="border-b border-border/40">
              <td className="py-0.5 pr-2 text-secondary">{i + 1}</td>
              <td className="py-0.5 pr-1">
                <NumInput
                  value={row.length}
                  onChange={(v) => onRowChange(i, { ...row, length: v })}
                />
              </td>
              <td className="py-0.5 pr-1">
                <NumInput
                  value={row.rho}
                  onChange={(v) => onRowChange(i, { ...row, rho: v })}
                />
              </td>
              <td className="py-0.5">
                <NumInput
                  value={row.sigma}
                  onChange={(v) => onRowChange(i, { ...row, sigma: v })}
                  disabled={oneSigma}
                />
              </td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}

function NumInput({ value, onChange, disabled }: { value: number; onChange: (v: number) => void; disabled?: boolean }) {
  const [text, setText] = React.useState(String(value));
  const focused = React.useRef(false);

  React.useEffect(() => {
    if (!focused.current) setText(String(value));
  }, [value]);

  if (disabled) {
    return (
      <div className="w-full h-6 px-1 text-xs text-right bg-surface border border-border/40 rounded text-secondary/60 flex items-center justify-end">
        {value}
      </div>
    );
  }

  return (
    <input
      type="text"
      inputMode="decimal"
      value={text}
      onFocus={() => { focused.current = true; }}
      onBlur={() => {
        focused.current = false;
        const v = parseFloat(text);
        setText(isNaN(v) ? String(value) : String(v));
      }}
      onChange={(e) => {
        setText(e.target.value);
        const v = parseFloat(e.target.value);
        if (!isNaN(v)) onChange(v);
      }}
      className="w-full h-6 px-1 text-xs text-right bg-elevated border border-border rounded text-primary focus:outline-none focus:border-accent/50"
    />
  );
}
