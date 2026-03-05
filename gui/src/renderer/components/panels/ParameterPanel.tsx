import React from 'react';
import { useSettingsStore } from '../../stores/settings-store';
import type { ModelSettings } from '../../lib/types';

function Field({
  label,
  field,
  type = 'number',
  step,
}: {
  label: string;
  field: keyof ModelSettings;
  type?: 'number' | 'checkbox' | 'text';
  step?: number;
}) {
  const { settings, update } = useSettingsStore();
  const value = settings[field];

  if (type === 'checkbox') {
    return (
      <label className="flex items-center gap-2 text-xs cursor-pointer select-none">
        <input
          type="checkbox"
          checked={Boolean(value)}
          onChange={(e) => update({ [field]: e.target.checked })}
          className="w-3.5 h-3.5 accent-accent"
        />
        <span className="text-secondary">{label}</span>
      </label>
    );
  }

  return (
    <div className="flex flex-col gap-0.5">
      <label className="text-xs text-secondary">{label}</label>
      <input
        type={type}
        value={String(value)}
        step={step}
        onChange={(e) => {
          const v = type === 'number' ? parseFloat(e.target.value) : e.target.value;
          update({ [field]: v });
        }}
        className="h-7 px-2 text-xs bg-elevated border border-border rounded-input text-primary focus:outline-none focus:border-accent/50"
      />
    </div>
  );
}

function Section({ title, children }: { title: string; children: React.ReactNode }) {
  return (
    <div className="flex flex-col gap-2">
      <div className="text-xs font-semibold text-secondary uppercase tracking-wider border-b border-border pb-1">
        {title}
      </div>
      {children}
    </div>
  );
}

export function ParameterPanel() {
  return (
    <div className="flex flex-col gap-5 p-4 overflow-y-auto">
      <h2 className="text-sm font-semibold text-primary uppercase tracking-wider">Model Parameters</h2>

      <Section title="Sample">
        <Field label="Substrate SLD (×10⁻⁶ Å⁻²)" field="subSLD" step={0.01} />
        <Field label="Film SLD (×10⁻⁶ Å⁻²)" field="filmSLD" step={0.01} />
        <Field label="Superphase SLD (×10⁻⁶ Å⁻²)" field="supSLD" step={0.01} />
        <Field label="Boxes" field="boxes" step={1} />
        <Field label="Wavelength (Å)" field="wavelength" step={0.001} />
        <Field label="Total Length (Å)" field="totallength" step={1} />
        <Field label="Film Length (Å)" field="filmLength" step={1} />
        <Field label="Left Offset" field="leftoffset" step={1} />
      </Section>

      <Section title="Corrections">
        <Field label="Film Absorption" field="filmAbs" step={0.0001} />
        <Field label="Substrate Absorption" field="subAbs" step={0.0001} />
        <Field label="Superphase Absorption" field="supAbs" step={0.0001} />
        <Field label="Q Spread (σ)" field="qErr" step={0.0001} />
        <Field label="Resolution (0=none)" field="resolution" step={1} />
        <Field label="Force σ" field="forcesig" step={0.1} />
        <Field label="Crit. Edge Offset" field="critEdgeOffset" step={1} />
        <Field label="High-Q Offset" field="highQOffset" step={1} />
        <Field label="Objective Function" field="objectivefunction" step={1} />
        <Field label="Use Surf. Absorption" field="useSurfAbs" type="checkbox" />
        <Field label="Force Normalization" field="forcenorm" type="checkbox" />
        <Field label="Improved Normalization" field="impnorm" type="checkbox" />
        <Field label="XR Only" field="xrOnly" type="checkbox" />
      </Section>

      <Section title="Annealing">
        <Field label="Algorithm" field="algorithm" step={1} />
        <Field label="Initial Temp" field="inittemp" step={0.1} />
        <Field label="Plateau Iterations" field="platiter" step={1} />
        <Field label="Temp. Iterations" field="tempiter" step={1} />
        <Field label="Slope" field="slope" step={0.01} />
        <Field label="Gamma" field="gamma" step={0.01} />
        <Field label="Gamma Decrease" field="gammadec" step={0.01} />
        <Field label="STUN Function" field="stunFunc" step={1} />
        <Field label="STUN Dec. Iterations" field="stunDeciter" step={10} />
        <Field label="Param Temp" field="paramtemp" step={0.01} />
        <Field label="Sigma Search %" field="sigmasearch" step={1} />
        <Field label="Norm. Search %" field="normSearchPerc" step={1} />
        <Field label="Abs. Search %" field="absSearchPerc" step={1} />
        <Field label="Iterations" field="iterations" step={100000} />
        <Field label="Adaptive" field="adaptive" type="checkbox" />
      </Section>

      <Section title="Output">
        <Field label="Title" field="title" type="text" />
      </Section>
    </div>
  );
}
