/* File: ohhelp4s.h
   Version 1.1.1 (2015/10/23)
   Copyright (C) 2009-2015  Hiroshi Nakashima <h.nakashima@media.kyoto-u.ac.jp>
                            (ACCMS, Kyoto University)
   This program can be freely used, redistributed and modified for non-
   commercial purpose providing that the copyright notice above remains
   unchanged.
*/
#ifndef OHHELP4S_H
#define OHHELP4S_H

#define OH_PGRID_EXT    1
#define OH_NBR_SELF     (OH_NEIGHBORS>>1)
#define OH_NBR_BCC      (1+1*3+0*3*3)
#define OH_NBR_TCC      (1+1*3+2*3*3)

#ifdef __cplusplus
extern "C" {
#endif

/* Level 4s is a default-context API. Configure adapters and region weights
   through the default-context/legacy Level 1-3 entry points before calling
   these functions; heap-owned oh_context Level 4 APIs are not exposed yet. */
void oh4s_init(int** sdid, const int nspec, const int maxfrac,
    const dint npmax, const int minmargin,
    const int maxdensity, int** totalp, int** pbase,
    int* maxlocalp, int* cbufsize, void* mycomm, int** nbor,
    int* pcoord, int** sdoms, int* scoord, const int nbound,
    int* bcond, int** bounds, int* ftypes, int* cfields,
    int* ctypes, int** fsizes, int** zbound,
    const int stats, const int repiter, const int verbose);
void oh4s_particle_buffer(const int maxlocalp, void** pbuf);
void oh4s_per_grid_histogram(int** pghgram, int** pgindex);
int  oh4s_transbound(int currmode, int stats);
void oh4s_exchange_border_data(void* buf, void* sbuf, void* rbuf,
    MPI_Datatype type);
int  oh4s_map_particle_to_neighbor(void* part, const int ps,
    const int s);
int  oh4s_map_particle_to_subdomain(void* part, const int ps,
    const int s);
int  oh4s_inject_particle(const void* part, const int ps);
void oh4s_remove_mapped_particle(void* part, const int ps,
    const int s);
int  oh4s_remap_particle_to_neighbor(void* part, const int ps,
    const int s);
int  oh4s_remap_particle_to_subdomain(void* part, const int ps,
    const int s);

void oh4s_init_(int* sdid, const int* nspec, const int* maxfrac,
    const dint* npmax, const int* minmargin, const int* maxdensity,
    int* totalp, int* pbase, int* maxlocalp, int* cbufsize,
    struct S_mycommf* mycomm, int* nbor, int* pcoord, int* sdoms,
    int* scoord, const int* nbound, int* bcond, int* bounds,
    int* ftypes, int* cfields, int* ctypes, int* fsizes,
    int* zbound,
    const int* stats, const int* repiter, const int* verbose);
void oh4s_particle_buffer_(const int* maxlocalp, struct S_particle* pbuf);
void oh4s_per_grid_histogram_(int* pghgram, int* pgindex);
int  oh4s_transbound_(int* currmode, int* stats);
void oh4s_exchange_border_data_(void* buf, void* sbuf, void* rbuf, int* type);
int  oh4s_map_particle_to_neighbor_(struct S_particle* part, const int* ps,
    const int* s);
int  oh4s_map_particle_to_subdomain_(struct S_particle* part, const int* ps,
    const int* s);
int  oh4s_inject_particle_(const struct S_particle* part, const int* ps);
void oh4s_remove_mapped_particle_(struct S_particle* part, const int* ps,
    const int* s);
int  oh4s_remap_particle_to_neighbor_(struct S_particle* part, const int* ps,
    const int* s);
int  oh4s_remap_particle_to_subdomain_(struct S_particle* part, const int* ps,
    const int* s);

#ifdef __cplusplus
}
#endif

#endif
