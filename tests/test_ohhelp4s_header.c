#define OH_LIB_LEVEL_4S
#include "ohhelp_c.h"

int
main(void) {
  struct S_particle *particles = 0;
  int *histogram = 0;
  int *index = 0;

  oh_particle_buffer(0, &particles);
  oh_per_grid_histogram(&histogram, &index);
  return 0;
}
