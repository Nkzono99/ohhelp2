/* Internal Level-2 state globals retained during the v2 context migration.
   Public users should include ohhelp_c.h or ohhelp2.h, not this file. */
#ifndef OHHELP2_INTERNAL_H
#define OHHELP2_INTERNAL_H

#include "ohhelp2.h"

EXTERN int nOfLocalPLimit;
EXTERN void *Particles;                 /* [nOfLocalPLimit] */
EXTERN void *SendBuf;                   /* [nOfLocalPLimit] */
EXTERN void **RecvBufBases;             /* [2][nOfSpecies] */
EXTERN int *secondaryBase, *totalLocalParticles;
EXTERN int *SendBufDisps;               /* [nOfSpecies][nOfNodes] */
EXTERN int *RecvBufDisps;               /* [nOfNodes] */
EXTERN int nOfInjections;
EXTERN int specBase;
EXTERN MPI_Datatype T_Particle;
EXTERN int ownsTParticle;
EXTERN MPI_Datatype CustomTParticle;
EXTERN int useCustomTParticle;
EXTERN int ownsCustomTParticle;
EXTERN oh_particle_adapter ParticleAdapter;
EXTERN oh_particle_adapter CustomParticleAdapter;
EXTERN int useCustomParticleAdapter;
EXTERN MPI_Request *Requests;           /* [nOfNodes*nOfSpecies*2*2] */
EXTERN MPI_Status *Statuses;            /* [nOfNodes*nOfSpecies*2*2] */

#ifdef OH_POS_AWARE
EXTERN int gridMask, logGrid;
EXTERN int AbsNeighbors[2][OH_NEIGHBORS];
#endif

/* Prototypes for the functions called from higher-level library code */
void init2(int **sdid, int nspec, int maxfrac, int **nphgram,
           int **totalp, void **pbuf, int **pbase, int maxlocalp,
           struct S_mycommc *mycommc, struct S_mycommf *mycommf,
           int **nbor, int *pcoord, int stats, int repiter, int verbose);
void *oh2_bind_particle_buffer_state(struct oh_state *state, void *particles,
                                     int maxlocalp, int ownership);
void oh2_unbind_particle_buffer_state(struct oh_state *state);
int  transbound2_state(struct oh_state *state, int currmode, int stats,
                       int level);
void oh2_set_particle_mpi_type_state(struct oh_state *state,
                                     MPI_Datatype type);
void oh2_set_particle_adapter_state(struct oh_state *state,
                                    const oh_particle_adapter *adapter);
void *oh2_inject_particle_state(struct oh_state *state, void *part);
void oh2_remap_injected_particle_state(struct oh_state *state, void *part);
void oh2_remove_injected_particle_state(struct oh_state *state, void *part);
void oh2_set_total_particles_state(struct oh_state *state);
void exchange_primary_particles_state(struct oh_state *state, int currmode,
                                      int stats);
void move_to_sendbuf_primary_state(struct oh_state *state, int secondary,
                                   int stats);
void set_sendbuf_disps_state(struct oh_state *state, int secondary,
                             int parent);
void exchange_particles_state(struct oh_state *state,
                              struct S_commlist *secrlist, int secrlsize,
                              int oldparent, int neighboring, int currmode,
                              int stats);

#endif
