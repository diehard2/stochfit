# Source: refl1d Examples

**Repository:** https://github.com/reflectometry/refl1d
**Maintainer:** NIST Center for Neutron Research (NCNR)
**License:** Public domain / BSD
**Downloaded from:** `https://raw.githubusercontent.com/reflectometry/refl1d/master/doc/examples/`

## Samples

### ni-film/
Neutron reflectometry of a nickel thin film on silicon, measured in time-of-flight (TOF) mode.
- Data: 4 TOF datasets (`nifilm-tof-1.dat` to `nifilm-tof-4.dat`), columns: Q R dR dQ
- Model: `nifilm-fit.py` — layer stack Si / Ni (125 Å) / air; fits thickness and roughness
- Fit parameters defined in model.py: Ni thickness ~125 Å, interface roughnesses

### four-column/
Generic example of a four-column reflectivity file in the standard Q R dR dQ format.
- Data: `refl.txt`
- Model: `model.py`

### polymer/
Neutron reflectometry of tethered deuterated polystyrene (dPS) in two solvent contrasts.
- Data: `10ndt001.refl` (in deuterated toluene), `10nht001.refl` (in hydrogenated toluene)
- Model: `tethered.py` and `freeform.py` — ~10 nm chains swelling to 14–18 nm in toluene
- Fit parameters: polymer layer thickness, SLD profile, roughness

### xray/
X-ray reflectometry example data.
- Data: `e1085009.log` — intensity vs angle/Q
- Model: `model.py`
