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

typedef struct oh_particle_adapter {
  size_t stride;
  MPI_Datatype mpi_type;
  int (*get_region)(const void *particle, int primary_or_secondary);
  void (*set_region)(void *particle, int region, int primary_or_secondary);
  int (*get_species)(const void *particle);
  int (*map_to_neighbor)(void *particle, int primary_or_secondary);
  int (*map_to_subdomain)(void *particle, int primary_or_secondary);
} oh_particle_adapter;

#define OH_DEFINE_PARTICLE_ADAPTER_ACCESSORS(PREFIX, TYPE, REGION_FIELD, SPECIES_FIELD) \
  static int PREFIX##_get_region(const void *particle, int primary_or_secondary) { \
    const TYPE *p = (const TYPE*)particle; \
    (void)primary_or_secondary; \
    return (int)p->REGION_FIELD; \
  } \
  static void PREFIX##_set_region(void *particle, int region, \
                                  int primary_or_secondary) { \
    TYPE *p = (TYPE*)particle; \
    (void)primary_or_secondary; \
    p->REGION_FIELD = region; \
  } \
  static int PREFIX##_get_species(const void *particle) { \
    const TYPE *p = (const TYPE*)particle; \
    return (int)p->SPECIES_FIELD; \
  }

#define OH_DEFINE_PARTICLE_ADAPTER_SINGLE_SPECIES_ACCESSORS(PREFIX, TYPE, REGION_FIELD) \
  static int PREFIX##_get_region(const void *particle, int primary_or_secondary) { \
    const TYPE *p = (const TYPE*)particle; \
    (void)primary_or_secondary; \
    return (int)p->REGION_FIELD; \
  } \
  static void PREFIX##_set_region(void *particle, int region, \
                                  int primary_or_secondary) { \
    TYPE *p = (TYPE*)particle; \
    (void)primary_or_secondary; \
    p->REGION_FIELD = region; \
  } \
  static int PREFIX##_get_species(const void *particle) { \
    (void)particle; \
    return 0; \
  }

#define OH_DEFINE_PARTICLE_ADAPTER_REGION_MAPPING(PREFIX, TYPE, REGION_FIELD) \
  static int PREFIX##_map_to_neighbor(void *particle, int primary_or_secondary) { \
    const TYPE *p = (const TYPE*)particle; \
    (void)primary_or_secondary; \
    return (int)p->REGION_FIELD; \
  } \
  static int PREFIX##_map_to_subdomain(void *particle, int primary_or_secondary) { \
    const TYPE *p = (const TYPE*)particle; \
    (void)primary_or_secondary; \
    return (int)p->REGION_FIELD; \
  }

int oh_particle_adapter_validate(const oh_particle_adapter *adapter);
oh_particle_adapter oh_default_particle_adapter(MPI_Datatype mpi_type);

#ifdef __cplusplus
}
#endif

#endif
