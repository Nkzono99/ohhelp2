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
struct S_node {
  struct {int prime, sec;} stay;
  struct {dint prime, sec;} get;
  struct {int prime, sec, black, rank;} comm;
  struct S_node *parent, *sibling, *child;
  int id, parentid;
};
EXTERN struct S_node *Nodes, *NodesNext, **NodeQueue;

/* Heap structure for load rebalancing */
struct S_heap {
  int n, *node, *index;
};
EXTERN struct S_heap LessHeap, GreaterHeap;

/* Structured variables for particle transfer */
struct S_commlist {
  int sid, rid, region, count, tag;     /* tag = spec + nOfSpecies*sec */
};
struct S_commsched_context {
  int neighbor, sender, spec, comidx, dones, donen;
};
EXTERN struct S_commlist *CommList, *SecRList;
EXTERN int RLIndex[OH_NEIGHBORS+1];
EXTERN int SLHeadTail[2], SecSLHeadTail[2], SecRLSize;
EXTERN MPI_Datatype T_Commlist;

/* Structured variables for MPI communicator */
struct S_comms {
  int n;
  MPI_Comm *body;       /* [nOfNodes] */
};
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
struct S_statscurr {
  struct {
    double value, val[2*STATS_TIMINGS+2];
    int key, ev[2*STATS_TIMINGS+2];
  } time;
  dint part[STATS_PARTS];
};
struct S_statstime {
  double min, max, total;
  int ev;
};
struct S_statspart {
  dint min, max, total;
};
struct S_statstotal {
  struct S_statstime time[2*STATS_TIMINGS];
  struct S_statspart part[STATS_PARTS];
};
struct S_stats {
  struct S_statscurr curr;
  struct S_statstotal subtotal, total;
};
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
