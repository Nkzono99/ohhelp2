# OhHelp v2 Redesign Notes

## Goals

- Remove the v1 assumption that one process-global library instance owns all
  state.
- Let applications define their own particle layout.
- Balance by estimated computational load, not just particle count.

## State Ownership

The current code exposes most state through `EXTERN` declarations in
`include/ohhelp1.h`, `include/ohhelp2.h`, and `include/ohhelp3.h`. The v2 target
is an explicit context, for example:

```c
typedef struct oh_context oh_context;

int oh_context_create(MPI_Comm comm, const struct oh_config *config,
                      oh_context **ctx);
void oh_context_destroy(oh_context *ctx);
```

Existing globals can be migrated incrementally by first grouping them into an
internal `struct oh_state`, then threading a pointer through the current call
tree. The public `oh_*` functions can become thin wrappers around a default
context during migration.

The current code now has this first migration layer:

- `struct oh_state` mirrors the Level-1 global state.
- `oh1_state()` returns the synchronized default state.
- `include/oh_context.h` exposes an opaque `oh_context` alias and
  `oh_default_context()`.
- `oh_context_set_region_weights()` provides the first context-shaped setter.
- `src/c/oh_context.c` owns the default context facade, while Level-1 code calls
  the internal sync hook during the migration away from process globals.

## Particle Layout

The fixed `struct S_particle` / `type oh_particle` model should be replaced by a
particle adapter. The adapter should describe how OhHelp moves particles and how
it reads or writes the fields needed by the selected library level.

Minimum C-side shape:

```c
struct oh_particle_ops {
  size_t stride;
  MPI_Datatype mpi_type;
  int (*get_region)(const void *particle);
  void (*set_region)(void *particle, int region);
  int (*get_species)(const void *particle);
  int (*map_to_neighbor)(void *particle, int primary_or_secondary);
};
```

Level-1 does not need particle layout at all. Level-2 only needs region/species
and byte movement. Level-3/4 need coordinate accessors or mapping callbacks.

As an interim hook, `include/oh_part.h` now allows a user particle header through
`OH_PARTICLE_HEADER`. That still requires a compatible `struct S_particle` name
and fields used by the selected level, so it is not the final adapter design.

`include/oh_particle_adapter.h` defines the target adapter shape and
`oh_default_particle_adapter()` describes the current `S_particle` layout. Level
2 also has `oh2_set_particle_mpi_type()` / `oh_set_particle_mpi_type()` so a
caller can provide the MPI datatype used for particle movement instead of always
using a raw contiguous byte type.

## Weighted Balancing

For a region `r`, define:

```text
region_load[r] = particle_count[r] * region_weight[r]
```

The balancing target is:

```text
target_load = sum(region_load) / number_of_nodes
max_load = target_load * (100 + maxfrac) / 100
```

Transfer schedules still move integer particle counts. If a helper needs
`load_deficit` from parent region `p`, the requested particle count is:

```text
ceil(load_deficit / region_weight[p])
```

The first implementation applies weighted load to primary-mode eligibility and
secondary assignment rebuilding. The old stable-secondary check remains
count-based and is bypassed while custom weights are active, so weighted runs
rebuild the helper assignment instead of accepting a stale count-balanced tree.

## Verification

The current Docker verification path is:

```sh
docker run --rm -v "$PWD:/work" -w /work ubuntu:24.04 bash -lc \
  'apt-get update && apt-get install -y build-essential gfortran mpich libmpich-dev && scripts/docker-build-test.sh'
```

This compiles the C and Fortran library units, builds the v2 helper tests, and
runs the MPI-backed particle adapter test with one rank.
