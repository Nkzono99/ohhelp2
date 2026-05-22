/* Internal Level-4p declarations. */
#ifndef OHHELP4P_INTERNAL_H
#define OHHELP4P_INTERNAL_H

#include "ohhelp4p.h"
#include "ohhelp4_internal.h"

EXTERN int gridOverflowLimit;

struct S_recvsched_context {
    int x, y, z, g, hs;
    dint nptotal, nplimit, carryover;
    struct S_commlist* cptr;
    };

struct S_hotspot {
    int g, n, lev, self;
    struct S_commlist* comm;
    struct S_hotspot* next;
    };

EXTERN struct S_hotspot* HotSpotList, * HotSpotTop;      /* [2*nn+2*3^D+1] */

struct S_hotspotbase {
    struct S_hotspot* head, * tail;
    };

EXTERN struct S_hotspotbase HotSpot[3][OH_NEIGHBORS];
EXTERN int* HSRecv[OH_NEIGHBORS];                       /* [3^D][nn][ns] */
EXTERN int* HSSend, * HSRecvFromParent, * HSReceiver;     /* [ns] */

#endif
