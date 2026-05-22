#include <assert.h>
#include <stddef.h>

#include "oh_particle_adapter.h"

struct my_particle {
  double x;
  double y;
  double z;
  int region;
  int species;
};

OH_DEFINE_PARTICLE_ADAPTER_ACCESSORS(my_particle, struct my_particle,
                                     region, species)
OH_DEFINE_PARTICLE_ADAPTER_REGION_MAPPING(my_particle, struct my_particle,
                                          region)

int
main(void) {
  oh_particle_adapter adapter;
  struct my_particle particle = { 1.0, 2.0, 3.0, 7, 2 };

  assert(my_particle_get_region(0, &particle, 0) == 7);
  assert(my_particle_get_species(0, &particle) == 2);
  my_particle_set_region(0, &particle, 5, 1);
  assert(particle.region == 5);
  assert(my_particle_map_to_neighbor(0, &particle, 0) == 5);
  assert(my_particle_map_to_subdomain(0, &particle, 1) == 5);

  adapter = oh_default_particle_adapter(MPI_DATATYPE_NULL);
  oh_particle_adapter_use_int_fields(&adapter,
                                     offsetof(struct my_particle, region),
                                     offsetof(struct my_particle, species));
  adapter.position_offset[0] = offsetof(struct my_particle, x);
  adapter.position_offset[1] = offsetof(struct my_particle, y);
  adapter.position_offset[2] = offsetof(struct my_particle, z);
  assert(adapter.get_region(&adapter, &particle, 0) == 5);
  assert(adapter.get_species(&adapter, &particle) == 2);
  assert(oh_particle_adapter_position(&adapter, &particle, 0) == &particle.x);
  assert(oh_particle_adapter_position(&adapter, &particle, 1) == &particle.y);
  assert(oh_particle_adapter_position(&adapter, &particle, 2) == &particle.z);
  adapter.set_region(&adapter, &particle, 3, 0);
  assert(particle.region == 3);
  assert(adapter.map_to_neighbor(&adapter, &particle, 0) == 3);

  oh_particle_adapter_use_single_species_int_region(
    &adapter, offsetof(struct my_particle, region));
  assert(adapter.get_species(&adapter, &particle) == 0);
  return 0;
}
