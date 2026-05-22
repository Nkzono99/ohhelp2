#include <assert.h>

#include <mpi.h>

#include "oh_particle_adapter.h"
#include "oh_part.h"

int
main(int argc, char **argv) {
  MPI_Datatype particle_type;
  MPI_Datatype padded_type;
  oh_particle_adapter adapter;
  struct S_particle particle;

  MPI_Init(&argc, &argv);
  assert(oh_particle_adapter_make_byte_type(0, &particle_type) != MPI_SUCCESS);
  assert(particle_type == MPI_DATATYPE_NULL);
  assert(oh_particle_adapter_make_byte_type(sizeof(struct S_particle),
                                            &particle_type) == MPI_SUCCESS);

  adapter = oh_default_particle_adapter(particle_type);
  assert(oh_particle_adapter_validate(&adapter));
  assert(adapter.stride == sizeof(struct S_particle));

  particle.nid = 7;
  particle.spec = 3;
  assert(adapter.get_region(&adapter, &particle, 0) == 7);
  assert(adapter.get_species(&adapter, &particle) == 3);
  adapter.set_region(&adapter, &particle, 5, 0);
  assert(particle.nid == 5);

  assert(oh_particle_adapter_make_byte_type(sizeof(struct S_particle)+8,
                                            &padded_type) == MPI_SUCCESS);
  adapter.mpi_type = padded_type;
  assert(!oh_particle_adapter_validate(&adapter));
  adapter.stride = sizeof(struct S_particle)+8;
  assert(oh_particle_adapter_validate(&adapter));

  MPI_Type_free(&padded_type);
  MPI_Type_free(&particle_type);
  MPI_Finalize();
  return 0;
}
