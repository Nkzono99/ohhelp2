/* File: oh_particle_adapter.c
   v2 particle layout adapter draft.
*/
#include "oh_particle_adapter.h"
#include "oh_part.h"

static int
default_get_region(const void *particle, int primary_or_secondary) {
  const struct S_particle *p = (const struct S_particle*)particle;

  (void)primary_or_secondary;
  return (int)p->nid;
}

static void
default_set_region(void *particle, int region, int primary_or_secondary) {
  struct S_particle *p = (struct S_particle*)particle;

  (void)primary_or_secondary;
  p->nid = (OH_nid_t)region;
}

static int
default_get_species(const void *particle) {
#ifdef OH_HAS_SPEC
  const struct S_particle *p = (const struct S_particle*)particle;

  return p->spec;
#else
  (void)particle;
  return 0;
#endif
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

oh_particle_adapter
oh_default_particle_adapter(MPI_Datatype mpi_type) {
  oh_particle_adapter adapter;

  adapter.stride = sizeof(struct S_particle);
  adapter.mpi_type = mpi_type;
  adapter.get_region = default_get_region;
  adapter.set_region = default_set_region;
  adapter.get_species = default_get_species;
  adapter.map_to_neighbor = 0;
  adapter.map_to_subdomain = 0;
  return adapter;
}
