/* File: ohhelp3.c
   Version 1.1.1 (2015/10/23)
   Copyright (C) 2009-2015  Hiroshi Nakashima <h.nakashima@media.kyoto-u.ac.jp>
                            (ACCMS, Kyoto University)
   This program can be freely used, redistributed and modified for non-
   commercial purpose providing that the copyright notice above remains
   unchanged.
*/
#define EXTERN extern
#include "ohhelp1.h"
#include "ohhelp1_internal.h"
#include "ohhelp2.h"
#include "ohhelp2_internal.h"
#undef  EXTERN
#define EXTERN
#include "ohhelp3_internal.h"
#include "oh_context_internal.h"

static void init_subdomain_actively(struct oh_state *state,
                                    int (*sd)[OH_DIMENSION][2],
                                    int sc[OH_DIMENSION][2],
                                    int *pcoord, int bc[OH_DIMENSION][2],
                                    int (*bd)[OH_DIMENSION][2], int nb,
                                    int bbase);
static void init_subdomain_passively(struct oh_state *state,
                                     int (*sd)[OH_DIMENSION][2],
                                     int (*bd)[OH_DIMENSION][2], int nb,
                                     int bbase);
static int  comp_xyz(const void* aa, const void* bb);
static void init_fields(int (*ft)[OH_FTYPE_N], int *cf, int cfid,
                        int (*ct)[2][OH_CTYPE_N], int nb,
                        int sd[OH_DIMENSION][2], int **fsizes);
static void state_init_fields(struct oh_state *state, int (*ft)[OH_FTYPE_N],
                              int *cf, int cfid,
                              int (*ct)[2][OH_CTYPE_N], int nb,
                              int sd[OH_DIMENSION][2], int *fsizes);
static void configure_context_neighbors_from_grid(struct oh_state *state,
                                                  int pc[3]);
static void free_border_exchange_types(struct oh_state *state);
static void state_set_border_exchange(struct oh_state *state, int e, int ps,
                                      MPI_Datatype type);
static void set_border_comm(int esize, int f, int *xyz, int *wdh,
                            struct S_flddesc *field_desc,
                            int (*exti)[2], int (*exto)[2],
                            int (*off)[2], int (*size)[2],
                            int lu, int sr, MPI_Datatype basetype,
                            struct S_borderexc bx[OH_DIMENSION][2]);
static int  transbound3(struct oh_state *state, int currmode, int stats,
                        int level);
static void install_default_level3_particle_maps(struct oh_state *state);
static oh_particle_region_t offset_level3_map_particle_to_neighbor(
  const oh_particle_adapter *adapter, void *particle,
  int primary_or_secondary);
static oh_particle_region_t offset_level3_map_particle_to_subdomain(
  const oh_particle_adapter *adapter, void *particle,
  int primary_or_secondary);
static oh_particle_region_t context_level3_map_particle_to_neighbor(
  const oh_particle_adapter *adapter, void *particle,
  int primary_or_secondary);
static oh_particle_region_t context_level3_map_particle_to_subdomain(
  const oh_particle_adapter *adapter, void *particle,
  int primary_or_secondary);
static int  state_map_particle_to_neighbor(struct oh_state *state, double *x,
                                           double *y, double *z, int ps);
static int  state_map_particle_to_subdomain(struct oh_state *state, double x,
                                            double y, double z);
static int  state_map_irregular_subdomain(struct oh_state *state, double x,
                                          double y, double z);
static int  state_map_irregular(struct oh_state *state, double p0, double p1,
                                double p2, int dim, int from, int n);
static int  state_map_irregular_range(struct oh_state *state, double p,
                                      int dim, int from, int to);
static void state_bcast_field(struct oh_state *state, void *pfld, void *sfld,
                              int ftype);
static void state_reduce_field(struct oh_state *state, void *pfld, void *sfld,
                               int ftype);
static void state_allreduce_field(struct oh_state *state, void *pfld,
                                  void *sfld, int ftype);
static void state_exchange_borders(struct oh_state *state, void *pfld,
                                   void *sfld, int ctype, int bcast);
static void state_require_field_type(struct oh_state *state, int ftype,
                                     const char *api);
static void state_require_exchange_type(struct oh_state *state, int ctype,
                                        const char *api);
static void state_grid_size(struct oh_state *state, double size[OH_DIMENSION]);
void state_clear_border_exchange(struct oh_state *state);
void state_set_field_descriptors(struct oh_state *state,
                                 int (*ft)[OH_FTYPE_N],
                                 int sd[OH_DIMENSION][2], int ps);

void
oh3_init_(int *sdid, int *nspec, int *maxfrac, int *nphgram,
          int *totalp, struct S_particle *pbuf, int *pbase, int *maxlocalp,
          struct S_mycommf *mycomm, int *nbor, int *pcoord,
          int *sdoms, int *scoord, int *nbound, int *bcond, int *bounds,
          int *ftypes, int *cfields, int *ctypes, int *fsizes,
          int *stats, int *repiter, int *verbose) {
  void *raw_pbuf = pbuf;

  specBase = 1;
  init3(&sdid, *nspec, *maxfrac, &nphgram, &totalp, NULL, NULL, &raw_pbuf,
        &pbase, *maxlocalp, NULL, mycomm, &nbor, pcoord, &sdoms, scoord,
        *nbound, bcond, &bounds, ftypes, cfields, -1, ctypes, &fsizes,
        *stats, *repiter, *verbose, 0);
}
void
oh3_init(int **sdid, int nspec, int maxfrac, int **nphgram,
         int **totalp, void **pbuf, int **pbase, int maxlocalp,
         void *mycomm, int **nbor, int *pcoord,
         int **sdoms, int *scoord, int nbound, int *bcond, int **bounds,
         int *ftypes, int *cfields, int *ctypes, int **fsizes,
         int stats, int repiter, int verbose) {
  specBase = 0;
  if (!pbuf) local_errstop("oh3_init() requires a particle pointer slot");
  if (!pbase) local_errstop("oh3_init() requires a particle base slot");
  init3(sdid, nspec, maxfrac, nphgram, totalp, NULL, NULL, pbuf, pbase,
        maxlocalp,
        (struct S_mycommc*)mycomm, NULL, nbor, pcoord, sdoms, scoord,
        nbound, bcond, bounds, ftypes, cfields, 0, ctypes, fsizes,
        stats, repiter, verbose, 0);
}
void
oh13_init_(int *sdid, int *nspec, int *maxfrac, int *nphgram,
           int *totalp, int *rcounts, int *scounts,
           struct S_mycommf *mycomm, int *nbor, int *pcoord,
           int *sdoms, int *scoord, int *nbound, int *bcond, int *bounds,
           int *ftypes, int *cfields, int *ctypes, int *fsizes,
           int *stats, int *repiter, int *verbose) {
  init3(&sdid, *nspec, *maxfrac, &nphgram, &totalp, &rcounts, &scounts,
        NULL, NULL, 0, NULL, mycomm, &nbor, pcoord, &sdoms, scoord, *nbound,
        bcond, &bounds, ftypes, cfields, -1, ctypes, &fsizes,
        *stats, *repiter, *verbose, 1);
}
void
oh13_init(int **sdid, int nspec, int maxfrac, int **nphgram,
          int **totalp, int **rcounts, int **scounts,
          void *mycomm, int **nbor, int *pcoord,
          int **sdoms, int *scoord, int nbound, int *bcond, int **bounds,
          int *ftypes, int *cfields, int *ctypes, int **fsizes,
          int stats, int repiter, int verbose) {
  init3(sdid, nspec, maxfrac, nphgram, totalp, rcounts, scounts, NULL, NULL,
        0, (struct S_mycommc*)mycomm, NULL, nbor, pcoord, sdoms, scoord,
        nbound, bcond, bounds, ftypes, cfields, 0, ctypes, fsizes,
        stats, repiter, verbose, 1);
}
void
init3(int **sdid, int nspec, int maxfrac, int **nphgram,
      int **totalp, int **rcounts, int **scounts,
      void **pbuf, int **pbase, int maxlocalp,
      struct S_mycommc *mycommc, struct S_mycommf *mycommf,
      int **nbor, int *pcoord, int **sdoms, int *scoord,
      int nbound, int *bcond, int **bounds, int *ftypes,
      int *cfields, int cfid, int *ctypes, int **fsizes,
      int stats, int repiter, int verbose, int skip2) {
  int nn;
  struct oh_state *state;
  int (*sd)[OH_DIMENSION][2]=(int(*)[OH_DIMENSION][2])*sdoms;
  double (*sdf)[OH_DIMENSION][2];
  int (*sc)[2]=(int(*)[2])scoord;
  int (*bc)[2]=(int(*)[2])bcond;
  int (*bd)[OH_DIMENSION][2]=(int(*)[OH_DIMENSION][2])*bounds;
  int (*ft)[OH_FTYPE_N]=(int(*)[OH_FTYPE_N])ftypes;
  int (*ct)[2][OH_CTYPE_N]=(int(*)[2][OH_CTYPE_N])ctypes;
  int d, n, m;

  if (skip2)
    init1_state(&OhDefaultState, sdid, nspec, maxfrac, nphgram, totalp,
                rcounts, scounts, mycommc, mycommf, nbor, pcoord, stats,
                repiter, verbose);
  else
    init2(sdid, nspec, maxfrac, nphgram, totalp, pbuf, pbase, maxlocalp,
          mycommc, mycommf, nbor, pcoord, stats, repiter, verbose);
  excludeLevel2 = skip2;
  state = oh1_state();
  state->exclude_level2 = excludeLevel2;
  nn = state->n_of_nodes;

  if (!sd) {
    sd = (int(*)[OH_DIMENSION][2])
         (*sdoms = (int*)mem_alloc(sizeof(int), nn*OH_DIMENSION*2,
                                   "SubDomains"));
    sd[0][OH_DIM_X][OH_LOWER] = 0;  sd[0][OH_DIM_X][OH_UPPER] = -1;
  }
  if (!bd)
    bd = (int(*)[OH_DIMENSION][2])
         (*bounds = (int*)mem_alloc(sizeof(int), nn*OH_DIMENSION*2,
                                    "Boundaries"));

  for (d=0,n=1,m=OH_NEIGHBORS>>1; d<OH_DIMENSION; d++,n*=3) {
    int nl=state->dst_neighbors[m-n], nu=state->dst_neighbors[m+n];
    Adjacent[d][OH_LOWER] = nl<0 ? -(nl+1) : nl;
    Adjacent[d][OH_UPPER] = nu<0 ? -(nu+1) : nu;
  }
  if (sd[0][OH_DIM_X][OH_LOWER]>sd[0][OH_DIM_X][OH_UPPER])
    init_subdomain_actively(state, sd, sc, pcoord, bc, bd, nbound, -cfid);
  else
    init_subdomain_passively(state, sd, bd, nbound, -cfid);

  SubDomains = (int(*)[OH_DIMENSION][2])
               mem_alloc(sizeof(int), nn*OH_DIMENSION*2, "SubDomains");
  sdf = SubDomainsFloat =
        (double(*)[OH_DIMENSION][2])
        mem_alloc(sizeof(double), nn*OH_DIMENSION*2, "SubDomainsFloat");
  Boundaries = (int(*)[OH_DIMENSION][2])
               mem_alloc(sizeof(int), nn*OH_DIMENSION*2, "Boundaries");
  memcpy(SubDomains, sd, sizeof(int)*nn*OH_DIMENSION*2);
  for (n=0; n<nn; n++)  for (d=0; d<OH_DIMENSION; d++) {
    sdf[n][d][OH_LOWER] = sd[n][d][OH_LOWER];
    sdf[n][d][OH_UPPER] = sd[n][d][OH_UPPER];
  }
  memcpy(Boundaries, bd, sizeof(int)*nn*OH_DIMENSION*2);
  bd = Boundaries;
  if (cfid) {
    for (n=0; n<nn; n++)  for (d=0; d<OH_DIMENSION; d++) {
      bd[n][d][OH_LOWER]--;  bd[n][d][OH_UPPER]--;
    }
  }
  oh1_sync_default_state();
  init_fields(ft, cfields, cfid, ct, nbound, sd[state->my_rank], fsizes);
  install_default_level3_particle_maps(state);
  oh1_sync_default_state();
}
static void
install_default_level3_particle_maps(struct oh_state *state) {
  if (state->exclude_level2 || state->use_custom_particle_adapter) return;
  state->particle_adapter->user_data = state;
  state->particle_adapter->map_to_neighbor =
    context_level3_map_particle_to_neighbor;
  state->particle_adapter->map_to_subdomain =
    context_level3_map_particle_to_subdomain;
}
void
oh3_particle_adapter_use_position_fields(oh_particle_adapter *adapter,
                                         size_t x_offset, size_t y_offset,
                                         size_t z_offset) {
  const int had_subdomain_mapper = adapter && adapter->map_to_subdomain;

  if (!adapter) return;
  oh3_particle_adapter_use_neighbor_position_fields(adapter, x_offset,
                                                    y_offset, z_offset);
  if (!had_subdomain_mapper)
    oh3_particle_adapter_use_subdomain_position_fields(adapter, x_offset,
                                                       y_offset, z_offset);
}

void
oh3_particle_adapter_use_neighbor_position_fields(oh_particle_adapter *adapter,
                                                  size_t x_offset,
                                                  size_t y_offset,
                                                  size_t z_offset) {
  if (!adapter) return;
  oh_particle_adapter_use_position_fields(adapter, x_offset, y_offset,
                                          z_offset);
  adapter->map_to_neighbor = offset_level3_map_particle_to_neighbor;
}

void
oh3_particle_adapter_use_subdomain_position_fields(oh_particle_adapter *adapter,
                                                   size_t x_offset,
                                                   size_t y_offset,
                                                   size_t z_offset) {
  if (!adapter) return;
  oh_particle_adapter_use_position_fields(adapter, x_offset, y_offset,
                                          z_offset);
  adapter->map_to_subdomain = offset_level3_map_particle_to_subdomain;
}

void
oh3_bind_context_particle_adapter(struct oh_state *state) {
  oh_particle_adapter *adapter;

  if (!state || oh_context_is_default_state(state)) return;
  if (!state->use_custom_particle_adapter)
    install_default_level3_particle_maps(state);
  adapter = state->particle_adapter;
  if (!adapter) return;
  if (adapter->map_to_neighbor == offset_level3_map_particle_to_neighbor) {
    adapter->user_data = state;
    adapter->map_to_neighbor = context_level3_map_particle_to_neighbor;
  }
  if (adapter->map_to_subdomain == offset_level3_map_particle_to_subdomain) {
    adapter->user_data = state;
    adapter->map_to_subdomain = context_level3_map_particle_to_subdomain;
  }
}

void
oh3_configure_context_state(struct oh_state *state, const int *pcoord,
                            const int *sdoms, const int *scoord, int nbound,
                            const int *bcond, const int *bounds,
                            const int *ftypes, const int *cfields,
                            const int *ctypes, int *fsizes) {
  int nn, d, i, n, m;
  int pc[3] = {1, 1, 1};
  int sc[OH_DIMENSION][2];
  int bc[OH_DIMENSION][2];
  int (*sd)[OH_DIMENSION][2];
  int (*bd)[OH_DIMENSION][2];

  if (!state) state = oh1_state();
  if (oh_context_is_default_state(state))
    local_errstop("oh_context_configure_level3() is for non-default context");
  if (state->n_of_nodes<=0 || state->n_of_species<=0)
    local_errstop("Level 3 context configuration requires "
                  "oh_context_configure_particles() first");
  if (!state->owns_level1_storage)
    local_errstop("Level 3 context configuration requires Level 1 storage");

  oh3_free_context_state(state);

  nn = state->n_of_nodes;
  state->grid = (struct S_grid*)mem_alloc(sizeof(struct S_grid), 3, "Grid");
  state->subdomains =
    (int(*)[OH_DIMENSION][2])mem_alloc(sizeof(int), nn*OH_DIMENSION*2,
                                       "SubDomains");
  state->subdomains_float =
    (double(*)[OH_DIMENSION][2])
      mem_alloc(sizeof(double), nn*OH_DIMENSION*2, "SubDomainsFloat");
  state->boundaries =
    (int(*)[OH_DIMENSION][2])mem_alloc(sizeof(int), nn*OH_DIMENSION*2,
                                       "Boundaries");
  state->adjacent = (int*)mem_alloc(sizeof(int), OH_DIMENSION*2, "Adjacent");

  for (d=0; d<OH_DIMENSION; d++) {
    pc[d] = pcoord ? pcoord[d] : 1;
    sc[d][OH_LOWER] = scoord ? scoord[d*2 + OH_LOWER] : 0;
    sc[d][OH_UPPER] = scoord ? scoord[d*2 + OH_UPPER] : pc[d];
    bc[d][OH_LOWER] = bcond ? bcond[d*2 + OH_LOWER] : 0;
    bc[d][OH_UPPER] = bcond ? bcond[d*2 + OH_UPPER] : 0;
  }
  if (nbound<=0) nbound = 1;
  configure_context_neighbors_from_grid(state, pc);

  for (d=0,n=1,m=OH_NEIGHBORS>>1; d<OH_DIMENSION; d++,n*=3) {
    int nl=state->dst_neighbors[m-n], nu=state->dst_neighbors[m+n];
    ((int(*)[2])state->adjacent)[d][OH_LOWER] = nl<0 ? -(nl+1) : nl;
    ((int(*)[2])state->adjacent)[d][OH_UPPER] = nu<0 ? -(nu+1) : nu;
  }

  sd = state->subdomains;
  bd = state->boundaries;
  if (sdoms) {
    memcpy(sd, sdoms, sizeof(int)*nn*OH_DIMENSION*2);
    if (bounds)
      memcpy(bd, bounds, sizeof(int)*nn*OH_DIMENSION*2);
    else
      memset(bd, 0, sizeof(int)*nn*OH_DIMENSION*2);
    init_subdomain_passively(state, sd, bd, nbound, 0);
  } else {
    init_subdomain_actively(state, sd, sc, pc, bc, bd, nbound, 0);
  }

  for (i=0; i<nn; i++) {
    for (d=0; d<OH_DIMENSION; d++) {
      state->subdomains_float[i][d][OH_LOWER] = sd[i][d][OH_LOWER];
      state->subdomains_float[i][d][OH_UPPER] = sd[i][d][OH_UPPER];
    }
  }

  if (ftypes) {
    int default_cfields[1] = {-1};
    int (*ft)[OH_FTYPE_N] = (int(*)[OH_FTYPE_N])ftypes;
    int (*ct)[2][OH_CTYPE_N] = (int(*)[2][OH_CTYPE_N])ctypes;
    int *cf = cfields ? (int*)cfields : default_cfields;
    state_init_fields(state, ft, cf, 0, ct, nbound, sd[state->my_rank],
                      fsizes);
  }

  state->exclude_level2 = 0;
  state->owns_level3_storage = 1;
  install_default_level3_particle_maps(state);
  oh3_bind_context_particle_adapter(state);
}
static void
init_subdomain_actively(struct oh_state *state,
                        int (*sd)[OH_DIMENSION][2], int sc[OH_DIMENSION][2],
                        int *pcoord, int bc[OH_DIMENSION][2],
                        int (*bd)[OH_DIMENSION][2], int nb, int bbase) {
  struct S_grid *grid=state->grid;
  int nn=state->n_of_nodes, pqr=1;
  int d, lu, i, j, k, x, y, z, n;

  if (oh_context_is_default_state(state)) SubDomainDesc = NULL;
  state->subdomain_desc = NULL;
  for (d=0; d<OH_DIMENSION; d++) {
    int lo = grid[d].coord[OH_LOWER] = sc[d][OH_LOWER];
    int up = grid[d].coord[OH_UPPER] = sc[d][OH_UPPER];
    int size = up - lo;
    int ave, nl;
    grid[d].fcoord[OH_LOWER] = lo;  grid[d].fcoord[OH_UPPER] = up;
    n = grid[d].n = pcoord[d];
    if (n<=0)
      errstop("# of %c-nodes (%d) should be positive", Message.xyz[d], n);
    if (size<=0)
      errstop("upper edge of %c-coordinate (%d) should be greater than "
              "lower edge (%d)", Message.xyz[d], up, lo);
    ave = grid[d].light.size = size/n;
    grid[d].light.rfsize = 1.0/(double)ave;
    grid[d].light.rfsizeplus = 1.0/(double)(ave+1);
    nl =  grid[d].light.n = n - size%n;
    grid[d].light.fthresh = (grid[d].light.thresh = lo + nl * ave);
    grid[d].fsize = (grid[d].size = n==nl ? ave : ave+1);
    grid[d].gsize = grid[d].rgsize = 1.0;
    pqr *= n;
  }
  for (; d<3; d++) {
    grid[d].n = grid[d].light.n = 1;
    grid[d].coord[OH_LOWER] = grid[d].coord[OH_UPPER] = 0;
    grid[d].fcoord[OH_LOWER] = grid[d].fcoord[OH_UPPER] = 0.0;
    grid[d].size = grid[d].light.size = grid[d].light.thresh = 0;
    grid[d].fsize = grid[d].light.rfsize
                  = grid[d].light.rfsizeplus = grid[d].light.fthresh = 0.0;
    grid[d].gsize = grid[d].rgsize = 1.0;
  }
  if (pqr!=nn) {
    if (OH_DIMENSION==1)
      errstop("<# of x-nodes>(%d) should be eqal to <# of nodes>(%d)",
              pcoord[0], nn);
    else if (OH_DIMENSION==2)
      errstop("<# of x-nodes>(%d) * <# of y-nodes>(%d) "
              "should be eqal to <# of nodes>(%d)",
              pcoord[0], pcoord[1], nn);
    else
      errstop("<# of x-nodes>(%d) * <# of y-nodes>(%d) * <# of z-nodes>(%d) "
              "should be eqal to <# of nodes>(%d)",
              pcoord[0], pcoord[1], pcoord[2], nn);
  }
  for (d=0; d<OH_DIMENSION; d++) {
    for (lu=OH_LOWER; lu<=OH_UPPER; lu++) {
      if (bc[d][lu]<bbase || bc[d][lu]>=nb+bbase)
        errstop("system's %s boundary condition for %c-coordinate %d is "
                "invalid",
                Message.loup[lu], Message.xyz[d], bc[d][lu]);
    }
  }
  for (i=0,z=grid[OH_DIM_Z].coord[OH_LOWER],n=0; i<grid[OH_DIM_Z].n; i++) {
    int top=z+grid[OH_DIM_Z].light.size;
#if OH_DIMENSION >= 3
    int bot=z;
    int bzlo = i==0 ? bc[OH_DIM_Z][OH_LOWER] : bbase;
    int bzup = i==grid[OH_DIM_Z].n-1 ? bc[OH_DIM_Z][OH_UPPER] : bbase;
#endif
    if (i>=grid[OH_DIM_Z].light.n)  top++;
    z = top;
    for (j=0,y=grid[OH_DIM_Y].coord[OH_LOWER]; j<grid[OH_DIM_Y].n; j++) {
      int north=y+grid[OH_DIM_Y].light.size;
#if OH_DIMENSION >= 2
      int south=y;
      int bylo = j==0 ? bc[OH_DIM_Y][OH_LOWER] : bbase;
      int byup = j==grid[OH_DIM_Y].n-1 ? bc[OH_DIM_Y][OH_UPPER] : bbase;
#endif
      if (j>=grid[OH_DIM_Y].light.n)  north++;
      y = north;
      for (k=0,x=grid[OH_DIM_X].coord[OH_LOWER]; k<grid[OH_DIM_X].n; k++,n++) {
        int west=x, east=x+grid[OH_DIM_X].light.size;
        if (k>=grid[OH_DIM_X].light.n)  east++;
        x = east;
        sd[n][OH_DIM_X][OH_LOWER] = west;
        sd[n][OH_DIM_X][OH_UPPER] = east;
        bd[n][OH_DIM_X][OH_LOWER] = bd[n][OH_DIM_X][OH_UPPER] = bbase;
#if OH_DIMENSION >= 2
        sd[n][OH_DIM_Y][OH_LOWER] = south;
        sd[n][OH_DIM_Y][OH_UPPER] = north;
        bd[n][OH_DIM_Y][OH_LOWER] = bylo;
        bd[n][OH_DIM_Y][OH_UPPER] = byup;
#endif
#if OH_DIMENSION >= 3
        sd[n][OH_DIM_Z][OH_LOWER] = bot;
        sd[n][OH_DIM_Z][OH_UPPER] = top;
        bd[n][OH_DIM_Z][OH_LOWER] = bzlo;
        bd[n][OH_DIM_Z][OH_UPPER] = bzup;
#endif
      }
      bd[n-grid[OH_DIM_X].n][OH_DIM_X][OH_LOWER] = bc[OH_DIM_X][OH_LOWER];
      bd[n-1][OH_DIM_X][OH_UPPER] = bc[OH_DIM_X][OH_UPPER];
    }
  }
}
static void
configure_context_neighbors_from_grid(struct oh_state *state, int pc[3]) {
  int raw[OH_NEIGHBORS];

  oh_context_build_grid_neighbors(state, pc, raw);
  oh_context_apply_neighbors(state, raw);
}
static void
init_subdomain_passively(struct oh_state *state,
                         int (*sd)[OH_DIMENSION][2],
                         int (*bd)[OH_DIMENSION][2], int nb, int bbase) {
  struct S_grid *grid=state->grid;
  int (*adjacent)[2]=(int(*)[2])state->adjacent;
  int nn=state->n_of_nodes;
  struct S_subdomdesc *sdd =
    (struct S_subdomdesc*)mem_alloc(sizeof(struct S_subdomdesc), nn,
                                    "SubDomainDesc");
  int min[OH_DIMENSION], max[OH_DIMENSION];
  int smin[OH_DIMENSION], smax[OH_DIMENSION];
  int me=state->my_rank;
  int i, d, dd, lu, l;
  int lo[OH_DIMENSION-1], up[OH_DIMENSION-1], h[OH_DIMENSION-1];

  state->subdomain_desc = sdd;
  if (oh_context_is_default_state(state)) SubDomainDesc = sdd;
  for (d=0; d<OH_DIMENSION; d++) {
    min[d] = sd[0][d][OH_LOWER];  max[d] = sd[0][d][OH_UPPER];
    smin[d] = smax[d] = max[d] - min[d];
  }
  for (i=0; i<nn; i++) {
    for (d=0; d<OH_DIMENSION; d++) {
      int lo=sd[i][d][OH_LOWER], up=sd[i][d][OH_UPPER], n=up-lo;
      sdd[i].coord[d].fc[OH_LOWER] = (sdd[i].coord[d].c[OH_LOWER] = lo);
      sdd[i].coord[d].fc[OH_UPPER] = (sdd[i].coord[d].c[OH_UPPER] = up);
      sdd[i].coord[d].n = 0;
      if (n<smin[d])  smin[d] = n;
      if (n>smax[d])  smax[d] = n;
      if (lo<min[d])  min[d] = lo;
      if (up>max[d])  max[d] = up;
      if (n<=0)
        errstop("subdomain %d has %c-coordinate lower boundary %d "
                "not less than upper boundary %d", i, Message.xyz[d], lo, up);
      for (lu=OH_LOWER; lu<=OH_UPPER; lu++) {
        if (bd[i][d][lu]<bbase || bd[i][d][lu]>=nb+bbase)
          errstop("rank-%d's %s boundary condition for %c-coordinate %d is "
                  "invalid",
                  i, Message.loup[lu], Message.xyz[d], bd[i][d][lu]);
      }
    }
    sdd[i].id = i;
  }
  for (d=0; d<OH_DIMENSION; d++) {
    grid[d].fsize = (grid[d].size = smax[d]);
    grid[d].light.size = smin[d];
    grid[d].light.rfsize = grid[d].light.rfsizeplus = 0.0;
    grid[d].fcoord[OH_LOWER] = (grid[d].coord[OH_LOWER] = min[d]);
    grid[d].fcoord[OH_UPPER] = (grid[d].coord[OH_UPPER] = max[d]);
    grid[d].n = grid[d].light.n = 0;    /* never referred but ... */
    grid[d].light.thresh = 0;  grid[d].light.fthresh = 0.0;
    grid[d].gsize = grid[d].rgsize = 1.0;
    for (lu=OH_LOWER; lu<=OH_UPPER; lu++) {
      int n=adjacent[d][lu];
      if (n==nn || bd[me][d][lu]!=bbase) continue;
      for (dd=0; dd<OH_DIMENSION; dd++) {
        if (d==dd) {
          int diff = sd[n][dd][OH_UPPER-lu] - sd[me][dd][lu];
          int dsize = max[dd] - min[dd];
          if (diff!=0 && diff!=dsize && diff!=-dsize)
            local_errstop("rank-%d and its %c-%s neighbor rank-%d have "
                          "incompatible %s/%s boundaries of %c-coordinate "
                          "%d and %d",
                          me, Message.xyz[d], Message.loup[lu], n,
                          Message.loup[lu], Message.loup[OH_UPPER-lu],
                          Message.xyz[dd],
                          sd[me][dd][lu], sd[n][dd][OH_UPPER-lu]);
        } else {
          for (l=OH_LOWER; l<=OH_UPPER; l++) {
            if (sd[n][dd][l]!=sd[me][dd][l])
              local_errstop("rank-%d and its %c-%s neighbor rank-%d have "
                            "incompatible %s boundary of %c-coordinate "
                            "%d and %d",
                            me, Message.xyz[d], Message.loup[lu], n,
                            Message.loup[l], Message.xyz[dd],
                            sd[me][dd][l], sd[n][dd][l]);
          }
        }
      }
    }
  }
  for (; d<3; d++) {
    grid[d].n = grid[d].light.n = 1;
    grid[d].coord[OH_LOWER] = grid[d].coord[OH_UPPER] = 0;
    grid[d].fcoord[OH_LOWER] = grid[d].fcoord[OH_UPPER] = 0.0;
    grid[d].size = grid[d].light.size = grid[d].light.thresh = 0;
    grid[d].fsize = grid[d].light.rfsize
                  = grid[d].light.rfsizeplus = grid[d].light.fthresh = 0.0;
    grid[d].gsize = grid[d].rgsize = 1.0;
  }
  qsort(sdd, nn, sizeof(struct S_subdomdesc), comp_xyz);
  for (d=0; d<OH_DIMENSION-1; d++) {
    sdd[0].coord[d].h = h[d] = 0;
    lo[d] = sdd[0].coord[d].c[OH_LOWER];
    up[d] = sdd[0].coord[d].c[OH_UPPER];
  }
  for (i=1; i<nn; i++) {
    for (d=0; d<OH_DIMENSION-1; d++) {
      if (lo[d]!=sdd[i].coord[d].c[OH_LOWER] ||
          up[d]!=sdd[i].coord[d].c[OH_UPPER]) {
        for (dd=d; dd<OH_DIMENSION-1; dd++) {
          sdd[h[dd]].coord[dd].n = i - h[dd];
          sdd[i].coord[dd].h = h[dd] = i;
          lo[dd] = sdd[i].coord[dd].c[OH_LOWER];
          up[dd] = sdd[i].coord[dd].c[OH_UPPER];
        }
        break;
      } else {
        sdd[i].coord[d].h = h[d];
      }
    }
    sdd[i].coord[OH_DIMENSION-1].n = 1;  sdd[i].coord[OH_DIMENSION-1].h = i;
  }
  for (d=0; d<OH_DIMENSION-1; d++)  sdd[h[d]].coord[d].n = nn - h[d];
}
static int
comp_xyz(const void* aa, const void* bb) {
  struct S_subdomdesc *a=(struct S_subdomdesc*)aa, *b=(struct S_subdomdesc*)bb;
  int d;

  for (d=0; d<OH_DIMENSION; d++) {
    if (a->coord[d].c[OH_LOWER]+a->coord[d].c[OH_UPPER]<
        b->coord[d].c[OH_LOWER]+b->coord[d].c[OH_UPPER])  return(-1);
    if (a->coord[d].c[OH_LOWER]+a->coord[d].c[OH_UPPER]>
        b->coord[d].c[OH_LOWER]+b->coord[d].c[OH_UPPER])  return(1);
    if (a->coord[d].c[OH_LOWER]<b->coord[d].c[OH_LOWER])  return(-1);
    if (a->coord[d].c[OH_LOWER]>b->coord[d].c[OH_LOWER])  return(1);
    if (a->coord[d].c[OH_UPPER]<b->coord[d].c[OH_UPPER])  return(-1);
    if (a->coord[d].c[OH_UPPER]>b->coord[d].c[OH_UPPER])  return(1);
  }
  return(a->id<b->id ? -1 : 1);
}
#if OH_DIMENSION==1
#define Field_Disp(FD,F,X,Y,Z) ((FD)[F].esize * (X))
#elif OH_DIMENSION==2
#define Field_Disp(FD,F,X,Y,Z)\
  ((FD)[F].esize *\
   ((X) + (FD)[F].size[OH_DIM_X] * (Y)))
#else
#define Field_Disp(FD,F,X,Y,Z)\
  ((FD)[F].esize *\
   ((X) + (FD)[F].size[OH_DIM_X] *\
    ((Y) + (FD)[F].size[OH_DIM_Y] * (Z))))
#endif
static void
init_fields(int (*ft)[OH_FTYPE_N], int *cf, int cfid, int (*ct)[2][OH_CTYPE_N],
            int nb, int sd[OH_DIMENSION][2], int **fsizes) {
  struct oh_state *state = oh1_state();
  int nf;

  for (nf=0; ft[nf][OH_FTYPE_ES]>0; nf++);
  if (!*fsizes)
    *fsizes = (int*)mem_alloc(sizeof(int), nf*OH_DIMENSION*2,
                              "FieldSizes");
  state_init_fields(state, ft, cf, cfid, ct, nb, sd, *fsizes);
  oh1_sync_default_state();
}
static void
state_init_fields(struct oh_state *state, int (*ft)[OH_FTYPE_N], int *cf,
                  int cfid, int (*ct)[2][OH_CTYPE_N], int nb,
                  int sd[OH_DIMENSION][2], int *fsizes) {
  struct S_flddesc *fd;
  struct S_borderexc (*bx)[2][OH_DIMENSION][2];
  int *owned_fsizes = NULL;
  int (*fs)[OH_DIMENSION][2];
  int nf, ne;
  int f, e, d, lu;
  int *tmp;
#ifndef OH_POS_AWARE
  int b, i;
#endif

  state->n_of_boundaries = nb;
  if (oh_context_is_default_state(state)) nOfBoundaries = nb;
  for (nf=0; ft[nf][OH_FTYPE_ES]>0; nf++);
  if (!fsizes)
    owned_fsizes = (int*)mem_alloc(sizeof(int), nf*OH_DIMENSION*2,
                                   "FieldSizes");
  fs = (int(*)[OH_DIMENSION][2])(fsizes ? fsizes : owned_fsizes);
  state->n_of_fields = nf;
  if (oh_context_is_default_state(state)) nOfFields = nf;
#ifdef OH_POS_AWARE
  for (ne=0; cf[ne]>=0; ne++);
#else
  for (ne=0; cf[ne]+cfid>=0; ne++);
#endif
  state->n_of_exchanges = ne;
  if (oh_context_is_default_state(state)) nOfExc = ne;
  if (ne>0 && !ct)
    local_errstop("Level 3 field configuration requires ctypes when cfields defines boundary exchanges");

  fd = (struct S_flddesc*)mem_alloc(sizeof(struct S_flddesc), nf,
                                    "FieldDesc");
  state->field_desc = fd;
  if (oh_context_is_default_state(state)) FieldDesc = fd;
  state->boundary_comm_types =
    ne ? (int*)mem_alloc(sizeof(int), ne*nb*2*OH_CTYPE_N,
                         "BoundaryCommTypes") : NULL;
  if (ne) memcpy(state->boundary_comm_types, ct,
                 sizeof(int)*ne*nb*2*OH_CTYPE_N);
  ct = (int(*)[2][OH_CTYPE_N])state->boundary_comm_types;
  if (oh_context_is_default_state(state)) {
    BoundaryCommTypes = ct;
  }

  tmp = ne ? (int*)mem_alloc(sizeof(int), ne, "BoundaryCommFields") : NULL;
  for (e=0; e<ne; e++) {
#ifdef OH_POS_AWARE
    tmp[e] = cf[e];
#else
    tmp[e] = cf[e] + cfid;
#endif
  }
  state->boundary_comm_fields = cf = tmp;
  if (oh_context_is_default_state(state)) BoundaryCommFields = cf;

  for (f=0; f<nf; f++) {
    int lo=ft[f][OH_FTYPE_LO], up=ft[f][OH_FTYPE_UP];
    fd[f].esize = ft[f][OH_FTYPE_ES];
    for (lu=OH_FTYPE_BL; lu<OH_FTYPE_RU; lu+=2) {
      int lot=ft[f][lu], upt=ft[f][lu+1];
      if (lot<lo)  lo = lot;
      if (upt>up)  up = upt;
    }
    fd[f].ext[OH_LOWER] = lo;  fd[f].ext[OH_UPPER] = up;
  }
  state->field_types =
    (int*)mem_alloc(sizeof(int), nf*OH_FTYPE_N, "FieldTypes");
  memcpy(state->field_types, ft, sizeof(int)*nf*OH_FTYPE_N);
  ft = (int(*)[OH_FTYPE_N])state->field_types;
  if (oh_context_is_default_state(state)) FieldTypes = ft;

#ifndef OH_POS_AWARE
  for (e=0,i=0; e<ne; e++) {
    int f=cf[e];
    int lo, up;
    if (f>=nf)
      errstop("boundary communication #%d cannot be defined for "
              "undefined field #%d", e-cfid, f-cfid);
    lo = fd[f].ext[OH_LOWER];  up = fd[f].ext[OH_UPPER];
    for (b=0; b<nb; b++,i++) {
      int sl=ct[i][OH_LOWER][OH_CTYPE_SIZE];
      int su=ct[i][OH_UPPER][OH_CTYPE_SIZE];
      int lo1=ct[i][OH_LOWER][OH_CTYPE_FROM];
      int lo2=ct[i][OH_UPPER][OH_CTYPE_TO];
      int up1=ct[i][OH_LOWER][OH_CTYPE_TO]   + sl;
      int up2=ct[i][OH_UPPER][OH_CTYPE_FROM] + su;
      if (sl && lo1<lo)  lo = lo1;
      if (su && lo2<lo)  lo = lo2;
      if (sl && up1>up)  up = up1;
      if (su && up2>up)  up = up2;
    }
    fd[f].ext[OH_LOWER] = lo;  fd[f].ext[OH_UPPER] = up;
  }
#endif
  for (f=0; f<nf; f++) {
    int lo=fd[f].ext[OH_LOWER], up=fd[f].ext[OH_UPPER];
    for (d=0; d<OH_DIMENSION; d++) {
      fs[f][d][OH_LOWER] = lo;
      fs[f][d][OH_UPPER] = (state->grid[d].size+cfid) + up;
      fd[f].size[d] = state->grid[d].size + (up - lo);
    }
  }
  for (f=0; f<nf; f++) {
    int bl = ft[f][OH_FTYPE_BL];
    int rl = ft[f][OH_FTYPE_RL];
    fd[f].bc.base  = Field_Disp(fd, f, bl, bl, bl);
    fd[f].red.base = Field_Disp(fd, f, rl, rl, rl);
  }
  state_set_field_descriptors(state, ft, sd, 0);

  bx =
    (struct S_borderexc(*)[2][OH_DIMENSION][2])
    mem_alloc(sizeof(struct S_borderexc), ne*2*OH_DIMENSION*2, "BorderExc");
  state->border_exchange = (struct S_borderexc*)bx;
  if (oh_context_is_default_state(state)) BorderExc = bx;

  for (e=0; e<ne; e++) {
    for (d=0; d<OH_DIMENSION; d++) {
      for (lu=0; lu<2; lu++)
        bx[e][1][d][lu].send.deriv = bx[e][1][d][lu].recv.deriv = 0;
    }
#ifdef OH_POS_AWARE
    state_set_border_exchange(state, e, 0,
                              e<ne-1 ? MPI_DOUBLE : MPI_LONG_LONG_INT);
#else
    state_set_border_exchange(state, e, 0, MPI_DOUBLE);
#endif
  }
  state_clear_border_exchange(state);
  if (owned_fsizes) free(owned_fsizes);
}
void
state_set_field_descriptors(struct oh_state *state, int (*ft)[OH_FTYPE_N],
                            int sd[OH_DIMENSION][2], int ps) {

  int nf=state->n_of_fields;
  struct S_flddesc *fd = state->field_desc;
  int size[3] = {0,0,0};
  int d, f;

  for (d=0; d<OH_DIMENSION; d++)  size[d] = sd[d][OH_UPPER] - sd[d][OH_LOWER];
  for (f=0; f<nf; f++) {
    int bu = ft[f][OH_FTYPE_BU] - 1;
    int ru = ft[f][OH_FTYPE_RU] - 1;
    int es = ft[f][OH_FTYPE_ES];
    fd[f].bc.size[ps] =
      Field_Disp(fd, f, size[OH_DIM_X]+bu, size[OH_DIM_Y]+bu,
                 size[OH_DIM_Z]+bu) -
      fd[f].bc.base + es;
    fd[f].red.size[ps] =
      Field_Disp(fd, f, size[OH_DIM_X]+ru, size[OH_DIM_Y]+ru,
                 size[OH_DIM_Z]+ru) -
      fd[f].red.base + es;
  }
}
static void
state_set_border_exchange(struct oh_state *state, int e, int ps,
                          MPI_Datatype type) {
  struct S_borderexc (*border_exchange)[2][OH_DIMENSION][2] =
    (struct S_borderexc(*)[2][OH_DIMENSION][2])state->border_exchange;
  int (*boundary_comm_types)[2][OH_CTYPE_N] =
    (int(*)[2][OH_CTYPE_N])state->boundary_comm_types;
  struct S_borderexc (*bx)[2] = border_exchange[e][ps];
  int f = state->boundary_comm_fields[e];
  int nb = state->n_of_boundaries;
  int (*bt)[2][OH_CTYPE_N] = &boundary_comm_types[e*nb];
  int (*bd)[2] = state->boundaries[state->region_id[ps]];
  int (*sd)[2] = state->subdomains[state->region_id[ps]];
  struct S_flddesc *fd = &state->field_desc[f];
  int esize = fd->esize;
  int xyz[3] = {
    sd[OH_DIM_X][OH_UPPER]-sd[OH_DIM_X][OH_LOWER],
    OH_DIMENSION>OH_DIM_Y ? sd[OH_DIM_Y][OH_UPPER]-sd[OH_DIM_Y][OH_LOWER] : 0,
    OH_DIMENSION>OH_DIM_Z ? sd[OH_DIM_Z][OH_UPPER]-sd[OH_DIM_Z][OH_LOWER] : 0
  };
  int *wdh = fd->size;
  int exti[OH_DIMENSION][2], exto[OH_DIMENSION][2];
  int soff[OH_DIMENSION][2], roff[OH_DIMENSION][2];
  int ssize[OH_DIMENSION][2], rsize[OH_DIMENSION][2];
  int d, lu;

  for (d=0; d<OH_DIMENSION; d++) {
    int blo=bd[d][OH_LOWER], bup=bd[d][OH_UPPER];
    exti[d][OH_LOWER] = bt[blo][OH_LOWER][OH_CTYPE_FROM];
    exti[d][OH_UPPER] =
      bt[bup][OH_UPPER][OH_CTYPE_FROM] + bt[bup][OH_UPPER][OH_CTYPE_SIZE];
    exto[d][OH_LOWER] = bt[blo][OH_UPPER][OH_CTYPE_TO];
    exto[d][OH_UPPER] =
      bt[bup][OH_LOWER][OH_CTYPE_TO] + bt[bup][OH_LOWER][OH_CTYPE_SIZE];
    for (lu=OH_LOWER; lu<=OH_UPPER; lu++) {
      int sb=bd[d][lu], rb=bd[d][1-lu];
      soff[d][lu] = bt[sb][lu][OH_CTYPE_FROM];
      roff[d][lu] = bt[rb][lu][OH_CTYPE_TO];
      ssize[d][lu] = bt[sb][lu][OH_CTYPE_SIZE];
      rsize[d][lu] = bt[rb][lu][OH_CTYPE_SIZE];
    }
  }
  for (lu=OH_LOWER; lu<=OH_UPPER; lu++) {
    set_border_comm(esize, f, xyz, wdh, state->field_desc, exti, exto, soff,
                    ssize, lu, 0, type, bx);
    set_border_comm(esize, f, xyz, wdh, state->field_desc, exti, exto, roff,
                    rsize, lu, 1, type, bx);
  }
}
static void
set_border_comm(int esize, int f, int *xyz, int *wdh,
                struct S_flddesc *field_desc,
                int exti[OH_DIMENSION][2], int exto[OH_DIMENSION][2],
                int off[OH_DIMENSION][2], int size[OH_DIMENSION][2],
                int lu, int sr, MPI_Datatype basetype,
                struct S_borderexc bx[OH_DIMENSION][2]) {
  MPI_Datatype vector_type = MPI_DATATYPE_NULL;
  int w=wdh[OH_DIM_X], wd=w*esize;
  int dp=OH_DIMENSION==1 ? 1 : wdh[OH_DIM_Y];
  MPI_Aint z_extent = (MPI_Aint)wd * (MPI_Aint)dp * (MPI_Aint)sizeof(double);
  struct S_bcomm *bcx, *bcy, *bcz;
  int xexto, yexti, yexto, zexti;
  int s;
  int lower = sr ? lu==OH_UPPER : lu==OH_LOWER;

  bcx = (sr==0) ? &bx[OH_DIM_X][lu].send : &bx[OH_DIM_X][lu].recv;
  bcx->deriv = 0;
  xexto = xyz[OH_DIM_X] + exto[OH_DIM_X][OH_UPPER] - exto[OH_DIM_X][OH_LOWER];
  if (OH_DIMENSION>OH_DIM_Y) {
    bcy = (sr==0) ? &bx[OH_DIM_Y][lu].send : &bx[OH_DIM_Y][lu].recv;
    bcy->deriv = 0;
    yexti = xyz[OH_DIM_Y] +
            exti[OH_DIM_Y][OH_UPPER] - exti[OH_DIM_Y][OH_LOWER];
    yexto = xyz[OH_DIM_Y] +
            exto[OH_DIM_Y][OH_UPPER] - exto[OH_DIM_Y][OH_LOWER];
  }
  if (OH_DIMENSION>OH_DIM_Z) {
    bcz = (sr==0) ? &bx[OH_DIM_Z][lu].send : &bx[OH_DIM_Z][lu].recv;
    bcz->deriv = 0;
    zexti = xyz[OH_DIM_Z] +
            exti[OH_DIM_Z][OH_UPPER] - exti[OH_DIM_Z][OH_LOWER];
  }
  if (OH_DIMENSION==1) {
    if ((s=size[OH_DIM_X][lu])==0) {
      bcx->buf = bcx->count = 0;  bcx->type = MPI_DATATYPE_NULL;
    } else {
      bcx->type = basetype;
      bcx->count = s * esize;
      bcx->buf =
        Field_Disp(field_desc, f,
                   lower ? off[OH_DIM_X][lu] : xyz[OH_DIM_X]+off[OH_DIM_X][lu],
                   0, 0);
    }
  } else if (OH_DIMENSION==2) {
    if ((s=size[OH_DIM_X][lu])==0) {
      bcx->buf = bcx->count = 0;  bcx->type = MPI_DATATYPE_NULL;
    } else {
      MPI_Type_vector(yexti, s*esize, wd, basetype, &(bcx->type));
      MPI_Type_commit(&(bcx->type));  bcx->deriv = 1;
      bcx->count = 1;
      bcx->buf =
        Field_Disp(field_desc, f,
                   lower ? off[OH_DIM_X][lu] : xyz[OH_DIM_X]+off[OH_DIM_X][lu],
                   exti[OH_DIM_Y][OH_LOWER], 0);
    }
    if ((s=size[OH_DIM_Y][lu])==0) {
      bcy->buf = bcy->count = 0;  bcy->type = MPI_DATATYPE_NULL;
    } else {
      if (xexto==w) {
        bcy->type = basetype;
        bcy->count = s * wd;
      } else {
        MPI_Type_vector(s, xexto*esize, wd, basetype, &(bcy->type));
        MPI_Type_commit(&(bcy->type));  bcy->deriv = 1;
        bcy->count = 1;
      }
      bcy->buf =
        Field_Disp(field_desc, f, exto[OH_DIM_X][OH_LOWER],
                   lower ? off[OH_DIM_Y][lu] : xyz[OH_DIM_Y]+off[OH_DIM_Y][lu],
                   0);
    }
  } else {
    if ((s=size[OH_DIM_X][lu])==0) {
      bcx->buf = bcx->count = 0;  bcx->type = MPI_DATATYPE_NULL;
    } else {
      MPI_Type_vector(yexti, s*esize, wd, basetype, &vector_type);
      MPI_Type_create_resized(vector_type, 0, z_extent, &(bcx->type));
      MPI_Type_free(&vector_type);
      MPI_Type_commit(&(bcx->type));  bcx->deriv = 1;
      bcx->count = zexti;
      bcx->buf =
        Field_Disp(field_desc, f,
                   lower ? off[OH_DIM_X][lu] : xyz[OH_DIM_X]+off[OH_DIM_X][lu],
                   exti[OH_DIM_Y][OH_LOWER], exti[OH_DIM_Z][OH_LOWER]);
    }
    if ((s=size[OH_DIM_Y][lu])==0) {
      bcy->buf = bcy->count = 0;  bcy->type = MPI_DATATYPE_NULL;
    } else {
      if (xexto==w) {
        MPI_Type_vector(zexti, s*wd, wd*dp, basetype, &(bcy->type));
        bcy->count = 1;
      } else {
        MPI_Type_vector(s, xexto*esize, wd, basetype, &vector_type);
        MPI_Type_create_resized(vector_type, 0, z_extent, &(bcy->type));
        MPI_Type_free(&vector_type);
        bcy->count = zexti;
      }
      MPI_Type_commit(&(bcy->type));  bcy->deriv = 1;
      bcy->buf =
        Field_Disp(field_desc, f, exto[OH_DIM_X][OH_LOWER],
                   lower ? off[OH_DIM_Y][lu] : xyz[OH_DIM_Y]+off[OH_DIM_Y][lu],
                   exti[OH_DIM_Z][OH_LOWER]);
    }
    if ((s=size[OH_DIM_Z][lu])==0) {
      bcz->buf = bcz->count = 0;  bcz->type = MPI_DATATYPE_NULL;
    } else {
      if (xexto==w && yexto==dp) {
        bcz->type = basetype;
        bcz->count = s * wd * dp;
      } else {
        if (xexto==w) {
          MPI_Type_vector(s, wd*yexto, wd*dp, basetype, &(bcz->type));
          bcz->count = 1;
        } else if (yexto==dp) {
          MPI_Type_vector(s*yexto, xexto*esize, wd, basetype, &(bcz->type));
          bcz->count = 1;
        } else {
          MPI_Type_vector(yexto, xexto*esize, wd, basetype, &vector_type);
          MPI_Type_create_resized(vector_type, 0, z_extent, &(bcz->type));
          MPI_Type_free(&vector_type);
          bcz->count = s;
        }
        MPI_Type_commit(&(bcz->type));  bcz->deriv = 1;
      }
      bcz->buf =
        Field_Disp(field_desc, f, exto[OH_DIM_X][OH_LOWER],
                   exto[OH_DIM_Y][OH_LOWER],
                   lower ? off[OH_DIM_Z][lu] :
                           xyz[OH_DIM_Z]+off[OH_DIM_Z][lu]);
    }
  }
}
void
state_clear_border_exchange(struct oh_state *state) {
  int ne=state->n_of_exchanges, e, d, lu;
  int initialized = 0;
  int finalized = 0;
  int mpi_active = 0;
  struct S_borderexc (*bx)[2][OH_DIMENSION][2] =
    (struct S_borderexc(*)[2][OH_DIMENSION][2])state->border_exchange;

  if (!bx) return;
  MPI_Initialized(&initialized);
  if (initialized) MPI_Finalized(&finalized);
  mpi_active = initialized && !finalized;
  for (e=0; e<ne; e++) {
    for (d=0; d<OH_DIMENSION; d++) {
      for (lu=OH_LOWER; lu<=OH_UPPER; lu++) {
        if (mpi_active && bx[e][1][d][lu].send.deriv)
          MPI_Type_free(&bx[e][1][d][lu].send.type);
        if (mpi_active && bx[e][1][d][lu].recv.deriv)
          MPI_Type_free(&bx[e][1][d][lu].recv.type);
        bx[e][1][d][lu].send.buf =   bx[e][1][d][lu].recv.buf = 0;
        bx[e][1][d][lu].send.count = bx[e][1][d][lu].recv.count = -1;
        bx[e][1][d][lu].send.deriv = bx[e][1][d][lu].recv.deriv = 0;
        bx[e][1][d][lu].send.type =  bx[e][1][d][lu].recv.type =
          MPI_DATATYPE_NULL;
      }
    }
  }
}

static void
free_border_exchange_types(struct oh_state *state) {
  int ne=state->n_of_exchanges, e, ps, d, lu;
  int initialized = 0;
  int finalized = 0;
  struct S_borderexc (*bx)[2][OH_DIMENSION][2] =
    (struct S_borderexc(*)[2][OH_DIMENSION][2])state->border_exchange;

  if (!bx) return;
  MPI_Initialized(&initialized);
  if (initialized) MPI_Finalized(&finalized);
  if (!initialized || finalized) return;
  for (e=0; e<ne; e++) {
    for (ps=0; ps<2; ps++) {
      for (d=0; d<OH_DIMENSION; d++) {
        for (lu=OH_LOWER; lu<=OH_UPPER; lu++) {
          if (bx[e][ps][d][lu].send.deriv)
            MPI_Type_free(&bx[e][ps][d][lu].send.type);
          if (bx[e][ps][d][lu].recv.deriv)
            MPI_Type_free(&bx[e][ps][d][lu].recv.type);
        }
      }
    }
  }
}

void
oh3_free_context_state(struct oh_state *state) {
  if (!state || oh_context_is_default_state(state) || !state->owns_level3_storage)
    return;

  free_border_exchange_types(state);
  free(state->subdomains);
  free(state->subdomains_float);
  free(state->grid);
  free(state->subdomain_desc);
  free(state->boundaries);
  free(state->adjacent);
  free(state->field_types);
  free(state->field_desc);
  free(state->boundary_comm_fields);
  free(state->boundary_comm_types);
  free(state->border_exchange);

  state->subdomains = NULL;
  state->subdomains_float = NULL;
  state->grid = NULL;
  state->subdomain_desc = NULL;
  state->boundaries = NULL;
  state->adjacent = NULL;
  state->n_of_fields = 0;
  state->field_types = NULL;
  state->field_desc = NULL;
  state->n_of_exchanges = 0;
  state->boundary_comm_fields = NULL;
  state->boundary_comm_types = NULL;
  state->border_exchange = NULL;
  state->n_of_boundaries = 0;
  state->owns_level3_storage = 0;
}
void
oh3_grid_size_(double size[OH_DIMENSION]) {
  oh3_grid_size(size);
}
void
oh3_grid_size(double size[OH_DIMENSION]) {
  oh3_grid_size_state(oh1_state(), size);
}
void
oh3_grid_size_state(struct oh_state *state, double size[OH_DIMENSION]) {
  state_grid_size(state, size);
}
static void
state_grid_size(struct oh_state *state, double size[OH_DIMENSION]) {
  int d, n, nn=state->n_of_nodes;
  for (d=0; d<OH_DIMENSION; d++) {
    double s = (state->grid[d].gsize = size[d]);
    state->grid[d].rgsize = 1 / s;
    for (n=0; n<nn; n++) {
      state->subdomains_float[n][d][OH_LOWER] *= s;
      state->subdomains_float[n][d][OH_UPPER] *= s;
    }
    state->grid[d].fcoord[OH_LOWER] *= s;
    state->grid[d].fcoord[OH_UPPER] *= s;
    state->grid[d].fsize *= s;
    state->grid[d].light.rfsize /= s;
    state->grid[d].light.rfsizeplus /= s;
    state->grid[d].light.fthresh *= s;
    if (state->subdomain_desc) {
      for (n=0; n<nn; n++) {
        state->subdomain_desc[n].coord[d].fc[OH_LOWER] *= s;
        state->subdomain_desc[n].coord[d].fc[OH_UPPER] *= s;
      }
    }
  }
}
int
oh3_transbound_(int *currmode, int *stats) {
  return oh3_transbound_state(oh1_state(), *currmode, *stats);
}
int
oh3_transbound(int currmode, int stats) {
  return oh3_transbound_state(oh1_state(), currmode, stats);
}
int
oh3_transbound_state(struct oh_state *state, int currmode, int stats) {
  return transbound3(state, currmode, stats, 3);
}
static int
transbound3(struct oh_state *state, int currmode, int stats, int level) {
  int oldp=state->region_id[1], newp;
  int (*field_types)[OH_FTYPE_N]=(int(*)[OH_FTYPE_N])state->field_types;

  currmode = state->exclude_level2 ?
             transbound1_state(state, currmode, stats, 1) :
             transbound2_state(state, currmode, stats, level);
  newp = state->region_id[1];
  if (oldp!=newp) {
    if (oldp>=0)  state_clear_border_exchange(state);
    if (newp>=0)  state_set_field_descriptors(state, field_types,
                                              state->subdomains[newp], 1);
  }
  return(currmode);
}
#define Map_Particle_To_Neighbor(STATE,XYZ,RID,DIM,N,INC) {\
  double xyz=*XYZ;\
  if (xyz<(STATE)->subdomains_float[RID][DIM][OH_LOWER]) {\
    N -= INC;\
    if (xyz<(STATE)->grid[DIM].fcoord[OH_LOWER]) {\
      if ((STATE)->boundaries[RID][DIM][OH_LOWER])  return(-1);\
      *XYZ += (STATE)->grid[DIM].fcoord[OH_UPPER] -\
              (STATE)->grid[DIM].fcoord[OH_LOWER];\
    }\
  } else if (xyz>=(STATE)->subdomains_float[RID][DIM][OH_UPPER]) {\
    N += INC;\
    if (xyz>=(STATE)->grid[DIM].fcoord[OH_UPPER]) {\
      if ((STATE)->boundaries[RID][DIM][OH_UPPER])  return(-1);\
      *XYZ -= (STATE)->grid[DIM].fcoord[OH_UPPER] -\
              (STATE)->grid[DIM].fcoord[OH_LOWER];\
    }\
  }\
}
#define Neighbor_Id(STATE,N) \
  ((n=(N))<0 ? ((n=-n-1)<(STATE)->n_of_nodes ? n : -1) : n)
#if OH_DIMENSION==1
int
oh3_map_region_to_adjacent_node_(double *x, int *ps) {
  return(oh3_map_particle_to_neighbor(x, *ps));
}
int
oh3_map_particle_to_neighbor_(double *x, int *ps) {
  return(oh3_map_particle_to_neighbor(x, *ps));
}
int
oh3_map_particle_to_neighbor(double *x, int ps) {
  return oh3_map_particle_to_neighbor_state(oh1_state(), x, NULL, NULL, ps);
}
#elif OH_DIMENSION==2
int
oh3_map_region_to_adjacent_node_(double *x, double *y, int *ps) {
  return(oh3_map_particle_to_neighbor(x, y, *ps));
}
int
oh3_map_particle_to_neighbor_(double *x, double *y, int *ps) {
  return(oh3_map_particle_to_neighbor(x, y, *ps));
}
int
oh3_map_particle_to_neighbor(double *x, double *y, int ps) {
  return oh3_map_particle_to_neighbor_state(oh1_state(), x, y, NULL, ps);
}
#else
int
oh3_map_region_to_adjacent_node_(double *x, double *y, double *z, int *ps) {
  return(oh3_map_particle_to_neighbor(x, y, z, *ps));
}
int
oh3_map_particle_to_neighbor_(double *x, double *y, double *z, int *ps) {
  return(oh3_map_particle_to_neighbor(x, y, z, *ps));
}
int
oh3_map_particle_to_neighbor(double *x, double *y, double *z, int ps) {
  return oh3_map_particle_to_neighbor_state(oh1_state(), x, y, z, ps);
}
#endif
int
oh3_map_particle_to_neighbor_state(struct oh_state *state, double *x,
                                   double *y, double *z, int ps) {
  return state_map_particle_to_neighbor(state, x, y, z, ps);
}
static int
state_map_particle_to_neighbor(struct oh_state *state, double *x, double *y,
                               double *z, int ps) {
  int rid=state->region_id[ps], n=OH_NEIGHBORS>>1;

  Map_Particle_To_Neighbor(state, x, rid, OH_DIM_X, n, 1);
#if OH_DIMENSION>=2
  Map_Particle_To_Neighbor(state, y, rid, OH_DIM_Y, n, 3);
#endif
#if OH_DIMENSION>=3
  Map_Particle_To_Neighbor(state, z, rid, OH_DIM_Z, n, 9);
#endif
  return(Neighbor_Id(state, state->neighbors[ps][n]));
}
#define Map_Particle_To_Subdomain(STATE,XYZ,DIM,SDOM) {\
  double thresh = (STATE)->grid[DIM].light.fthresh;\
  if (XYZ<(STATE)->grid[DIM].fcoord[OH_LOWER] ||\
      XYZ>=(STATE)->grid[DIM].fcoord[OH_UPPER])\
    return(-1);\
  if (XYZ<thresh)\
    SDOM = (XYZ - (STATE)->grid[DIM].fcoord[OH_LOWER]) *\
           (STATE)->grid[DIM].light.rfsize;\
  else  SDOM = (int)((XYZ - thresh) *\
                     (STATE)->grid[DIM].light.rfsizeplus) + \
               (STATE)->grid[DIM].light.n;\
}
#define Adjust_Subdomain(STATE,XYZ,DIM,SDOM,INC) {\
  if (XYZ<(STATE)->subdomains_float[SDOM][DIM][OH_LOWER])  SDOM-=INC;\
  else if (XYZ>=(STATE)->subdomains_float[SDOM][DIM][OH_UPPER])  SDOM+=INC;\
}
#if OH_DIMENSION==1
int
oh3_map_region_to_node_(double *x) {
  return(oh3_map_particle_to_subdomain(*x));
}
int
oh3_map_particle_to_subdomain_(double *x) {
  return(oh3_map_particle_to_subdomain(*x));
}
int
oh3_map_particle_to_subdomain(double x) {
  return oh3_map_particle_to_subdomain_state(oh1_state(), x, 0.0, 0.0);
}
#elif OH_DIMENSION==2
int
oh3_map_region_to_node_(double *x, double *y) {
  return(oh3_map_particle_to_subdomain(*x, *y));
}
int
oh3_map_particle_to_subdomain_(double *x, double *y) {
  return(oh3_map_particle_to_subdomain(*x, *y));
}
int
oh3_map_particle_to_subdomain(double x, double y) {
  return oh3_map_particle_to_subdomain_state(oh1_state(), x, y, 0.0);
}
#else
int
oh3_map_region_to_node_(double *x, double *y, double *z) {
  return(oh3_map_particle_to_subdomain(*x, *y, *z));
}
int
oh3_map_particle_to_subdomain_(double *x, double *y, double *z) {
  return(oh3_map_particle_to_subdomain(*x, *y, *z));
}
int
oh3_map_particle_to_subdomain(double x, double y, double z) {
  return oh3_map_particle_to_subdomain_state(oh1_state(), x, y, z);
}
#endif
int
oh3_map_particle_to_subdomain_state(struct oh_state *state, double x,
                                    double y, double z) {
  return state_map_particle_to_subdomain(state, x, y, z);
}
static int
state_map_particle_to_subdomain(struct oh_state *state, double x, double y,
                                double z) {
  int sdx, sd;
#if OH_DIMENSION>=2
  int sdy=0, nx=state->grid[OH_DIM_X].n;
#endif
#if OH_DIMENSION>=3
  int sdz=0;
#endif

  if (state->subdomain_desc)
    return state_map_irregular_subdomain(state, x, y, z);
  Map_Particle_To_Subdomain(state, x, OH_DIM_X, sdx);
#if OH_DIMENSION>=2
  Map_Particle_To_Subdomain(state, y, OH_DIM_Y, sdy);
#endif
#if OH_DIMENSION>=3
  Map_Particle_To_Subdomain(state, z, OH_DIM_Z, sdz);
#endif
  sd = sdx;
#if OH_DIMENSION>=2
  sd += nx * sdy;
#endif
#if OH_DIMENSION>=3
  {
    int nxy=nx*state->grid[OH_DIM_Y].n;
    sd += nxy * sdz;
  }
#endif
  Adjust_Subdomain(state, x, OH_DIM_X, sd, 1);
#if OH_DIMENSION>=2
  Adjust_Subdomain(state, y, OH_DIM_Y, sd, nx);
#endif
#if OH_DIMENSION>=3
  {
    int nxy=nx*state->grid[OH_DIM_Y].n;
    Adjust_Subdomain(state, z, OH_DIM_Z, sd, nxy);
  }
#endif
  return(sd);
}
static oh_particle_region_t
offset_level3_map_particle_to_neighbor(const oh_particle_adapter *adapter,
                                       void *particle,
                                       int primary_or_secondary) {
  double *x = oh_particle_adapter_position(adapter, particle, OH_DIM_X);
#if OH_DIMENSION==1
  return state_map_particle_to_neighbor(oh1_state(), x, NULL, NULL,
                                        primary_or_secondary);
#elif OH_DIMENSION==2
  double *y = oh_particle_adapter_position(adapter, particle, OH_DIM_Y);
  return state_map_particle_to_neighbor(oh1_state(), x, y, NULL,
                                        primary_or_secondary);
#else
  double *y = oh_particle_adapter_position(adapter, particle, OH_DIM_Y);
  double *z = oh_particle_adapter_position(adapter, particle, OH_DIM_Z);
  return state_map_particle_to_neighbor(oh1_state(), x, y, z,
                                        primary_or_secondary);
#endif
}
static oh_particle_region_t
offset_level3_map_particle_to_subdomain(const oh_particle_adapter *adapter,
                                        void *particle,
                                        int primary_or_secondary) {
  double *x = oh_particle_adapter_position(adapter, particle, OH_DIM_X);

  (void)primary_or_secondary;
#if OH_DIMENSION==1
  return state_map_particle_to_subdomain(oh1_state(), *x, 0.0, 0.0);
#elif OH_DIMENSION==2
  double *y = oh_particle_adapter_position(adapter, particle, OH_DIM_Y);
  return state_map_particle_to_subdomain(oh1_state(), *x, *y, 0.0);
#else
  double *y = oh_particle_adapter_position(adapter, particle, OH_DIM_Y);
  double *z = oh_particle_adapter_position(adapter, particle, OH_DIM_Z);
  return state_map_particle_to_subdomain(oh1_state(), *x, *y, *z);
#endif
}
static oh_particle_region_t
context_level3_map_particle_to_neighbor(const oh_particle_adapter *adapter,
                                        void *particle,
                                        int primary_or_secondary) {
  struct oh_state *state = (struct oh_state*)adapter->user_data;
  double *x = oh_particle_adapter_position(adapter, particle, OH_DIM_X);

#if OH_DIMENSION==1
  return state_map_particle_to_neighbor(state, x, NULL, NULL,
                                        primary_or_secondary);
#elif OH_DIMENSION==2
  double *y = oh_particle_adapter_position(adapter, particle, OH_DIM_Y);
  return state_map_particle_to_neighbor(state, x, y, NULL,
                                        primary_or_secondary);
#else
  double *y = oh_particle_adapter_position(adapter, particle, OH_DIM_Y);
  double *z = oh_particle_adapter_position(adapter, particle, OH_DIM_Z);
  return state_map_particle_to_neighbor(state, x, y, z,
                                        primary_or_secondary);
#endif
}
static oh_particle_region_t
context_level3_map_particle_to_subdomain(const oh_particle_adapter *adapter,
                                         void *particle,
                                         int primary_or_secondary) {
  struct oh_state *state = (struct oh_state*)adapter->user_data;
  double *x = oh_particle_adapter_position(adapter, particle, OH_DIM_X);

  (void)primary_or_secondary;
#if OH_DIMENSION==1
  return state_map_particle_to_subdomain(state, *x, 0.0, 0.0);
#elif OH_DIMENSION==2
  double *y = oh_particle_adapter_position(adapter, particle, OH_DIM_Y);
  return state_map_particle_to_subdomain(state, *x, *y, 0.0);
#else
  double *y = oh_particle_adapter_position(adapter, particle, OH_DIM_Y);
  double *z = oh_particle_adapter_position(adapter, particle, OH_DIM_Z);
  return state_map_particle_to_subdomain(state, *x, *y, *z);
#endif
}
int
map_irregular_subdomain(double x, double y, double z) {
  return state_map_irregular_subdomain(oh1_state(), x, y, z);
}
static int
state_map_irregular_subdomain(struct oh_state *state, double x, double y,
                              double z) {
  return state_map_irregular(state, x, y, z, OH_DIM_X, 0, state->n_of_nodes);
}
static int
state_map_irregular(struct oh_state *state, double p0, double p1, double p2,
                    int dim, int from, int n) {
  double size=state->grid[dim].fsize;
  int to=from+n, lo, up, i;
  struct S_subdomdesc *sd = state->subdomain_desc;

  lo = state_map_irregular_range(state, p0*2.0-size, dim, from, to);
  up = state_map_irregular_range(state, p0*2.0+size, dim, lo, to);
  for (i=lo; i<up; ) {
    int n = sd[i].coord[dim].n;
    if (p0>=sd[i].coord[dim].fc[OH_LOWER] &&
        p0< sd[i].coord[dim].fc[OH_UPPER]) {
      if (dim<OH_DIMENSION-1) {
        int ret = state_map_irregular(state, p1, p2, 0.0, dim+1, i, n);
        if (ret>=0)  return(ret);
      }
      else
        return(sd[i].id);
    }
    i += n;
  }
  return(-1);
}
static int
state_map_irregular_range(struct oh_state *state, double p, int dim, int from,
                          int to) {
  struct S_subdomdesc *sd = state->subdomain_desc;
  int i;

  if (from==to) return(to);
  if (p<sd[from].coord[dim].fc[OH_LOWER]+sd[from].coord[dim].fc[OH_UPPER])
    return(from);
  if (p>=sd[to-1].coord[dim].fc[OH_LOWER]+sd[to-1].coord[dim].fc[OH_UPPER])
    return(to);
  for (i=(from+to)>>1; from<i; i=(from+to)>>1) {
    if (p<sd[i].coord[dim].fc[OH_LOWER]+sd[i].coord[dim].fc[OH_UPPER])
      to = i;
    else
      from = i;
  }
  return(to);
}
void
oh3_bcast_field_(void *pfld, void *sfld, int *ftype) {
  oh3_bcast_field_state(oh1_state(), pfld, sfld, *ftype-1);
}
void
oh3_bcast_field(void *pfld, void *sfld, int ftype) {
  oh3_bcast_field_state(oh1_state(), pfld, sfld, ftype);
}
void
oh3_bcast_field_state(struct oh_state *state, void *pfld, void *sfld,
                      int ftype) {
  state_bcast_field(state, pfld, sfld, ftype);
}
void
oh3_reduce_field_(void *pfld, void *sfld, int *ftype) {
  oh3_reduce_field_state(oh1_state(), pfld, sfld, *ftype-1);
}
void
oh3_reduce_field(void *pfld, void *sfld, int ftype) {
  oh3_reduce_field_state(oh1_state(), pfld, sfld, ftype);
}
void
oh3_reduce_field_state(struct oh_state *state, void *pfld, void *sfld,
                       int ftype) {
  state_reduce_field(state, pfld, sfld, ftype);
}
void
oh3_allreduce_field_(void *pfld, void *sfld, int *ftype) {
  oh3_allreduce_field_state(oh1_state(), pfld, sfld, *ftype-1);
}
void
oh3_allreduce_field(void *pfld, void *sfld, int ftype) {
  oh3_allreduce_field_state(oh1_state(), pfld, sfld, ftype);
}
void
oh3_allreduce_field_state(struct oh_state *state, void *pfld, void *sfld,
                          int ftype) {
  state_allreduce_field(state, pfld, sfld, ftype);
}
void
oh3_exchange_borders_(void *pfld, void *sfld, int *ctype, int *bcast) {
  oh3_exchange_borders_state(oh1_state(), pfld, sfld, *ctype-1, *bcast);
}
void
oh3_exchange_borders(void *pfld, void *sfld, int ctype, int bcast) {
  oh3_exchange_borders_state(oh1_state(), pfld, sfld, ctype, bcast);
}
void
oh3_exchange_borders_state(struct oh_state *state, void *pfld, void *sfld,
                           int ctype, int bcast) {
  state_exchange_borders(state, pfld, sfld, ctype, bcast);
}
static void
state_require_field_type(struct oh_state *state, int ftype, const char *api) {
  if (!state->field_desc || state->n_of_fields<=0)
    local_errstop("%s requires configured fields", api);
  if (ftype<0 || ftype>=state->n_of_fields)
    local_errstop("%s field type %d outside configured range [0,%d)",
                  api, ftype, state->n_of_fields);
}
static void
state_require_exchange_type(struct oh_state *state, int ctype,
                            const char *api) {
  if (!state->border_exchange || state->n_of_exchanges<=0)
    local_errstop("%s requires configured boundary exchanges", api);
  if (ctype<0 || ctype>=state->n_of_exchanges)
    local_errstop("%s boundary exchange type %d outside configured range [0,%d)",
                  api, ctype, state->n_of_exchanges);
}
static void
state_bcast_field(struct oh_state *state, void *pfld, void *sfld, int ftype) {
  int base;
  int *size;

  state_require_field_type(state, ftype, "oh_bcast_field()");
  base=state->field_desc[ftype].bc.base;
  size=state->field_desc[ftype].bc.size;
  oh1_broadcast_state(state, (double*)pfld+base, (double*)sfld+base,
                      size[0], size[1], MPI_DOUBLE, MPI_DOUBLE);
}
static void
state_reduce_field(struct oh_state *state, void *pfld, void *sfld, int ftype) {
  int base;
  int *size;

  state_require_field_type(state, ftype, "oh_reduce_field()");
  base=state->field_desc[ftype].red.base;
  size=state->field_desc[ftype].red.size;
  oh1_reduce_state(state, (double*)pfld+base, (double*)sfld+base,
                   size[0], size[1], MPI_DOUBLE, MPI_DOUBLE, MPI_SUM,
                   MPI_SUM);
}
static void
state_allreduce_field(struct oh_state *state, void *pfld, void *sfld,
                      int ftype) {
  int base;
  int *size;

  state_require_field_type(state, ftype, "oh_allreduce_field()");
  base=state->field_desc[ftype].red.base;
  size=state->field_desc[ftype].red.size;
  oh1_all_reduce_state(state, (double*)pfld+base, (double*)sfld+base,
                       size[0], size[1], MPI_DOUBLE, MPI_DOUBLE, MPI_SUM,
                       MPI_SUM);
}
static void
state_exchange_borders(struct oh_state *state, void *pfld, void *sfld,
                       int ctype, int bcast) {
  MPI_Status st;
  int d, lu;
  int (*adjacent)[2]=(int(*)[2])state->adjacent;
  struct S_borderexc (*border_exchange)[2][OH_DIMENSION][2] =
    (struct S_borderexc(*)[2][OH_DIMENSION][2])state->border_exchange;
  double *pf=(double*)pfld, *sf=(double*)sfld;

  state_require_exchange_type(state, ctype, "oh_exchange_borders()");
  for (d=0; d<OH_DIMENSION; d++) {
    for (lu=OH_LOWER; lu<=OH_UPPER; lu++) {
      int dst=adjacent[d][lu], src=adjacent[d][1-lu];
      struct S_borderexc *bx=&border_exchange[ctype][0][d][lu];
      int scount=bx->send.count;
      int rcount=bx->recv.count;
      if (scount && rcount)
        MPI_Sendrecv(pf+bx->send.buf, scount, bx->send.type, dst, 0,
                     pf+bx->recv.buf, rcount, bx->recv.type, src, 0,
                     state->comm, &st);
      else if (scount)
        MPI_Send(pf+bx->send.buf, scount, bx->send.type, dst, 0,
                 state->comm);
      else if (rcount)
        MPI_Recv(pf+bx->recv.buf, rcount, bx->recv.type, src, 0, state->comm,
                 &st);
    }
  }
  if (Mode_PS(state->curr_mode) && bcast) {
    if (state->region_id[1]>=0 &&
        border_exchange[ctype][1][OH_DIM_X][OH_LOWER].send.count<0)
      state_set_border_exchange(state, ctype, 1, MPI_DOUBLE);
    for (d=0; d<OH_DIMENSION; d++) {
      for (lu=OH_LOWER; lu<=OH_UPPER; lu++) {
        struct S_borderexc *bxp=&border_exchange[ctype][0][d][lu];
        struct S_borderexc *bxs=&border_exchange[ctype][1][d][lu];
        oh1_broadcast_state(state, pf+bxp->recv.buf, sf+bxs->recv.buf,
                            bxp->recv.count, bxs->recv.count,
                            bxp->recv.type, bxs->recv.type);
      }
    }
  }
}
