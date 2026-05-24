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

## 5. PIC Step

```fortran
integer(c_int) :: dst, mode

dst = oh_context_map_particle_to_subdomain(ctx, x, y, z)
mode = oh_context_transbound3(ctx, OH_MODE_NORMAL_PRIMARY, stats)
call oh_context_get_region_ids(ctx, c_loc(sdid(1)))
call oh_context_exchange_borders(ctx, pfld_ptr, sfld_ptr, ctype, bcast)
```

`ctype` and field-operation `ftype` values are zero-based for the v2 context
API. A v1-compatible application wrapper should subtract one from one-based
Fortran field ids before calling `oh_context_bcast_field()`,
`oh_context_reduce_field()`, `oh_context_allreduce_field()`, or
`oh_context_exchange_borders()`.

After `oh_context_transbound3()`, use the bound `sdid` or
`oh_context_get_region_ids()` before refreshing primary/secondary field,
charging, or flux buffers.

`pbase` is an offset/count array. With a Fortran `integer(c_int) :: pbase(3)`,
`pbase(2)` is the primary/secondary split offset and `pbase(3)` is the total
local particle count / end offset. These values are offsets into the bound
particle buffer, not Fortran lower-bound indices.

## Injection

```fortran
type(c_ptr) :: injected_copy

call oh_context_inject_particle(ctx, particle_ptr)
injected_copy = oh_context_inject_particle_get(ctx, particle_ptr)
```

`oh_context_inject_particle_get` returns the injected copy as `type(c_ptr)`.
Remove counted injected particles with `oh_context_remove_injected_particle()`.

`oh_context_set_total_particles()` may be called after injection. In that case
OhHelp finalizes the pending injected copies as ordinary local particles for
the next transbound, so `oh_context_remove_injected_particle()` no longer
applies to those copies.

## Raw Init Bridge

Existing code that already has v1-style init argument arrays can still stay on
the v2 module:

```fortran
call oh3_init_raw(sdid_ptr, nspec, maxfrac, nphgram_ptr, totalp_ptr, pbuf, &
                  pbase_ptr, maxlocalp, mycomm_ptr, nbor_ptr, pcoord_ptr, &
                  sdoms_ptr, scoord_ptr, nbound, bcond_ptr, bounds_ptr, &
                  ftypes_ptr, cfields_ptr, ctypes_ptr, fsizes_ptr, stats, &
                  repiter, verbose)
```

No v1 module is required for this v2 path.
