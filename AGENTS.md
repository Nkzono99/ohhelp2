# AGENTS.md

This repository is the v2 line of OhHelp. Compatibility with the v1 public API,
binary layout, or behavior is not a constraint unless a task explicitly says so.

## Direction

- Prefer a cleaner v2 design over preserving historical interfaces.
- Reduce library-wide mutable globals. New state should move toward explicit
  context objects that can support multiple independent OhHelp instances.
- Do not make `S_particle` / `oh_particle` the long-term owner of user particle
  layout. The library should operate on user-owned particle data through an
  adapter, offsets, callbacks, or an opaque byte-stride API.
- Load balancing must support non-uniform per-region particle cost. Particle
  count alone is not an acceptable balancing metric for v2.
- Keep C and Fortran users in scope, but do not block a C-side redesign merely
  to preserve v1 Fortran signatures. Add clear migration shims where useful.

## Current v2 Refactor Targets

1. Introduce a context-based API and migrate global variables into that context.
2. Replace fixed particle structs with a particle adapter contract:
   - element size / stride,
   - MPI datatype or pack/unpack callbacks,
   - accessors for region id, species, and coordinates where needed.
3. Make weighted load the primary balancing metric:
   - per-region `real(8)` / `double` weight,
   - region load = local particle count in that region times region weight,
   - balancing target = total weighted load divided by node count,
   - transfer schedules still move integer particle counts.
4. Keep generated Markdown documentation under `doc/v1/markdown/` reproducible
   from `scripts/convert_pdfs_to_md.py`.

## Engineering Notes

- Treat the old PDF manuals as historical references, not binding v2 specs.
- Prefer small, compilable migration steps over large rewrites that leave the
  library half-converted.
- When changing balancing behavior, document whether a path is count-based,
  weight-based, or a temporary compatibility shim.
- If a change touches MPI communication or particle layout, verify at least
  compile-level coverage and add a focused sample or test when practical.
- Treat `tests/test_particle_contract_audit.sh` as the guardrail for hidden
  particle-layout contracts. If a new direct `nid`/`spec`/coordinate access is
  intentional, update `doc/v2/design/particle-adapter.md` in the same change
  and keep the access inside the documented migration boundary.
- Treat injected-particle accounting as part of the particle contract. In
  Level 2/3, remap/remove APIs operate on the injected copy inside OhHelp's
  particle buffer, and remap is additive; avoid examples or code paths that
  remap an already-counted injected particle without first removing its count.
