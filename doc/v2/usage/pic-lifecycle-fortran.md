# PIC Integration Lifecycle Fortran

C mirror: [pic-lifecycle.md](pic-lifecycle.md)

v2 の Fortran code は `ohhelp_v2` だけを使います。v1 style module については
[`../../v1/`](../../v1/) を参照してください。

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

## 2. Describe Particle Layout

```fortran
type(oh_particle_adapter_handle) :: adapter

call oh_particle_adapter_create_byte(adapter, stride, ierr)
call oh_particle_adapter_use_int_fields(adapter, region_offset, species_offset)
call oh_particle_adapter_use_level3_position_fields(adapter, x_offset, &
                                                    y_offset, z_offset)
call oh_context_set_particle_adapter(ctx, adapter)
```

For callback adapter layouts, pass `c_funloc()` values to
`oh_particle_adapter_set_callbacks()`.

## 3. Bind Buffers And Accounting

```fortran
type(c_ptr) :: particles_ptr, nphgram_ptr, totalp_ptr, pbase_ptr

call oh_context_bind_particles(ctx, particles_ptr, maxlocalp, &
                               OH_PARTICLES_BORROWED)
call oh_context_bind_particle_accounting(ctx, nphgram_ptr, totalp_ptr, &
                                         pbase_ptr, OH_PARTICLES_BORROWED)
```

The bound arrays must stay alive until unbind or context destroy.

## 4. Configure Level 3 Geometry

```fortran
call oh_context_configure_level3(ctx, pcoord_ptr, sdoms_ptr, scoord_ptr, &
                                 nbound, bcond_ptr, bounds_ptr, ftypes_ptr, &
                                 cfields_ptr, ctypes_ptr, fsizes_ptr)
```

## 5. PIC Step

```fortran
integer(c_int) :: dst, mode

dst = oh_context_map_particle_to_subdomain(ctx, x, y, z)
mode = oh_context_transbound3(ctx, OH_MODE_NORMAL_PRIMARY, stats)
call oh_context_exchange_borders(ctx, pfld_ptr, sfld_ptr, ctype, bcast)
```

## Injection

```fortran
type(c_ptr) :: injected_copy

call oh_context_inject_particle(ctx, particle_ptr)
injected_copy = oh_context_inject_particle_get(ctx, particle_ptr)
```

`oh_context_inject_particle_get` returns the injected copy as `type(c_ptr)`.
Remove counted injected particles with `oh_context_remove_injected_particle()`.

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
