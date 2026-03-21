import React from 'react';
import { Field, Section } from '../shared/Field';
import { useFitStore } from '../../stores/fit-store';
import { useUiStore } from '../../stores/ui-store';
import { useSettingsStore } from '../../stores/settings-store';

export function ParameterPanel() {
  const { status } = useFitStore();
  const isRunning = status === 'running';
  const setSldCalcOpen = useUiStore((s) => s.setSldCalcOpen);
  const isNeutron = useSettingsStore((s) => s.settings.neutron);
  const isUseSurfAbs = useSettingsStore((s) => s.settings.useSurfAbs);

  return (
    <div className="flex flex-col gap-5 p-4 overflow-y-auto">
      <h2 className="text-sm font-semibold text-primary uppercase tracking-wider">Model Parameters</h2>

      <Section title="Sample">
        <Field label="Substrate SLD (×10⁻⁶ Å⁻²)" field="subSLD" step={0.01} disabled={isRunning} />
        <Field label="Film SLD (×10⁻⁶ Å⁻²)" field="filmSLD" step={0.01} disabled={isRunning} />
        <Field label="Superphase SLD (×10⁻⁶ Å⁻²)" field="supSLD" step={0.01} disabled={isRunning} />
        <Field label="Wavelength (Å)" field="wavelength" step={0.001} disabled={isRunning} />
        <Field label="Film Length (Å)" field="filmLength" step={1} disabled={isRunning} />
        <Field label="Neutron" field="neutron" type="checkbox" tooltip="Neutron reflectivity mode. Switches SLD calculator to neutron SLD." disabled={isRunning} />
        <button
          onClick={() => setSldCalcOpen(true)}
          className="text-xs text-accent hover:text-primary transition-colors mt-1 text-left"
        >
          SLD Calculator…
        </button>
      </Section>

      <Section title="Corrections">
        <Field label="Absorption" field="useSurfAbs" type="checkbox" tooltip="Include absorption in reflectivity calculation." disabled={isRunning} />
        {isUseSurfAbs && <Field label="Film Absorption" field="filmAbs" step={0.0001} disabled={isRunning} />}
        {isUseSurfAbs && <Field label="Substrate Absorption" field="subAbs" step={0.0001} disabled={isRunning} />}
        {isUseSurfAbs && <Field label="Superphase Absorption" field="supAbs" step={0.0001} disabled={isRunning} />}
        {isUseSurfAbs && <Field label="Abs. Search %" field="absSearchPerc" step={1} tooltip="Search range for absorption during fitting." disabled={isRunning} />}
        <Field label="Crit. Edge Offset" field="critEdgeOffset" step={1} tooltip="Number of low-Q points to skip (removes critical edge region)." disabled={isRunning} />
        <Field label="High-Q Offset" field="highQOffset" step={1} tooltip="Number of high-Q points to skip." disabled={isRunning} />
        <Field label="Force Normalization" field="forcenorm" type="checkbox" tooltip="Force reflectivity curve to normalize to 1 at Q=0." disabled={isRunning} />
        {isNeutron && <Field label="% Error in Q" field="qSpread" step={1} tooltip="Instrument Q resolution: percent error for Gaussian smearing. Enter 5 for 5%." disabled={isRunning} />}
      </Section>
    </div>
  );
}
