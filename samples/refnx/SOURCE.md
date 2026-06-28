# Source: refnx Test Data

**Repository:** https://github.com/refnx/refnx
**Maintainer:** ANSTO (Australian Nuclear Science and Technology Organisation)
**License:** BSD-3-Clause
**Downloaded from:** `https://raw.githubusercontent.com/refnx/refnx/main/refnx/dataset/tests/`

## Samples

### PLP0000708/
Reduced neutron reflectometry data from the Platypus time-of-flight reflectometer at ANSTO.
- Data: `c_PLP0000708.dat` — 3 columns: Q R dR (Å⁻¹, dimensionless, dimensionless)
- Fit: `coef_c_PLP0000708.xml` — XML file with fitted layer model coefficients
- Sample: Silicon-based thin film (exact composition from instrument record)

### PLP0033831/
A second Platypus dataset in text format.
- Data: `c_PLP0033831.txt` — columns: Q R dR [dQ]

### orso/
ORSO-format example file produced by refnx.
- Data: `ORSO_data.ort` — ORSO .ort text format with YAML header and Q R dR dQ columns
- Demonstrates the ORSO header structure with instrument and sample metadata
