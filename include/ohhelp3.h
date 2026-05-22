/* File: ohhelp3.h
   Version 1.1.1 (2015/10/23)
   Copyright (C) 2009-2015  Hiroshi Nakashima <h.nakashima@media.kyoto-u.ac.jp>
                            (ACCMS, Kyoto University)
   This program can be freely used, redistributed and modified for non-
   commercial purpose providing that the copyright notice above remains
   unchanged.
*/
#define OH_LOWER 0
#define OH_UPPER 1

#define OH_FTYPE_ES 0
#define OH_FTYPE_LO 1
#define OH_FTYPE_UP 2
#define OH_FTYPE_BL 3
#define OH_FTYPE_BU 4
#define OH_FTYPE_RL 5
#define OH_FTYPE_RU 6
#define OH_FTYPE_N  7

#define OH_CTYPE_FROM 0
#define OH_CTYPE_TO   1
#define OH_CTYPE_SIZE 2
#define OH_CTYPE_N    3

/* Prototypes for the functions called from simulator code */
void oh3_grid_size(double size[OH_DIMENSION]);
void oh3_particle_adapter_use_position_fields(oh_particle_adapter *adapter,
                                              size_t x_offset,
                                              size_t y_offset,
                                              size_t z_offset);
void oh3_bcast_field(void *pfld, void *sfld, int ftype);
void oh3_reduce_field(void *pfld, void *sfld, int ftype);
void oh3_allreduce_field(void *pfld, void *sfld, int ftype);
void oh3_exchange_borders(void *pfld, void *sfld, int ctype, int bcast);

void oh3_grid_size_(double size[OH_DIMENSION]);
void oh3_bcast_field_(void *pfld, void *sfld, int *ftype);
void oh3_reduce_field_(void *pfld, void *sfld, int *ftype);
void oh3_allreduce_field_(void *pfld, void *sfld, int *ftype);
void oh3_exchange_borders_(void *pfld, void *sfld, int *ctype, int *bcast);

#if OH_DIMENSION==1
int  oh3_map_particle_to_neighbor(double *x, int ps);
int  oh3_map_particle_to_subdomain(double x);
int  oh3_map_region_to_adjacent_node_(double *x, int *ps);
int  oh3_map_particle_to_neighbor_(double *x, int *ps);
int  oh3_map_region_to_node_(double *x);
int  oh3_map_particle_to_subdomain_(double *x);
#elif OH_DIMENSION==2
int  oh3_map_particle_to_neighbor(double *x, double *y, int ps);
int  oh3_map_particle_to_subdomain(double x, double y);
int  oh3_map_region_to_adjacent_node_(double *x, double *y, int *ps);
int  oh3_map_particle_to_neighbor_(double *x, double *y, int *ps);
int  oh3_map_region_to_node_(double *x, double *y);
int  oh3_map_particle_to_subdomain_(double *x, double *y);
#else
int  oh3_map_particle_to_neighbor(double *x, double *y, double *z, int ps);
int  oh3_map_particle_to_subdomain(double x, double y, double z);
int  oh3_map_region_to_adjacent_node_(double *x, double *y, double *z,
                                      int *ps);
int  oh3_map_particle_to_neighbor_(double *x, double *y, double *z,
                                   int *ps);
int  oh3_map_region_to_node_(double *x, double *y, double *z);
int  oh3_map_particle_to_subdomain_(double *x, double *y, double *z);
#endif

void oh3_init(int **sdid, int nspec, int maxfrac, int **nphgram, int **totalp,
              struct S_particle **pbuf, int **pbase, int maxlocalp,
              void *mycomm, int **nbor, int *pcoord,
              int **sdoms, int *scoord, int nbound, int *bcond, int **bounds,
              int *ftypes, int *cfields, int *ctypes, int **fsizes,
              int stats, int repiter, int verbose);
void oh13_init(int **sdid, int nspec, int maxfrac, int **nphgram,
               int **totalp, int **rcounts, int **scounts,
               void *mycomm, int **nbor, int *pcoord,
               int **sdoms, int *scoord, int nbound, int *bcond, int **bounds,
               int *ftypes, int *cfields, int *ctypes, int **fsizes,
               int stats, int repiter, int verbose);
int  oh3_transbound(int currmode, int stats);

void oh3_init_(int *sdid, int *nspec, int *maxfrac, int *nphgram,
               int *totalp, struct S_particle *pbuf, int *pbase,
               int *maxlocalp, struct S_mycommf *mycomm, int *nbor,
               int *pcoord, int *sdoms, int *scoord, int *nbound, int *bcond,
               int *bounds, int *ftypes, int *cfields, int *ctypes,
               int *fsizes, int *stats, int *repiter, int *verbose);
void oh13_init_(int *sdid, int *nspec, int *maxfrac, int *nphgram,
                int *totalp, int *rcounts, int *scounts,
                struct S_mycommf *mycomm, int *nbor, int *pcoord,
                int *sdoms, int *scoord, int *nbound, int *bcond, int *bounds,
                int *ftypes, int *cfields, int *ctypes, int *fsizes,
                int *stats, int *repiter, int *verbose);
int  oh3_transbound_(int *currmode, int *stats);

/* Prototype for the function called from higher-level library code */
void init3(int **sdid, int nspec, int maxfrac, int **nphgram, int **totalp,
           int **rcounts, int **scounts, struct S_particle **pbuf, int **pbase,
           int maxlocalp, struct S_mycommc *mycommc, struct S_mycommf *mycommf,
           int **nbor, int *pcoord, int **sdoms, int *scoord, int nbound,
           int *bcond, int **bounds, int *ftypes, int *cfields, int cfid,
           int *ctypes, int **fsizes, int stats, int repiter, int verbose,
           int skip2);
