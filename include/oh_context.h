/* File: oh_context.h
   v2 context-oriented API draft.
*/
#ifndef OH_CONTEXT_H
#define OH_CONTEXT_H

#include <mpi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct oh_state oh_context;
typedef struct oh_particle_adapter oh_particle_adapter;

oh_context *oh_default_context(void);
/* Passing NULL resets all region weights to 1.0 on the default context. */
void oh_context_set_region_weights(oh_context *context, const double *weights);
/* Passing MPI_DATATYPE_NULL resets to the default byte datatype. */
void oh_context_set_particle_mpi_type(oh_context *context, MPI_Datatype type);
/* Passing NULL resets particle movement to the default S_particle adapter. */
void oh_context_set_particle_adapter(oh_context *context,
                                     const oh_particle_adapter *adapter);

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
