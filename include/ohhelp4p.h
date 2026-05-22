/* File: ohhelp4p.h
   Version 1.1.1 (2015/10/23)
   Copyright (C) 2009-2015  Hiroshi Nakashima <h.nakashima@media.kyoto-u.ac.jp>
                            (ACCMS, Kyoto University)
   This program can be freely used, redistributed and modified for non-
   commercial purpose providing that the copyright notice above remains
   unchanged.
*/
#ifndef OHHELP4P_H
#define OHHELP4P_H

#define OH_PGRID_EXT 1
#define OH_NBR_SELF (OH_NEIGHBORS>>1)

/* Prototypes for the functions called from simulator code */
void oh4p_init(int** sdid, const int nspec, const int maxfrac, int** totalp,
    struct S_particle** pbuf, int** pbase, const int maxlocalp,
    void* mycomm, int** nbor, int* pcoord, int** sdoms, int* scoord,
    const int nbound, int* bcond, int** bounds, int* ftypes,
    int* cfields, int* ctypes, int** fsizes,
    const int stats, const int repiter, const int verbose);
int  oh4p_max_local_particles(const dint npmax, const int maxfrac,
    const int minmargin, const int hsthresh);
void oh4p_per_grid_histogram(int** pghgram);
int  oh4p_transbound(int currmode, int stats);
int  oh4p_map_particle_to_neighbor(struct S_particle* part, const int ps,
    const int s);
int  oh4p_map_particle_to_subdomain(struct S_particle* part, const int ps,
    const int s);
int  oh4p_inject_particle(const struct S_particle* part, const int ps);
void oh4p_remove_mapped_particle(struct S_particle* part, const int ps,
    const int s);
int  oh4p_remap_particle_to_neighbor(struct S_particle* part, const int ps,
    const int s);
int  oh4p_remap_particle_to_subdomain(struct S_particle* part, const int ps,
    const int s);

void oh4p_init_(int* sdid, const int* nspec, const int* maxfrac, int* totalp,
    struct S_particle* pbuf, int* pbase, const int* maxlocalp,
    struct S_mycommf* mycomm, int* nbor, int* pcoord, int* sdoms,
    int* scoord, const int* nbound, int* bcond, int* bounds,
    int* ftypes, int* cfields, int* ctypes, int* fsizes,
    const int* stats, const int* repiter, const int* verbose);
int  oh4p_max_local_particles_(const dint* npmax, const int* maxfrac,
    const int* minmargin, const int* hsthresh);
void oh4p_per_grid_histogram_(int* pghgram);
int  oh4p_transbound_(int* currmode, int* stats);
int  oh4p_map_particle_to_neighbor_(struct S_particle* part, const int* ps,
    const int* s);
int  oh4p_map_particle_to_subdomain_(struct S_particle* part, const int* ps,
    const int* s);
int  oh4p_inject_particle_(const struct S_particle* part, const int* ps);
void oh4p_remove_mapped_particle_(struct S_particle* part, const int* ps,
    const int* s);
int  oh4p_remap_particle_to_neighbor_(struct S_particle* part, const int* ps,
    const int* s);
int  oh4p_remap_particle_to_subdomain_(struct S_particle* part, const int* ps,
    const int* s);

#endif
