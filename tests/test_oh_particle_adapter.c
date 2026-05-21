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
  MPI_Type_contiguous(sizeof(struct S_particle), MPI_BYTE, &particle_type);
  MPI_Type_commit(&particle_type);

  adapter = oh_default_particle_adapter(particle_type);
  assert(oh_particle_adapter_validate(&adapter));
  assert(adapter.stride == sizeof(struct S_particle));

  particle.nid = 7;
  particle.spec = 3;
  assert(adapter.get_region(&particle, 0) == 7);
  assert(adapter.get_species(&particle) == 3);
  adapter.set_region(&particle, 5, 0);
  assert(particle.nid == 5);

  MPI_Type_contiguous(sizeof(struct S_particle)+8, MPI_BYTE, &padded_type);
  MPI_Type_commit(&padded_type);
  adapter.mpi_type = padded_type;
  assert(!oh_particle_adapter_validate(&adapter));
  adapter.stride = sizeof(struct S_particle)+8;
  assert(oh_particle_adapter_validate(&adapter));

  MPI_Type_free(&padded_type);
  MPI_Type_free(&particle_type);
  MPI_Finalize();
  return 0;
}
