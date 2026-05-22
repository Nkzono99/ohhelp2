#define OH_LIB_LEVEL_4S
#include "ohhelp_c.h"

struct my_particle {
  double x, y, z;
  long long region;
  int species;
};

int
main(void) {
  oh_particle_adapter *adapter = 0;
  void *particle_buffer = 0;
  struct my_particle *particles;
  int *histogram = 0;
  int *index = 0;

  oh_set_particle_adapter(adapter);
  oh_particle_buffer(0, &particle_buffer);
  particles = particle_buffer;
  (void)particles;
  oh_per_grid_histogram(&histogram, &index);
  return 0;
}
