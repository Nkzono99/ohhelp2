/* File: oh_particle_adapter.h
   v2 particle layout adapter draft.
*/
#ifndef OH_PARTICLE_ADAPTER_H
#define OH_PARTICLE_ADAPTER_H

#include <stddef.h>
#include <string.h>
#include <mpi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct oh_particle_adapter oh_particle_adapter;
typedef long long oh_particle_region_t;

typedef oh_particle_region_t (*oh_particle_get_region_fn)(
  const oh_particle_adapter *adapter, const void *particle,
  int primary_or_secondary);
typedef void (*oh_particle_set_region_fn)(const oh_particle_adapter *adapter,
                                          void *particle,
                                          oh_particle_region_t region,
                                          int primary_or_secondary);
typedef int (*oh_particle_get_species_fn)(const oh_particle_adapter *adapter,
                                          const void *particle);
typedef oh_particle_region_t (*oh_particle_map_fn)(
  const oh_particle_adapter *adapter, void *particle,
  int primary_or_secondary);

#define OH_PARTICLE_ADAPTER_ACCESS_CALLBACK 0
#define OH_PARTICLE_ADAPTER_ACCESS_INTEGER_FIELD 1
#define OH_PARTICLE_ADAPTER_ACCESS_SINGLE_SPECIES 2

#define OH_PARTICLE_ADAPTER_MAP_CALLBACK 0
#define OH_PARTICLE_ADAPTER_MAP_REGION_FIELD 1

struct oh_particle_adapter {
  size_t stride;
  MPI_Datatype mpi_type;
  void *user_data;
  size_t region_offset;
  size_t region_size;
  size_t species_offset;
  size_t species_size;
  size_t position_offset[3];
  int has_position_fields;
  int single_species;
  int species_base;
  oh_particle_get_region_fn get_region;
  oh_particle_set_region_fn set_region;
  oh_particle_get_species_fn get_species;
  oh_particle_map_fn map_to_neighbor;
  oh_particle_map_fn map_to_subdomain;
  int region_access;
  int species_access;
  int map_to_neighbor_access;
  int map_to_subdomain_access;
};

#define OH_DEFINE_PARTICLE_ADAPTER_ACCESSORS(PREFIX, TYPE, REGION_FIELD, SPECIES_FIELD) \
  static oh_particle_region_t PREFIX##_get_region( \
      const oh_particle_adapter *adapter, const void *particle, \
      int primary_or_secondary) { \
    const TYPE *p = (const TYPE*)particle; \
    (void)adapter; \
    (void)primary_or_secondary; \
    return (oh_particle_region_t)p->REGION_FIELD; \
  } \
  static void PREFIX##_set_region(const oh_particle_adapter *adapter, \
                                  void *particle, \
                                  oh_particle_region_t region, \
                                  int primary_or_secondary) { \
    TYPE *p = (TYPE*)particle; \
    (void)adapter; \
    (void)primary_or_secondary; \
    p->REGION_FIELD = region; \
  } \
  static int PREFIX##_get_species(const oh_particle_adapter *adapter, \
                                  const void *particle) { \
    const TYPE *p = (const TYPE*)particle; \
    (void)adapter; \
    return (int)p->SPECIES_FIELD; \
  }

#define OH_DEFINE_PARTICLE_ADAPTER_SINGLE_SPECIES_ACCESSORS(PREFIX, TYPE, REGION_FIELD) \
  static oh_particle_region_t PREFIX##_get_region( \
      const oh_particle_adapter *adapter, const void *particle, \
      int primary_or_secondary) { \
    const TYPE *p = (const TYPE*)particle; \
    (void)adapter; \
    (void)primary_or_secondary; \
    return (oh_particle_region_t)p->REGION_FIELD; \
  } \
  static void PREFIX##_set_region(const oh_particle_adapter *adapter, \
                                  void *particle, \
                                  oh_particle_region_t region, \
                                  int primary_or_secondary) { \
    TYPE *p = (TYPE*)particle; \
    (void)adapter; \
    (void)primary_or_secondary; \
    p->REGION_FIELD = region; \
  } \
  static int PREFIX##_get_species(const oh_particle_adapter *adapter, \
                                  const void *particle) { \
    (void)adapter; \
    (void)particle; \
    return 0; \
  }

#define OH_DEFINE_PARTICLE_ADAPTER_REGION_MAPPING(PREFIX, TYPE, REGION_FIELD) \
  static oh_particle_region_t PREFIX##_map_to_neighbor( \
      const oh_particle_adapter *adapter, void *particle, \
      int primary_or_secondary) { \
    const TYPE *p = (const TYPE*)particle; \
    (void)adapter; \
    (void)primary_or_secondary; \
    return (oh_particle_region_t)p->REGION_FIELD; \
  } \
  static oh_particle_region_t PREFIX##_map_to_subdomain( \
      const oh_particle_adapter *adapter, void *particle, \
      int primary_or_secondary) { \
    const TYPE *p = (const TYPE*)particle; \
    (void)adapter; \
    (void)primary_or_secondary; \
    return (oh_particle_region_t)p->REGION_FIELD; \
  }

int oh_particle_adapter_validate(const oh_particle_adapter *adapter);
void oh_particle_adapter_refresh_fast_flags(oh_particle_adapter *adapter);
int oh_particle_adapter_make_byte_type(size_t stride, MPI_Datatype *type);
double *oh_particle_adapter_position(const oh_particle_adapter *adapter,
                                     void *particle, int dim);
const double *oh_particle_adapter_const_position(
  const oh_particle_adapter *adapter, const void *particle, int dim);
void oh_particle_adapter_use_position_fields(oh_particle_adapter *adapter,
                                             size_t x_offset,
                                             size_t y_offset,
                                             size_t z_offset);
void oh_particle_adapter_use_int_fields(oh_particle_adapter *adapter,
                                        size_t region_offset,
                                        size_t species_offset);
void oh_particle_adapter_use_single_species_int_region(
  oh_particle_adapter *adapter, size_t region_offset);
void oh_particle_adapter_use_integer_fields(oh_particle_adapter *adapter,
                                            size_t region_offset,
                                            size_t region_size,
                                            size_t species_offset,
                                            size_t species_size);
void oh_particle_adapter_use_single_species_integer_region(
  oh_particle_adapter *adapter, size_t region_offset, size_t region_size);
void oh_particle_adapter_set_species_base(oh_particle_adapter *adapter,
                                          int species_base);
oh_particle_adapter oh_default_particle_adapter(MPI_Datatype mpi_type);

static inline oh_particle_region_t
oh_particle_adapter_read_integer_value(const void *field, size_t size) {
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

static inline void
oh_particle_adapter_write_integer_value(void *field, size_t size,
                                        oh_particle_region_t value) {
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

static inline oh_particle_region_t
oh_particle_adapter_read_region_field(const oh_particle_adapter *adapter,
                                      const void *particle) {
  return oh_particle_adapter_read_integer_value(
    (const char*)particle + adapter->region_offset, adapter->region_size);
}

static inline void
oh_particle_adapter_write_region_field(const oh_particle_adapter *adapter,
                                       void *particle,
                                       oh_particle_region_t region) {
  oh_particle_adapter_write_integer_value(
    (char*)particle + adapter->region_offset, adapter->region_size, region);
}

static inline int
oh_particle_adapter_read_species_field(const oh_particle_adapter *adapter,
                                       const void *particle) {
  if (adapter->single_species) return 0;
  return (int)oh_particle_adapter_read_integer_value(
    (const char*)particle + adapter->species_offset, adapter->species_size);
}

#ifdef __cplusplus
}
#endif

#endif
