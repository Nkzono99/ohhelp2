#include <assert.h>
#include <mpi.h>
#include <string.h>

#include "oh_context.h"
#include "oh_fortran_v2.h"

int
main(int argc, char **argv) {
  oh_context *context = 0;
  int err;

  if (argc > 1 && strcmp(argv[1], "before-init") == 0) {
    err = oh_context_create(MPI_COMM_WORLD, &context);
    assert(err == MPI_ERR_OTHER);
    assert(context == 0);
    return 0;
  }
  if (argc > 1 && strcmp(argv[1], "fortran-before-init") == 0) {
    err = oh_fortran_context_create(0, &context);
    assert(err == MPI_ERR_OTHER);
    assert(context == 0);
    return 0;
  }

  MPI_Init(&argc, &argv);
  MPI_Finalize();

  if (argc > 1 && strcmp(argv[1], "fortran-after-finalize") == 0) {
    err = oh_fortran_context_create(0, &context);
    assert(err == MPI_ERR_OTHER);
    assert(context == 0);
    return 0;
  }

  err = oh_context_create(MPI_COMM_WORLD, &context);
  assert(err == MPI_ERR_OTHER);
  assert(context == 0);
  return 0;
}
