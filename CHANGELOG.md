# Changelog

## Unreleased

## v2.3.2 - 2026-05-30

### Added

- Added context query APIs for region count and Level 3 configuration state in
  C and Fortran.
- Added a counted C region-weight setter for size-safe weight updates.
- Added explicit Level 3 neighbor-position and subdomain-position adapter
  helpers so applications can avoid ambiguous position-field side effects.
- Documented the supported scope of `oh_context_configure_level3_legacy()` and
  the recommended `oh3_init_raw()` migration path for historical Fortran field
  descriptors.
- Clarified that Level 4p/4s remain default-context APIs until Level 4 storage
  becomes heap-context owned.
- Added a GitHub Actions CI gate and made the standard local gate include fpm
  package-build coverage.
- Added `MPIRUN_FLAGS` support for constrained CI MPI launchers.
- Updated GitHub Actions CI to avoid Node.js 20 actions by using a Node 24
  checkout action and installing fpm from its release binary.

### Fixed

- Removed duplicate Level 4 test definitions of `gridMask`/`logGrid` so GCC
  `-fno-common` CI builds link cleanly.
- Initialized Level 4 position-aware field exchange tables on the context state
  so Fortran Level 4 initialization does not dereference NULL state pointers.
- Changed legacy Level 4 Fortran interfaces to pass raw contiguous arrays to
  the C implementation on gfortran instead of assumed-shape descriptors.
- Changed legacy Level 1/2/3 Fortran init interfaces to pass raw contiguous
  arrays to the C implementation on gfortran.
- Preserved Level 4p send counts separately from send-buffer cursors before
  particle exchange.
- Improved injection overflow diagnostics with rank, particle counts,
  injection index, capacity, species count, adapter mode, and API path.
- Reported a clear region-weight precondition error when Fortran callers set
  weights before context/region configuration.
- Kept weighted rebalance schedule load accounting balanced after assigning a
  weighted transfer.
- Reused stable weighted secondary schedules when particles remain inside the
  existing primary/secondary family, avoiding repeated weighted rebalance on
  unchanged ownership.

### Verification

- `git diff --check`
- `bash tests/test_particle_contract_audit.sh`
- `bash scripts/fpm-build-test.sh`
- `bash scripts/test.sh`

## v2.3.1 - 2026-05-30

### Added

- Added Level 4p/4s Fortran runtime transbound coverage for default and
  weighted secondary paths.

### Fixed

- Made Level 4p/4s Fortran per-grid histogram wrappers allocate their internal
  grids safely instead of treating a Fortran scalar argument as an offset
  C-side grid buffer.
- Converted Level 4p/4s Fortran boundary condition ids before storing the
  Level 4 mapping boundary table.

### Verification

- `git diff --check`
- `bash tests/test_particle_contract_audit.sh`
- `bash -n scripts/docker-build-test.sh`
- KUDPC `tssrun` two-rank smoke tests for Level 4p/4s Fortran default and
  weighted-secondary paths.

## v2.3.0 - 2026-05-30

This minor release promotes Level 4p/4s into the supported v2 API surface and
adds runtime coverage for weighted secondary transbound.

### Changed

- Promoted Level 4p/4s from migration-smoke coverage to supported v2 API
  coverage through the Level 4 C/Fortran entry points.
- Added successful Level 4 weighted secondary transbound coverage for both
  4p and 4s.

### Fixed

- Preserved Level 4s send counts separately from send-buffer cursors so
  weighted secondary transbound posts matching particle transfers.

### Verification

- `git diff --check`
- `bash tests/test_particle_contract_audit.sh`
- KUDPC `tssrun` two-rank smoke tests for Level 4p/4s default,
  custom-adapter, weighted-load, and weighted-secondary paths.

## v2.2.0 - 2026-05-30

This minor release advances the v2 context and particle-adapter migration while
keeping the supported public v2 scope at Level 1-3. Level 4 sources now live
only in the normal `src/` and `include/` tree and remain covered as migration
and compile-smoke paths.

### Added

- Added broader C and Fortran runtime coverage for context lifecycle, raw
  Fortran initialization, legacy Fortran compatibility entry points, public
  header C++ compatibility, and Level 4 capacity/runtime smoke checks.
- Added v2 Level 2 custom-particle samples for C and Fortran.
- Added repository-local `scripts/test.sh` as the standard release gate.
- Added v1 Markdown reproducibility checking through
  `scripts/check-v1-markdown.sh`.

### Changed

- Strengthened context ownership, particle-buffer, accounting, and default
  particle-type guards around the v2 adapter contract.
- Expanded weighted-load and particle-adapter tests so layout, callback, and
  accounting assumptions are checked directly.
- Updated C and Fortran usage documentation for context lifecycle, particle
  adapter ownership, injected-particle accounting, and API scope by level.
- Updated sample makefiles and Docker build checks to cover the current sample
  set and migration smoke paths.

### Removed

- Removed stale duplicate Level 4 source copies from `bag_src/`; canonical
  Level 4 sources are under `src/c`, `src/fortran`, and `include`.

### Verification

- `bash scripts/test.sh`
- `git diff --check`
- `PYTHON=/tmp/ohhelp2-doc-check-venv/bin/python bash scripts/check-v1-markdown.sh`

## v2.1.3 - 2026-05-24

This patch release publishes the repository with clarified licensing and keeps
the supported v2 scope at Level 1-3.

### Fixed

- Preserved integer-region `map_to_subdomain` routing when Level 3
  position-field helpers are attached to a particle adapter.
- Finalized pending injected particles before `oh_context_set_total_particles()`
  / `oh2_set_total_particles()` recomputes particle totals and buffer offsets.
- Documented Level 1-3 `transbound` return modes separately from historical
  compatibility mode constants.

### License

- Added repository-level license summary in `LICENSE.md`.
- Renamed `LISENCES/` to `LICENSES/`.
- Reproduced the original OhHelp non-commercial notice and the MIT text for
  contributions by Jin Nakazono under `LICENSES/`.
- Clarified that the repository as a whole should be treated as
  `LicenseRef-OhHelp-NonCommercial AND MIT`.

### Verification

- `git diff --check`
- `bash tests/test_particle_contract_audit.sh`
- Docker `mpicc` / `mpifort` full build and runtime smoke:
  `scripts/docker-build-test.sh`
- Docker Intel MPI C lifecycle and POS_AWARE lifecycle smoke tests.

## v2.1.2 - 2026-05-24

This patch release fixes two downstream migration issues reported from DRIFT
while keeping the supported v2 scope at Level 1-3.

### Fixed

- Fixed default fpm dependency builds so Level 4 C translation units compile
  without downstream users defining `OH_LIB_LEVEL_4P` or `OH_LIB_LEVEL_4S`.
- Added the Fortran v2 helper `oh_context_configure_level3_legacy()` for
  migration code that still has the old active-decomposition sentinel and
  one-based Level 3 boundary IDs.

### Documentation

- Documented the Level 4 fpm build guard in the v2 level-scope design note.
- Documented the Fortran Level 3 legacy configuration helper in the v2 usage
  and context design guides.

### Verification

- `bash tests/test_particle_contract_audit.sh`
- Docker `mpicc` compile check for Level 4 C sources with and without Level 4
  macros.
- Docker fpm builds:
  - `fpm build --compiler mpifort --c-compiler mpicc`
  - `fpm build --compiler mpiifx --c-compiler mpiicx`

## v2.1.1 - 2026-05-24

This patch release keeps the supported scope at Level 1-3 and promotes the
non-default context path from migration-only smoke coverage to a runtime-covered
C/Fortran use path.

### Added

- Added named public `currmode` constants for C and Fortran:
  - `OH_MODE_NORMAL_PRIMARY`
  - `OH_MODE_NORMAL_SECONDARY`
  - `OH_MODE_REBALANCE_SECONDARY`
  - `OH_MODE_ANY_PRIMARY`
  - `OH_MODE_ANY_SECONDARY`
- Added C and Fortran runtime smoke coverage for two heap-owned non-default
  contexts on the same communicator.
- Added direct runtime coverage for `oh_context_transbound1()`,
  `oh_context_transbound2()`, and `oh_context_transbound3()` on non-default
  contexts.

### Changed

- Level-2 injection state entry points now preserve layout opacity by accepting
  `void *` particle pointers internally instead of forcing context callers back
  through `struct S_particle *`.
- Documentation now states that C and Fortran can use non-default Level 1-3
  contexts, with Level 3 geometry, mapping, field facade, particle/accounting
  binding, and transbound calls covered by Docker runtime tests.

### Verification

- `bash tests/test_particle_contract_audit.sh`
- Docker build/runtime smoke using Ubuntu 24.04 with `mpicc`, `gfortran`,
  `mpich`, and `libmpich-dev`: `scripts/docker-build-test.sh`

## v2.1.0 - 2026-05-24

This minor release keeps the supported scope at Level 1-3 and adds explicit
lifecycle control for the mutable particle state that OhHelp stores in the
default context.

### Added

- Added particle-buffer binding APIs:
  - `oh_context_bind_particles()`
  - `oh_context_unbind_particles()`
- Added particle accounting binding APIs for `nphgram`, `totalp`, and `pbase`:
  - `oh_context_bind_particle_accounting()`
  - `oh_context_unbind_particle_accounting()`
- Added storage ownership flags:
  - `OH_PARTICLES_BORROWED`
  - `OH_PARTICLES_OWNED`
- Added matching Fortran `ohhelp_v2` bindings for particle buffer and
  accounting state binding.
- Added guards so transfer calls fail clearly when required particle buffer or
  accounting state is not bound.

### Documentation

- Documented the lifetime and ownership contract for particle buffers,
  accounting arrays, adapters, MPI datatypes, and callback state.
- Clarified that explicit binding currently targets the default context, while
  fully independent non-default contexts remain the next v2.x refactor.

### Release Scope

- Level 1-3 remain the supported API surface.
- Level 4p/4s remain under compile coverage only; full v2 support is deferred
  to v2.x.
- This release is intended as the last checkpoint before continuing the global
  context migration.

### Verification

- `bash tests/test_particle_contract_audit.sh`
- Docker build smoke using Ubuntu 24.04 with `mpicc`, `gfortran`, `mpich`, and
  `libmpich-dev`: `scripts/docker-build-test.sh`

## v2.0.1 - 2026-05-23

This patch release keeps the v2.0 supported scope at Level 1-3 and adds the
missing Fortran raw initialization path for custom particle layouts.

### Added

- Added `ohhelp_v2` raw init bridge APIs:
  - `oh2_init_raw()`
  - `oh3_init_raw()`
- Added `oh_mycomm_v2` for C-interoperable Fortran raw init calls.
- Added C bridge entry points for raw init:
  - `oh_fortran_oh2_init_raw()`
  - `oh_fortran_oh3_init_raw()`
- Added compile-smoke coverage for Fortran custom particle layout init through
  `tests/test_oh_v2_fortran.F90`.

### Documentation

- Updated Fortran usage docs to describe `c_loc()`-based raw init for arbitrary
  `bind(C)` particle layouts.
- Clarified that the traditional Fortran `oh_init()` path remains available for
  the default `type(oh_particle)` layout.

### Release Scope

- Level 1-3 remain the supported v2.0 API surface.
- Level 4p/4s remain under compile coverage only; full v2 support is deferred
  to v2.x.
- The context API still targets the default OhHelp instance. Fully independent
  multiple contexts remain a v2.x task.

### License Notice

This repository contains code derived from the original OhHelp software. Some
original source files include a non-commercial-use notice. Commercial use of
the repository as a whole has not been confirmed.

## v2.0.0 - 2026-05-22

Initial release of the independent `ohhelp2` v2 line.

### Highlights

- Renamed the package/repository line to `ohhelp2`.
- Supported Level 1-3 as the primary usable scope.
- Deferred full Level 4p/4s v2 support to v2.x while keeping compile coverage.
- Added default-context wrappers for Level 1-3 operations through
  `oh_context_*`.
- Added `ohhelp_v2` Fortran bindings for context handles and opaque particle
  adapter handles.
- Added weighted region load support through `oh_set_region_weights()`.
