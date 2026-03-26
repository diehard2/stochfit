# Source: Schreiber-Lab Reflectometry Dataset

**Repository:** https://github.com/schreiber-lab/reflectometry-dataset
**Zenodo record:** https://zenodo.org/records/6497438
**Maintainer:** Schreiber Lab, Helmholtz-Zentrum Berlin
**License:** CC-BY-4.0
**Downloaded from:** `https://zenodo.org/records/6497438/files/xrr_dataset.h5`

## Sample: xrr_dataset.h5

X-ray reflectometry (XRR) dataset of various thin film materials deposited on SiOx substrates,
measured in situ as a function of deposited layer thickness.

**Format:** HDF5 (~254 KB)

**HDF5 structure:**
- `experimental/` group — measured XRR curves (Q vs R arrays)
- `fit/` group — corresponding fitted parameters (layer thicknesses, roughnesses, SLDs)

**Note:** This file does not follow the NXcanSAS application definition. The StochFit
NXcanSAS parser will not load it directly. It is included as a reference for the
HDF5-with-fits data structure and for future parser development.
To load it, you would need to navigate to the appropriate subgroup for a specific sample.

**Citation:**
Greco, A., Starostin, V., Munteanu, V., Knoefel, N., Davydok, A., Bertram, F., ...
& Schreiber, F. (2022). *Fast autonomous megavolt electron diffraction and X-ray
reflectometry*. Zenodo. https://doi.org/10.5281/zenodo.6497438
