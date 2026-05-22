#ifndef OHHELP1_INTERNAL_H
#define OHHELP1_INTERNAL_H

#include "ohhelp1.h"

#ifndef EXTERN
#define EXTERN extern
#endif

EXTERN MPI_Comm fam_comm;

/* Basic process configuration variables */
EXTERN int nOfNodes;
EXTERN int myRank;
EXTERN int RegionId[2], *SubdomainId;
EXTERN int currMode, accMode;

/* Number of particles and related variables */
EXTERN int  nOfSpecies;
EXTERN int  maxFraction;
EXTERN int  *NOfPLocal;                 /* [2][nOfSpecies][nOfNodes] */
EXTERN int  *NOfPrimaries;              /* [2][nOfSpecies][nOfNodes] */
EXTERN dint *TotalPGlobal;              /* [nOfNodes+1] */
EXTERN double *RegionWeights;           /* [nOfNodes] */
EXTERN double *TotalLoadGlobal;         /* [nOfNodes] */
EXTERN dint nOfParticles;
EXTERN double nOfLoad;
EXTERN int  nOfLocalPMax;
EXTERN double nOfLocalLoadMax;
EXTERN int  weightedLoadBalancing;
EXTERN dint *NOfPToStay;                /* [nOfNodes] */
EXTERN int  *TotalP;                    /* [2][nOfSpecies] */
EXTERN int  *TotalPNext;                /* [2][nOfSpecies] */
EXTERN int  primaryParts, totalParts;
EXTERN int  *NOfRecv, *RecvCounts;      /* [2][nOfSpecies][nOfNodes] */
EXTERN int  *NOfSend, *SendCounts;      /* [2][nOfSpecies][nOfNodes] */
EXTERN int  *InjectedParticles;         /* [2][2][nOfSpecies] */
EXTERN int  *TempArray;                 /* [nOfNodes] */
EXTERN MPI_Datatype T_Histogram;

/* Computation node descriptors */
EXTERN struct S_node *Nodes, *NodesNext, **NodeQueue;

/* Heap structure for load rebalancing */
EXTERN struct S_heap LessHeap, GreaterHeap;

/* Structured variables for particle transfer */
EXTERN struct S_commlist *CommList, *SecRList;
EXTERN int RLIndex[OH_NEIGHBORS+1];
EXTERN int SLHeadTail[2], SecSLHeadTail[2], SecRLSize;
EXTERN MPI_Datatype T_Commlist;

/* Structured variables for MPI communicator */
EXTERN MPI_Group GroupWorld;
EXTERN struct S_comms Comms;
EXTERN struct S_mycommc *MyComm, *MyCommC;
EXTERN struct S_mycommf *MyCommF;

/* Neighboring information */
EXTERN int Neighbors[3][OH_NEIGHBORS], SrcNeighbors[OH_NEIGHBORS];
  /* <BSW,BS,BSE,BW,B,BE,BNW,BN,BNE,    : 00..04..08
       SW, S, SE, W,O  E, NW, N, NE,    : 09..13..17
      TSW,TS,TSE,TW,T,TE,TNW,TN,TNE>    : 18..22..26 */
EXTERN int *DstNeighbors;

/* Statistics and verbose messaging */
EXTERN struct S_stats Stats;
EXTERN MPI_Datatype T_StatsTime;
EXTERN MPI_Op Op_StatsTime, Op_StatsPart;
EXTERN int statsMode, reportIteration, verboseMode;

/* Macro for verbose messaging. */
#define Verbose(L,VP) {\
  if (verboseMode>=L) {\
    MPI_Barrier(MCW);\
    if (myRank==0 || verboseMode>=3) VP;\
  }\
}

#endif
