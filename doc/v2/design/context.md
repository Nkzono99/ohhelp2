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

/* bind particles, accounting arrays, and optional Level 3 geometry */

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

## Transbound Modes

Use named constants, not magic numbers:

- `OH_MODE_NORMAL_PRIMARY`
- `OH_MODE_NORMAL_SECONDARY`
- `OH_MODE_REBALANCE_SECONDARY`
- `OH_MODE_ANY_PRIMARY`
- `OH_MODE_ANY_SECONDARY`

Short aliases such as `OH_MODE_NORM_PRI` are provided for compact code.
