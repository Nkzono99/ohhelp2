# API By OhHelp Level Fortran

C mirror: [api-by-level.md](api-by-level.md)

Fortran では `ohhelp_v2` の opaque handle を使います。v2 guide の Fortran
examples は `use ohhelp_v2` だけを前提にします。v1 style module については
[`../../v1/`](../../v1/) を参照してください。

```fortran
use iso_c_binding
use ohhelp_v2
```

## Common Setup

```fortran
type(oh_context_handle) :: ctx
integer(c_int) :: ierr, mode

call oh_context_create(ctx, comm, ierr)
call oh_context_configure_particles(ctx, nspec, maxfrac)
```

`ctx` は heap-owned context です。粒子 adapter、region weights、particle
buffer、accounting arrays、Level 3 geometry は context に保持されます。

`currmode` には magic number ではなく、`ohhelp_v2` の constants を使います。

```fortran
mode = oh_context_transbound3(ctx, OH_MODE_NORMAL_PRIMARY, stats)
```

## Level 1

Level 1 は負荷分散 schedule と communicator を扱います。粒子転送は利用側で
行います。

```fortran
type(c_ptr) :: nphgram_ptr, totalp_ptr, pbase_ptr

call oh_context_bind_particle_accounting(ctx, nphgram_ptr, totalp_ptr, &
                                         pbase_ptr, OH_PARTICLES_BORROWED)
mode = oh_context_transbound1(ctx, OH_MODE_NORMAL_PRIMARY, stats)
```

## Level 2

Level 2 は particle buffer を OhHelp が移動します。adapter と accounting
arrays を bind してから transbound します。

```fortran
type(oh_particle_adapter_handle) :: adapter
type(c_ptr) :: particles_ptr

call oh_particle_adapter_create_byte(adapter, stride, ierr)
call oh_particle_adapter_use_integer_fields(adapter, region_offset, &
                                            region_size, species_offset, &
                                            species_size)
call oh_context_set_particle_adapter(ctx, adapter)
call oh_context_bind_particles(ctx, particles_ptr, maxlocalp, &
                               OH_PARTICLES_BORROWED)

mode = oh_context_transbound2(ctx, OH_MODE_NORMAL_PRIMARY, stats)
```

`oh_particle_adapter_create_byte` は任意 layout の粒子を byte-stride で扱うための
entry point です。

## Level 3

Level 3 は geometry mapping と field-border exchange を追加します。

```fortran
call oh_particle_adapter_use_level3_position_fields(adapter, x_offset, &
                                                    y_offset, z_offset)
call oh_context_configure_level3(ctx, pcoord_ptr, sdoms_ptr, scoord_ptr, &
                                 nbound, bcond_ptr, bounds_ptr, ftypes_ptr, &
                                 cfields_ptr, ctypes_ptr, fsizes_ptr)

mode = oh_context_transbound3(ctx, OH_MODE_NORMAL_PRIMARY, stats)
```

任意 layout の既存 init argument list を使いたい場合は、`ohhelp_v2` の
raw init bridge を使います。`oh2_init_raw()` / `oh3_init_raw()` は
`type(c_ptr)` で粒子配列と accounting arrays を受け取ります。

既存 Fortran code が Level 3 の active-decomposition sentinel と 1-based
boundary IDs を持っている場合は、`oh_context_configure_level3_legacy()` を
使えます。この helper は sentinel を active decomposition として扱い、
boundary IDs を context API の 0-based 表現へ変換します。
