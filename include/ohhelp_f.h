/* File: ohhelp_f.h
   Version 1.1.1 (2015/10/23)
   Copyright (C) 2009-2015  Hiroshi Nakashima <h.nakashima@media.kyoto-u.ac.jp>
                            (ACCMS, Kyoto University)
   This program can be freely used, redistributed and modified for non-
   commercial purpose providing that the copyright notice above remains
   unchanged.
*/
#ifndef OHHELP_F_H
#define OHHELP_F_H

#include "oh_config.h"

#define oh_neighbors             oh1_neighbors
#define oh_families              oh1_families
#define oh_accom_mode            oh1_accom_mode
#define oh_broadcast             oh1_broadcast
#define oh_all_reduce            oh1_all_reduce
#define oh_reduce                oh1_reduce
#define oh_init_stats            oh1_init_stats
#define oh_stats_time            oh1_stats_time
#define oh_show_stats            oh1_show_stats
#define oh_print_stats           oh1_print_stats
#define oh_verbose               oh1_verbose
#define oh_set_region_weights    oh1_set_region_weights
#if OH_LIB_LEVEL==1
#define oh_init                  oh1_init
#define oh_transbound            oh1_transbound
#else
#define oh_set_total_particles oh2_set_total_particles
#if OH_LIB_LEVEL!=4
#define oh_max_local_particles   oh2_max_local_particles
#define oh_set_particle_mpi_type oh2_set_particle_mpi_type
#define oh_inject_particle       oh2_inject_particle
#define oh_remap_injected_particle  oh2_remap_injected_particle
#define oh_remove_injected_particle oh2_remove_injected_particle
#endif
#if OH_LIB_LEVEL==2
#define oh_init                  oh2_init
#define oh_transbound            oh2_transbound
#else
#define oh_grid_size             oh3_grid_size
#define oh_bcast_field           oh3_bcast_field
#define oh_reduce_field          oh3_reduce_field
#define oh_allreduce_field       oh3_allreduce_field
#define oh_exchange_borders      oh3_exchange_borders
#if OH_LIB_LEVEL==3
#define oh_map_particle_to_neighbor  oh3_map_particle_to_neighbor
#define oh_map_particle_to_subdomain oh3_map_particle_to_subdomain
#define oh_init                  oh3_init
#define oh_transbound            oh3_transbound
#else
#ifdef OH_LIB_LEVEL_4P
#define oh_init                  oh4p_init
#define oh_max_local_particles   oh4p_max_local_particles
#define oh_per_grid_histogram    oh4p_per_grid_histogram
#define oh_transbound            oh4p_transbound
#define oh_map_particle_to_neighbor  oh4p_map_particle_to_neighbor
#define oh_map_particle_to_subdomain oh4p_map_particle_to_subdomain
#define oh_inject_particle       oh4p_inject_particle
#define oh_remove_mapped_particle oh4p_remove_mapped_particle
#define oh_remap_particle_to_neighbor  oh4p_remap_particle_to_neighbor
#define oh_remap_particle_to_subdomain oh4p_remap_particle_to_subdomain
#else
#define oh_init                  oh4s_init
#define oh_particle_buffer       oh4s_particle_buffer
#define oh_per_grid_histogram    oh4s_per_grid_histogram
#define oh_transbound            oh4s_transbound
#define oh_exchange_border_data  oh4s_exchange_border_data
#define oh_map_particle_to_neighbor  oh4s_map_particle_to_neighbor
#define oh_map_particle_to_subdomain oh4s_map_particle_to_subdomain
#define oh_inject_particle       oh4s_inject_particle
#define oh_remove_mapped_particle oh4s_remove_mapped_particle
#define oh_remap_particle_to_neighbor  oh4s_remap_particle_to_neighbor
#define oh_remap_particle_to_subdomain oh4s_remap_particle_to_subdomain
#endif
#endif
#endif

#endif
#endif
