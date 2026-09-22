# SoA Particles in Levels 2–4

Native SoA storage is available in C and Fortran for heap-owned Level 2/3
contexts and the Level 4p/4s default-context bridge. AoS remains the default,
including existing adapters and entry points. Both backends use the same
weighted scheduler and integer particle counts.

Register borrowed or owned host columns for the region, coordinates, species,
and every payload that must follow the particle. Local movement and sorting
operate on columns; no persistent AoS particle mirror is created. Custom
indexed mapping callbacks and separate primary/secondary region columns remain
future extensions.

Level 2/3 region columns contain absolute zero-based node ids or negative skip
markers. Level 3 provides explicit indexed coordinate mapping. Level 4 owns the
packed grid/neighbor encoding, secondary offset, and `-1`/`-2` markers: use its
mapping entry points to update both the region and histogram.

## C Binding

Include `oh_particle_storage.h` (also included by `ohhelp_c.h`). Configuration
and binding functions return MPI error codes; check every result. This example
assumes MPI is initialized and `rank` / `nranks` have been queried:

```c
#include <assert.h>
#include "oh_particle_storage.h"
#include "oh_particle_ownership.h"

/* Inside the application's initialization routine. */
oh_context *ctx = NULL;
long long region[32] = {0};
double x[32] = {0};
void *columns[] = {region, x};
oh_particle_field schema[] = {
  {0, OH_PARTICLE_FIELD_LONG_LONG, OH_PARTICLE_ROLE_REGION, sizeof(long long)},
  {1, OH_PARTICLE_FIELD_DOUBLE, OH_PARTICLE_ROLE_X, sizeof(double)}
};
int *hist = NULL, *total = NULL, *pbase = NULL;
assert(oh_context_create(MPI_COMM_WORLD, &ctx) == MPI_SUCCESS);
oh_context_configure_particles(ctx, 1, 20);
assert(oh_context_set_particle_storage(ctx, OH_PARTICLE_STORAGE_SOA) == MPI_SUCCESS);
assert(oh_context_configure_soa(ctx, schema, 2, 0) == MPI_SUCCESS);
assert(oh_context_bind_soa_particles(ctx, columns, 2, 32,
                                   OH_PARTICLES_BORROWED) == MPI_SUCCESS);
oh_context_bind_particle_accounting(ctx, &hist, &total, &pbase,
                                   OH_PARTICLES_OWNED);
region[0] = (rank + 1) % nranks;
x[0] = rank + 0.5;
hist[region[0]] = 1;
oh_context_set_total_particles(ctx);
int mode = oh_context_transbound2(ctx, OH_MODE_NORMAL_PRIMARY, 0);
/* x[0:pbase[2]] and region[0:pbase[2]] now hold the local particles.
   Continue using the returned mode and rebuild hist for the next exchange. */
oh_context_destroy(ctx);  /* Releases accounting and work buffers, not columns. */
```

A field key is a nonnegative application id, unique within the schema.
`oh_context_soa_field(ctx, key)` returns its bound array, not a single-particle
pointer. Register every field that must be copied, sorted, or transferred with
the particle, including application ids, velocity, charge, and weight.
Unregistered arrays will not follow particle movement.

| Field type | Required item size |
| --- | --- |
| `OH_PARTICLE_FIELD_INT` | `sizeof(int)` |
| `OH_PARTICLE_FIELD_LONG_LONG` | `sizeof(long long)` |
| `OH_PARTICLE_FIELD_DOUBLE` | `sizeof(double)` |
| `OH_PARTICLE_FIELD_BYTES` | Any positive fixed number of bytes |

Exactly one integer region field is required. An integer species field is
required for multiple species; omit it for a single species. Coordinates must
be `double` when given the X/Y/Z roles. Other fields use
`OH_PARTICLE_ROLE_PAYLOAD`. Species values are normalized by the `species_base`
passed to `oh_context_configure_soa`; this is independent of particle indices.
Raw process-local pointers and variable-length pointees are not serializable
payloads.

Every column must provide at least `capacity` contiguous items and columns must
not overlap. Array lengths cannot be inferred from C pointers. Borrowed arrays
and their addresses must remain valid until unbind. Capacity includes pending
injections and cannot grow while bound. With zero capacity, borrowed storage
may pass `NULL, 0` for the column pointer vector and its length.

For owned arrays, call `oh_context_bind_soa_particles(ctx, NULL, 0, capacity,
OH_PARTICLES_OWNED)` and obtain columns with `oh_context_soa_field`. Those views
expire at `oh_context_unbind_particles` or destroy. Unbind retains the backend
and schema; rebinding does not restore particle values or accounting. Change
backend/schema only while unbound. Selecting AoS preserves its previously
configured adapter.

## Injection and References

Initialize accounting with `oh_context_set_total_particles()` before the first
injection. Pass one pointer to each field value, in schema order:

```c
long long destination = rank;
double position = 0.5;
const void *values[] = {&destination, &position};
oh_particle_ref injected;
assert(oh_context_inject_soa_particle(ctx, values, 2, &injected) == MPI_SUCCESS);
/* The copy is at zero-based slot injected.index in every bound column. */
assert(oh_context_remove_injected_particle_ref(ctx, injected) == MPI_SUCCESS);
region[injected.index] = (rank + 1) % nranks;
assert(oh_context_remap_injected_particle_ref(ctx, injected) == MPI_SUCCESS);
```

Remap is additive: remove the old count before changing a counted copy and
remapping it. Changing a region to a negative value alone does not undo its
count. References expire on transbound, set-total/finalization, unbind, or
rebind; stale references and references from another context are rejected.
They must never be used after context destruction. Appending another injection
does not invalidate earlier references. Use a registered payload id if a
particle needs persistent identity across reordering.

Pointer-based AoS injection and binding APIs reject SoA contexts. They do not
create a temporary AoS representation for callers.

## Level 3 Geometry

Configure topology with `oh_context_configure_level3` and register a double
coordinate column for every active dimension. Configuration may precede or
follow storage binding. Use:

```c
int dst = oh_context_map_soa_particle_to_neighbor(ctx, index, ps);
/* Periodic coordinates and region[index] have been updated. */
if (dst >= 0) hist[(ps * nspecies + species) * nranks + dst]++;
int mode = oh_context_transbound3(ctx, previous_mode, 0);
```

`oh_context_map_soa_particle_to_subdomain(ctx,index)` supports arbitrary
subdomain lookup inside the global grid; it returns -1 outside it. Periodic
wrapping belongs to neighbor mapping. Mapping does not increment Level 3 histogram counts; the
application maintains them as for the coordinate-based API. Transfer uses the
stored region, so map after updating coordinates. Level 3 injection and remap
locate the copied particle by its coordinates. Remove its old contribution
before editing/remapping. Mesh and field communication APIs are unchanged.

## Level 4p and 4s

Level 4 retains its default-context lifecycle; there is no heap-context Level 4
API. Select/configure SoA on `oh_default_context()` **before** `oh4p_init` or
`oh4s_init`. Coordinate roles for all active dimensions are required. Use a
`long long` region column for grids requiring `OH_BIG_SPACE`; compile the whole
library consistently with the Level 4 variant and width flags.

1. Set the backend and schema on the default context. Species may be declared
   before init supplies `nspec`.
2. For 4p, call `oh4p_max_local_particles`, then `oh4p_init` with a NULL particle
   pointer slot. Init allocates owned columns unless columns were already bound
   with `oh_context_bind_soa_particles` and exactly that capacity.
3. For 4s, call `oh4s_init` to obtain capacity. Optionally bind borrowed columns,
   then call `oh4s_particle_buffer(capacity,&buffer)` with `buffer=NULL`.
4. Obtain arrays with `oh_context_soa_field`. The raw AoS pointer slot stays
   NULL. Allocate the per-grid histogram through the existing Level 4 API.
5. Map each live particle, then call the existing `oh4p_transbound` or
   `oh4s_transbound`. Bound column addresses stay stable across sorting.

The indexed mapping functions take `(index, ps, species)`, all zero-based,
including normalized species independent of the schema's stored species base:

- `oh4p_map_soa_particle_to_neighbor` / `oh4s_map_soa_particle_to_neighbor`
- `oh4p_map_soa_particle_to_subdomain` / `oh4s_map_soa_particle_to_subdomain`
- `oh4p_remove_mapped_soa_particle` / `oh4s_remove_mapped_soa_particle`
- `oh4p_remap_soa_particle_to_neighbor` / `oh4s_remap_soa_particle_to_neighbor`
- `oh4p_remap_soa_particle_to_subdomain` / `oh4s_remap_soa_particle_to_subdomain`

Level 4 mapping increments its histogram; its **remap removes the old count
first**, following the existing Level 4 contract. Never map a counted particle
again without removing or remapping it. Skip boundary copies marked `-2` in the
physical particle loop. Arbitrary border data still uses
`oh4s_exchange_border_data` with arrays indexed in the current particle order.

`oh4p_inject_soa_particle(values,nfields,ps,&ref,&destination)` and its 4s
counterpart copy one item per field, map the copy, and return an MPI status.
The destination is a node id, or `-1` when the particle leaves an absorbing
boundary (the reference is then invalid). Before using a saved injection
reference's index, check `oh_context_particle_ref_valid(ctx,ref)`; transbound,
set-total, and unbind invalidate references. Plain indices always refer to the
current ordering. Legacy particle-pointer mapping/injection APIs reject SoA.

The tests [`test_oh_soa_level4.c`](../../../tests/test_oh_soa_level4.c) and
[`test_oh_soa_level34_fortran.F90`](../../../tests/test_oh_soa_level34_fortran.F90)
show initialization, mapping, injection, and transfer.

## Fortran Binding

Use `ohhelp_v2` with `iso_c_binding`. The module exposes the same constants,
`type(oh_particle_field)`, and `type(oh_particle_ref)`. Fortran schema helpers
use species base 1 unless an explicit `species_base` argument is supplied.

```fortran
integer(c_long_long), target :: region(32)
real(c_double), target :: x(32)
integer(c_long_long), pointer :: region_view(:)
real(c_double), pointer :: x_view(:)
type(c_ptr) :: columns(2)
type(oh_particle_field) :: schema(2)
! ctx is an initialized/configured oh_context_handle; ierr is integer(c_int).
schema(1) = oh_particle_field(0, OH_PARTICLE_FIELD_LONG_LONG, &
                             OH_PARTICLE_ROLE_REGION, c_sizeof(region(1)))
schema(2) = oh_particle_field(1, OH_PARTICLE_FIELD_DOUBLE, &
                             OH_PARTICLE_ROLE_X, c_sizeof(x(1)))
call oh_context_set_particle_storage(ctx, OH_PARTICLE_STORAGE_SOA, ierr)
! Check ierr after each call.
call oh_context_configure_soa(ctx, schema, ierr)
region_view => region
x_view => x
columns = [oh_soa_column(region_view), oh_soa_column(x_view)]
call oh_context_bind_soa_particles(ctx, 32_c_int, OH_PARTICLES_BORROWED, &
                                   ierr, columns)
```

`oh_soa_column` accepts associated array pointers of `integer(c_int)`,
`integer(c_long_long)`, or `real(c_double)`. Associate the view explicitly,
including for a slice (`x_view => x(::2)`). It returns `c_null_ptr` for
non-contiguous views,
which binding rejects for positive capacity. Supply actual persistent `target`
arrays, not expressions or temporary copies. If constructing pointers manually
with `c_loc`, the caller must ensure contiguity and lifetime. Contiguous columns
of `position(capacity,3)` are suitable; sections of `position(3,capacity)` are
not contiguous per coordinate.

For owned storage, omit `columns`. Access a field with
`c_f_pointer(oh_context_soa_field(ctx,key), array_pointer, [capacity])`; do not
deallocate that pointer. Context indices, including `ref%index`, are zero-based
in Fortran too: slot `i` corresponds to array element `lbound(array,1)+i`.

For Level 3 the module exposes `oh_context_map_soa_particle_to_neighbor` and
`oh_context_map_soa_particle_to_subdomain`. Level 4 indexed functions have the
same names as C. The `oh4p_init_raw` / `oh4s_init_raw`,
`oh4p_max_local_particles_raw`, `oh4s_particle_buffer_raw`, per-grid histogram
`_raw` procedures, and `oh4p_transbound_raw` / `oh4s_transbound_raw` interfaces
use C-compatible descriptors and pointer slots. They do not perform the legacy
Fortran one-based descriptor conversion. Coordinate bounds and normalized
species arguments follow the C contract; array element `i+1` represents index
`i`. `oh_context_particle_ref_valid` returns a Fortran logical.

See the executable examples in
[`test_oh_soa_runtime.c`](../../../tests/test_oh_soa_runtime.c) and
[`test_oh_soa_fortran.F90`](../../../tests/test_oh_soa_fortran.F90).

## MPI Contract and Current Costs

Ranks within a context must select the same backend and SoA schema (including
field order, keys, types, roles, sizes, and species base). Capacities may differ.
Agreement is checked collectively before movement; descriptor comparisons are
cached until any rank reconfigures. Each particle transfer currently adds one
small collective backend check, including for AoS. A performance benchmark has
not yet established its cost.

Level 2/3 SoA wire records concatenate field bytes without struct padding. A contiguous
MPI byte datatype represents one record, so schedule counts/displacements stay
in particle units. This requires matching native scalar representation and
endianness across ranks. Allocation/address products and receive capacities
are checked; this is not a heterogeneous-machine serialization format.

Level 2/3 reserve send and receive staging up to the configured capacity. Injection
finalization additionally uses an index list and one field-sized scratch array
for gathering, rather than an AoS mirror. Local configuration errors return MPI
error codes; collective schema/capacity failures abort with a diagnostic,
following the runtime's fail-fast model.

Level 4 uses column banks for bound particles, sorting scratch, and (4s) border
send scratch. Internal legacy cursors hold only bank/index metadata. Transfers
use committed MPI datatypes selecting the participating contiguous slices of
each column; cursor bytes never enter MPI. Each transfer temporarily builds a
datatype; MPI retains it until its request completes. Sorting copies the result
back to the bound columns to preserve application views. These operations have
not been benchmarked.
