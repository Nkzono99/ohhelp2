/* Internal Level-2 state globals retained during the v2 context migration.
   Public users should include ohhelp_c.h or ohhelp2.h, not this file. */
#ifndef OHHELP2_INTERNAL_H
#define OHHELP2_INTERNAL_H

#include "ohhelp2.h"

EXTERN int nOfLocalPLimit;
EXTERN struct S_particle *Particles;    /* [nOfLocalPLimit] */
EXTERN struct S_particle *SendBuf;      /* [nOfLocalPLimit] */
EXTERN struct S_particle **RecvBufBases;/* [2][nOfSpecies] */
EXTERN int *secondaryBase, *totalLocalParticles;
EXTERN int *SendBufDisps;               /* [nOfSpecies][nOfNodes] */
EXTERN int *RecvBufDisps;               /* [nOfNodes] */
EXTERN int nOfInjections;
EXTERN int specBase;
EXTERN MPI_Datatype T_Particle;
EXTERN MPI_Datatype CustomTParticle;
EXTERN int useCustomTParticle;
EXTERN oh_particle_adapter ParticleAdapter;
EXTERN oh_particle_adapter CustomParticleAdapter;
EXTERN int useCustomParticleAdapter;
EXTERN MPI_Request *Requests;           /* [nOfNodes*nOfSpecies*2*2] */
EXTERN MPI_Status *Statuses;            /* [nOfNodes*nOfSpecies*2*2] */

#ifdef OH_POS_AWARE
EXTERN int gridMask, logGrid;
EXTERN int AbsNeighbors[2][OH_NEIGHBORS];
#endif

#endif
