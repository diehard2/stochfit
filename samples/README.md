# StochFit Sample Data

Test datasets for verifying StochFit's data import support. Each subfolder contains
data from a specific source with a `SOURCE.md` describing provenance and license.

## Folder Structure

```
samples/
├── refl1d/          Refl1D examples — NIST/NCNR, public domain
│   ├── ni-film/     Neutron TOF: Ni thin film on Si, 4 datasets + fit model
│   ├── four-column/ Standard Q R dR dQ text format example
│   ├── polymer/     Neutron: tethered dPS in two solvent contrasts
│   └── xray/        X-ray reflectivity example
├── refnx/           ANSTO Platypus beamline data — BSD-3
│   ├── PLP0000708/  Neutron: reduced data + fitted coefficients XML
│   ├── PLP0033831/  Neutron: reduced data (text)
│   └── orso/        ORSO .ort format example from refnx
├── orsopy/          ORSO community examples — MIT
│   └── prist5/      Neutron: ORSO .ort with YAML header + layer model YAML
├── schreiber-lab/   XRR HDF5 dataset with fits — CC-BY-4.0 (Zenodo)
└── genx/            GenX examples — GPL-3.0
    ├── xray-tutorial/ X-ray reflectivity tutorial data
    └── neutron-sio/   Neutron: SiO₂ on Si (two instruments)
```

## Formats Covered

| Format | Files |
|--------|-------|
| Plain text (Q R dR dQ) | refl1d/*, refnx/PLP*, genx/* |
| ORSO .ort text | refnx/orso/ORSO_data.ort, orsopy/prist5/*.ort |
| HDF5 (custom layout) | schreiber-lab/xrr_dataset.h5 |

See each `SOURCE.md` for download URLs, licenses, and fit parameter details.
