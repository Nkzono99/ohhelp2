#include <assert.h>
#include <math.h>

#include "oh_load_balance.h"

int
main(void) {
  assert(oh_region_load(10, 2.5) == 25.0);
  assert(oh_region_load(0, 2.5) == 0.0);
  assert(oh_region_load(10, 0.0) == 0.0);

  assert(fabs(oh_load_limit(100.0, 20, 4) - 30.0) < 1.0e-12);
  assert(oh_load_limit(0.0, 20, 4) == 0.0);
  assert(oh_load_limit(100.0, 20, 0) == 0.0);

  assert(oh_particles_for_load(10.0, 2.0, 100) == 5);
  assert(oh_particles_for_load(10.1, 2.0, 100) == 6);
  assert(oh_particles_for_load(0.1, 2.0, 100) == 1);
  assert(oh_particles_for_load(10.0, 2.0, 3) == 3);
  assert(oh_particles_for_load(10.0, 0.0, 100) == 0);

  assert(oh_load_after_transfer(25.0, 3, 2.0) == 19.0);
  assert(oh_load_after_transfer(25.0, 20, 2.0) == 0.0);
  assert(oh_load_after_transfer(25.0, 20, 0.0) == 25.0);

  assert(oh_weighted_transfer_count(50.0, 40.0, 2.0, 100) == 5);
  assert(oh_weighted_transfer_count(50.0, 39.9, 2.0, 100) == 6);
  assert(oh_weighted_transfer_count(50.0, 50.0, 2.0, 100) == 0);
  assert(oh_weighted_transfer_count(50.0, 40.0, 2.0, 3) == 3);
  assert(oh_weighted_transfer_count(50.0, 40.0, 0.0, 100) == 0);

  return 0;
}
