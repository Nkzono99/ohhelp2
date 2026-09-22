/* Context-local particle storage, with a default-context Level 4 bridge. */
#ifndef OH_PARTICLE_STORAGE_H
#define OH_PARTICLE_STORAGE_H

#include <stddef.h>
#include "oh_context.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OH_PARTICLE_STORAGE_AOS 0
#define OH_PARTICLE_STORAGE_SOA 1

#define OH_PARTICLE_FIELD_INT 1
#define OH_PARTICLE_FIELD_LONG_LONG 2
#define OH_PARTICLE_FIELD_DOUBLE 3
#define OH_PARTICLE_FIELD_BYTES 4

#define OH_PARTICLE_ROLE_PAYLOAD 0
#define OH_PARTICLE_ROLE_REGION 1
#define OH_PARTICLE_ROLE_SPECIES 2
#define OH_PARTICLE_ROLE_X 4
#define OH_PARTICLE_ROLE_Y 8
#define OH_PARTICLE_ROLE_Z 16

/* key is a nonnegative, unique application field id. size must equal the
   native scalar sizeof for numeric fields; bytes fields have any positive
   fixed item size. Every field is a contiguous array of capacity items.
   Levels 2/3 use absolute zero-based node ids; Level 4 uses packed grid ids. */
typedef struct oh_particle_field {
  int key;
  int type;
  int role;
  size_t size;
} oh_particle_field;

/* References identify injected copies, not persistent particle identities.
   Invalidated by unbind/rebind, transbound, or set_total_particles. */
typedef struct oh_particle_ref {
  oh_context *context;
  long long generation;
  int index;
} oh_particle_ref;

/* Configuration/binding calls are local and return MPI error codes. They
   require an unbound context. Selecting AoS preserves its adapter.
   For Level 4 select/configure the default context before oh4p/oh4s_init. */
int oh_context_set_particle_storage(oh_context *context, int backend);
int oh_context_get_particle_storage(const oh_context *context);
int oh_context_configure_soa(oh_context *context,
                            const oh_particle_field *fields, int nfields,
                            int species_base);
/* Borrowed: bases contains nfields pointers, matching the schema field count.
   Owned: bases is NULL, nfields is zero, and OhHelp allocates the arrays.
   Use unbind_particles for either.
   Schema and backend survive unbind. Empty borrowed storage permits NULL. */
int oh_context_bind_soa_particles(oh_context *context, void *const *bases,
                                 int nfields, int capacity, int ownership);
/* Returns a borrowed field view by key, or NULL if unbound/not found. */
void *oh_context_soa_field(oh_context *context, int key);

/* Level 3 mapping updates the region column; neighbor mapping also wraps
   periodic coordinates. Subdomain mapping rejects points outside the grid. As for
   coordinate-based Level 3 mapping, the caller maintains its histogram.
   Indices are zero based in both C and Fortran. Returns a node id, or -1. */
int oh_context_map_soa_particle_to_neighbor(oh_context *context, int index, int ps);
int oh_context_map_soa_particle_to_subdomain(oh_context *context, int index);

/* values contains pointers to one item per field, in schema order. All
   values are copied before writing the injected slot, permitting aliases
   into the bound arrays. The result references the injected copy. */
int oh_context_inject_soa_particle(oh_context *context,
                                  const void *const *values, int nfields,
                                  oh_particle_ref *result);
int oh_context_particle_ref_valid(const oh_context *context, oh_particle_ref ref);
/* Remap is additive: remove a counted copy before editing/remapping it. */
int oh_context_remap_injected_particle_ref(oh_context *context,
                                          oh_particle_ref ref);
int oh_context_remove_injected_particle_ref(oh_context *context,
                                           oh_particle_ref ref);

#ifdef __cplusplus
}
#endif
#endif
