# v2 Particle Layout And Weighted Load Fortran

C mirror: [v2-particle-and-weight.md](v2-particle-and-weight.md)

Fortran v2 uses `ohhelp_v2` only:

```fortran
use iso_c_binding
use ohhelp_v2
```

For v1 particle layout references, see [`../../v1/`](../../v1/).

## Adapter Handle

Fortran uses opaque handles instead of exposing C structs.

```fortran
type(oh_particle_adapter_handle) :: adapter
integer(c_int) :: ierr

call oh_particle_adapter_create_byte(adapter, stride, ierr)
call oh_particle_adapter_use_integer_fields(adapter, region_offset, &
                                            region_size, species_offset, &
                                            species_size)
call oh_particle_adapter_use_level3_position_fields(adapter, x_offset, &
                                                    y_offset, z_offset)
call oh_context_set_particle_adapter(ctx, adapter)
```

Fortran integer-field helpers treat the particle species field as 1-based.
OhHelp converts it to the internal 0-based species index through the adapter.
For callback adapters, call `oh_particle_adapter_set_species_base(adapter, 1)`
if the callback returns 1-based species ids.

## Callback Adapter

For layouts that cannot be described by offsets, define `bind(C)` callbacks and
register them as a callback adapter:

```fortran
call oh_particle_adapter_set_callbacks(adapter, c_funloc(get_region), &
    c_funloc(set_region), c_funloc(get_species), c_funloc(map_neighbor), &
    c_funloc(map_subdomain))
```

The callback interfaces are exported by `ohhelp_v2`.

## custom particle layout の raw init

The raw init bridge lets existing code pass its own particle buffer and
accounting arrays as `type(c_ptr)` without using v1 modules.

```fortran
type(c_ptr) :: pbuf
pbuf = c_loc(particles(1))

call oh2_init_raw(sdid_ptr, nspec, maxfrac, nphgram_ptr, totalp_ptr, pbuf, &
                  pbase_ptr, maxlocalp, mycomm_ptr, nbor_ptr, pcoord_ptr, &
                  stats, repiter, verbose)
```

For Level 3, use `oh3_init_raw()` with the geometry descriptors.
For context configuration with legacy Fortran Level 3 geometry arrays, use
`oh_context_configure_level3_legacy()`.

## raw init の lifetime / ownership contract

Pointers passed through raw init and bind calls must remain valid until the
context is unbound or destroyed. With `OH_PARTICLES_BORROWED`, OhHelp borrows
the particle buffer and mutable accounting state; the application owns
allocation and lifetime.

If OhHelp owns a buffer, use `OH_PARTICLES_OWNED` and treat the returned pointer
as the active storage.

## Injection

`oh_context_inject_particle_get()` が返す value は `type(c_ptr)` です。
Fortran には pointer-return helper はなく、typed access is recovered by the
application with `c_f_pointer()` if needed.

```fortran
type(c_ptr) :: injected_copy

call oh_context_inject_particle(ctx, particle_ptr)
injected_copy = oh_context_inject_particle_get(ctx, particle_ptr)
call oh_context_remove_injected_particle(ctx, injected_copy)
```

Do not remap a counted injected copy without removing its previous count.

## Weighted Load

```fortran
real(c_double), target :: weights(nregions)

call oh_context_set_region_weights(ctx, weights)
```

Omitting the optional `weights` argument resets all region weights to
`1.0_c_double`.
