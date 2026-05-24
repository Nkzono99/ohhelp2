## Scope

ohhelp2 v2.1.1 is a patch release for the independent v2 line. It keeps the
supported API surface at Level 1-3 and makes the non-default context path usable
from both C and Fortran.

Level 4p/4s sources remain under compile coverage, but full v2 support for
Level 4 is still deferred to v2.x.

## Highlights

- C and Fortran can create heap-owned non-default contexts and use Level 1-3
  context APIs.
- Runtime smoke tests create two non-default contexts on the same communicator,
  configure Level 3 geometry, bind independent particle/accounting storage, and
  call `oh_context_transbound1/2/3`.
- C runtime coverage additionally verifies context-local Level 3 particle
  adapter state.
- Public `currmode` constants are available in C and Fortran:
  - `OH_MODE_NORMAL_PRIMARY`
  - `OH_MODE_NORMAL_SECONDARY`
  - `OH_MODE_REBALANCE_SECONDARY`
  - `OH_MODE_ANY_PRIMARY`
  - `OH_MODE_ANY_SECONDARY`
- Level-2 injection internals keep context particle pointers layout-opaque.

## Migration Notes

- Existing default-context `oh_init()` / `oh2_init_raw()` / `oh3_init_raw()`
  users can keep using the current initialization path.
- New C code can use `oh_context_create()` and `oh_context_configure_level3()`
  for heap-owned contexts.
- New Fortran code can use the `ohhelp_v2` `oh_context_handle` API with
  `iso_c_binding`.
- Prefer the named `OH_MODE_*` constants instead of raw `0`, `1`, or `-1`
  `currmode` values.

## Verification

- `bash tests/test_particle_contract_audit.sh`
- Docker build/runtime smoke using Ubuntu 24.04 with `mpicc`, `gfortran`,
  `mpich`, and `libmpich-dev`: `scripts/docker-build-test.sh`

## License Notice

This repository contains code derived from the original OhHelp software. Some
original source files include a non-commercial-use notice. The MIT License
applies only to files or portions explicitly authored by Jin Nakazono, unless
otherwise stated. Commercial use of the repository as a whole has not been
confirmed.
