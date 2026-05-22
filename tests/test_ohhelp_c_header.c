#include "ohhelp_c.h"

int
main(void) {
  oh_particle_adapter *adapter = 0;

  oh_set_particle_position_fields(adapter, 0, 0, 0);
  oh_set_particle_adapter(adapter);
  return 0;
}
