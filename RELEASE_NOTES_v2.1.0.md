## Scope

ohhelp2 v2.1.0 is a minor release for the independent v2 line. It keeps the
supported API surface at Level 1-3 and adds explicit lifecycle control for the
mutable particle state currently stored in the default OhHelp context.

This release is intended as the checkpoint before continuing the broader global
context migration.

Level 4p/4s sources remain under compile coverage, but full v2 support for
Level 4 is still deferred to v2.x.

## Highlights

- Added particle-buffer binding APIs:
  - `oh_context_bind_particles()`
  - `oh_context_unbind_particles()`
- Added particle accounting binding APIs for `nphgram`, `totalp`, and `pbase`:
  - `oh_context_bind_particle_accounting()`
  - `oh_context_unbind_particle_accounting()`
- Added ownership flags:
  - `OH_PARTICLES_BORROWED`
  - `OH_PARTICLES_OWNED`
- Added matching Fortran `ohhelp_v2` bindings for particle buffer and
  accounting state binding.
- Added transfer guards for missing particle buffer or accounting bindings.
- Documented the lifetime requirements for particle buffers, accounting arrays,
  adapters, MPI datatypes, and callback-owned state.

## Migration Notes

- Existing `oh_init()` / `oh2_init_raw()` / `oh3_init_raw()` users can keep using
  the current initialization path. These calls still bind the default-context
  particle state automatically.
- New context-oriented code can use explicit bind/unbind calls to make the
  side-effect boundary visible.
- Borrowed storage is never freed by OhHelp. Owned storage is allocated by
  OhHelp and released during the matching unbind call.
- Fully independent non-default contexts remain a v2.x task.

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
