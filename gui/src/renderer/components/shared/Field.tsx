import React, { useState, useRef, useEffect } from 'react';
import ReactDOM from 'react-dom';
import { useSettingsStore } from '../../stores/settings-store';
import type { ModelSettings } from '../../lib/types';
import { Tooltip } from './Tooltip';

export interface SelectOption {
  label: string;
  value: number;
  description?: React.ReactNode;
}

// Custom dropdown used when any option has a description field.
function DescribedSelect({
  value,
  options,
  onChange,
  disabled,
}: {
  value: number;
  options: SelectOption[];
  onChange: (v: number) => void;
  disabled?: boolean;
}) {
  const [open, setOpen] = useState(false);
  const [dropPos, setDropPos] = useState<{ top: number; left: number; width: number } | null>(null);
  const triggerRef = useRef<HTMLButtonElement>(null);

  const current = options.find((o) => o.value === value) ?? options[0];

  function handleToggle() {
    if (disabled) return;
    if (!open && triggerRef.current) {
      const rect = triggerRef.current.getBoundingClientRect();
      setDropPos({ top: rect.bottom + 2, left: rect.left, width: rect.width });
    }
    setOpen((v) => !v);
  }

  function handleSelect(v: number) {
    onChange(v);
    setOpen(false);
  }

  // Close on outside click
  useEffect(() => {
    if (!open) return;
    function onDown(e: MouseEvent) {
      if (triggerRef.current && !triggerRef.current.contains(e.target as Node)) {
        setOpen(false);
      }
    }
    document.addEventListener('mousedown', onDown);
    return () => document.removeEventListener('mousedown', onDown);
  }, [open]);

  return (
    <>
      <button
        ref={triggerRef}
        type="button"
        onClick={handleToggle}
        disabled={disabled}
        className={`h-7 px-2 text-xs bg-elevated border border-border rounded-input text-primary focus:outline-none focus:border-accent/50 cursor-pointer text-left flex items-center justify-between gap-1 w-full ${disabled ? 'opacity-50 cursor-not-allowed' : 'hover:border-accent/50'}`}
      >
        <span>{current.label}</span>
        <span className="text-secondary">▾</span>
      </button>

      {open && dropPos && ReactDOM.createPortal(
        <div
          style={{
            position: 'fixed',
            top: dropPos.top,
            left: dropPos.left,
            width: Math.max(dropPos.width, 260),
            zIndex: 9999,
          }}
          className="bg-surface border border-border rounded-input shadow-lg overflow-hidden"
        >
          {options.map((opt) => (
            <button
              key={opt.value}
              type="button"
              onClick={() => handleSelect(opt.value)}
              className={`w-full text-left px-2 py-1.5 flex flex-col gap-0.5 hover:bg-elevated transition-colors ${opt.value === value ? 'bg-accent/10' : ''}`}
            >
              <span className="text-xs text-primary">{opt.label}</span>
              {opt.description && (
                <div className="text-[10px] text-secondary leading-tight">{opt.description}</div>
              )}
            </button>
          ))}
        </div>,
        document.body,
      )}
    </>
  );
}

export function Field({
  label,
  field,
  type = 'number',
  step,
  tooltip,
  disabled,
  options,
}: {
  label: string;
  field: keyof ModelSettings;
  type?: 'number' | 'checkbox' | 'text' | 'select';
  step?: number;
  tooltip?: string;
  disabled?: boolean;
  options?: SelectOption[];
}) {
  const { settings, update } = useSettingsStore();
  const value = settings[field];

  const labelText = tooltip ? (
    <Tooltip text={tooltip}><span className="text-secondary">{label}</span></Tooltip>
  ) : (
    <span className="text-secondary">{label}</span>
  );

  if (type === 'checkbox') {
    return (
      <label className={`flex items-center gap-2 text-xs cursor-pointer select-none ${disabled ? 'opacity-50 cursor-not-allowed' : ''}`}>
        <input
          type="checkbox"
          checked={Boolean(value)}
          onChange={(e) => update({ [field]: e.target.checked })}
          disabled={disabled}
          className="w-3.5 h-3.5 accent-accent disabled:cursor-not-allowed"
        />
        {labelText}
      </label>
    );
  }

  if (type === 'select') {
    const hasDescriptions = options?.some((o) => o.description);
    return (
      <div className="flex flex-col gap-0.5">
        <label className="text-xs">{labelText}</label>
        {hasDescriptions ? (
          <DescribedSelect
            value={Number(value)}
            options={options!}
            onChange={(v) => update({ [field]: v })}
            disabled={disabled}
          />
        ) : (
          <select
            value={String(value)}
            onChange={(e) => update({ [field]: parseInt(e.target.value) })}
            disabled={disabled}
            className={`h-7 px-2 text-xs bg-elevated border border-border rounded-input text-primary focus:outline-none focus:border-accent/50 cursor-pointer ${disabled ? 'opacity-50 cursor-not-allowed' : ''}`}
          >
            {options?.map((opt) => (
              <option key={opt.value} value={opt.value}>
                {opt.label}
              </option>
            ))}
          </select>
        )}
      </div>
    );
  }

  return (
    <div className="flex flex-col gap-0.5">
      <label className="text-xs">{labelText}</label>
      <input
        type={type}
        value={String(value)}
        step={step}
        min={field === 'slope' ? '1' : undefined}
        disabled={disabled}
        onChange={(e) => {
          let v: string | number = type === 'number' ? parseFloat(e.target.value) : e.target.value;
          if (field === 'slope' && typeof v === 'number' && v < 1) v = 1;
          update({ [field]: v });
        }}
        className={`h-7 px-2 text-xs bg-elevated border border-border rounded-input text-primary focus:outline-none focus:border-accent/50 ${disabled ? 'opacity-50 cursor-not-allowed' : ''}`}
      />
    </div>
  );
}

export function Section({ title, children }: { title: string; children: React.ReactNode }) {
  return (
    <div className="flex flex-col gap-2">
      <div className="text-xs font-semibold text-secondary uppercase tracking-wider border-b border-border pb-1">
        {title}
      </div>
      {children}
    </div>
  );
}
