# Source: GenX Examples

**Repository:** https://github.com/aglavic/genx
**Maintainer:** Artur Glavic (Paul Scherrer Institut)
**License:** GPL-3.0
**Downloaded from:** `https://raw.githubusercontent.com/aglavic/genx/master/genx/genx/examples/`

## Samples

### xray-tutorial/
X-ray reflectivity tutorial dataset used in the GenX documentation.
- Data: `xray-tutorial.dat` — columns: Q(Å⁻¹) R (or angle/intensity depending on measurement)
- Fit parameters documented in GenX tutorials:
  https://aglavic.github.io/genx/doc/tutorials/xrr_fitting.html
- Sample: Thin film system used as a beginner fitting example

### neutron-sio/
Neutron reflectometry data for a SiO₂ layer on silicon measured at two instruments.
- `Neutron_SiO_ref.dat` — General neutron reflectivity reference data for SiO₂/Si
- `D17_SiO.out` — Data from the ILL D17 reflectometer (monochromatic mode)
- Columns: Q R [dR [dQ]]
- Sample: Thermal SiO₂ on Si; fitted with a simple two-layer model (SiO₂ + Si substrate)
- Fit parameters: SiO₂ thickness (~15–100 Å), roughness, SLD values for n and x-ray
