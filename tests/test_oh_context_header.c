#include "oh_context.h"

int
main(void) {
  oh_context *context = 0;
  double *weights = 0;

  oh_context_set_particle_mpi_type(context, MPI_DATATYPE_NULL);
  oh_context_set_particle_adapter(context, 0);
  oh_context_set_region_weights(context, weights);
  return 0;
}
