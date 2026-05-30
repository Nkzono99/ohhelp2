#include <assert.h>
#include <limits.h>
#include <mpi.h>
#include <string.h>

#include "ohhelp1.h"
#include "ohhelp4p.h"

int
main(int argc, char **argv) {
  int capacity;

  MPI_Init(&argc, &argv);
  if (argc > 1 && strcmp(argv[1], "zero-hotspot") == 0)
    (void)oh4p_max_local_particles(1, 1, 0, 0);
  if (argc > 1 && strcmp(argv[1], "overflow-hotspot") == 0)
    (void)oh4p_max_local_particles(1, 1, 0, INT_MAX);
  if (argc > 1 && strcmp(argv[1], "overflow-doubled-capacity") == 0)
    (void)oh4p_max_local_particles((dint)INT_MAX / 2 + 1, 1, 0, 1);

  capacity = oh4p_max_local_particles(1, 1, 0, 1);
  assert(capacity > 0);
  MPI_Finalize();
  return 0;
}
