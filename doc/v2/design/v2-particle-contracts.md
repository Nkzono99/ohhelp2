# v2 Particle Contracts and Legacy Hidden Semantics

This note records particle-layout assumptions that were implicit in v1 and must
not be lost during the v2 migration. v2 does not have to preserve the exact
field names, but every semantic dependency below needs either an adapter hook,
a documented v2 replacement, or an intentional removal.

## Level 2/3 Contracts

Level 2 no longer requires a field literally named `nid` for custom particles.
The region identity is whatever `oh_particle_adapter.get_region()` returns, and
updates go through `set_region()`. The callback type is
`oh_particle_region_t`, not `int`, so Level-4 packed ids and `OH_BIG_SPACE`
values are not truncated by the adapter boundary.

For common C structs with `int` fields, v2 provides:

- `oh_particle_adapter_use_int_fields(adapter, region_offset, species_offset)`
- `oh_particle_adapter_use_single_species_int_region(adapter, region_offset)`

For common C structs with wider signed integer fields, v2 also provides:

- `oh_particle_adapter_use_integer_fields(adapter, region_offset, region_size,
  species_offset, species_size)`
- `oh_particle_adapter_use_single_species_integer_region(adapter,
  region_offset, region_size)`

Use the width-aware helpers when the region field may need to carry
`OH_BIG_SPACE` or Level-4 packed ids.

The legacy `S_particle.nid` semantics are:

- `nid >= 0`: particle belongs to a region/subdomain.
- `nid < 0`: particle is skipped by Level-2 movement loops and is effectively
  removed from normal transfer output.
- In POS-aware mode, `nid` may be packed. It can encode a neighbor id,
  subdomain id, grid position, and secondary-injected state.

For custom particles, the same meanings apply to the adapter-provided region
value. The field does not need to be called `nid`.

`S_particle.spec` was the species id. v2 maps that through
`oh_particle_adapter.get_species()`. If there is only one species, the adapter
can use `oh_particle_adapter_use_single_species_int_region()` and always return
species zero.

Level 3 has two mapping modes:

- Default `S_particle`: after `oh3_init()`, OhHelp installs mapping callbacks
  that read `x`, `y`, and `z` from `S_particle`.
- Custom particle layout: call `oh_particle_adapter_use_position_fields()` to
  set offsets only, or `oh3_particle_adapter_use_position_fields()` /
  `oh_set_particle_position_fields()` before `oh_set_particle_adapter()` to
  also install Level-3 geometry mapping callbacks.

The Level-3 position-field mapping reuses OhHelp's own subdomain geometry. Its
neighbor mapping intentionally mutates position fields when periodic wrapping is
needed, matching the original `oh3_map_particle_to_neighbor()` behavior.

Adapter callbacks receive `primary_or_secondary` as a channel selector. Level 2
mostly treats this as the primary/secondary histogram side, but POS-aware
Level 3/4 uses it to decode different packed-region meanings through
`AbsNeighbors[0]` and `AbsNeighbors[1]`. Custom adapters that store separate
primary and secondary region encodings must honor this argument; adapters with a
single region field may ignore it.

Neighbor tables have a separate negative-id convention that must not be confused
with a removed particle region. In Level 3 and Level 4, some entries in
`Neighbors`, `DstNeighbors`, or `SrcNeighbors` use `-(node + 1)` to mark a
periodic or otherwise indirect neighbor while preserving the absolute node id.
The code resolves that form through `Neighbor_Id()` or by filling
`AbsNeighbors`. That convention belongs to topology tables only; user particle
region values still use negative values as removal/skip markers unless they are
inside a documented packed Level-4 encoding path.

Species numbering is also normalized at the adapter boundary. C entry points use
zero-based species ids, while Fortran-compatible entry points set `specBase = 1`
and then subtract it before indexing internal arrays. A custom
`get_species()` callback should return the species numbering used by the caller
side of the API; OhHelp will normalize it before using `Particle_Spec()`.

## Removal And Injection

There are two different cases.

Normal existing particles:

- If mapping returns a negative destination, Level-2 movement loops skip that
  particle in the next `oh_transbound()`.
- The application must keep its particle histograms/counts consistent with that
  removal decision.

Injected particles:

- `oh_inject_particle()` copies the particle into the injection area.
- C callers that need to remap or remove the injected copy can use
  `oh_inject_particle_get()` to receive the pointer inside the OhHelp particle
  buffer.
  Fortran-compatible APIs keep the v1-style interface; callers pass the
  injected particle element already stored in `pbuf`.
- If the particle has a negative region at injection time, it is copied but not
  counted for any destination.
- After an injected particle has been counted/remapped, removal must call
  `oh_remove_injected_particle()`. Setting the region field to `-1` alone does
  not decrement injection counters.
- `oh_remap_injected_particle()` is additive. It is for particles injected with
  a negative region, or for injected particles whose previous count has already
  been removed with `oh_remove_injected_particle()`.
- `oh_remove_injected_particle()` validates that the pointer is inside the
  current injection buffer, decrements the relevant counters, then sets the
  region to `-1`.
- Injected-particle pointers are recognized by their buffer index. Indices
  greater than or equal to `totalParts` and lower than
  `totalParts + nOfInjections` are treated as the injection area.
- `InjectedParticles` is an internal counter array with primary/secondary and
  species dimensions. Mapping/removal code must update it when an injected
  particle becomes local, becomes secondary-local, or is explicitly removed.
  Level 2 centralizes those count updates in
  `state_update_injected_particle_count()` so inject/remap/remove paths share
  the same accounting rule.
- There is no current `original` particle field or hidden original-particle
  flag in `S_particle` or the active C implementation. The injected-vs-normal
  distinction is derived from the particle buffer index and the injection
  counters above.

## Level 4 Contracts Still Requiring Migration

Level 4p/4s still have stronger direct `S_particle` dependencies than Level 2/3.
The remaining hidden contracts include:

- direct reads/writes of `part->nid`,
- direct reads of `part->spec`,
- packed-grid id operations such as `Grid_Position()`,
  `Combine_Subdom_Pos()`, `Primarize_Id()`, `Secondarize_Id()`, and
  `Secondary_Injected()`,
- `OH_BIG_SPACE` / `OH_nid_t` width: packed ids include subdomain and grid
  position bits, so Level 4 initialization can reject configurations that do
  not fit in `int` ids,
- `gridMask` and `logGrid`: these encode the grid-position bitfield size inside
  packed region ids,
- sentinel values:
  - `nid < 0`: removed/skipped particle,
  - `nid == -2`: temporary boundary-exchange marker in Level 4s send buffers.

The v2 target is to move these behind adapter/state helpers. Until that work is
complete, Level 4 custom particle layouts are only partially supported even
though stride-aware storage and copy paths are being migrated.

Level 4s has one additional geometry contract: boundary plane thickness must be
one grid cell. The current implementation aborts otherwise, so a v2 redesign
must either preserve that constraint explicitly or replace the scheduling logic.

The public Level-4 injection/removal entry points now use the active particle
adapter for species reads and for writing the negative removal marker. Packed
Level-4 region and coordinate reads now pass through local helpers using the
active particle adapter. Packed-id semantics still depend on the legacy `nid`
encoding until the packed-id contract is redesigned.

## Migration Checklist

When editing particle movement or mapping code, check for:

- raw `part->nid`, `part->spec`, `part->x/y/z` access,
- raw `state->particles[index]` or `state->send_buffer[index]` indexing,
- raw `state->particles + index`, `state->send_buffer + index`,
  `Particles[index]`, `SendBuf[index]`, or `sb[index] = ...` movement paths,
- direct `struct S_particle` assignment instead of adapter copy helpers,
- assumptions that `sizeof(struct S_particle)` is the particle stride,
- counter updates that depend on `region >= 0`,
- injected-particle paths that need explicit removal accounting,
- `specBase` or caller-side one-based species indexing,
- `primary_or_secondary` callback behavior in mapping code,
- negative topology-table ids encoded as `-(node + 1)`,
- Level-4 packed id manipulation that has not yet moved behind adapter helpers.

`tests/test_particle_contract_audit.sh` enforces the current boundary:

- Public C headers must keep include guards, and shared public structs must not
  be multiply defined when `ohhelp1.h` and `ohhelp_c.h` are included together.
- `ohhelp_c.h` must include the level-specific public headers so declarations
  stay anchored to the same public API surface while the compatibility macro
  layer is being reduced.
- `ohhelp_c.h` must not duplicate `ohN_*` function prototypes; those
  declarations belong in the guarded level headers.
- Public Level-2/4 headers must not encode `S_particle.nid` or
  `S_particle.spec` directly. Packed-id helper macros must read/write the
  region through the active `oh_particle_adapter`.
- Public Level-2 headers must not publish POS-aware packed-id helper macros such
  as `Subdomain_Id()` or `Primarize_Id()`; those semantics belong behind
  state-aware implementation helpers.
- Public Level-2 headers must not publish `EXTERN` mutable implementation
  globals. During migration those declarations belong in internal headers under
  `src/c/`.
- Public Level-2 headers must not publish implementation helper prototypes
  used only by higher library levels; those declarations belong in
  `src/c/ohhelp2_internal.h`.
- Public Level-1 headers must not publish mutable implementation globals or the
  default communicator storage. During migration those declarations belong in
  `src/c/ohhelp1_internal.h`; public code should use API calls rather than
  linking against `nOfNodes`, `myRank`, `fam_comm`, or similar mirrors.
- Public headers must not publish hidden particle-accounting globals such as
  `InjectedParticles`, `nOfInjections`, `specBase`, `primaryParts`,
  `secondaryBase`, `gridMask`, `logGrid`, `BoundaryCondition`, or
  `BoundarySendBuf`.
- Public Level-1 headers must not publish implementation struct layouts for the
  node tree, communication lists, rebalance heaps, communicator containers, or
  stats accumulators; those layouts belong in `src/c/ohhelp1_internal.h`.
- Public Level-1 headers must not publish implementation helper prototypes
  used only by other library levels; those declarations belong in
  `src/c/ohhelp1_internal.h`.
- Public headers must not publish the `oh_state` or default context layout.
  `struct oh_state` and `OhDefaultState` belong in
  `src/c/oh_context_internal.h`, and user code should treat `oh_context` as an
  opaque handle even while the default-context migration is incomplete.
- Public Level-4 headers must not publish `EXTERN` mutable implementation
  globals or internal scheduling structs. Those declarations belong in
  `src/c/ohhelp4*_internal.h`.
- Public Level-3 headers must not publish `EXTERN` mutable implementation
  globals or internal field/subdomain structs. Those declarations belong in
  `src/c/ohhelp3_internal.h`.
- Public Level-3 headers must not publish implementation helper prototypes
  used only by Level-4; those declarations belong in
  `src/c/ohhelp3_internal.h`.
- Level 2 implementation code must not directly read or write `nid` or `spec`.
  Region writes use `OH_nid_t` at the state helper boundary so packed ids are
  not narrowed before reaching the active adapter.
- Level 3 implementation code must not directly read or write `nid` or `spec`;
  direct `x/y/z` access is limited to the default `S_particle` mapping adapter.
- Level 4 implementation code must not directly read or write `nid` or `spec`.
  Packed-id manipulation is still present, but it must pass through local helper
  functions/macros using the active particle adapter.
- Level 4 packed-id manipulation must be implemented in
  `src/c/ohhelp4_particle.h`; Level-4p/4s call sites may keep compatibility
  macros, but the bit layout arithmetic should not be duplicated in those
  translation units.
- Level-4p/4s call sites no longer use the legacy packed-id compatibility
  macros; they call the `level4_*` helper functions directly.
- The secondary-injected packed-id offset is centralized in
  `level4_secondary_region_offset()` so primarize, secondarize, and injected
  detection share one definition.
- Level 4 negative sentinel writes are centralized in
  `level4_mark_particle_removed()` and
  `level4_mark_boundary_exchange_particle()` so the `-1` removed marker and
  `-2` boundary-exchange marker do not leak back into call sites.
- Level 4 implementation code must not directly read or write particle
  coordinates. Position access must pass through adapter offsets.
- Level 4 particle-buffer allocation must use adapter stride, not
  `sizeof(struct S_particle)`, when custom adapters can be active.
- Level 4 particle/send-buffer indexing and compaction must use
  `level4_particle_at()`, `oh_particle_buffer_at()`,
  `oh_particle_buffer_index()`, or `level4_copy_particle_to_buffer()` instead
  of typed array indexing or pointer arithmetic.
- Shared position offset arithmetic belongs in
  `oh_particle_adapter_position()` or
  `oh_particle_adapter_const_position()`, not in level-specific code.
- New direct particle-field access must not spread outside the known migration
  files: `oh_particle_adapter.c`, `ohhelp3.c`, `ohhelp4p.c`, and `ohhelp4s.c`.

This test is intentionally not a declaration that Level 4 is finished. It keeps
the remaining hidden contracts visible while preventing new v2 code from
depending on them accidentally.
