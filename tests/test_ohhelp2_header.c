#define OH_LIB_LEVEL 2
#include "ohhelp_c.h"

struct my_particle {
  double x, y, z;
  long long region;
  int species;
};

int
main(void) {
  struct my_particle particle;
  struct my_particle *injected;

  oh_inject_particle(&particle);
  injected = oh_inject_particle_get(&particle);
  oh_remap_injected_particle(injected);
  oh_remove_injected_particle(injected);
  return 0;
}
