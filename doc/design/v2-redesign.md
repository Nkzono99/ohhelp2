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
- Level-1 rebalance heap ordering now receives explicit particle/load key arrays
  from `oh_state` instead of reading the balancing mode from globals.
- Stable-secondary stay counting now takes `oh_state` and uses context-owned
  rank, communicator, particle histograms, node descriptors, and scratch space.
- Stable-secondary particle assignment now receives `oh_state`, including the
  local particle limit, node queue, and scratch buffer, instead of reaching into
  the legacy globals directly.
- Particle-exchange schedule construction now receives `oh_state` for rank,
  communicator, histograms, node descriptors, neighbor lists, and the schedule
  buffer. The remaining global schedule counters are a migration target.
- Communication-count construction now receives `oh_state` for receive/send
  histograms, local particle histograms, `TotalPNext`, and the schedule buffer;
  the head/tail counters remain as a smaller follow-up migration.
- Rebalance tree construction now takes node arrays and the node queue from
  `oh_state`; communicator rebuilding is the next large Level-1 global user.
- Communicator rebuilding now has a state-backed internal path for rank,
  communicator, node arrays, node queue, scratch buffer, and region id updates.
  The communicator container and user-facing `MyComm` mirrors still need to move
  into context-owned storage.
- The communicator container now has a named type and is reachable from
  `oh_state`, along with `MyComm` C/Fortran mirrors. Rebuild writes through the
  context fields; collective wrappers still need to read from the context.
- Level-1 broadcast/all-reduce/reduce wrappers now delegate to state-backed
  helpers that read communicator state from `oh_state`.
- Internal Level-1 paths that already have `oh_state` now call the state-backed
  broadcast helper directly instead of resynchronizing through the public wrapper.
- Level-1 schedule counters (`RLIndex`, `SLHeadTail`, `SecSLHeadTail`,
  `SecRLSize`) are now reachable through `oh_state` and used that way by the
  state-backed scheduling/count construction paths.
- Level-2 particle buffers, displacement arrays, request/status buffers,
  injection counters, particle MPI datatype, and particle adapter pointers are
  now mirrored into `oh_state`. Level-2 code still reads the globals directly;
  the mirror is the next migration bridge for threading state through movement
  and exchange helpers.
- Primary-mode Level-2 particle exchange now has a state-backed internal path
  for communicator, rank, particle histograms, neighbor arrays, particle MPI
  datatype, send/receive buffers, displacement arrays, and scratch counts.
- Secondary-mode Level-2 particle exchange now reads its communicator,
  request/status buffers, communication lists, count arrays, displacement
  arrays, node descriptors, particle MPI datatype, and particle buffers through
  `oh_state` after local movement has prepared the buffers.
- Level-2 send-buffer displacement construction now has a state-backed helper
  and uses context-owned particle histograms, injection counters, rank/node
  counts, and displacement storage.
- Level-2 injected-particle buffer movement now receives `oh_state` and uses
  context-owned particle buffers, injection counters, node/species counts, and
  send-buffer displacements.
- Level-2 local upward/downward particle movement now receives `oh_state` and
  uses context-owned particle buffers, send-buffer displacements, receive-buffer
  bases, and node/species counts for the core copy/routing loops.
- State-backed Level-2 movement paths now use the particle adapter pointer from
  `oh_state` for buffer element addressing and byte-wise copies, rather than
  relying on the process-global adapter in those loops.
- Level-2 particle region/species/subdomain helpers now have state-aware
  variants, and injected-particle inject/remap/remove operations use `oh_state`
  for adapter callbacks, particle counters, injection counters, region ids,
  buffer limits, and buffer indexing.
- Level-2 primary/secondary send-buffer movement now has state-backed entry
  points that use context-owned particle histograms, total particle arrays,
  injection counters, node descriptors, and primary/secondary base counters.
- Level-2 transbound finalization now uses `oh_state` to reset particle
  histograms, carry `TotalPNext` into `TotalP`, clear injection counters, and
  update total-particle/current-mode mirrors.
- Level-2 primary/stable/rebalance transition wrappers now have state-backed
  internal paths for rank/node lookups, secondary receive lists, schedule sizes,
  injection counters, and primary/secondary base updates.
- Level-2 initialization now separates particle-adapter selection, particle
  storage allocation, base-counter allocation, and communication work-buffer
  allocation, then synchronizes the default `oh_state` mirror explicitly.
- `oh_context_set_particle_adapter()` exposes the Level-2 particle adapter hook
  through the context facade. It still targets the default context only, matching
  the current migration stage.
- Level-3 geometry, field, boundary, and border-exchange globals are now
  mirrored into `oh_state`. Fields that depend on Level-3-only array constants
  are stored as opaque `int *`/struct pointers for now so `ohhelp1.h` does not
  need to include `ohhelp3.h`; later Level-3 helpers can cast them at the point
  of use while state ownership is moved behind the context.
- Level-3 particle-to-neighbor and particle-to-subdomain coordinate mapping now
  routes through state-backed internal helpers. The public C/Fortran-compatible
  wrappers still provide the old signatures, but the core mapping path reads
  region ids, neighbor tables, grid geometry, subdomains, and boundary flags
  through `oh_state`.
- Level-3 field broadcast/reduce/allreduce and border-exchange entry points now
  delegate to state-backed helpers. They still call the existing Level-1
  collective wrappers, but field descriptors, communicator, adjacency, current
  mode, region ids, and border-exchange descriptors are read through `oh_state`.
- Level-3 grid-size scaling now updates grid geometry, floating subdomain
  bounds, and irregular subdomain descriptors through a state-backed helper.
- Level-3 transbound now keeps its Level-1/2 delegation wrapper but reads
  `excludeLevel2`, region ids, field types, and subdomain descriptors through
  `oh_state` when deciding whether to rebuild secondary field descriptors.
- Level-3 secondary border-exchange clearing now reads exchange counts and
  descriptors through `oh_state`, including the transbound path that invalidates
  stale secondary datatypes after the secondary region changes.
- Level-3 field descriptor sizing and border datatype construction now have
  state-backed internal helpers. The legacy `set_field_descriptors()` and
  `set_border_exchange()` entry points remain as wrappers, while the core
  descriptor, boundary-communication, subdomain, and field-size reads flow
  through `oh_state`.
- Level-3 field initialization now synchronizes the default context before
  descriptor sizing and border datatype creation, then calls the state-backed
  descriptor/exchange helpers directly instead of routing through the legacy
  wrappers.
- Level-3 top-level initialization now synchronizes `oh_state` immediately after
  Level-1/2 initialization and uses it for node count, rank, and destination
  neighbor reads before field initialization.
- Level-3 active/passive subdomain initialization now receives `oh_state` and
  updates grid geometry, subdomain descriptors, adjacency checks, rank, and node
  counts through the context mirror. The passive path also fixes the grid maximum
  size assignment to use the current dimension index.
- Level-4 C implementations and Fortran modules have been restored into the
  active `src/c`, `include`, and `src/fortran` trees. Docker verification now
  compiles both Level-4p and Level-4s active sources against the current public
  headers so future migration work cannot silently break them.
- Level-4p/4s particle transfer now has state-backed internal exchange helpers
  for node/species counts, receive/send histograms, receive buffer bases,
  particle MPI datatype, communicator, requests, and statuses. Level-4 grid
  routing tables are still Level-4 globals and remain a follow-up migration
  target.
- Level-4s boundary particle exchange now uses `oh_state` for species count,
  particle/send buffers, particle MPI datatype, communicator, requests, and
  statuses. The boundary-plane descriptors remain Level-4s-owned globals.
- Level-4 grid histogram/routing state is now mirrored into `oh_state` by
  Level-4p/4s-local sync helpers. `src/c/oh_context.c` deliberately does not
  include Level-4 headers because the Level-4p and Level-4s headers carry
  overlapping implementation-private type and global names.
- Level-4p/4s particle transfer now reads real-neighbor routing tables through
  the Level-4 `oh_state` mirror, leaving the Level-4-specific casts local to
  the translation units that own those private types.
- Level-4p/4s transfer-count exchange and send-buffer displacement construction
  now have state-backed helpers for communicator, requests/statuses,
  receive/send histograms, node/species counts, and real-neighbor routing
  tables.
- Level-4s user border-data exchange now synchronizes `oh_state` at the public
  entry point and passes it through the vertical/horizontal exchange helpers for
  communicator, request/status buffers, species count, and active region reads.
- Level-4p/4s receive-list exchange now synchronizes `oh_state` before schedule
  construction and uses the context mirror for rank/node/species counts,
  communicator, communication-list storage, node-tree pointers, and Level-4
  first-neighbor routing.

## Particle Layout

The fixed `struct S_particle` / `type oh_particle` model should be replaced by a
particle adapter. The adapter should describe how OhHelp moves particles and how
it reads or writes the fields needed by the selected library level.

Minimum C-side shape:

```c
struct oh_particle_ops {
  size_t stride;
  MPI_Datatype mpi_type;
  int (*get_region)(const void *particle, int primary_or_secondary);
  void (*set_region)(void *particle, int region, int primary_or_secondary);
  int (*get_species)(const void *particle);
  int (*map_to_neighbor)(void *particle, int primary_or_secondary);
  int (*map_to_subdomain)(void *particle, int primary_or_secondary);
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

Level 2 now also has `oh2_set_particle_adapter()` / `oh_set_particle_adapter()`.
The first implementation stores the adapter and uses its MPI datatype during
initialization.

The Level-2 injection/remap/remove path now reads region/species and marks a
removed particle through the active adapter.

Level-2 bulk send-buffer construction now obtains particle destination regions
through the active adapter. The default adapter preserves the old `nid`-based
mapping, including the POS_AWARE primarization side effect for injected
particles.

Level-2 local/send/receive particle copies now go through small copy helpers
rather than open-coded `S_particle` assignments. Storage is still
`struct S_particle`-backed, so this is a staging step toward stride-based
adapter storage rather than the final opaque-particle implementation.

Level-2 buffer allocation and internal element addressing now use
`oh_particle_adapter.stride`. The public buffer type still names
`struct S_particle`, but internal pointer arithmetic no longer assumes
`sizeof(struct S_particle)`. Adapter validation also requires the MPI datatype
extent to match the stride, because `MPI_Alltoallv` displacements are expressed
in datatype units.

The stride-aware storage helpers live in `src/c/oh_particle_buffer.h` so
Level-2 movement code can share one implementation for element addressing,
index validation, and byte-wise copies.

Level-3 initialization now installs default `S_particle` coordinate mapping
callbacks into the active default adapter after geometry setup. Custom adapters
remain fully application-owned; if an application provides its own particle
layout, it should also provide `map_to_neighbor` / `map_to_subdomain` callbacks
when Level-3 particle remapping needs coordinate-aware behavior.

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

The arithmetic for this conversion lives in `oh_load_balance` so the
count-to-load policy is testable outside the MPI scheduling code:

- `oh_weighted_transfer_count()` converts target/receiver load into an integer
  particle transfer count.
- `oh_load_after_transfer()` updates the donor load using the same weight
  convention and clamps numerical underflow at zero.

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
