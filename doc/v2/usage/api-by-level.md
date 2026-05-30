# API By OhHelp Level

Fortran mirror: [api-by-level-fortran.md](api-by-level-fortran.md)

This page describes the C v2 surface. For historical v1 APIs, see
[`../../v1/`](../../v1/).

## Common Setup

Use the context API:

```c
#include "oh_context.h"
#include "oh_mode.h"
#include "oh_particle_adapter.h"
#include "oh_particle_ownership.h"

oh_context *ctx = NULL;
oh_context_create(MPI_COMM_WORLD, &ctx);
oh_context_configure_particles(ctx, nspec, maxfrac);
```

Use a heap-owned context for new integrations. A heap-owned context keeps
particle adapter state, region weights, buffers, accounting arrays, and Level 3
geometry separate from other contexts.

Use named mode constants:

```c
mode = oh_context_transbound3(ctx, OH_MODE_NORMAL_PRIMARY, stats);
```

Short aliases such as `OH_MODE_NORM_PRI` / `OH_MODE_NORM_SEC` /
`OH_MODE_REB_SEC` are also available.

Level 1-3 `transbound` returns one of the normal primary/secondary modes or
`OH_MODE_REBALANCE_SECONDARY`. `OH_MODE_ANY_PRIMARY` and
`OH_MODE_ANY_SECONDARY` are compatibility constants for the historical mode
encoding; do not treat them as expected Level 1-3 return values in new v2 code.

`maxfrac` is the load-balance threshold. For particle-buffer capacity headroom,
size the buffer separately:

```c
maxlocalp = oh_context_max_local_particles_for_capacity(
    ctx, global_particle_limit, capacity_percent, min_margin);
```

## Level 1

Level 1 manages the load-balance schedule and communicator. The application
keeps particle movement.

```c
int *nphgram_slot = nphgram;
int *totalp_slot = totalp;
int *pbase_slot = pbase;

oh_context_bind_particle_accounting(ctx, &nphgram_slot, &totalp_slot,
                                    &pbase_slot,
                                    OH_PARTICLES_BORROWED);
mode = oh_context_transbound1(ctx, OH_MODE_NORMAL_PRIMARY, stats);
```

The v2 heap-owned context path initializes Level 1 state explicitly. The
current migration surface supports `stats == 0` and `verbose == 0`; borrowed
`rcounts` / `scounts` from the legacy raw initializer are not accepted yet.

Choose Level 1 if the application already has a particle exchange path.

## Level 2

Level 2 adds particle transfer. Bind both particle storage and accounting
arrays before calling transbound.

```c
MPI_Datatype particle_type = MPI_DATATYPE_NULL;
oh_particle_adapter_make_byte_type(sizeof(struct my_particle), &particle_type);
oh_particle_adapter adapter = oh_default_particle_adapter(particle_type);
int *nphgram_slot = nphgram;
int *totalp_slot = totalp;
int *pbase_slot = pbase;

adapter.stride = sizeof(struct my_particle);
oh_particle_adapter_use_integer_fields(&adapter, region_offset, region_size,
                                       species_offset, species_size);

oh_context_set_particle_adapter(ctx, &adapter);
oh_context_bind_region_ids(ctx, sdid, OH_PARTICLES_BORROWED);
oh_context_bind_particles(ctx, particles, maxlocalp, OH_PARTICLES_BORROWED);
oh_context_bind_particle_accounting(ctx, &nphgram_slot, &totalp_slot,
                                    &pbase_slot,
                                    OH_PARTICLES_BORROWED);

mode = oh_context_transbound2(ctx, OH_MODE_NORMAL_PRIMARY, stats);
```

## Level 3

Level 3 adds geometry mapping and field-border exchange.

```c
oh_context_configure_level3(ctx, pcoord, sdoms, scoord, nbound, bcond,
                            bounds, ftypes, cfields, ctypes, fsizes);

dst = oh_context_map_particle_to_subdomain(ctx, x, y, z);
mode = oh_context_transbound3(ctx, OH_MODE_NORMAL_PRIMARY, stats);
oh_context_get_region_ids(ctx, sdid);
oh_context_exchange_borders(ctx, pfld, sfld, ctype, bcast);
```

For adapter layouts with integer region fields, `dst` should be written back to
the particle's region field before `transbound`. Position-field helpers support
Level 3 neighbor/boundary mapping; they do not replace an existing
integer-region `map_to_subdomain` path.
Use `oh3_particle_adapter_use_neighbor_position_fields()` when positions should
drive only edge/neighbor movement, and
`oh3_particle_adapter_use_subdomain_position_fields()` when positions should
also become the subdomain mapping policy.

If `mode == OH_MODE_REBALANCE_SECONDARY`, refresh region-local buffers after
reading the updated `sdid`/`pbase` state.

Choose Level 3 for normal PIC integrations where OhHelp maps particles between
subdomains and handles field halo exchange.

After transbound, `sdid[0]` / `sdid[1]` are the active primary and secondary
regions. `pbase[1]` is the primary/secondary split offset, and `pbase[2]` is
the total local particle count / end offset.

## Level 4p/4s

Level 4p/4s are supported v2 APIs through the Level 4 C/Fortran entry points.
Use `OH_LIB_LEVEL_4P` or `OH_LIB_LEVEL_4S` to select the Level 4 implementation
at build time. The supported path includes the v2 particle adapter contract,
default and custom particle layouts, injected-particle accounting, and weighted
primary and secondary transbound schedules.

Level 4p/4s currently operate through the default-context state bridge. Use
`oh4p_*` / `oh4s_*` entry points and configure adapters or region weights
through the default-context Level 1-3 APIs before initialization. There is
intentionally no `oh_context_transbound4*` API until Level 4 storage is
heap-context owned.
