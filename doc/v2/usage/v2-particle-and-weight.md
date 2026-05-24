# v2 Particle Layout And Weighted Load

Fortran mirror: [v2-particle-and-weight-fortran.md](v2-particle-and-weight-fortran.md)

This page describes the C v2 particle adapter and weighted-load path. For v1
particle layout details, see [`../../v1/`](../../v1/).

## Particle Adapter

v2 treats the application particle array as a byte-stride buffer. OhHelp does
not require `S_particle`. 粒子ポインタを `void *` として bind し、layout は
adapter で説明します。

```c
oh_particle_adapter adapter = oh_default_particle_adapter(MPI_DATATYPE_NULL);
oh_particle_adapter_use_integer_fields(&adapter, region_offset, region_size,
                                       species_offset, species_size);
oh_particle_adapter_use_position_fields(&adapter, x_offset, y_offset,
                                        z_offset);
oh_context_set_particle_adapter(ctx, &adapter);
```

Use `OH_BIG_SPACE`-capable integer fields when region ids may exceed `int`.
C adapters use 0-based species ids by default. If an application stores
1-based species ids, set `oh_particle_adapter_set_species_base(&adapter, 1)`
before binding the adapter to a context.

## Borrowed Buffer

```c
oh_context_bind_particles(ctx, particles, maxlocalp, OH_PARTICLES_BORROWED);
```

A borrowed buffer stays allocated by the application. OhHelp may reorder and
overwrite elements during `oh_context_transbound2()` or
`oh_context_transbound3()`.

## Accounting Arrays

Level 1-3 transfer also mutates accounting arrays:

```c
oh_context_bind_particle_accounting(ctx, nphgram, totalp, pbase,
                                    OH_PARTICLES_BORROWED);
```

The `accounting arrays` must describe the same species/region layout as the
particle adapter. The application should not remap an already-counted injected
particle without first removing its count; otherwise 二重計上 can occur.

## Weighted Load

```c
double weights[nregions];
oh_context_set_region_weights(ctx, weights);
```

Region load is particle count multiplied by region weight. Passing `NULL`
resets every region weight to `1.0`.

## Injection

When injecting particles, use the context helpers:

```c
oh_context_inject_particle(ctx, part);
void *copy = oh_context_inject_particle_get(ctx, part);
oh_context_remove_injected_particle(ctx, copy);
```

`oh_context_remap_injected_particle()` is additive. Use it only after a previous
count has been removed, or for an injected copy that was not counted yet.
