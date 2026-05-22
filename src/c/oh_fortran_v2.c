/* File: oh_fortran_v2.c
   Fortran ISO_C_BINDING facade for v2 context and particle adapters.
*/
#include <stdlib.h>

#include <mpi.h>

#include "oh_fortran_v2.h"
#include "ohhelp1_internal.h"
#include "ohhelp2_internal.h"
#include "ohhelp3.h"
#include "ohhelp3_internal.h"

struct oh_fortran_particle_adapter {
  oh_particle_adapter adapter;
  int owns_mpi_type;
};

static MPI_Datatype
fortran_type_or_null(int fortran_type) {
  if (fortran_type == 0) return MPI_DATATYPE_NULL;
  return MPI_Type_f2c(fortran_type);
}

static oh_particle_adapter *
unwrap_adapter(oh_fortran_particle_adapter *adapter) {
  return adapter ? &adapter->adapter : NULL;
}

static const oh_particle_adapter *
unwrap_const_adapter(const oh_fortran_particle_adapter *adapter) {
  return adapter ? &adapter->adapter : NULL;
}

oh_context *
oh_fortran_default_context(void) {
  return oh_default_context();
}

void
oh_fortran_context_set_region_weights(oh_context *context,
                                      const double *weights) {
  oh_context_set_region_weights(context, weights);
}

void
oh_fortran_context_set_particle_mpi_type(oh_context *context,
                                         int fortran_type) {
  oh_context_set_particle_mpi_type(context, fortran_type_or_null(fortran_type));
}

void
oh_fortran_context_set_particle_adapter(
  oh_context *context, const oh_fortran_particle_adapter *adapter) {
  oh_context_set_particle_adapter(context, unwrap_const_adapter(adapter));
}

int
oh_fortran_context_transbound1(oh_context *context, int currmode, int stats) {
  return oh_context_transbound1(context, currmode, stats);
}

int
oh_fortran_context_transbound2(oh_context *context, int currmode, int stats) {
  return oh_context_transbound2(context, currmode, stats);
}

int
oh_fortran_context_transbound3(oh_context *context, int currmode, int stats) {
  return oh_context_transbound3(context, currmode, stats);
}

void
oh_fortran_context_broadcast(oh_context *context, void *pbuf, void *sbuf,
                             int pcount, int scount, int ptype, int stype) {
  oh_context_broadcast(context, pbuf, sbuf, pcount, scount,
                       MPI_Type_f2c(ptype), MPI_Type_f2c(stype));
}

void
oh_fortran_context_all_reduce(oh_context *context, void *pbuf, void *sbuf,
                              int pcount, int scount, int ptype, int stype,
                              int pop, int sop) {
  oh_context_all_reduce(context, pbuf, sbuf, pcount, scount,
                        MPI_Type_f2c(ptype), MPI_Type_f2c(stype),
                        MPI_Op_f2c(pop), MPI_Op_f2c(sop));
}

void
oh_fortran_context_reduce(oh_context *context, void *pbuf, void *sbuf,
                          int pcount, int scount, int ptype, int stype,
                          int pop, int sop) {
  oh_context_reduce(context, pbuf, sbuf, pcount, scount,
                    MPI_Type_f2c(ptype), MPI_Type_f2c(stype),
                    MPI_Op_f2c(pop), MPI_Op_f2c(sop));
}

void
oh_fortran_context_set_total_particles(oh_context *context) {
  oh_context_set_total_particles(context);
}

void
oh_fortran_context_inject_particle(oh_context *context, void *part) {
  oh_context_inject_particle(context, part);
}

void *
oh_fortran_context_inject_particle_get(oh_context *context, void *part) {
  return oh_context_inject_particle_get(context, part);
}

void
oh_fortran_context_remap_injected_particle(oh_context *context, void *part) {
  oh_context_remap_injected_particle(context, part);
}

void
oh_fortran_context_remove_injected_particle(oh_context *context, void *part) {
  oh_context_remove_injected_particle(context, part);
}

void
oh_fortran_context_grid_size(oh_context *context, double *size) {
  oh_context_grid_size(context, size);
}

int
oh_fortran_context_map_particle_to_neighbor(oh_context *context, double *x,
                                            double *y, double *z, int ps) {
  return oh_context_map_particle_to_neighbor(context, x, y, z, ps);
}

int
oh_fortran_context_map_particle_to_subdomain(oh_context *context, double x,
                                             double y, double z) {
  return oh_context_map_particle_to_subdomain(context, x, y, z);
}

void
oh_fortran_context_bcast_field(oh_context *context, void *pfld, void *sfld,
                               int ftype) {
  oh_context_bcast_field(context, pfld, sfld, ftype);
}

void
oh_fortran_context_reduce_field(oh_context *context, void *pfld, void *sfld,
                                int ftype) {
  oh_context_reduce_field(context, pfld, sfld, ftype);
}

void
oh_fortran_context_allreduce_field(oh_context *context, void *pfld,
                                   void *sfld, int ftype) {
  oh_context_allreduce_field(context, pfld, sfld, ftype);
}

void
oh_fortran_context_exchange_borders(oh_context *context, void *pfld,
                                    void *sfld, int ctype, int bcast) {
  oh_context_exchange_borders(context, pfld, sfld, ctype, bcast);
}

void
oh_fortran_oh2_init_raw(
  int *sdid, int nspec, int maxfrac, int *nphgram, int *totalp, void **pbuf,
  int *pbase, int maxlocalp, struct S_mycommf *mycomm, int *nbor,
  int *pcoord, int stats, int repiter, int verbose) {
  int *sdid_ptr = sdid;
  int *nphgram_ptr = nphgram;
  int *totalp_ptr = totalp;
  struct S_particle *particle_ptr = pbuf ? (struct S_particle*)*pbuf : NULL;
  int *pbase_ptr = pbase;
  int *nbor_ptr = nbor;

  if (!pbuf) local_errstop("oh2_init_raw requires a particle pointer slot");
  specBase = 1;
  init2(&sdid_ptr, nspec, maxfrac, &nphgram_ptr, &totalp_ptr, &particle_ptr,
        &pbase_ptr, maxlocalp, NULL, mycomm, &nbor_ptr, pcoord, stats,
        repiter, verbose);
  *pbuf = particle_ptr;
}

void
oh_fortran_oh3_init_raw(
  int *sdid, int nspec, int maxfrac, int *nphgram, int *totalp, void **pbuf,
  int *pbase, int maxlocalp, struct S_mycommf *mycomm, int *nbor,
  int *pcoord, int *sdoms, int *scoord, int nbound, int *bcond,
  int *bounds, int *ftypes, int *cfields, int *ctypes, int *fsizes,
  int stats, int repiter, int verbose) {
  int *sdid_ptr = sdid;
  int *nphgram_ptr = nphgram;
  int *totalp_ptr = totalp;
  struct S_particle *particle_ptr = pbuf ? (struct S_particle*)*pbuf : NULL;
  int *pbase_ptr = pbase;
  int *nbor_ptr = nbor;
  int *sdoms_ptr = sdoms;
  int *bounds_ptr = bounds;
  int *fsizes_ptr = fsizes;

  if (!pbuf) local_errstop("oh3_init_raw requires a particle pointer slot");
  specBase = 1;
  init3(&sdid_ptr, nspec, maxfrac, &nphgram_ptr, &totalp_ptr, NULL, NULL,
        &particle_ptr, &pbase_ptr, maxlocalp, NULL, mycomm, &nbor_ptr,
        pcoord, &sdoms_ptr, scoord, nbound, bcond, &bounds_ptr, ftypes,
        cfields, -1, ctypes, &fsizes_ptr, stats, repiter, verbose, 0);
  *pbuf = particle_ptr;
}

int
oh_fortran_particle_adapter_create_byte(
  size_t stride, oh_fortran_particle_adapter **adapter) {
  MPI_Datatype type = MPI_DATATYPE_NULL;
  oh_fortran_particle_adapter *created;
  int err;

  if (!adapter) return MPI_ERR_ARG;
  *adapter = NULL;

  err = oh_particle_adapter_make_byte_type(stride, &type);
  if (err != MPI_SUCCESS) return err;

  created = (oh_fortran_particle_adapter*)calloc(1, sizeof(*created));
  if (!created) {
    MPI_Type_free(&type);
    return MPI_ERR_NO_MEM;
  }

  created->adapter = oh_default_particle_adapter(type);
  created->adapter.stride = stride;
  created->owns_mpi_type = 1;
  *adapter = created;
  return MPI_SUCCESS;
}

void
oh_fortran_particle_adapter_destroy(oh_fortran_particle_adapter *adapter) {
  int initialized = 0;

  if (!adapter) return;
  MPI_Initialized(&initialized);
  if (initialized && adapter->owns_mpi_type &&
      adapter->adapter.mpi_type != MPI_DATATYPE_NULL)
    MPI_Type_free(&adapter->adapter.mpi_type);
  free(adapter);
}

int
oh_fortran_particle_adapter_validate(
  const oh_fortran_particle_adapter *adapter) {
  return oh_particle_adapter_validate(unwrap_const_adapter(adapter));
}

void
oh_fortran_particle_adapter_set_mpi_type(oh_fortran_particle_adapter *adapter,
                                         int fortran_type) {
  if (!adapter) return;
  adapter->adapter.mpi_type = fortran_type_or_null(fortran_type);
  adapter->owns_mpi_type = 0;
}

void
oh_fortran_particle_adapter_use_int_fields(
  oh_fortran_particle_adapter *adapter, size_t region_offset,
  size_t species_offset) {
  oh_particle_adapter_use_int_fields(unwrap_adapter(adapter), region_offset,
                                     species_offset);
}

void
oh_fortran_particle_adapter_use_single_species_int_region(
  oh_fortran_particle_adapter *adapter, size_t region_offset) {
  oh_particle_adapter_use_single_species_int_region(unwrap_adapter(adapter),
                                                    region_offset);
}

void
oh_fortran_particle_adapter_use_integer_fields(
  oh_fortran_particle_adapter *adapter, size_t region_offset,
  size_t region_size, size_t species_offset, size_t species_size) {
  oh_particle_adapter_use_integer_fields(unwrap_adapter(adapter),
                                         region_offset, region_size,
                                         species_offset, species_size);
}

void
oh_fortran_particle_adapter_use_single_species_integer_region(
  oh_fortran_particle_adapter *adapter, size_t region_offset,
  size_t region_size) {
  oh_particle_adapter_use_single_species_integer_region(unwrap_adapter(adapter),
                                                        region_offset,
                                                        region_size);
}

void
oh_fortran_particle_adapter_use_position_fields(
  oh_fortran_particle_adapter *adapter, size_t x_offset, size_t y_offset,
  size_t z_offset) {
  oh_particle_adapter_use_position_fields(unwrap_adapter(adapter), x_offset,
                                          y_offset, z_offset);
}

void
oh_fortran_particle_adapter_use_level3_position_fields(
  oh_fortran_particle_adapter *adapter, size_t x_offset, size_t y_offset,
  size_t z_offset) {
  oh3_particle_adapter_use_position_fields(unwrap_adapter(adapter), x_offset,
                                           y_offset, z_offset);
}

void
oh_fortran_particle_adapter_set_callbacks(
  oh_fortran_particle_adapter *adapter, oh_particle_get_region_fn get_region,
  oh_particle_set_region_fn set_region, oh_particle_get_species_fn get_species,
  oh_particle_map_fn map_to_neighbor, oh_particle_map_fn map_to_subdomain) {
  oh_particle_adapter *unwrapped = unwrap_adapter(adapter);

  if (!unwrapped) return;
  unwrapped->get_region = get_region;
  unwrapped->set_region = set_region;
  unwrapped->get_species = get_species;
  unwrapped->map_to_neighbor = map_to_neighbor;
  unwrapped->map_to_subdomain = map_to_subdomain;
}

size_t
oh_fortran_particle_field_offset(const void *base, const void *field) {
  return (size_t)((const char*)field - (const char*)base);
}
