# legend-celerg4

Standalone Geant4–Celeritas optical-transport application for LEGEND.

Geant4 creates optical photons with a fixed-energy optical gun and Celeritas
transports the photon tracks. Set `CELER_DISABLE=1` to transport them in
Geant4 instead.

Choose the sensitive detector with `LEGEND_SENSDET`:

- `sipm` (default): SiPM channels from the GDML `RMG_detector` metadata
- `fiber`: volumes made of `tpb_on_fibers`

Only the selected detector is sensitive, and it absorbs the photons it detects.

Run in batch mode with a GDML geometry and Geant4 command macro:

```sh
mkdir build
cd build
cmake ..
LEGEND_SENSDET=fiber ./legend_celer ../input/[geometry.gdml] ../input/legend-optical.mac
```

Configure the gun in the macro:

```
/legend/gun/photons 1000
/legend/gun/energy 9.69 eV
/legend/gun/addPosition 30 0 -325 mm   # event i uses position i mod N
```

With no `addPosition`, the gun uses a default that hits the selected detector:

- `sipm`: `(52.75, 0, 349.12) mm`, LAr just below top SiPM S0102T
- `fiber`: `(30, 0, -325) mm`, inside the shroud at the fiber middle

Output goes to `<transport>-<sensdet>-<file>`, e.g. `celeritas-fiber-legend.root`
(set `<file>` with `/legend/output/file`). The `run_info` ntuple records the
`sensitive_detector` (`sipm`/`fiber`) and `transport` (`celeritas`/`geant4`).
