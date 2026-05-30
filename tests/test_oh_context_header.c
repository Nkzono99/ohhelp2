#include "oh_context.h"

int
main(void) {
  oh_context *context = 0;
  oh_context *owned_context = 0;
  int ierr;
  double *weights = 0;
  double size[3] = {0.0, 0.0, 0.0};
  void *particle = 0;
  void *injected;
  void *primary = 0;
  void *secondary = 0;
  int *nphgram = 0;
  int *totalp = 0;
  int *pbase = 0;
  int *owned_sdid = 0;
  int borrowed_sdid[2] = {0, -1};
  int copied_sdid[2] = {0, 0};
  int capacity;
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;

  oh_context_set_particle_mpi_type(context, MPI_DATATYPE_NULL);
  oh_context_set_particle_adapter(context, 0);
  particle = oh_context_bind_particles(context, particle, 0,
                                       OH_PARTICLES_OWNED);
  oh_context_unbind_particles(context);
  oh_context_bind_particle_accounting(context, &nphgram, &totalp, &pbase,
                                      OH_PARTICLES_OWNED);
  oh_context_unbind_particle_accounting(context);
  oh_context_set_region_weights(context, weights);
  oh_context_set_region_weights_n(context, weights, 1);
  (void)oh_context_transbound1(context, OH_MODE_NORMAL_PRIMARY, 0);
  (void)oh_context_transbound2(context, OH_MODE_NORMAL_PRIMARY, 0);
  (void)oh_context_transbound3(context, OH_MODE_NORMAL_PRIMARY, 0);
  (void)OH_MODE_NORMAL_SECONDARY;
  (void)OH_MODE_REBALANCE_SECONDARY;
  (void)OH_MODE_ANY_PRIMARY;
  (void)OH_MODE_ANY_SECONDARY;
  oh_context_broadcast(context, primary, secondary, 0, 0, MPI_DOUBLE,
                       MPI_DOUBLE);
  oh_context_all_reduce(context, primary, secondary, 0, 0, MPI_DOUBLE,
                        MPI_DOUBLE, MPI_SUM, MPI_SUM);
  oh_context_reduce(context, primary, secondary, 0, 0, MPI_DOUBLE,
                    MPI_DOUBLE, MPI_SUM, MPI_SUM);
  oh_context_set_total_particles(context);
  injected = oh_context_inject_particle_get(context, particle);
  oh_context_remap_injected_particle(context, injected);
  oh_context_remove_injected_particle(context, injected);
  oh_context_grid_size(context, size);
  (void)oh_context_map_particle_to_neighbor(context, &x, &y, &z, 0);
  (void)oh_context_map_particle_to_subdomain(context, x, y, z);
  oh_context_bcast_field(context, primary, secondary, 0);
  oh_context_reduce_field(context, primary, secondary, 0);
  oh_context_allreduce_field(context, primary, secondary, 0);
  oh_context_exchange_borders(context, primary, secondary, 0, 0);
  ierr = oh_context_create(MPI_COMM_WORLD, &owned_context);
  (void)ierr;
  oh_context_configure_particles(owned_context, 1, 20);
  owned_sdid = oh_context_bind_region_ids(owned_context, 0,
                                          OH_PARTICLES_OWNED);
  oh_context_get_region_ids(owned_context, copied_sdid);
  (void)owned_sdid;
  oh_context_unbind_region_ids(owned_context);
  oh_context_bind_region_ids(owned_context, borrowed_sdid,
                             OH_PARTICLES_BORROWED);
  oh_context_get_region_ids(owned_context, copied_sdid);
  oh_context_unbind_region_ids(owned_context);
  capacity = oh_context_max_local_particles_for_capacity(owned_context,
                                                         1000, 250, 8);
  (void)capacity;
  oh_context_configure_level3(owned_context, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  oh_context_set_region_weights(owned_context, weights);
  oh_context_set_particle_mpi_type(owned_context, MPI_DATATYPE_NULL);
  oh_context_set_particle_adapter(owned_context, 0);
  oh_context_destroy(owned_context);
  return 0;
}
