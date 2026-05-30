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
#include "oh_context_internal.h"

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

static int *
copy_boundary_ids_zero_based(const int *values, int count, const char *name) {
  int *copy;
  int i;

  if (!values) return NULL;
  copy = (int*)malloc(sizeof(int) * count);
  if (!copy) local_errstop("out of memory for %s", name);
  for (i = 0; i < count; i++) copy[i] = values[i] - 1;
  return copy;
}

static int
legacy_sdoms_requests_active_decomposition(const int *sdoms) {
  return sdoms && sdoms[OH_LOWER] > sdoms[OH_UPPER];
}

static oh_context *
require_fortran_context(oh_context *context, const char *api) {
  if (!context) local_errstop("%s requires an associated context handle", api);
  return context;
}

oh_context *
oh_fortran_default_context(void) {
  return oh_default_context();
}

int
oh_fortran_context_create(int fortran_comm, oh_context **context) {
  int mpi_initialized = 0;
  int mpi_finalized = 0;

  if (!context) return MPI_ERR_ARG;
  *context = NULL;
  MPI_Initialized(&mpi_initialized);
  if (mpi_initialized) MPI_Finalized(&mpi_finalized);
  if (!mpi_initialized || mpi_finalized) return MPI_ERR_OTHER;
  return oh_context_create(MPI_Comm_f2c(fortran_comm), context);
}

void
oh_fortran_context_destroy(oh_context *context) {
  oh_context_destroy(context);
}

void
oh_fortran_context_configure_particles(oh_context *context, int nspec,
                                       int maxfrac) {
  context = require_fortran_context(context, "oh_context_configure_particles()");
  oh_context_configure_particles(context, nspec, maxfrac);
}

void
oh_fortran_context_set_region_weights(oh_context *context,
                                      const double *weights,
                                      int weight_count) {
  context = require_fortran_context(context, "oh_context_set_region_weights()");
  if (weights && weight_count != context->n_of_nodes)
    local_errstop("oh_context_set_region_weights() requires %d weights for this context, got %d",
                  context->n_of_nodes, weight_count);
  if (!weights && weight_count >= 0)
    local_errstop("oh_context_set_region_weights() requires %d weights for this context, got %d",
                  context->n_of_nodes, weight_count);
  oh_context_set_region_weights(context, weights);
}

void
oh_fortran_context_set_particle_mpi_type(oh_context *context,
                                         int fortran_type) {
  context = require_fortran_context(context,
                                    "oh_context_set_particle_mpi_type()");
  oh_context_set_particle_mpi_type(context, fortran_type_or_null(fortran_type));
}

void
oh_fortran_context_set_particle_adapter(
  oh_context *context, const oh_fortran_particle_adapter *adapter) {
  context = require_fortran_context(context,
                                    "oh_context_set_particle_adapter()");
  oh_context_set_particle_adapter(context, unwrap_const_adapter(adapter));
}

void *
oh_fortran_context_bind_particles(oh_context *context, void *particles,
                                  int maxlocalp, int ownership) {
  context = require_fortran_context(context, "oh_context_bind_particles()");
  return oh_context_bind_particles(context, particles, maxlocalp, ownership);
}

void
oh_fortran_context_unbind_particles(oh_context *context) {
  context = require_fortran_context(context, "oh_context_unbind_particles()");
  oh_context_unbind_particles(context);
}

int *
oh_fortran_context_bind_region_ids(oh_context *context, int *sdid,
                                   int ownership) {
  context = require_fortran_context(context, "oh_context_bind_region_ids()");
  return oh_context_bind_region_ids(context, sdid, ownership);
}

void
oh_fortran_context_unbind_region_ids(oh_context *context) {
  context = require_fortran_context(context, "oh_context_unbind_region_ids()");
  oh_context_unbind_region_ids(context);
}

void
oh_fortran_context_get_region_ids(oh_context *context, int *sdid) {
  context = require_fortran_context(context, "oh_context_get_region_ids()");
  oh_context_get_region_ids(context, sdid);
}

void
oh_fortran_context_bind_particle_accounting(
  oh_context *context, int **nphgram, int **totalp, int **pbase,
  int ownership) {
  context = require_fortran_context(
    context, "oh_context_bind_particle_accounting()");
  oh_context_bind_particle_accounting(context, nphgram, totalp, pbase,
                                      ownership);
}

void
oh_fortran_context_unbind_particle_accounting(oh_context *context) {
  context = require_fortran_context(
    context, "oh_context_unbind_particle_accounting()");
  oh_context_unbind_particle_accounting(context);
}

int
oh_fortran_context_max_local_particles_for_capacity(
    oh_context *context, long long global_particle_limit,
    int capacity_percent, int min_margin) {
  context = require_fortran_context(
    context, "oh_context_max_local_particles_for_capacity()");
  return oh_context_max_local_particles_for_capacity(
      context, global_particle_limit, capacity_percent, min_margin);
}

void
oh_fortran_context_configure_level3(
  oh_context *context, const int *pcoord, const int *sdoms,
  const int *scoord, int nbound, const int *bcond, const int *bounds,
  const int *ftypes, const int *cfields, const int *ctypes, int *fsizes) {
  context = require_fortran_context(context, "oh_context_configure_level3()");
  oh_context_configure_level3(context, pcoord, sdoms, scoord, nbound, bcond,
                              bounds, ftypes, cfields, ctypes, fsizes);
}

void
oh_fortran_context_configure_level3_legacy(
  oh_context *context, const int *pcoord, const int *sdoms,
  const int *scoord, int nbound, const int *bcond, const int *bounds,
  const int *ftypes, const int *cfields, const int *ctypes, int *fsizes) {
  int active = legacy_sdoms_requests_active_decomposition(sdoms);
  int nn;
  int *bcond_zero = copy_boundary_ids_zero_based(
      bcond, OH_DIMENSION * 2, "Level 3 boundary conditions");
  int *bounds_zero = NULL;

  context = require_fortran_context(context,
                                    "oh_context_configure_level3_legacy()");
  nn = context->n_of_nodes;
  if (!active && bounds) {
    if (nn <= 0)
      local_errstop("legacy Level 3 helper requires a configured context");
    bounds_zero = copy_boundary_ids_zero_based(
        bounds, nn * OH_DIMENSION * 2, "Level 3 boundary ids");
  }

  oh_context_configure_level3(context, pcoord, active ? NULL : sdoms, scoord,
                              nbound, bcond_zero, bounds_zero, ftypes,
                              cfields, ctypes, fsizes);
  free(bounds_zero);
  free(bcond_zero);
}

int
oh_fortran_context_transbound1(oh_context *context, int currmode, int stats) {
  context = require_fortran_context(context, "oh_context_transbound1()");
  return oh_context_transbound1(context, currmode, stats);
}

int
oh_fortran_context_transbound2(oh_context *context, int currmode, int stats) {
  context = require_fortran_context(context, "oh_context_transbound2()");
  return oh_context_transbound2(context, currmode, stats);
}

int
oh_fortran_context_transbound3(oh_context *context, int currmode, int stats) {
  context = require_fortran_context(context, "oh_context_transbound3()");
  return oh_context_transbound3(context, currmode, stats);
}

void
oh_fortran_context_broadcast(oh_context *context, void *pbuf, void *sbuf,
                             int pcount, int scount, int ptype, int stype) {
  context = require_fortran_context(context, "oh_context_broadcast()");
  oh_context_broadcast(context, pbuf, sbuf, pcount, scount,
                       MPI_Type_f2c(ptype), MPI_Type_f2c(stype));
}

void
oh_fortran_context_all_reduce(oh_context *context, void *pbuf, void *sbuf,
                              int pcount, int scount, int ptype, int stype,
                              int pop, int sop) {
  context = require_fortran_context(context, "oh_context_all_reduce()");
  oh_context_all_reduce(context, pbuf, sbuf, pcount, scount,
                        MPI_Type_f2c(ptype), MPI_Type_f2c(stype),
                        MPI_Op_f2c(pop), MPI_Op_f2c(sop));
}

void
oh_fortran_context_reduce(oh_context *context, void *pbuf, void *sbuf,
                          int pcount, int scount, int ptype, int stype,
                          int pop, int sop) {
  context = require_fortran_context(context, "oh_context_reduce()");
  oh_context_reduce(context, pbuf, sbuf, pcount, scount,
                    MPI_Type_f2c(ptype), MPI_Type_f2c(stype),
                    MPI_Op_f2c(pop), MPI_Op_f2c(sop));
}

void
oh_fortran_context_set_total_particles(oh_context *context) {
  context = require_fortran_context(context, "oh_context_set_total_particles()");
  oh_context_set_total_particles(context);
}

void
oh_fortran_context_inject_particle(oh_context *context, void *part) {
  context = require_fortran_context(context, "oh_context_inject_particle()");
  oh_context_inject_particle(context, part);
}

void *
oh_fortran_context_inject_particle_get(oh_context *context, void *part) {
  context = require_fortran_context(context,
                                    "oh_context_inject_particle_get()");
  return oh_context_inject_particle_get(context, part);
}

void
oh_fortran_context_remap_injected_particle(oh_context *context, void *part) {
  context = require_fortran_context(context,
                                    "oh_context_remap_injected_particle()");
  oh_context_remap_injected_particle(context, part);
}

void
oh_fortran_context_remove_injected_particle(oh_context *context, void *part) {
  context = require_fortran_context(context,
                                    "oh_context_remove_injected_particle()");
  oh_context_remove_injected_particle(context, part);
}

void
oh_fortran_context_grid_size(oh_context *context, double *size) {
  context = require_fortran_context(context, "oh_context_grid_size()");
  oh_context_grid_size(context, size);
}

int
oh_fortran_context_map_particle_to_neighbor(oh_context *context, double *x,
                                            double *y, double *z, int ps) {
  context = require_fortran_context(context,
                                    "oh_context_map_particle_to_neighbor()");
  return oh_context_map_particle_to_neighbor(context, x, y, z, ps);
}

int
oh_fortran_context_map_particle_to_subdomain(oh_context *context, double x,
                                             double y, double z) {
  context = require_fortran_context(context,
                                    "oh_context_map_particle_to_subdomain()");
  return oh_context_map_particle_to_subdomain(context, x, y, z);
}

void
oh_fortran_context_bcast_field(oh_context *context, void *pfld, void *sfld,
                               int ftype) {
  context = require_fortran_context(context, "oh_context_bcast_field()");
  oh_context_bcast_field(context, pfld, sfld, ftype);
}

void
oh_fortran_context_reduce_field(oh_context *context, void *pfld, void *sfld,
                                int ftype) {
  context = require_fortran_context(context, "oh_context_reduce_field()");
  oh_context_reduce_field(context, pfld, sfld, ftype);
}

void
oh_fortran_context_allreduce_field(oh_context *context, void *pfld,
                                   void *sfld, int ftype) {
  context = require_fortran_context(context, "oh_context_allreduce_field()");
  oh_context_allreduce_field(context, pfld, sfld, ftype);
}

void
oh_fortran_context_exchange_borders(oh_context *context, void *pfld,
                                    void *sfld, int ctype, int bcast) {
  context = require_fortran_context(context, "oh_context_exchange_borders()");
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
  int *pbase_ptr = pbase;
  int *nbor_ptr = nbor;

  if (!pbuf) local_errstop("oh2_init_raw requires a particle pointer slot");
  if (!pbase) local_errstop("oh2_init_raw requires a particle base array");
  if (!pcoord) local_errstop("oh2_init_raw requires a process grid array");
  if (!sdid) local_errstop("oh2_init_raw requires a region id array");
  if (!nphgram)
    local_errstop("oh2_init_raw requires a particle histogram array");
  if (!totalp)
    local_errstop("oh2_init_raw requires a total particle array");
  specBase = 1;
  init2(&sdid_ptr, nspec, maxfrac, &nphgram_ptr, &totalp_ptr, pbuf,
        &pbase_ptr, maxlocalp, NULL, mycomm, &nbor_ptr, pcoord, stats,
        repiter, verbose);
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
  int *pbase_ptr = pbase;
  int *nbor_ptr = nbor;
  int *sdoms_ptr = sdoms;
  int *bounds_ptr = bounds;
  int *fsizes_ptr = fsizes;

  if (!pbuf) local_errstop("oh3_init_raw requires a particle pointer slot");
  if (!pbase) local_errstop("oh3_init_raw requires a particle base array");
  if (!pcoord) local_errstop("oh3_init_raw requires a process grid array");
  if (!sdid) local_errstop("oh3_init_raw requires a region id array");
  if (!nphgram)
    local_errstop("oh3_init_raw requires a particle histogram array");
  if (!totalp)
    local_errstop("oh3_init_raw requires a total particle array");
  specBase = 1;
  init3(&sdid_ptr, nspec, maxfrac, &nphgram_ptr, &totalp_ptr, NULL, NULL,
        pbuf, &pbase_ptr, maxlocalp, NULL, mycomm, &nbor_ptr,
        pcoord, &sdoms_ptr, scoord, nbound, bcond, &bounds_ptr, ftypes,
        cfields, -1, ctypes, &fsizes_ptr, stats, repiter, verbose, 0);
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
  int finalized = 0;

  if (!adapter) return;
  MPI_Initialized(&initialized);
  if (initialized) MPI_Finalized(&finalized);
  if (initialized && !finalized && adapter->owns_mpi_type &&
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
  int initialized = 0;
  int finalized = 0;

  if (!adapter) return;
  MPI_Initialized(&initialized);
  if (initialized) MPI_Finalized(&finalized);
  if (initialized && !finalized && adapter->owns_mpi_type &&
      adapter->adapter.mpi_type != MPI_DATATYPE_NULL)
    MPI_Type_free(&adapter->adapter.mpi_type);
  adapter->adapter.mpi_type = fortran_type_or_null(fortran_type);
  adapter->owns_mpi_type = 0;
}

void
oh_fortran_particle_adapter_set_species_base(
  oh_fortran_particle_adapter *adapter, int species_base) {
  oh_particle_adapter_set_species_base(unwrap_adapter(adapter), species_base);
}

void
oh_fortran_particle_adapter_use_int_fields(
  oh_fortran_particle_adapter *adapter, size_t region_offset,
  size_t species_offset) {
  oh_particle_adapter_use_int_fields(unwrap_adapter(adapter), region_offset,
                                     species_offset);
  oh_particle_adapter_set_species_base(unwrap_adapter(adapter), 1);
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
  oh_particle_adapter_set_species_base(unwrap_adapter(adapter), 1);
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
