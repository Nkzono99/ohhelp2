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

struct oh_state {
  MPI_Comm comm;
  int n_of_nodes;
  int my_rank;
  int *region_id;
  int *subdomain_id;
  int curr_mode;
  int acc_mode;
  int n_of_species;
  int max_fraction;
  int *n_of_particles_local;
  int *n_of_primaries;
  dint *total_particles_global;
  double *region_weights;
  double *total_load_global;
  dint n_of_particles;
  double total_load;
  int n_of_local_particles_max;
  double n_of_local_load_max;
  int weighted_load_balancing;
  dint *n_of_particles_to_stay;
  int *total_particles;
  int *total_particles_next;
  int *injected_particles;
  int *n_of_recv;
  int *n_of_send;
  int primary_parts;
  int total_parts;
  struct S_node *nodes;
  struct S_node *nodes_next;
  struct S_node **node_queue;
  int *temp_array;
  int (*neighbors)[OH_NEIGHBORS];
  int *dst_neighbors;
  int *src_neighbors;
  struct S_commlist *comm_list;
  struct S_commlist *sec_recv_list;
  int *rl_index;
  int *sl_head_tail;
  int *sec_sl_head_tail;
  int *sec_rl_size;
  MPI_Group world_group;
  struct S_comms *communicators;
  struct S_mycommc *my_comm;
  struct S_mycommc *my_comm_c;
  struct S_mycommf *my_comm_f;
  int n_of_local_particles_limit;
  struct S_particle *particles;
  struct S_particle *send_buffer;
  struct S_particle **recv_buffer_bases;
  int *secondary_base;
  int *total_local_particles;
  int *send_buffer_disps;
  int *recv_buffer_disps;
  int n_of_injections;
  int spec_base;
  MPI_Datatype particle_mpi_type;
  MPI_Datatype custom_particle_mpi_type;
  int use_custom_particle_mpi_type;
  oh_particle_adapter *particle_adapter;
  oh_particle_adapter *custom_particle_adapter;
  int use_custom_particle_adapter;
  MPI_Request *requests;
  MPI_Status *statuses;
  int exclude_level2;
  int (*abs_neighbors)[OH_NEIGHBORS];
  int (*subdomains)[OH_DIMENSION][2];
  double (*subdomains_float)[OH_DIMENSION][2];
  struct S_grid *grid;
  int grid_mask;
  int log_grid;
  struct S_subdomdesc *subdomain_desc;
  int n_of_boundaries;
  int (*boundaries)[OH_DIMENSION][2];
  int *adjacent;
  int n_of_fields;
  int *field_types;
  struct S_flddesc *field_desc;
  int n_of_exchanges;
  int *boundary_comm_fields;
  int *boundary_comm_types;
  struct S_borderexc *border_exchange;
  struct S_griddesc *level4_grid_desc;
  int *level4_pbuf_index;
  dint **level4_particle_grid[2];
  dint **level4_particle_grid_total[2];
  int **level4_particle_grid_out[2];
  int **level4_particle_grid_index[2];
  int **level4_particle_grid_out_shadow[2];
  int **level4_particle_grid_index_shadow[2];
  dint *level4_particle_grid_z;
  int **level4_hotspot_recv;
  int *level4_hotspot_send;
  int *level4_hotspot_recv_from_parent;
  int *level4_hotspot_receiver;
  struct S_hotspot *level4_hotspot_list;
  struct S_hotspot *level4_hotspot_top;
  struct S_hotspotbase *level4_hotspots;
  struct S_hplane *level4_horizontal_planes;
  struct S_vplane *level4_vertical_planes;
  int *level4_vertical_plane_head;
  struct S_commlist *level4_alt_sec_recv_list;
  struct S_commlist *level4_primary_comm_list;
  int *level4_sec_rl_index;
  int *level4_alt_sec_rl_index;
  int *level4_primary_rl_index;
  MPI_Datatype level4_histogram_half_type;
  struct S_interiorp *level4_interior_parts;
  int level4_grid_overflow_limit;
  struct S_particle *level4_boundary_send_buffer;
  int *level4_first_neighbor;
  int *level4_grid_offset;
  void *level4_real_dst_neighbors;
  void *level4_real_src_neighbors;
  int *level4_boundary_condition;
  int *level4_z_bound;
  int *level4_z_bound_shadow;
  struct S_stats *stats;
  int stats_mode;
  int report_iteration;
  MPI_Datatype *stats_time_type;
  MPI_Op *stats_time_op;
  MPI_Op *stats_part_op;
};
extern struct oh_state OhDefaultState;

/* Basic process configuration modes */
#define MODE_NORM_PRI (0)
#define MODE_NORM_SEC (1)
#define MODE_REB_SEC  (-1)
#define MODE_ANY_PRI  (2)
#define MODE_ANY_SEC  (3)
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

#endif
