# Weighted Load Balancing

v2 balances estimated work, not only particle count.

For historical count-based behavior, see [`../../v1/`](../../v1/).

## Load Model

Each region has a `double` / `real(c_double)` weight. The scheduler computes:

```text
region load = local particle count in that region * region weight
target load = total weighted load / number of nodes
```

Transfer schedules still move integer particle counts. Weights affect the
amount of load each region contributes to the target and rebalance decision.

Weights must be finite and greater than zero. Passing `NULL` resets all region
weights to `1.0` and disables non-uniform weighted mode. Supplying all `1.0`
weights also uses count-equivalent behavior, even though the load helper uses
the same weighted formula.

When a weighted rebalance chooses how many particles to transfer from a donor
region, it rounds up the receiver's load deficit divided by the donor region
weight and caps the result by the donor's available integer particle count.

## C

```c
double weights[nregions];
/* fill weights; pass NULL to reset all weights to 1.0 */
oh_context_set_region_weights_n(ctx, weights, nregions);
```

`oh_context_region_count(ctx)` returns the required array length. Setting
weights requires configured region storage, which is established by
`oh_context_configure_particles()`, `oh3_init_raw()`, or the equivalent
Level-1/2/3 initialization path. It does not require Level 3 geometry.
`oh_context_is_level3_configured(ctx)` can be used separately when an
application needs to gate geometry-dependent work.
`oh_context_set_region_weights(ctx, weights)` remains as an uncounted
compatibility shim; new C code should prefer the counted form.

## Fortran

```fortran
real(c_double), target :: weights(nregions)

call oh_context_set_region_weights(ctx, weights)
```

`size(weights)` must match the number of regions/nodes in the context. Weights
must remain valid through the call. OhHelp copies them into the context-owned
state.

## Default Weight

If no weights are supplied, every region has weight `1.0`. That reduces the v2
metric to particle-count balancing. The current migration code keeps this case
on the count-equivalent compatibility path; non-uniform positive weights select
the weighted-load rebalance path.

Weighted mode reuses the current secondary schedule when every region's
particles are still inside the existing primary/secondary family. That stable
check avoids rebuilding a weighted rebalance schedule on repeated `transbound`
calls with unchanged particle ownership. If weighted particles have crossed out
of the existing family, the migration path still falls back to weighted
rebalance instead of applying the count-based stable shortcut.

## Integer Donor Assignments and Heap Ordering

The count-equivalent compatibility path rounds per-process targets to the
floor or ceiling of the global average. An old parent is eligible for reuse
only when its remaining primary particles can supply the receiver's entire
secondary assignment. Otherwise the scheduler selects the largest remaining
donor. A floor-average donor must not be overdrawn to satisfy a ceil-average
receiver; no particle counts are clipped or invented to repair a schedule.
Non-uniform weighted mode keeps its existing weighted transfer calculation
and its cap on the donor's available integer particles.

Both paths share indexed heaps. Removing an arbitrary old parent from a heap
must repair the replacement item upwards when it outranks its new parent,
or downwards otherwise. This preserves ordering for both particle-count keys
and weighted-load keys.

The standard test gate exercises the actual scheduler implementation with
arbitrary heap removals and checks primary/secondary particle conservation,
non-negative donor remainders, and floor/ceiling process totals across
deterministic count-based rebalance trees.
