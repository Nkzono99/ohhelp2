# Changelog

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
