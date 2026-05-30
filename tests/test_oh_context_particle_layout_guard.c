#include <assert.h>
#include <limits.h>
#include <math.h>
#include <mpi.h>
#include <string.h>

#include "oh_context.h"
#include "oh_part.h"
#include "oh_particle_adapter.h"
#include "oh_particle_ownership.h"
#include "ohhelp1_internal.h"
#include "oh_context_internal.h"
#include "ohhelp3.h"

struct guard_particle {
  int region;
  int species;
};

static oh_particle_region_t
guard_get_region(const oh_particle_adapter *adapter, const void *particle,
                 int primary_or_secondary) {
  const struct guard_particle *p = (const struct guard_particle*)particle;

  (void)adapter;
  (void)primary_or_secondary;
  return p->region;
}

static void
guard_set_region(const oh_particle_adapter *adapter, void *particle,
                 oh_particle_region_t region, int primary_or_secondary) {
  struct guard_particle *p = (struct guard_particle*)particle;

  (void)adapter;
  (void)primary_or_secondary;
  p->region = (int)region;
}

static int
guard_get_species(const oh_particle_adapter *adapter, const void *particle) {
  const struct guard_particle *p = (const struct guard_particle*)particle;

  (void)adapter;
  return p->species;
}

static oh_particle_region_t
guard_map_invalid_destination(const oh_particle_adapter *adapter,
                              void *particle,
                              int primary_or_secondary) {
  const int *n_of_nodes = (const int*)adapter->user_data;

  (void)particle;
  (void)primary_or_secondary;
  return *n_of_nodes;
}

int
main(int argc, char **argv) {
  oh_context *context = 0;
  int err;

  MPI_Init(&argc, &argv);
  err = oh_context_create(MPI_COMM_WORLD, &context);
  assert(err == MPI_SUCCESS);
  assert(context);

  if (argc > 1 && strcmp(argv[1], "unconfigured-bind") == 0)
    oh_context_bind_particles(context, 0, 0, OH_PARTICLES_BORROWED);
  if (argc > 1 && strcmp(argv[1], "species-index-overflow") == 0) {
    int n_of_nodes = 0;

    MPI_Comm_size(MPI_COMM_WORLD, &n_of_nodes);
    assert(n_of_nodes == 1);
    oh_context_configure_particles(context, INT_MAX, 0);
  }
  if (argc > 1 && strcmp(argv[1], "capacity-add-overflow") == 0) {
    int n_of_nodes = 0;

    MPI_Comm_size(MPI_COMM_WORLD, &n_of_nodes);
    assert(n_of_nodes == 1);
    (void)oh_context_max_local_particles_for_capacity(context, LLONG_MAX, 0,
                                                      1);
  }
  if (argc > 1 && strcmp(argv[1], "configure-zero-species") == 0)
    oh_context_configure_particles(context, 0, 0);
  if (argc > 1 && strcmp(argv[1], "configure-negative-maxfrac") == 0)
    oh_context_configure_particles(context, 1, -1);

  oh_context_configure_particles(context, 1, 0);

  if (argc > 1 && strcmp(argv[1], "zero-weight") == 0) {
    double weights[1] = {0.0};

    oh_context_set_region_weights(context, weights);
  }
  if (argc > 1 && strcmp(argv[1], "negative-region-weight") == 0) {
    double weights[1] = {-1.0};

    oh_context_set_region_weights(context, weights);
  }
  if (argc > 1 && strcmp(argv[1], "nan-region-weight") == 0) {
    double weights[1] = {NAN};

    oh_context_set_region_weights(context, weights);
  }
  if (argc > 1 && strcmp(argv[1], "inf-region-weight") == 0) {
    double weights[1] = {HUGE_VAL};

    oh_context_set_region_weights(context, weights);
  }

  if (argc > 1 && strcmp(argv[1], "owned-region-nonnull") == 0) {
    int sdid[2] = {0, -1};
    oh_context_bind_region_ids(context, sdid, OH_PARTICLES_OWNED);
  }
  if (argc > 1 && strcmp(argv[1], "borrowed-region-null") == 0)
    oh_context_bind_region_ids(context, 0, OH_PARTICLES_BORROWED);
  if (argc > 1 && strcmp(argv[1], "invalid-region-ownership") == 0) {
    int sdid[2] = {0, -1};
    oh_context_bind_region_ids(context, sdid, 99);
  }
  if (argc > 1 && strcmp(argv[1], "get-region-null") == 0)
    oh_context_get_region_ids(context, 0);
  if (argc > 1 && strcmp(argv[1], "owned-particle-nonnull") == 0) {
    int particle = 0;
    oh_context_bind_particles(context, &particle, 1, OH_PARTICLES_OWNED);
  }
  if (argc > 1 && strcmp(argv[1], "invalid-particle-ownership") == 0) {
    int particle = 0;
    oh_context_bind_particles(context, &particle, 1, 99);
  }
  if (argc > 1 && strcmp(argv[1], "borrowed-particle-null") == 0)
    oh_context_bind_particles(context, 0, 1, OH_PARTICLES_BORROWED);
  if (argc > 1 && strcmp(argv[1], "owned-accounting-nonnull") == 0) {
    int nphgram[2] = {0, 0};
    int totalp[2] = {0, 0};
    int pbase[3] = {0, 0, 0};
    int *nphgram_ptr = nphgram;
    int *totalp_ptr = totalp;
    int *pbase_ptr = pbase;
    oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                        &pbase_ptr, OH_PARTICLES_OWNED);
  }
  if (argc > 1 && strcmp(argv[1], "invalid-accounting-ownership") == 0) {
    int nphgram[2] = {0, 0};
    int totalp[2] = {0, 0};
    int pbase[3] = {0, 0, 0};
    int *nphgram_ptr = nphgram;
    int *totalp_ptr = totalp;
    int *pbase_ptr = pbase;
    oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                        &pbase_ptr, 99);
  }
  if (argc > 1 && strcmp(argv[1], "accounting-null-nphgram-slot") == 0) {
    int totalp[2] = {0, 0};
    int pbase[3] = {0, 0, 0};
    int *totalp_ptr = totalp;
    int *pbase_ptr = pbase;
    oh_context_bind_particle_accounting(context, 0, &totalp_ptr, &pbase_ptr,
                                        OH_PARTICLES_BORROWED);
  }
  if (argc > 1 && strcmp(argv[1], "accounting-null-totalp-slot") == 0) {
    int nphgram[2] = {0, 0};
    int pbase[3] = {0, 0, 0};
    int *nphgram_ptr = nphgram;
    int *pbase_ptr = pbase;
    oh_context_bind_particle_accounting(context, &nphgram_ptr, 0, &pbase_ptr,
                                        OH_PARTICLES_BORROWED);
  }
  if (argc > 1 && strcmp(argv[1], "borrowed-accounting-null") == 0) {
    int *nphgram_ptr = 0;
    int *totalp_ptr = 0;
    int *pbase_ptr = 0;
    oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                        &pbase_ptr, OH_PARTICLES_BORROWED);
  }
  if (argc > 1 && strcmp(argv[1], "borrowed-accounting-null-pbase") == 0) {
    int nphgram[2] = {0, 0};
    int totalp[2] = {0, 0};
    int *nphgram_ptr = nphgram;
    int *totalp_ptr = totalp;
    int *pbase_ptr = 0;
    oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                        &pbase_ptr, OH_PARTICLES_BORROWED);
  }
  if (argc > 1 && strcmp(argv[1], "owned-accounting-nonnull-pbase") == 0) {
    int pbase[3] = {0, 0, 0};
    int *nphgram_ptr = 0;
    int *totalp_ptr = 0;
    int *pbase_ptr = pbase;
    oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                        &pbase_ptr, OH_PARTICLES_OWNED);
  }
  if (argc > 1 && strcmp(argv[1], "accounting-null-pbase-slot") == 0) {
    int nphgram[2] = {0, 0};
    int totalp[2] = {0, 0};
    int *nphgram_ptr = nphgram;
    int *totalp_ptr = totalp;
    oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                        0, OH_PARTICLES_BORROWED);
  }
  if (argc > 1 && strcmp(argv[1], "set-total-unbound") == 0)
    oh_context_set_total_particles(context);
  if (argc > 1 && strcmp(argv[1], "grid-size-null") == 0)
    oh_context_grid_size(context, 0);
  if (argc > 1 && strcmp(argv[1], "level3-field-unconfigured") == 0) {
    double field[1] = {0.0};

    oh_context_configure_level3(context, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    oh_context_bcast_field(context, field, field, 0);
  }
  if (argc > 1 && strcmp(argv[1], "level3-exchange-unconfigured") == 0) {
    double field[1] = {0.0};

    oh_context_configure_level3(context, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    oh_context_exchange_borders(context, field, field, 0, 0);
  }
  if (argc > 1 && strcmp(argv[1], "level3-missing-ctypes") == 0) {
    int pcoord[OH_DIMENSION] = {0};
    int scoord[OH_DIMENSION][2] = {{0}};
    int bcond[OH_DIMENSION][2] = {{0}};
    int ftypes[2][OH_FTYPE_N] = {{0}};
    int cfields[2] = {0, -1};
    int fsizes[1][OH_DIMENSION][2] = {{{0}}};
    int d;

    for (d=0; d<OH_DIMENSION; d++) {
      pcoord[d] = 1;
      scoord[d][OH_UPPER] = 1;
    }
    ftypes[0][OH_FTYPE_ES] = 1;
    oh_context_configure_level3(context, pcoord, 0, &scoord[0][0], 1,
                                &bcond[0][0], 0, &ftypes[0][0], cfields, 0,
                                &fsizes[0][0][0]);
  }
  if (argc > 1 && strcmp(argv[1], "inject-index-overflow") == 0) {
    struct S_particle particles[1] = {{0}};
    struct S_particle injected = {0};

    oh_context_bind_particles(context, particles, 1, OH_PARTICLES_BORROWED);
    context->total_parts = INT_MAX;
    context->n_of_injections = 1;
    context->n_of_local_particles_limit = INT_MAX;
    oh_context_inject_particle(context, &injected);
  }
  if (argc > 1 &&
      (strcmp(argv[1], "remap-finalized-injected-copy") == 0 ||
       strcmp(argv[1], "remove-finalized-injected-copy") == 0 ||
       strcmp(argv[1], "remap-null-injected-pointer") == 0 ||
       strcmp(argv[1], "remove-null-injected-pointer") == 0 ||
       strcmp(argv[1], "remap-interior-injected-pointer") == 0 ||
       strcmp(argv[1], "remove-interior-injected-pointer") == 0 ||
       strcmp(argv[1], "remap-active-particle") == 0 ||
       strcmp(argv[1], "remove-active-particle") == 0 ||
       strcmp(argv[1], "remap-original-injected-source") == 0 ||
       strcmp(argv[1], "remove-original-injected-source") == 0)) {
    MPI_Datatype type = MPI_DATATYPE_NULL;
    oh_particle_adapter adapter;
    struct guard_particle particles[2] = {{0, 0}, {0, 0}};
    struct guard_particle injected = {0, 0};
    struct guard_particle *copy;
    int sdid[2] = {0, -1};
    int nphgram[2] = {0, 0};
    int totalp[2] = {0, 0};
    int pbase[3] = {0, 0, 0};
    int *nphgram_ptr = nphgram;
    int *totalp_ptr = totalp;
    int *pbase_ptr = pbase;
    int n_of_nodes = 0;

    MPI_Comm_size(MPI_COMM_WORLD, &n_of_nodes);
    assert(n_of_nodes == 1);
    nphgram[0] = 1;
    err = oh_particle_adapter_make_byte_type(sizeof(struct guard_particle),
                                             &type);
    assert(err == MPI_SUCCESS);
    adapter = oh_default_particle_adapter(type);
    adapter.stride = sizeof(struct guard_particle);
    adapter.get_region = guard_get_region;
    adapter.set_region = guard_set_region;
    adapter.get_species = guard_get_species;
    oh_context_set_particle_adapter(context, &adapter);
    oh_context_bind_region_ids(context, sdid, OH_PARTICLES_BORROWED);
    oh_context_bind_particles(context, particles, 2, OH_PARTICLES_BORROWED);
    oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                        &pbase_ptr, OH_PARTICLES_BORROWED);
    oh_context_set_total_particles(context);
    copy = (struct guard_particle*)oh_context_inject_particle_get(context,
                                                                  &injected);
    if (strcmp(argv[1], "remap-null-injected-pointer") == 0)
      oh_context_remap_injected_particle(context, 0);
    if (strcmp(argv[1], "remove-null-injected-pointer") == 0)
      oh_context_remove_injected_particle(context, 0);
    if (strcmp(argv[1], "remap-interior-injected-pointer") == 0)
      oh_context_remap_injected_particle(context, (char*)copy + 1);
    if (strcmp(argv[1], "remove-interior-injected-pointer") == 0)
      oh_context_remove_injected_particle(context, (char*)copy + 1);
    if (strcmp(argv[1], "remap-active-particle") == 0)
      oh_context_remap_injected_particle(context, &particles[0]);
    if (strcmp(argv[1], "remove-active-particle") == 0)
      oh_context_remove_injected_particle(context, &particles[0]);
    if (strcmp(argv[1], "remap-original-injected-source") == 0)
      oh_context_remap_injected_particle(context, &injected);
    if (strcmp(argv[1], "remove-original-injected-source") == 0)
      oh_context_remove_injected_particle(context, &injected);
    oh_context_set_total_particles(context);
    if (strcmp(argv[1], "remove-finalized-injected-copy") == 0)
      oh_context_remove_injected_particle(context, copy);
    else
      oh_context_remap_injected_particle(context, copy);
  }
  if (argc > 1 &&
      (strcmp(argv[1], "active-invalid-destination-transbound2") == 0 ||
       strcmp(argv[1], "active-invalid-destination-transbound3") == 0)) {
    MPI_Datatype type = MPI_DATATYPE_NULL;
    oh_particle_adapter adapter;
    struct guard_particle particles[1] = {{0, 0}};
    int n_of_nodes = 0;
    int sdid[2] = {0, -1};
    int nphgram[2] = {1, 0};
    int totalp[2] = {0, 0};
    int pbase[3] = {0, 0, 0};
    int *nphgram_ptr = nphgram;
    int *totalp_ptr = totalp;
    int *pbase_ptr = pbase;
    int level3 = strcmp(argv[1],
                        "active-invalid-destination-transbound3") == 0;

    MPI_Comm_size(MPI_COMM_WORLD, &n_of_nodes);
    assert(n_of_nodes == 1);
    err = oh_particle_adapter_make_byte_type(sizeof(struct guard_particle),
                                             &type);
    assert(err == MPI_SUCCESS);
    adapter = oh_default_particle_adapter(type);
    adapter.stride = sizeof(struct guard_particle);
    adapter.user_data = &n_of_nodes;
    adapter.get_region = guard_get_region;
    adapter.set_region = guard_set_region;
    adapter.get_species = guard_get_species;
    adapter.map_to_subdomain = guard_map_invalid_destination;
    oh_context_set_particle_adapter(context, &adapter);
    oh_context_bind_region_ids(context, sdid, OH_PARTICLES_BORROWED);
    oh_context_bind_particles(context, particles, 1, OH_PARTICLES_BORROWED);
    oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                        &pbase_ptr, OH_PARTICLES_BORROWED);
    oh_context_set_total_particles(context);
    if (level3)
      oh_context_transbound3(context, OH_MODE_NORMAL_PRIMARY, 0);
    else
      oh_context_transbound2(context, OH_MODE_NORMAL_PRIMARY, 0);
  }
  if (argc > 1 && strcmp(argv[1], "invalid-destination") == 0) {
    MPI_Datatype type = MPI_DATATYPE_NULL;
    oh_particle_adapter adapter;
    struct guard_particle particles[1] = {{0, 0}};
    struct guard_particle injected = {0, 0};
    int n_of_nodes = 0;
    int sdid[2] = {0, -1};
    int nphgram[2] = {0, 0};
    int totalp[2] = {0, 0};
    int pbase[3] = {0, 0, 0};
    int *nphgram_ptr = nphgram;
    int *totalp_ptr = totalp;
    int *pbase_ptr = pbase;

    MPI_Comm_size(MPI_COMM_WORLD, &n_of_nodes);
    assert(n_of_nodes == 1);
    err = oh_particle_adapter_make_byte_type(sizeof(struct guard_particle),
                                             &type);
    assert(err == MPI_SUCCESS);
    adapter = oh_default_particle_adapter(type);
    adapter.stride = sizeof(struct guard_particle);
    adapter.user_data = &n_of_nodes;
    adapter.get_region = guard_get_region;
    adapter.set_region = guard_set_region;
    adapter.get_species = guard_get_species;
    adapter.map_to_subdomain = guard_map_invalid_destination;
    oh_context_set_particle_adapter(context, &adapter);
    oh_context_bind_region_ids(context, sdid, OH_PARTICLES_BORROWED);
    oh_context_bind_particles(context, particles, 1, OH_PARTICLES_BORROWED);
    oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                        &pbase_ptr, OH_PARTICLES_BORROWED);
    oh_context_set_total_particles(context);
    oh_context_inject_particle(context, &injected);
    oh_context_unbind_particle_accounting(context);
    oh_context_unbind_particles(context);
    oh_context_unbind_region_ids(context);
    MPI_Type_free(&type);
    oh_context_destroy(context);
    MPI_Finalize();
    return 0;
  }
  if (argc > 1 && strcmp(argv[1], "invalid-species") == 0) {
    MPI_Datatype type = MPI_DATATYPE_NULL;
    oh_particle_adapter adapter;
    struct guard_particle particles[1] = {{0, 0}};
    struct guard_particle injected = {0, 2};
    int sdid[2] = {0, -1};
    int nphgram[2] = {0, 0};
    int totalp[2] = {0, 0};
    int pbase[3] = {0, 0, 0};
    int *nphgram_ptr = nphgram;
    int *totalp_ptr = totalp;
    int *pbase_ptr = pbase;
    int n_of_nodes = 0;

    MPI_Comm_size(MPI_COMM_WORLD, &n_of_nodes);
    assert(n_of_nodes == 1);
    err = oh_particle_adapter_make_byte_type(sizeof(struct guard_particle),
                                             &type);
    assert(err == MPI_SUCCESS);
    adapter = oh_default_particle_adapter(type);
    adapter.stride = sizeof(struct guard_particle);
    adapter.get_region = guard_get_region;
    adapter.set_region = guard_set_region;
    adapter.get_species = guard_get_species;
    oh_context_set_particle_adapter(context, &adapter);
    oh_context_bind_region_ids(context, sdid, OH_PARTICLES_BORROWED);
    oh_context_bind_particles(context, particles, 1, OH_PARTICLES_BORROWED);
    oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                        &pbase_ptr, OH_PARTICLES_BORROWED);
    oh_context_set_total_particles(context);
    oh_context_inject_particle(context, &injected);
  }

  oh_context_bind_particles(context, 0, 0, OH_PARTICLES_BORROWED);

  if (argc > 1 && strcmp(argv[1], "reconfigure-bound") == 0)
    oh_context_configure_particles(context, 1, 0);

  if (argc > 1 && strcmp(argv[1], "adapter") == 0)
    oh_context_set_particle_adapter(context, 0);
  if (argc > 1 && strcmp(argv[1], "type") == 0)
    oh_context_set_particle_mpi_type(context, MPI_DATATYPE_NULL);
  if (argc > 1 && strcmp(argv[1], "type-extent") == 0) {
    MPI_Datatype type = MPI_DATATYPE_NULL;

    err = oh_particle_adapter_make_byte_type(sizeof(struct S_particle) + 8,
                                             &type);
    assert(err == MPI_SUCCESS);
    oh_context_unbind_particles(context);
    oh_context_set_particle_mpi_type(context, type);
  }

  oh_context_unbind_particles(context);
  oh_context_set_particle_adapter(context, 0);
  oh_context_set_particle_mpi_type(context, MPI_DATATYPE_NULL);
  oh_context_destroy(context);
  MPI_Finalize();
  return 0;
}
