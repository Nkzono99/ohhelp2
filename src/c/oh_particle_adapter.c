/* File: oh_particle_adapter.c
   v2 particle layout adapter draft.
*/
#include <limits.h>

#include "oh_particle_adapter.h"
#include "oh_part.h"

static oh_particle_region_t
default_get_region(const oh_particle_adapter *adapter, const void *particle,
                   int primary_or_secondary) {
  const struct S_particle *p = (const struct S_particle*)particle;

  (void)adapter;
  (void)primary_or_secondary;
  return (oh_particle_region_t)p->nid;
}

static void
default_set_region(const oh_particle_adapter *adapter, void *particle,
                   oh_particle_region_t region, int primary_or_secondary) {
  struct S_particle *p = (struct S_particle*)particle;

  (void)adapter;
  (void)primary_or_secondary;
  p->nid = (OH_nid_t)region;
}

static int
default_get_species(const oh_particle_adapter *adapter, const void *particle) {
#ifdef OH_HAS_SPEC
  const struct S_particle *p = (const struct S_particle*)particle;

  (void)adapter;
  return p->spec;
#else
  (void)adapter;
  (void)particle;
  return 0;
#endif
}

static oh_particle_region_t
int_field_get_region(const oh_particle_adapter *adapter, const void *particle,
                     int primary_or_secondary) {
  const char *base = (const char*)particle;

  (void)primary_or_secondary;
  return *(const int*)(base + adapter->region_offset);
}

static void
int_field_set_region(const oh_particle_adapter *adapter, void *particle,
                     oh_particle_region_t region, int primary_or_secondary) {
  char *base = (char*)particle;

  (void)primary_or_secondary;
  *(int*)(base + adapter->region_offset) = region;
}

static int
int_field_get_species(const oh_particle_adapter *adapter,
                      const void *particle) {
  const char *base = (const char*)particle;

  if (adapter->single_species) return 0;
  return *(const int*)(base + adapter->species_offset);
}

static oh_particle_region_t
int_field_map_to_region(const oh_particle_adapter *adapter, void *particle,
                        int primary_or_secondary) {
  return int_field_get_region(adapter, particle, primary_or_secondary);
}

int
oh_particle_adapter_validate(const oh_particle_adapter *adapter) {
  MPI_Aint lb, extent;
  int mpi_initialized;

  if (!adapter) return 0;
  if (adapter->stride == 0) return 0;
  if (adapter->mpi_type == MPI_DATATYPE_NULL) return 0;
  MPI_Initialized(&mpi_initialized);
  if (mpi_initialized) {
    MPI_Type_get_extent(adapter->mpi_type, &lb, &extent);
    if (lb != 0 || extent != (MPI_Aint)adapter->stride) return 0;
  }
  if (!adapter->get_region || !adapter->set_region) return 0;
  if (!adapter->get_species) return 0;
  return 1;
}

int
oh_particle_adapter_make_byte_type(size_t stride, MPI_Datatype *type) {
  MPI_Datatype raw_type = MPI_DATATYPE_NULL;
  MPI_Datatype particle_type = MPI_DATATYPE_NULL;
  int err;

  if (!type) return MPI_ERR_ARG;
  *type = MPI_DATATYPE_NULL;
  if (stride == 0 || stride > (size_t)INT_MAX) return MPI_ERR_COUNT;

  err = MPI_Type_contiguous((int)stride, MPI_BYTE, &raw_type);
  if (err != MPI_SUCCESS) return err;

  err = MPI_Type_create_resized(raw_type, 0, (MPI_Aint)stride,
                                &particle_type);
  MPI_Type_free(&raw_type);
  if (err != MPI_SUCCESS) return err;

  err = MPI_Type_commit(&particle_type);
  if (err != MPI_SUCCESS) {
    MPI_Type_free(&particle_type);
    return err;
  }

  *type = particle_type;
  return MPI_SUCCESS;
}

void
oh_particle_adapter_use_int_fields(oh_particle_adapter *adapter,
                                   size_t region_offset,
                                   size_t species_offset) {
  if (!adapter) return;
  adapter->region_offset = region_offset;
  adapter->species_offset = species_offset;
  adapter->single_species = 0;
  adapter->get_region = int_field_get_region;
  adapter->set_region = int_field_set_region;
  adapter->get_species = int_field_get_species;
  adapter->map_to_neighbor = int_field_map_to_region;
  adapter->map_to_subdomain = int_field_map_to_region;
}

void
oh_particle_adapter_use_single_species_int_region(oh_particle_adapter *adapter,
                                                  size_t region_offset) {
  if (!adapter) return;
  adapter->region_offset = region_offset;
  adapter->species_offset = 0;
  adapter->single_species = 1;
  adapter->get_region = int_field_get_region;
  adapter->set_region = int_field_set_region;
  adapter->get_species = int_field_get_species;
  adapter->map_to_neighbor = int_field_map_to_region;
  adapter->map_to_subdomain = int_field_map_to_region;
}

oh_particle_adapter
oh_default_particle_adapter(MPI_Datatype mpi_type) {
  oh_particle_adapter adapter;

  adapter.stride = sizeof(struct S_particle);
  adapter.mpi_type = mpi_type;
  adapter.user_data = 0;
  adapter.region_offset = 0;
  adapter.species_offset = 0;
  adapter.position_offset[0] = 0;
  adapter.position_offset[1] = 0;
  adapter.position_offset[2] = 0;
  adapter.single_species = 0;
  adapter.get_region = default_get_region;
  adapter.set_region = default_set_region;
  adapter.get_species = default_get_species;
  adapter.map_to_neighbor = 0;
  adapter.map_to_subdomain = 0;
  return adapter;
}
