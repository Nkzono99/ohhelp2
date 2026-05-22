# v2 Particle Contracts and Legacy Hidden Semantics

This note records particle-layout assumptions that were implicit in v1 and must
not be lost during the v2 migration. v2 does not have to preserve the exact
field names, but every semantic dependency below needs either an adapter hook,
a documented v2 replacement, or an intentional removal.

## Level 2/3 Contracts

Level 2 no longer requires a field literally named `nid` for custom particles.
The region identity is whatever `oh_particle_adapter.get_region()` returns, and
updates go through `set_region()`.

For common C structs with `int` fields, v2 provides:

- `oh_particle_adapter_use_int_fields(adapter, region_offset, species_offset)`
- `oh_particle_adapter_use_single_species_int_region(adapter, region_offset)`

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
- Custom particle layout: call
  `oh3_particle_adapter_use_position_fields(adapter, x_offset, y_offset,
  z_offset)` before `oh_set_particle_adapter()`.

The Level-3 position-field mapping reuses OhHelp's own subdomain geometry. Its
neighbor mapping intentionally mutates position fields when periodic wrapping is
needed, matching the original `oh3_map_particle_to_neighbor()` behavior.

## Removal And Injection

There are two different cases.

Normal existing particles:

- If mapping returns a negative destination, Level-2 movement loops skip that
  particle in the next `oh_transbound()`.
- The application must keep its particle histograms/counts consistent with that
  removal decision.

Injected particles:

- `oh_inject_particle()` copies the particle into the injection area.
- If the particle has a negative region at injection time, it is copied but not
  counted for any destination.
- After an injected particle has been counted/remapped, removal must call
  `oh_remove_injected_particle()`. Setting the region field to `-1` alone does
  not decrement injection counters.
- `oh_remove_injected_particle()` validates that the pointer is inside the
  current injection buffer, decrements the relevant counters, then sets the
  region to `-1`.

## Level 4 Contracts Still Requiring Migration

Level 4p/4s still have stronger direct `S_particle` dependencies than Level 2/3.
The remaining hidden contracts include:

- direct reads/writes of `part->nid`,
- direct reads of `part->spec`,
- direct reads/writes of `part->x`, `part->y`, and `part->z`,
- packed-grid id operations such as `Grid_Position()`,
  `Combine_Subdom_Pos()`, `Primarize_Id()`, `Secondarize_Id()`, and
  `Secondary_Injected()`,
- sentinel values:
  - `nid < 0`: removed/skipped particle,
  - `nid == -2`: temporary boundary-exchange marker in Level 4s send buffers.

The v2 target is to move these behind adapter/state helpers. Until that work is
complete, Level 4 custom particle layouts are only partially supported even
though stride-aware storage and copy paths are being migrated.

The public Level-4 injection/removal entry points now use the active particle
adapter for species reads and for writing the negative removal marker. Packed
Level-4 region reads still depend on the legacy `nid` representation until the
packed-id contract is redesigned.

## Migration Checklist

When editing particle movement or mapping code, check for:

- raw `part->nid`, `part->spec`, `part->x/y/z` access,
- raw `state->particles[index]` or `state->send_buffer[index]` indexing,
- direct `struct S_particle` assignment instead of adapter copy helpers,
- assumptions that `sizeof(struct S_particle)` is the particle stride,
- counter updates that depend on `region >= 0`,
- injected-particle paths that need explicit removal accounting,
- Level-4 packed id manipulation that has not yet moved behind adapter helpers.
