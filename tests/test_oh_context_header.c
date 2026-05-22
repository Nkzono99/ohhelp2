#include "oh_context.h"

int
main(void) {
  oh_context *context = 0;
  double *weights = 0;
  double size[3] = {0.0, 0.0, 0.0};
  void *particle = 0;
  void *injected;
  void *primary = 0;
  void *secondary = 0;
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;

  oh_context_set_particle_mpi_type(context, MPI_DATATYPE_NULL);
  oh_context_set_particle_adapter(context, 0);
  oh_context_set_region_weights(context, weights);
  (void)oh_context_transbound1(context, 0, 0);
  (void)oh_context_transbound2(context, 0, 0);
  (void)oh_context_transbound3(context, 0, 0);
  oh_context_broadcast(context, primary, secondary, 0, 0, MPI_DOUBLE,
                       MPI_DOUBLE);
  oh_context_all_reduce(context, primary, secondary, 0, 0, MPI_DOUBLE,
                        MPI_DOUBLE, MPI_SUM, MPI_SUM);
  oh_context_reduce(context, primary, secondary, 0, 0, MPI_DOUBLE,
                    MPI_DOUBLE, MPI_SUM, MPI_SUM);
  oh_context_set_total_particles(context);
  oh_context_inject_particle(context, particle);
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
  return 0;
}
