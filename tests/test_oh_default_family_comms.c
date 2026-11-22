/* Family communicators of the default (legacy oh1_init) state.

   init1 allocates Comms.body[] without initialising it, and every rebalance
   frees each non-null entry below Comms.n before building the new families.
   So after a rebalance, an entry must be MPI_COMM_NULL unless it is one of
   the two families this rank belongs to (MyComm->prime, MyComm->sec).
   The test fills body[] with a handle that is neither, to stand for the
   uninitialised memory, and checks this after two rebalances that place the
   load on different regions (so some ranks are outside some families). */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "oh_mode.h"
#include "ohhelp1.h"
#include "ohhelp1_internal.h"

static int
check_family_entries(int rank) {
  int i, mine = 0, outside = 0;

  for (i = 0; i < Comms.n; i++) {
    MPI_Comm c = Comms.body[i];
    if (c == MPI_COMM_NULL) {
      outside++;
      continue;
    }
    if (c != MyComm->prime && c != MyComm->sec) {
      fprintf(stderr, "rank %d: Comms.body[%d] of %d is neither MPI_COMM_NULL "
              "nor one of my families\n", rank, i, Comms.n);
      abort();
    }
    mine++;
  }
  assert(mine == (MyComm->prime != MPI_COMM_NULL) +
                 (MyComm->sec != MPI_COMM_NULL));
  return outside;
}

static void
rebalance(int *nphgram, const int *load, int rank, int *mode) {
  nphgram[rank] = load[rank];
  *mode = oh1_transbound(*mode, 0);
}

int
main(int argc, char **argv) {
  int *sdid = 0, *nphgram = 0, *totalp = 0, *rcounts = 0, *scounts = 0;
  int *nbor = 0;
  int pcoord[3];
  int rank, n, i, mode = OH_MODE_NORMAL_PRIMARY, outside, outside_sum;
  int *load;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &n);
  if (n < 4) {
    if (rank == 0) printf("test_oh_default_family_comms: needs 4+ ranks\n");
    MPI_Finalize();
    return 0;
  }

  pcoord[0] = n;  pcoord[1] = 1;  pcoord[2] = 1;
  oh1_init(&sdid, 1, 20, &nphgram, &totalp, &rcounts, &scounts, NULL,
           &nbor, pcoord, 0, 0, 0);
  for (i = 0; i < n; i++)  Comms.body[i] = MPI_COMM_SELF;
  load = (int*)calloc((size_t)n, sizeof(int));
  assert(load);

  /* Two heavy regions at the ends: several families. */
  load[0] = 10 * n;  load[n - 1] = 7 * n;
  rebalance(nphgram, load, rank, &mode);
  assert(mode == OH_MODE_REBALANCE_SECONDARY);
  outside = check_family_entries(rank);
  MPI_Allreduce(&outside, &outside_sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  if (rank == 0)
    printf("rebalance 1: %d families, %d (rank, family) pairs outside\n",
           Comms.n, outside_sum);
  assert(outside_sum > 0);

  /* Move the load: the families are rebuilt and the old ones freed. */
  for (i = 0; i < n; i++)  load[i] = 0;
  load[1] = 9 * n;  load[n / 2] = 8 * n;
  rebalance(nphgram, load, rank, &mode);
  outside = check_family_entries(rank);
  MPI_Allreduce(&outside, &outside_sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  if (rank == 0)
    printf("rebalance 2: %d families, %d (rank, family) pairs outside\n",
           Comms.n, outside_sum);

  /* And once more, so the second set is freed as well. */
  for (i = 0; i < n; i++)  load[i] = 0;
  load[n - 2] = 11 * n;
  rebalance(nphgram, load, rank, &mode);
  check_family_entries(rank);

  free(load);
  MPI_Barrier(MPI_COMM_WORLD);
  if (rank == 0)  printf("test_oh_default_family_comms: ok (n=%d)\n", n);
  MPI_Finalize();
  return 0;
}
