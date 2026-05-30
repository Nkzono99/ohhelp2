/* Shared internal Level-4 declarations retained during the v2 context
   migration. Public users should include ohhelp_c.h or the level header. */
#ifndef OHHELP4_INTERNAL_H
#define OHHELP4_INTERNAL_H

struct S_griddesc {
    int x, y, z, w, d, h, dw;
    };

struct S_realneighbor {
    int n, * nbor;
    };

EXTERN int* PbufIndex;                                  /* [2*ns+1] */
EXTERN dint** NOfPGrid[2], ** NOfPGridTotal[2];          /* [2][ns][z][y][x] */
EXTERN int** NOfPGridOut[2];                            /* [2][ns][z][y][x] */
EXTERN struct S_griddesc GridDesc[3];
EXTERN struct S_commlist* AltSecRList;
EXTERN int SecRLIndex[OH_NEIGHBORS + 1];
EXTERN MPI_Datatype T_Hgramhalf;
EXTERN int FirstNeighbor[OH_NEIGHBORS], GridOffset[2][OH_NEIGHBORS];
EXTERN struct S_realneighbor RealDstNeighbors[2][2], RealSrcNeighbors[2][2];
EXTERN int BoundaryCondition[OH_DIMENSION][2];

static inline void
level4_fail_if_weighted_secondary_transbound(struct oh_state* state,
                                             int currmode) {
    if (state->weighted_load_balancing && Mode_PS(state->curr_mode) &&
        Mode_PS(currmode))
        local_errstop("Level 4 weighted secondary transbound is not supported");
}

#endif
