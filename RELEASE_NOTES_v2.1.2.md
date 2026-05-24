# ohhelp2 v2.1.2

## Scope

ohhelp2 v2.1.2 is a patch release for the independent v2 line. It keeps the
supported API surface at Level 1-3.

Level 4p/4s sources remain under compile coverage, but full v2 support for
Level 4 is still deferred to v2.x.

## Fixes

- Fixed fpm dependency builds that compile all C translation units. Level 4 C
  source bodies are now enabled only when `OH_LIB_LEVEL_4P` or
  `OH_LIB_LEVEL_4S` is explicitly defined, so downstream Level 1-3 users do not
  need to provide a Level 4 macro.
- Added `oh_context_configure_level3_legacy()` to the Fortran `ohhelp_v2`
  facade. The helper accepts the legacy Fortran Level 3 active-decomposition
  sentinel and translates one-based boundary IDs to the zero-based IDs used by
  the v2 context API.

## Migration Notes

- Prefer `oh_context_configure_level3()` for new v2 code.
- Use `oh_context_configure_level3_legacy()` only when migrating existing
  Fortran code that still stores legacy Level 3 geometry inputs.
- `oh_context_configure_level3()` itself keeps the clean v2 contract:
  `sdoms == NULL` selects active decomposition, non-null `sdoms` selects
  passive decomposition, and boundary IDs are zero-based.

## Verification

- `bash tests/test_particle_contract_audit.sh`
- Docker `mpicc` compile check for Level 4 C sources with and without Level 4
  macros.
- Docker fpm builds:
  - `fpm build --compiler mpifort --c-compiler mpicc`
  - `fpm build --compiler mpiifx --c-compiler mpiicx`

## License Notice

This repository contains code derived from the original OhHelp software. Some
original source files include a non-commercial-use notice. The MIT License
applies only to files or portions explicitly authored by Jin Nakazono, unless
otherwise stated. Commercial use of the repository as a whole has not been
confirmed.
