## Scope

ohhelp2 v2.0.1 is a patch release for the independent v2 line. It keeps the
v2.0 supported scope at Level 1-3 and fills in the Fortran raw initialization
path for custom particle layouts.

Level 4p/4s sources remain under compile coverage, but full v2 support for
Level 4 is still deferred to v2.x.

## Highlights

- Added Fortran `ohhelp_v2` raw init bridge APIs:
  - `oh2_init_raw()`
  - `oh3_init_raw()`
- Added `oh_mycomm_v2` for C-interoperable raw init calls from Fortran.
- Added C bridge entry points:
  - `oh_fortran_oh2_init_raw()`
  - `oh_fortran_oh3_init_raw()`
- Updated Fortran usage docs so arbitrary `bind(C)` particle layouts use
  `c_loc()`, a registered particle adapter, and the raw init bridge.
- Kept the traditional Fortran `oh_init()` path for the default
  `type(oh_particle)` layout.
- Added compile-smoke coverage for the raw init API in
  `tests/test_oh_v2_fortran.F90`.

## Migration Notes

- Existing Fortran Level 1-3 users with `type(oh_particle)` can keep using
  `ohhelp_f.h` and `oh_init()`.
- Fortran custom particle layouts should use `use ohhelp_v2`, a
  `type(oh_particle_adapter_handle)`, `c_loc()`, and `oh2_init_raw()` or
  `oh3_init_raw()`.
- The context API currently targets the default OhHelp instance. Fully
  independent multiple contexts remain a v2.x task.

## Verification

- `bash tests/test_particle_contract_audit.sh`
- Docker build smoke using Ubuntu 24.04 with `mpicc`, `gfortran`, `mpich`, and
  `libmpich-dev`: `scripts/docker-build-test.sh`

## License Notice

This repository contains code derived from the original OhHelp software. Some
original source files include a non-commercial-use notice. The MIT License
applies only to files or portions explicitly authored by Jin Nakazono, unless
otherwise stated. Commercial use of the repository as a whole has not been
confirmed.
