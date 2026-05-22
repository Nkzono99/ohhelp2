#include "oh_context.h"

int
main(void) {
  oh_context *context = 0;

  (void)context;
  oh_context_set_particle_mpi_type(context, MPI_DATATYPE_NULL);
  oh_context_set_particle_adapter(context, 0);
  return 0;
}
