import React from 'react';
import { Field, Section } from '../shared/Field';
import { useFitStore } from '../../stores/fit-store';

export function ParameterPanel() {
  const { status } = useFitStore();
  const isRunning = status === 'running';

  return (
    <div className="flex flex-col gap-5 p-4 overflow-y-auto">
      <h2 className="text-sm font-semibold text-primary uppercase tracking-wider">Model Parameters</h2>

      <Section title="Sample">
        <Field label="Substrate SLD (×10⁻⁶ Å⁻²)" field="subSLD" step={0.01} disabled={isRunning} />
        <Field label="Film SLD (×10⁻⁶ Å⁻²)" field="filmSLD" step={0.01} disabled={isRunning} />
        <Field label="Superphase SLD (×10⁻⁶ Å⁻²)" field="supSLD" step={0.01} disabled={isRunning} />
        <Field label="Wavelength (Å)" field="wavelength" step={0.001} disabled={isRunning} />
        <Field label="Film Length (Å)" field="filmLength" step={1} disabled={isRunning} />
      </Section>

      <Section title="Corrections">
        <Field label="Film Absorption" field="filmAbs" step={0.0001} disabled={isRunning} />
        <Field label="Substrate Absorption" field="subAbs" step={0.0001} disabled={isRunning} />
        <Field label="Superphase Absorption" field="supAbs" step={0.0001} disabled={isRunning} />
        <Field label="Q Spread (σ)" field="qErr" step={0.0001} tooltip="Instrument resolution: Gaussian smearing width in Q-space." disabled={isRunning} />
        <Field label="Crit. Edge Offset" field="critEdgeOffset" step={1} tooltip="Number of low-Q points to skip (removes critical edge region)." disabled={isRunning} />
        <Field label="High-Q Offset" field="highQOffset" step={1} tooltip="Number of high-Q points to skip." disabled={isRunning} />
        <Field label="Use Surf. Absorption" field="useSurfAbs" type="checkbox" tooltip="Include surface absorption in reflectivity calculation." disabled={isRunning} />
        <Field label="Force Normalization" field="forcenorm" type="checkbox" tooltip="Force reflectivity curve to normalize to 1 at Q=0." disabled={isRunning} />
        <Field label="Improved Normalization" field="impnorm" type="checkbox" tooltip="Search for optimal normalization factor during fitting." disabled={isRunning} />
        <Field label="XR Only" field="xrOnly" type="checkbox" tooltip="X-ray reflectivity mode (vs neutron)." disabled={isRunning} />
      </Section>
    </div>
  );
}
