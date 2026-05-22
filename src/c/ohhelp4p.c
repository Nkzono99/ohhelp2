/* File: ohhelp4p.c
   Version 1.1.1 (2015/10/23)
   Copyright (C) 2009-2015  Hiroshi Nakashima <h.nakashima@media.kyoto-u.ac.jp>
                            (ACCMS, Kyoto University)
   This program can be freely used, redistributed and modified for non-
   commercial purpose providing that the copyright notice above remains
   unchanged.
*/
#define EXTERN extern
#include "ohhelp1.h"
#include "ohhelp2.h"
#include "ohhelp3.h"
#include "oh_particle_buffer.h"
#undef  EXTERN
#define EXTERN
#include "ohhelp4p.h"

static struct oh_state* oh4p_state(void);
static struct S_particle* level4_particle_at(struct oh_state* state, int index);
static int  level4_particle_index(struct oh_state* state,
                                  const struct S_particle* part);
static int  level4_particle_is_injected(struct oh_state* state,
                                        const struct S_particle* part);
static OH_nid_t level4_particle_region(struct oh_state* state,
                                       const struct S_particle* part,
                                       int primary_or_secondary);
static int  level4_particle_species(struct oh_state* state,
                                    const struct S_particle* part);
static void level4_set_particle_region(struct oh_state* state,
                                       struct S_particle* part,
                                       OH_nid_t region,
                                       int primary_or_secondary);
static void level4_copy_particle(struct oh_state* state,
                                 struct S_particle* dst,
                                 const struct S_particle* src);
static void level4_push_particle(struct oh_state* state,
                                 struct S_particle** cursor,
                                 const struct S_particle* src);
static void init4p(int** sdid, const int nspec, const int maxfrac,
                   int** totalp, struct S_particle** pbuf, int** pbase,
                   int maxlocalp, struct S_mycommc* mycommc,
                   struct S_mycommf* mycommf, int** nbor, int* pcoord,
                   int** sdoms, int* scoord, const int nbound, int* bcond,
                   int** bounds, int* ftypes, int* cfields, const int cfid,
                   int* ctypes, int** fsizes,
                   const int stats, const int repiter, const int verbose);
static int  transbound4p(int currmode, int stats, const int level);
static int  try_primary4p(const int currmode, const int level,
                          const int stats);
static int  try_stable4p(const int currmode, const int level, const int stats);
static void rebalance4p(const int currmode, const int level, const int stats);
static void exchange_particles4p(const int currmode, const int level,
                                 int reb, int oldp, const int newp,
                                 const int stats);
static void exchange_population(struct oh_state* state, const int currmode,
                                const int nextmode);
static void add_population(struct oh_state* state, dint* npd, const int xl,
                           const int xu, const int yl, const int yu,
                           const int zl, const int zu, const int src);
static int  mpi_allreduce_wrapper(const void* sendbuf, void* recvbuf,
                                  const int count, MPI_Datatype datatype,
                                  MPI_Op op, const int root, MPI_Comm comm);
static void reduce_population(struct oh_state* state,
                              int (*mpired)(const void*, void*, int,
                                            MPI_Datatype, MPI_Op, int,
                                            MPI_Comm));
static struct S_commlist* make_recv_list(const int currmode,
                                         const int level, const int reb,
                                         const int oldp, const int newp,
                                         const int stats);
static void sched_recv(struct oh_state* state, const int currmode,
                       const int reb, const int get,
                       const int stay, const int nid, const int tag,
                       struct S_recvsched_context* context);
static void make_send_sched(const int currmode, const int reb, const int pcode,
                            const int oldp, const int newp,
                            struct S_commlist* hslist, int* nacc, int* nsend);
static void make_send_sched_body(struct oh_state* state, const int psor2,
                                 const int n, const int sdid, const int self,
                                 const int sender, struct S_commlist* rlist,
                                 int* maxhs, int* naccs, int* nsendptr,
                                 struct S_hotspot** hotspot_top);
static int  gather_hspot_recv(struct oh_state* state, const int currmode,
                              const int reb, const struct S_hotspot* hs);
static void gather_hspot_send(struct oh_state* state, const int hsidx,
                              const int pcode, const int rreq,
                              const int nfrom, const int nto,
                              struct S_commlist** hslist, int* sreqptr);
static void gather_hspot_send_body(struct oh_state* state, const int hsidx,
                                   const int psor2, const int n, int dst,
                                   const int sender, struct S_commlist** hslist,
                                   MPI_Request* reqs, int* sreqptr);
static void scatter_hspot_send(struct oh_state* state, const int rreq, int* nacc,
                               struct S_commlist** hslist);
static void scatter_hspot_recv(struct oh_state* state, const int hsidx,
                               const int pcode, const int rreq,
                               const int sreq, const int nfrom, const int nto,
                               int* nacc, int* nsend);
static void scatter_hspot_recv_body(struct oh_state* state, const int hsidx,
                                    const int psor2, const int n,
                                    int* naccptr, int* nsendptr);
static void update_descriptors(struct oh_state* state, const int oldp,
                               const int newp);
static void update_neighbors(struct oh_state* state, const int ps);
static void set_grid_descriptor(struct oh_state* state, const int idx,
                                const int nid);
static void adjust_field_descriptor(struct oh_state* state, const int ps);
static void update_real_neighbors(struct oh_state* state, const int mode,
                                  const int dosec, const int oldp,
                                  const int newp);
static void upd_real_nbr(struct oh_state* state, const int root, const int psp,
                         const int pss, const int nbr, const int dosec,
                         struct S_node* node,
                         struct S_realneighbor rnbrptr[2], int* occur[2]);
static void exchange_xfer_amount(const int trans, const int psnew);
static void state_exchange_xfer_amount4p(struct oh_state* state,
                                         const int trans, const int psnew);
static void count_population(struct oh_state* state, const int nextmode,
                             const int psnew, const int stats);
static void sort_particles(struct oh_state* state, dint*** npg,
                           const int nextmode, const int psnew,
                           const int stats);
static void move_and_sort_primary(struct oh_state* state, dint*** npg,
                                  const int psold, const int stats);
static void sort_received_particles(struct oh_state* state, const int nextmode,
                                    const int psnew, const int stats);
static void move_to_sendbuf_sec4p(const int psold, const int trans,
                                  const int oldp, const int* nacc,
                                  const int nsend, const int stats);
static void move_to_sendbuf_uw4p(struct oh_state* state, const int ps,
                                 const int mysd, const int cbase,
                                 const int nbase);
static void move_to_sendbuf_dw4p(struct oh_state* state, const int ps,
                                 const int mysd, const int ctail,
                                 const int ntail);
static void move_and_sort_secondary(const int psold, const int psnew,
                                    const int trans, const int oldp,
                                    const int* nacc, const int stats);
static void state_set_sendbuf_disps4p(struct oh_state* state, const int trans);
static void xfer_particles(const int trans, const int psnew,
                           struct S_particle* sbuf);
static void state_xfer_particles4p(struct oh_state* state, const int trans,
                                   const int psnew, struct S_particle* sbuf);

static struct oh_state*
oh4p_state(void) {
    struct oh_state* state = oh1_state();
    state->level4_grid_desc = GridDesc;
    state->level4_pbuf_index = PbufIndex;
    state->level4_particle_grid[0] = NOfPGrid[0];
    state->level4_particle_grid[1] = NOfPGrid[1];
    state->level4_particle_grid_total[0] = NOfPGridTotal[0];
    state->level4_particle_grid_total[1] = NOfPGridTotal[1];
    state->level4_particle_grid_out[0] = NOfPGridOut[0];
    state->level4_particle_grid_out[1] = NOfPGridOut[1];
    state->level4_particle_grid_index[0] = NULL;
    state->level4_particle_grid_index[1] = NULL;
    state->level4_particle_grid_out_shadow[0] = NULL;
    state->level4_particle_grid_out_shadow[1] = NULL;
    state->level4_particle_grid_index_shadow[0] = NULL;
    state->level4_particle_grid_index_shadow[1] = NULL;
    state->level4_particle_grid_z = NULL;
    state->level4_hotspot_recv = HSRecv;
    state->level4_hotspot_send = HSSend;
    state->level4_hotspot_recv_from_parent = HSRecvFromParent;
    state->level4_hotspot_receiver = HSReceiver;
    state->level4_hotspot_list = HotSpotList;
    state->level4_hotspot_top = HotSpotTop;
    state->level4_hotspots = &HotSpot[0][0];
    state->level4_horizontal_planes = NULL;
    state->level4_vertical_planes = NULL;
    state->level4_vertical_plane_head = NULL;
    state->level4_alt_sec_recv_list = AltSecRList;
    state->level4_primary_comm_list = NULL;
    state->level4_sec_rl_index = SecRLIndex;
    state->level4_alt_sec_rl_index = NULL;
    state->level4_primary_rl_index = NULL;
    state->level4_histogram_half_type = T_Hgramhalf;
    state->level4_interior_parts = NULL;
    state->level4_grid_overflow_limit = gridOverflowLimit;
    state->level4_boundary_send_buffer = NULL;
    state->level4_first_neighbor = FirstNeighbor;
    state->level4_grid_offset = &GridOffset[0][0];
    state->level4_real_dst_neighbors = RealDstNeighbors;
    state->level4_real_src_neighbors = RealSrcNeighbors;
    state->level4_boundary_condition = &BoundaryCondition[0][0];
    state->level4_z_bound = NULL;
    state->level4_z_bound_shadow = NULL;
    return state;
}

static struct S_particle*
level4_particle_at(struct oh_state* state, int index) {
    return oh_particle_buffer_at(state->particle_adapter,
                                 state->particles, index);
}

static int
level4_particle_index(struct oh_state* state, const struct S_particle* part) {
    return oh_particle_buffer_index(state->particle_adapter,
                                    state->particles, part);
}

static int
level4_particle_is_injected(struct oh_state* state,
                            const struct S_particle* part) {
    return level4_particle_index(state, part) >= state->total_parts;
}

static OH_nid_t
level4_particle_region(struct oh_state* state, const struct S_particle* part,
                       int primary_or_secondary) {
    return (OH_nid_t)state->particle_adapter->get_region(
        state->particle_adapter, part, primary_or_secondary);
}

static int
level4_particle_species(struct oh_state* state, const struct S_particle* part) {
    return Particle_Spec(
        state->particle_adapter->get_species(state->particle_adapter, part) -
        state->spec_base);
}

static void
level4_set_particle_region(struct oh_state* state, struct S_particle* part,
                           OH_nid_t region, int primary_or_secondary) {
    state->particle_adapter->set_region(state->particle_adapter, part, region,
                                        primary_or_secondary);
}

static void
level4_copy_particle(struct oh_state* state, struct S_particle* dst,
                     const struct S_particle* src) {
    oh_particle_buffer_copy(state->particle_adapter, dst, src);
}

static void
level4_push_particle(struct oh_state* state, struct S_particle** cursor,
                     const struct S_particle* src) {
    level4_copy_particle(state, *cursor, src);
    *cursor = oh_particle_buffer_at(state->particle_adapter, *cursor, 1);
}

#ifdef OH_POS_AWARE
#undef Decl_Grid_Info
#undef Subdomain_Id
#undef Primarize_Id
#undef Primarize_Id_Only
#undef Secondarize_Id
#undef Secondary_Injected
#undef Neighbor_Subdomain_Id
#define Decl_Grid_Info() \
  OH_nid_t nidelement;  int subdomid;\
  const int gridmask = state->grid_mask, loggrid = state->log_grid
#define Subdomain_Id(ID, PS) \
  ((nidelement = (ID)) < 0 ? -1 :\
      ((subdomid = nidelement >> loggrid) < OH_NEIGHBORS ?\
          state->abs_neighbors[PS][subdomid] : subdomid - OH_NEIGHBORS))
#define Primarize_Id(P, SD) {\
  const OH_nid_t nidelem =\
    level4_particle_region(state, P, 1) -\
    ((OH_nid_t)(state->n_of_nodes + OH_NEIGHBORS) << loggrid);\
  level4_set_particle_region(state, P, nidelem, 1);\
  SD = Subdomain_Id(nidelem, 1);\
}
#define Primarize_Id_Only(P) \
  level4_set_particle_region(\
      state, P,\
      level4_particle_region(state, P, 1) -\
      ((OH_nid_t)(state->n_of_nodes + OH_NEIGHBORS) << loggrid), 1)
#define Secondarize_Id(P) \
  level4_set_particle_region(\
      state, P,\
      level4_particle_region(state, P, 1) +\
      ((OH_nid_t)(state->n_of_nodes + OH_NEIGHBORS) << loggrid), 1)
#define Secondary_Injected(ID) \
  (((ID) >> loggrid) >= state->n_of_nodes + OH_NEIGHBORS)
#define Neighbor_Subdomain_Id(ID, PS) \
  state->abs_neighbors[PS][(ID) >> loggrid]
#endif

#define Level4_Boundary_Condition(DIM, SIDE) \
  (((int (*)[2])state->level4_boundary_condition)[DIM][SIDE])
#define Level4_Grid_Desc(PS) (state->level4_grid_desc[PS])

#define If_Dim(D, ET, EF)  (OH_DIMENSION>D ? (ET) : (EF))
#define For_Y(LINIT, LCONT, LNEXT) LINIT;
#define For_Z(LINIT, LCONT, LNEXT) LINIT;
#define Do_Y(ACT)
#define Do_Z(ACT)
#if OH_DIMENSION==1
#define Coord_To_Index(GX, GY, GZ, W, DW)  (GX)
#define Index_To_Coord(IDX, GX, GY, GZ, W, DW) {\
  GX = (IDX);  GY = 0;  GZ = 0;\
}
#else
#undef  For_Y
#define For_Y(LINIT, LCONT, LNEXT) for(LINIT; LCONT; LNEXT)
#undef  Do_Y
#define Do_Y(ACT) ACT
#if OH_DIMENSION==2
#define Coord_To_Index(GX, GY, GZ, W, DW)  ((GX) + (GY)*(W))
#define Index_To_Coord(IDX, GX, GY, GZ, W, DW)  {\
  const int idx=(IDX), w=(W);\
  GX = idx % w;  GY = idx / w;  GZ = 0;\
}
#else
#undef  For_Z
#define For_Z(LINIT, LCONT, LNEXT) for(LINIT; LCONT; LNEXT)
#undef  Do_Z
#define Do_Z(ACT) ACT
#define Coord_To_Index(GX, GY, GZ, W, DW)  ((GX) + (GY)*(W) + (GZ)*(DW))
#define Index_To_Coord(IDX, GX, GY, GZ, W, DW)  {\
  const int idx=(IDX), w=(W), dw=(DW);\
  GX = idx % w;  GY = (idx % dw) / w;  GZ = idx / dw;\
}
#endif
#endif

#define Decl_For_All_Grid()\
  int fag_x1, fag_y1, fag_z1;\
  int fag_xidx, fag_yidx, fag_zidx, fag_gx, fag_gy, fag_gz;\
  int fag_w, fag_dw;

#define For_All_Grid(PS, X0, Y0, Z0, X1, Y1, Z1)\
  For_Z((fag_zidx=(Z0), fag_x1=Level4_Grid_Desc(PS).x+(X1),\
         fag_y1=Level4_Grid_Desc(PS).y+(Y1),\
         fag_z1=Level4_Grid_Desc(PS).z+(Z1),\
         fag_w=Level4_Grid_Desc(PS).w, fag_dw=Level4_Grid_Desc(PS).dw,\
         fag_gz=Coord_To_Index(X0,Y0,Z0,fag_w,fag_dw)),\
        (fag_zidx<fag_z1), (fag_zidx++,fag_gz+=fag_dw))\
    For_Y((fag_yidx=(Y0), fag_gy=fag_gz),\
          (fag_yidx<fag_y1), (fag_yidx++,fag_gy+=fag_w))\
      for (fag_xidx=(X0),fag_gx=fag_gy; fag_xidx<fag_x1; fag_xidx++,fag_gx++)

#define For_All_Grid_Abs(PS, X0, Y0, Z0, X1, Y1, Z1)\
  For_Z((fag_zidx=(Z0), fag_x1=(X1), fag_y1=(Y1), fag_z1=(Z1),\
         fag_w=Level4_Grid_Desc(PS).w, fag_dw=Level4_Grid_Desc(PS).dw,\
         fag_gz=Coord_To_Index(X0,Y0,Z0,fag_w,fag_dw)),\
        (fag_zidx<fag_z1), (fag_zidx++,fag_gz+=fag_dw))\
    For_Y((fag_yidx=(Y0), fag_gy=fag_gz),\
          (fag_yidx<fag_y1), (fag_yidx++,fag_gy+=fag_w))\
      for (fag_xidx=(X0),fag_gx=fag_gy; fag_xidx<fag_x1; fag_xidx++,fag_gx++)

#define The_Grid()  (fag_gx)
#define Grid_X()  (fag_xidx)
#define Grid_Y()  (fag_yidx)
#define Grid_Z()  (fag_zidx)

#define URN_PRI 0
#define URN_SEC 1
#define URN_TRN 2

void oh4p_init_(int* sdid, const int* nspec, const int* maxfrac, int* totalp,
                struct S_particle* pbuf, int* pbase, const int* maxlocalp,
                struct S_mycommf* mycomm, int* nbor, int* pcoord, int* sdoms,
                int* scoord, const int* nbound, int* bcond, int* bounds,
                int* ftypes, int* cfields, int* ctypes, int* fsizes,
                const int* stats, const int* repiter, const int* verbose) {
    specBase = 1;
    init4p(&sdid, *nspec, *maxfrac, &totalp, &pbuf, &pbase, *maxlocalp, NULL,
           mycomm, &nbor, pcoord, &sdoms, scoord, *nbound, bcond, &bounds,
           ftypes, cfields, -1, ctypes, &fsizes,
           *stats, *repiter, *verbose);
}

void oh4p_init(int** sdid, const int nspec, const int maxfrac, int** totalp,
               struct S_particle** pbuf, int** pbase, const int maxlocalp,
               void* mycomm, int** nbor, int* pcoord, int** sdoms, int* scoord,
               const int nbound, int* bcond, int** bounds, int* ftypes,
               int* cfields, int* ctypes, int** fsizes,
               const int stats, const int repiter, const int verbose) {
    specBase = 0;
    init4p(sdid, nspec, maxfrac, totalp, pbuf, pbase, maxlocalp,
           (struct S_mycommc*)mycomm, NULL, nbor, pcoord, sdoms, scoord, nbound,
           bcond, bounds, ftypes, cfields, 0, ctypes, fsizes,
           stats, repiter, verbose);
}

#define Allocate_NOfPGrid(BODY, NPG, TYPE, SIZE, MSG) {\
  const int ns2 = state->n_of_species<<1;\
  const int gridsize = SIZE;\
  TYPE *npg = BODY;\
  TYPE **npgp = (TYPE**)mem_alloc(sizeof(TYPE*), ns2, MSG);\
  int s, g, exto=OH_PGRID_EXT<<1;\
  const int base = Coord_To_Index(exto, exto, exto,\
                                  Level4_Grid_Desc(0).w,\
                                  Level4_Grid_Desc(0).dw);\
  if (!npg)\
    BODY = npg = (TYPE*)mem_alloc(sizeof(TYPE), ns2*gridsize, MSG) + base;\
  for (s=0; s<ns2; s++,npg+=gridsize) {\
    npgp[s] = npg;\
    for (g=0; g<gridsize; g++)  npg[g-base] = 0;\
  }\
  NPG[0] = npgp;  NPG[1] = npgp + state->n_of_species;\
}

static int nOfLocalPLimitShadow = -1;

static void init4p(int** sdid, const int nspec, const int maxfrac, int** totalp,
                   struct S_particle** pbuf, int** pbase, int maxlocalp,
                   struct S_mycommc* mycommc, struct S_mycommf* mycommf,
                   int** nbor, int* pcoord, int** sdoms, int* scoord,
                   const int nbound, int* bcond, int** bounds, int* ftypes, int* cfields,
                   const int cfid, int* ctypes, int** fsizes,
                   const int stats, const int repiter, const int verbose) {
    int nn, me, nnns, nnns2, n;
    int(*ft)[OH_FTYPE_N] = (int(*)[OH_FTYPE_N])ftypes;
    int* cf = cfields;
    int(*ct)[2][OH_CTYPE_N] = (int(*)[2][OH_CTYPE_N])ctypes;
    int nf, ne, c, b, size, ps, tr;
    int* nphgram = NULL;
    int* hsr, * rnbr;
    dint* npgdummy = NULL, * npgtdummy = NULL;
    int loggrid;
    dint idmax;

    MPI_Comm_size(MCW, &nn);  nnns = nn * nspec;  nnns2 = nnns << 1;
    TempArray = (int*)mem_alloc(sizeof(int), nn << 2, "TempArray");
    if (*pbuf)
        Particles = *pbuf;
    else
        Particles = *pbuf =
        (struct S_particle*)mem_alloc(sizeof(struct S_particle),
                                      maxlocalp << 1, "Particles");
    SendBuf = Particles + maxlocalp;

    for (nf = 0; ft[nf][OH_FTYPE_ES] > 0; nf++);
    for (ne = 0; cf[ne] + cfid >= 0; ne++);
    FieldTypes = (int(*)[OH_FTYPE_N])
        mem_alloc(sizeof(int), (nf + 2) * OH_FTYPE_N, "FieldTypes");
    BoundaryCommFields = cf =
        (int(*))mem_alloc(sizeof(int), ne + 2, "BoundaryCommFields");
    BoundaryCommTypes = (int(*)[2][OH_CTYPE_N])
        mem_alloc(sizeof(int), (ne + 1) * nbound * 2 * OH_CTYPE_N,
                  "BoundaryCommTypes");
    memcpy(FieldTypes, ft, sizeof(int) * nf * OH_FTYPE_N);
    for (c = 0; c < ne; c++)  cf[c] = cfields[c] + cfid;
    memcpy(BoundaryCommTypes, ct, sizeof(int) * ne * nbound * 2 * OH_CTYPE_N);
    ft = FieldTypes;  ct = BoundaryCommTypes + ne * nbound;
    ft[nf][OH_FTYPE_ES] = 1;
    ft[nf][OH_FTYPE_LO] = 0;  ft[nf][OH_FTYPE_UP] = 0;
    ft[nf][OH_FTYPE_BL] = 0;  ft[nf][OH_FTYPE_BU] = 0;
    ft[nf][OH_FTYPE_RL] = -OH_PGRID_EXT;  ft[nf][OH_FTYPE_RU] = OH_PGRID_EXT;
    ft[nf + 1][OH_FTYPE_ES] = -1;
    cf[ne] = nf;  cf[ne + 1] = -1;
    ct[0][OH_LOWER][OH_CTYPE_FROM] = -OH_PGRID_EXT;
    ct[0][OH_LOWER][OH_CTYPE_TO] = OH_PGRID_EXT;
    ct[0][OH_LOWER][OH_CTYPE_SIZE] = OH_PGRID_EXT;
    ct[0][OH_UPPER][OH_CTYPE_FROM] = 0;
    ct[0][OH_UPPER][OH_CTYPE_TO] = -(OH_PGRID_EXT << 1);
    ct[0][OH_UPPER][OH_CTYPE_SIZE] = OH_PGRID_EXT;
    for (b = 1; b < nbound; b++)
        ct[b][OH_LOWER][OH_CTYPE_FROM] =
        ct[b][OH_LOWER][OH_CTYPE_TO] =
        ct[b][OH_LOWER][OH_CTYPE_SIZE] =
        ct[b][OH_UPPER][OH_CTYPE_FROM] =
        ct[b][OH_UPPER][OH_CTYPE_TO] =
        ct[b][OH_UPPER][OH_CTYPE_SIZE] = 0;

    init3(sdid, nspec, maxfrac, &nphgram, totalp, NULL, NULL, pbuf, pbase,
          maxlocalp, mycommc, mycommf, nbor, pcoord, sdoms, scoord, nbound,
          bcond, bounds, (int*)ft, cf, cfid, (int*)BoundaryCommTypes, fsizes,
          stats, repiter, verbose, 0);

    if (nOfLocalPLimitShadow < 0)
        errstop("oh4p_max_local_particles() has to be called before oh4p_init()");
    else if (maxlocalp < nOfLocalPLimitShadow)
        errstop("argument maxlocalp %d given to oh4p_init() is less than that "
                "calculated by oh4p_max_local_particles() %d",
                maxlocalp, nOfLocalPLimitShadow);

    struct oh_state* state = oh4p_state();
    state->level4_pbuf_index = PbufIndex = NULL;
    struct S_grid* Grid = state->grid;
    struct S_griddesc* GridDesc = state->level4_grid_desc;
    me = state->my_rank;
    set_grid_descriptor(state, 0, me);
    size = GridDesc[0].dw * GridDesc[0].h;
    Allocate_NOfPGrid(npgdummy, NOfPGrid, dint, size, "NOfPGrid");
    Allocate_NOfPGrid(npgtdummy, NOfPGridTotal, dint, size, "NOfPGridTotal");

    size = Coord_To_Index(Grid[OH_DIM_X].size - 1,
                          If_Dim(OH_DIM_Y, Grid[OH_DIM_Y].size - 1, 0),
                          If_Dim(OH_DIM_Z, Grid[OH_DIM_Z].size - 1, 0),
                          GridDesc[0].w, GridDesc[0].dw);
    for (loggrid = 0; size; loggrid++, size >>= 1);
    idmax = (dint)(((nn + OH_NEIGHBORS) << 1) - 1) << loggrid;
    if (idmax > INT_MAX && sizeof(OH_nid_t) == sizeof(int)) {
#if OH_DIMENSION==1
        errstop("local grid size (%d+%d) times number of nodes %d "
                "is too large for OH_nid_t=int and thus OH_BIG_SPACE should be "
                "defined.",
                GridDesc[0].w - (OH_PGRID_EXT << 2), OH_PGRID_EXT << 2, nn);
#elif OH_DIMENSION==2
        errstop("local grid size (%d+%d)*(%d+%d) times number of nodes %d "
                "is too large for OH_nid_t=int and thus OH_BIG_SPACE should be "
                "defined.",
                GridDesc[0].w - (OH_PGRID_EXT << 2), OH_PGRID_EXT << 2,
                GridDesc[0].d - (OH_PGRID_EXT << 2), OH_PGRID_EXT << 2, nn);
#else
        errstop("local grid size (%d+%d)*(%d+%d)*(%d+%d) times number of nodes %d "
                "is too large for OH_nid_t=int and thus OH_BIG_SPACE should be "
                "defined.",
                GridDesc[0].w - (OH_PGRID_EXT << 2), OH_PGRID_EXT << 2,
                GridDesc[0].d - (OH_PGRID_EXT << 2), OH_PGRID_EXT << 2,
                GridDesc[0].h - (OH_PGRID_EXT << 2), OH_PGRID_EXT << 2, nn);
#endif
    }

    logGrid = loggrid;  gridMask = (1 << loggrid) - 1;
    state->log_grid = loggrid;  state->grid_mask = gridMask;
    adjust_field_descriptor(state, 0);

    state->level4_hotspot_list =
        HotSpotList = (struct S_hotspot*)
        mem_alloc(sizeof(struct S_hotspot),
                  2 * nn + 2 * OH_NEIGHBORS + 1, "HotSpotList");
    hsr = (int*)mem_alloc(sizeof(int), OH_NEIGHBORS * nn * nspec, "HSRecv");
    for (n = 0; n < OH_NEIGHBORS; n++, hsr += nnns)
        state->level4_hotspot_recv[n] = hsr;
    state->level4_hotspot_send =
        HSSend = (int*)mem_alloc(sizeof(int), nspec * 3, "HSSend");
    state->level4_hotspot_recv_from_parent =
        HSRecvFromParent = state->level4_hotspot_send + nspec;
    state->level4_hotspot_receiver =
        HSReceiver = state->level4_hotspot_recv_from_parent + nspec;
    MPI_Type_vector(nspec, 1, nn, MPI_INT, &T_Hgramhalf);
    MPI_Type_commit(&T_Hgramhalf);
    for (n = 0; n < nnns2; n++)  NOfSend[n] = 0;

    {
        int* first_neighbor;
        state = oh4p_state();
        first_neighbor = state->level4_first_neighbor;
        for (n = 0; n < OH_NEIGHBORS; n++) {
            const int snbr = state->src_neighbors[n];
            if (snbr >= 0)
                first_neighbor[n] = state->temp_array[snbr] = n;
            else if (snbr < -nn)
                first_neighbor[n] = n;
            else
                first_neighbor[n] = state->temp_array[-(snbr + 1)];
        }
        update_neighbors(state, 0);
    }
    rnbr = (int*)mem_alloc(sizeof(int), nn * 2 * 2 * 2, "RealNeighbors");
    {
        struct S_realneighbor (*real_dst_neighbors)[2] =
            (struct S_realneighbor (*)[2])state->level4_real_dst_neighbors;
        struct S_realneighbor (*real_src_neighbors)[2] =
            (struct S_realneighbor (*)[2])state->level4_real_src_neighbors;
        for (tr = 0; tr < 2; tr++)  for (ps = 0; ps < 2; ps++, rnbr += nn) {
            real_dst_neighbors[tr][ps].n = real_src_neighbors[tr][ps].n = 0;
            real_dst_neighbors[tr][ps].nbor = rnbr;
            real_src_neighbors[tr][ps].nbor = rnbr + nn * 2 * 2;
        }
    }
    update_real_neighbors(state, URN_PRI, 0, -1, -1);

    if (!SubDomainDesc) {
        int (*boundary_condition)[2] =
            (int (*)[2])state->level4_boundary_condition;
        memcpy(boundary_condition, bcond, sizeof(int) * OH_DIMENSION * 2);
    }
    oh4p_state();
}

int oh4p_max_local_particles_(const dint* npmax, const int* maxfrac,
                              const int* minmargin, const int* hsthresh) {
    return(oh4p_max_local_particles(*npmax, *maxfrac, *minmargin, *hsthresh));
}

int oh4p_max_local_particles(const dint npmax, const int maxfrac,
                             const int minmargin, const int hsthresh) {
    const dint npl = (dint)oh2_max_local_particles(npmax, maxfrac, minmargin) +
        ((gridOverflowLimit = hsthresh << 1) << 1);
    const int nplint = npl;

    if (npl > INT_MAX) mem_alloc_error("Particles", 0);
    return((nOfLocalPLimitShadow = nplint));
}

void oh4p_per_grid_histogram_(int* pghgram) {
    oh4p_per_grid_histogram(&pghgram);
}

void oh4p_per_grid_histogram(int** pghgram) {
    struct oh_state* state = oh4p_state();
    struct S_griddesc* GridDesc = state->level4_grid_desc;
    Allocate_NOfPGrid(*pghgram, NOfPGridOut, int,
                      GridDesc[0].dw * GridDesc[0].h,
                      "NOfPGridOut");
    oh4p_state();
}

int oh4p_transbound_(int* currmode, int* stats) {
    return(transbound4p(*currmode, *stats, 4));
}

int oh4p_transbound(int currmode, int stats) {
    return(transbound4p(currmode, stats, 4));
}

static int transbound4p(int currmode, int stats, const int level) {
    int ret = MODE_NORM_SEC;
    struct oh_state* state;
    int nn, ns, ns2, nnns2;
    struct S_particle* tmp;
    int i, ps, s, tp;
    Decl_For_All_Grid();

    stats = stats && statsMode;
    currmode = transbound1(currmode, stats, level);
    state = oh4p_state();
    nn = state->n_of_nodes;  ns = state->n_of_species;
    ns2 = ns << 1;  nnns2 = nn * ns2;

    if (try_primary4p(currmode, level, stats))  ret = MODE_NORM_PRI;
    else if (!Mode_PS(currmode) || !try_stable4p(currmode, level, stats)) {
        rebalance4p(currmode, level, stats);  ret = MODE_REB_SEC;
    }
    state = oh4p_state();
    if (!state->level4_pbuf_index) {
        state->level4_pbuf_index =
            PbufIndex = (int*)mem_alloc(sizeof(int), ns2 + 1, "PbufIndex");
    }
    for (i = 0; i < nnns2; i++) state->n_of_particles_local[i] = 0;
    for (s = 0, tp = 0; s < ns2; s++) {
        state->total_particles[s] = state->total_particles_next[s];
        state->level4_pbuf_index[s] = tp;
        tp += state->total_particles_next[s];
    }
    state->level4_pbuf_index[s] = tp;
    totalParts = state->total_parts = *state->total_local_particles = tp;
    nOfInjections = state->n_of_injections = 0;
    for (s = 0; s < ns2; s++)  state->injected_particles[s] = 0;

    for (ps = 0; ps <= Mode_PS(ret); ps++) {
        const int extio = (ps == 1 && ret < 0) ? OH_PGRID_EXT << 1 : OH_PGRID_EXT;
        for (s = 0; s < ns; s++) {
            dint* npg = state->level4_particle_grid[ps][s];
            For_All_Grid(ps, -extio, -extio, -extio, extio, extio, extio)
                npg[The_Grid()] = 0;
        }
    }
    tmp = state->particles;
    Particles = state->particles = state->send_buffer;
    SendBuf = state->send_buffer = tmp;
    currMode = state->curr_mode = ret < 0 ? -ret : ret;
    return(ret);
}

static int try_primary4p(const int currmode, const int level, const int stats) {
    struct oh_state* state = oh4p_state();
    const int nn = state->n_of_nodes, ns = state->n_of_species;
    const int nnns = nn * ns, me = state->my_rank;
    const int oldp = state->region_id[1];
    int i, s, nsend, * np;
    dint*** npg = Mode_PS(currmode) ? state->level4_particle_grid_total :
                                      state->level4_particle_grid;

    if (!try_primary1(currmode, level, stats)) return(FALSE);
    state = oh4p_state();
    if (Mode_PS(currmode))  update_real_neighbors(state, URN_PRI, 0, -1, -1);
    if (Mode_Acc(currmode)) {
        move_to_sendbuf_primary(Mode_PS(currmode), stats);
        exchange_primary_particles(currmode, stats);
        count_population(oh4p_state(), 0, 0, stats);
        sort_particles(oh4p_state(), NULL, 0, 0, 0);
    } else {
        exchange_population(state, currmode, 0);
        for (s = 0, nsend = 0, np = state->n_of_particles_local;
             s < ns; s++, np += nn) {
            for (i = 0; i < nn; i++)  nsend += np[i] + np[i + nnns];
            nsend -= np[me];
        }
        if (state->total_particles_global[me] + nsend >
            (dint)state->n_of_local_particles_limit) {
            move_to_sendbuf_primary(Mode_PS(currmode), stats);
            exchange_primary_particles(currmode, stats);
            sort_particles(oh4p_state(), npg, 0, 0, stats);
        } else {
            struct S_particle* sbuf = state->send_buffer;
            move_and_sort_primary(state, npg, (oldp >= 0 ? 1 : 0), stats);
            SendBuf = oh_particle_buffer_at(state->particle_adapter,
                                            state->send_buffer,
                                            state->total_particles_global[me]);
            exchange_primary_particles(currmode, stats);
            SendBuf = sbuf;
            sort_received_particles(oh4p_state(), 0, 0, stats);
        }
    }
    state = oh4p_state();
    primaryParts = state->primary_parts =
        *state->secondary_base = state->total_particles_global[me];
    return(TRUE);
}

static int try_stable4p(const int currmode, const int level, const int stats) {
    struct oh_state* state = oh4p_state();

    if (!try_stable1(currmode, (Mode_Acc(currmode) ? level : -level), stats))
        return(FALSE);
    state = oh4p_state();
    exchange_particles4p(currmode, level, 0, state->region_id[1],
                         state->region_id[1], stats);
    return(TRUE);
}

static void rebalance4p(const int currmode, const int level, const int stats) {
    struct oh_state* state = oh4p_state();
    const int me = state->my_rank, ns = state->n_of_species;
    const int oldp = state->region_id[1], amode = Mode_Acc(currmode);
    const int ninj = state->n_of_injections;
    int s, n, newp;

    rebalance1(currmode, (amode ? level : -level), stats);
    state = oh4p_state();
    newp = amode ? state->nodes[me].parentid : state->nodes_next[me].parentid;
    if (ninj && amode && oldp != newp) {
        int* sinj = state->injected_particles + ns;
        int i;
        struct S_particle* p;
        Decl_Grid_Info();
        for (s = 0; s < ns; s++)  sinj[s] = 0;
        if (newp >= 0) {
            for (i = 0, p = level4_particle_at(state, state->total_parts);
                 i < ninj;
                 i++, p = oh_particle_buffer_at(state->particle_adapter, p, 1)) {
                const OH_nid_t nid = level4_particle_region(state, p, 0);
                int sdid;
                if (Secondary_Injected(nid)) {
                    Primarize_Id(p, sdid);  Secondarize_Id(p);
                    if (sdid == newp)  sinj[level4_particle_species(state, p)]++;
                }
            }
        }
    }
    exchange_particles4p(currmode, level, 1, oldp, newp, stats);
    if (!amode) {
        state = oh4p_state();
        set_grid_descriptor(state, 1, newp);
        for (n = 0; n < OH_NEIGHBORS; n++)
            state->neighbors[1][n] = state->neighbors[2][n];
        update_neighbors(state, 1);
    }
}

#define Parent_Old(PCODE)       ((PCODE) & 4)
#define Parent_New(PCODE)       ((PCODE) & 2)
#define Parent_New_Same(PCODE)  (((PCODE) & 3) == 3)
#define Parent_New_Diff(PCODE)  (((PCODE) & 3) == 2)

static void exchange_particles4p(const int currmode, const int level, int reb,
                                 int oldp, const int newp, const int stats) {
    struct oh_state* state = oh4p_state();
    const int trans = !Mode_Acc(currmode) && reb ? 1 : 0;
    int pcode =
        (oldp >= 0 ? 4 : 0) + (newp >= 0 ? 2 : 0) + (oldp == newp ? 1 : 0);
    int psold, psnew;
    int nacc[2] = { 0,0 }, nsend = 0;
    struct S_commlist* hslist;

    if (Mode_Acc(currmode)) {
        int i;
        const int nnns2 = state->n_of_nodes * state->n_of_species * 2;
        if (reb) {
            exchange_particles(state->sec_recv_list, *state->sec_rl_size,
                               oldp, 0, currmode, stats);
            update_descriptors(state, oldp, newp);
            set_grid_descriptor(state, 1, newp);
            state = oh4p_state();
            update_neighbors(state, 1);
            update_real_neighbors(state, URN_SEC, 0, -1, newp);
        } else
            exchange_particles(state->comm_list + state->sl_head_tail[1],
                               state->sec_sl_head_tail[0], oldp, 0,
                               currmode, stats);
        for (i = 0; i < nnns2; i++)  state->n_of_send[i] = 0;
        count_population(state, 1, (Parent_New(pcode) ? 1 : 0), 0);
        reduce_population(state, mpi_allreduce_wrapper);
        reb = 0;  oldp = newp;  pcode = newp >= 0 ? 7 : 0;
    } else
        exchange_population(state, currmode, 1);

    psold = Parent_Old(pcode) ? 1 : 0;
    psnew = Parent_New(pcode) ? 1 : 0;
    hslist = make_recv_list(currmode, level, reb, oldp, newp, stats);
    make_send_sched(currmode, reb, pcode, oldp, newp, hslist, nacc, &nsend);
    state_exchange_xfer_amount4p(state, trans, psnew);

    if ((dint)nacc[0] + (dint)nacc[1] + nsend >
        (dint)state->n_of_local_particles_limit) {
        move_to_sendbuf_sec4p(psold, trans, oldp, nacc, nsend, stats);
        state_xfer_particles4p(state, trans, psnew, state->send_buffer);
        sort_particles(state, NULL, trans + 1, psnew, stats);
    } else {
        move_and_sort_secondary(psold, psnew, trans, oldp, nacc, stats);
        state_xfer_particles4p(state, trans, psnew,
                               state->send_buffer + nacc[0] + nacc[1]);
        sort_received_particles(state, 1, psnew, stats);
    }
}

static void exchange_population(struct oh_state* state, const int currmode,
                                const int nextmode) {
    const int ns = state->n_of_species;
    int s;
    dint** npg = state->level4_particle_grid[0];
    const int ct = state->n_of_exchanges - 1;
    const int exti = OH_PGRID_EXT, exto = exti << 1;
    const struct S_griddesc* gd = state->level4_grid_desc;
    const int x = gd[0].x, y = gd[0].y, z = gd[0].z;
    const int w = gd[0].w, dw = gd[0].dw;
    Decl_For_All_Grid();

    if (Mode_PS(currmode)) {
        reduce_population(state, MPI_Reduce);
        npg = state->level4_particle_grid_total[0];
    } else if (nextmode) {
        npg = state->level4_particle_grid_total[0];
        for (s = 0; s < ns; s++) {
            dint* npgs = state->level4_particle_grid[0][s], * npgt = npg[s];
            For_All_Grid(0, -exti, -exti, -exti, exti, exti, exti)
                npgt[The_Grid()] = npgs[The_Grid()];
        }
    }
    for (s = 0; s < ns; s++) {
        oh3_exchange_borders(npg[s], NULL, ct, 0);
        if (OH_DIMENSION > OH_DIM_Z) {
            add_population(state, npg[s], -exto, x + exto, -exto,
                           y + exto, 0, exti, -dw * exto);
            add_population(state, npg[s], -exto, x + exto, -exto,
                           y + exto, z - exti, z, dw * exto);
        }
        if (OH_DIMENSION > OH_DIM_Y) {
            add_population(state, npg[s], -exto, x + exto, 0, exti, 0, z,
                           -w * exto);
            add_population(state, npg[s], -exto, x + exto, y - exti, y, 0, z,
                           w * exto);
        }
        add_population(state, npg[s], 0, exti, 0, y, 0, z, -exto);
        add_population(state, npg[s], x - exti, x, 0, y, 0, z, exto);
    }
}

static void add_population(struct oh_state* state, dint* npd, const int xl,
                           const int xu, const int yl, const int yu,
                           const int zl, const int zu, const int src) {
    dint* nps = npd + src;
    Decl_For_All_Grid();

    For_All_Grid_Abs(0, xl, yl, zl, xu, yu, zu)
        npd[The_Grid()] += nps[The_Grid()];
}

static int mpi_allreduce_wrapper(const void* sendbuf, void* recvbuf, int count,
                                 MPI_Datatype datatype, MPI_Op op, int root,
                                 MPI_Comm comm) {
    return(MPI_Allreduce(sendbuf, recvbuf, count, datatype, op, comm));
}

static void reduce_population(struct oh_state* state,
                              int (*mpired)(const void*, void*, int,
                                            MPI_Datatype, MPI_Op, int,
                                            MPI_Comm)) {
    const int ft = state->n_of_fields - 1;
    const int base = state->field_desc[ft].red.base;
    const int* size = state->field_desc[ft].red.size;
    struct S_mycommc* mycomm = state->my_comm;

    if (mycomm->black) {
        if (mycomm->prime != MPI_COMM_NULL)
            mpired(state->level4_particle_grid[0][0] + base,
                   state->level4_particle_grid_total[0][0] + base, size[0],
                   MPI_LONG_LONG_INT, MPI_SUM, mycomm->rank, mycomm->prime);
        if (mycomm->sec != MPI_COMM_NULL)
            mpired(state->level4_particle_grid[1][0] + base,
                   state->level4_particle_grid_total[1][0] + base, size[1],
                   MPI_LONG_LONG_INT, MPI_SUM, mycomm->root, mycomm->sec);
    } else {
        if (mycomm->sec != MPI_COMM_NULL)
            mpired(state->level4_particle_grid[1][0] + base,
                   state->level4_particle_grid_total[1][0] + base, size[1],
                   MPI_LONG_LONG_INT, MPI_SUM, mycomm->root, mycomm->sec);
        if (mycomm->prime != MPI_COMM_NULL)
            mpired(state->level4_particle_grid[0][0] + base,
                   state->level4_particle_grid_total[0][0] + base, size[0],
                   MPI_LONG_LONG_INT, MPI_SUM, mycomm->rank, mycomm->prime);
    }
    if (mycomm->prime == MPI_COMM_NULL)
        memcpy(state->level4_particle_grid_total[0][0] + base,
               state->level4_particle_grid[0][0] + base,
               size[0] * sizeof(dint));
}

static struct S_commlist* make_recv_list(const int currmode, const int level, const int reb,
                                         const int oldp, const int newp, const int stats) {
    struct oh_state* state = oh4p_state();
    const int me = state->my_rank, ns = state->n_of_species;
    const int nn = state->n_of_nodes, nnns = nn * ns;
    const int nn2 = nn << 1;
    struct S_node* nodes = reb ? state->nodes_next : state->nodes;
    struct S_node* mynode = nodes + me;
    struct S_node* ch;
    int* first_neighbor = state->level4_first_neighbor;
    int* rl_index = state->rl_index;
    MPI_Comm comm = state->comm;
    struct S_recvsched_context
        context = { 0, 0, 0, 0, 0, 0, 0, 0, state->comm_list };
    int rlsize, rlidx;
    const int ft = state->n_of_fields - 1;
    const int npgbase = state->field_desc[ft].bc.base;
    const int* npgsize = state->field_desc[ft].bc.size;
    const struct S_griddesc* grid_desc = state->level4_grid_desc;
    const int lastg =
        Coord_To_Index(grid_desc[0].x - 1, grid_desc[0].y - 1,
                       grid_desc[0].z - 1, grid_desc[0].w, grid_desc[0].dw);
    struct S_commlist* lastrl;
    int i;

    for (ch = mynode->child; ch; ch = ch->sibling)
        sched_recv(state, currmode, reb, ch->get.sec, ch->stay.sec, ch->id,
                   nnns, &context);
    sched_recv(state, currmode, reb, mynode->get.prime, mynode->stay.prime, me,
               0, &context);
    rlidx = rlsize = context.cptr - state->comm_list;  lastrl = context.cptr - 1;
    if (rlsize == 0 || (lastrl->region < lastg && lastrl->count)) {
        struct S_commlist* rl = lastrl + 1;
        rl->rid = rlsize ? lastrl->rid : me;
        rl->tag = 0;  rl->sid = 0;  rl->count = 0;  rl->region = lastg;
        rlidx = ++rlsize;
    } else
        lastrl->region = lastg;
    if (Mode_Acc(currmode)) {
        state->sec_recv_list = SecRList = state->comm_list + rlidx;  rlsize = 0;
        oh1_broadcast(&rlidx, &rlsize, 1, 1, MPI_INT, MPI_INT);
        oh1_broadcast(state->comm_list, state->sec_recv_list, rlidx, rlsize,
                      T_Commlist, T_Commlist);
        return(state->sec_recv_list + rlsize);
    }
    for (i = 0; i < OH_NEIGHBORS; i++) {
        const int dst = state->dst_neighbors[i], src = state->src_neighbors[i];
        int rc;
        MPI_Status st;
        if (dst == me) {
            rl_index[i] = 0;  continue;
        }
        if (src >= 0) {
            rl_index[i] = rlidx;
            if (dst >= 0)
                MPI_Sendrecv(state->comm_list, rlsize, T_Commlist, dst, 0,
                             state->comm_list + rlidx, nn2, T_Commlist, src, 0,
                             comm, &st);
            else
                MPI_Recv(state->comm_list + rlidx, nn2, T_Commlist, src, 0,
                         comm, &st);
            MPI_Get_count(&st, T_Commlist, &rc);  rlidx += rc;
        } else {
            if (dst >= 0)
                MPI_Send(state->comm_list, rlsize, T_Commlist, dst, 0, comm);
            rl_index[i] = (src < -nn) ? rlidx : rl_index[first_neighbor[i]];
        }
    }
    rl_index[OH_NEIGHBORS] = rlidx;  state->level4_sec_rl_index[OH_NEIGHBORS] = 0;
    AltSecRList = state->level4_alt_sec_recv_list =
        state->sec_recv_list = SecRList = state->comm_list + rlidx;
    if (Mode_PS(currmode)) {
        oh1_broadcast(rl_index, state->level4_sec_rl_index, OH_NEIGHBORS + 1,
                      OH_NEIGHBORS + 1, MPI_INT, MPI_INT);
        oh1_broadcast(state->comm_list, state->sec_recv_list, rlidx,
                      state->level4_sec_rl_index[OH_NEIGHBORS], T_Commlist,
                      T_Commlist);
        AltSecRList = state->level4_alt_sec_recv_list =
            state->comm_list + (rlidx += state->level4_sec_rl_index[OH_NEIGHBORS]);
    }
    if (reb) {
        int altrlsize = 0;
        build_new_comm(currmode, -level, 2, stats);
        update_descriptors(state, oldp, newp);
        set_grid_descriptor(state, 2, newp);
        update_real_neighbors(state, URN_TRN, Mode_PS(currmode), oldp, newp);
        oh1_broadcast(&rlsize, &altrlsize, 1, 1, MPI_INT, MPI_INT);
        oh1_broadcast(state->comm_list, state->level4_alt_sec_recv_list,
                      rlsize, altrlsize, T_Commlist, T_Commlist);
        rlidx += altrlsize;
    }
    oh1_broadcast(state->level4_particle_grid_total[0][0] + npgbase,
                  state->level4_particle_grid_total[1][0] + npgbase,
                  npgsize[0], npgsize[1], MPI_LONG_LONG_INT, MPI_LONG_LONG_INT);
    return(state->comm_list + rlidx);
}

#define Sched_Recv_Check(INLOOP, G) {\
  if (nptotal>=nplimit) {\
    cptr->region = G;\
    if (nptotal-nplimit>ovflimit) {\
      const int thresh = ovflimit>>1;\
      const int count = (((nplimit-(nptotal-npt)-1)/thresh) + 1) * thresh;\
      cptr->count = count;  carryover = npt - count;\
      Sched_Recv_Return(INLOOP);\
    } else\
      ret = 1;\
  }\
}

#define Sched_Recv_Return(INLOOP) {\
  if (INLOOP) {\
    context->x = Grid_X();  context->y = Grid_Y();  context->z = Grid_Z();\
    context->g = The_Grid();\
  }\
  context->nptotal = nptotal;  context->carryover = carryover;\
  context->cptr = cptr + 1;\
  return;\
}

#define For_All_Grid_From(X0, Y0, Z0)\
  For_Z((fag_zidx=(Z0),\
         fag_x1=Level4_Grid_Desc(0).x, fag_y1=Level4_Grid_Desc(0).y,\
         fag_z1=Level4_Grid_Desc(0).z,\
         fag_x0=(X0), fag_y0=(Y0),\
         fag_w=Level4_Grid_Desc(0).w, fag_dw=Level4_Grid_Desc(0).dw,\
         fag_gz=(Z0)*fag_dw, fag_gy=fag_gz+fag_y0*fag_w,\
         fag_gx=fag_gy+fag_x0),\
        (fag_zidx<fag_z1),\
        (fag_zidx++, fag_gz+=fag_dw, fag_gx=fag_gy=fag_gz, fag_x0=fag_y0=0))\
    For_Y((fag_yidx=fag_y0), (fag_yidx<fag_y1),\
          (fag_yidx++, fag_gy+=fag_w, fag_gx=fag_gy, fag_x0=0))\
      for (fag_xidx=fag_x0; fag_xidx<fag_x1; fag_xidx++,fag_gx++)

static void sched_recv(struct oh_state* state, const int currmode,
                       const int reb, const int get, const int stay,
                       const int nid, const int tag,
                       struct S_recvsched_context* context) {
    const int x0 = context->x, y0 = context->y, z0 = context->z, g = context->g;
    const int ovflimit = state->level4_grid_overflow_limit;
    dint nptotal = context->nptotal;
    dint nplimit = context->nplimit;
    dint carryover = context->carryover;
    dint** npg = state->level4_particle_grid_total[0];
    struct S_commlist* cptr = context->cptr;
    struct S_griddesc* GridDesc = state->level4_grid_desc;
    const int ns = state->n_of_species;
    int s, npt = carryover, ret = 0;
    Decl_For_All_Grid();
    int fag_x0, fag_y0, fag_z0;

    if (!Mode_Acc(currmode) && reb && tag)
        nplimit += get;
    else
        nplimit += get + stay;

    context->nplimit = nplimit;
    if (nptotal - carryover >= nplimit)  return;
    cptr->rid = nid;  cptr->tag = tag;  cptr->sid = context->hs;
    cptr->count = 0;
    Sched_Recv_Check(0, g);
    if (carryover) {
        cptr->count = carryover;  cptr->sid = -1;  context->hs++;
        if (!ret) {
            cptr->region = g;  cptr++;
            cptr->rid = nid;  cptr->tag = tag;  cptr->sid = context->hs;
            cptr->count = 0;
        }
    }
    For_All_Grid_From(x0, y0, z0) {
        if (carryover) { carryover = 0;  continue; }
        if (ret) Sched_Recv_Return(1);
        for (s = 0, npt = 0; s < ns; s++)  npt += npg[s][The_Grid()];
        nptotal += npt;
        Sched_Recv_Check(1, The_Grid());
    }
    Sched_Recv_Return(1);
}

static void make_send_sched(const int currmode, const int reb, const int pcode,
                            const int oldp, const int newp, struct S_commlist* hslist,
                            int* nacc, int* nsend) {
    struct oh_state* state = oh4p_state();
    const int psold = Parent_Old(pcode) ? 1 : 0;
    const int ns2 = state->n_of_species << 1, nn = state->n_of_nodes;
    int s, ps, n, h, maxhs = -1;
    int nfrom, nto;
    struct S_commlist* rlist[2] = { state->comm_list, state->sec_recv_list };
    int* rlidx[2] = { state->rl_index, state->level4_sec_rl_index };
    struct S_hotspotbase (*hotspots)[OH_NEIGHBORS] =
        (struct S_hotspotbase (*)[OH_NEIGHBORS])state->level4_hotspots;
    struct S_hotspot* hotspot_top = state->level4_hotspot_list;

    for (s = 0; s < ns2; s++)  state->total_particles_next[s] = 0;
    state->level4_hotspot_top = hotspot_top;
    if (Mode_Acc(currmode)) {
        nfrom = OH_NBR_SELF;  nto = nfrom + 1;
    } else {
        nfrom = 0;  nto = OH_NEIGHBORS;
    }
    for (ps = 0; ps <= psold; ps++) {
        const int root = ps ? oldp : state->my_rank;
        for (n = nfrom; n < nto; n++) {
            int nrev = OH_NEIGHBORS - 1 - n;
            int sdid = state->neighbors[ps][n];
            const int self = n == OH_NBR_SELF && (ps == 0 || Parent_New_Same(pcode));
            struct S_hotspot* hs = hotspot_top++;
            hs->comm = NULL;  hs->next = NULL;  hs->g = 0;  hs->lev = INT_MAX;
            hotspots[ps][n].head = hotspots[ps][n].tail = hs;
            if (sdid < 0)  sdid = -(sdid + 1);
            if (sdid < nn && (sdid != root || n == OH_NBR_SELF))
                make_send_sched_body(state, ps, n, sdid, self, 1,
                                     rlist[ps] + rlidx[ps][nrev], &maxhs,
                                     nacc + ps, nsend, &hotspot_top);
        }
    }
    if (!Mode_Acc(currmode) && Parent_New_Diff(pcode)) {
        struct S_hotspot* hs = hotspot_top++;
        hs->comm = NULL;  hs->next = NULL;  hs->g = 0;  hs->lev = INT_MAX;
        hotspots[2][OH_NBR_SELF].head = hotspots[2][OH_NBR_SELF].tail = hs;
        make_send_sched_body(state, 2, OH_NBR_SELF, newp, 1, 0,
                             state->level4_alt_sec_recv_list,
                             &maxhs, nacc + 1, nsend, &hotspot_top);
    }
    for (h = 0; h <= maxhs; h++) {
        int rreq = 0, sreq;
        struct S_hotspot* hs = hotspots[0][OH_NBR_SELF].head;
        const int self = hs->lev == h;
        if (self)  rreq = gather_hspot_recv(state, currmode, reb, hs);
        gather_hspot_send(state, h, pcode, rreq, nfrom, nto, &hslist, &sreq);
        if (self)  scatter_hspot_send(state, rreq, nacc, &hslist);
        scatter_hspot_recv(state, h, pcode, rreq, sreq, nfrom, nto, nacc, nsend);
    }
    HotSpotTop = state->level4_hotspot_top = hotspot_top;
}

#define Grid_Boundary(N, GS, SD, DIM, PL, PU, OFF) {\
  const int e = OH_PGRID_EXT;\
  const int *b = SubDomains[SD][DIM];\
  const int off = If_Dim(DIM, b[OH_UPPER]-b[OH_LOWER], 0);\
  if (N==0)      { PL = -e;    PU = -(GS);  OFF = off; }\
  else if (N==1) { PL = 0;     PU = 0;      OFF = 0; }\
  else           { PL = (GS);  PU = e;      OFF = -(GS); }\
}

static void make_send_sched_body(struct oh_state* state, const int psor2,
                                 const int n, const int sdid, const int self,
                                 const int sender, struct S_commlist* rlist,
                                 int* maxhs, int* naccptr, int* nsendptr,
                                 struct S_hotspot** hotspot_top_ptr) {
    const int me = state->my_rank, nn = state->n_of_nodes;
    const int ns = state->n_of_species;
    const int ps = psor2 == 2 ? 1 : psor2;
    const int nsor0 = ps ? ns : 0;
    const int nx = n % 3, ny = n / 3 % 3, nz = n / 9;
    struct S_griddesc* GridDesc = state->level4_grid_desc;
    int (*SubDomains)[OH_DIMENSION][2] = state->subdomains;
    int xl, xu, yl, yu, zl, zu, xoff, yoff, zoff, ngoff;
    int rlg = rlist->region;
    int nacc = *naccptr, nsend = *nsendptr;
    struct S_hotspotbase (*hotspots)[OH_NEIGHBORS] =
        (struct S_hotspotbase (*)[OH_NEIGHBORS])state->level4_hotspots;
    struct S_hotspot* hotspot_top = *hotspot_top_ptr;
    Decl_For_All_Grid();

    Grid_Boundary(nx, GridDesc[psor2].x, sdid, OH_DIM_X, xl, xu, xoff);
    Grid_Boundary(ny, GridDesc[psor2].y, sdid, OH_DIM_Y, yl, yu, yoff);
    Grid_Boundary(nz, GridDesc[psor2].z, sdid, OH_DIM_Z, zl, zu, zoff);
    ngoff = Coord_To_Index(xoff, yoff, zoff, GridDesc[psor2].w,
                           GridDesc[psor2].dw);

    For_All_Grid(psor2, xl, yl, zl, xu, yu, zu) {
        const int g = The_Grid();
        const int ng = g + ngoff;
        while (rlg < ng)  rlg = (++rlist)->region;
        if (rlg == ng && rlist->count) {
            struct S_hotspot* hs = hotspots[psor2][n].tail;
            struct S_hotspot* hst = hotspots[psor2][n].tail = hotspot_top++;
            struct S_commlist* rl = rlist;
            int involved = rlist->rid == me, lev, s;
            *hst = *hs;
            hs->g = g;  hs->next = hst;  lev = hs->lev = rlist->sid;
            hs->comm = rlist;
            for (rlist++; rlist->sid >= 0; rlist++)
                involved = involved || rlist->rid == me;
            involved = involved || rlist->rid == me;  rlist++;
            /* involved = involved || (rlist++)->rid==me
               doesn't work if involved has been true */
            hs->n = rlist - rl;  rlg = rlist->region;
            hs->self = self && involved;
            if (self && !involved)
                for (s = 0; s < ns; s++)
                    state->level4_particle_grid_out[ps][s][g] = 0;
            if (lev > *maxhs)  *maxhs = lev;
        } else {
            const int rid = rlist->rid;
            int s;
            if (rid == me && self) {
                for (s = 0; s < ns; s++) {
                    int naccinc = state->level4_particle_grid_out[ps][s][g] =
                        state->level4_particle_grid_total[ps][s][g];
                    nacc += naccinc;
                    state->total_particles_next[nsor0 + s] += naccinc;
                    if (sender)  state->level4_particle_grid[ps][s][g] = 0;
                }
            } else {
                if (self)
                    for (s = 0; s < ns; s++)
                        state->level4_particle_grid_out[ps][s][g] = 0;
                if (sender) {
                    int nofsidx = rlist->tag + rid;               /* [ps][0][rid] */
                    for (s = 0; s < ns; s++, nofsidx += nn) {
                        int nsendinc = state->level4_particle_grid[ps][s][g];
                        nsend += nsendinc;  state->n_of_send[nofsidx] += nsendinc;
                        state->level4_particle_grid[ps][s][g] = nofsidx + 1;
                    }
                }
            }
        }
    }
    *naccptr = nacc; *nsendptr = nsend;
    *hotspot_top_ptr = hotspot_top;
    state->level4_hotspot_top = hotspot_top;
}

#define Is_Boundary(P, B)  (P<OH_PGRID_EXT ? -1 :\
                           (P>=B-OH_PGRID_EXT ? 1 : 0))

static int gather_hspot_recv(struct oh_state* state, const int currmode,
                             const int reb, const struct S_hotspot* hs) {
    const int me = state->my_rank, ns = state->n_of_species;
    const int nn = state->n_of_nodes;
    const int g = hs->g, psold = Mode_PS(currmode) || Mode_Acc(currmode);
    int rreq = 0, nbx, nby, nbz, nx, ny, nz;
    const struct S_node* nodes = reb ? state->nodes_next : state->nodes;
    MPI_Request* reqs = state->requests;
    int** hotspot_recv = state->level4_hotspot_recv;
    struct S_griddesc* GridDesc = state->level4_grid_desc;

    if (Mode_Acc(currmode))
        nbx = nby = nbz = 0;
    else {
        int x, y, z;
        Index_To_Coord(g, x, y, z, GridDesc[0].w, GridDesc[0].dw);
        nbx = Is_Boundary(x, GridDesc[0].x);
        nby = If_Dim(OH_DIM_Y, Is_Boundary(y, GridDesc[0].y), 0);
        nbz = If_Dim(OH_DIM_Z, Is_Boundary(z, GridDesc[0].z), 0);
    }
    for (nz = -1; nz < 2; nz++) {
        if (nz && nz != nbz)  continue;
        for (ny = -1; ny < 2; ny++) {
            if (ny && ny != nby)  continue;
            for (nx = -1; nx < 2; nx++) {
                const int nbr = (nx + 1) + 3 * ((ny + 1) + 3 * (nz + 1));
                int nid = state->dst_neighbors[nbr];
                struct S_node* ch;
                if (nx && nx != nbx)  continue;
                if (nid < 0)  nid = -(nid + 1);
                if (nid >= nn)  continue;
                if (nid != me)
                    MPI_Irecv(hotspot_recv[nbr] + nid * ns, ns, MPI_INT, nid, nbr,
                              state->comm, reqs + rreq++);
                if (nbr == OH_NBR_SELF) {
                    int s, * hsr = hotspot_recv[OH_NBR_SELF] + me * ns;
                    for (s = 0; s < ns; s++)
                        hsr[s] = state->level4_particle_grid[0][s][g];
                }
                if (psold && (nid != me || nbr == OH_NBR_SELF)) {
                    for (ch = nodes[nid].child; ch; ch = ch->sibling) {
                        const int chid = ch->id;
                        MPI_Irecv(hotspot_recv[nbr] + chid * ns, ns, MPI_INT, chid,
                                  nbr, state->comm, reqs + rreq++);
                    }
                }
            }
        }
    }
    return(rreq);
}

static void gather_hspot_send(struct oh_state* state, const int hsidx,
                              const int pcode, const int rreq, const int nfrom,
                              const int nto, struct S_commlist** hslist,
                              int* sreqptr) {
    const int psold = Parent_Old(pcode) ? 1 : 0;
    MPI_Request* reqs = state->requests + rreq;
    int ps, n;

    *sreqptr = 0;
    for (ps = 0; ps <= psold; ps++) {
        for (n = nfrom; n < nto; n++)
            gather_hspot_send_body(state, hsidx, ps, n, state->neighbors[ps][n],
                                   1, hslist, reqs, sreqptr);
    }
    if (Parent_New_Diff(pcode))
        gather_hspot_send_body(state, hsidx, 2, OH_NBR_SELF,
                               state->region_id[1], 0, hslist, reqs, sreqptr);
}

static void gather_hspot_send_body(struct oh_state* state, const int hsidx,
                                   const int psor2, const int n, int dst,
                                   const int sender, struct S_commlist** hslist,
                                   MPI_Request* reqs, int* sreqptr) {
    struct S_hotspotbase (*hotspots)[OH_NEIGHBORS] =
        (struct S_hotspotbase (*)[OH_NEIGHBORS])state->level4_hotspots;
    struct S_hotspot* hs = hotspots[psor2][n].head;
    const int ns = state->n_of_species, g = hs->g, nrec = hs->n * ns;
    const int ps = psor2 == 2 ? 1 : psor2;
    const int nrev = OH_NEIGHBORS - 1 - n;
    struct S_commlist* hsl = *hslist;
    int sreq = *sreqptr;
    int* hotspot_send = state->level4_hotspot_send;
    int* hotspot_recv_from_parent = state->level4_hotspot_recv_from_parent;
    int np, s;

    if (hs->lev != hsidx || (ps == 0 && n == OH_NBR_SELF))  return;
    if (dst < 0)  dst = -(dst + 1);
    if (hs->self)
        MPI_Irecv(hotspot_recv_from_parent, ns, MPI_INT, dst, OH_NEIGHBORS << 1,
                  state->comm, reqs + sreq++);
    hs->comm = NULL;
    if (sender) {
        for (s = 0, np = 0; s < ns; s++)
            np += (hotspot_send[s] = state->level4_particle_grid[ps][s][g]);
        MPI_Send(hotspot_send, ns, MPI_INT, dst, nrev, state->comm);
        if (np) {
            MPI_Irecv(hsl, nrec, T_Commlist, dst, OH_NEIGHBORS + nrev,
                      state->comm, reqs + sreq++);
            hs->comm = hsl;  hsl += nrec;
        }
    }
    *hslist = hsl;  *sreqptr = sreq;
}

static void scatter_hspot_send(struct oh_state* state, const int rreq, int* nacc,
                               struct S_commlist** hslist) {
    struct S_hotspotbase (*hotspots)[OH_NEIGHBORS] =
        (struct S_hotspotbase (*)[OH_NEIGHBORS])state->level4_hotspots;
    struct S_hotspot* hs = hotspots[0][OH_NBR_SELF].head;
    const struct S_commlist* rl = hs->comm;
    struct S_commlist* slhead = *hslist, * sl;
    const int ns = state->n_of_species, nn = state->n_of_nodes;
    const int me = state->my_rank, g = hs->g, nr = hs->n;
    dint** total_grid = state->level4_particle_grid_total[0];
    int** grid_out = state->level4_particle_grid_out[0];
    int** hotspot_recv = state->level4_hotspot_recv;
    int* hotspot_receiver = state->level4_hotspot_receiver;
    int r, ri, s, sinc, * hsr, * nofr;
    dint hst;

    if (rreq)  MPI_Waitall(rreq, state->requests, state->statuses);
    for (s = 0, hst = 0; s < ns; s++)  hst += total_grid[s][g];
    for (ri = 0, sinc = 0, nofr = state->n_of_recv; ri < nr; ri++, nofr += ns) {
        const int count = rl[ri].count, rid = rl[ri].rid;
        int nget = 0;
        for (s = 0; s < ns; s++) {
            const int ng = nofr[s] = (total_grid[s][g] * count) / hst;
            nget += ng;  total_grid[s][g] -= ng;
        }
        for (nget = count - nget; nget > 0;) {
            if (total_grid[sinc][g]) {
                nofr[sinc]++;  total_grid[sinc][g]--;  nget--;
            }
            if (++sinc >= ns)  sinc = 0;
        }
        hst -= count;
        if (rid == me) {
            for (s = 0; s < ns; s++) {
                nget = grid_out[s][g] = nofr[s];
                state->total_particles_next[s] += nget;
            }
            *nacc += count;
        } else {
            MPI_Send(nofr, ns, MPI_INT, rid, OH_NEIGHBORS << 1, state->comm);
        }
    }
    for (s = 0; s < ns; s++)  hotspot_receiver[s] = 0;
    for (r = 0; r <= rreq; r++) {
        const int dst = r == rreq ? me : state->statuses[r].MPI_SOURCE;
        const int nbr = r == rreq ? OH_NBR_SELF : state->statuses[r].MPI_TAG;
        struct S_commlist* slsave;
        int tag;
        hsr = hotspot_recv[nbr] + dst * ns;
        for (s = 0, sl = slsave = slhead, tag = 0; s < ns; s++, tag += nn) {
            int nput = hsr[s], nget = 0;
            if (nput == 0) continue;
            for (ri = hotspot_receiver[s], nofr = state->n_of_recv + ri * ns; ;
                 ri++, nofr += ns) {
                const int ng = nofr[s], ngetsave = nget;
                if (ng) {
                    nget += ng;  *sl = rl[ri];  sl->tag += tag;
                    if (nput > nget) {
                        nofr[s] = 0;  (sl++)->count = ng;
                    } else {
                        nofr[s] -= ((sl++)->count = nput - ngetsave);
                        hotspot_receiver[s] = ri;
                        break;
                    }
                }
            }
            slsave->sid = sl - slsave;  slsave = sl;
        }
        if (r == rreq) {
            hs->comm = sl > slhead ? slhead : NULL;  *hslist = sl;
        } else if (sl > slhead) {
            MPI_Send(slhead, sl - slhead, T_Commlist, dst, OH_NEIGHBORS + nbr,
                     state->comm);
        }
    }
}

static void scatter_hspot_recv(struct oh_state* state, const int hsidx,
                               const int pcode, const int rreq, const int sreq,
                               const int nfrom, const int nto, int* nacc,
                               int* nsend) {
    const int psold = Parent_Old(pcode) ? 1 : 0;
    int ps, n;
    MPI_Status* st = state->statuses + rreq;

    if (sreq > 0)  MPI_Waitall(sreq, state->requests + rreq, st);
    for (ps = 0; ps <= psold; ps++) {
        for (n = nfrom; n < nto; n++) {
            scatter_hspot_recv_body(state, hsidx, ps, n, nacc + ps, nsend);
        }
    }
    if (Parent_New_Diff(pcode)) {
        scatter_hspot_recv_body(state, hsidx, 2, OH_NBR_SELF, nacc + 1, nsend);
    }
}

static void scatter_hspot_recv_body(struct oh_state* state, const int hsidx,
                                    const int psor2, const int n,
                                    int* naccptr, int* nsendptr) {
    const int ns = state->n_of_species, me = state->my_rank;
    const int ps = psor2 == 2 ? 1 : psor2;
    struct S_hotspotbase (*hotspots)[OH_NEIGHBORS] =
        (struct S_hotspotbase (*)[OH_NEIGHBORS])state->level4_hotspots;
    const struct S_hotspot* hs = hotspots[psor2][n].head;
    struct S_commlist* sl = hs->comm;
    const int g = hs->g, self = hs->self;
    int nsend = *nsendptr;
    int slidx, s, si;
    int* hotspot_recv_from_parent = state->level4_hotspot_recv_from_parent;

    if (hs->lev != hsidx)  return;
    hotspots[psor2][n].head = hs->next;
    if (self && ps) {
        int nacc = *naccptr;
        for (s = 0; s < ns; s++) {
            const int nget = state->level4_particle_grid_out[1][s][g] =
                hotspot_recv_from_parent[s];
            nacc += nget;  state->total_particles_next[ns + s] += nget;
        }
        *naccptr = nacc;
    }
    if (!sl)  return;
    slidx = -(sl - state->comm_list + 1);
    for (s = 0, si = 0; s < ns; s++) {
        int mysi = -1, r;
        const int nr = sl[si].sid;
        if (state->level4_particle_grid[ps][s][g] == 0)  continue;
        state->level4_particle_grid[ps][s][g] = slidx - si;
        for (r = 0; r < nr; r++, si++) {
            const int rid = sl[si].rid, count = sl[si].count;
            sl[si].sid = nr;
            if (rid == me && self)  mysi = si;
            else {
                state->n_of_send[sl[si].tag + rid] += count;  nsend += count;
            }
        }
        if (mysi >= 0) {
            struct S_commlist sltmp = sl[mysi];
            sl[mysi] = sl[si - 1];  sl[si - 1] = sltmp;  sl[si - 1].tag = -1;
        }
    }
    *nsendptr = nsend;
}

static void update_descriptors(struct oh_state* state, const int oldp,
                               const int newp) {
    int n;

    if (oldp != newp) {
        if (oldp >= 0)  clear_border_exchange();
        if (newp >= 0) {
            set_field_descriptors((int (*)[OH_FTYPE_N])state->field_types,
                                  state->subdomains[newp], 1);
            adjust_field_descriptor(state, 1);
        }
    }
}

#define Neighbor_Grid_Offset(PS, N, SD, D, XYZ)\
  (N==0 ? 0 : (N<0 ? SubDomains[SD][D][OH_LOWER]-SubDomains[SD][D][OH_UPPER] :\
                     GridDesc[ps].XYZ))

static void update_neighbors(struct oh_state* state, const int ps) {
    int n = 0, nx, ny = 0, nz = 0;
    const int nn = state->n_of_nodes;
    int (*abs_neighbors)[OH_NEIGHBORS] = state->abs_neighbors;
    int* grid_offset = state->level4_grid_offset;
    int (*SubDomains)[OH_DIMENSION][2] = state->subdomains;
    struct S_griddesc* GridDesc = state->level4_grid_desc;

    Do_Z(for (nz = -1; nz < 2; nz++)) {
        Do_Y(for (ny = -1; ny < 2; ny++)) {
            for (nx = -1; nx < 2; nx++, n++) {
                int nbr = state->neighbors[ps][n];
                nbr = abs_neighbors[ps][n] = nbr < 0 ? -(nbr + 1) : nbr;
                if (nbr >= nn)  grid_offset[ps * OH_NEIGHBORS + n] = 0;
                else
                    grid_offset[ps * OH_NEIGHBORS + n] =
                    Coord_To_Index(Neighbor_Grid_Offset(ps, nx, nbr, OH_DIM_X, x),
                                   Neighbor_Grid_Offset(ps, ny, nbr, OH_DIM_Y, y),
                                   Neighbor_Grid_Offset(ps, nz, nbr, OH_DIM_Z, z),
                                   GridDesc[0].w, GridDesc[0].dw);
            }
        }
    }
}

static void set_grid_descriptor(struct oh_state* state, const int idx,
                                const int nid) {
    const int exto2 = OH_PGRID_EXT << 2;
    struct S_griddesc* GridDesc = state->level4_grid_desc;
    struct S_grid* Grid = state->grid;
    int (*SubDomains)[OH_DIMENSION][2] = state->subdomains;
    const int w = GridDesc[idx].w = Grid[OH_DIM_X].size + (exto2);
    const int d = GridDesc[idx].d =
        If_Dim(OH_DIM_Y, Grid[OH_DIM_Y].size + (exto2), 1);

    GridDesc[idx].h = If_Dim(OH_DIM_Z, Grid[OH_DIM_Z].size + (exto2), 1);
    GridDesc[idx].dw = d * w;
    if (nid >= 0) {
        GridDesc[idx].x = SubDomains[nid][OH_DIM_X][OH_UPPER] -
            SubDomains[nid][OH_DIM_X][OH_LOWER];
        GridDesc[idx].y = If_Dim(OH_DIM_Y,
                                 SubDomains[nid][OH_DIM_Y][OH_UPPER] -
                                 SubDomains[nid][OH_DIM_Y][OH_LOWER], 0);
        GridDesc[idx].z = If_Dim(OH_DIM_Z,
                                 SubDomains[nid][OH_DIM_Z][OH_UPPER] -
                                 SubDomains[nid][OH_DIM_Z][OH_LOWER], 0);
    } else {
        GridDesc[idx].x = GridDesc[idx].y = GridDesc[idx].z = -exto2;
        /* to ensure, e.g., x+2*(OH_PGRID_EXT)<=-2*(OH_PGRID_EXT) */
    }
}

static void adjust_field_descriptor(struct oh_state* state, const int ps) {
    const int f = state->n_of_fields - 1, ns = state->n_of_species;
    struct S_flddesc* FieldDesc = state->field_desc;
    int d, fs;

    for (d = 0, fs = 1; d < OH_DIMENSION; d++)  fs *= FieldDesc[f].size[d];
    fs *= ns - 1;
    FieldDesc[f].bc.size[ps] += fs;    FieldDesc[f].red.size[ps] += fs;
}

static void update_real_neighbors(struct oh_state* state, const int mode,
                                  const int dosec, const int oldp,
                                  const int newp) {
    const int me = state->my_rank, nn = state->n_of_nodes, nn4 = nn << 2;
    const int dosec0 = mode != URN_PRI;
    struct S_realneighbor (*real_dst)[2] =
        (struct S_realneighbor (*)[2])state->level4_real_dst_neighbors;
    struct S_realneighbor (*real_src)[2] =
        (struct S_realneighbor (*)[2])state->level4_real_src_neighbors;
    int i, nbridx, ps, * doccur[2], * soccur[2];

    for (i = 0; i < nn4; i++)  state->temp_array[i] = 0;
    doccur[0] = state->temp_array;       doccur[1] = doccur[0] + nn;
    soccur[0] = doccur[1] + nn;  soccur[1] = soccur[0] + nn;

    if (mode == URN_TRN) {
        int* tmp = real_src[1][0].nbor;
        real_src[1][0].n = real_src[0][0].n;
        real_src[1][0].nbor = real_src[0][0].nbor;
        real_src[0][0].nbor = tmp;
    }
    real_dst[0][0].n = real_dst[0][1].n = 0;
    real_src[0][0].n = real_src[0][1].n = 0;
    upd_real_nbr(state, me, 0, 1, 0, dosec0, state->nodes, real_dst[0], doccur);
    upd_real_nbr(state, me, 0, 0, 0, dosec0, state->nodes, real_src[0], soccur);
    if (mode == URN_PRI)  return;

    nbridx = mode == URN_TRN ? 2 : 1;
    upd_real_nbr(state, newp, 0, 1, nbridx, 1, state->nodes, real_dst[0],
                 doccur);
    upd_real_nbr(state, newp, 1, 1, nbridx, 1, state->nodes, real_src[0],
                 soccur);
    if (mode != URN_TRN)  return;

    for (ps = 0; ps < 2; ps++) {
        const int nd = real_dst[0][ps].n;
        const int ns = real_src[0][ps].n;
        for (i = 0; i < nd; i++)  doccur[ps][real_dst[0][ps].nbor[i]] = 0;
        for (i = 0; i < ns; i++)  soccur[ps][real_src[0][ps].nbor[i]] = 0;
    }
    real_dst[1][0].n = real_dst[1][1].n = 0;
    real_src[1][1].n = 0;
    upd_real_nbr(state, me, 0, 1, 0, 1, state->nodes, real_dst[1], doccur);
    upd_real_nbr(state, oldp, 0, 1, 1, 1, state->nodes, real_dst[1], doccur);
    upd_real_nbr(state, newp, 1, 1, 2, dosec, state->nodes_next, real_src[1],
                 soccur);
}

static void upd_real_nbr(struct oh_state* state, const int root, const int psp,
                         const int pss, const int nbr, const int dosec,
                         struct S_node* nodes,
                         struct S_realneighbor rnbrptr[2], int* occur[2]) {
    const int me = state->my_rank;
    struct S_realneighbor* pnbr = rnbrptr + psp, * snbr = rnbrptr + pss;
    int* poccur = occur[psp], * soccur = occur[pss];
    int i;

    if (root < 0)  return;
    if (root != me && !poccur[root]) {
        pnbr->nbor[pnbr->n++] = root;  poccur[root] = 1;
    }
    if (dosec) {
        struct S_node* ch;
        for (ch = nodes[root].child; ch; ch = ch->sibling) {
            const int nid = ch->id;
            if (nid != me && !soccur[nid]) {
                snbr->nbor[snbr->n++] = nid;  soccur[nid] = 1;
            }
        }
    }
    for (i = 0; i < OH_NEIGHBORS; i++) {
        const int nid = state->neighbors[nbr][i];
        struct S_node* ch;
        if (nid < 0 || nid == root)  continue;
        if (!poccur[nid]) {
            pnbr->nbor[pnbr->n++] = nid;  poccur[nid] = 1;
        }
        if (dosec) {
            for (ch = nodes[nid].child; ch; ch = ch->sibling) {
                const int cid = ch->id;
                if (!soccur[cid]) {
                    snbr->nbor[snbr->n++] = cid;  soccur[cid] = 1;
                }
            }
        }
    }
}

static void exchange_xfer_amount(const int trans, const int psnew) {
    state_exchange_xfer_amount4p(oh4p_state(), trans, psnew);
}

static void state_exchange_xfer_amount4p(struct oh_state* state,
                                         const int trans, const int psnew) {
    struct S_realneighbor (*real_dst)[2] =
        (struct S_realneighbor (*)[2])state->level4_real_dst_neighbors;
    struct S_realneighbor (*real_src)[2] =
        (struct S_realneighbor (*)[2])state->level4_real_src_neighbors;
    const struct S_realneighbor* snbr = real_src[trans];
    const struct S_realneighbor* dnbr = real_dst[trans];
    const int nnns = state->n_of_nodes * state->n_of_species;
    int ps, tag, req;

    for (ps = 0, tag = 0, req = 0; ps <= psnew; ps++, tag += nnns) {
        const int n = snbr[ps].n;
        const int* nbor = snbr[ps].nbor;
        int i, * nrbase = state->n_of_recv + tag;
        for (i = 0; i < n; i++, req++) {
            const int nid = nbor[i];
            MPI_Irecv(nrbase + nid, 1, state->level4_histogram_half_type, nid,
                      tag, state->comm, state->requests + req);
        }
    }
    for (ps = 0, tag = 0; ps < 2; ps++, tag += nnns) {
        const int n = dnbr[ps].n;
        const int* nbor = dnbr[ps].nbor;
        int i, * nsbase = state->n_of_send + tag;
        for (i = 0; i < n; i++, req++) {
            const int nid = nbor[i];
            MPI_Isend(nsbase + nid, 1, state->level4_histogram_half_type, nid,
                      tag, state->comm, state->requests + req);
        }
    }
    MPI_Waitall(req, state->requests, state->statuses);
}

static void count_population(struct oh_state* state, const int nextmode,
                             const int psnew, const int stats) {
    int ps, s, t, i, j, tp;
    const int ns = state->n_of_species, exti = OH_PGRID_EXT;
    Decl_For_All_Grid();
    Decl_Grid_Info();

    if (stats) oh1_stats_time(STATS_TB_SORT, nextmode);
    for (ps = 0, t = 0, j = 0, tp = 0; ps <= psnew; ps++) {
        for (s = 0; s < ns; s++, t++) {
            dint* npgs = state->level4_particle_grid[ps][s];
            const int tpn = state->total_particles[t] =
                state->total_particles_next[t];
            tp += tpn;
            For_All_Grid(ps, -exti, -exti, -exti, exti, exti, exti)
                npgs[The_Grid()] = 0;
            for (i = 0; i < tpn; i++, j++) {
                struct S_particle* p = level4_particle_at(state, j);
                const int g = Grid_Position(level4_particle_region(state, p, ps));
                npgs[g]++;
                level4_set_particle_region(state, p,
                                           Combine_Subdom_Pos(OH_NBR_SELF, g),
                                           ps);
            }
        }
        if (ps == 0)  primaryParts = state->primary_parts = tp;
    }
    totalParts = state->total_parts = tp;
    nOfInjections = state->n_of_injections = 0;
}

static void sort_particles(struct oh_state* state, dint*** npg,
                           const int nextmode, const int psnew,
                           const int stats) {
    const int ns = state->n_of_species;
    struct S_particle* p = state->particles;
    int ps, s, t, i, npt;
    Decl_For_All_Grid();
    Decl_Grid_Info();

    if (stats) oh1_stats_time(STATS_TB_SORT, nextmode ? 1 : 0);
    if (!npg)  npg = state->level4_particle_grid;
    for (ps = 0, t = 0, npt = 0; ps <= psnew; ps++) {
        for (s = 0; s < ns; s++, t++) {
            int* npgo = state->level4_particle_grid_out[ps][s];
            dint* npgt = state->level4_particle_grid_total[ps][s];
            const int tpn = state->total_particles_next[t];
            if (nextmode) {
                const int gdidx = ps ? nextmode : 0;
                For_All_Grid(gdidx, 0, 0, 0, 0, 0, 0) {
                    const int np = npgo[The_Grid()];
                    npgt[The_Grid()] = npt;  npt += np;
                }
            } else {
                dint* npgs = npg[ps][s];
                For_All_Grid(0, 0, 0, 0, 0, 0, 0) {
                    const int np = npgo[The_Grid()] = npgs[The_Grid()];
                    npgt[The_Grid()] = npt;  npt += np;
                }
            }
            for (i = 0; i < tpn; i++, p++)
                state->send_buffer[
                    npgt[Grid_Position(level4_particle_region(state, p, ps))]++] =
                    *p;
        }
    }
}

static void move_and_sort_primary(struct oh_state* state, dint*** npg,
                                  const int psold, const int stats) {
    const int nn = state->n_of_nodes, ns = state->n_of_species;
    const int nnns = nn * ns, me = state->my_rank;
    const int ninj = state->n_of_injections;
    struct S_particle* rbb, * p, * sbuf;
    int ps, s, t, i, nacc, mysd, * sbd;
    Decl_For_All_Grid();
    Decl_Grid_Info();

    if (stats) oh1_stats_time(STATS_TB_MOVE, 0);
    for (s = 0, t = 0, nacc = 0, rbb = state->particles;
         s < ns; s++, t += nn) {
        int n, tpn;
        int* npgo = state->level4_particle_grid_out[0][s];
        int* nprime = state->n_of_primaries + t;
        dint* npgs = npg[0][s];
        dint* npgt = state->level4_particle_grid_total[0][s];
        for (n = 0, tpn = 0; n < nn; n++)  tpn += nprime[n] + nprime[n + nnns];
        state->total_particles_next[s] = tpn;
        state->total_particles_next[ns + s] = 0;
        state->recv_buffer_bases[s] = rbb;
        rbb += tpn - state->n_of_particles_local[t + me];
        state->n_of_particles_local[t + me] = 0;
        state->injected_particles[s] = state->injected_particles[ns + s] = 0;
        For_All_Grid(0, 0, 0, 0, 0, 0, 0) {
            const int np = npgo[The_Grid()] = npgs[The_Grid()];
            npgt[The_Grid()] = nacc;  nacc += np;
        }
    }
    state->recv_buffer_bases[s] = rbb;
    sbuf = state->send_buffer + nacc;
    set_sendbuf_disps(psold, -1);
    for (ps = 0, t = 0, p = state->particles, mysd = me; ps <= psold;
         ps++, mysd = -1) {
        for (s = 0, sbd = state->send_buffer_disps; s < ns;
             s++, t++, sbd += nn) {
            dint* npgt = state->level4_particle_grid_total[0][s];
            const int itail = state->total_particles[t];
            for (i = 0; i < itail; i++, p++) {
                const OH_nid_t nid = level4_particle_region(state, p, ps);
                if (nid >= 0) {
                    const int sdid = Neighbor_Subdomain_Id(nid, ps);
                    if (sdid == mysd)
                        state->send_buffer[npgt[Grid_Position(nid)]++] = *p;
                    else             sbuf[sbd[sdid]++] = *p;
                }
            }
        }
    }
    for (i = 0; i < ninj; i++, p++) {
        const OH_nid_t nid = level4_particle_region(state, p, 0);
        const int s = level4_particle_species(state, p);
        int sdid;
        if (nid < 0) continue;
        sdid = Subdomain_Id(nid, 0);
        if (sdid == me)
            state->send_buffer[
                state->level4_particle_grid_total[0][s][Grid_Position(nid)]++] =
                *p;
        else {
            if (sdid >= nn) Primarize_Id(p, sdid);
            sbuf[state->send_buffer_disps[s * nn + sdid]++] = *p;
        }
    }
    set_sendbuf_disps(psold, -1);
}

static void sort_received_particles(struct oh_state* state, const int nextmode,
                                    const int psnew, const int stats) {
    const int ns = state->n_of_species;
    int ps, s;
    struct S_particle* p = state->particles;
    struct S_particle** rbb = state->recv_buffer_bases + 1;
    Decl_Grid_Info();

    if (stats) oh1_stats_time(STATS_TB_SORT, nextmode);
    for (ps = 0; ps <= psnew; ps++) {
        for (s = 0; s < ns; s++, rbb++) {
            dint* npgt = state->level4_particle_grid_total[ps][s];
            const struct S_particle* rbtail = *rbb;
            for (; p < rbtail; p++)
                state->send_buffer[
                    npgt[Grid_Position(level4_particle_region(state, p, ps))]++] =
                    *p;
        }
    }
}

#define Local_Grid_Position(G, NID, PS) \
  ((G) + state->level4_grid_offset[(PS) * OH_NEIGHBORS + ((NID) >> loggrid)])

#define Move_Or_Do(P, PS, MYSD, MOVEIF, ACT) {\
  const OH_nid_t nid = level4_particle_region(state, P, PS);\
  int g = Grid_Position(nid);\
  int sdid, dst;\
  if (nid<0)  continue;\
  sdid = Neighbor_Subdomain_Id(nid, PS);\
  if (sdid!=(MYSD)) g = Local_Grid_Position(g, nid, PS);\
  dst = npg[g];\
  if (dst==0)  { ACT; }\
  else if (MOVEIF) {\
    if (dst>0)  sb[state->n_of_send[dst-1]++] = *P;\
    else {\
      struct S_commlist *hs = state->comm_list - (dst + 1);\
      if (hs->tag<0) {\
        npg[g] = 0;  ACT;\
      } else {\
        sb[state->n_of_send[hs->tag+hs->rid]++] = *P;\
        if (MOVEIF<0)  level4_set_particle_region(state, P, -1, PS);\
        if (--(hs->count)==0)  npg[g]--;\
      }\
    }\
  }\
}

static void move_to_sendbuf_sec4p(const int psold, const int trans, const int oldp,
                                  const int* nacc, const int nsend, const int stats) {
    struct oh_state* state = oh4p_state();
    const int me = state->my_rank, ns = state->n_of_species;
    const int ninj = state->n_of_injections;
    const int nplim = state->n_of_local_particles_limit;
    int ninjp = 0, ninjs = nplim, i;
    struct S_particle* sb = state->send_buffer, * p;
    Decl_Grid_Info();

    if (stats) oh1_stats_time(STATS_TB_MOVE, 1);
    state_set_sendbuf_disps4p(state, trans);

    for (i = 0, p = level4_particle_at(state, state->total_parts);
         i < ninj;
         i++, p = oh_particle_buffer_at(state->particle_adapter, p, 1)) {
        const int s = level4_particle_species(state, p);
        const OH_nid_t nid = level4_particle_region(state, p, 0);
        const int ps = Secondary_Injected(nid) ? 1 : 0;
        dint* npg = state->level4_particle_grid[ps][s];
        if (nid < 0) continue;
        if (ps) {
            Primarize_Id_Only(p);
            Move_Or_Do(p, ps, oldp, 1, (sb[--ninjs] = *p));
        } else
            Move_Or_Do(p, ps, me, 1, (sb[nsend + ninjp++] = *p));
    }
    move_to_sendbuf_uw4p(state, 0, me, 0, 0);
    if (psold) {
        move_to_sendbuf_uw4p(state, 1, oldp, state->primary_parts, nacc[0]);
        move_to_sendbuf_dw4p(state, 1, oldp, state->total_parts,
                             nacc[0] + nacc[1]);
    } else {
        struct S_particle* rbb = state->particles + nacc[0];
        int s;
        for (s = 0; s < ns; s++) {
            state->recv_buffer_bases[ns + s] = rbb;
            rbb += state->total_particles_next[ns + s];
        }
    }
    move_to_sendbuf_dw4p(state, 0, me, state->primary_parts, nacc[0]);

    for (i = 0; i < ninjp; i++) {
        p = oh_particle_buffer_at(state->particle_adapter, state->send_buffer,
                                  nsend + i);
        level4_push_particle(state,
                             state->recv_buffer_bases +
                             level4_particle_species(state, p),
                             p);
    }
    for (i = ninjs; i < nplim; i++) {
        p = oh_particle_buffer_at(state->particle_adapter, state->send_buffer,
                                  i);
        level4_push_particle(state,
                             state->recv_buffer_bases +
                             level4_particle_species(state, p) + ns,
                             p);
    }

    primaryParts = state->primary_parts = *state->secondary_base = nacc[0];
}

static void move_to_sendbuf_uw4p(struct oh_state* state, const int ps,
                                 const int mysd, const int cbase,
                                 const int nbase) {
    const int ns = state->n_of_species;
    const int nsor0 = ps ? ns : 0;
    const int* ctp = state->total_particles + nsor0;
    const int* ntp = state->total_particles_next + nsor0;
    struct S_particle* p, ** rbb = state->recv_buffer_bases + nsor0;
    struct S_particle* sb = state->send_buffer;
    int s, c, d, cn, dn;
    Decl_Grid_Info();

    for (s = 0, c = cbase, d = nbase; s < ns; s++, c = cn, d = dn) {
        dint* npg = state->level4_particle_grid[ps][s];
        cn = c + ctp[s];  dn = d + ntp[s];
        if (d <= c) {
            for (p = state->particles + c; c < cn; c++, p++)
                Move_Or_Do(p, ps, mysd, 1, (state->particles[d++] = *p));
            rbb[s] = state->particles + d;
        } else if (dn > cn) {
            rbb[s] = state->particles + d;
        } else {
            const int cb = c;
            int cm, dm;
            for (p = state->particles + c; c < d; c++, p++)
                Move_Or_Do(p, ps, mysd, -1, (d++));
            cm = c - 1;  dm = d - 1;
            for (p = state->particles + c; c < cn; c++, p++)
                Move_Or_Do(p, ps, mysd, 1, (state->particles[d++] = *p));
            rbb[s] = state->particles + d;
            for (c = dm, d = dm, p = state->particles + c; c >= cb; c--, p--)
                Move_Or_Do(p, ps, mysd, 0, (state->particles[d--] = *p));
        }
    }
}

static void move_to_sendbuf_dw4p(struct oh_state* state, const int ps,
                                 const int mysd, const int ctail,
                                 const int ntail) {
    const int ns = state->n_of_species;
    const int nsor0 = ps ? ns : 0;
    const int* ctp = state->total_particles + nsor0;
    const int* ntp = state->total_particles_next + nsor0;
    struct S_particle* sb = state->send_buffer, * p;
    int s, c, d, cn, dn;
    Decl_Grid_Info();

    cn = ctail;  dn = ntail;
    for (s = ns - 1, c = cn - 1, d = dn - 1; s >= 0; s--, c = cn - 1, d = dn - 1) {
        dint* npg = state->level4_particle_grid[ps][s];
        cn -= ctp[s];  dn -= ntp[s];
        if (c >= d || cn >= dn)  continue;
        for (p = state->particles + c; c >= cn; c--, p--)
            Move_Or_Do(p, ps, mysd, 1, (state->particles[d--] = *p));
    }
}

static void move_and_sort_secondary(const int psold, const int psnew, const int trans,
                                    const int oldp, const int* nacc, const int stats) {
    struct oh_state* state = oh4p_state();
    const int me = state->my_rank, ns = state->n_of_species;
    const int nn = state->n_of_nodes;
    const int mysubdom[2] = { me, oldp }, ninj = state->n_of_injections;
    struct S_realneighbor (*real_src)[2] =
        (struct S_realneighbor (*)[2])state->level4_real_src_neighbors;
    struct S_particle* p, * rbb;
    struct S_particle* sb = state->send_buffer + nacc[0] + nacc[1];
    int* nofr;
    int ps, s, t, npt, i;
    Decl_For_All_Grid();
    Decl_Grid_Info();

    if (stats) oh1_stats_time(STATS_TB_MOVE, 1);
    state_set_sendbuf_disps4p(state, trans);
    for (ps = 0, t = 0, nofr = state->n_of_recv, rbb = state->particles,
         npt = 0; ps <= psnew; ps++) {
        const int nnbr = real_src[trans][ps].n;
        const int* rnbr = real_src[trans][ps].nbor;
        const int gdidx = ps ? trans + 1 : 0;
        for (s = 0; s < ns; s++, t++, nofr += nn) {
            int n, nrec;
            dint* npgt = state->level4_particle_grid_total[ps][s];
            const int* npgo = state->level4_particle_grid_out[ps][s];
            for (n = 0, nrec = 0; n < nnbr; n++)  nrec += nofr[rnbr[n]];
            state->recv_buffer_bases[t] = rbb;  rbb += nrec;
            For_All_Grid(gdidx, 0, 0, 0, 0, 0, 0) {
                const int np = npgo[The_Grid()];
                npgt[The_Grid()] = npt;  npt += np;
            }
        }
    }
    state->recv_buffer_bases[t] = rbb;

    for (ps = 0, p = state->particles, t = 0; ps <= psold; ps++) {
        const int mysd = mysubdom[ps];
        for (s = 0; s < ns; s++, t++) {
            dint* npg = state->level4_particle_grid[ps][s];
            dint* npgt = state->level4_particle_grid_total[ps][s];
            const int itail = state->total_particles[t];
            for (i = 0; i < itail; i++, p++)
                Move_Or_Do(p, ps, mysd, 1,
                           (state->send_buffer[npgt[g]++] = *p));
        }
    }
    for (i = 0; i < ninj; i++, p++) {
        const int s = level4_particle_species(state, p);
        const OH_nid_t nid = level4_particle_region(state, p, 0);
        const int ps = Secondary_Injected(nid) ? 1 : 0;
        const int mysd = mysubdom[ps];
        dint* npg = state->level4_particle_grid[ps][s];
        dint* npgt = state->level4_particle_grid_total[ps][s];
        if (nid < 0) continue;
        if (ps)  Primarize_Id_Only(p);
        Move_Or_Do(p, ps, mysd, 1,
                   (state->send_buffer[npgt[g]++] = *p));
    }
    primaryParts = state->primary_parts = *state->secondary_base = nacc[0];
}

static void state_set_sendbuf_disps4p(struct oh_state* state, const int trans) {
    const int nn = state->n_of_nodes, ns = state->n_of_species;
    struct S_realneighbor (*real_dst)[2] =
        (struct S_realneighbor (*)[2])state->level4_real_dst_neighbors;
    int ps, s, i, np, * sbd;

    for (ps = 0, sbd = state->n_of_send, np = 0; ps < 2; ps++) {
        const int n = real_dst[trans][ps].n;
        const int* nbor = real_dst[trans][ps].nbor;
        for (s = 0; s < ns; s++, sbd += nn) {
            for (i = 0; i < n; i++) {
                const int nid = nbor[i];
                const int nsend = sbd[nid];
                sbd[nid] = np;  np += nsend;
            }
        }
    }
}

static void xfer_particles(const int trans, const int psnew, struct S_particle* sbuf) {
    state_xfer_particles4p(oh4p_state(), trans, psnew, sbuf);
}

static void state_xfer_particles4p(struct oh_state* state, const int trans,
                                   const int psnew, struct S_particle* sbuf) {
    const int nn = state->n_of_nodes, ns = state->n_of_species;
    struct S_realneighbor (*real_dst)[2] =
        (struct S_realneighbor (*)[2])state->level4_real_dst_neighbors;
    struct S_realneighbor (*real_src)[2] =
        (struct S_realneighbor (*)[2])state->level4_real_src_neighbors;
    int ps, s, t, i, req, sdisp, * nofr, * nofs;

    for (ps = 0, t = 0, nofr = state->n_of_recv, req = 0; ps <= psnew; ps++) {
        const int n = real_src[trans][ps].n;
        const int* nbor = real_src[trans][ps].nbor;
        for (s = 0; s < ns; s++, t++, nofr += nn) {
            struct S_particle* rbuf = state->recv_buffer_bases[t];
            for (i = 0; i < n; i++) {
                const int nid = nbor[i];
                const int nrecv = nofr[nid];
                if (nrecv) {
                    MPI_Irecv(rbuf, nrecv, state->particle_mpi_type, nid, t,
                              state->comm, state->requests + req++);
                    rbuf += nrecv;
                }
            }
        }
    }
    for (ps = 0, t = 0, sdisp = 0, nofs = state->n_of_send; ps < 2; ps++) {
        const int n = real_dst[trans][ps].n;
        const int* nbor = real_dst[trans][ps].nbor;
        for (s = 0; s < ns; s++, t++, nofs += nn) {
            for (i = 0; i < n; i++) {
                const int nid = nbor[i];
                const int sdnxt = nofs[nid];
                const int nsend = sdnxt - sdisp;
                nofs[nid] = 0;
                if (nsend) {
                    MPI_Isend(sbuf + sdisp, nsend, state->particle_mpi_type,
                              nid, t, state->comm, state->requests + req++);
                }
                sdisp = sdnxt;
            }
        }
    }
    MPI_Waitall(req, state->requests, state->statuses);
}

static void check_particle_location4p(struct oh_state* state,
                                      const struct S_particle* part,
                                      const int ps, const int s,
                                      const int inj) {
#ifndef OH_NO_CHECK
    const int ns = state->n_of_species;
    const int t = ps ? s + ns : s;
    const int pidx = level4_particle_index(state, part);
    if (ps < 0 || ps > 1 || s < 0 || s >= ns ||
        (state->level4_pbuf_index &&
         (inj ? ((ps && state->region_id[1] < 0) ||
                 pidx >= state->total_parts + state->n_of_injections)
              : (pidx < state->level4_pbuf_index[t] ||
                 pidx >= state->level4_pbuf_index[t + 1])))) {
        local_errstop("'part' argument pointing %c%d%c of the particle buffer is "
                      "inconsistent with 'ps'=%d and 's'=%d",
                      state->spec_base ? '(' : '[', pidx + state->spec_base,
                      state->spec_base ? ')' : ']', ps, s + state->spec_base);
    }
#else
    (void)state;
    (void)part;
    (void)ps;
    (void)s;
    (void)inj;
#endif
}

#define Map_Particle_To_Neighbor(P, XYZ, DIM, MYSD, K, INC, UB, G, IDX) {\
  const double xyz = XYZ;\
  const double gsize = Grid[DIM].gsize;\
  const double lb = Grid[DIM].fcoord[OH_LOWER];\
  const double gf =\
    (G = (xyz-lb)*Grid[DIM].rgsize + Grid[DIM].coord[OH_LOWER]) * gsize;\
  if (gf>xyz) G--;\
  else if (gf+gsize<=xyz) G++;\
  G -= SubDomains[MYSD][DIM][OH_LOWER];  IDX += G;\
  if (G<0) {\
    K -= INC;\
    if (xyz<lb) {\
      if (Boundaries[MYSD][DIM][OH_LOWER]) {\
        level4_set_particle_region(state, P, -1, ps);  return(-1);\
      }\
      XYZ += Grid[DIM].fcoord[OH_UPPER] - lb;\
    }\
    if (G<-OH_PGRID_EXT)  K = -OH_NEIGHBORS;\
  } else if (G>=UB) {\
    double ub = Grid[DIM].fcoord[OH_UPPER];\
    K += INC;\
    if (xyz>=ub) {\
      if (Boundaries[MYSD][DIM][OH_UPPER]) {\
        level4_set_particle_region(state, P, -1, ps);  return(-1);\
      }\
      XYZ -= ub - lb;\
    }\
    G-=UB;\
    if (G>=OH_PGRID_EXT)  K = -OH_NEIGHBORS;\
  }\
}

#define Adjust_Neighbor_Grid(G, N, DIM)\
  if (G<0) G += SubDomains[N][DIM][OH_UPPER]-SubDomains[N][DIM][OH_LOWER];

int oh4p_map_particle_to_neighbor_(struct S_particle* part, const int* ps, const int* s) {
    return(oh4p_map_particle_to_neighbor(part, *ps, *s - 1));
}

int oh4p_map_particle_to_neighbor(struct S_particle* part, const int ps, const int s) {
    struct oh_state* state = oh4p_state();
    const int ns = state->n_of_species, nn = state->n_of_nodes;
    const int inj = level4_particle_is_injected(state, part);
    struct S_grid* Grid = state->grid;
    struct S_griddesc* GridDesc = state->level4_grid_desc;
    int (*SubDomains)[OH_DIMENSION][2] = state->subdomains;
    int (*Boundaries)[OH_DIMENSION][2] = state->boundaries;
    int (*abs_neighbors)[OH_NEIGHBORS] = state->abs_neighbors;
    int x, y, z, w, d, dw, mysd;
    const int psnn = ps ? (s + ns) * nn : s * nn;
    int k = OH_NBR_SELF, idx = 0;
    int gz, gy, gx;
    int sd;
    Decl_Grid_Info();

    check_particle_location4p(state, part, ps, s, inj);
    x = GridDesc[ps].x;  y = GridDesc[ps].y;  z = GridDesc[ps].z;
    w = GridDesc[ps].w;  d = GridDesc[ps].d;  dw = GridDesc[ps].dw;
    mysd = state->region_id[ps];
    Do_Z(Map_Particle_To_Neighbor(part, part->z, OH_DIM_Z, mysd, k, 9, z, gz,
                                  idx));
    Do_Z(idx *= d);
    Do_Y(Map_Particle_To_Neighbor(part, part->y, OH_DIM_Y, mysd, k, 3, y, gy,
                                  idx));
    Do_Y(idx *= w);
    Map_Particle_To_Neighbor(part, part->x, OH_DIM_X, mysd, k, 1, x, gx, idx);

    if (k == OH_NBR_SELF) {
        state->level4_particle_grid[ps][s][idx]++;
        state->n_of_particles_local[psnn + mysd]++;
        level4_set_particle_region(state, part, Combine_Subdom_Pos(k, idx),
                                   ps);
        if (inj) {
            if (ps) {
                state->injected_particles[ns + s]++;  Secondarize_Id(part);
            } else {
                state->injected_particles[s]++;
            }
        }
        return(mysd);
    } else if (k < 0)
        return(oh4p_map_particle_to_subdomain(part, ps, s));
    sd = abs_neighbors[ps][k];
    if (sd >= nn) {
        level4_set_particle_region(state, part, -1, ps);
        return(-1);
    }
    Adjust_Neighbor_Grid(gx, sd, OH_DIM_X);
    Do_Y(Adjust_Neighbor_Grid(gy, sd, OH_DIM_Y));
    Do_Z(Adjust_Neighbor_Grid(gz, sd, OH_DIM_Z));
    state->n_of_particles_local[psnn + sd]++;

    if (sd == mysd) {
        idx = Coord_To_Index(gx, gy, gz, w, dw);
        state->level4_particle_grid[ps][s][idx]++;
        level4_set_particle_region(
            state, part, Combine_Subdom_Pos(OH_NBR_SELF, idx), ps);
        if (inj)  state->injected_particles[ps ? ns + s : s]++;
    } else {
        state->level4_particle_grid[ps][s][idx]++;
        level4_set_particle_region(
            state, part, Combine_Subdom_Pos(k, Coord_To_Index(gx, gy, gz, w, dw)),
            ps);
    }
    if (inj && ps)  Secondarize_Id(part);
    return(sd);
}

#define Map_To_Grid(P, PXYZ, XYZ, DIM, GG, LG) {\
  const double gsize = Grid[DIM].gsize;\
  const double lb = Grid[DIM].fcoord[OH_LOWER];\
  const double ub = Grid[DIM].fcoord[OH_UPPER];\
  double gf;\
  XYZ = PXYZ;\
  LG = 0;\
  if (XYZ<lb) {\
    if (Level4_Boundary_Condition(DIM, OH_LOWER)) {\
      level4_set_particle_region(state, P, -1, ps);  return(-1);\
    }\
    XYZ += (ub - lb);  PXYZ = XYZ;\
    LG = Grid[DIM].coord[OH_LOWER] - Grid[DIM].coord[OH_UPPER];\
  }\
  else if (XYZ>=ub) {\
    if (Level4_Boundary_Condition(DIM, OH_UPPER)) {\
      level4_set_particle_region(state, P, -1, ps);  return(-1);\
    }\
    XYZ -= (ub - lb);  PXYZ = XYZ;\
    LG = Grid[DIM].coord[OH_UPPER] - Grid[DIM].coord[OH_LOWER];\
  }\
  GG = (XYZ-lb)*Grid[DIM].rgsize + Grid[DIM].coord[OH_LOWER];\
  gf = GG * gsize;\
  if (gf>XYZ) GG--;\
  else if (gf+gsize<=XYZ) GG++;\
  LG += GG;\
}

#define Map_Particle_To_Subdomain(XYZ, DIM, SDOM) {\
  double thresh = Grid[DIM].light.thresh;\
  if (XYZ<thresh)\
    SDOM = (XYZ - Grid[DIM].coord[OH_LOWER]) / Grid[DIM].light.size;\
  else\
    SDOM = (XYZ - thresh)/ (Grid[DIM].light.size + 1) + Grid[DIM].light.n;\
}

#define Local_Coordinate(N, MYSD, GG, LG, DIM, K, INC, AA) {\
  GG -= SubDomains[N][DIM][OH_LOWER];\
  if (N==MYSD)  LG = GG;\
  else {\
    const int ub = SubDomains[MYSD][DIM][OH_UPPER];\
    if (LG>=ub+OH_PGRID_EXT)  AA = 1;\
    else {\
      const int inc = LG<ub ? 0 : INC;\
      LG -= SubDomains[MYSD][DIM][OH_LOWER];\
      if (LG<-OH_PGRID_EXT)  AA = 1;\
      k += LG<0 ? -INC : inc;\
    }\
  }\
}

int oh4p_map_particle_to_subdomain_(struct S_particle* part, const int* ps,
                                    const int* s) {
    return(oh4p_map_particle_to_subdomain(part, *ps, *s - 1));
}

int oh4p_map_particle_to_subdomain(struct S_particle* part, const int ps,
                                   const int s) {
    struct oh_state* state = oh4p_state();
    const int ns = state->n_of_species, nn = state->n_of_nodes;
    const int inj = level4_particle_is_injected(state, part);
    struct S_grid* Grid = state->grid;
    struct S_subdomdesc* SubDomainDesc = state->subdomain_desc;
    struct S_griddesc* GridDesc = state->level4_grid_desc;
    int (*SubDomains)[OH_DIMENSION][2] = state->subdomains;
    const int nx = Grid[OH_DIM_X].n;
    const int nxy = If_Dim(OH_DIM_Y, nx * Grid[OH_DIM_Y].n, 0);
    const int t = ps ? ns + s : s;
    int w, dw, mysd;
    int sd;
    double x, y, z;
    int px, py, pz;
    int gx, gy, gz;
    int lx, ly, lz;
    int k = OH_NBR_SELF, aacc = 0;
    Decl_Grid_Info();

    check_particle_location4p(state, part, ps, s, inj);
    w = GridDesc[ps].w;  dw = GridDesc[ps].dw;  mysd = state->region_id[ps];
    Map_To_Grid(part, part->x, x, OH_DIM_X, gx, lx);
    Do_Y(Map_To_Grid(part, part->y, y, OH_DIM_Y, gy, ly));
    Do_Z(Map_To_Grid(part, part->z, z, OH_DIM_Z, gz, lz));
    if (SubDomainDesc) {
        sd = map_irregular_subdomain(x, If_Dim(OH_DIM_Y, y, 0),
                                     If_Dim(OH_DIM_Z, z, 0));
        if (sd < 0) {
            level4_set_particle_region(state, part, -1, ps);
            return(-1);
        }
    } else {
        Map_Particle_To_Subdomain(gx, OH_DIM_X, px);
        Do_Y(Map_Particle_To_Subdomain(gy, OH_DIM_Y, py));
        Do_Z(Map_Particle_To_Subdomain(gz, OH_DIM_Z, pz));
        sd = Coord_To_Index(px, py, pz, nx, nxy);
    }
    Local_Coordinate(sd, mysd, gx, lx, OH_DIM_X, k, 1, aacc);
    Do_Y(Local_Coordinate(sd, mysd, gy, ly, OH_DIM_Y, k, 3, aacc));
    Do_Z(Local_Coordinate(sd, mysd, gz, lz, OH_DIM_Z, k, 9, aacc));
    state->n_of_particles_local[t * nn + sd]++;
    if (aacc) {
        currMode = state->curr_mode = Mode_Set_Any(state->curr_mode);
        level4_set_particle_region(
            state, part,
            Combine_Subdom_Pos(sd + OH_NEIGHBORS,
                               Coord_To_Index(gx, gy, gz, w, dw)),
            ps);
    } else {
        state->level4_particle_grid[ps][s][Coord_To_Index(lx, ly, lz, w, dw)]++;
        level4_set_particle_region(
            state, part, Combine_Subdom_Pos(k, Coord_To_Index(gx, gy, gz, w, dw)),
            ps);
    }
    if (inj) {
        if (sd == mysd)  state->injected_particles[t]++;
        if (ps)  Secondarize_Id(part);
    }
    return(sd);
}

int oh4p_inject_particle_(const struct S_particle* part, const int* ps) {
    return(oh4p_inject_particle(part, *ps));
}

int oh4p_inject_particle(const struct S_particle* part, const int ps) {
    struct oh_state* state = oh4p_state();
    const int ns = state->n_of_species;
    int inj = state->total_parts + state->n_of_injections++;
    struct S_particle* p = level4_particle_at(state, inj);
    int s = level4_particle_species(state, part);
    int sd;

#ifndef OH_HAS_SPEC
    if (!state->use_custom_particle_adapter && ns != 1)
        local_errstop("particles cannot be injected when S_particle does not "
                      "have 'spec' element and you have two or more species");
#endif
    nOfInjections = state->n_of_injections;
    if (inj >= state->n_of_local_particles_limit)
        local_errstop("injection causes local particle buffer overflow");
    level4_copy_particle(state, p, part);
    sd = oh4p_map_particle_to_neighbor(p, ps, s);
    if (sd < 0) {
        state->n_of_injections--;
        nOfInjections = state->n_of_injections;
    }
    return(sd);
}

void oh4p_remove_mapped_particle_(struct S_particle* part, const int* ps, const int* s) {
    oh4p_remove_mapped_particle(part, *ps, *s - 1);
}

void oh4p_remove_mapped_particle(struct S_particle* part, const int ps, const int s) {
    struct oh_state* state = oh4p_state();
    const int nn = state->n_of_nodes, ns = state->n_of_species;
    const int inj = level4_particle_is_injected(state, part);
    int sd, g, psreal = ps, mysd, t;
    OH_nid_t nid = level4_particle_region(state, part, psreal);
    Decl_Grid_Info();

    check_particle_location4p(state, part, psreal, s, inj);
    if (nid < 0)  return;
    sd = Subdomain_Id(nid, psreal);
    g = Grid_Position(nid);
    if (sd >= nn) {
        psreal = 1;  Primarize_Id(part, sd);
        nid = level4_particle_region(state, part, psreal);
    }
    mysd = state->region_id[psreal];
    level4_set_particle_region(state, part, -1, psreal);
    t = psreal ? ns + s : s;
    state->n_of_particles_local[t * nn + sd]--;
    if (inj && sd == mysd)  state->injected_particles[t]--;
    if (Mode_Acc(state->curr_mode))  return;
    if (sd != mysd)  g = Local_Grid_Position(g, nid, psreal);
    state->level4_particle_grid[psreal][s][g]--;
}

int oh4p_remap_particle_to_neighbor_(struct S_particle* part, const int* ps, const int* s) {
    return(oh4p_remap_particle_to_neighbor(part, *ps, *s - 1));
}

int oh4p_remap_particle_to_neighbor(struct S_particle* part, const int ps, const int s) {
    oh4p_remove_mapped_particle(part, ps, s);
    return(oh4p_map_particle_to_neighbor(part, ps, s));
}

int oh4p_remap_particle_to_subdomain_(struct S_particle* part, const int* ps, const int* s) {
    return(oh4p_remap_particle_to_subdomain(part, *ps, *s - 1));
}

int oh4p_remap_particle_to_subdomain(struct S_particle* part, const int ps, const int s) {
    oh4p_remove_mapped_particle(part, ps, s);
    return(oh4p_map_particle_to_subdomain(part, ps, s));
}
