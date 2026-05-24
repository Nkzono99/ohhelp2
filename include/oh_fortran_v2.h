/* File: oh_fortran_v2.h
   Fortran ISO_C_BINDING facade for v2 context and particle adapters.
*/
#ifndef OH_FORTRAN_V2_H
#define OH_FORTRAN_V2_H

#include <stddef.h>

#include "oh_context.h"
#include "ohhelp1.h"
#include "oh_particle_adapter.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct oh_fortran_particle_adapter oh_fortran_particle_adapter;

oh_context *oh_fortran_default_context(void);
void oh_fortran_context_set_region_weights(oh_context *context,
                                           const double *weights);
void oh_fortran_context_set_particle_mpi_type(oh_context *context,
                                             int fortran_type);
void oh_fortran_context_set_particle_adapter(
  oh_context *context, const oh_fortran_particle_adapter *adapter);
void *oh_fortran_context_bind_particles(oh_context *context, void *particles,
                                        int maxlocalp, int ownership);
void oh_fortran_context_unbind_particles(oh_context *context);
void oh_fortran_context_bind_particle_accounting(
  oh_context *context, int **nphgram, int **totalp, int **pbase,
  int ownership);
void oh_fortran_context_unbind_particle_accounting(oh_context *context);
int oh_fortran_context_transbound1(oh_context *context, int currmode,
                                  int stats);
int oh_fortran_context_transbound2(oh_context *context, int currmode,
                                  int stats);
int oh_fortran_context_transbound3(oh_context *context, int currmode,
                                  int stats);
void oh_fortran_context_broadcast(oh_context *context, void *pbuf,
                                  void *sbuf, int pcount, int scount,
                                  int ptype, int stype);
void oh_fortran_context_all_reduce(oh_context *context, void *pbuf,
                                   void *sbuf, int pcount, int scount,
                                   int ptype, int stype, int pop, int sop);
void oh_fortran_context_reduce(oh_context *context, void *pbuf, void *sbuf,
                               int pcount, int scount, int ptype, int stype,
                               int pop, int sop);
void oh_fortran_context_set_total_particles(oh_context *context);
void oh_fortran_context_inject_particle(oh_context *context, void *part);
void *oh_fortran_context_inject_particle_get(oh_context *context, void *part);
void oh_fortran_context_remap_injected_particle(oh_context *context,
                                                void *part);
void oh_fortran_context_remove_injected_particle(oh_context *context,
                                                 void *part);
void oh_fortran_context_grid_size(oh_context *context, double *size);
int oh_fortran_context_map_particle_to_neighbor(oh_context *context,
                                                double *x, double *y,
                                                double *z, int ps);
int oh_fortran_context_map_particle_to_subdomain(oh_context *context,
                                                 double x, double y,
                                                 double z);
void oh_fortran_context_bcast_field(oh_context *context, void *pfld,
                                    void *sfld, int ftype);
void oh_fortran_context_reduce_field(oh_context *context, void *pfld,
                                     void *sfld, int ftype);
void oh_fortran_context_allreduce_field(oh_context *context, void *pfld,
                                        void *sfld, int ftype);
void oh_fortran_context_exchange_borders(oh_context *context, void *pfld,
                                         void *sfld, int ctype, int bcast);

void oh_fortran_oh2_init_raw(
  int *sdid, int nspec, int maxfrac, int *nphgram, int *totalp, void **pbuf,
  int *pbase, int maxlocalp, struct S_mycommf *mycomm, int *nbor,
  int *pcoord, int stats, int repiter, int verbose);
void oh_fortran_oh3_init_raw(
  int *sdid, int nspec, int maxfrac, int *nphgram, int *totalp, void **pbuf,
  int *pbase, int maxlocalp, struct S_mycommf *mycomm, int *nbor,
  int *pcoord, int *sdoms, int *scoord, int nbound, int *bcond,
  int *bounds, int *ftypes, int *cfields, int *ctypes, int *fsizes,
  int stats, int repiter, int verbose);

int oh_fortran_particle_adapter_create_byte(
  size_t stride, oh_fortran_particle_adapter **adapter);
void oh_fortran_particle_adapter_destroy(oh_fortran_particle_adapter *adapter);
int oh_fortran_particle_adapter_validate(
  const oh_fortran_particle_adapter *adapter);
void oh_fortran_particle_adapter_set_mpi_type(
  oh_fortran_particle_adapter *adapter, int fortran_type);
void oh_fortran_particle_adapter_use_int_fields(
  oh_fortran_particle_adapter *adapter, size_t region_offset,
  size_t species_offset);
void oh_fortran_particle_adapter_use_single_species_int_region(
  oh_fortran_particle_adapter *adapter, size_t region_offset);
void oh_fortran_particle_adapter_use_integer_fields(
  oh_fortran_particle_adapter *adapter, size_t region_offset,
  size_t region_size, size_t species_offset, size_t species_size);
void oh_fortran_particle_adapter_use_single_species_integer_region(
  oh_fortran_particle_adapter *adapter, size_t region_offset,
  size_t region_size);
void oh_fortran_particle_adapter_use_position_fields(
  oh_fortran_particle_adapter *adapter, size_t x_offset, size_t y_offset,
  size_t z_offset);
void oh_fortran_particle_adapter_use_level3_position_fields(
  oh_fortran_particle_adapter *adapter, size_t x_offset, size_t y_offset,
  size_t z_offset);
void oh_fortran_particle_adapter_set_callbacks(
  oh_fortran_particle_adapter *adapter, oh_particle_get_region_fn get_region,
  oh_particle_set_region_fn set_region, oh_particle_get_species_fn get_species,
  oh_particle_map_fn map_to_neighbor, oh_particle_map_fn map_to_subdomain);
size_t oh_fortran_particle_field_offset(const void *base, const void *field);

#ifdef __cplusplus
}
#endif

#endif
