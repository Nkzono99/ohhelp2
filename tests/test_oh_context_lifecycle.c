#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "oh_context.h"
#include "oh_particle_adapter.h"
#include "oh_part.h"
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

struct callback_particle {
  double marker;
  long long logical_region;
  int species_id;
};

OH_DEFINE_PARTICLE_ADAPTER_ACCESSORS(callback_particle,
                                     struct callback_particle,
                                     logical_region, species_id)
OH_DEFINE_PARTICLE_ADAPTER_REGION_MAPPING(callback_particle,
                                          struct callback_particle,
                                          logical_region)

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
configure_level3_context_with_border_exchange(oh_context *context, int nranks) {
  int pcoord[OH_DIMENSION] = {0};
  int scoord[OH_DIMENSION][2] = {{0}};
  int bcond[OH_DIMENSION][2] = {{0}};
  int ftypes[2][OH_FTYPE_N] = {{0}};
  int cfields[2] = {0, -1};
  int ctypes[1][2][OH_CTYPE_N] = {{{0}}};
  int fsizes[1][OH_DIMENSION][2] = {{{0}}};
  double gsize[OH_DIMENSION] = {0.0};

  oh_context_configure_particles(context, 1, 20);
  for (int i=0; i<OH_DIMENSION; i++) {
    pcoord[i] = (i==OH_DIM_X) ? nranks : 1;
    scoord[i][OH_LOWER] = 0;
    scoord[i][OH_UPPER] = pcoord[i];
    gsize[i] = 1.0 / (double)pcoord[i];
    ctypes[0][OH_LOWER][OH_CTYPE_SIZE] = 1;
    ctypes[0][OH_UPPER][OH_CTYPE_SIZE] = 1;
  }
  ftypes[0][OH_FTYPE_ES] = 1;
  oh_context_configure_level3(context, pcoord, 0, &scoord[0][0], 1,
                              &bcond[0][0], 0, &ftypes[0][0], cfields,
                              &ctypes[0][0][0], &fsizes[0][0][0]);
  oh_context_grid_size(context, gsize);
}

static void
run_border_exchange_destroy_after_finalize_test(int n) {
  oh_context *context = 0;
  MPI_Datatype particle_type = MPI_DATATYPE_NULL;
  oh_particle_adapter adapter;
  int err;

  err = oh_context_create(MPI_COMM_WORLD, &context);
  assert(err == MPI_SUCCESS);
  configure_level3_context_with_border_exchange(context, n);
  assert(context->n_of_exchanges == 1);
  assert(context->border_exchange);
#if OH_DIMENSION > 1
  {
    struct S_borderexc (*bx)[2][OH_DIMENSION][2] =
      (struct S_borderexc(*)[2][OH_DIMENSION][2])context->border_exchange;
    assert(bx[0][0][OH_DIM_X][OH_LOWER].send.deriv);
  }
#endif

  assert(oh_particle_adapter_make_byte_type(sizeof(struct pic_particle),
                                            &particle_type) == MPI_SUCCESS);
  adapter = oh_default_particle_adapter(particle_type);
  adapter.stride = sizeof(struct pic_particle);
  oh_particle_adapter_use_int_fields(&adapter,
                                     offsetof(struct pic_particle, region),
                                     offsetof(struct pic_particle, species));
  oh3_particle_adapter_use_position_fields(&adapter,
                                           offsetof(struct pic_particle, x),
                                           offsetof(struct pic_particle, y),
                                           offsetof(struct pic_particle, z));
  oh_context_set_particle_adapter(context, &adapter);
  assert(context->use_custom_particle_adapter);
  assert(context->particle_adapter->mpi_type != particle_type);
  MPI_Type_free(&particle_type);

  MPI_Finalize();
  oh_context_destroy(context);
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
run_region_weight_copy_reset_test(oh_context *context, int n) {
  double *local_weights = (double*)calloc((size_t)n, sizeof(*local_weights));

  assert(local_weights);
  for (int i=0; i<n; i++) local_weights[i] = 1.0 + (double)i;
  if (n == 1) local_weights[0] = 2.0;

  oh_context_set_region_weights(context, local_weights);
  assert(context->weighted_load_balancing);
  for (int i=0; i<n; i++)
    assert(context->region_weights[i] == local_weights[i]);

  local_weights[0] = 99.0;
  assert(context->region_weights[0] != local_weights[0]);

  oh_context_set_region_weights(context, NULL);
  assert(!context->weighted_load_balancing);
  for (int i=0; i<n; i++)
    assert(context->region_weights[i] == 1.0);

  free(local_weights);
}

static void
run_region_weights_context_isolation_test(int n) {
  oh_context *context_a = 0;
  oh_context *context_b = 0;
  double *local_weights = (double*)calloc((size_t)n, sizeof(*local_weights));
  int legacy_weighted_load_balancing = weightedLoadBalancing;
  int err;

  assert(local_weights);
  err = oh_context_create(MPI_COMM_WORLD, &context_a);
  assert(err == MPI_SUCCESS);
  err = oh_context_create(MPI_COMM_WORLD, &context_b);
  assert(err == MPI_SUCCESS);
  configure_level3_context(context_a, n, OH_DIM_X);
  configure_level3_context(context_b, n, OH_DIM_X);

  for (int i=0; i<n; i++) local_weights[i] = 1.0;
  local_weights[0] = 2.0;
  oh_context_set_region_weights(context_a, local_weights);
  assert(weightedLoadBalancing == legacy_weighted_load_balancing);
  local_weights[0] = 1.0;
  oh_context_set_region_weights(context_b, local_weights);
  assert(weightedLoadBalancing == legacy_weighted_load_balancing);

  assert(context_a->weighted_load_balancing);
  assert(context_a->region_weights[0] == 2.0);
  for (int i=1; i<n; i++)
    assert(context_a->region_weights[i] == 1.0);
  assert(!context_b->weighted_load_balancing);
  for (int i=0; i<n; i++)
    assert(context_b->region_weights[i] == 1.0);

  oh_context_set_region_weights(context_a, NULL);
  assert(weightedLoadBalancing == legacy_weighted_load_balancing);
  assert(!context_a->weighted_load_balancing);
  for (int i=0; i<n; i++)
    assert(context_a->region_weights[i] == 1.0);
  assert(!context_b->weighted_load_balancing);
  for (int i=0; i<n; i++)
    assert(context_b->region_weights[i] == 1.0);

  oh_context_destroy(context_a);
  oh_context_destroy(context_b);
  free(local_weights);
}

static void
run_context_owned_comm_test(int n, int rank) {
  oh_context *context = 0;
  MPI_Comm app_comm = MPI_COMM_NULL;
  int sdid[2] = {rank, -1};
  int totalp[2] = {0, 0};
  int pbase[3] = {0, 0, 0};
  int *nphgram = (int*)calloc((size_t)(2*n), sizeof(*nphgram));
  int *nphgram_ptr = nphgram;
  int *totalp_ptr = totalp;
  int *pbase_ptr = pbase;
  int err;
  int compare;

  assert(nphgram);
  MPI_Comm_dup(MPI_COMM_WORLD, &app_comm);
  err = oh_context_create(app_comm, &context);
  assert(err == MPI_SUCCESS);
  assert(context);
  assert(context->owns_comm);
  MPI_Comm_compare(context->comm, app_comm, &compare);
  assert(compare == MPI_CONGRUENT || compare == MPI_IDENT);
  MPI_Comm_free(&app_comm);

  oh_context_configure_particles(context, 1, 0);
  oh_context_bind_region_ids(context, sdid, OH_PARTICLES_BORROWED);
  oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                      &pbase_ptr, OH_PARTICLES_BORROWED);
  assert(oh_context_transbound1(context, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_NORMAL_PRIMARY);

  oh_context_unbind_particle_accounting(context);
  oh_context_unbind_region_ids(context);
  oh_context_destroy(context);
  free(nphgram);
}

static void
run_context_create_invalid_comm_test(void) {
  oh_context *context = (oh_context*)1;
  int err;

  err = oh_context_create(MPI_COMM_NULL, &context);
  assert(err == MPI_ERR_COMM);
  assert(context == 0);
}

static void
run_context_reconfigure_particles_test(int n) {
  oh_context *context = 0;
  int *nphgram = 0;
  int *totalp = 0;
  int *pbase = 0;
  int err;

  err = oh_context_create(MPI_COMM_WORLD, &context);
  assert(err == MPI_SUCCESS);
  assert(context);

  oh_context_configure_particles(context, 1, 20);
  assert(context->n_of_species == 1);
  assert(context->owns_level1_storage);
  context->n_of_recv[2*n - 1] = 123;
  context->injected_particles[3] = 456;

  oh_context_configure_particles(context, 2, 30);
  assert(context->n_of_species == 2);
  assert(context->max_fraction == 30);
  assert(context->owns_level1_storage);
  assert(context->n_of_recv[2*n - 1] == 0);
  assert(context->injected_particles[3] == 0);

  oh_context_bind_particle_accounting(context, &nphgram, &totalp, &pbase,
                                      OH_PARTICLES_OWNED);
  assert(nphgram);
  assert(totalp);
  assert(pbase);
  assert(oh_context_transbound1(context, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_NORMAL_PRIMARY);
  oh_context_unbind_particle_accounting(context);
  oh_context_destroy(context);
}

static void
run_context_particle_mpi_type_ownership_test(int n, int rank) {
  oh_context *context = 0;
  MPI_Datatype particle_type = MPI_DATATYPE_NULL;
  struct S_particle particles[4] = {{0}};
  int *nphgram = (int*)calloc((size_t)2*n, sizeof(*nphgram));
  int totalp[2] = {0, 0};
  int pbase[3] = {0, 0, 0};
  int sdid[2] = {rank, -1};
  int *nphgram_ptr = nphgram;
  int *totalp_ptr = totalp;
  int *pbase_ptr = pbase;
  int err;
  int destination;

  if (n != 2) {
    free(nphgram);
    return;
  }

  err = oh_context_create(MPI_COMM_WORLD, &context);
  assert(err == MPI_SUCCESS);
  configure_level3_context_with_maxfrac(context, n, OH_DIM_X, 10000);
  err = oh_particle_adapter_make_byte_type(sizeof(particles[0]),
                                           &particle_type);
  assert(err == MPI_SUCCESS);
  oh_context_set_particle_mpi_type(context, particle_type);
  MPI_Type_free(&particle_type);
  assert(context->particle_mpi_type != MPI_DATATYPE_NULL);

  destination = 1 - rank;
  particles[0].nid = destination;
  particles[0].spec = 0;
  particles[0].x = ((double)destination + 0.5) / (double)n;
  particles[0].y = 0.5;
  particles[0].z = 0.5;
  nphgram[destination] = 1;

  oh_context_bind_region_ids(context, sdid, OH_PARTICLES_BORROWED);
  oh_context_bind_particles(context, particles, 4, OH_PARTICLES_BORROWED);
  oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                      &pbase_ptr, OH_PARTICLES_BORROWED);
  oh_context_set_total_particles(context);
  assert(oh_context_transbound2(context, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_NORMAL_PRIMARY);
  assert(pbase[2] == 1);
  assert(particles[0].nid == rank);
  assert(particles[0].spec == 0);

  oh_context_unbind_region_ids(context);
  oh_context_unbind_particle_accounting(context);
  oh_context_unbind_particles(context);
  oh_context_destroy(context);
  free(nphgram);
}

static void
run_owned_particle_buffer_test(int n, int rank,
                               const oh_particle_adapter *adapter) {
  oh_context *context = 0;
  struct pic_particle *particles;
  int *nphgram = (int*)calloc((size_t)2*n, sizeof(*nphgram));
  int totalp[2] = {0, 0};
  int pbase[3] = {0, 0, 0};
  int sdid[2] = {rank, -1};
  int *nphgram_ptr = nphgram;
  int *totalp_ptr = totalp;
  int *pbase_ptr = pbase;
  int err;

  assert(nphgram);
  err = oh_context_create(MPI_COMM_WORLD, &context);
  assert(err == MPI_SUCCESS);
  configure_level3_context(context, n, OH_DIM_X);
  set_custom_adapter(context, adapter);

  particles = (struct pic_particle*)oh_context_bind_particles(
    context, NULL, 16, OH_PARTICLES_OWNED);
  assert(particles);
  assert(context->particles == particles);
  assert(context->particle_buffer_ownership == OH_PARTICLES_OWNED);

  particles[0].x = ((double)rank + 0.5) / (double)n;
  particles[0].y = 0.5;
  particles[0].z = 0.5;
  particles[0].region = rank;
  particles[0].species = 1;
  nphgram[rank] = 1;

  oh_context_bind_region_ids(context, sdid, OH_PARTICLES_BORROWED);
  oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                      &pbase_ptr, OH_PARTICLES_BORROWED);
  oh_context_set_total_particles(context);
  assert(pbase[0] == 0);
  assert(pbase[1] == 1);
  assert(pbase[2] == 1);
  assert(oh_context_transbound2(context, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_NORMAL_PRIMARY);

  oh_context_unbind_particle_accounting(context);
  oh_context_unbind_region_ids(context);
  oh_context_unbind_particles(context);
  assert(!context->particles);
  assert(!context->particle_buffer_bound);
  oh_context_destroy(context);
  free(nphgram);
}

static void
run_nondefault_init1_state_test(int n, int rank) {
  oh_context *context = 0;
  oh_context *borrowed_context = 0;
  int *sdid = 0;
  int *nphgram = 0;
  int *totalp = 0;
  int *rcounts = 0;
  int *scounts = 0;
  int *nbor = 0;
  int borrowed_sdid[2] = {99, 99};
  int *borrowed_sdid_ptr = borrowed_sdid;
  int *borrowed_nphgram = (int*)calloc((size_t)2*n, sizeof(int));
  int *borrowed_totalp = (int*)calloc(2, sizeof(int));
  int *borrowed_nphgram_ptr = borrowed_nphgram;
  int *borrowed_totalp_ptr = borrowed_totalp;
  int *borrowed_nbor = 0;
  int pcoord[3] = {n, 1, 1};
  struct S_mycommc mycommc;
  struct S_mycommf mycommf;
  int err;

  assert(borrowed_nphgram);
  assert(borrowed_totalp);
  err = oh_context_create(MPI_COMM_WORLD, &context);
  assert(err == MPI_SUCCESS);
  init1_state(context, &sdid, 2, 20, &nphgram, &totalp, &rcounts, &scounts,
              &mycommc, &mycommf, &nbor, pcoord, 0, 0, 0);

  assert(sdid);
  assert(sdid[0] == rank);
  assert(sdid[1] == -1);
  assert(context->n_of_species == 2);
  assert(context->max_fraction == 20);
  assert(context->n_of_particles_local == nphgram);
  assert(context->total_particles_next == totalp);
  assert(context->particle_accounting_bound);
  assert(rcounts == context->recv_counts);
  assert(scounts == context->send_counts);
  assert(nbor == context->neighbors[1]);
  assert(context->histogram_type != MPI_DATATYPE_NULL);
  assert(context->comm_list_type != MPI_DATATYPE_NULL);
  assert(mycommc.prime == MPI_COMM_NULL);
  assert(mycommf.prime == MPI_Comm_c2f(MPI_COMM_NULL));
  assert(oh_context_transbound1(context, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_NORMAL_PRIMARY);

  init1_state(context, &sdid, 1, 30, &nphgram, &totalp, &rcounts, &scounts,
              &mycommc, &mycommf, &nbor, pcoord, 0, 0, 0);
  assert(sdid[0] == rank);
  assert(sdid[1] == -1);
  assert(context->n_of_species == 1);
  assert(context->max_fraction == 30);
  assert(context->n_of_particles_local == nphgram);
  assert(context->total_particles_next == totalp);
  assert(rcounts == context->recv_counts);
  assert(scounts == context->send_counts);
  assert(nbor == context->neighbors[1]);
  assert(oh_context_transbound1(context, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_NORMAL_PRIMARY);
  oh_context_destroy(context);

  err = oh_context_create(MPI_COMM_WORLD, &borrowed_context);
  assert(err == MPI_SUCCESS);
  init1_state(borrowed_context, &borrowed_sdid_ptr, 1, 20,
              &borrowed_nphgram_ptr, &borrowed_totalp_ptr, 0, 0, 0, 0,
              &borrowed_nbor, pcoord, 0, 0, 0);
  assert(borrowed_sdid_ptr == borrowed_sdid);
  assert(borrowed_sdid[0] == rank);
  assert(borrowed_sdid[1] == -1);
  assert(borrowed_context->owns_region_id == 0);
  assert(borrowed_context->n_of_particles_local == borrowed_nphgram);
  assert(borrowed_context->total_particles_next == borrowed_totalp);
  assert(borrowed_context->n_of_particles_local_ownership ==
         OH_PARTICLES_BORROWED);
  assert(oh_context_transbound1(borrowed_context, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_NORMAL_PRIMARY);
  for (int i=0; i<2*n; i++) assert(borrowed_nphgram[i] == 0);
  for (int i=0; i<2; i++) assert(borrowed_totalp[i] == 0);

  borrowed_nphgram[0] = 11;
  borrowed_totalp[0] = 13;
  borrowed_nphgram_ptr = 0;
  borrowed_totalp_ptr = 0;
  init1_state(borrowed_context, &borrowed_sdid_ptr, 1, 25,
              &borrowed_nphgram_ptr, &borrowed_totalp_ptr, 0, 0, 0, 0,
              &borrowed_nbor, pcoord, 0, 0, 0);
  assert(borrowed_sdid_ptr == borrowed_sdid);
  assert(borrowed_nphgram[0] == 11);
  assert(borrowed_totalp[0] == 13);
  assert(borrowed_context->n_of_particles_local == borrowed_nphgram_ptr);
  assert(borrowed_context->total_particles_next == borrowed_totalp_ptr);
  assert(borrowed_context->n_of_particles_local != borrowed_nphgram);
  assert(borrowed_context->total_particles_next != borrowed_totalp);
  assert(borrowed_context->n_of_particles_local_ownership ==
         OH_PARTICLES_OWNED);
  assert(oh_context_transbound1(borrowed_context, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_NORMAL_PRIMARY);
  oh_context_destroy(borrowed_context);
  borrowed_sdid[0] = 123;
  borrowed_nphgram[0] = 7;
  borrowed_totalp[0] = 7;
  free(borrowed_nphgram);
  free(borrowed_totalp);
}

static void
run_nondefault_init1_rebalance_test(int n, int rank) {
  oh_context *context = 0;
  int *sdid = 0;
  int *nphgram = 0;
  int *totalp = 0;
  int *rcounts = 0;
  int *scounts = 0;
  int *nbor = 0;
  int pcoord[3] = {n, 1, 1};
  int err;
  int mode;
  int send_total = 0;
  int recv_total = 0;

  if (n != 2) return;

  err = oh_context_create(MPI_COMM_WORLD, &context);
  assert(err == MPI_SUCCESS);
  init1_state(context, &sdid, 1, 20, &nphgram, &totalp, &rcounts, &scounts,
              0, 0, &nbor, pcoord, 0, 0, 0);

  if (rank == 0) nphgram[0] = 8;
  mode = oh_context_transbound1(context, OH_MODE_NORMAL_PRIMARY, 0);
  assert(mode == OH_MODE_REBALANCE_SECONDARY);
  for (int i=0; i<2*n; i++) {
    send_total += scounts[i];
    recv_total += rcounts[i];
  }
  if (rank == 0) {
    assert(sdid[1] == -1);
    assert(send_total > 0);
  } else {
    assert(sdid[1] == 0);
    assert(recv_total > 0);
  }

  oh_context_destroy(context);
}

static void
run_nondefault_legacy_side_channel_isolation_test(int n, int rank) {
  oh_context *context = 0;
  int *legacy_nbor = 0;
  int *legacy_fidx = 0;
  int *legacy_fmem = 0;
  int *sdid = 0;
  int *nphgram = 0;
  int *totalp = 0;
  int *rcounts = 0;
  int *scounts = 0;
  int *nbor = 0;
  int pcoord[3] = {n, 1, 1};
  int mode;
  int err;

  if (n != 2) return;

  legacy_nbor = (int*)calloc(3 * OH_NEIGHBORS, sizeof(int));
  legacy_fidx = (int*)calloc((size_t)n + 1, sizeof(int));
  legacy_fmem = (int*)calloc((size_t)2 * n, sizeof(int));
  assert(legacy_nbor);
  assert(legacy_fidx);
  assert(legacy_fmem);
  oh1_neighbors(&legacy_nbor);
  oh1_families(&legacy_fidx, &legacy_fmem);
  for (int i=0; i<3*OH_NEIGHBORS; i++) legacy_nbor[i] = -7000 - i;
  for (int i=0; i<n+1; i++) legacy_fidx[i] = -8000 - i;
  for (int i=0; i<2*n; i++) legacy_fmem[i] = -9000 - i;

  err = oh_context_create(MPI_COMM_WORLD, &context);
  assert(err == MPI_SUCCESS);
  init1_state(context, &sdid, 1, 20, &nphgram, &totalp, &rcounts, &scounts,
              0, 0, &nbor, pcoord, 0, 0, 0);
  if (rank == 0) nphgram[0] = 8;
  mode = oh_context_transbound1(context, OH_MODE_NORMAL_PRIMARY, 0);
  assert(mode == OH_MODE_REBALANCE_SECONDARY);
  for (int i=0; i<3*OH_NEIGHBORS; i++) assert(legacy_nbor[i] == -7000 - i);
  for (int i=0; i<n+1; i++) assert(legacy_fidx[i] == -8000 - i);
  for (int i=0; i<2*n; i++) assert(legacy_fmem[i] == -9000 - i);

  mode = oh_context_transbound1(context, mode, 0);
  (void)mode;
  for (int i=0; i<3*OH_NEIGHBORS; i++) assert(legacy_nbor[i] == -7000 - i);
  for (int i=0; i<n+1; i++) assert(legacy_fidx[i] == -8000 - i);
  for (int i=0; i<2*n; i++) assert(legacy_fmem[i] == -9000 - i);

  oh_context_destroy(context);
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
run_injected_accounting_contract_test(int rank, int n, MPI_Datatype pic_type,
                                      const oh_particle_adapter *adapter) {
  oh_context *context = 0;
  struct pic_particle particles[8] = {{0}};
  struct pic_particle injected_particle = {0};
  struct pic_particle *copy;
  int *nphgram = (int*)calloc((size_t)2*n, sizeof(*nphgram));
  int totalp[2] = {0, 0};
  int pbase[3] = {0, 0, 0};
  int sdid[2] = {rank, -1};
  int *nphgram_ptr = nphgram;
  int *totalp_ptr = totalp;
  int *pbase_ptr = pbase;
  int err;

  assert(nphgram);
  err = oh_context_create(MPI_COMM_WORLD, &context);
  assert(err == MPI_SUCCESS);
  configure_level3_context_with_maxfrac(context, n, OH_DIM_X, 10000);
  set_custom_adapter(context, adapter);

  oh_context_bind_region_ids(context, sdid, OH_PARTICLES_BORROWED);
  oh_context_bind_particles(context, particles, 8, OH_PARTICLES_BORROWED);
  oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                      &pbase_ptr, OH_PARTICLES_BORROWED);
  oh_context_set_total_particles(context);
  assert(context->n_of_injections == 0);
  assert(context->injected_particles[0] == 0);
  assert(nphgram[rank] == 0);

  injected_particle.x = ((double)rank + 0.5) / (double)n;
  injected_particle.y = 0.5;
  injected_particle.z = 0.5;
  injected_particle.region = rank;
  injected_particle.species = 1;
  copy = (struct pic_particle*)oh_context_inject_particle_get(
    context, &injected_particle);
  assert(copy != &injected_particle);
  assert(context->n_of_injections == 1);
  assert(context->injected_particles[0] == 1);
  assert(nphgram[rank] == 1);

  oh_context_remove_injected_particle(context, copy);
  assert(context->n_of_injections == 1);
  assert(context->injected_particles[0] == 0);
  assert(nphgram[rank] == 0);
  assert(context->particle_adapter->get_region(
           context->particle_adapter, copy, 0) < 0);

  context->particle_adapter->set_region(context->particle_adapter, copy,
                                        rank, 0);
  oh_context_remap_injected_particle(context, copy);
  assert(context->n_of_injections == 1);
  assert(context->injected_particles[0] == 1);
  assert(nphgram[rank] == 1);

  oh_context_set_total_particles(context);
  assert(context->n_of_injections == 0);
  assert(context->injected_particles[0] == 0);
  assert(nphgram[rank] == 1);
  assert(pbase[0] == 0);
  assert(pbase[1] == 1);
  assert(pbase[2] == 1);

  oh_context_unbind_region_ids(context);
  oh_context_unbind_particle_accounting(context);
  oh_context_unbind_particles(context);
  oh_context_destroy(context);
  free(nphgram);
  (void)pic_type;
}

static void
run_particle_adapter_reset_rebind_test(int rank, int n, MPI_Datatype pic_type,
                                       const oh_particle_adapter *adapter) {
  oh_context *context = 0;
  oh_particle_adapter copied_adapter = *adapter;
  MPI_Datatype copied_type = MPI_DATATYPE_NULL;
  struct pic_particle custom_particle = {0};
  struct S_particle default_particle = {0};
  int err;

  err = oh_context_create(MPI_COMM_WORLD, &context);
  assert(err == MPI_SUCCESS);
  configure_level3_context(context, n, OH_DIM_X);
  err = oh_particle_adapter_make_byte_type(sizeof(custom_particle),
                                           &copied_type);
  assert(err == MPI_SUCCESS);
  copied_adapter.mpi_type = copied_type;
  set_custom_adapter(context, &copied_adapter);
  MPI_Type_free(&copied_type);
  assert(context->particle_adapter->mpi_type != MPI_DATATYPE_NULL);
  copied_adapter.region_offset = (size_t)-1;
  copied_adapter.species_offset = (size_t)-1;
  copied_adapter.map_to_neighbor = 0;
  copied_adapter.map_to_subdomain = 0;

  custom_particle.x = ((double)rank + 0.5) / (double)n;
  custom_particle.y = 0.5;
  custom_particle.z = 0.5;
  custom_particle.region = rank;
  custom_particle.species = 1;
  assert(context->particle_adapter->map_to_subdomain(
           context->particle_adapter, &custom_particle, 0) == rank);

  oh_context_set_particle_adapter(context, NULL);
  assert(!context->use_custom_particle_adapter);
  assert(context->particle_adapter);
  assert(context->particle_adapter->user_data == context);
  assert(context->particle_adapter->map_to_subdomain);
  assert(context->particle_adapter->map_to_neighbor);

  default_particle.x = ((double)rank + 0.5) / (double)n;
#if OH_DIMENSION >= 2
  default_particle.y = 0.5;
#endif
#if OH_DIMENSION >= 3
  default_particle.z = 0.5;
#endif
  assert(context->particle_adapter->map_to_subdomain(
           context->particle_adapter, &default_particle, 0) == rank);

  oh_context_set_particle_mpi_type(context, MPI_DATATYPE_NULL);
  assert(!context->use_custom_particle_adapter);
  assert(context->particle_adapter);
  assert(context->particle_adapter->user_data == context);
  assert(context->particle_adapter->map_to_subdomain);
  assert(context->particle_adapter->map_to_neighbor);
  assert(context->particle_adapter->map_to_subdomain(
           context->particle_adapter, &default_particle, 0) == rank);

  oh_context_destroy(context);
  (void)pic_type;
}

static void
run_position_only_adapter_context_mapping_test(
    int rank, int n, const oh_particle_adapter *base_adapter) {
  oh_context *context_x = 0;
  oh_context *context_y = 0;
  oh_particle_adapter position_adapter = *base_adapter;
  struct pic_particle particle_x = {0};
  struct pic_particle particle_y = {0};
  int err;

  err = oh_context_create(MPI_COMM_WORLD, &context_x);
  assert(err == MPI_SUCCESS);
  err = oh_context_create(MPI_COMM_WORLD, &context_y);
  assert(err == MPI_SUCCESS);
  configure_level3_context(context_x, n, OH_DIM_X);
#if OH_DIMENSION >= 2
  configure_level3_context(context_y, n, OH_DIM_Y);
#else
  configure_level3_context(context_y, n, OH_DIM_X);
#endif

  position_adapter.map_to_neighbor = 0;
  position_adapter.map_to_subdomain = 0;
  oh3_particle_adapter_use_position_fields(&position_adapter,
                                           offsetof(struct pic_particle, x),
                                           offsetof(struct pic_particle, y),
                                           offsetof(struct pic_particle, z));
  set_custom_adapter(context_x, &position_adapter);
  set_custom_adapter(context_y, &position_adapter);

  particle_x.x = ((double)rank + 0.5) / (double)n;
  particle_x.y = 0.5;
  particle_x.z = 0.5;
  particle_y.x = 0.5;
  particle_y.y = 0.5;
  particle_y.z = 0.5;
#if OH_DIMENSION >= 2
  particle_y.y = ((double)rank + 0.5) / (double)n;
#else
  particle_y.x = ((double)rank + 0.5) / (double)n;
#endif

  assert(context_x->particle_adapter->map_to_subdomain(
           context_x->particle_adapter, &particle_x, 0) == rank);
  assert(context_y->particle_adapter->map_to_subdomain(
           context_y->particle_adapter, &particle_y, 0) == rank);
  assert(context_x->particle_adapter->map_to_neighbor(
           context_x->particle_adapter, &particle_x, 0) == rank);
  assert(context_y->particle_adapter->map_to_neighbor(
           context_y->particle_adapter, &particle_y, 0) == rank);

  oh_context_destroy(context_x);
  oh_context_destroy(context_y);
}

static void
run_weighted_load_rebalance_test(int rank, int n, MPI_Datatype pic_type,
                                 const oh_particle_adapter *adapter) {
  oh_context *context = 0;
  struct pic_particle particles[16] = {{0}};
  int nphgram[6] = {0, 0, 0, 0, 0, 0};
  int totalp[2] = {0, 0};
  int pbase[3] = {0, 0, 0};
  int sdid[2] = {rank, -1};
  int copied_sdid[2] = {0, 0};
  int *nphgram_ptr = nphgram;
  int *totalp_ptr = totalp;
  int *pbase_ptr = pbase;
  double weights[3] = {4.0, 1.0, 1.0};
  int err;

  if (n != 2 && n != 3) return;

  err = oh_context_create(MPI_COMM_WORLD, &context);
  assert(err == MPI_SUCCESS);
  configure_level3_context(context, n, OH_DIM_X);
  set_custom_adapter(context, adapter);

  if (n == 2) {
    particles[0].x = ((double)rank + 0.5) / (double)n;
    particles[0].y = 0.5;
    particles[0].z = 0.5;
    particles[0].region = rank;
    particles[0].species = 1;
    nphgram[rank] = 1;
  } else if (rank == 0) {
    for (int i=0; i<3; i++) {
      particles[i].x = ((double)i + 0.5) / 9.0;
      particles[i].y = 0.5;
      particles[i].z = 0.5;
      particles[i].region = 0;
      particles[i].species = 1;
    }
    nphgram[0] = 3;
  }

  oh_context_bind_region_ids(context, sdid, OH_PARTICLES_BORROWED);
  oh_context_bind_particles(context, particles, 16, OH_PARTICLES_BORROWED);
  oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                      &pbase_ptr, OH_PARTICLES_BORROWED);
  oh_context_set_region_weights(context, weights);
  assert(context->weighted_load_balancing);
  assert(!try_stable1_state(context, OH_MODE_NORMAL_SECONDARY, 1, 0));
  oh_context_set_total_particles(context);

  assert(oh_context_transbound3(context, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_REBALANCE_SECONDARY);
  oh_context_get_region_ids(context, copied_sdid);
  if (n == 2) {
    assert(context->total_load == 5.0);
    assert(context->n_of_local_load_max == 3.0);
    if (rank == 0) {
      assert(copied_sdid[1] == -1);
      assert(context->nodes[rank].parentid == -1);
      assert(context->nodes[rank].get.sec == 0);
    } else {
      assert(copied_sdid[1] == 0);
      assert(context->nodes[rank].parentid == 0);
      assert(context->nodes[rank].get.sec == 1);
      assert(pbase[0] == 0);
      assert(pbase[1] == 1);
      assert(pbase[2] == 2);
    }
  } else {
    const double target_load = context->total_load / (double)n;
    for (int i=0; i<n; i++)
      assert(context->total_load_global[i] >= target_load);
    if (rank == 0) {
      assert(copied_sdid[1] == -1);
      assert(pbase[0] == 0);
      assert(pbase[1] == 1);
      assert(pbase[2] == 1);
      assert(totalp[0] == 1);
      assert(totalp[1] == 0);
    } else {
      assert(copied_sdid[1] == 0);
      assert(pbase[0] == 0);
      assert(pbase[1] == 0);
      assert(pbase[2] == 1);
      assert(totalp[0] == 0);
      assert(totalp[1] == 1);
    }
  }
  assert(oh_context_transbound3(context, OH_MODE_REBALANCE_SECONDARY, 0) ==
         OH_MODE_NORMAL_PRIMARY);
  oh_context_get_region_ids(context, copied_sdid);
  assert(copied_sdid[1] == -1);

  oh_context_unbind_region_ids(context);
  oh_context_unbind_particle_accounting(context);
  oh_context_unbind_particles(context);
  oh_context_destroy(context);
  (void)pic_type;
}

static void
run_weighted_load_level1_level2_api_test(int rank, int n,
                                         MPI_Datatype pic_type,
                                         const oh_particle_adapter *adapter) {
  oh_context *level1 = 0;
  oh_context *level2 = 0;
  struct pic_particle particles[4] = {{0}};
  int nphgram1[4] = {0, 0, 0, 0};
  int nphgram2[4] = {0, 0, 0, 0};
  int totalp1[2] = {0, 0};
  int totalp2[2] = {0, 0};
  int pbase1[3] = {0, 0, 0};
  int pbase2[3] = {0, 0, 0};
  int sdid1[2] = {rank, -1};
  int sdid2[2] = {rank, -1};
  int copied_sdid[2] = {0, 0};
  int *nphgram1_ptr = nphgram1;
  int *nphgram2_ptr = nphgram2;
  int *totalp1_ptr = totalp1;
  int *totalp2_ptr = totalp2;
  int *pbase1_ptr = pbase1;
  int *pbase2_ptr = pbase2;
  double weights[2] = {4.0, 1.0};
  int err;

  if (n != 2) return;

  err = oh_context_create(MPI_COMM_WORLD, &level1);
  assert(err == MPI_SUCCESS);
  configure_level3_context(level1, n, OH_DIM_X);
  nphgram1[rank] = 1;
  oh_context_bind_region_ids(level1, sdid1, OH_PARTICLES_BORROWED);
  oh_context_bind_particle_accounting(level1, &nphgram1_ptr, &totalp1_ptr,
                                      &pbase1_ptr, OH_PARTICLES_BORROWED);
  oh_context_set_region_weights(level1, weights);
  oh_context_set_total_particles(level1);
  assert(oh_context_transbound1(level1, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_REBALANCE_SECONDARY);
  assert(level1->total_load == 5.0);
  assert(level1->n_of_local_load_max == 3.0);
  oh_context_get_region_ids(level1, copied_sdid);
  if (rank == 0) {
    assert(copied_sdid[1] == -1);
    assert(level1->nodes[rank].get.sec == 0);
  } else {
    assert(copied_sdid[1] == 0);
    assert(level1->nodes[rank].parentid == 0);
    assert(level1->nodes[rank].get.sec == 1);
  }
  oh_context_destroy(level1);

  err = oh_context_create(MPI_COMM_WORLD, &level2);
  assert(err == MPI_SUCCESS);
  configure_level3_context(level2, n, OH_DIM_X);
  set_custom_adapter(level2, adapter);
  particles[0].x = ((double)rank + 0.5) / (double)n;
  particles[0].y = 0.5;
  particles[0].z = 0.5;
  particles[0].region = rank;
  particles[0].species = 1;
  nphgram2[rank] = 1;
  oh_context_bind_region_ids(level2, sdid2, OH_PARTICLES_BORROWED);
  oh_context_bind_particles(level2, particles, 4, OH_PARTICLES_BORROWED);
  oh_context_bind_particle_accounting(level2, &nphgram2_ptr, &totalp2_ptr,
                                      &pbase2_ptr, OH_PARTICLES_BORROWED);
  oh_context_set_region_weights(level2, weights);
  oh_context_set_total_particles(level2);
  assert(oh_context_transbound2(level2, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_REBALANCE_SECONDARY);
  assert(level2->total_load == 5.0);
  assert(level2->n_of_local_load_max == 3.0);
  oh_context_get_region_ids(level2, copied_sdid);
  if (rank == 0) {
    assert(copied_sdid[1] == -1);
  } else {
    assert(copied_sdid[1] == 0);
    assert(pbase2[2] == 2);
  }

  oh_context_unbind_region_ids(level2);
  oh_context_unbind_particle_accounting(level2);
  oh_context_unbind_particles(level2);
  oh_context_destroy(level2);
  (void)pic_type;
}

static void
run_callback_only_adapter_transbound_test(int rank, int n, int level3) {
  oh_context *context = 0;
  MPI_Datatype particle_type = MPI_DATATYPE_NULL;
  oh_particle_adapter adapter;
  struct callback_particle particles[4] = {{0}};
  int nphgram[4] = {0, 0, 0, 0};
  int totalp[2] = {0, 0};
  int pbase[3] = {0, 0, 0};
  int sdid[2] = {rank, -1};
  int *nphgram_ptr = nphgram;
  int *totalp_ptr = totalp;
  int *pbase_ptr = pbase;
  int destination;
  int mode;
  int err;

  if (n != 2) return;

  err = oh_context_create(MPI_COMM_WORLD, &context);
  assert(err == MPI_SUCCESS);
  configure_level3_context_with_maxfrac(context, n, OH_DIM_X, 10000);
  err = oh_particle_adapter_make_byte_type(sizeof(struct callback_particle),
                                           &particle_type);
  assert(err == MPI_SUCCESS);
  adapter = oh_default_particle_adapter(particle_type);
  adapter.stride = sizeof(struct callback_particle);
  adapter.get_region = callback_particle_get_region;
  adapter.set_region = callback_particle_set_region;
  adapter.get_species = callback_particle_get_species;
  adapter.map_to_neighbor = callback_particle_map_to_neighbor;
  adapter.map_to_subdomain = callback_particle_map_to_subdomain;
  oh_particle_adapter_set_species_base(&adapter, 1);
  assert(oh_particle_adapter_validate(&adapter));
  oh_context_set_particle_adapter(context, &adapter);
  MPI_Type_free(&particle_type);

  assert(context->use_custom_particle_adapter);
  assert(context->particle_adapter != &adapter);
  assert(context->particle_adapter->stride ==
         sizeof(struct callback_particle));
  assert(context->particle_adapter->get_region ==
         callback_particle_get_region);
  assert(context->particle_adapter->set_region ==
         callback_particle_set_region);
  assert(context->particle_adapter->get_species ==
         callback_particle_get_species);
  assert(context->particle_adapter->map_to_subdomain ==
         callback_particle_map_to_subdomain);
  assert(context->particle_adapter->map_to_neighbor ==
         callback_particle_map_to_neighbor);
  assert(context->particle_adapter->species_base == 1);

  destination = 1 - rank;
  particles[0].marker = 1000.0 + (double)rank;
  particles[0].logical_region = destination;
  particles[0].species_id = 1;
  nphgram[destination] = 1;

  oh_context_bind_region_ids(context, sdid, OH_PARTICLES_BORROWED);
  oh_context_bind_particles(context, particles, 4, OH_PARTICLES_BORROWED);
  oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                      &pbase_ptr, OH_PARTICLES_BORROWED);
  oh_context_set_total_particles(context);
  mode = level3 ?
    oh_context_transbound3(context, OH_MODE_NORMAL_PRIMARY, 0) :
    oh_context_transbound2(context, OH_MODE_NORMAL_PRIMARY, 0);
  assert(mode == OH_MODE_NORMAL_PRIMARY);
  assert(pbase[0] == 0);
  assert(pbase[1] == 1);
  assert(pbase[2] == 1);
  assert(particles[0].logical_region == rank);
  assert(particles[0].species_id == 1);
  assert(particles[0].marker == 1000.0 + (double)(1 - rank));

  oh_context_unbind_region_ids(context);
  oh_context_unbind_particle_accounting(context);
  oh_context_unbind_particles(context);
  oh_context_destroy(context);
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

  if (argc > 1 && strcmp(argv[1], "destroy-border-after-finalize") == 0) {
    run_border_exchange_destroy_after_finalize_test(n);
    return 0;
  }

  run_nondefault_init1_state_test(n, rank);
  run_nondefault_init1_rebalance_test(n, rank);
  run_nondefault_legacy_side_channel_isolation_test(n, rank);
  run_region_weights_context_isolation_test(n);
  run_context_create_invalid_comm_test();
  run_context_owned_comm_test(n, rank);
  run_context_reconfigure_particles_test(n);
  run_context_particle_mpi_type_ownership_test(n, rank);

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
  run_region_weight_copy_reset_test(context_x, n);
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

  run_owned_particle_buffer_test(n, rank, &adapter);
  run_localized_secondary_test(rank, n, pic_type, &adapter);
  run_position_fields_preserve_region_routing_test(rank, n, pic_type,
                                                   &adapter);
  run_injected_position_routing_test(rank, n, pic_type, &adapter);
  run_injected_accounting_contract_test(rank, n, pic_type, &adapter);
  run_particle_adapter_reset_rebind_test(rank, n, pic_type, &adapter);
  run_position_only_adapter_context_mapping_test(rank, n, &adapter);
  run_weighted_load_rebalance_test(rank, n, pic_type, &adapter);
  run_weighted_load_level1_level2_api_test(rank, n, pic_type, &adapter);
  run_callback_only_adapter_transbound_test(rank, n, 0);
  run_callback_only_adapter_transbound_test(rank, n, 1);

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
  assert(context_x->stats_mode == 0);
  assert(oh_context_transbound1(context_x, OH_MODE_NORMAL_PRIMARY, 1) ==
         OH_MODE_NORMAL_PRIMARY);
  assert(oh_context_transbound2(context_x, OH_MODE_NORMAL_PRIMARY, 1) ==
         OH_MODE_NORMAL_PRIMARY);
  assert(oh_context_transbound3(context_x, OH_MODE_NORMAL_PRIMARY, 1) ==
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
