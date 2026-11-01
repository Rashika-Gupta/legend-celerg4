# Change log: DUNE/Opticks removal, LEGEND port

Each entry records one approved edit made while stripping DUNE-specific
assumptions and Opticks dependencies from this app, while preserving the
reusable Geant4-Celeritas integration and geometry-import code.

## CMakeLists.txt
- Renamed project from `GDMLOpticks` to `legend-celerg4`.
- Removed `With_Opticks` option and the entire Opticks build block
  (`OPTICKS_HOME`/`OPTICKS_PREFIX`, `G4CX`/`U4` `find_package`, Opticks
  source globbing and linking).
- Replaced hardcoded personal `Celeritas_DIR` path with an
  `ENV{Celeritas_DIR}`-driven fallback plus a TODO for LEGEND builders.
- Fixed the stale run-script copy list (`G04_SCRIPTS`), which referenced
  deleted DUNE macros/GDML files, to reference the files that actually
  exist (`macros/g04.mac`, `macros/g04-photon.mac`, `vis.mac`,
  `celeritas-legendl1000-no-tesselated.gdml`).

## config.h.in
- Removed `#cmakedefine With_Opticks` (Opticks support removed).

## gdml_det.cc
- Removed Opticks includes (`QString`, `QtGlobal`, `SEventConfig.hh`,
  `OPTICKS_LOG.hh`, `cuda_runtime.h`, `G4OpticalPhysicsOpticks.hh`) and the
  `#ifdef With_Opticks` GPU-device-init block and the
  `G4OpticalPhysicsOpticks` physics-list registration.
- Replaced the hardcoded DUNE GDML path in the usage/help text with a
  generic `<geometry.gdml>` placeholder.

## src/DetectorConstruction.cc
- Removed the `#ifdef With_Opticks` block (`Opticks/MySensorIdentifier.hh`,
  `G4CXOpticks`, `U4SensorIdentifier.h`) that set the Opticks sensor
  identifier/geometry after `Construct()`.
- Removed dead commented-out code referencing DUNE's `ArapucaSurface`/
  `ArapucaWindowProperties` material.a
- Marked the hardcoded `"volCryostat_PV"` border-surface source volume
  (DUNE's cryostat) with a `TODO(LEGEND)` to supply the correct LEGEND
  outer volume name.
- Reworded the "Keep supporting the DUNE convention" comment to be
  detector-neutral (functionality unchanged: `SensDet="PhotonDetector"`
  aux-tag support).

## include/PrimaryGeneratorAction.hh, src/PrimaryGeneratorAction.cc
- Discovered that everything after the offload-mode `if`/`else if`/`else`
  chain in `GeneratePrimaries` was unreachable dead code (every valid mode
  hits an early `return;`, and the invalid-mode branch aborts via
  `G4Exception(FatalException)`). That dead code was also where all the
  Opticks/LArSoft/DUNE-benchmark logic lived, so it was deleted outright:
  `GenStorchPrimaries`, `GenSphotonsPrimary`, `SinglePhotonGenerator`,
  `GeneratePrimaryLinearly`, `linspace`, the `"larsoft"`/`"ROOT"`
  TTreeReader ingestion branch, the `"Performance"` benchmarking branch,
  the dead GPS/`"opticalphoton"` tail, the `PrimaryPhoton` struct, and the
  `EnergyToWavelength`/`EnergySigmaToWavelengthSigma` helpers (only used
  by the removed code).
- Removed now-unused members: `fmsg`, `fFileName`, `finitParticleType`,
  `fAmount`, `fPosition`, `fMom`, `fSigmaMom`, `fPhotonAmount`, `fVerbose`,
  `simPhotonCPU`, `fGPUPhotonType`, `sphotons`.
- Removed the `#ifdef With_Opticks` includes (`sphoton.h`,
  `OpticksPhoton.h`, `SEvt.hh`, `Opticks/OpticksHitHandler.hh`, `srng.h`,
  `storch.h`) and the Opticks GPS/GPU-photon-count bookkeeping.
- Replaced hardcoded DUNE cryostat coordinates (optical-gun position,
  electron-photon generation position) with `TODO(LEGEND)`-flagged origin
  placeholders; flagged the 50 MeV electron energy similarly.
- Simplified the constructor to just build the `G4GeneralParticleSource`
  and reset the analysis helper (the removed messenger commands are no
  longer used by any surviving macro).

## src/EventAction.cc
- Removed the `#ifdef With_Opticks` includes (`SEvt.hh`, `G4CXOpticks.hh`,
  `Opticks/OpticksHitHandler.hh`) and the associated Opticks mutex,
  photon-genstep batching, and hit-saving calls in `EndOfEventAction`.
- Removed the now-unused `G4AutoLock.hh` include.

## src/RunAction.cc
- Removed the dead commented-out block that dumped a `GROUPVEL`/`RINDEX`
  table for a hardcoded `"lAr"` material to `gdml-group-vel-g4.jsonl`
  (DUNE liquid-argon benchmark artifact, already inert).

## src/SteppingAction.cc
- Removed the group-velocity debug dump ("optical photon's first step in
  LAr") that wrote a JSON record per photon to
  `group-vel-g4-steppingaction.jsonl`, and its `nlohmann/json.hpp`
  include. This block was gated by `if (GetCurrentStepNumber() != 1)
  return;`, which as a side effect skipped boundary-hit detection for
  every step past the first; removing it restores normal per-step hit
  detection. `ArapucaHit` naming left as-is for the pending
  hit-class rename.

## src/PhysicsList.cc
- Removed the unused `#include "G4ScintillationOpticks.hh"` (an Opticks
  header that was never referenced in the file; scintillation/Cherenkov
  offload is already handled generically via `accel/gen/CherenkovOffload.hh`
  and `accel/gen/ScintillationOffload.hh`). `include/PhysicsList.hh` had
  no Opticks/DUNE references and needed no changes.

## src/SensitiveDetector.cc (bugfix, unrelated to DUNE/Opticks stripping)
- `ProcessHits` referenced an undeclared `it` iterator (compile error).
  Added the missing `auto it = fDetectIds->find(detectName);` lookup and
  removed a duplicate `if (it == fDetectIds->end()) return false;` check
  that followed it.

## CMakeLists.txt
- Renamed the `gdml_det` executable target to `legend_celer` (in
  `add_executable`, `target_compile_definitions`,
  `target_link_libraries`, `add_custom_target(G04 DEPENDS ...)`, and
  `install(TARGETS ...)`). The main source file remains
  `gdml_det.cc` (rename not requested).

## include/ArapucaHit.hh -> include/OpticalHit.hh, src/ArapucaHit.cc -> src/OpticalHit.cc
- Renamed the `ArapucaHit` class (named after DUNE's ARAPUCA photon
  detector) to the detector-neutral `OpticalHit`, along with
  `ArapucaHitsCollection` -> `OpticalHitsCollection` and the
  thread-local allocator `ArapucaHitA` -> `OpticalHitAllocator`.
- Added a proper `#ifndef`/`#define`/`#endif` include guard
  (`LEGEND_CELERG4_OPTICALHIT_HH`); the original header only had a
  dangling `#endif //GDMLOPTICKS_ARAPUCAHIT_HH` with no matching
  opening guard.
- Propagated the rename through `include/SensitiveDetector.hh`
  (`fArapucaHitsCollection` -> `fOpticalHitsCollection`),
  `src/SensitiveDetector.cc`, `include/AnalysisManagerHelper.hh` and
  `src/AnalysisManagerHelper.cc` (`ArapucaHits` member ->
  `OpticalHits`), `src/EventAction.cc`, and `src/SteppingAction.cc`.
  No behavioral changes besides the naming.

## Flat-layout restructuring (external change; build fixes applied)
- User flattened `include/`/`src/` into the `legend-celerg4/` root,
  moved macros/GDML into `input/`, renamed `gdml_det.cc` ->
  `legend_celer.cc`, and deleted `ColorReader` (unneeded). Fixed the
  resulting build breaks:
  - `CMakeLists.txt`: `include_directories`/`file(GLOB ...)` updated to
    scan `${PROJECT_SOURCE_DIR}` directly instead of `include/`/`src/`;
    added `list(REMOVE_ITEM sources .../legend_celer.cc)` so the glob
    doesn't pull in `main()` a second time alongside the explicit
    `add_executable` source.
  - `legend_celer.cc`: removed the `new ColorReader` custom GDML reader
    (class deleted); `G4GDMLParser` now uses its default reader.
  - `OpticalHit.cc`: `#include "../include/OpticalHit.hh"` ->
    `#include "OpticalHit.hh"` (same directory now).

## Celeritas style pass (pure restyle, no behavior changes)

## legend_celer.cc
- Replaced the Geant4 legal-disclaimer banner with the Celeritas
  modeline/license/`\file`/`\brief` banner (matching
  `example/geant4/simple-offload.cc`).
- Reorganized includes into `// Geant4`, `// Celeritas`, `// Local`
  groups, alphabetized within each group. No header added, removed, or
  changed from quotes to angle brackets (or vice versa); `main()` body
  untouched.

## ActionInitialization.hh, ActionInitialization.cc
- Replaced the Geant4 legal-disclaimer banner with the Celeritas
  modeline/license/`\file`/`\brief` banner.
- `.hh`: switched the `#ifndef`/`#define`/`#endif` include guard to
  `#pragma once`; grouped includes (`// Geant4`); added a one-line
  Doxygen `\brief` on the class definition.
- `.cc`: grouped includes (`// Geant4`, `// Local`); replaced the
  `//....oooOO0OOooo...` decorative separators with the 77-dash
  `//---...---//` rule. No logic changes.

**Flag and suggestion**: `ActionInitialization::Build()` uses the
global `anaHelper` (assigning/dereferencing it) without including
`AnalysisManagerHelper.hh` directly in this file — it must be coming
in transitively through another included header. Not fixed as part of
this style pass (would require adding a direct include, a behavioral
correctness/build-hygiene change); worth adding an explicit include of
`AnalysisManagerHelper.hh` in a follow-up.

## DetectorConstruction.hh, DetectorConstruction.cc
- Replaced the Geant4 legal-disclaimer banner with the Celeritas
  modeline/license/`\file`/`\brief` banner.
- `.hh`: switched the `#ifndef _DETECTORCONSTRUCTION_H_`/`#endif`
  include guard to `#pragma once`; grouped includes (`// Geant4`,
  `// Local`); added a one-line Doxygen `\brief` on the class
  definition.
- `.cc`: grouped includes (`// Geant4`, `// Local`), alphabetized
  within each group, and removed a duplicate `#include
  <G4GDMLParser.hh>` and duplicate `"SensitiveDetector.hh"` /
  `"G4PhysicalVolumeStore.hh"` / `"G4OpticalSurface.hh"` includes (each
  had appeared twice); replaced the `//....oooOO0OOooo...` decorative
  separators with the 77-dash `//---...---//` rule. No logic changes.

## PrimaryGeneratorAction.hh, PrimaryGeneratorAction.cc
- Replaced the Geant4 legal-disclaimer banner with the Celeritas
  modeline/license/`\file`/`\brief` banner.
- `.hh`: switched the `#ifndef _G04PRIMARYGENERATORACTION_H_`/`#endif`
  include guard to `#pragma once`; grouped includes (`// Geant4`,
  `// Local`); added a one-line Doxygen `\brief` on the class
  definition.
- `.cc`: grouped/alphabetized includes (`// Geant4`, plus a top block
  for `<iostream>`/`<string>`); replaced the `//....oooOO0OOooo...`
  decorative separators with the 77-dash `//---...---//` rule. No
  logic changes; all existing `TODO(LEGEND)` comments preserved
  verbatim.

## EventAction.hh, EventAction.cc
- Replaced the personal "Created by ilker on 10/22/25." comment with
  the Celeritas modeline/license/`\file`/`\brief` banner.
- `.hh`: switched the `#ifndef GDMLOPTICKS_EVENTACTION_HH`/`#endif`
  include guard to `#pragma once`; grouped includes (`// Geant4`);
  added a one-line Doxygen `\brief` on the class definition.
- `.cc`: grouped/alphabetized includes (`// Geant4`, `// Celeritas`,
  `// Local`); removed a duplicate `#include "include/config.h"`
  (appeared twice); reformatted the constructor onto multiple lines
  (member-init list one per line, matching the other restyled files);
  added a `//---------------------------------------------------------------------------//`
  separator between `BeginOfEventAction` and `EndOfEventAction` for
  consistency with the rest of the codebase. No logic changes.

**Flag and suggestion**: `EventAction.hh` has `using namespace std;`
at header/global scope, and `EventAction.cc` relies on it for
unqualified `cout`/`endl`/`chrono::` usage. This violates the
AGENTS.md rule against namespace-scope `using namespace` in headers.
Not fixed here — properly qualifying every `chrono::`/`cout`/`endl`
usage across both files is a broader change than pure cosmetic
restyling and risks introducing typos without a build to verify;
worth doing as a dedicated follow-up.

## RunAction.hh, RunAction.cc
- Replaced the personal "Created by ilker on 10/22/25." comment with
  the Celeritas modeline/license/`\file`/`\brief` banner.
- `.hh`: switched the `#ifndef GDMLOPTICKS_RUNACTION_HH`/`#endif`
  include guard to `#pragma once`; grouped includes (`// Geant4`,
  `// Celeritas`, plus a top block for `<chrono>`/`<string>`/
  `<vector>`); changed `#include "chrono"`/`#include "vector"` (quoted
  standard-library headers) to `<chrono>`/`<vector>`; removed the
  unused `#include <nlohmann/json.hpp>` (dead leftover from the
  earlier DUNE/Opticks-stripping pass that removed the JSON-dumping
  code using it); added a one-line Doxygen `\brief` on the class
  definition.
- `.cc`: deduplicated `#include "G4Run.hh"`/`#include <G4Run.hh>`
  (appeared twice) and grouped includes (`// Geant4`); added
  `//---------------------------------------------------------------------------//`
  separators between constructor/destructor/`BeginOfRunAction`/
  `EndOfRunAction`; removed the decorative `// ───...───` Unicode
  divider and the stale duplicate commented-out `// std::string
  transport = ...` line inside `BeginOfRunAction` (both previously
  flagged as leftover cruft from the earlier DUNE/Opticks-stripping
  edit); removed a non-ASCII emoji (🔴) from a comment per the
  AGENTS.md ASCII-only rule, rewording to "required for MT merging".
  No logic changes.

**Flag and suggestion**: `EndOfRunAction`/`BeginOfRunAction` still
create ntuples named `"OpticksHits"` with a `// Opticks Hits` comment
— a DUNE/Opticks-era table name. Not renamed here since it changes the
on-disk ROOT schema (a functional/output-format change, not styling);
flagging for a decision on whether downstream consumers expect this
name before renaming.

## SteppingAction.hh, SteppingAction.cc
- Replaced the personal "Created by ilker on 10/22/25." comment with
  the Celeritas modeline/license/`\file`/`\brief` banner.
- `.hh`: switched the `#ifndef GDMLOPTICKS_STEPPINGACTION_HH`/`#endif`
  include guard to `#pragma once`; grouped includes (`// Geant4`,
  `// Local`); added one-line Doxygen `\brief` comments on the class
  and on the free `EtoWavelength` helper; trimmed a redundant
  "input photon energy in eV / return wavelength in nm" comment down to
  the single `// Wavelength = h*c/e` line already present below it.
- `.cc`: grouped includes (`// Geant4`, `// Local`); added
  `//---------------------------------------------------------------------------//`
  separators between the constructor/destructor/`UserSteppingAction`/
  `GetOpticalBoundaryProcess` definitions; removed a large block of
  dead commented-out code ("Optical-gun comparison: score primary
  photons only") that has been inert since it was commented out. No
  logic changes.

## TrackingAction.hh, TrackingAction.cc
- Replaced the personal "Created by ilker on 10/22/25." comment with
  the Celeritas modeline/license/`\file`/`\brief` banner.
- `.hh`: switched the `#ifndef GDMLOPTICKS_TRACKINGACTION_HH`/`#endif`
  include guard to `#pragma once`; grouped the include (`// Geant4`);
  added a one-line Doxygen `\brief` on the class definition.
- `.cc`: grouped/alphabetized includes (`// Geant4`); added
  `//---------------------------------------------------------------------------//`
  separators between the constructor/destructor/
  `PreUserTrackingAction`/`PostUserTrackingAction` definitions. No
  logic changes; `PostUserTrackingAction`'s empty body left as-is
  (functional question, not style).

## SensitiveDetector.hh, SensitiveDetector.cc
- Replaced the Geant4 legal-disclaimer banner with the Celeritas
  modeline/license/`\file`/`\brief` banner.
- `.hh`: switched the `#ifndef SensitiveDetector_h`/`#endif` include
  guard to `#pragma once`; grouped includes (`// Geant4`, `// Local`);
  added one-line Doxygen `\brief` comments on the class and on the
  free `EtoWavelengthNM` helper; trimmed a redundant "input photon
  energy in eV / return wavelength in nm" comment down to the single
  `// Wavelength = h*c/e` line.
- `.cc`: grouped includes (`// Geant4`, `// Local`); replaced the
  `//....oooOO0OOooo...` decorative separators with the 77-dash
  `//---...---//` rule; removed three lines of dead commented-out code
  in `ProcessHits` (`// auto analysisManager = ...`, a stale
  `// G4String detectName = ...` comment predating the current
  `detectName` computation, and a stale `// G4String detectName =
  SensitiveDetectorName; ... // G4int Sid = 0;` pair). No logic
  changes (the earlier `it`-undeclared bugfix is untouched).

**Flag and suggestion**: `SensitiveDetector.hh` has `using namespace
std;` at header/global scope (same AGENTS.md violation flagged for
`EventAction.hh`). Not fixed here for the same reason — the fix scope
extends beyond this file/pure restyling.

## OpticalHit.hh, OpticalHit.cc
- Replaced the personal "Created by ilker on 10/27/25." comment with
  the Celeritas modeline/license/`\file`/`\brief` banner.
- `.hh`: switched the `#ifndef LEGEND_CELERG4_OPTICALHIT_HH`/`#endif`
  include guard (added earlier in the DUNE/Opticks-stripping pass) to
  `#pragma once`; grouped the includes (`// Geant4`); added a one-line
  Doxygen `\brief` on the class definition.
- `.cc`: grouped the includes (`// Geant4`); added
  `//---------------------------------------------------------------------------//`
  separators between the default constructor/destructor, the
  parameterized/copy constructors, `operator=`, `operator==`, `Draw`,
  and `Print` definitions. No logic changes; private `f`-prefixed
  member naming left as-is per the earlier scope decision (naming
  convention changes excluded from this pass).
## AnalysisManagerHelper.hh, AnalysisManagerHelper.cc
- Replaced the personal "Created by ilker on 11/5/25." comment with
  the Celeritas modeline/license/`\file`/`\brief` banner.
- `.hh`: grouped the includes (`// Geant4`, `// Celeritas`, `// Local`);
  added a one-line Doxygen `\brief` on the class definition.
- `.cc`: grouped the includes (`// Geant4`); added
  `//---------------------------------------------------------------------------//`
  separators at logical boundaries (constructor/destructor block,
  before `Reset()`, and before each `Save*ToFile`/`SaveParticleSteps`
  function). Given the large number of trivial one-line
  getters/adders in this file, separators were not inserted between
  every individual accessor, unlike the smaller files restyled
  earlier — only at the higher-level logical boundaries. No logic
  changes.

**Flag and suggestion**: `AnalysisManagerHelper.hh`'s include guard was
malformed before this pass — the `#ifndef GDMLOPTICKS_ANALYSISMANAGERHELPER_HH`
/`#define` began *after* all the `#include` lines, so those includes
were never actually covered by the guard, and a redundant `#pragma once`
sat between the `#define` and the first declaration. This restyle
consolidates the file to a single `#pragma once` at the top (the
originally-intended protection), removing the ineffective/redundant
guard. This is a correctness fix bundled with the restyle because it
was directly in the banner/guard region being touched; flagging here
in case a wider audit of `#pragma once` usage elsewhere is desired.

## PhysicsList.hh, PhysicsList.cc
- Replaced the personal "Created by ilker on 6/14/25." comment with
  the Celeritas modeline/license/`\file`/`\brief` banner.
- `.hh`: switched `#ifndef G4_PHYSICSLIST_HH`/`#endif` to
  `#pragma once`; grouped the includes (`// Geant4`, `// Celeritas`);
  added a one-line Doxygen `\brief` on the class definition.
- `.cc`: grouped the includes (`// Geant4`, `// Celeritas`); added
  `//---------------------------------------------------------------------------//`
  separators between the constructor, destructor, `ConstructProcess`,
  `optical_options`, and `physics_options` definitions. No logic
  changes.

## MakeCelerOptions.hh
- Fixed the stale Doxygen path in `\file offload-template/src/MakeCelerOptions.hh`
  to `\file MakeCelerOptions.hh` (this file lives directly in
  `legend-celerg4/`, not under an `offload-template/src/` tree — a
  leftover from whatever template this file was originally copied
  from).
- Grouped/alphabetized the includes into `// Geant4 includes`,
  `// Celeritas includes`, and a new `// Local` group for
  `AnalysisManagerHelper.hh` (previously mixed into the Geant4 group).
  The file's pre-existing Doxygen briefs and
  `//---------------------------------------------------------------------------//`
  separators (already Celeritas-style) were left as-is.

**Flag and suggestion**: this header previously had **no** include
guard or `#pragma once` at all, despite defining several `inline`
functions and the `OffloadConfiguration`/`CelerOpticalHit` types at
namespace scope. Since it's `#include`d only from `legend_celer.cc`
today, this hasn't caused an observed multiple-inclusion problem, but
it is a latent risk if the header is ever included from more than one
translation unit. Added `#pragma once` as part of this pass since it
is a zero-risk, purely additive fix consistent with every other header
touched in this restyle.

## config.h.in
- Added the Celeritas modeline/license banner (no `\file` tag, since
  this is a CMake-configured generated header, not a source file with
  a stable path) and a `#pragma once` guard; the file previously had
  no banner and no include guard at all. `#cmakedefine With_DEBUG`
  content is unchanged.

## Build-error fix: SteppingAction.hh
- Restoring the build (reported by the user via `ninja`/`make` output)
  revealed that the `// Geant4` include-grouping edit to
  `SteppingAction.hh` had dropped the `#include "G4PhysicalConstants.hh"`
  that the original file relied on for `CLHEP::h_Planck`, `c_light`,
  `eV`, and `nm` in the free `EtoWavelength()` helper (compile errors:
  "no member named 'h_Planck' in namespace 'CLHEP'", etc.). Restored
  the include in the `// Geant4` group, matching the identical pattern
  already present in `SensitiveDetector.hh` (same `CLHEP::h_Planck`
  usage, same include). Checked all other restyled files for the same
  `CLHEP::`-without-`G4PhysicalConstants.hh`/`G4SystemOfUnits.hh`
  gap; none found.