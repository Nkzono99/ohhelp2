# Context API

The context API makes OhHelp state explicit. It is the preferred v2 entry point
for both C and Fortran.

For v1 manuals and historical global-state behavior, see
[`../../v1/`](../../v1/).

## Lifecycle

C:

```c
#include "oh_context.h"
#include "oh_mode.h"

oh_context *ctx = NULL;
if (oh_context_create(MPI_COMM_WORLD, &ctx) != 0) {
  /* handle allocation or MPI conversion failure */
}

oh_context_configure_particles(ctx, nspec, maxfrac);
oh_context_set_region_weights(ctx, weights);

/* bind particles, region ids, accounting arrays, and optional Level 3 geometry */

mode = oh_context_transbound3(ctx, OH_MODE_NORMAL_PRIMARY, stats);

oh_context_destroy(ctx);
```

Fortran:

```fortran
use iso_c_binding
use ohhelp_v2

type(oh_context_handle) :: ctx
integer(c_int) :: ierr, mode

call oh_context_create(ctx, comm, ierr)
call oh_context_configure_particles(ctx, nspec, maxfrac)
call oh_context_set_region_weights(ctx, weights)

mode = oh_context_transbound3(ctx, OH_MODE_NORMAL_PRIMARY, stats)

call oh_context_destroy(ctx)
```

## Default And Heap-Owned Contexts

The default context is available through `oh_default_context()` for existing
single-instance integration paths. It is still useful when using the raw init
bridge.

Heap-owned context objects are created with `oh_context_create()` and destroyed
with `oh_context_destroy()`. They are the v2 direction for multiple independent
OhHelp instances. Use them for new code unless a raw initialization path forces
the default context. Multiple independent OhHelp instances should not share
mutable particle, accounting, geometry, or scheduling state.

## Particle Buffer Ownership

Particle storage is attached to a context with bind/unbind calls.

```c
void *bound = oh_context_bind_particles(ctx, particles, maxlocalp,
                                        OH_PARTICLES_BORROWED);
oh_context_unbind_particles(ctx);
```

Ownership is explicit:

- `OH_PARTICLES_BORROWED`: OhHelp stores the pointer and mutates the caller's
  buffer. The caller owns allocation and lifetime.
- `OH_PARTICLES_OWNED`: OhHelp may allocate or replace the buffer and owns the
  bound storage until unbind/destroy.

Transfer calls mutate the bound particle buffer. This is intentional API
behavior, not an incidental side effect.

## Region Id State

Level 2/3 transbound maintains the active primary/secondary region ids. In the
raw v1-style init surface this was visible through caller-owned `sdid`. In the
context API, ids are context-owned unless the application binds storage:

```c
int sdid[2] = {rank, -1};
oh_context_bind_region_ids(ctx, sdid, OH_PARTICLES_BORROWED);
oh_context_get_region_ids(ctx, sdid);
```

`sdid[0]` is the active primary region. `sdid[1]` is the active secondary
region, or `-1` when no secondary region is active. Binding is the recommended
path for applications that refresh region-local field, charging, or flux
buffers after `oh_context_transbound3()`. `oh_context_get_region_ids()` is a
snapshot helper for applications that do not want to bind caller-owned storage.

## Particle Accounting

Level 1-3 transfer logic also needs accounting arrays. The context binds
`nphgram`, `totalp`, and `pbase` together so the transfer path can update
particle counts consistently.

```c
oh_context_bind_particle_accounting(ctx, nphgram, totalp, pbase,
                                    OH_PARTICLES_BORROWED);
```

For borrowed accounting arrays, OhHelp does not take ownership of allocation.
The application must keep the arrays alive for the whole bound lifetime.
`oh_context_unbind_particle_accounting()` detaches them from the context.

`pbase` is an offset/count array, not a Fortran lower-bound array. In C,
`pbase[1]` is the first secondary-particle offset after transbound and
`pbase[2]` is the total local particle count / end offset. In Fortran those are
`pbase(2)` and `pbase(3)`. The primary range is normally `[pbase[0], pbase[1])`;
the secondary range is `[pbase[1], pbase[2])` when `sdid[1] >= 0`.

## Capacity Headroom

`maxfrac` remains the load-imbalance threshold used by rebalancing. Do not use
it as the only particle-buffer capacity policy for bursty injection. For
temporary storage headroom, use:

```c
int maxlocalp = oh_context_max_local_particles_for_capacity(
    ctx, global_particle_limit, capacity_percent, min_margin);
```

`capacity_percent` is independent of `maxfrac`; for example, `250` allocates
ceil-average particles plus 250 percent headroom, subject to `min_margin`.

## Level 3 Geometry

Level 3 geometry and field descriptors are configured on the context:

```c
oh_context_configure_level3(ctx, pcoord, sdoms, scoord, nbound, bcond,
                            bounds, ftypes, cfields, ctypes, fsizes);
```

After configuration, the same context can map particles, exchange field
borders, and run `oh_context_transbound3()`.

Fortran migration code that still has legacy Level 3 arrays can call
`oh_context_configure_level3_legacy()` from `ohhelp_v2`. That helper accepts the
active-decomposition sentinel used by the old Fortran initializer and translates
one-based boundary IDs to the zero-based IDs used by the context API.

Recommended Level 3 ordering is: create/configure context, configure particle
adapter, bind region ids, bind particle buffer, bind accounting arrays,
configure Level 3 geometry, call transbound, then read `sdid`/`pbase` and
refresh application-side region buffers.

## Transbound Modes

Use named constants, not magic numbers:

- `OH_MODE_NORMAL_PRIMARY`
- `OH_MODE_NORMAL_SECONDARY`
- `OH_MODE_REBALANCE_SECONDARY`
- `OH_MODE_ANY_PRIMARY`
- `OH_MODE_ANY_SECONDARY`

Short aliases such as `OH_MODE_NORM_PRI` are provided for compact code.

For Level 1-3 context APIs, applications should handle these values as
`transbound` return modes:

- `OH_MODE_NORMAL_PRIMARY`: the context is operating as a primary region only.
- `OH_MODE_NORMAL_SECONDARY`: the context has an active secondary region and
  the current primary/secondary assignment remains usable.
- `OH_MODE_REBALANCE_SECONDARY`: the secondary assignment was rebuilt; refresh
  secondary-side field, charging, flux, and other region-local buffers from
  the updated `sdid` and `pbase` state.

`OH_MODE_ANY_PRIMARY` and `OH_MODE_ANY_SECONDARY` remain public compatibility
constants for the historical mode encoding and may be accepted as input
`currmode` values. They are not the normal Level 1-3 return contract. New v2
code should not branch on them as expected `transbound` results unless a later
Level 4 or compatibility path documents that behavior explicitly.
