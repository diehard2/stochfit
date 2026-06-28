# Source: orsopy Examples

**Repository:** https://github.com/reflectivity/orsopy
**Maintainer:** ORSO (Open Reflectometry Standards Organisation) community
**License:** MIT
**Downloaded from:** `https://raw.githubusercontent.com/reflectivity/orsopy/main/examples/`

## Samples

### prist5/
Neutron reflectometry data in the ORSO .ort format.
- Data: `prist5_10K_m_025.Rqz.ort` — ORSO text format with full YAML metadata header
  Columns: Qz R sigma_R dQz (Å⁻¹, dimensionless, dimensionless, Å⁻¹)
- Model: `sample_model_example_1.yml` — ORSO simple model language describing the layer
  stack with material SLDs, thicknesses, and roughnesses

The YAML header in the .ort file contains:
- Data source: instrument, sample, owner
- Reduction metadata: software, corrections applied
- Column definitions with units

ORSO format specification: https://www.reflectometry.org/advanced_and_expert_level/file_format
