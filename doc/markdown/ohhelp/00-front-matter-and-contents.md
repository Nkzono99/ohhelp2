# Front Matter and Contents

Source: `doc/original/ohhelp.pdf`, pages 1-9.

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
This document also describes the implementation details of the OhHelp library
showing every line of each source file. Since the source files are extracted from this file,
the descriptions and explanations perfectly corresponds to the real implemenatation
with which you play for your own PIC simulation.





∗This file has version number v1.1.1, last revised 2015/10/23.


<!-- Page 2 -->

## Contents

1  Introduction                                                     10

2 OhHelp Algorithm                                                11
2.1  Overview and Definitions   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   11
2.2  Secondary Subdomain Assignment   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   12
2.3  Checking and Keeping Local Balancing .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   13

3 OhHelp Library                                                  17
3.1  Library Layers   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   17
3.2  Applying OhHelp to PIC Simulators  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   18
3.2.1    Duplication of Data Structures   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   19
3.2.2    Duplication of Computation  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   21
3.2.3    Addition of Collective Communications .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   22
3.2.4   Attachment of Load Balancer  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   23
3.3  Configuration: Dimension of Simulated Space and Library Level   .  .  .  .  .  .   25
3.4  Level-1 Library Functions   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   26
3.4.1   oh1_init()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   27
3.4.2   oh1_neighbors() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   33
3.4.3   oh1_families()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   34
3.4.4   oh1_transbound()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   36
3.4.5   oh1_accom_mode()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   37
3.4.6   oh1_broadcast() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   38
3.4.7   oh1_all_reduce()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   39
3.4.8   oh1_reduce() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   40
3.5  Level-2 Library Functions   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   41
3.5.1    Particle Data Type  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   42
3.5.2   oh2_init()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   43
3.5.3   oh2_max_local_particles()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   45
3.5.4   oh2_transbound()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   45
3.5.5   oh2_inject_particle() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   46
3.5.6   oh2_remap_injected_particle()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   46
3.5.7   oh2_remove_injected_particle()    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   47
3.5.8   oh2_set_total_particles()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   47
3.6  Level-3 Library Functions   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   48
3.6.1   oh3_init()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   49
3.6.2   oh13_init()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   62
3.6.3   oh3_grid_size() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   64
3.6.4   oh3_transbound()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   65
3.6.5   oh3_map_particle_to_neighbor()    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   65
3.6.6   oh3_map_particle_to_subdomain()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   67
3.6.7   oh3_bcast_field()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   68
3.6.8   oh3_allreduce_field() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   69
3.6.9   oh3_reduce_field() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   70
3.6.10  oh3_exchange_borders()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   70
3.7  Level-4p Extension and Its Functions .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   72
3.7.1    Position-Aware Particle Management  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   72
3.7.2    Level-4p Functions   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   74
3.7.3   oh4p_init()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   75


<!-- Page 3 -->

3.7.4   oh4p_max_local_particles()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   77
3.7.5   oh4p_per_grid_histogram()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   78
3.7.6   oh4p_transbound()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   79
3.7.7   oh4p_map_particle_to_neighbor()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   79
3.7.8   oh4p_map_particle_to_subdomain()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   81
3.7.9   oh4p_inject_particle()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   81
3.7.10  oh4p_remove_mapped_particle()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   82
3.7.11  oh4p_remap_particle_to_neighbor()    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   83
3.7.12  oh4p_remap_particle_to_subdomain()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   83
3.8  Level-4s Extension and Its Functions  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   84
3.8.1    Position-Aware Particle Management in Level-4s   .  .  .  .  .  .  .  .  .  .   84
3.8.2    Level-4s Functions   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   86
3.8.3   oh4s_init()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   87
3.8.4   oh4s_particle_buffer()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   90
3.8.5   oh4s_per_grid_histogram()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   91
3.8.6   oh4s_transbound()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   92
3.8.7   oh4s_exchange_border_data() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   92
3.8.8   oh4s_map_particle_to_neighbor()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   93
3.8.9   oh4s_map_particle_to_subdomain()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   94
3.8.10  oh4s_inject_particle()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   94
3.8.11  oh4s_remove_mapped_particle()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   94
3.8.12  oh4s_remap_particle_to_neighbor()    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   95
3.8.13  oh4s_remap_particle_to_subdomain()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   95
3.9  Particle Injection and Removal   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   96
3.9.1    Level-1 Injection and Removal .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   96
3.9.2    Level-2 (and 3) Injection and Removal  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   96
3.9.3    Level-4p and 4s Injection and Removal  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   97
3.9.4    Identification of Injected Particles .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   98
3.10 Statistics    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   98
3.10.1  Timing Statistics Keys and Header File oh stats.h .  .  .  .  .  .  .  .  .  .   99
3.10.2  Arguments of oh1_init() for Statistics   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  101
3.10.3  oh1_init_stats()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  101
3.10.4  oh1_stats_time()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  102
3.10.5  oh1_show_stats()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  102
3.10.6  oh1_print_stats()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  103
3.11 Verbose Messaging  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  105
3.12 Aliases of Functions   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  106
3.13 Sample Code   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  106
3.13.1   Fortran Sample Code .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  108
3.13.2  C Sample Code  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  119
3.14 How to make   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  128

4  Implementation                                                 130
4.1  Naming Convention    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  130
4.2  Header File ohhelp1.h    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  132
4.2.1   Header File Inclusion  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  132
4.2.2    Constants and Shorthands  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  132
4.2.3    Basic Process Configuration Variables   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  134
4.2.4    Particle Histograms .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  136
4.2.5   Node Descriptors  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  139


<!-- Page 4 -->

4.2.6   Heap Structures for Rebalancing   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  141
4.2.7    Variables for Particle Transfer Scheduling   .  .  .  .  .  .  .  .  .  .  .  .  .  .  142
4.2.8    Variables for Family Communicators   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  145
4.2.9    Variables for Neighboring Information   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  146
4.2.10   Variables for Statistics and Verbose Messaging    .  .  .  .  .  .  .  .  .  .  .  147
4.2.11   Function Prototypes   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  152
4.2.12  Macro Verbose() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  155
4.3 C Source File ohhelp1.c    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  157
4.3.1   Header File Inclusion  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  157
4.3.2    Function Prototypes   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  157
4.3.3   oh1_init() and init1()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  158
4.3.4   mem_alloc()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  167
4.3.5   mem_alloc_error()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  167
4.3.6   errstop() and local_errstop() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  168
4.3.7   oh1_neighbors() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  168
4.3.8   oh1_families()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  169
4.3.9   set_total_particles() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  170
4.3.10  oh1_transbound() and transbound1()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  171
4.3.11  try_primary1()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  175
4.3.12  Macro Special_Pexc_Sched()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  176
4.3.13  try_stable1()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  176
4.3.14  count_stay() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  184
4.3.15  assign_particles() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  185
4.3.16  compare_int()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  188
4.3.17  schedule_particle_exchange()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  189
4.3.18  count_real_stay()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  193
4.3.19  sched_comm() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  193
4.3.20  make_comm_count()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  197
4.3.21  make_recv_count()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  200
4.3.22  make_send_count()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  200
4.3.23  count_next_particles()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  201
4.3.24  oh1_broadcast() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  201
4.3.25  rebalance1() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  203
4.3.26  build_new_comm()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  206
4.3.27  push_heap()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  209
4.3.28  pop_heap()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  210
4.3.29  remove_heap()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  210
4.3.30  oh1_accom_mode()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  211
4.3.31  oh1_all_reduce()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  212
4.3.32  oh1_reduce() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  213
4.3.33  oh1_init_stats()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  214
4.3.34  clear_stats()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  215
4.3.35  oh1_stats_time()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  215
4.3.36  stats_primary_comm()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  216
4.3.37  stats_secondary_comm()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  216
4.3.38  stats_comm() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  217
4.3.39  oh1_show_stats()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  218
4.3.40  Macro Round()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  219
4.3.41  update_stats()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  219
4.3.42  Macro Stats_Reduce_Part_{Min, Max, Sum}()  .  .  .  .  .  .  .  .  .  .  .  .  221


<!-- Page 5 -->

4.3.43  stats_reduce_part()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  221
4.3.44  print_stats()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  222
4.3.45  stats_reduce_time()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  223
4.3.46  oh1_print_stats()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  223
4.3.47  oh1_verbose()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  223
4.3.48  Macros Vprint() and Vprint_Norank()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  224
4.3.49  vprint()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  224
4.3.50  dprint()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  225
4.4  Header File ohhelp2.h    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  226
4.4.1   Header File Inclusion  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  226
4.4.2    Particle Buffers and Related Variables   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  226
4.4.3   Macro Particle_Spec() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  229
4.4.4   Macros Decl_Grid_Info(), Subdomain_Id() and Primarize_id()  229
4.4.5    Function Prototypes   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  231
4.5 C Source File ohhelp2.c    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  234
4.5.1   Header File Inclusion  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  234
4.5.2    Function Prototypes   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  234
4.5.3   oh2_init() and init2()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  235
4.5.4   oh2_transbound() and transbound2()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  238
4.5.5   try_primary2()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  239
4.5.6   exchange_primary_particles()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  240
4.5.7   try_stable2()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  243
4.5.8   rebalance2() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  244
4.5.9   move_to_sendbuf_primary()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  245
4.5.10  move_to_sendbuf_secondary() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  248
4.5.11  set_sendbuf_disps()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  252
4.5.12  exchange_particles()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  253
4.5.13  move_to_sendbuf_uw()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  257
4.5.14  move_to_sendbuf_dw()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  260
4.5.15  move_injected_to_sendbuf()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  261
4.5.16  move_injected_from_sendbuf()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  262
4.5.17  receive_particles()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  263
4.5.18  send_particles()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  264
4.5.19  oh2_inject_particle() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  265
4.5.20  oh2_remap_injected_particle()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  266
4.5.21  oh2_remove_injected_particle()    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  267
4.5.22  oh2_set_total_particles()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  268
4.5.23  oh2_max_local_particles()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  268
4.6  Header File ohhelp3.h    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  270
4.6.1    Control Variable   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  270
4.6.2   Domain and Subdomain Descriptors   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  270
4.6.3   Domain and Subdomain Boundaries   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  273
4.6.4    Field Array Descriptors   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  274
4.6.5   Boundary Communication Descriptors   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  275
4.6.6    Function Prototypes   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  277
4.7 C Source File ohhelp3.c    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  282
4.7.1   Header File Inclusion  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  282
4.7.2    Function Prototypes   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  282
4.7.3   oh3_init() and oh13_init() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  283
4.7.4   init3()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  287


<!-- Page 6 -->

4.7.5   init_subdomain_actively()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  289
4.7.6   init_subdomain_passively()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  293
4.7.7   comp_xyz()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  296
4.7.8   Macro Field_Disp() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  297
4.7.9   init_fields()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  297
4.7.10  set_field_descriptors()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  301
4.7.11  set_border_exchange() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  302
4.7.12  set_border_comm()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  304
4.7.13  clear_border_exchange()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  309
4.7.14  oh3_grid_size() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  310
4.7.15  oh3_transbound() and transbound3()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  311
4.7.16  Macro Map_Particle_To_Neighbor()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  312
4.7.17  Macro Neighbor_Id()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  312
4.7.18  oh3_map_particle_to_neighbor()    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  313
4.7.19  Macros Map_Particle_To_Subdomain() and Adjust_Subdomain()  314
4.7.20  oh3_map_particle_to_subdomain()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  315
4.7.21  map_irregular_subdomain()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  317
4.7.22  map_irregular() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  317
4.7.23  map_irregular_range() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  318
4.7.24  oh3_bcast_field()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  319
4.7.25  oh3_reduce_field() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  319
4.7.26  oh3_allreduce_field() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  320
4.7.27  oh3_exchange_borders()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  320
4.8  Level-4p Library Overview .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  323
4.9  Header File ohhelp4p.h  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  325
4.9.1    Constants   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  325
4.9.2   Macros for Grid-Position .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  325
4.9.3    Per-Grid Histograms and Related Variables   .  .  .  .  .  .  .  .  .  .  .  .  .  328
4.9.4    Variables for Particle Transfer Scheduling   .  .  .  .  .  .  .  .  .  .  .  .  .  .  335
4.9.5    Variables for Neighboring Information   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  341
4.9.6    Variable for Boundary Condition   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  342
4.9.7    Function Prototypes   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  343
4.10 C Source File ohhelp4p.c  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  346
4.10.1  Header File Inclusion  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  346
4.10.2   Function Prototypes   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  346
4.10.3  Macros If_Dim(), For_Z(), For_Y(), Do_Z(), Do_Y(),
Coord_To_Index() and Index_To_Coord()   .  .  .  .  .  .  .  .  .  .  .  .  .  351
4.10.4  Macros Decl_For_All_Grid(), For_All_Grid(),
For_All_Grid_Abs(), The_Grid(), Grid_X(), Grid_Y()
and Grid_Z()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  352
4.10.5   Constants URN_PRI, URN_SEC and URN_TRN  .  .  .  .  .  .  .  .  .  .  .  .  .  .  354
4.10.6  oh4p_init() and init4p()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  354
4.10.7  oh4p_max_local_particles()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  361
4.10.8  oh4p_per_grid_histogram()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  361
4.10.9  oh4p_transbound() and transbound4p()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  362
4.10.10 try_primary4p() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  364
4.10.11 try_stable4p()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  366
4.10.12 rebalance4p()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  367
4.10.13 Macros Parent_Old(), Parent_New(), Parent_New_Same() and
Parent_New_Diff()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  368


<!-- Page 7 -->

4.10.14 exchange_particles4p()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  369
4.10.15 exchange_population() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  371
4.10.16 add_population()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  373
4.10.17 mpi_allreduce_wrapper()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  374
4.10.18 reduce_population()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  374
4.10.19 make_recv_list()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  375
4.10.20 sched_recv() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  379
4.10.21 make_send_sched()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  383
4.10.22 make_send_sched_body()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  386
4.10.23 gather_hspot_recv()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  390
4.10.24 gather_hspot_send()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  392
4.10.25 gather_hspot_send_body() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  393
4.10.26 scatter_hspot_send()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  395
4.10.27 scatter_hspot_recv()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  398
4.10.28 scatter_hspot_recv_body()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  399
4.10.29 update_descriptors()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  401
4.10.30 update_neighbors() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  401
4.10.31 set_grid_descriptor() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  402
4.10.32 adjust_field_descriptor()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  403
4.10.33 update_real_neighbors()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  404
4.10.34 upd_real_nbr()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  408
4.10.35 exchange_xfer_amount()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  409
4.10.36 count_population() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  410
4.10.37 sort_particles()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  411
4.10.38 move_and_sort_primary()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  413
4.10.39 sort_received_particles()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  415
4.10.40 Macros Local_Grid_Position() and Move_Or_Do()  .  .  .  .  .  .  .  .  416
4.10.41 move_to_sendbuf_sec4p()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  418
4.10.42 move_to_sendbuf_uw4p()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  420
4.10.43 move_to_sendbuf_dw4p()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  422
4.10.44 move_and_sort_secondary()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  422
4.10.45 set_sendbuf_disps4p() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  424
4.10.46 xfer_particles()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  425
4.10.47 Macro Check_Particle_Location()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  427
4.10.48 Macros Map_Particle_To_Neighbor() and
Adjust_Neighbor_Grid()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  428
4.10.49 oh4p_map_particle_to_neighbor()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  429
4.10.50 Macros Map_To_Grid, Map_Particle_To_Subdomain() and
Local_Coordinate() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  432
4.10.51 oh4p_map_particle_to_subdomain()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  434
4.10.52 oh4p_inject_particle()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  436
4.10.53 oh4p_remove_mapped_particle()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  437
4.10.54 oh4p_remap_particle_to_neighbor()    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  438
4.10.55 oh4p_remap_particle_to_subdomain()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  439
4.11 Level-4s Library Overview  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  440
4.12 Header File ohhelp4s.h  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  442
4.12.1   Constants   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  442
4.12.2  Macros for Grid-Position .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  442
4.12.3   Per-Grid Histograms and Related Variables   .  .  .  .  .  .  .  .  .  .  .  .  .  444
4.12.4   Variables for Particle Transfer Scheduling   .  .  .  .  .  .  .  .  .  .  .  .  .  .  454


<!-- Page 8 -->

4.12.5   Variables for Neighboring Information   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  458
4.12.6   Variable for Boundary Condition   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  459
4.12.7   Function Prototypes   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  459
4.13 C Source File ohhelp4s.c  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  462
4.13.1  Header File Inclusion  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  462
4.13.2   Function Prototypes   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  462
4.13.3  Macros If_Dim(), For_Y(), For_Z(), Do_Y(), Do_Z() and
Coord_To_Index()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  467
4.13.4  Macros Decl_For_All_Grid(), For_All_Grid(),
For_All_Grid_Abs(), The_Grid(), Grid_X(), Grid_Y()
and Grid_Z()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  468
4.13.5   Constants URN_PRI, URN_SEC and URN_TRN  .  .  .  .  .  .  .  .  .  .  .  .  .  .  469
4.13.6  oh4s_init() and init4s()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  469
4.13.7  oh4s_particle_buffer()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  478
4.13.8  oh4s_per_grid_histogram()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  479
4.13.9  oh4s_transbound() and transbound4s()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  480
4.13.10 try_primary4s() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  482
4.13.11 try_stable4s()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  483
4.13.12 rebalance4s()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  483
4.13.13 Macros Parent_Old(), Parent_New(), Parent_New_Same() and
Parent_New_Diff()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  484
4.13.14 exchange_particles4s()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  485
4.13.15 count_population() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  492
4.13.16 exchange_population() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  492
4.13.17 reduce_population()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  494
4.13.18 add_population()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  495
4.13.19 make_recv_list()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  496
4.13.20 sched_recv() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  499
4.13.21 make_send_sched()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  500
4.13.22 Macros For_All_Grid_Z(), For_All_Grid_XY(),
Grid_Exterior_Boundary() and Grid_Interior_Boundary()   .  .  503
4.13.23 make_send_sched_body()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  505
4.13.24 make_send_sched_self()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  507
4.13.25 make_send_sched_hplane() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  511
4.13.26 update_descriptors()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  512
4.13.27 update_neighbors() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  512
4.13.28 set_grid_descriptor() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  513
4.13.29 adjust_field_descriptor()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  514
4.13.30 update_real_neighbors()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  515
4.13.31 upd_real_nbr()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  516
4.13.32 exchange_xfer_amount()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  516
4.13.33 make_bxfer_sched() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  518
4.13.34 Macros Add_Pillar_Voxel(), Is_Pillar_Voxel(),
Pillar_Lower() and Pillar_Upper()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  519
4.13.35 make_bsend_sched() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  520
4.13.36 make_brecv_sched() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  523
4.13.37 Macros Local_Grid_Position() and Move_Or_Do()  .  .  .  .  .  .  .  .  524
4.13.38 move_to_sendbuf_4s()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  526
4.13.39 move_to_sendbuf_uw4s()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  529
4.13.40 move_to_sendbuf_dw4s()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  530


<!-- Page 9 -->

4.13.41 Macro Sort_Particle() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  531
4.13.42 sort_particles()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  532
4.13.43 move_and_sort() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  533
4.13.44 sort_received_particles()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  535
4.13.45 set_sendbuf_disps4s() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  535
4.13.46 xfer_particles()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  536
4.13.47 xfer_boundary_particles_v() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  537
4.13.48 xfer_boundary_particles_h() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  540
4.13.49 oh4s_exchange_border_data() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  542
4.13.50 exchange_border_data_v() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  542
4.13.51 exchange_border_data_h() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  545
4.13.52 Macro Check_Particle_Location()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  546
4.13.53 Macros Map_Particle_To_Neighbor() and
Adjust_Neighbor_Grid()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  547
4.13.54 oh4s_map_particle_to_neighbor()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  548
4.13.55 Macros Map_To_Grid, Map_Particle_To_Subdomain() and
Local_Coordinate() .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  549
4.13.56 oh4s_map_particle_to_subdomain()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  550
4.13.57 oh4s_inject_particle()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  551
4.13.58 oh4s_remove_mapped_particle()   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  552
4.13.59 oh4s_remap_particle_to_neighbor()    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  552
4.13.60 oh4s_remap_particle_to_subdomain()  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  553
4.14 Sample make Files   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  554
4.14.1   samplef.mk for Fortran  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  554
4.14.2   samplec.mk for C   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  555
