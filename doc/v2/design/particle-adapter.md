# Particle Adapter

The particle adapter is the v2 contract between OhHelp and application-owned
particle storage. It replaces the old assumption that all users store particles
as `S_particle` or `type(oh_particle)`.

For the historical v1 particle layout, see [`../../v1/`](../../v1/).

## Adapter Contract

An adapter describes:

- element stride,
- MPI datatype for one particle, or a byte datatype created from the stride,
- region getter and setter,
- species getter,
- optional position offsets,
- optional mapping callbacks.

Common integer-field layouts can use offset helpers:

```c
oh_particle_adapter_use_int_fields(&adapter, region_offset, species_offset);
oh_particle_adapter_use_single_species_int_region(&adapter, region_offset);
oh_particle_adapter_use_integer_fields(&adapter, region_offset, region_size,
                                       species_offset, species_size);
oh_particle_adapter_use_single_species_integer_region(&adapter, region_offset,
                                                      region_size);
oh_particle_adapter_set_species_base(&adapter, species_base);
```

Use `oh_particle_adapter_use_integer_fields()` when the region field may need to
carry wide ids such as `OH_BIG_SPACE` or future Level 4 packed ids.

Generic coordinate offsets can be recorded with:

```c
oh_particle_adapter_use_position_fields(&adapter, x_offset, y_offset,
                                        z_offset);
```

For Level 3 automatic position-to-region mapping, use the Level 3 helper:

```c
oh3_particle_adapter_use_position_fields(&adapter, x_offset, y_offset,
                                         z_offset);
```

Fortran exposes the same contract through `ohhelp_v2` opaque adapter handles.

## Region Semantics

The adapter region value is the v2 replacement for the old `nid` field. The
field does not need to be named `nid`, but these semantics matter:

- `nid >= 0`: the particle belongs to a local or remote region.
- `nid < 0`: the particle is skipped by normal Level 2/3 movement loops.
- `nid == -2`: this value is reserved by hidden Level 4/injection paths and
  must not be used casually as an ordinary application region.

For custom layouts, the same meanings apply to the value returned by
`get_region()` and written by `set_region()`.

If a custom adapter supplies `map_to_subdomain()`, the produced destination
uses the same skip rule for negative values. Every non-negative destination
must be a valid node index in `[0, n_of_nodes)`; OhHelp treats out-of-range
destinations as adapter contract violations before they can index particle
accounting or transfer buffers.

## Species Semantics

OhHelp normalizes species to zero-based internal indices before indexing
species arrays. The base of an application particle field is an adapter
property:

- C adapters default to `species_base = 0`.
- Fortran `ohhelp_v2` integer-field helpers set `species_base = 1`, matching
  normal Fortran/PIC species numbering.
- single-species adapters ignore the species base and always map to internal
  species `0`.
- callback adapters keep the adapter default unless the application calls
  `oh_particle_adapter_set_species_base()`.

The old raw Fortran init bridge still applies `specBase = 1` to the default
`S_particle` adapter for migration compatibility. New v2 context code should
prefer adapter-level species-base configuration over changing context-wide
state.

## Mapping Callbacks

Adapter callbacks receive `primary_or_secondary` as a channel selector. Layouts
with one region field may ignore it. Layouts with separate primary and
secondary encodings must honor it.

Level 3 topology tables also use a negative-id convention separate from
particle removal. Some entries in `Neighbors`, `DstNeighbors`, or
`SrcNeighbors` use `-(node + 1)` to mark an indirect or periodic neighbor while
preserving the absolute node id. The implementation resolves this through
`Neighbor_Id()` and `AbsNeighbors`. This convention belongs to topology tables;
normal particle regions still use negative values as remove/skip markers unless
inside a documented packed-id path.

Level 3 boundary mapping may mutate position fields when periodic wrapping is
needed. This is part of the mapping contract.

Position-field helpers provide coordinates for Level 3 neighbor/boundary
mapping. If the adapter already has a `map_to_subdomain()` implementation, such
as the integer-region helper used for v1-style load balancing, adding position
fields must not replace that subdomain mapper. New v2 integrations should map a
particle's physical position explicitly with `oh_context_map_particle_to_subdomain()`
and store the result through the adapter region field or callback before
`transbound`.

## Injection Accounting

Injected-particle accounting is part of the particle contract.

- `oh_inject_particle()` copies a particle into OhHelp's injection area.
- `oh_inject_particle_get()` returns the pointer to the injected copy.
- `oh_remove_injected_particle()` decrements the injection counter and marks the
  injected copy as removed.
- `oh_remap_injected_particle()` is additive; use it only for an injected copy
  that was not already counted, or after removing its previous count.
- `remap_injected_particle()` is additive for the same reason.

Setting a region field negative is not enough to undo a counted injection. Use
`oh_remove_injected_particle()` to avoid double counting.

Injected copies are recognized by buffer index. Indices from `totalParts` to
`totalParts + nOfInjections - 1` are the injection area. `InjectedParticles` is
the internal primary/secondary/species counter array that tracks those copies.
Level 2 centralizes those updates in `state_update_injected_particle_count()`
so inject, remap, and remove paths use the same accounting rule.

For Level 3/context adapters, pending injected particles are accounted with the
same subdomain mapper used later by `move_injected_to_sendbuf()`. When the
adapter uses the integer-region helper, that mapper reads the region field; a
position-field helper alone does not make the physical position authoritative
for load balancing. If an injected particle should be routed by physical
position, first call `oh_context_map_particle_to_subdomain()` and store the
returned region id in the injected copy, or provide an explicit callback
`map_to_subdomain()` with that policy.

Calling `set_total_particles_state()` after injection finalizes pending
injected copies as ordinary local particles by clearing `nOfInjections` and
`InjectedParticles` after their counts are included in `totalParts`. This keeps
post-injection `oh_context_set_total_particles()` compatible with the next
Level 2/3 transbound while avoiding a second injected-particle pass over stale
buffer indices.

There is no current `original` particle field or hidden original-particle flag.

## Level 4 Boundary

Level 4p/4s are not part of the v2.0 supported surface. Their remaining
semantics are documented here so the adapter boundary does not regress:

- packed-grid id operations,
- `OH_BIG_SPACE` region width,
- `level4_secondary_region_offset`,
- boundary plane thickness,
- default context layout constraints for Level 4 state binding.

Level 4 code should keep packed-id representation details behind local helper
functions until the v2.x adapter contract is finalized.

Level 4 still treats `state->particles`, `state->send_buffer`,
`state->recv_buffer_bases`, and `state->level4_boundary_send_buffer` as
adapter-strided particle storage or cursors. Temporary `struct S_particle *`
aliases are migration-boundary handles for helper and MPI calls only; they do
not permit direct field access or raw particle pointer arithmetic.
