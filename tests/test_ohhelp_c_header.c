#include "ohhelp_c.h"

int
main(void) {
  oh_particle_adapter *adapter = 0;
  void *particle = 0;
  void *injected;

  oh_set_particle_position_fields(adapter, 0, 0, 0);
  oh_set_particle_adapter(adapter);
  oh_inject_particle(particle);
  injected = oh_inject_particle_get(particle);
  oh_remap_injected_particle(injected);
  oh_remove_injected_particle(injected);
  return 0;
}
