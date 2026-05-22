/* Internal Level-3 declarations retained during the v2 context migration. */
#ifndef OHHELP3_INTERNAL_H
#define OHHELP3_INTERNAL_H

#include "ohhelp3.h"

EXTERN int excludeLevel2;

EXTERN int (*SubDomains)[OH_DIMENSION][2];      /* [N][D][l,u] */
EXTERN double (*SubDomainsFloat)[OH_DIMENSION][2];

struct S_grid {
  int n, coord[2], size;
  double fcoord[2], fsize, gsize, rgsize;
  struct {
    int size, n, thresh;
    double rfsize, rfsizeplus, fthresh;
  } light;
};

EXTERN struct S_grid Grid[3];

struct S_subdomdesc {
  struct {
    int c[2], h, n;
    double fc[2];
  } coord[OH_DIMENSION];
  int id;
};

EXTERN struct S_subdomdesc *SubDomainDesc;

struct S_message {
  char xyz[4];
  char loup[2][6];
};

static const struct S_message Message = {
  "xyz",
  {"lower", "upper"}
};

EXTERN int nOfBoundaries;
EXTERN int (*Boundaries)[OH_DIMENSION][2];      /* [N][D][l,u] */
EXTERN int Adjacent[OH_DIMENSION][2];           /* [D][l,u] */
EXTERN int nOfFields;
EXTERN int (*FieldTypes)[OH_FTYPE_N];           /* [F][es,lo,up,bl,bu,rl,ru] */

struct S_brdesc {
  int base, size[2];
};

struct S_flddesc {
  int esize, ext[2], size[OH_DIMENSION];
  struct S_brdesc bc, red;
};

EXTERN struct S_flddesc *FieldDesc;             /* [F] */
EXTERN int nOfExc;
EXTERN int *BoundaryCommFields;                 /* [C] */
EXTERN int (*BoundaryCommTypes)[2][OH_CTYPE_N]; /* [C][B][d,u][from,to,size] */

struct S_bcomm {
  int buf, count, deriv;
  MPI_Datatype type;
};

struct S_borderexc {
  struct S_bcomm send, recv;
};

EXTERN struct S_borderexc (*BorderExc)[2][OH_DIMENSION][2];
                                                        /* [C][ps][D][l,u] */

void init3(int **sdid, int nspec, int maxfrac, int **nphgram, int **totalp,
           int **rcounts, int **scounts, struct S_particle **pbuf, int **pbase,
           int maxlocalp, struct S_mycommc *mycommc, struct S_mycommf *mycommf,
           int **nbor, int *pcoord, int **sdoms, int *scoord, int nbound,
           int *bcond, int **bounds, int *ftypes, int *cfields, int cfid,
           int *ctypes, int **fsizes, int stats, int repiter, int verbose,
           int skip2);
void state_set_field_descriptors(struct oh_state *state,
                                 int (*ft)[OH_FTYPE_N],
                                 int sd[OH_DIMENSION][2], int ps);
void state_clear_border_exchange(struct oh_state *state);
int  map_irregular_subdomain(double x, double y, double z);

#endif
