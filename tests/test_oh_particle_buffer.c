#include <assert.h>
#include <string.h>

#include <mpi.h>

#include "oh_part.h"
#include "oh_particle_buffer.h"

struct padded_particle {
  struct S_particle particle;
  int marker[2];
};

int
main(int argc, char **argv) {
  MPI_Datatype particle_type;
  oh_particle_adapter adapter;
  struct padded_particle source[3];
  struct padded_particle dest[3];
  struct S_particle *source_base;
  struct S_particle *second;

  MPI_Init(&argc, &argv);
  MPI_Type_contiguous(sizeof(struct padded_particle), MPI_BYTE,
                      &particle_type);
  MPI_Type_commit(&particle_type);

  adapter = oh_default_particle_adapter(particle_type);
  adapter.stride = sizeof(struct padded_particle);
  assert(oh_particle_adapter_validate(&adapter));

  memset(source, 0, sizeof(source));
  memset(dest, 0, sizeof(dest));
  source[1].particle.nid = 17;
  source[1].particle.spec = 2;
  source[1].marker[0] = 91;
  source[1].marker[1] = 92;

  source_base = (struct S_particle*)source;
  second = oh_particle_buffer_at(&adapter, source_base, 1);
  assert(second == &source[1].particle);
  assert(oh_particle_buffer_const_at(&adapter, source_base, 1) == second);
  assert(oh_particle_buffer_index(&adapter, source_base, second) == 1);
  assert(oh_particle_buffer_index(&adapter, source_base,
                                  (struct S_particle*)((char*)second+1)) == -1);

  oh_particle_buffer_copy_n(&adapter, (struct S_particle*)dest, source_base, 3);
  assert(dest[1].particle.nid == 17);
  assert(dest[1].particle.spec == 2);
  assert(dest[1].marker[0] == 91);
  assert(dest[1].marker[1] == 92);

  MPI_Type_free(&particle_type);
  MPI_Finalize();
  return 0;
}
