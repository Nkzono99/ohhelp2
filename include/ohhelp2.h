/* File: ohhelp2.h
   Version 1.1.1 (2015/10/23)
   Copyright (C) 2009-2015  Hiroshi Nakashima <h.nakashima@media.kyoto-u.ac.jp>
                            (ACCMS, Kyoto University)
   This program can be freely used, redistributed and modified for non-
   commercial purpose providing that the copyright notice above remains
   unchanged.
*/
#include "oh_part.h"
#include "oh_particle_adapter.h"

#ifdef OH_HAS_SPEC
#define Particle_Spec(S) (S)
#else
#define Particle_Spec(S) (0)
#endif

/* Prototypes for the functions called from simulator code */
void oh2_set_total_particles();
int  oh2_max_local_particles(dint npmax, int maxfrac, int minmargin);
void oh2_set_particle_mpi_type(MPI_Datatype type);
void oh2_set_particle_adapter(const oh_particle_adapter *adapter);
void oh2_inject_particle(struct S_particle *part);
void oh2_remap_injected_particle(struct S_particle *part);
void oh2_remove_injected_particle(struct S_particle *part);
void oh2_init(int **sdid, int nspec, int maxfrac, int **nphgram,
              int **totalp, struct S_particle **pbuf, int **pbase,
              int maxlocalp, void *mycomm, int **nbor,
              int *pcoord, int stats, int repiter, int verbose);
int  oh2_transbound(int currmode, int stats);

void oh2_set_total_particles_();
int  oh2_max_local_particles_(dint *npmax, int *maxfrac, int *minmargin);
void oh2_set_particle_mpi_type_(int *type);
void oh2_inject_particle_(struct S_particle *part);
void oh2_remap_injected_particle_(struct S_particle *part);
void oh2_remove_injected_particle_(struct S_particle *part);
void oh2_init_(int *sdid, int *nspec, int *maxfrac, int *nphgram,
               int *totalp, struct S_particle *pbuf, int *pbase,
               int *maxlocalp, struct S_mycommf *mycomm, int *nbor,
               int *pcoord, int *stats, int *repiter, int *verbose);
int  oh2_transbound_(int *currmode, int *stats);
