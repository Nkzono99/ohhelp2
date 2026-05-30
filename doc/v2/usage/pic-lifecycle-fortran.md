# PIC Integration Lifecycle Fortran

C mirror: [pic-lifecycle.md](pic-lifecycle.md)

v2 の Fortran code は `ohhelp_v2` だけを使います。v1 style module については
[`../../v1/`](../../v1/) を参照してください。

For index conventions, especially where v1-style Fortran wrappers must convert
one-based ids to v2 context ids, see
[`../design/index-conventions.md`](../design/index-conventions.md).

```fortran
use iso_c_binding
use ohhelp_v2
```

## 1. Create And Configure A Context

```fortran
type(oh_context_handle) :: ctx
integer(c_int) :: ierr

call oh_context_create(ctx, comm, ierr)
call oh_context_configure_particles(ctx, nspec, maxfrac)
call oh_context_set_region_weights(ctx, weights)
```

`maxfrac` は load-balance threshold です。一時的な injection burst 用の
buffer capacity は別に見積もれます。

```fortran
maxlocalp = oh_context_max_local_particles_for_capacity( &
  ctx, global_particle_limit, capacity_percent, min_margin)
```

## 2. Describe Particle Layout

```fortran
type(oh_particle_adapter_handle) :: adapter

call oh_particle_adapter_create_byte(adapter, stride, ierr)
call oh_particle_adapter_use_int_fields(adapter, region_offset, species_offset)
call oh_particle_adapter_use_level3_position_fields(adapter, x_offset, &
                                                    y_offset, z_offset)
call oh_context_set_particle_adapter(ctx, adapter)
call oh_particle_adapter_destroy(adapter)
```

The Fortran field helpers assume the particle `species` field is 1-based and
normalize it inside OhHelp. If callbacks return species ids instead, set the
base explicitly with `oh_particle_adapter_set_species_base()`.

For callback adapter layouts, pass `c_funloc()` values to
`oh_particle_adapter_set_callbacks()`.

## 3. Bind Buffers And Accounting

```fortran
integer(c_int), target :: sdid(2)
type(c_ptr) :: particles_ptr, sdid_ptr, nphgram_ptr, totalp_ptr, pbase_ptr

sdid_ptr = c_loc(sdid(1))
particles_ptr = c_loc(particles(1))
nphgram_ptr = c_loc(nphgram(1))
totalp_ptr = c_loc(totalp(1))
pbase_ptr = c_loc(pbase(1))
call oh_context_bind_region_ids(ctx, sdid_ptr, OH_PARTICLES_BORROWED)
call oh_context_bind_particles(ctx, particles_ptr, maxlocalp, &
                               OH_PARTICLES_BORROWED)
call oh_context_bind_particle_accounting(ctx, nphgram_ptr, totalp_ptr, &
                                         pbase_ptr, OH_PARTICLES_BORROWED)
```

The bound arrays must stay alive until unbind or context destroy. `sdid(1)` is
the active primary region id. `sdid(2)` is the active secondary region id, or
`-1` when no secondary region is active.

## 4. Configure Level 3 Geometry

```fortran
type(c_ptr) :: pcoord_ptr, sdoms_ptr, scoord_ptr
type(c_ptr) :: bcond_ptr, bounds_ptr, ftypes_ptr, cfields_ptr, ctypes_ptr
type(c_ptr) :: fsizes_ptr

pcoord_ptr = c_loc(pcoord(1))
sdoms_ptr = c_null_ptr
scoord_ptr = c_loc(scoord(1))
bcond_ptr = c_loc(bcond(1))
bounds_ptr = c_null_ptr
ftypes_ptr = c_loc(ftypes(1))
cfields_ptr = c_loc(cfields(1))
ctypes_ptr = c_loc(ctypes(1))
fsizes_ptr = c_loc(fsizes(1))
call oh_context_configure_level3(ctx, pcoord_ptr, sdoms_ptr, scoord_ptr, &
                                 nbound, bcond_ptr, bounds_ptr, ftypes_ptr, &
                                 cfields_ptr, ctypes_ptr, fsizes_ptr)
```

`oh_context_configure_level3()` expects the C/v2 descriptor representation.
Use `oh_context_configure_level3_legacy()` only when passing historical Fortran
geometry arrays with one-based boundary ids.

既存 Fortran initializer と同じ active-decomposition sentinel と 1-based
boundary IDs を渡したい場合は、`oh_context_configure_level3_legacy()` を
使います。
This helper is not a full replacement for the historical Fortran `oh3_init`
field descriptor path: `cfields` / `ftypes` / `ctypes` / `fsizes` are still the
context descriptor contract. Existing callers that need the old Fortran field
descriptor semantics should use `oh3_init_raw()` for initialization, then call
the v2 `oh_context_*` operations on the default context.

## 5. PIC Step

```fortran
integer(c_int) :: dst, mode

dst = oh_context_map_particle_to_subdomain(ctx, x, y, z)
mode = oh_context_transbound3(ctx, OH_MODE_NORMAL_PRIMARY, stats)
call oh_context_get_region_ids(ctx, c_loc(sdid(1)))
call oh_context_exchange_borders(ctx, pfld_ptr, sfld_ptr, ctype, bcast)
```

For integer-region adapters, store `dst` in the particle's region field before
`transbound`. Position-field helpers are used for Level 3 neighbor/boundary
mapping and do not override an existing integer-region `map_to_subdomain`
policy.

`ctype` and field-operation `ftype` values are zero-based for the v2 context
API. A v1-compatible application wrapper should subtract one from one-based
Fortran field ids before calling `oh_context_bcast_field()`,
`oh_context_reduce_field()`, `oh_context_allreduce_field()`, or
`oh_context_exchange_borders()`.

After `oh_context_transbound3()`, use the bound `sdid` or
`oh_context_get_region_ids()` before refreshing primary/secondary field,
charging, or flux buffers.

Level 1-3 `transbound` returns `OH_MODE_NORMAL_PRIMARY`,
`OH_MODE_NORMAL_SECONDARY`, or `OH_MODE_REBALANCE_SECONDARY`. The `ANY` mode
constants are compatibility values for historical mode encoding, not expected
Level 1-3 results. If `mode == OH_MODE_REBALANCE_SECONDARY`, refresh
region-local buffers after reading the updated `sdid`/`pbase` state.

`pbase` is an offset/count array. With a Fortran `integer(c_int) :: pbase(3)`,
`pbase(2)` is the primary/secondary split offset and `pbase(3)` is the total
local particle count / end offset. These values are offsets into the bound
particle buffer, not Fortran lower-bound indices.

## Injection

```fortran
type(c_ptr) :: injected_copy

injected_copy = oh_context_inject_particle_get(ctx, particle_ptr)
```

`oh_context_inject_particle_get` returns the injected copy as `type(c_ptr)`.
Remove counted injected particles with `oh_context_remove_injected_particle()`.
`oh_context_remap_injected_particle()` is additive; remove the previous count
before remapping a counted injected copy.

`oh_context_set_total_particles()` may be called after injection. In that case
OhHelp finalizes the pending injected copies as ordinary local particles for
the next transbound, so `oh_context_remove_injected_particle()` no longer
applies to those copies.

## Raw Init Bridge

Existing code that already has v1-style init argument arrays can still stay on
the v2 module:

```fortran
pcoord_ptr = c_loc(pcoord(1))
sdoms_ptr = c_null_ptr
scoord_ptr = c_loc(scoord(1))
bcond_ptr = c_loc(bcond(1))
bounds_ptr = c_null_ptr
ftypes_ptr = c_loc(ftypes(1))
cfields_ptr = c_loc(cfields(1))
ctypes_ptr = c_loc(ctypes(1))
fsizes_ptr = c_loc(fsizes(1))
call oh3_init_raw(sdid_ptr, nspec, maxfrac, nphgram_ptr, totalp_ptr, pbuf, &
                  pbase_ptr, maxlocalp, mycomm_ptr, nbor_ptr, pcoord_ptr, &
                  sdoms_ptr, scoord_ptr, nbound, bcond_ptr, bounds_ptr, &
                  ftypes_ptr, cfields_ptr, ctypes_ptr, fsizes_ptr, stats, &
                  repiter, verbose)
```

No v1 module is required for this v2 path.
This is the recommended migration path for existing Fortran Level 3 callers
that already own `cfields` / `ftypes` / `ctypes` / `fsizes` in the historical
`oh3_init` layout.
