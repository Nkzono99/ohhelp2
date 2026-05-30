/* File: oh_context.h
   v2 context-oriented API draft.
*/
#ifndef OH_CONTEXT_H
#define OH_CONTEXT_H

#include <mpi.h>

#include "oh_mode.h"
#include "oh_particle_ownership.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct oh_state oh_context;
typedef struct oh_particle_adapter oh_particle_adapter;

/* Creates a heap-owned context by duplicating comm. Must be called after
   MPI_Init and before MPI_Finalize. Returns MPI_ERR_ARG for a NULL context
   slot, MPI_ERR_COMM for MPI_COMM_NULL, MPI_ERR_OTHER outside the MPI
   lifetime, MPI_ERR_NO_MEM on allocation failure, or an MPI error from
   communicator/datatype creation. On failure, *context is set to NULL.
   oh_context_destroy() tolerates heap cleanup after MPI_Finalize(), but users
   should destroy contexts before MPI_Finalize() to release MPI-owned handles. */
int oh_context_create(MPI_Comm comm, oh_context **context);
void oh_context_destroy(oh_context *context);
oh_context *oh_default_context(void);
void oh_context_configure_particles(oh_context *context, int nspec,
                                    int maxfrac);
int oh_context_region_count(const oh_context *context);
int oh_context_is_level3_configured(const oh_context *context);
/* Passing NULL weights resets all weights to 1.0 on the selected context.
   Passing a NULL context selects the default context. */
void oh_context_set_region_weights(oh_context *context, const double *weights);
/* Passing MPI_DATATYPE_NULL resets to the default byte datatype. */
void oh_context_set_particle_mpi_type(oh_context *context, MPI_Datatype type);
/* Passing NULL resets particle movement to the default S_particle adapter. */
void oh_context_set_particle_adapter(oh_context *context,
                                     const oh_particle_adapter *adapter);
/* Heap contexts must be configured before binding particles.
   Borrowed storage requires a non-NULL buffer when maxlocalp > 0.
   Owned storage requires particles == NULL and returns the owned buffer. */
void *oh_context_bind_particles(oh_context *context, void *particles,
                                int maxlocalp, int ownership);
void oh_context_unbind_particles(oh_context *context);
/* Borrowed region ids require non-NULL sdid. Owned region ids require
   sdid == NULL and return the owned two-element array. */
int *oh_context_bind_region_ids(oh_context *context, int *sdid,
                                int ownership);
void oh_context_unbind_region_ids(oh_context *context);
void oh_context_get_region_ids(oh_context *context, int sdid[2]);
/* nphgram, totalp, and pbase are pointer slots; all three slot addresses must
   be non-NULL. Borrowed accounting requires non-NULL storage in the slots.
   Owned accounting requires NULL slots and writes the allocated arrays back. */
void oh_context_bind_particle_accounting(oh_context *context, int **nphgram,
                                         int **totalp, int **pbase,
                                         int ownership);
void oh_context_unbind_particle_accounting(oh_context *context);
int oh_context_max_local_particles_for_capacity(
  oh_context *context, long long global_particle_limit,
  int capacity_percent, int min_margin);
void oh_context_configure_level3(oh_context *context, const int *pcoord,
                                 const int *sdoms, const int *scoord,
                                 int nbound, const int *bcond,
                                 const int *bounds, const int *ftypes,
                                 const int *cfields, const int *ctypes,
                                 int *fsizes);

int oh_context_transbound1(oh_context *context, int currmode, int stats);
int oh_context_transbound2(oh_context *context, int currmode, int stats);
int oh_context_transbound3(oh_context *context, int currmode, int stats);

void oh_context_broadcast(oh_context *context, void *pbuf, void *sbuf,
                          int pcount, int scount, MPI_Datatype ptype,
                          MPI_Datatype stype);
void oh_context_all_reduce(oh_context *context, void *pbuf, void *sbuf,
                           int pcount, int scount, MPI_Datatype ptype,
                           MPI_Datatype stype, MPI_Op pop, MPI_Op sop);
void oh_context_reduce(oh_context *context, void *pbuf, void *sbuf,
                       int pcount, int scount, MPI_Datatype ptype,
                       MPI_Datatype stype, MPI_Op pop, MPI_Op sop);

void oh_context_set_total_particles(oh_context *context);
void oh_context_inject_particle(oh_context *context, void *part);
void *oh_context_inject_particle_get(oh_context *context, void *part);
void oh_context_remap_injected_particle(oh_context *context, void *part);
void oh_context_remove_injected_particle(oh_context *context, void *part);

void oh_context_grid_size(oh_context *context, double *size);
int oh_context_map_particle_to_neighbor(oh_context *context, double *x,
                                        double *y, double *z, int ps);
int oh_context_map_particle_to_subdomain(oh_context *context, double x,
                                         double y, double z);
void oh_context_bcast_field(oh_context *context, void *pfld, void *sfld,
                            int ftype);
void oh_context_reduce_field(oh_context *context, void *pfld, void *sfld,
                             int ftype);
void oh_context_allreduce_field(oh_context *context, void *pfld, void *sfld,
                                int ftype);
void oh_context_exchange_borders(oh_context *context, void *pfld, void *sfld,
                                 int ctype, int bcast);

#ifdef __cplusplus
}
#endif

#endif
