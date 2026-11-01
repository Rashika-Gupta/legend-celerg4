# legend-celerg4

Standalone Geant4–Celeritas optical-transport application for LEGEND.

Geant4 creates optical photons with a fixed-energy optical gun and Celeritas
transports the photon tracks. Set `CELER_DISABLE=1` to transport them in
Geant4 instead.

Choose the sensitive detector with `LEGEND_SENSDET`:

- `sipm` (default): SiPM channels from the GDML `RMG_detector` metadata
- `fiber`: volumes made of `tpb_on_fibers`

Only the selected detector is sensitive, and it absorbs the photons it detects.

Choose how optical photons reach Celeritas with `LEGEND_CELER_OFFLOAD_MODE`:

- `optical-track` (default): Geant4 creates each optical photon with the gun,
  and the track is handed off to Celeritas for transport.
- `optical-gun`: Celeritas generates and transports the photons itself,
  using the same photon count, energy, and position as the configured gun.
  This skips the per-track Geant4↔Celeritas hand-off, which is more
  efficient on GPU for large photon counts.

Tune for GPU throughput with:

- `LEGEND_NUM_THREADS`: Geant4 worker threads (default: hardware concurrency).
  Each thread drives its own Celeritas stream, so use several threads to keep
  the GPU fed instead of the default single-threaded run.
- `LEGEND_NUM_TRACK_SLOTS`: Celeritas optical track-slot capacity (default
  65536). Raise this (e.g. into the hundreds of thousands or millions,
  depending on GPU memory) so enough tracks are resident to saturate the GPU.


Run in batch mode with a GDML geometry and Geant4 command macro:

```sh
mkdir build
cd build
cmake ..
LEGEND_CELER_OFFLOAD_MODE=optical-gun
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
