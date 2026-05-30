#include <assert.h>
#include <limits.h>
#include <mpi.h>
#include <string.h>

#include "ohhelp1.h"
#include "ohhelp3.h"
#include "ohhelp4s.h"

int gridMask;
int logGrid;

int
main(int argc, char **argv) {
  int *sdid = NULL;
  int *totalp = NULL;
  int *pbase = NULL;
  void *pbuf = NULL;
  int *nbor = NULL;
  int *sdoms = NULL;
  int *bounds = NULL;
  int *fsizes = NULL;
  int *zbound = NULL;
  int pcoord[3] = {1, 1, 1};
  int scoord[6] = {0, 1, 0, 1, 0, 1};
  int bcond[6] = {1, 1, 1, 1, 1, 1};
  int ftypes[1][OH_FTYPE_N] = {{-1, 0, 0, 0, 0, 0, 0}};
  int cfields[1] = {-1};
  int ctypes[1][2][OH_CTYPE_N] = {{{0}}};
  int maxlocalp = 0;
  int cbufsize = 0;

  MPI_Init(&argc, &argv);

  oh4s_init(&sdid, 1, 20, 1, 1, 1, &totalp, &pbase, &maxlocalp,
            &cbufsize, NULL, &nbor, pcoord, &sdoms, scoord, 2, bcond,
            &bounds, (int*)ftypes, cfields, (int*)ctypes, &fsizes, &zbound,
            0, 0, 0);

  if (argc > 1 && strcmp(argv[1], "null-pbuf-slot") == 0)
    oh4s_particle_buffer(maxlocalp, NULL);
  if (argc > 1 && strcmp(argv[1], "overflow-maxlocalp") == 0)
    oh4s_particle_buffer(INT_MAX, &pbuf);

  oh4s_particle_buffer(maxlocalp, &pbuf);
  assert(pbuf != NULL);

  MPI_Finalize();
  return 0;
}
