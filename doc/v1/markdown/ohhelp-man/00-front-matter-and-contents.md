# Front Matter and Contents

Source: `doc/v1/original/ohhelp-man.pdf`, pages 1-3.

<!-- Page 1 -->

OhHelp Library Package
for Scalable Domain-Decomposed PIC Simulation∗

Hiroshi Nakashima
(ACCMS, Kyoto University)

2015/10/23




## Abstract

This document describes the usage of a C-code library package named OhHelp for
domain-decoposed Particle-in-Cell (PIC) simulations. The library has the following
three layers. Level-1 code provides a load-balancer function which examines whether
particles are distriuted among computation nodes (MPI processes) in a well-balanced
manner, reforms the configuration of particle assignment to each node if necessary,
and tells you how to move particles among nodes. In Level-2 code, the load balancer
function is also capable to move particles among nodes by MPI functions for you. In
addition, Level-3 code has vairous useful functions for domain-decomposed simulations
such as for exchanging boundary values of electromagnetic fields associated to decopm-
posed subdomain. Furthermore, the library has two types of extensions, Level-4p and
Level-4s, in which the load balancing mechanism takes care of particle positions so that
all particles in a grid-voxel are accommodated by a particular node, to implement, e.g.,
Monte Carlo Collision with the former and Smoothed Particle Hydrodynamics (SPH)
method with the latter.





∗This file has version number v1.1.1, last revised 2015/10/23.


<!-- Page 2 -->

## Contents

1  Introduction                                                      4

2 OhHelp Algorithm                                                 5
2.1  Overview and Definitions   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    5
2.2  Secondary Subdomain Assignment   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    6
2.3  Checking and Keeping Local Balancing .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    7

3 OhHelp Library                                                  11
3.1  Library Layers   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   11
3.2  Applying OhHelp to PIC Simulators  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   12
3.2.1    Duplication of Data Structures   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   13
3.2.2    Duplication of Computation  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   15
3.2.3    Addition of Collective Communications .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   16
3.2.4   Attachment of Load Balancer  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   17
3.3  Configuration: Dimension of Simulated Space and Library Level   .  .  .  .  .  .   19
3.4  Level-1 Library Functions   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   20
3.4.1   oh1_init()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   21
3.4.2   oh1_neighbors() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   27
3.4.3   oh1_families()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   28
3.4.4   oh1_transbound()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   30
3.4.5   oh1_accom_mode()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   31
3.4.6   oh1_broadcast() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   32
3.4.7   oh1_all_reduce()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   33
3.4.8   oh1_reduce() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   34
3.5  Level-2 Library Functions   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   35
3.5.1    Particle Data Type  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   36
3.5.2   oh2_init()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   37
3.5.3   oh2_max_local_particles()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   39
3.5.4   oh2_transbound()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   39
3.5.5   oh2_inject_particle() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   40
3.5.6   oh2_remap_injected_particle()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   40
3.5.7   oh2_remove_injected_particle()    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   41
3.5.8   oh2_set_total_particles()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   41
3.6  Level-3 Library Functions   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   42
3.6.1   oh3_init()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   43
3.6.2   oh13_init()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   56
3.6.3   oh3_grid_size() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   58
3.6.4   oh3_transbound()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   59
3.6.5   oh3_map_particle_to_neighbor()    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   59
3.6.6   oh3_map_particle_to_subdomain()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   61
3.6.7   oh3_bcast_field()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   62
3.6.8   oh3_allreduce_field() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   63
3.6.9   oh3_reduce_field() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   64
3.6.10  oh3_exchange_borders()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   64
3.7  Level-4p Extension and Its Functions .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   66
3.7.1    Position-Aware Particle Management  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   66
3.7.2    Level-4p Functions   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   68
3.7.3   oh4p_init()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   69


<!-- Page 3 -->

3.7.4   oh4p_max_local_particles()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   71
3.7.5   oh4p_per_grid_histogram()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   72
3.7.6   oh4p_transbound()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   73
3.7.7   oh4p_map_particle_to_neighbor()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   73
3.7.8   oh4p_map_particle_to_subdomain()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   75
3.7.9   oh4p_inject_particle()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   75
3.7.10  oh4p_remove_mapped_particle()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   76
3.7.11  oh4p_remap_particle_to_neighbor()    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   77
3.7.12  oh4p_remap_particle_to_subdomain()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   77
3.8  Level-4s Extension and Its Functions  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   78
3.8.1    Position-Aware Particle Management in Level-4s   .  .  .  .  .  .  .  .  .  .   78
3.8.2    Level-4s Functions   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   80
3.8.3   oh4s_init()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   81
3.8.4   oh4s_particle_buffer()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   84
3.8.5   oh4s_per_grid_histogram()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   85
3.8.6   oh4s_transbound()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   86
3.8.7   oh4s_exchange_border_data() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   86
3.8.8   oh4s_map_particle_to_neighbor()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   87
3.8.9   oh4s_map_particle_to_subdomain()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   88
3.8.10  oh4s_inject_particle()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   88
3.8.11  oh4s_remove_mapped_particle()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   88
3.8.12  oh4s_remap_particle_to_neighbor()    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   89
3.8.13  oh4s_remap_particle_to_subdomain()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   89
3.9  Particle Injection and Removal   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   90
3.9.1    Level-1 Injection and Removal .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   90
3.9.2    Level-2 (and 3) Injection and Removal  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   90
3.9.3    Level-4p and 4s Injection and Removal  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   91
3.9.4    Identification of Injected Particles .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   92
3.10 Statistics    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   92
3.10.1  Timing Statistics Keys and Header File oh stats.h .  .  .  .  .  .  .  .  .  .   93
3.10.2  Arguments of oh1_init() for Statistics   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   95
3.10.3  oh1_init_stats()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   95
3.10.4  oh1_stats_time()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   96
3.10.5  oh1_show_stats()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   96
3.10.6  oh1_print_stats()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   97
3.11 Verbose Messaging  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   99
3.12 Aliases of Functions   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  100
3.13 Sample Code   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  100
3.13.1   Fortran Sample Code .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  102
3.13.2  C Sample Code  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  113
3.14 How to make   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  122
