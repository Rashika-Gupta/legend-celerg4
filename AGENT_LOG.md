# Agent Log

Running log of proposed and applied changes made under the Celeritas
coding-guideline compliance pass (naming, Doxygen, assertions, design,
testing). One entry per proposed edit: timestamp, file path(s), summary,
reason/guideline addressed, and status (approved/rejected, with wording if
ambiguous).

Guideline reference: https://celeritas-project.github.io/celeritas/user/development/coding.html

---

## 2026-10-07 — Celeritas optical sensitive-detector diagnostic
**Files**: `DiagnosticOpticalSensitiveDetector.hh`,
`DiagnosticOpticalSensitiveDetector.cc`, `DetectorConstruction.cc`, and
`CMakeLists.txt`.
**Summary**: Added a temporary sensitive detector to count all optical-photon
step callbacks in the `liquid_argon_underground` source volume. Added
initialization and per-event status logging. Confirmed that the existing
Celeritas setup already enables sensitive-detector reconstruction for
zero-energy-deposition steps.
**Reason**: Distinguish a Celeritas optical-to-Geant4 callback limitation from
fiber-volume selection, boundary-status filtering, and ROOT-output failures.
The diagnostic detector deliberately applies no boundary filter and does not
write physics-analysis hits.
**Validation**: Read-back review only. No build or tests were run under the
operating constraints. The diagnostic should be removed after establishing
whether Celeritas invokes `ProcessHits` for optical steps.
**Status**: Approved ("okay implemenbt the change"); applied.

## 2026-10-07 — Outer TPB fiber-contact detector
**Files**: `FiberSensitiveDetector.hh`, `FiberSensitiveDetector.cc`,
`DetectorConstruction.hh`, `DetectorConstruction.cc`, `EventOutput.hh`,
`EventOutput.cc`, `CeleritasSetup.cc`, and `CMakeLists.txt`.
**Summary**: Added a dedicated detail sensitive detector for optical photons
entering the shared outer TPB fiber-coating logical volume, while preserving
the existing SiPM detector. Added a separate `fiber_hits` ntuple and
`num_fiber_hits` event-summary column, and routed Celeritas detector callbacks
associated with the fiber-coating logical volume away from SiPM output.
**Reason**: Diagnose photon transport through the 12,096 fiber-coating
placements without treating fiber contacts as SiPM detections or changing the
photon's transport state. The new class, functions, and output fields follow
Celeritas naming, documentation, assertion, and detail-namespace conventions.
**Validation**: Read-back review only. No build or tests were run under the
operating constraints. Geant4 records entry steps; Celeritas can record only
fiber interactions emitted by its optical detector callback, not arbitrary
boundary crossings.
**Status**: Approved ("go ahead do it"); applied.

## 2026-09-30 — OpticalHit.hh, OpticalHit.cc, AnalysisManagerHelper.cc
**Summary**: Renamed `OpticalHit`'s private members (`fpid`→`process_id_`,
`fsid`→`detector_id_`, `fname`→`detector_name_`, `fwave`→`wavelength_`,
`ft`→`time_`, `fpos`→`position_`, `fdir`→`direction_`,
`fpol`→`polarization_`) and public accessors (`GetPid()`→`process_id()
const`, `GetSid()`→`detector_id() const`, `GetDetName()`→`detector_name()
const`, `GetWave()`→`wavelength() const`, `GetTime()`→`time() const`,
`GetPos()`→`position() const`, `GetDir()`→`direction() const`,
`GetPol()`→`polarization() const`); renamed constructor/copy-ctor/
assignment parameters to match; added brief Doxygen comments on the
constructor and an accessor group; updated the only external call site
using the old getter names, `AnalysisManagerHelper.cc`'s
`SaveG4HitsToFile()` and `SaveG4SensitiveDetectorHitToFile()`. Fixed one
missed reference to the old `fdir` member in `OpticalHit::Print()` that
was caught during verification.
**Reason**: Celeritas coding guidelines section 1 (naming: `snake_case`
methods/locals, trailing `_` on private members) and section 2
(Doxygen on public members).
**Deviations (not applied, flagged to user)**: `Draw()`/`Print()` left
`CamelCase` (they mirror the `G4VHit` base-class convention; `Draw()`
overrides a Geant4 virtual). `operator new`/`operator delete` left as
fixed operator names. `process_id_`/`detector_id_` left as `int` rather
than `celeritas::OpaqueId<...>` — would ripple into
`AnalysisManagerHelper`'s `FillNtupleIColumn` (expects `int`) and the
`SteppingAction.cc`/`SensitiveDetector.cc` construction call sites;
scoped as a separate follow-up pending user decision.
**Status**: Approved ("yes please"); applied.

## 2026-10-02 — OpticalHit.hh reconciliation
**Summary**: Reapplied the previously recorded `OpticalHit` naming and
documentation update to the current flat-layout header: private members now
use descriptive `snake_case_` names, accessors use `snake_case` and are
`const`, constructor arguments describe their values, and the file/class/API
have Celeritas-style documentation.
**Reason**: The current header had drifted from the change recorded above;
this restores the Celeritas naming, documentation, and header conventions.
**Status**: Approved ("yes"); applied.

## 2026-10-05 — Geant4 action and detector-construction batch
**Files**: `RunAction.hh`, `RunAction.cc`, `DetectorConstruction.hh`,
`DetectorConstruction.cc`, `EventAction.hh`, `EventAction.cc`,
`SteppingAction.hh`, `SteppingAction.cc`, `TrackingAction.hh`, and
`TrackingAction.cc`.
**Summary**: Reconciled the ten files with Celeritas headers, documentation,
naming, include hygiene, private state, and `final` Geant4 overrides. Fixed
the `optical-distribution` run lifecycle mismatch, guarded the optional
optical collector, decomposed ntuple and detector setup into smaller helpers,
validated GDML/runtime state, included Celeritas event flushing in event
timing, preserved both Geant4 and Celeritas hits in hybrid events, cached the
Geant4 boundary process, checked the post-step volume, and simplified optical
trajectory handling.
**Reason**: Applies the recorded Celeritas style pass and corrects the runtime
defects identified during the requested review: mismatched begin/end
integration, possible null dereference, discarded mixed hit data, repeated
boundary-process lookup, and unchecked geometry data.
**Status**: Applied under the user's batch authorization ("Make changes to
these files at once and then in the end ask for approval"); pending final
approval.

## 2026-10-02 — OpticalHit.cc restoration
**Summary**: Restored the missing `OpticalHit` implementation with the
thread-local allocator definition, constructors, copy operations, property
comparison, visualization, and diagnostic output using the reconciled member
names.
**Reason**: `OpticalHit.hh` declared non-inline functions with no definitions
in the current tree, causing unresolved symbols and leaving the previously
recorded naming change incomplete.
**Status**: Approved ("yes, i approve this for all the .hh and .cc files");
applied.

## 2026-10-02 — OpticalHit.cc style reconciliation
**Summary**: Reconciled the externally replaced `OpticalHit` implementation
with Celeritas conventions while preserving its behavior: added direct
includes and Doxygen comments, removed a redundant allocator declaration,
used constructor initialization, and renamed the visualization-manager local.
**Reason**: The replacement implementation used the new member names but had
regressed from the documented Celeritas formatting, naming, and documentation
requirements.
**Status**: Approved ("yes'"); applied.

## 2026-10-05 — AnalysisManagerHelper.hh reconciliation
**Summary**: Replaced the malformed mixed include guard with a top-level
`#pragma once`, added direct standard-library and Geant4 dependencies,
grouped includes, used the local include form for `OpticalHit.hh`, and added
the standard file, hit-struct, and helper-class documentation.
**Reason**: Restores the header hygiene and documentation change recorded in
`CHANGES.md` and makes the header self-contained without changing its API or
behavior.
**Status**: Approved ("yes, please edit them. .hh and .cc"); applied.

## 2026-10-05 — AnalysisManagerHelper.cc debug ntuple correction
**Summary**: Changed `SaveParticleSteps()` to write to the debug-step ntuple
with ID 6 and replaced the generic local `id` with the descriptive constant
`ntuple_id`.
**Reason**: `RunAction.cc` creates `PhotonInfo` as ntuple 5 and the optional
debug-step table as ntuple 6; writing step columns to ntuple 5 used the wrong
schema and risked corrupt output.
**Status**: Approved ("yes"); applied.

## 2026-10-05 — Analysis output deduplication
**Files**: `AnalysisManagerHelper.hh`, `AnalysisManagerHelper.cc`, and
`RunAction.cc`.
**Summary**: Centralized all ROOT ntuple schema construction in
`AnalysisManagerHelper::create_ntuples`, replaced hard-coded output table and
column numbers with strongly typed identifiers, and made both Geant4 optical
hit paths use one shared row writer. Removed the duplicate schema-construction
implementation from `RunAction.cc`, which now makes one initialization call.
The existing table names, column names, column order, and output types remain
unchanged for downstream compatibility. Reviewed `OpticalHit.hh`,
`OpticalHit.cc`, `RunAction.hh`, and `MakeCelerOptions.hh`; no changes were
needed because they do not own duplicated output-schema or row-writing logic.
**Reason**: Establishes one source of truth for the output contract, prevents
schema/writer index drift, removes duplicated optical-hit serialization, and
keeps hit data and run-control classes focused on their own responsibilities.
**Status**: Approved ("yes please."); applied.

## 2026-10-05 — Unified event-level optical-hit output
**Files**: `AnalysisManagerHelper.hh`, `AnalysisManagerHelper.cc`,
`SensitiveDetector.cc`, and `EventAction.cc`.
**Summary**: Replaced the three public Geant4, Celeritas, and sensitive-
detector save paths with one `save_optical_hits()` event-level operation.
Sensitive-detector hits are now buffered instead of writing ROOT rows during
tracking. The unified operation takes the output lock once, writes all three
populated optical-hit buffers to their existing ntuples, and clears the
buffers afterward. The selected transport mode implicitly controls which
buffers are populated, so hybrid events retain hits from multiple sources.
**Reason**: Provides one optical I/O entry point, separates detector response
from persistency, avoids mode-string dispatch and repeated locking, and keeps
the existing output schema compatible with downstream analysis.
**Status**: Approved ("yes pelase. approved do it"); applied.

## 2026-10-05 — Remove provisional DUNE detector setup
**Files**: `DetectorConstruction.hh`, `DetectorConstruction.cc`, and
`ActionInitialization.cc`.
**Summary**: Removed the hard-coded DUNE cryostat border-surface path, all
manual optical skin/border surface attachment, auxiliary detector metadata
parsing, detector-channel mapping, and master/worker sensitive-detector
attachment. `DetectorConstruction::Construct()` now returns the world volume
already owned by the GDML parser without modifying it. Removed the obsolete
detector-ID transfer from action initialization and added its direct analysis
helper dependency.
**Reason**: The current LEGEND integration does not yet define application-
specific optical surfaces or sensitive detectors; retaining provisional DUNE
logic could mutate the imported geometry incorrectly and implied unsupported
detector behavior.
**Status**: Approved ("yes please do that"); applied.

## 2026-10-05 — Repository-wide LEGEND/Celeritas architecture migration
**Created**: `OpticalOffloadMode.hh`, `OpticalOffloadMode.cc`,
`CeleritasSetup.hh`, `CeleritasSetup.cc`, `EventOutput.hh`,
`EventOutput.cc`, and `input/legend-optical.mac`.
**Modified**: `ActionInitialization.hh`, `ActionInitialization.cc`,
`DetectorConstruction.hh`, `DetectorConstruction.cc`, `EventAction.hh`,
`EventAction.cc`, `PhysicsList.hh`, `PhysicsList.cc`,
`PrimaryGeneratorAction.hh`, `PrimaryGeneratorAction.cc`, `RunAction.hh`,
`RunAction.cc`, `TrackingAction.hh`, `TrackingAction.cc`, `legend_celer.cc`,
`CMakeLists.txt`, and `README.md`.
**Deleted**: `AnalysisManagerHelper.hh`, `AnalysisManagerHelper.cc`,
`MakeCelerOptions.hh`, `OpticalHit.hh`, `OpticalHit.cc`,
`SensitiveDetector.hh`, `SensitiveDetector.cc`, `SteppingAction.hh`,
`SteppingAction.cc`, and `input/g04-photon.mac`.
**Summary**: Replaced mode strings with `OpticalOffloadMode` and
`IntegrationType`; moved setup construction into the single-state
`CeleritasSetup` function-like class; replaced the mutable public analysis
singleton and legacy multi-schema output with thread-local `EventOutput` and
opaque Celeritas detector-hit data; removed unused Opticks output, Geant4
sensitive-detector/hit paths, and stale event counters; placed application
classes in namespace `legend`; replaced raw application ownership with RAII;
validated command-line mode and argument handling; moved the fixed source
point into the run macro; renamed the cache/environment option to
`LEGEND_CELER_OFFLOAD_MODE`; made the CMake source list explicit; removed the
legacy G04 target and macro; and disabled Geant4 SD invocation. Existing
third-party license text was retained where source files derived from Geant4.
Generated files under `build/` were intentionally not edited.
**Reason**: Applies Celeritas conventions for typed state, encapsulation,
function-like setup objects, assertions, opaque identifiers, descriptive
naming, explicit ownership, and deletion of obsolete code while removing
application behavior and terminology inherited from unrelated examples.
The ROOT schema intentionally changes to `optical_hits` and `event_summary`.
**Status**: Approved ("do it!"); applied without building or running tests.

## 2026-10-05 — LEGEND SiPM sensitive-detector output
**Created**: `detail/SipmSensitiveDetector.hh` and
`detail/SipmSensitiveDetector.cc`.
**Modified**: `DetectorConstruction.hh`, `DetectorConstruction.cc`,
`EventOutput.hh`, `EventOutput.cc`, `CeleritasSetup.cc`, `PhysicsList.cc`, and
`CMakeLists.txt`.
**Summary**: Added one internal optical sensitive detector for all LEGEND SiPM
placements, discovered channel IDs from the existing
`RMG_detector=optical` GDML metadata, and attached it to the shared silicon
logical volume. Enabled Geant4 optical-boundary sensitive-detector invocation
and routed detected photons through the existing event-level output writer.
Converted the buffered hit record into a backend-independent representation so
Geant4 and Celeritas detections use the same ROOT serialization path.
**Reason**: Geant4 optical-boundary detections previously had no sensitive
detector callback, leaving the optical-hit ntuple empty. The change restores a
LEGEND-specific detection path without creating new surfaces or duplicating
ROOT I/O.
**Status**: Approved ("go for it"); applied without building or running tests.

## 2026-10-06 — Remove tessellated solids from full LEGEND GDML
**Modified**: `l1000.gdml`.
**Summary**: Removed the tessellated PEN assembly plate, its 336 physical
placements, 672 directional LAr/PEN border surfaces, 10,974 facet-vertex
definitions, and now-unused PEN material and optical-property definitions.
Replaced the 444-facet copper hanger with the established 14-point
`genericPolycone` approximation. Replaced the non-axisymmetric 642-facet
copper tristar with the established `xtru` outline because a polycone cannot
represent its three arms. Removed 3,258 orphan copper facet vertices while
preserving both copper logical volumes, all 42 placements of each, and all 84
directional `surface_to_copper` boundaries for each.
**Reason**: Celeritas geometry import requires supported CSG primitives; the
conversion removes unsupported tessellated input without approximating the
tristar as an incorrect rotational solid. Removing PEN intentionally removes
its WLS, scintillation, and dielectric-boundary response from this geometry.
**Verification**: Parsed the resulting XML, confirmed zero active
`tessellated` elements, confirmed the hanger is a `genericPolycone` and the
tristar is an `xtru`, and confirmed copper material, placement, and boundary
surface counts are preserved. No build or transport test was run.
**Status**: Approved ("yes please do it"); applied.
