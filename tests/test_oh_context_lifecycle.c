#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "oh_context.h"
#include "oh_particle_adapter.h"
#include "ohhelp1.h"
#include "ohhelp1_internal.h"
#include "oh_context_internal.h"
#include "ohhelp3.h"
#include "ohhelp3_internal.h"

struct pic_particle {
  double x;
  double y;
  double z;
  int region;
  int species;
};

static void
configure_level3_context_with_maxfrac(oh_context *context, int nranks,
                                      int axis, int maxfrac) {
  int pcoord[OH_DIMENSION] = {0};
  int scoord[OH_DIMENSION][2] = {{0}};
  int bcond[OH_DIMENSION][2] = {{0}};
  int ftypes[2][OH_FTYPE_N] = {{0}};
  int cfields[1] = {-1};
  int ctypes[1][2][OH_CTYPE_N] = {{{0}}};
  int fsizes[1][OH_DIMENSION][2] = {{{0}}};
  double gsize[OH_DIMENSION] = {0.0};

  oh_context_configure_particles(context, 1, maxfrac);
  for (int i=0; i<OH_DIMENSION; i++) {
    pcoord[i] = (i==axis) ? nranks : 1;
    scoord[i][OH_LOWER] = 0;
    scoord[i][OH_UPPER] = pcoord[i];
    gsize[i] = 1.0 / (double)pcoord[i];
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
}

static void
configure_level3_context(oh_context *context, int nranks, int axis) {
  configure_level3_context_with_maxfrac(context, nranks, axis, 20);
}

static void
set_custom_adapter(oh_context *context, const oh_particle_adapter *adapter) {
  oh_context_set_particle_adapter(context, adapter);
  assert(context->particle_adapter);
  assert(context->particle_adapter->user_data == context);
  assert(context->particle_adapter->map_to_subdomain);
  assert(context->particle_adapter->map_to_neighbor);
}

static void
assert_region(const char *label, int actual, int expected, int rank) {
  if (actual != expected) {
    fprintf(stderr, "%s: rank %d expected %d got %d\n",
            label, rank, expected, actual);
    fflush(stderr);
    assert(actual == expected);
  }
}

static void
run_position_fields_preserve_region_routing_test(
    int rank, int n, MPI_Datatype pic_type, const oh_particle_adapter *adapter) {
  oh_context *context = 0;
  struct pic_particle particles[16] = {{0}};
  int nphgram[4] = {0, 0, 0, 0};
  int totalp[2] = {0, 0};
  int pbase[3] = {0, 0, 0};
  int sdid[2] = {rank, -1};
  int *nphgram_ptr = nphgram;
  int *totalp_ptr = totalp;
  int *pbase_ptr = pbase;
  int err;

  if (n != 2) return;

  err = oh_context_create(MPI_COMM_WORLD, &context);
  assert(err == MPI_SUCCESS);
  configure_level3_context_with_maxfrac(context, n, OH_DIM_X, 10000);
  set_custom_adapter(context, adapter);

  if (rank == 0) {
    particles[0].x = 0.75;
    particles[0].y = 0.5;
    particles[0].z = 0.5;
    particles[0].region = 0;
    particles[0].species = 1;
    nphgram[0] = 1;
  }

  oh_context_bind_region_ids(context, sdid, OH_PARTICLES_BORROWED);
  oh_context_bind_particles(context, particles, 16, OH_PARTICLES_BORROWED);
  oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                      &pbase_ptr, OH_PARTICLES_BORROWED);
  oh_context_set_total_particles(context);

  (void)oh_context_transbound3(context, OH_MODE_NORMAL_PRIMARY, 0);
  if (rank == 0) {
    assert(pbase[2] == 1);
    assert(particles[0].region == 0);
  } else {
    assert(pbase[2] == 0);
  }

  oh_context_unbind_region_ids(context);
  oh_context_unbind_particle_accounting(context);
  oh_context_unbind_particles(context);
  oh_context_destroy(context);
  (void)pic_type;
}

static void
run_injected_position_routing_test(int rank, int n, MPI_Datatype pic_type,
                                   const oh_particle_adapter *adapter) {
  oh_context *context = 0;
  struct pic_particle particles[16] = {{0}};
  struct pic_particle injected_particle = {0};
  int nphgram[4] = {0, 0, 0, 0};
  int totalp[2] = {0, 0};
  int pbase[3] = {0, 0, 0};
  int sdid[2] = {rank, -1};
  int *nphgram_ptr = nphgram;
  int *totalp_ptr = totalp;
  int *pbase_ptr = pbase;
  int err;

  if (n != 2) return;

  err = oh_context_create(MPI_COMM_WORLD, &context);
  assert(err == MPI_SUCCESS);
  configure_level3_context_with_maxfrac(context, n, OH_DIM_X, 10000);
  set_custom_adapter(context, adapter);

  oh_context_bind_region_ids(context, sdid, OH_PARTICLES_BORROWED);
  oh_context_bind_particles(context, particles, 16, OH_PARTICLES_BORROWED);
  oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                      &pbase_ptr, OH_PARTICLES_BORROWED);
  oh_context_set_total_particles(context);

  injected_particle.x = 0.75;
  injected_particle.y = 0.5;
  injected_particle.z = 0.5;
  injected_particle.region = oh_context_map_particle_to_subdomain(
    context, injected_particle.x, injected_particle.y, injected_particle.z);
  injected_particle.species = 1;
  if (rank == 0)
    oh_context_inject_particle(context, &injected_particle);

  (void)oh_context_transbound3(context, OH_MODE_NORMAL_PRIMARY, 0);
  if (rank == 0) {
    assert(pbase[2] == 0);
  } else {
    assert(pbase[2] == 1);
    assert(particles[0].x == injected_particle.x);
  }

  oh_context_unbind_region_ids(context);
  oh_context_unbind_particle_accounting(context);
  oh_context_unbind_particles(context);
  oh_context_destroy(context);
  (void)pic_type;
}

static void
run_localized_secondary_test(int rank, int n, MPI_Datatype pic_type,
                             const oh_particle_adapter *adapter) {
  oh_context *context = 0;
  struct pic_particle particles[16] = {{0}};
  int nphgram[4] = {0, 0, 0, 0};
  int totalp[2] = {0, 0};
  int pbase[3] = {0, 0, 0};
  int sdid[2] = {rank, -1};
  int copied_sdid[2] = {0, 0};
  int *nphgram_ptr = nphgram;
  int *totalp_ptr = totalp;
  int *pbase_ptr = pbase;
  int err;

  if (n != 2) return;

  err = oh_context_create(MPI_COMM_WORLD, &context);
  assert(err == MPI_SUCCESS);
  configure_level3_context(context, n, OH_DIM_X);
  set_custom_adapter(context, adapter);

  if (rank == 0) {
    for (int i=0; i<8; i++) {
      particles[i].x = 0.25;
      particles[i].y = 0.5;
      particles[i].z = 0.5;
      particles[i].region = 0;
      particles[i].species = 1;
    }
    nphgram[0] = 8;
  }

  oh_context_bind_region_ids(context, sdid, OH_PARTICLES_BORROWED);
  oh_context_bind_particles(context, particles, 16, OH_PARTICLES_BORROWED);
  oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                      &pbase_ptr, OH_PARTICLES_BORROWED);
  oh_context_set_total_particles(context);
  assert(oh_context_transbound3(context, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_REBALANCE_SECONDARY);
  oh_context_get_region_ids(context, copied_sdid);
  if (rank == 0) {
    assert(copied_sdid[1] == -1);
  } else {
    assert(copied_sdid[1] == 0);
    assert(pbase[2] > 0);
  }

  oh_context_unbind_region_ids(context);
  oh_context_unbind_particle_accounting(context);
  oh_context_unbind_particles(context);
  oh_context_destroy(context);
  (void)pic_type;
}

int
main(int argc, char **argv) {
  oh_context *context_x = 0;
  oh_context *context_y = 0;
  oh_particle_adapter adapter;
  MPI_Datatype pic_type = MPI_DATATYPE_NULL;
  double *weights = 0;
  struct pic_particle particles_x[16] = {{0}};
  struct pic_particle particles_y[16] = {{0}};
  struct pic_particle injected_particle = {0};
  int *nphgram_x = 0;
  int *totalp_x = 0;
  int *pbase_x = 0;
  int *nphgram_y = 0;
  int *totalp_y = 0;
  int *pbase_y = 0;
  int sdid_x[2] = {0, -1};
  int sdid_y[2] = {0, -1};
  int copied_sdid[2] = {0, 0};
  double coord_x[3] = {0.5, 0.5, 0.5};
  double coord_y[3] = {0.5, 0.5, 0.5};
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

  err = oh_context_create(MPI_COMM_WORLD, &context_x);
  assert(err == MPI_SUCCESS);
  assert(context_x);
  err = oh_context_create(MPI_COMM_WORLD, &context_y);
  assert(err == MPI_SUCCESS);
  assert(context_y);
  assert(context_x != context_y);
  assert(oh_context_max_local_particles_for_capacity(
           context_x, 1000, 250, 8) ==
         ((1000 - 1) / n + 1) + ((((1000 - 1) / n + 1) * 250 - 1) / 100 + 1));

  configure_level3_context(context_x, n, OH_DIM_X);
#if OH_DIMENSION >= 2
  configure_level3_context(context_y, n, OH_DIM_Y);
#else
  configure_level3_context(context_y, n, OH_DIM_X);
#endif

  assert(context_x->grid[OH_DIM_X].n == n);
#if OH_DIMENSION >= 2
  assert(context_x->grid[OH_DIM_Y].n == 1);
  assert(context_y->grid[OH_DIM_X].n == 1);
  assert(context_y->grid[OH_DIM_Y].n == n);
#endif

  coord_x[OH_DIM_X] = ((double)rank + 0.5) / (double)n;
#if OH_DIMENSION >= 2
  coord_y[OH_DIM_Y] = ((double)rank + 0.5) / (double)n;
#else
  coord_y[OH_DIM_X] = ((double)rank + 0.5) / (double)n;
#endif
  assert_region("context_x map subdomain",
                oh_context_map_particle_to_subdomain(
                  context_x, coord_x[0], coord_x[1], coord_x[2]),
                rank, rank);
  assert_region("context_y map subdomain",
                oh_context_map_particle_to_subdomain(
                  context_y, coord_y[0], coord_y[1], coord_y[2]),
                rank, rank);
  assert_region("context_x map neighbor",
                oh_context_map_particle_to_neighbor(
                  context_x, coord_x, coord_x+1, coord_x+2, 0),
                rank, rank);
  assert_region("context_y map neighbor",
                oh_context_map_particle_to_neighbor(
                  context_y, coord_y, coord_y+1, coord_y+2, 0),
                rank, rank);
  oh_context_bcast_field(context_x, field, field, 0);
  oh_context_reduce_field(context_x, field, field, 0);
  oh_context_allreduce_field(context_x, field, field, 0);
  oh_context_bcast_field(context_y, field, field, 0);
  oh_context_reduce_field(context_y, field, field, 0);
  oh_context_allreduce_field(context_y, field, field, 0);
  oh_context_set_region_weights(context_x, weights);
  oh_context_set_region_weights(context_y, weights);

  particles_x[0].x = coord_x[0];
  particles_x[0].y = coord_x[1];
  particles_x[0].z = coord_x[2];
  particles_x[0].region = rank;
  particles_x[0].species = 1;
  particles_y[0].x = coord_y[0];
  particles_y[0].y = coord_y[1];
  particles_y[0].z = coord_y[2];
  particles_y[0].region = rank;
  particles_y[0].species = 1;
  assert(oh_particle_adapter_make_byte_type(sizeof(particles_x[0]),
                                            &pic_type) == MPI_SUCCESS);
  adapter = oh_default_particle_adapter(pic_type);
  adapter.stride = sizeof(particles_x[0]);
  oh_particle_adapter_use_int_fields(&adapter,
                                     offsetof(struct pic_particle, region),
                                     offsetof(struct pic_particle, species));
  oh_particle_adapter_set_species_base(&adapter, 1);
  oh3_particle_adapter_use_position_fields(&adapter,
                                           offsetof(struct pic_particle, x),
                                           offsetof(struct pic_particle, y),
                                           offsetof(struct pic_particle, z));
  set_custom_adapter(context_x, &adapter);
  set_custom_adapter(context_y, &adapter);
  assert(context_x->particle_adapter != context_y->particle_adapter);
  assert(context_x->particle_adapter->map_to_subdomain(
           context_x->particle_adapter, &particles_x[0], 0) == rank);
  assert(context_y->particle_adapter->map_to_subdomain(
           context_y->particle_adapter, &particles_y[0], 0) == rank);

  run_localized_secondary_test(rank, n, pic_type, &adapter);
  run_position_fields_preserve_region_routing_test(rank, n, pic_type,
                                                   &adapter);
  run_injected_position_routing_test(rank, n, pic_type, &adapter);

  oh_context_bind_region_ids(context_x, sdid_x, OH_PARTICLES_BORROWED);
  oh_context_bind_region_ids(context_y, sdid_y, OH_PARTICLES_BORROWED);
  oh_context_bind_particles(context_x, particles_x, 16, OH_PARTICLES_BORROWED);
  oh_context_bind_particle_accounting(context_x, &nphgram_x, &totalp_x,
                                      &pbase_x, OH_PARTICLES_OWNED);
  oh_context_bind_particles(context_y, particles_y, 16, OH_PARTICLES_BORROWED);
  oh_context_bind_particle_accounting(context_y, &nphgram_y, &totalp_y,
                                      &pbase_y, OH_PARTICLES_OWNED);
  assert(nphgram_x);
  assert(totalp_x);
  assert(pbase_x);
  assert(nphgram_y);
  assert(totalp_y);
  assert(pbase_y);
  assert(nphgram_x != nphgram_y);
  assert(totalp_x != totalp_y);
  assert(pbase_x != pbase_y);
  assert(oh_context_transbound1(context_x, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_NORMAL_PRIMARY);
  assert(oh_context_transbound2(context_x, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_NORMAL_PRIMARY);
  assert(oh_context_transbound3(context_x, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_NORMAL_PRIMARY);
  assert(oh_context_transbound1(context_y, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_NORMAL_PRIMARY);
  assert(oh_context_transbound2(context_y, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_NORMAL_PRIMARY);
  assert(oh_context_transbound3(context_y, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_NORMAL_PRIMARY);
  oh_context_get_region_ids(context_x, copied_sdid);
  assert(copied_sdid[0] == sdid_x[0]);
  assert(copied_sdid[1] == sdid_x[1]);
  oh_context_get_region_ids(context_y, copied_sdid);
  assert(copied_sdid[0] == sdid_y[0]);
  assert(copied_sdid[1] == sdid_y[1]);

  if (n == 2) {
    injected_particle.x = 0.25;
    injected_particle.y = 0.5;
    injected_particle.z = 0.5;
    injected_particle.region = 0;
    injected_particle.species = 1;
    for (int i=0; rank==0 && i<8; i++)
      oh_context_inject_particle(context_x, &injected_particle);
    oh_context_set_total_particles(context_x);
    assert(oh_context_transbound3(context_x, OH_MODE_NORMAL_PRIMARY, 0) ==
           OH_MODE_REBALANCE_SECONDARY);
    oh_context_get_region_ids(context_x, copied_sdid);
    if (rank == 0) {
      assert(copied_sdid[1] == -1);
    } else {
      assert(copied_sdid[1] == 0);
      assert(pbase_x[2] > 0);
    }
  }

  oh_context_unbind_region_ids(context_x);
  oh_context_unbind_particle_accounting(context_x);
  oh_context_unbind_particles(context_x);
  oh_context_unbind_region_ids(context_y);
  oh_context_unbind_particle_accounting(context_y);
  oh_context_unbind_particles(context_y);
  oh_context_destroy(context_x);
  oh_context_destroy(context_y);
  MPI_Type_free(&pic_type);

  free(weights);
  MPI_Finalize();
  return 0;
}
