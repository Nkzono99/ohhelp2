#define OH_LIB_LEVEL_4P
#include "ohhelp_c.h"

int
main(void) {
  oh_particle_adapter *adapter = 0;
  struct S_particle *particle = 0;
  int *histogram = 0;

  oh_set_particle_adapter(adapter);
  (void)oh_max_local_particles(0, 0, 0, 0);
  oh_per_grid_histogram(&histogram);
  (void)oh_map_particle_to_neighbor(particle, 0, 0);
  (void)oh_map_particle_to_subdomain(particle, 0, 0);
  (void)oh_inject_particle(particle, 0);
  oh_remove_mapped_particle(particle, 0, 0);
  (void)oh_remap_particle_to_neighbor(particle, 0, 0);
  (void)oh_remap_particle_to_subdomain(particle, 0, 0);
  return 0;
}
