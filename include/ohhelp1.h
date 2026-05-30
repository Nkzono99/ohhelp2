/* File: ohhelp1.h
   Version 1.1.1 (2015/10/23)
   Copyright (C) 2009-2015  Hiroshi Nakashima <h.nakashima@media.kyoto-u.ac.jp>
                            (ACCMS, Kyoto University)
   This program can be freely used, redistributed and modified for non-
   commercial purpose providing that the copyright notice above remains
   unchanged.
*/
#ifndef OHHELP1_H
#define OHHELP1_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <stdarg.h>
#include <mpi.h>

#include "oh_config.h"
#include "oh_mode.h"
#include "oh_stats.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifdef  OH_LIB_LEVEL_4PS
#define OH_POS_AWARE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* constants for D-dimensional simulation */
#define OH_DIM_X        0
#define OH_DIM_Y        1
#define OH_DIM_Z        2
#if OH_DIMENSION==1
#define OH_NEIGHBORS    3
#elif OH_DIMENSION==2
#define OH_NEIGHBORS    (3*3)
#else
#define OH_NEIGHBORS    (3*3*3)
#endif

MPI_Comm oh1_comm(void);

#define MCW oh1_comm()    /* shorthand of communicator, MPI_COMM_WORLD or CTCA_subcomm */

typedef long long int dint;     /* shorthand of 64-bit integer */

#ifndef EXTERN
#define EXTERN extern
#endif

struct S_node;
struct S_commlist;
struct S_comms;
struct S_mycommc;
struct S_mycommf;
struct S_particle;
struct S_grid;
struct S_subdomdesc;
struct S_flddesc;
struct S_borderexc;
struct S_griddesc;
struct S_hplane;
struct S_vplane;
struct S_hotspot;
struct S_hotspotbase;
struct S_interiorp;
struct S_stats;
typedef struct oh_particle_adapter oh_particle_adapter;

struct oh_state;

/* Basic process configuration modes */
#define MODE_NORM_PRI OH_MODE_NORM_PRI
#define MODE_NORM_SEC OH_MODE_NORM_SEC
#define MODE_REB_SEC  OH_MODE_REB_SEC
#define MODE_ANY_PRI  OH_MODE_ANY_PRI
#define MODE_ANY_SEC  OH_MODE_ANY_SEC
#define Mode_PS(M)       (M&1)
#define Mode_Acc(M)      (M&2)
#define Mode_Set_Pri(M)  (M&2)
#define Mode_Set_Sec(M)  (M|1)
#define Mode_Set_Norm(M) (M&1)
#define Mode_Set_Any(M)  (M|2)
#define Mode_Is_Norm(M)  (M<2)
#define Mode_Is_Any(M)   (M>=2)
/* Structured variables for MPI communicator */
#ifndef OHHELP_MYCOMMC_DEFINED
#define OHHELP_MYCOMMC_DEFINED
struct S_mycommc {
  MPI_Comm prime, sec;
  int rank, root, black;
};
#endif
struct S_mycommf {
  int prime, sec;
  int rank, root, black;
};

/* Structures and variables for statistics and verbose messaging */
#define STATS_PART_MOVE_PRI_MIN 0
#define STATS_PART_MOVE_PRI_MAX 1
#define STATS_PART_MOVE_PRI_AVE 2
#define STATS_PART_GET_PRI_MIN  3
#define STATS_PART_GET_PRI_MAX  4
#define STATS_PART_PUT_PRI_MIN  5
#define STATS_PART_PUT_PRI_MAX  6
#define STATS_PART_PG_PRI_AVE   7
#define STATS_PART_MOVE_SEC_MIN 8
#define STATS_PART_MOVE_SEC_MAX 9
#define STATS_PART_MOVE_SEC_AVE 10
#define STATS_PART_GET_SEC_MIN  11
#define STATS_PART_GET_SEC_MAX  12
#define STATS_PART_PUT_SEC_MIN  13
#define STATS_PART_PUT_SEC_MAX  14
#define STATS_PART_PG_SEC_AVE   15
#define STATS_PART_PRIMARY      16
#define STATS_PART_SECONDARY    17
#define STATS_PARTS             (STATS_PART_SECONDARY+1)

#ifdef OH_DEFINE_STATS
static char *StatsPartStrings[STATS_PARTS] = {
  "p2p transfer[pri,min]",
  "p2p transfer[pri,max]",
  "p2p transfer[pri,ave]",
  "get[pri,min]",
  "get[pri,max]",
  "put[pri,min]",
  "put[pri,max]",
  "put&get[pri,ave]",
  "p2p transfer[sec,min]",
  "p2p transfer[sec,max]",
  "p2p transfer[sec,ave]",
  "get[sec,min]",
  "get[sec,max]",
  "put[sec,min]",
  "put[sec,max]",
  "put&get[sec,ave]",
  "transition to pri",
  "transition to sec",
};
#endif

/* Prototypes for the functions called from simulator code */
void oh1_neighbors(int **nbor);
void oh1_families(int **famindex, int **members);
int  oh1_accom_mode();
void oh1_broadcast(void *pbuf, void *sbuf, int pcount, int scount,
                   MPI_Datatype ptype, MPI_Datatype stype);
void oh1_all_reduce(void *pbuf, void *sbuf, int pcount, int scount,
                    MPI_Datatype ptype, MPI_Datatype stype,
                    MPI_Op pop, MPI_Op sop);
void oh1_reduce(void *pbuf, void *sbuf, int pcount, int scount,
                MPI_Datatype ptype, MPI_Datatype stype,
                MPI_Op pop, MPI_Op sop);
void oh1_init_stats(int key, int ps);
void oh1_stats_time(int key, int ps);
void oh1_show_stats(int step, int currmode);
void oh1_print_stats(int nstep);
void oh1_verbose(char *message);
/* Passing NULL resets all region weights to 1.0 and disables weighted mode. */
void oh1_set_region_weights(const double *weights);
struct oh_state *oh1_state(void);
struct oh_state *oh_default_context(void);
void oh_context_set_region_weights(struct oh_state *context,
                                   const double *weights);

void oh1_fam_comm(MPI_Comm *fortran_comm);

void oh1_init(int **sdid, int nspec, int maxfrac, int **nphgram,
              int **totalp, int **rcounts, int **scounts, void *mycomm,
              int **nbor, int *pcoord, int stats, int repiter, int verbose);
int  oh1_transbound(int currmode, int stats);

void oh1_neighbors_(int *nbor);
void oh1_families_(int *famindex, int *members);
int  oh1_accom_mode_();
void oh1_broadcast_(void *pbuf, void *sbuf, int *pcount, int *scount,
                    int *ptype, int *stype);
void oh1_all_reduce_(void *pbuf, void *sbuf, int *pcount, int *scount,
                     int *ptype, int *stype, int *pop, int *sop);
void oh1_reduce_(void *pbuf, void *sbuf, int *pcount, int *scount,
                 int *ptype, int *stype, int *pop, int *sop);
void oh1_init_stats_(int *key, int *ps);
void oh1_stats_time_(int *key, int *ps);
void oh1_show_stats_(int *step, int *currmode);
void oh1_print_stats_(int *nstep);
void oh1_verbose_(char *message);
void oh1_set_region_weights_(double *weights);
void oh1_state_(struct oh_state **state);
void oh1_init_(int *sdid, int *nspec, int *maxfrac, int *nphgram,
               int *totalp, int *rcounts, int *scounts,
               struct S_mycommf *mycomm, int *nbor, int *pcoord, int *stats,
               int *repiter, int *verbose);
int  oh1_transbound_(int *currmode, int *stats);

#ifdef __cplusplus
}
#endif

#endif
