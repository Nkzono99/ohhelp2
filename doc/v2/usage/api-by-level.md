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

## Level 1

Level 1 manages the load-balance schedule and communicator. The application
keeps particle movement.

```c
oh_context_bind_particle_accounting(ctx, nphgram, totalp, pbase,
                                    OH_PARTICLES_BORROWED);
mode = oh_context_transbound1(ctx, OH_MODE_NORMAL_PRIMARY, stats);
```

Choose Level 1 if the application already has a particle exchange path.

## Level 2

Level 2 adds particle transfer. Bind both particle storage and accounting
arrays before calling transbound.

```c
oh_particle_adapter adapter = oh_default_particle_adapter(MPI_DATATYPE_NULL);
oh_particle_adapter_use_integer_fields(&adapter, region_offset, region_size,
                                       species_offset, species_size);

oh_context_set_particle_adapter(ctx, &adapter);
oh_context_bind_particles(ctx, particles, maxlocalp, OH_PARTICLES_BORROWED);
oh_context_bind_particle_accounting(ctx, nphgram, totalp, pbase,
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
oh_context_exchange_borders(ctx, pfld, sfld, ctype, bcast);
```

Choose Level 3 for normal PIC integrations where OhHelp maps particles between
subdomains and handles field halo exchange.

## Level 4p/4s

Level 4p/4s are not v2.0 supported APIs. They remain under compile coverage and
are documented as a v2.x target in [Level Scope](../design/level-scope.md).
