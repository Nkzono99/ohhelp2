/* File: oh_particle_adapter.c
   v2 particle layout adapter draft.
*/
#include <limits.h>
#include <string.h>

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

static int
integer_field_size_valid(size_t size) {
  return size == sizeof(int) || size == sizeof(long) ||
         size == sizeof(long long);
}

static int
field_range_valid(size_t stride, size_t offset, size_t size) {
  return offset <= stride && size <= stride - offset;
}

static oh_particle_region_t
read_integer_field(const void *field, size_t size) {
  if (size == sizeof(int)) {
    int value;
    memcpy(&value, field, sizeof(value));
    return value;
  }
  if (size == sizeof(long long)) {
    long long value;
    memcpy(&value, field, sizeof(value));
    return value;
  }
  if (size == sizeof(long)) {
    long value;
    memcpy(&value, field, sizeof(value));
    return value;
  }
  return 0;
}

static void
write_integer_field(void *field, size_t size, oh_particle_region_t value) {
  if (size == sizeof(int)) {
    int narrowed = (int)value;
    memcpy(field, &narrowed, sizeof(narrowed));
  } else if (size == sizeof(long long)) {
    long long narrowed = (long long)value;
    memcpy(field, &narrowed, sizeof(narrowed));
  } else if (size == sizeof(long)) {
    long narrowed = (long)value;
    memcpy(field, &narrowed, sizeof(narrowed));
  }
}

static oh_particle_region_t
int_field_get_region(const oh_particle_adapter *adapter, const void *particle,
                     int primary_or_secondary) {
  const char *base = (const char*)particle;

  (void)primary_or_secondary;
  return read_integer_field(base + adapter->region_offset,
                            adapter->region_size);
}

static void
int_field_set_region(const oh_particle_adapter *adapter, void *particle,
                     oh_particle_region_t region, int primary_or_secondary) {
  char *base = (char*)particle;

  (void)primary_or_secondary;
  write_integer_field(base + adapter->region_offset, adapter->region_size,
                      region);
}

static int
int_field_get_species(const oh_particle_adapter *adapter,
                      const void *particle) {
  const char *base = (const char*)particle;

  if (adapter->single_species) return 0;
  return (int)read_integer_field(base + adapter->species_offset,
                                 adapter->species_size);
}

static oh_particle_region_t
int_field_map_to_region(const oh_particle_adapter *adapter, void *particle,
                        int primary_or_secondary) {
  return int_field_get_region(adapter, particle, primary_or_secondary);
}

int
oh_particle_adapter_validate(const oh_particle_adapter *adapter) {
  MPI_Aint lb, extent;
  int mpi_initialized = 0;
  int mpi_finalized = 0;

  if (!adapter) return 0;
  if (adapter->stride == 0) return 0;
  if (adapter->mpi_type == MPI_DATATYPE_NULL) return 0;
  MPI_Initialized(&mpi_initialized);
  if (mpi_initialized) MPI_Finalized(&mpi_finalized);
  if (!mpi_initialized) return 0;
  if (mpi_finalized) return 0;
  MPI_Type_get_extent(adapter->mpi_type, &lb, &extent);
  if (lb != 0 || extent != (MPI_Aint)adapter->stride) return 0;
  if (!adapter->get_region || !adapter->set_region) return 0;
  if (!adapter->get_species) return 0;
  if ((adapter->get_region == int_field_get_region ||
       adapter->set_region == int_field_set_region ||
       adapter->get_species == int_field_get_species ||
       adapter->map_to_neighbor == int_field_map_to_region ||
       adapter->map_to_subdomain == int_field_map_to_region) &&
      !integer_field_size_valid(adapter->region_size))
    return 0;
  if ((adapter->get_region == int_field_get_region ||
       adapter->set_region == int_field_set_region ||
       adapter->map_to_neighbor == int_field_map_to_region ||
       adapter->map_to_subdomain == int_field_map_to_region) &&
      !field_range_valid(adapter->stride, adapter->region_offset,
                         adapter->region_size))
    return 0;
  if (adapter->get_species == int_field_get_species &&
      !adapter->single_species &&
      !integer_field_size_valid(adapter->species_size))
    return 0;
  if (adapter->get_species == int_field_get_species &&
      !adapter->single_species &&
      !field_range_valid(adapter->stride, adapter->species_offset,
                         adapter->species_size))
    return 0;
  if (adapter->has_position_fields) {
    int dim;
    for (dim=0; dim<3; dim++)
      if (!field_range_valid(adapter->stride, adapter->position_offset[dim],
                             sizeof(double)))
        return 0;
  }
  return 1;
}

int
oh_particle_adapter_make_byte_type(size_t stride, MPI_Datatype *type) {
  MPI_Datatype raw_type = MPI_DATATYPE_NULL;
  MPI_Datatype particle_type = MPI_DATATYPE_NULL;
  int err;
  int mpi_initialized = 0;
  int mpi_finalized = 0;

  if (!type) return MPI_ERR_ARG;
  *type = MPI_DATATYPE_NULL;
  if (stride == 0 || stride > (size_t)INT_MAX) return MPI_ERR_COUNT;
  MPI_Initialized(&mpi_initialized);
  if (mpi_initialized) MPI_Finalized(&mpi_finalized);
  if (!mpi_initialized || mpi_finalized) return MPI_ERR_OTHER;

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

double*
oh_particle_adapter_position(const oh_particle_adapter *adapter,
                             void *particle, int dim) {
  if (!adapter || !particle || dim < 0 || dim >= 3) return 0;
  if (!field_range_valid(adapter->stride, adapter->position_offset[dim],
                         sizeof(double)))
    return 0;
  return (double*)((char*)particle + adapter->position_offset[dim]);
}

const double*
oh_particle_adapter_const_position(const oh_particle_adapter *adapter,
                                   const void *particle, int dim) {
  if (!adapter || !particle || dim < 0 || dim >= 3) return 0;
  if (!field_range_valid(adapter->stride, adapter->position_offset[dim],
                         sizeof(double)))
    return 0;
  return (const double*)((const char*)particle + adapter->position_offset[dim]);
}

void
oh_particle_adapter_use_position_fields(oh_particle_adapter *adapter,
                                        size_t x_offset, size_t y_offset,
                                        size_t z_offset) {
  if (!adapter) return;
  adapter->position_offset[0] = x_offset;
  adapter->position_offset[1] = y_offset;
  adapter->position_offset[2] = z_offset;
  adapter->has_position_fields = 1;
}

void
oh_particle_adapter_use_int_fields(oh_particle_adapter *adapter,
                                   size_t region_offset,
                                   size_t species_offset) {
  oh_particle_adapter_use_integer_fields(adapter, region_offset, sizeof(int),
                                         species_offset, sizeof(int));
}

void
oh_particle_adapter_use_single_species_int_region(oh_particle_adapter *adapter,
                                                  size_t region_offset) {
  oh_particle_adapter_use_single_species_integer_region(
    adapter, region_offset, sizeof(int));
}

void
oh_particle_adapter_use_integer_fields(oh_particle_adapter *adapter,
                                       size_t region_offset,
                                       size_t region_size,
                                       size_t species_offset,
                                       size_t species_size) {
  if (!adapter) return;
  adapter->region_offset = region_offset;
  adapter->region_size = region_size;
  adapter->species_offset = species_offset;
  adapter->species_size = species_size;
  adapter->single_species = 0;
  adapter->get_region = int_field_get_region;
  adapter->set_region = int_field_set_region;
  adapter->get_species = int_field_get_species;
  adapter->map_to_neighbor = int_field_map_to_region;
  adapter->map_to_subdomain = int_field_map_to_region;
}

void
oh_particle_adapter_use_single_species_integer_region(
  oh_particle_adapter *adapter, size_t region_offset, size_t region_size) {
  if (!adapter) return;
  adapter->region_offset = region_offset;
  adapter->region_size = region_size;
  adapter->species_offset = 0;
  adapter->species_size = 0;
  adapter->single_species = 1;
  adapter->species_base = 0;
  adapter->get_region = int_field_get_region;
  adapter->set_region = int_field_set_region;
  adapter->get_species = int_field_get_species;
  adapter->map_to_neighbor = int_field_map_to_region;
  adapter->map_to_subdomain = int_field_map_to_region;
}

void
oh_particle_adapter_set_species_base(oh_particle_adapter *adapter,
                                     int species_base) {
  if (!adapter) return;
  adapter->species_base = species_base;
}

oh_particle_adapter
oh_default_particle_adapter(MPI_Datatype mpi_type) {
  oh_particle_adapter adapter;

  adapter.stride = sizeof(struct S_particle);
  adapter.mpi_type = mpi_type;
  adapter.user_data = 0;
  adapter.region_offset = 0;
  adapter.region_size = sizeof(OH_nid_t);
  adapter.species_offset = 0;
  adapter.species_size = sizeof(int);
  adapter.position_offset[0] = offsetof(struct S_particle, x);
  adapter.position_offset[1] = offsetof(struct S_particle, y);
  adapter.position_offset[2] = offsetof(struct S_particle, z);
  adapter.has_position_fields = 0;
  adapter.single_species = 0;
  adapter.species_base = 0;
  adapter.get_region = default_get_region;
  adapter.set_region = default_set_region;
  adapter.get_species = default_get_species;
  adapter.map_to_neighbor = 0;
  adapter.map_to_subdomain = 0;
  return adapter;
}
