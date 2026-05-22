/* File: oh_particle_adapter.h
   v2 particle layout adapter draft.
*/
#ifndef OH_PARTICLE_ADAPTER_H
#define OH_PARTICLE_ADAPTER_H

#include <stddef.h>
#include <mpi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct oh_particle_adapter oh_particle_adapter;

typedef int (*oh_particle_get_region_fn)(const oh_particle_adapter *adapter,
                                         const void *particle,
                                         int primary_or_secondary);
typedef void (*oh_particle_set_region_fn)(const oh_particle_adapter *adapter,
                                          void *particle, int region,
                                          int primary_or_secondary);
typedef int (*oh_particle_get_species_fn)(const oh_particle_adapter *adapter,
                                          const void *particle);
typedef int (*oh_particle_map_fn)(const oh_particle_adapter *adapter,
                                  void *particle,
                                  int primary_or_secondary);

struct oh_particle_adapter {
  size_t stride;
  MPI_Datatype mpi_type;
  void *user_data;
  size_t region_offset;
  size_t species_offset;
  size_t position_offset[3];
  int single_species;
  oh_particle_get_region_fn get_region;
  oh_particle_set_region_fn set_region;
  oh_particle_get_species_fn get_species;
  oh_particle_map_fn map_to_neighbor;
  oh_particle_map_fn map_to_subdomain;
};

#define OH_DEFINE_PARTICLE_ADAPTER_ACCESSORS(PREFIX, TYPE, REGION_FIELD, SPECIES_FIELD) \
  static int PREFIX##_get_region(const oh_particle_adapter *adapter, \
                                 const void *particle, \
                                 int primary_or_secondary) { \
    const TYPE *p = (const TYPE*)particle; \
    (void)adapter; \
    (void)primary_or_secondary; \
    return (int)p->REGION_FIELD; \
  } \
  static void PREFIX##_set_region(const oh_particle_adapter *adapter, \
                                  void *particle, int region, \
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
  static int PREFIX##_get_region(const oh_particle_adapter *adapter, \
                                 const void *particle, \
                                 int primary_or_secondary) { \
    const TYPE *p = (const TYPE*)particle; \
    (void)adapter; \
    (void)primary_or_secondary; \
    return (int)p->REGION_FIELD; \
  } \
  static void PREFIX##_set_region(const oh_particle_adapter *adapter, \
                                  void *particle, int region, \
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
  static int PREFIX##_map_to_neighbor(const oh_particle_adapter *adapter, \
                                      void *particle, \
                                      int primary_or_secondary) { \
    const TYPE *p = (const TYPE*)particle; \
    (void)adapter; \
    (void)primary_or_secondary; \
    return (int)p->REGION_FIELD; \
  } \
  static int PREFIX##_map_to_subdomain(const oh_particle_adapter *adapter, \
                                       void *particle, \
                                       int primary_or_secondary) { \
    const TYPE *p = (const TYPE*)particle; \
    (void)adapter; \
    (void)primary_or_secondary; \
    return (int)p->REGION_FIELD; \
  }

int oh_particle_adapter_validate(const oh_particle_adapter *adapter);
int oh_particle_adapter_make_byte_type(size_t stride, MPI_Datatype *type);
void oh_particle_adapter_use_int_fields(oh_particle_adapter *adapter,
                                        size_t region_offset,
                                        size_t species_offset);
void oh_particle_adapter_use_single_species_int_region(
  oh_particle_adapter *adapter, size_t region_offset);
oh_particle_adapter oh_default_particle_adapter(MPI_Datatype mpi_type);

#ifdef __cplusplus
}
#endif

#endif
