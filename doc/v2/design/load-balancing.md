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
oh_context_set_region_weights(ctx, weights);
```

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

Weighted mode deliberately does not use the old stable secondary shortcut yet.
Until a weighted stable check exists, repeated weighted transfers rebalance
again instead of reusing a count-based stable schedule.
