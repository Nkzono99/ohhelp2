/* Internal Level-4s declarations. */
#ifndef OHHELP4S_INTERNAL_H
#define OHHELP4S_INTERNAL_H

#include "ohhelp4s.h"
#include "ohhelp4_internal.h"

EXTERN int** NOfPGridOutShadow[2];                      /* [2][ns][z][y][x] */
EXTERN int** NOfPGridIndex[2], ** NOfPGridIndexShadow[2];/* [2][ns][z][y][x] */
EXTERN dint* NOfPGridZ;                                 /* [z] */
EXTERN int ZBound[2][2], (*ZBoundShadow)[2];

struct S_hplane {
    int nbor, stag, rtag;
    int* nsend, * nrecv, * sbuf, * rbuf;                 /* [ns] */
    };

EXTERN struct S_hplane HPlane[2][2];                    /* [2][2] */

struct S_vplane {
    int nbor, stag, rtag;
    int nsend, nrecv, sbuf, rbuf;
    };

EXTERN struct S_vplane* VPlane;                         /* [2*nn+6] */
EXTERN int VPlaneHead[2 * 2 * 2 + 1];
EXTERN struct S_particle* BoundarySendBuf;

struct S_interiorp {
    int head, size;
    };

EXTERN struct S_interiorp* InteriorParts;
EXTERN struct S_commlist PrimaryCommList[2][OH_NEIGHBORS];
EXTERN int AltSecRLIndex[OH_NEIGHBORS + 1];
EXTERN int PrimaryRLIndex[OH_NEIGHBORS];

struct S_recvsched_context {
    int z;
    dint nptotal, nplimit;
    struct S_commlist* cptr;
    };

#endif
