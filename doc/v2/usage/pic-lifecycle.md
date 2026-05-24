# PIC Integration Lifecycle

Fortran mirror: [pic-lifecycle-fortran.md](pic-lifecycle-fortran.md)

This page describes the C v2 lifecycle. For v1 manuals, see
[`../../v1/`](../../v1/).

## 1. Create And Configure A Context

```c
oh_context *ctx = NULL;
oh_context_create(MPI_COMM_WORLD, &ctx);
oh_context_configure_particles(ctx, nspec, maxfrac);
oh_context_set_region_weights(ctx, weights);
```

Use one context per independent OhHelp instance.

`maxfrac` controls load-balance tolerance. For temporary storage headroom during
bursty injection, size the particle buffer separately:

```c
int maxlocalp = oh_context_max_local_particles_for_capacity(
    ctx, global_particle_limit, capacity_percent, min_margin);
```

## 2. Describe Particle Layout

```c
oh_particle_adapter adapter = oh_default_particle_adapter(MPI_DATATYPE_NULL);
oh_particle_adapter_use_int_fields(&adapter, region_offset, species_offset);
oh_particle_adapter_use_position_fields(&adapter, x_offset, y_offset,
                                        z_offset);
oh_context_set_particle_adapter(ctx, &adapter);
```

For callback adapter layouts, set `get_region`, `set_region`, `get_species`,
and optional mapping callbacks.

## 3. Bind Buffers And Accounting

```c
int sdid[2] = {rank, -1};

oh_context_bind_region_ids(ctx, sdid, OH_PARTICLES_BORROWED);
oh_context_bind_particles(ctx, particles, maxlocalp, OH_PARTICLES_BORROWED);
oh_context_bind_particle_accounting(ctx, nphgram, totalp, pbase,
                                    OH_PARTICLES_BORROWED);
```

Borrowed buffers stay owned by the application. OhHelp mutates the bound
particle buffer and accounting arrays during transfer.

After transbound, `sdid[0]` is the active primary region and `sdid[1]` is the
active secondary region, or `-1` when secondary mode is inactive. Use
`oh_context_get_region_ids(ctx, sdid)` if you want an explicit snapshot instead
of a bound id array.

## 4. Configure Level 3 Geometry

```c
oh_context_configure_level3(ctx, pcoord, sdoms, scoord, nbound, bcond,
                            bounds, ftypes, cfields, ctypes, fsizes);
```

This enables subdomain mapping and field-border exchange.

## 5. PIC Step

A typical Level 3 step is:

```text
push particles
map particle positions to destination regions
scatter current / charge
exchange field borders
solve fields
transbound particles
```

In C:

```c
int dst = oh_context_map_particle_to_subdomain(ctx, x, y, z);
int mode = oh_context_transbound3(ctx, OH_MODE_NORMAL_PRIMARY, stats);
oh_context_get_region_ids(ctx, sdid);
oh_context_exchange_borders(ctx, pfld, sfld, ctype, bcast);
```

`pbase[1]` is the primary/secondary split offset after transbound, and
`pbase[2]` is the total local particle count / end offset.

## Injection

Injection copies a particle into OhHelp's injection area:

```c
oh_context_inject_particle(ctx, &particle);
void *copy = oh_context_inject_particle_get(ctx, &particle);
```

`oh_context_inject_particle_get()` is a C 専用の helper in the sense that it
returns a raw pointer to the injected copy. If a counted injected copy must be
removed, call `oh_context_remove_injected_particle()` instead of only changing
its region value.

## Shutdown

```c
oh_context_unbind_particle_accounting(ctx);
oh_context_unbind_particles(ctx);
oh_context_destroy(ctx);
```
