# legend-celerg4

Standalone Geant4–Celeritas optical-transport application for LEGEND.

Select the transport mode at configuration time with
`LEGEND_CELER_OFFLOAD_MODE` or at runtime with the environment variable of the
same name. Supported values are `optical-gun`, `optical-track`,
`optical-distribution`, and `electron-photon`.

Run in batch mode with a GDML geometry and Geant4 command macro:

```sh
mkdir build
cd build
cmake ..
./legend_celer ../input/[geometry.gdml] input/legend-optical.mac
```
