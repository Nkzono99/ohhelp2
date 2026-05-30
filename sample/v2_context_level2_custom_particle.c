/* Minimal v2 heap-context Level-2 custom particle sample.
   This sample is compile- and run-checked by scripts/docker-build-test.sh. */
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include <mpi.h>

#include "oh_context.h"
#include "oh_particle_adapter.h"
#include "oh_particle_ownership.h"

struct pic_particle {
  double x, y, z;
  long long region;
  int species;
};

int
main(int argc, char **argv) {
  oh_context *context = NULL;
  MPI_Datatype particle_type = MPI_DATATYPE_NULL;
  oh_particle_adapter adapter;
  struct pic_particle particles[4] = {{0}};
  int sdid[2] = {0, -1};
  int *nphgram = NULL;
  int totalp[2] = {0, 0};
  int pbase[3] = {0, 0, 0};
  int *nphgram_slot = NULL;
  int *totalp_slot = totalp;
  int *pbase_slot = pbase;
  double *weights = NULL;
  int rank = 0;
  int nranks = 0;
  int mode;

  (void)argv;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nranks);

  assert(oh_context_create(MPI_COMM_WORLD, &context) == MPI_SUCCESS);
  oh_context_configure_particles(context, 1, 0);

  assert(oh_particle_adapter_make_byte_type(sizeof(struct pic_particle),
                                            &particle_type) == MPI_SUCCESS);
  adapter = oh_default_particle_adapter(particle_type);
  adapter.stride = sizeof(struct pic_particle);
  oh_particle_adapter_use_integer_fields(
    &adapter,
    offsetof(struct pic_particle, region),
    sizeof(((struct pic_particle*)0)->region),
    offsetof(struct pic_particle, species),
    sizeof(((struct pic_particle*)0)->species));
  oh_context_set_particle_adapter(context, &adapter);
  MPI_Type_free(&particle_type);

  weights = (double*)calloc((size_t)nranks, sizeof(*weights));
  nphgram = (int*)calloc((size_t)2 * nranks, sizeof(*nphgram));
  assert(weights);
  assert(nphgram);
  for (int i=0; i<nranks; i++) weights[i] = 1.0;

  particles[0].x = (double)rank + 0.5;
  particles[0].y = 0.5;
  particles[0].z = 0.5;
  particles[0].region = rank;
  particles[0].species = 0;
  sdid[0] = rank;
  nphgram[rank] = 1;
  nphgram_slot = nphgram;

  oh_context_set_region_weights(context, weights);
  oh_context_bind_region_ids(context, sdid, OH_PARTICLES_BORROWED);
  oh_context_bind_particles(context, particles, 4, OH_PARTICLES_BORROWED);
  oh_context_bind_particle_accounting(context, &nphgram_slot, &totalp_slot,
                                      &pbase_slot, OH_PARTICLES_BORROWED);
  oh_context_set_total_particles(context);
  mode = oh_context_transbound2(context, OH_MODE_NORMAL_PRIMARY, 0);
  assert(mode == OH_MODE_NORMAL_PRIMARY);
  assert(pbase[0] == 0);
  assert(pbase[1] == 1);
  assert(pbase[2] == 1);

  oh_context_unbind_particle_accounting(context);
  oh_context_unbind_particles(context);
  oh_context_unbind_region_ids(context);
  oh_context_destroy(context);
  free(nphgram);
  free(weights);
  MPI_Finalize();
  return 0;
}
