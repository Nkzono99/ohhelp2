#include <assert.h>
#include <limits.h>
#include <mpi.h>
#include <string.h>

#include "ohhelp3.h"
#include "ohhelp2.h"
#include "oh_fortran_v2.h"

static void
run_oh3_init(int argc, char **argv, int nranks) {
  int *sdid = 0;
  int *nphgram = 0;
  int *totalp = 0;
  void *pbuf = 0;
  int *pbase = 0;
  int *nbor = 0;
  int *sdoms = 0;
  int *bounds = 0;
  int *fsizes = 0;
  int pcoord[3] = {nranks, 1, 1};
  int scoord[6] = {0, nranks, 0, 1, 0, 1};
  int bcond[6] = {0, 0, 0, 0, 0, 0};
  int ftypes[2][OH_FTYPE_N] = {
    {1, 0, 1, 0, 1, 0, 1},
    {0, 0, 0, 0, 0, 0, 0}
  };
  int cfields[1] = {-1};

  if (argc > 1 && strcmp(argv[1], "oh3-null-pbuf-slot") == 0) {
    oh3_init(&sdid, 1, 0, &nphgram, &totalp, 0, &pbase, 4, 0, &nbor,
             pcoord, &sdoms, scoord, 1, bcond, &bounds, (int*)ftypes, cfields, 0,
             &fsizes, 0, 0, 0);
  } else if (argc > 1 && strcmp(argv[1], "oh3-null-pbase-slot") == 0) {
    oh3_init(&sdid, 1, 0, &nphgram, &totalp, &pbuf, 0, 4, 0, &nbor,
             pcoord, &sdoms, scoord, 1, bcond, &bounds, (int*)ftypes, cfields, 0,
             &fsizes, 0, 0, 0);
  } else if (argc > 1 && strcmp(argv[1], "oh3-negative-maxfrac") == 0) {
    oh3_init(&sdid, 1, -1, &nphgram, &totalp, &pbuf, &pbase, 4, 0, &nbor,
             pcoord, &sdoms, scoord, 1, bcond, &bounds, (int*)ftypes, cfields, 0,
             &fsizes, 0, 0, 0);
  } else {
    oh3_init(&sdid, 1, 0, &nphgram, &totalp, &pbuf, &pbase, 4, 0, &nbor,
             pcoord, &sdoms, scoord, 1, bcond, &bounds, (int*)ftypes, cfields, 0,
             &fsizes, 0, 0, 0);
    assert(sdid);
    assert(nphgram);
    assert(totalp);
    assert(pbuf);
    assert(pbase);
    assert(sdoms);
    assert(bounds);
  }
}

int
main(int argc, char **argv) {
  int *sdid = 0;
  int *nphgram = 0;
  int *totalp = 0;
  void *pbuf = 0;
  int *pbase = 0;
  int *nbor = 0;
  int nranks = 0;
  int pcoord[3] = {1, 1, 1};
  int scoord[6] = {0, 1, 0, 1, 0, 1};
  int bcond[6] = {0, 0, 0, 0, 0, 0};
  int ftypes[2][OH_FTYPE_N] = {
    {1, 0, 1, 0, 1, 0, 1},
    {0, 0, 0, 0, 0, 0, 0}
  };
  int cfields[1] = {-1};
  int ctypes[2][OH_CTYPE_N] = {{0}};
  int fsizes[6] = {0};
  int raw_sdid[2] = {0, -1};
  int raw_nphgram[2] = {0, 0};
  int raw_totalp[2] = {0, 0};
  int raw_pbase[3] = {0, 0, 0};
  void *raw_pbuf = 0;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nranks);
  pcoord[0] = nranks;
  scoord[1] = nranks;

  if (argc > 1 && strcmp(argv[1], "raw-oh2-null-pbuf-slot") == 0) {
    oh_fortran_oh2_init_raw(sdid, 1, 0, nphgram, totalp, 0, pbase, 4, 0,
                            nbor, pcoord, 0, 0, 0);
  } else if (argc > 1 && strcmp(argv[1], "maxlocal-negative-minmargin") == 0) {
    (void)oh2_max_local_particles(1, 1, -1);
  } else if (argc > 1 && strcmp(argv[1], "maxlocal-overflow-base") == 0) {
    (void)oh2_max_local_particles((dint)INT_MAX + 1, 1, 0);
  } else if (argc > 1 && strcmp(argv[1], "maxlocal-overflow-margin") == 0) {
    (void)oh2_max_local_particles((dint)INT_MAX, 1, INT_MAX);
  } else if (argc > 1 &&
             strcmp(argv[1], "raw-oh3-null-pbuf-slot") == 0) {
    oh_fortran_oh3_init_raw(sdid, 1, 0, nphgram, totalp, 0, pbase, 4, 0,
                            nbor, pcoord, 0, scoord, 1, bcond, 0,
                            (int*)ftypes, cfields, (int*)ctypes, fsizes, 0, 0,
                            0);
  } else if (argc > 1 && strcmp(argv[1], "raw-oh2-null-sdid") == 0) {
    oh_fortran_oh2_init_raw(0, 1, 0, raw_nphgram, raw_totalp, &raw_pbuf,
                            raw_pbase, 4, 0, nbor, pcoord, 0, 0, 0);
  } else if (argc > 1 && strcmp(argv[1], "raw-oh2-null-nphgram") == 0) {
    oh_fortran_oh2_init_raw(raw_sdid, 1, 0, 0, raw_totalp, &raw_pbuf,
                            raw_pbase, 4, 0, nbor, pcoord, 0, 0, 0);
  } else if (argc > 1 && strcmp(argv[1], "raw-oh2-null-totalp") == 0) {
    oh_fortran_oh2_init_raw(raw_sdid, 1, 0, raw_nphgram, 0, &raw_pbuf,
                            raw_pbase, 4, 0, nbor, pcoord, 0, 0, 0);
  } else if (argc > 1 && strcmp(argv[1], "raw-oh3-null-sdid") == 0) {
    oh_fortran_oh3_init_raw(0, 1, 0, raw_nphgram, raw_totalp, &raw_pbuf,
                            raw_pbase, 4, 0, nbor, pcoord, 0, scoord, 1,
                            bcond, 0, (int*)ftypes, cfields, (int*)ctypes,
                            fsizes, 0, 0, 0);
  } else if (argc > 1 && strcmp(argv[1], "raw-oh3-null-nphgram") == 0) {
    oh_fortran_oh3_init_raw(raw_sdid, 1, 0, 0, raw_totalp, &raw_pbuf,
                            raw_pbase, 4, 0, nbor, pcoord, 0, scoord, 1,
                            bcond, 0, (int*)ftypes, cfields, (int*)ctypes,
                            fsizes, 0, 0, 0);
  } else if (argc > 1 && strcmp(argv[1], "raw-oh3-null-totalp") == 0) {
    oh_fortran_oh3_init_raw(raw_sdid, 1, 0, raw_nphgram, 0, &raw_pbuf,
                            raw_pbase, 4, 0, nbor, pcoord, 0, scoord, 1,
                            bcond, 0, (int*)ftypes, cfields, (int*)ctypes,
                            fsizes, 0, 0, 0);
  } else if (argc > 1 && strcmp(argv[1], "raw-oh2-negative-maxfrac") == 0) {
    oh_fortran_oh2_init_raw(raw_sdid, 1, -1, raw_nphgram, raw_totalp,
                            &raw_pbuf, raw_pbase, 4, 0, nbor, pcoord, 0, 0,
                            0);
  } else if (argc > 1 && strcmp(argv[1], "raw-oh3-negative-maxfrac") == 0) {
    oh_fortran_oh3_init_raw(raw_sdid, 1, -1, raw_nphgram, raw_totalp,
                            &raw_pbuf, raw_pbase, 4, 0, nbor, pcoord, 0,
                            scoord, 1, bcond, 0, (int*)ftypes, cfields,
                            (int*)ctypes, fsizes, 0, 0, 0);
  } else if (argc > 1 && strncmp(argv[1], "oh3-", 4) == 0) {
    run_oh3_init(argc, argv, nranks);
  } else if (argc > 1 && strcmp(argv[1], "null-pbuf-slot") == 0) {
    oh2_init(&sdid, 1, 0, &nphgram, &totalp, 0, &pbase, 4, 0, &nbor,
             pcoord, 0, 0, 0);
  } else if (argc > 1 && strcmp(argv[1], "null-pbase-slot") == 0) {
    oh2_init(&sdid, 1, 0, &nphgram, &totalp, &pbuf, 0, 4, 0, &nbor,
             pcoord, 0, 0, 0);
  } else if (argc > 1 && strcmp(argv[1], "negative-maxfrac") == 0) {
    oh2_init(&sdid, 1, -1, &nphgram, &totalp, &pbuf, &pbase, 4, 0, &nbor,
             pcoord, 0, 0, 0);
  } else {
    oh2_init(&sdid, 1, 0, &nphgram, &totalp, &pbuf, &pbase, 4, 0, &nbor,
             pcoord, 0, 0, 0);
    assert(sdid);
    assert(nphgram);
    assert(totalp);
    assert(pbuf);
    assert(pbase);
  }

  MPI_Finalize();
  return 0;
}
