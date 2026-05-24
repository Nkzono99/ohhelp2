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

Weights must remain valid through the call. OhHelp copies them into the
context-owned state.

## Default Weight

If no weights are supplied, every region has weight `1.0`. That reduces the v2
metric to particle-count balancing while still using the same weighted-load
path.
