#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "oh_context.h"
#include "oh_particle_adapter.h"
#include "ohhelp1.h"
#include "ohhelp1_internal.h"
#include "oh_context_internal.h"
#include "ohhelp3.h"

struct pic_particle {
  double x;
  double y;
  double z;
  int region;
  int species;
};

int
main(int argc, char **argv) {
  oh_context *context = 0;
  oh_particle_adapter adapter;
  MPI_Datatype pic_type = MPI_DATATYPE_NULL;
  double *weights = 0;
  struct pic_particle particles[4] = {{0}};
  int *nphgram = 0;
  int *totalp = 0;
  int *pbase = 0;
  int pcoord[OH_DIMENSION] = {0};
  int scoord[OH_DIMENSION][2] = {{0}};
  int bcond[OH_DIMENSION][2] = {{0}};
  int ftypes[2][OH_FTYPE_N] = {{0}};
  int cfields[1] = {-1};
  int ctypes[1][2][OH_CTYPE_N] = {{{0}}};
  int fsizes[1][OH_DIMENSION][2] = {{{0}}};
  double gsize[OH_DIMENSION] = {0.0};
  double x = 0.5;
  double y = 0.5;
  double z = 0.5;
  double field[8] = {0.0};
  int rank = 0;
  int n = 0;
  int err;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &n);

  weights = (double*)calloc((size_t)n, sizeof(*weights));
  assert(weights);
  for (int i=0; i<n; i++) weights[i] = 1.0;

  err = oh_context_create(MPI_COMM_WORLD, &context);
  assert(err == MPI_SUCCESS);
  assert(context);

  oh_context_configure_particles(context, 1, 20);
  for (int i=0; i<OH_DIMENSION; i++) {
    pcoord[i] = (i==0) ? n : 1;
    scoord[i][0] = 0;
    scoord[i][1] = pcoord[i];
    gsize[i] = 1.0;
  }
  ftypes[0][OH_FTYPE_ES] = 1;
  ftypes[0][OH_FTYPE_LO] = 0;
  ftypes[0][OH_FTYPE_UP] = 0;
  ftypes[0][OH_FTYPE_BL] = 0;
  ftypes[0][OH_FTYPE_BU] = 0;
  ftypes[0][OH_FTYPE_RL] = 0;
  ftypes[0][OH_FTYPE_RU] = 0;
  oh_context_configure_level3(context, pcoord, 0, &scoord[0][0], 1,
                              &bcond[0][0], 0, &ftypes[0][0], cfields,
                              &ctypes[0][0][0], &fsizes[0][0][0]);
  oh_context_grid_size(context, gsize);
  assert(oh_context_map_particle_to_subdomain(context, x, y, z) == 0);
  assert(oh_context_map_particle_to_neighbor(context, &x, &y, &z, 0) == 0);
  oh_context_bcast_field(context, field, field, 0);
  oh_context_reduce_field(context, field, field, 0);
  oh_context_allreduce_field(context, field, field, 0);
  oh_context_set_region_weights(context, weights);
  particles[0].x = ((double)rank + 0.5) / (double)n;
  particles[0].y = 0.5;
  particles[0].z = 0.5;
  assert(oh_particle_adapter_make_byte_type(sizeof(particles[0]),
                                            &pic_type) == MPI_SUCCESS);
  adapter = oh_default_particle_adapter(pic_type);
  adapter.stride = sizeof(particles[0]);
  oh_particle_adapter_use_int_fields(&adapter,
                                     offsetof(struct pic_particle, region),
                                     offsetof(struct pic_particle, species));
  oh3_particle_adapter_use_position_fields(&adapter,
                                           offsetof(struct pic_particle, x),
                                           offsetof(struct pic_particle, y),
                                           offsetof(struct pic_particle, z));
  oh_context_set_particle_adapter(context, &adapter);
  assert(context->particle_adapter);
  assert(context->particle_adapter->user_data == context);
  assert(context->particle_adapter->map_to_subdomain(
           context->particle_adapter, &particles[0], 0) == rank);
  oh_context_bind_particles(context, particles, 4, OH_PARTICLES_BORROWED);
  oh_context_bind_particle_accounting(context, &nphgram, &totalp, &pbase,
                                      OH_PARTICLES_OWNED);
  assert(nphgram);
  assert(totalp);
  assert(pbase);
  assert(oh_context_transbound3(context, MODE_NORM_PRI, 0) == MODE_NORM_PRI);
  oh_context_unbind_particle_accounting(context);
  oh_context_unbind_particles(context);
  oh_context_destroy(context);
  MPI_Type_free(&pic_type);

  free(weights);
  MPI_Finalize();
  return 0;
}
