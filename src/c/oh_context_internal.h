/* File: oh_context_internal.h
   Internal bridge while v2 migrates process-global state into contexts.
*/
#ifndef OH_CONTEXT_INTERNAL_H
#define OH_CONTEXT_INTERNAL_H

#include "ohhelp1.h"
#include "oh_particle_adapter.h"

struct oh_state {
  MPI_Comm comm;
  int owns_comm;
  int n_of_nodes;
  int my_rank;
  int *region_id;
  int *subdomain_id;
  int owns_region_id;
  int owns_subdomain_id;
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
  int particle_accounting_bound;
  int n_of_particles_local_ownership;
  int total_particles_next_ownership;
  int *injected_particles;
  int *n_of_recv;
  int *n_of_send;
  int *recv_counts;
  int *send_counts;
  int owns_level1_storage;
  int primary_parts;
  int total_parts;
  struct S_node *nodes;
  struct S_node *nodes_next;
  struct S_node **node_queue;
  struct S_heap less_heap;
  struct S_heap greater_heap;
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
  void *particles;
  int particle_buffer_bound;
  int particle_buffer_ownership;
  void *send_buffer;
  void **recv_buffer_bases;
  int owns_level2_storage;
  int *secondary_base;
  int *total_local_particles;
  int particle_base_bound;
  int particle_base_ownership;
  int *send_buffer_disps;
  int *recv_buffer_disps;
  int n_of_injections;
  int spec_base;
  MPI_Datatype particle_mpi_type;
  MPI_Datatype histogram_type;
  MPI_Datatype comm_list_type;
  MPI_Datatype custom_particle_mpi_type;
  int use_custom_particle_mpi_type;
  oh_particle_adapter *particle_adapter;
  oh_particle_adapter *custom_particle_adapter;
  oh_particle_adapter owned_particle_adapter;
  oh_particle_adapter owned_custom_particle_adapter;
  int owns_particle_mpi_type;
  int use_custom_particle_adapter;
  MPI_Request *requests;
  MPI_Status *statuses;
  int exclude_level2;
  int owns_level3_storage;
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
  void *level4_boundary_send_buffer;
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

void oh1_sync_default_state(void);
int oh_context_is_default_state(const struct oh_state *state);
void oh_context_validate_species_node_capacity(int n_of_nodes, int nspec,
                                               const char *api);
void oh1_set_region_weights_state(struct oh_state *state,
                                  const double *weights);
void oh_context_init_level1_state(struct oh_state *state, int **sdid,
                                  int nspec, int maxfrac, int **nphgram,
                                  int **totalp, int **rcounts,
                                  int **scounts,
                                  struct S_mycommc *mycommc,
                                  struct S_mycommf *mycommf, int **nbor,
                                  int *pcoord, int stats, int repiter,
                                  int verbose);
void oh_context_build_grid_neighbors(struct oh_state *state,
                                     const int *pcoord,
                                     int raw[OH_NEIGHBORS]);
void oh_context_apply_neighbors(struct oh_state *state,
                                const int raw[OH_NEIGHBORS]);
void oh_context_bind_particle_accounting_state(struct oh_state *state,
                                               int **nphgram, int **totalp,
                                               int **pbase, int ownership);
void oh_context_unbind_particle_accounting_state(struct oh_state *state);

#endif
