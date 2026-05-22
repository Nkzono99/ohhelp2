#include <assert.h>

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
  struct my_particle particle = { 1.0, 2.0, 3.0, 7, 2 };

  assert(my_particle_get_region(&particle, 0) == 7);
  assert(my_particle_get_species(&particle) == 2);
  my_particle_set_region(&particle, 5, 1);
  assert(particle.region == 5);
  assert(my_particle_map_to_neighbor(&particle, 0) == 5);
  assert(my_particle_map_to_subdomain(&particle, 1) == 5);
  return 0;
}
