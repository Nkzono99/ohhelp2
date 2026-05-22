/* File: ohhelp4s.c
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
#include "ohhelp3.h"
#include "ohhelp3_internal.h"
#include "oh_context_internal.h"
#include "oh_particle_buffer.h"
#include "ohhelp4_particle.h"
#undef  EXTERN
#define EXTERN
#include "ohhelp4s_internal.h"
#include "ohhelp4_state.h"

static struct oh_state* oh4s_state(void);
static void init4s(int** sdid, const int nspec, const int maxfrac,
                   const dint npmax, const int minmargin, const int maxdensity,
                   int** totalp, int** pbase, int* maxlocalp, int* cbufsize,
                   struct S_mycommc* mycommc, struct S_mycommf* mycommf,
                   int** nbor, int* pcoord, int** sdoms, int* scoord,
                   const int nbound, int* bcond, int** bounds, int* ftypes,
                   int* cfields, const int cfid, int* ctypes, int** fsizes,
                   int** zbound,
                   const int stats, const int repiter, const int verbose);
static int  transbound4s(int currmode, int stats, const int level);
static int  try_primary4s(const int currmode, const int level,
                          const int stats);
static int  try_stable4s(const int currmode, const int level, const int stats);
static void rebalance4s(const int currmode, const int level, const int stats);
static void exchange_particles4s(int currmode, const int nextmode,
                                 const int level, int reb, int oldp, int newp,
                                 const int stats);
static void count_population(struct oh_state* state, const int nextmode,
                             const int psnew, const int stats);
static void exchange_population(struct oh_state* state, const int currmode);
static void reduce_population(struct oh_state* state);
static void add_population(struct oh_state* state, dint* npd, const int xl,
                           const int xu, const int yl, const int yu,
                           const int zl, const int zu, const int src);
static void make_recv_list(const int currmode, const int level, const int reb,
                           const int oldp, const int newp, const int stats);
static void sched_recv(struct oh_state* state, const int reb, const int get,
                       const int stay, const int nid, const int tag,
                       struct S_recvsched_context* context);
static void make_send_sched(const int reb, const int pcode, const int oldp,
                            const int newp, struct S_commlist* rlist[2],
                            int* rlidx[2], int* nacc, int* nsendptr);
static int  make_send_sched_body(struct oh_state* state, const int ps,
                                 const int n, const int sdid,
                                 struct S_commlist* rlist);
static void make_send_sched_self(struct oh_state* state, const int psor2,
                                 struct S_commlist* rlist, int* naccptr);
static void make_send_sched_hplane(struct oh_state* state, const int psor2,
                                   const int z, int* naccptr, int* np,
                                   int* buf);
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
static void exchange_xfer_amount(const int trans, const int psnew,
                                 const int nextmode);
static void state_exchange_xfer_amount4s(struct oh_state* state,
                                         const int trans, const int psnew,
                                         const int nextmode);
static void make_bxfer_sched(struct oh_state* state, const int trans,
                             const int psnew,
                             struct S_commlist* rlist[2], int* rlidx[2]);
static void make_bsend_sched(struct oh_state* state, const int psor2,
                             const int n, const int nx, const int ny,
                             struct S_commlist* rlist, int* nsendptr,
                             int* vpptr);
static void make_brecv_sched(struct oh_state* state, const int psor2,
                             const int n, const int nx, const int ny,
                             struct S_commlist* rlist, int* nrecvptr,
                             int vpidx);
static void move_to_sendbuf_4s(const int nextmode, const int psold,
                               const int psnew, const int trans,
                               const int oldp, const int* nacc,
                               const int nsend, const int stats);
static void move_to_sendbuf_uw4s(struct oh_state* state, const int ps,
                                 const int mysd, const int cbase,
                                 const int nbase);
static void move_to_sendbuf_dw4s(struct oh_state* state, const int ps,
                                 const int mysd, const int ctail,
                                 const int ntail);
static void sort_particles(struct oh_state* state, const int nextmode,
                           const int psnew, const int stats);
static void move_and_sort(const int nextmode, const int psold, const int psnew,
                          const int oldp, const int* nacc, const int stats);
static void sort_received_particles(struct oh_state* state, const int nextmode,
                                    const int psnew, const int stats);
static void state_set_sendbuf_disps4s(struct oh_state* state,
                                      const int nextmode, const int trans);
static void xfer_particles(const int trans, const int psnew,
                           const int nextmode, struct S_particle* sbuf);
static void state_xfer_particles4s(struct oh_state* state, const int trans,
                                   const int psnew, const int nextmode,
                                   struct S_particle* sbuf);
static void xfer_boundary_particles_v(struct oh_state* state, const int psnew,
                                      const int pcode, const int d);
static void xfer_boundary_particles_h(struct oh_state* state, const int psnew);
static void exchange_border_data_v(struct oh_state* state, void* buf,
                                   void* sbuf, void* rbuf, MPI_Datatype type,
                                   const MPI_Aint esize, const int d);
static void exchange_border_data_h(struct oh_state* state, void* buf,
                                   MPI_Datatype type,
                                   const MPI_Aint esize);

static struct oh_state*
oh4s_state(void) {
    struct oh_state* state = oh1_state();
    level4_bind_common_state(state);
    state->level4_particle_grid_index[0] = NOfPGridIndex[0];
    state->level4_particle_grid_index[1] = NOfPGridIndex[1];
    state->level4_particle_grid_out_shadow[0] = NOfPGridOutShadow[0];
    state->level4_particle_grid_out_shadow[1] = NOfPGridOutShadow[1];
    state->level4_particle_grid_index_shadow[0] = NOfPGridIndexShadow[0];
    state->level4_particle_grid_index_shadow[1] = NOfPGridIndexShadow[1];
    state->level4_particle_grid_z = NOfPGridZ;
    state->level4_hotspot_recv = NULL;
    state->level4_hotspot_send = NULL;
    state->level4_hotspot_recv_from_parent = NULL;
    state->level4_hotspot_receiver = NULL;
    state->level4_hotspot_list = NULL;
    state->level4_hotspot_top = NULL;
    state->level4_hotspots = NULL;
    state->level4_horizontal_planes = &HPlane[0][0];
    state->level4_vertical_planes = VPlane;
    state->level4_vertical_plane_head = VPlaneHead;
    state->level4_primary_comm_list = &PrimaryCommList[0][0];
    state->level4_alt_sec_rl_index = AltSecRLIndex;
    state->level4_primary_rl_index = PrimaryRLIndex;
    state->level4_interior_parts = InteriorParts;
    state->level4_grid_overflow_limit = 0;
    state->level4_boundary_send_buffer = BoundarySendBuf;
    state->level4_z_bound = &ZBound[0][0];
    state->level4_z_bound_shadow = ZBoundShadow ? &ZBoundShadow[0][0] : NULL;
    return state;
}

#define Level4_Boundary_Condition(DIM, SIDE) \
  (((int (*)[2])state->level4_boundary_condition)[DIM][SIDE])
#define Level4_Grid_Desc(PS) (state->level4_grid_desc[PS])

#define If_Dim(D, ET, EF)  (ET)
#define For_Y(LINIT, LCONT, LNEXT) for(LINIT; LCONT; LNEXT)
#define For_Z(LINIT, LCONT, LNEXT) for(LINIT; LCONT; LNEXT)
#define Do_Y(ACT) ACT
#define Do_Z(ACT) ACT
#define Coord_To_Index(GX, GY, GZ, W, DW)  ((GX) + (GY)*(W) + (GZ)*(DW))
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

void oh4s_init_(int* sdid, const int* nspec, const int* maxfrac, const dint* npmax,
                const int* minmargin, const int* maxdensity, int* totalp,
                int* pbase, int* maxlocalp, int* cbufsize, struct S_mycommf* mycomm,
                int* nbor, int* pcoord, int* sdoms, int* scoord, const int* nbound,
                int* bcond, int* bounds, int* ftypes, int* cfields, int* ctypes,
                int* fsizes, int* zbound,
                const int* stats, const int* repiter, const int* verbose) {
    specBase = 1;
    init4s(&sdid, *nspec, *maxfrac, *npmax, *minmargin, *maxdensity, &totalp,
           &pbase, maxlocalp, cbufsize, NULL, mycomm, &nbor, pcoord, &sdoms,
           scoord, *nbound, bcond, &bounds, ftypes, cfields, -1, ctypes, &fsizes,
           &zbound,
           *stats, *repiter, *verbose);
}

void oh4s_init(int** sdid, const int nspec, const int maxfrac, const dint npmax,
               const int minmargin, const int maxdensity, int** totalp,
               int** pbase, int* maxlocalp, int* cbufsize, void* mycomm,
               int** nbor, int* pcoord, int** sdoms, int* scoord, const int nbound,
               int* bcond, int** bounds, int* ftypes, int* cfields, int* ctypes,
               int** fsizes, int** zbound,
               const int stats, const int repiter, const int verbose) {
    specBase = 0;
    init4s(sdid, nspec, maxfrac, npmax, minmargin, maxdensity, totalp,
           pbase, maxlocalp, cbufsize, (struct S_mycommc*)mycomm, NULL, nbor,
           pcoord, sdoms, scoord, nbound, bcond, bounds, ftypes, cfields, 0,
           ctypes, fsizes, zbound,
           stats, repiter, verbose);
}

#define Allocate_NOfPGrid(BODY, NPG, TYPE, SIZE, MSG) {\
  const int ns2 = state->n_of_species<<1;\
  const int gridsize = SIZE;\
  TYPE *npg = BODY;\
  TYPE **npgp = (TYPE**)mem_alloc(sizeof(TYPE*), ns2, MSG);\
  int s, g, exto=OH_PGRID_EXT*3;\
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

static void init4s(int** sdid, const int nspec, const int maxfrac, const dint npmax,
                   const int minmargin, const int maxdensity, int** totalp, int** pbase,
                   int* maxlocalp, int* cbufsize, struct S_mycommc* mycommc,
                   struct S_mycommf* mycommf, int** nbor, int* pcoord, int** sdoms,
                   int* scoord, const int nbound, int* bcond, int** bounds, int* ftypes,
                   int* cfields, const int cfid, int* ctypes, int** fsizes, int** zbound,
                   const int stats, const int repiter, const int verbose) {
    int nn, me, nnns, nnns2, n;
    int(*ft)[OH_FTYPE_N] = (int(*)[OH_FTYPE_N])ftypes;
    int* cf = cfields;
    int(*ct)[2][OH_CTYPE_N] = (int(*)[2][OH_CTYPE_N])ctypes;
    int nf, ne, c, b, size, ps, s, tr, i, x, y, z;
    int* nphgram = NULL;
    int* rnbr, * iptr;
    dint* npgdummy = NULL, * npgtdummy = NULL;
    const int ext = OH_PGRID_EXT, ext2 = ext << 1, ext3 = ext * 3;
    struct S_particle pbufdummy, * pbufdummyptr = &pbufdummy;
    dint npl;

    if (OH_DIMENSION != 3)
        errstop("dimension size %d is not 3 which level-4s extension requires.",
                OH_DIMENSION);
    if (OH_PGRID_EXT != 1)
        errstop("boundary plane thickness %d is not 1 which level-4s extension "
                "requires.", OH_PGRID_EXT);

    MPI_Comm_size(MCW, &nn);  nnns = nn * nspec;  nnns2 = nnns << 1;
    TempArray = (int*)mem_alloc(sizeof(int), nn << 2, "TempArray");

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
    ft[nf][OH_FTYPE_BL] = -ext;  ft[nf][OH_FTYPE_BU] = ext;
    ft[nf][OH_FTYPE_RL] = -ext;  ft[nf][OH_FTYPE_RU] = ext;
    ft[nf + 1][OH_FTYPE_ES] = -1;
    cf[ne] = nf;  cf[ne + 1] = -1;
    ct[0][OH_LOWER][OH_CTYPE_FROM] = -ext;
    ct[0][OH_LOWER][OH_CTYPE_TO] = ext;
    ct[0][OH_LOWER][OH_CTYPE_SIZE] = ext2;
    ct[0][OH_UPPER][OH_CTYPE_FROM] = -ext;
    ct[0][OH_UPPER][OH_CTYPE_TO] = -ext3;
    ct[0][OH_UPPER][OH_CTYPE_SIZE] = ext2;
    for (b = 1; b < nbound; b++)
        ct[b][OH_LOWER][OH_CTYPE_FROM] =
        ct[b][OH_LOWER][OH_CTYPE_TO] =
        ct[b][OH_LOWER][OH_CTYPE_SIZE] =
        ct[b][OH_UPPER][OH_CTYPE_FROM] =
        ct[b][OH_UPPER][OH_CTYPE_TO] =
        ct[b][OH_UPPER][OH_CTYPE_SIZE] = 0;

    init3(sdid, nspec, maxfrac, &nphgram, totalp, NULL, NULL, &pbufdummyptr,
          pbase, 0, mycommc, mycommf, nbor, pcoord, sdoms, scoord, nbound,
          bcond, bounds, (int*)ft, cf, cfid, (int*)BoundaryCommTypes, fsizes,
          stats, repiter, verbose, 0);
    struct oh_state* state = oh4s_state();
    struct S_grid* grid = state->grid;
    struct S_griddesc* GridDesc = state->level4_grid_desc;

    size =
        ((grid[OH_DIM_X].size + ext2) * (grid[OH_DIM_Y].size + ext2) *
         (grid[OH_DIM_Z].size + ext2) -
         grid[OH_DIM_X].size * grid[OH_DIM_Y].size * grid[OH_DIM_Z].size) +
        grid[OH_DIM_X].size * grid[OH_DIM_Y].size;
    npl = (dint)oh2_max_local_particles(npmax, maxfrac, minmargin) +
        2 * maxdensity * size;
    if (npl > INT_MAX) mem_alloc_error("Particles", 0);
    nOfLocalPLimitShadow = *maxlocalp = npl;
    size =
        2 * maxdensity * grid[OH_DIM_Z].size *
        ((grid[OH_DIM_X].size + ext2) * (grid[OH_DIM_Y].size + ext2) -
         grid[OH_DIM_X].size * grid[OH_DIM_Y].size);
    state->level4_boundary_send_buffer = BoundarySendBuf =
        (struct S_particle*)mem_alloc(
            oh_particle_buffer_stride(state->particle_adapter), size,
            "BoundarySendBuf");
    size = grid[OH_DIM_X].size + ext2;
    if (size < grid[OH_DIM_Y].size)  size = grid[OH_DIM_Y].size;
    *cbufsize = 2 * maxdensity * grid[OH_DIM_Z].size * size;

    state->level4_pbuf_index = PbufIndex = NULL;
    me = state->my_rank;
    set_grid_descriptor(state, 0, me);
    size = GridDesc[0].dw * GridDesc[0].h;
    Allocate_NOfPGrid(npgdummy, NOfPGrid, dint, size, "NOfPGrid");
    Allocate_NOfPGrid(npgtdummy, NOfPGridTotal, dint, size, "NOfPGridTotal");
    state->level4_particle_grid_z =
        NOfPGridZ = (dint*)mem_alloc(sizeof(dint), grid[OH_DIM_Z].size,
                                     "NOfPGridZ");

    size = Coord_To_Index(grid[OH_DIM_X].size - 1,
                          If_Dim(OH_DIM_Y, grid[OH_DIM_Y].size - 1, 0),
                          If_Dim(OH_DIM_Z, grid[OH_DIM_Z].size - 1, 0),
                          GridDesc[0].w, GridDesc[0].dw);

    int loggrid;
    for (loggrid = 0; size; loggrid++, size >>= 1);

    dint idmax;
    idmax = (dint)(((nn + OH_NEIGHBORS) << 1) - 1) << loggrid;
    if (idmax > INT_MAX && sizeof(OH_nid_t) == sizeof(int)) {
        const int ext6 = ext3 << 1;
        errstop("local grid size (%d+%d)*(%d+%d)*(%d+%d) times number of nodes %d "
                "is too large for OH_nid_t=int and thus OH_BIG_SPACE should be "
                "defined.",
                GridDesc[0].w - ext6, ext6,
                GridDesc[0].d - ext6, ext6,
                GridDesc[0].h - ext6, ext6, nn);
    }

    logGrid = loggrid;  gridMask = (1 << loggrid) - 1;
    state->log_grid = loggrid;  state->grid_mask = gridMask;

    adjust_field_descriptor(state, 0);

    iptr = (int*)mem_alloc(sizeof(int), 2 * 2 * 4 * nspec, "HPlane");
    state->level4_horizontal_planes = &HPlane[0][0];
    for (ps = 0; ps < 2; ps++)  for (i = OH_LOWER; i <= OH_UPPER; i++) {
        struct S_hplane (*hplanes)[2] =
            (struct S_hplane (*)[2])state->level4_horizontal_planes;
        hplanes[ps][i].nsend = iptr;  iptr += nspec;
        hplanes[ps][i].nrecv = iptr;  iptr += nspec;
        hplanes[ps][i].sbuf = iptr;  iptr += nspec;
        hplanes[ps][i].rbuf = iptr;  iptr += nspec;
        hplanes[ps][i].nbor = MPI_PROC_NULL;
    }
    size = 2 * nn + 2 * 2 + 2;
    state->level4_vertical_planes =
        VPlane = (struct S_vplane*)mem_alloc(sizeof(struct S_vplane), size,
                                             "VPlane");
    memset(state->level4_vertical_plane_head, 0,
           sizeof(int) * (2 * 2 * 2 + 1));

    iptr = *zbound;
    if (!iptr)  iptr = *zbound = mem_alloc(sizeof(int), 4, "ZBound");
    ZBoundShadow = (int(*)[2])iptr;
    state->level4_z_bound_shadow = &ZBoundShadow[0][0];
    {
        int (*z_bound)[2] = (int (*)[2])state->level4_z_bound;
        int (*z_bound_shadow)[2] = (int (*)[2])state->level4_z_bound_shadow;
        z_bound[0][OH_LOWER] = 0;  z_bound[0][OH_UPPER] = GridDesc[0].z;
        z_bound[1][OH_LOWER] = z_bound[1][OH_UPPER] = 0;
        memcpy(z_bound_shadow, z_bound, sizeof(int) * 2 * 2);
    }

    state->level4_interior_parts =
        InteriorParts = mem_alloc(sizeof(struct S_interiorp), nspec * 2,
                                  "InteriorParts");

    MPI_Type_vector(nspec, 1, nn, MPI_INT, &T_Hgramhalf);
    MPI_Type_commit(&T_Hgramhalf);
    for (n = 0; n < nnns2; n++)  state->n_of_send[n] = 0;

    {
        struct oh_state* state = oh4s_state();
        int* dst_neighbors = state->dst_neighbors;
        int* src_neighbors = state->src_neighbors;
        int* temp_array = state->temp_array;
        for (z = 0, n = 0; z < 3; z++) {
            int(*bd)[OH_DIMENSION][2] = state->boundaries;
            const int nonpz = z != 1 && bd[me][OH_DIM_Z][z >> 1];
            for (y = 0; y < 3; y++) {
                const int nonpy =
                    nonpz || (y != 1 && bd[me][OH_DIM_Y][y >> 1]);
                for (x = 0; x < 3; x++, n++) {
                    int dnbr = dst_neighbors[n];
                    const int nrev = OH_NEIGHBORS - 1 - n;
                    if (nonpy || (x != 1 && bd[me][OH_DIM_X][x >> 1]))
                        dst_neighbors[n] = src_neighbors[nrev] = -(nn + 1);
                    else if (dnbr < 0 && dnbr >= -nn)
                        dst_neighbors[n] = src_neighbors[nrev] = -(dnbr + 1);
                    else
                        src_neighbors[nrev] = dnbr;
                }
            }
        }
        for (i = 0; i < nn; i++)  temp_array[i] = 0;
        {
            int* first_neighbor = state->level4_first_neighbor;
            int* primary_rl_index = state->level4_primary_rl_index;
            int* sfirst = temp_array + nn;
            for (n = 0; n < OH_NEIGHBORS; n++) {
                const int dnbr = dst_neighbors[n], snbr = src_neighbors[n];
                if (dnbr >= 0) {
                    if (temp_array[dnbr] & 1)  dst_neighbors[n] = -(dnbr + 1);
                    else                    temp_array[dnbr] |= 1;
                }
                if (snbr >= 0) {
                    if (temp_array[snbr] & 2) {
                        src_neighbors[n] = -(snbr + 1);
                        first_neighbor[n] = sfirst[snbr];
                    } else {
                        first_neighbor[n] = sfirst[snbr] = n;
                        temp_array[snbr] |= 2;
                    }
                } else
                    first_neighbor[n] = n;
                primary_rl_index[n] = n;
            }
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
    oh4s_state();
}

void oh4s_particle_buffer_(const int* maxlocalp, struct S_particle* pbuf) {
    void* buffer = pbuf;
    oh4s_particle_buffer(*maxlocalp, &buffer);
}

void oh4s_particle_buffer(const int maxlocalp, void** pbuf) {
    struct oh_state* state = oh4s_state();

    if (nOfLocalPLimitShadow < 0)
        errstop("oh4s_particle_buffer() has to be called after oh4s_init()");
    else if (maxlocalp < nOfLocalPLimitShadow)
        errstop("argument maxlocalp %d given to oh4s_particle_buffer() is less "
                "than that calculated by oh4s_init() %d",
                maxlocalp, nOfLocalPLimitShadow);
    if (*pbuf)
        state->particles = Particles = (struct S_particle*)*pbuf;
    else
        state->particles = Particles =
            (struct S_particle*)mem_alloc(
                oh_particle_buffer_stride(state->particle_adapter),
                maxlocalp << 1, "Particles");
    *pbuf = Particles;
    state->send_buffer = SendBuf =
        oh_particle_buffer_at(state->particle_adapter,
                              state->particles, maxlocalp);
    nOfLocalPLimit = state->n_of_local_particles_limit = maxlocalp;
    totalParts = state->total_parts = maxlocalp;
}

void oh4s_per_grid_histogram_(int* pghgram, int* pgindex) {
    oh4s_per_grid_histogram(&pghgram, &pgindex);
}

void oh4s_per_grid_histogram(int** pghgram, int** pgindex) {
    int* npgo = NULL, * npgi = NULL;
    struct oh_state* state = oh4s_state();
    struct S_griddesc* GridDesc = state->level4_grid_desc;
    const int size = GridDesc[0].dw * GridDesc[0].h;
    Allocate_NOfPGrid(npgo, NOfPGridOut, int, size, "NOfPGridOut");
    Allocate_NOfPGrid(*pghgram, NOfPGridOutShadow, int, size,
                      "NOfPGridOutShadow");
    Allocate_NOfPGrid(npgi, NOfPGridIndex, int, size, "NOfPGridIndex");
    Allocate_NOfPGrid(*pgindex, NOfPGridIndexShadow, int, size,
                      "NOfPGridIndexShadow");
    oh4s_state();
}

int oh4s_transbound_(int* currmode, int* stats) {
    return(transbound4s(*currmode, *stats, 4));
}

int oh4s_transbound(int currmode, int stats) {
    return(transbound4s(currmode, stats, 4));
}

static int transbound4s(int currmode, int stats, const int level) {
    int ret = MODE_NORM_SEC;
    struct oh_state* state;
    int nn, ns, ns2, nnns2;
    struct S_particle* tmp;
    int i, ps, s, tp;
    int (*z_bound)[2];
    Decl_For_All_Grid();

    state = oh4s_state();
    stats = stats && state->stats_mode;
    currmode = transbound1_state(state, currmode, stats, level);
    state = oh4s_state();
    nn = state->n_of_nodes;  ns = state->n_of_species;
    ns2 = ns << 1;  nnns2 = nn * ns2;
    z_bound = (int (*)[2])state->level4_z_bound;

    z_bound[0][OH_LOWER] = z_bound[0][OH_UPPER] = 0;
    z_bound[1][OH_LOWER] = z_bound[1][OH_UPPER] = 0;
    if (try_primary4s(currmode, level, stats))  ret = MODE_NORM_PRI;
    else if (!Mode_PS(currmode) || !try_stable4s(currmode, level, stats)) {
        rebalance4s(currmode, level, stats);  ret = MODE_REB_SEC;
    }
    state = oh4s_state();
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
        const int extio = (ps == 1 && ret < 0) ? OH_PGRID_EXT * 3 : OH_PGRID_EXT;
        for (s = 0; s < ns; s++) {
            dint* npg = state->level4_particle_grid[ps][s];
            For_All_Grid(ps, -extio, -extio, -extio, extio, extio, extio)
                npg[The_Grid()] = 0;
        }
    }
    if (state->level4_z_bound_shadow)
        memcpy(state->level4_z_bound_shadow, z_bound, sizeof(int) * 2 * 2);
    tmp = state->particles;
    Particles = state->particles = state->send_buffer;
    SendBuf = state->send_buffer = tmp;
    currMode = state->curr_mode = ret < 0 ? -ret : ret;
    return(ret);
}

static int try_primary4s(const int currmode, const int level, const int stats) {
    struct oh_state* state = oh4s_state();
    const int oldp = state->region_id[1];

    if (!try_primary1_state(state, currmode, level, stats)) return(FALSE);
    state = oh4s_state();
    exchange_particles4s(currmode, 0, level, 0, oldp, -1, stats);
    if (Mode_PS(currmode))  update_real_neighbors(state, URN_PRI, 0, -1, -1);
    return(TRUE);
}

static int try_stable4s(const int currmode, const int level, const int stats) {
    struct oh_state* state = oh4s_state();

    if (!try_stable1_state(state, currmode,
                           (Mode_Acc(currmode) ? level : -level), stats))
        return(FALSE);
    state = oh4s_state();
    exchange_particles4s(currmode, 1, level, 0, state->region_id[1],
                         state->region_id[1], stats);
    return(TRUE);
}

static void rebalance4s(const int currmode, const int level, const int stats) {
    struct oh_state* state = oh4s_state();
    const int me = state->my_rank, ns = state->n_of_species;
    const int oldp = state->region_id[1], amode = Mode_Acc(currmode);
    const int ninj = state->n_of_injections;
    int s, n, newp;

    rebalance1_state(state, currmode, (amode ? level : -level), stats);
    state = oh4s_state();
    newp = amode ? state->nodes[me].parentid : state->nodes_next[me].parentid;
    if (ninj && amode && oldp != newp) {
        int* sinj = state->injected_particles + ns;
        int i;
        struct S_particle* p;
        for (s = 0; s < ns; s++)  sinj[s] = 0;
        if (newp >= 0) {
            for (i = 0, p = level4_particle_at(state, state->total_parts);
                 i < ninj;
                 i++, p = oh_particle_buffer_at(state->particle_adapter, p, 1)) {
                const OH_nid_t nid = level4_particle_region(state, p, 0);
                int sdid;
                if (level4_secondary_injected(state, nid)) {
                    sdid = level4_primarize_particle(state, p);
                    level4_secondarize_particle(state, p);
                    if (sdid == newp)  sinj[level4_particle_species(state, p)]++;
                }
            }
        }
    }
    exchange_particles4s(currmode, 1, level, 1, oldp, newp, stats);
    if (!amode) {
        state = oh4s_state();
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

static void exchange_particles4s(int currmode, const int nextmode, const int level,
                                 int reb, int oldp, int newp, const int stats) {
    struct oh_state* state = oh4s_state();
    const int ns = state->n_of_species, exti = OH_PGRID_EXT;
    const int trans = !Mode_Acc(currmode) && reb ? 1 : 0;
    int pcode =
        (oldp >= 0 ? 4 : 0) + (newp >= 0 ? 2 : 0) + (oldp == newp ? 1 : 0);
    int ps, psold, psnew, s;
    int nacc[2], nsend, tp;
    struct S_commlist* rlist[2];
    int* rlidx[2];
    Decl_For_All_Grid();

    if (Mode_Acc(currmode)) {
        if (nextmode) {
            int i;
            const int nnns2 = state->n_of_nodes * state->n_of_species * 2;
            if (reb) {
                exchange_particles_state(state, state->sec_recv_list,
                                         *state->sec_rl_size,
                                         oldp, 0, currmode, stats);
                update_descriptors(state, oldp, newp);
                set_grid_descriptor(state, 1, newp);
                state = oh4s_state();
                update_neighbors(state, 1);
                update_real_neighbors(state, URN_SEC, 0, -1, newp);
            } else
                exchange_particles_state(
                    state, state->comm_list + state->sl_head_tail[1],
                    state->sec_sl_head_tail[0], oldp, 0, currmode, stats);
            for (i = 0; i < nnns2; i++)  state->n_of_send[i] = 0;
        } else {
            move_to_sendbuf_primary_state(state, Mode_PS(currmode), stats);
            exchange_primary_particles_state(state, currmode, stats);
        }
        state = oh4s_state();
        count_population(state, nextmode, (Parent_New(pcode) ? 1 : 0), 0);
        currmode = Mode_Set_Any(nextmode);
        reb = 0;  oldp = newp;  pcode = newp >= 0 ? 7 : 0;
    }
    exchange_population(state, currmode);
    psold = Parent_Old(pcode) ? 1 : 0;
    psnew = Parent_New(pcode) ? 1 : 0;
    if (nextmode) {
        make_recv_list(currmode, level, reb, oldp, newp, stats);
        rlist[0] = state->comm_list;  rlist[1] = state->sec_recv_list;
        rlidx[0] = state->rl_index;   rlidx[1] = state->level4_sec_rl_index;
    } else {
        struct S_commlist (*primary_comm_list)[OH_NEIGHBORS] =
            (struct S_commlist (*)[OH_NEIGHBORS])state->level4_primary_comm_list;
        rlist[0] = primary_comm_list[0];  rlist[1] = primary_comm_list[1];
        rlidx[0] = rlidx[1] = state->level4_primary_rl_index;
    }
    make_send_sched(reb, pcode, oldp, newp, rlist, rlidx, nacc, &nsend);
    state_exchange_xfer_amount4s(state, trans, psnew, nextmode);

    for (ps = 0, tp = 0; ps <= psnew; ps++) {
        const int psor2 = ps ? trans + 1 : 0;
        const int sb = state->spec_base;
        for (s = 0; s < ns; s++) {
            dint* npgt = state->level4_particle_grid_total[ps][s];
            int* npgo = state->level4_particle_grid_out[ps][s],
                * npgos = state->level4_particle_grid_out_shadow[ps][s];
            int* npgi = state->level4_particle_grid_index[ps][s],
                * npgis = state->level4_particle_grid_index_shadow[ps][s];
            For_All_Grid(psor2, -exti, -exti, -exti, exti, exti, exti) {
                const int g = The_Grid(), np = npgo[g];
                npgos[g] = np;  npgt[g] = npgi[g] = tp;  npgis[g] = tp + sb;  tp += np;
            }
        }
    }
    if (trans || (dint)nacc[1] + (dint)nsend >
        (dint)state->n_of_local_particles_limit) {
        move_to_sendbuf_4s(nextmode, psold, psnew, trans, oldp, nacc, nsend,
                           stats);
        state_xfer_particles4s(state, trans, psnew, nextmode,
                               state->send_buffer);
        make_bxfer_sched(state, trans, psnew, rlist, rlidx);
        sort_particles(state, nextmode, psnew, stats);
    } else {
        make_bxfer_sched(state, 0, psnew, rlist, rlidx);
        move_and_sort(nextmode, psold, psnew, oldp, nacc, stats);
        state_xfer_particles4s(state, trans, psnew, nextmode,
                               oh_particle_buffer_at(state->particle_adapter,
                                                     state->send_buffer,
                                                     nacc[1]));
        sort_received_particles(state, nextmode, psnew, stats);
    }
    xfer_boundary_particles_v(state, psnew, trans, 0);
    xfer_boundary_particles_v(state, psnew, trans, 1);
    xfer_boundary_particles_h(state, psnew);
}

static void count_population(struct oh_state* state, const int nextmode,
                             const int psnew, const int stats) {
    int ps, s, t, i, j, tp;
    const int ns = state->n_of_species, exti = OH_PGRID_EXT;
    Decl_For_All_Grid();

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
                const int g = level4_grid_position(
                    state, level4_particle_region(state, p, ps));
                npgs[g]++;
                level4_set_particle_region(state, p,
                                           level4_combine_subdomain_position(
                                               state, OH_NBR_SELF, g),
                                           ps);
            }
        }
        if (ps == 0)  primaryParts = state->primary_parts = tp;
    }
    totalParts = state->total_parts = tp;
    nOfInjections = state->n_of_injections = 0;
}

static void exchange_population(struct oh_state* state, const int currmode) {
    const int ns = state->n_of_species;
    int s, zz;
    dint** npg = state->level4_particle_grid_total[0];
    const int ct = state->n_of_exchanges - 1;
    const int ext = OH_PGRID_EXT, ext2 = ext << 1, ext3 = ext * 3;
    const struct S_griddesc* gd = state->level4_grid_desc;
    const int x = gd[0].x, y = gd[0].y, z = gd[0].z;
    const int w = gd[0].w, dw = gd[0].dw;
    Decl_For_All_Grid();

    if (Mode_PS(currmode))  reduce_population(state);
    else {
        for (s = 0; s < ns; s++) {
            dint* npgs = state->level4_particle_grid[0][s], * npgt = npg[s];
            For_All_Grid(0, -ext, -ext, -ext, ext, ext, ext)
                npgt[The_Grid()] = npgs[The_Grid()];
        }
    }
    for (zz = 0; zz < z; zz++)  state->level4_particle_grid_z[zz] = 0;
    for (s = 0; s < ns; s++) {
        dint* npgt = npg[s];
        oh3_exchange_borders(npgt, NULL, ct, 0);
        add_population(state, npgt, -ext3, x + ext3, -ext3, y + ext3,
                       -ext, ext, -dw * ext2);
        add_population(state, npgt, -ext3, x + ext3, -ext3, y + ext3,
                       z - ext, z + ext, dw * ext2);
        add_population(state, npgt, -ext3, x + ext3, -ext, ext,
                       -ext, z + ext, -w * ext2);
        add_population(state, npgt, -ext3, x + ext3, y - ext, y + ext,
                       -ext, z + ext, w * ext2);
        add_population(state, npgt, -ext, ext, -ext, y + ext, -ext,
                       z + ext, -ext2);
        add_population(state, npgt, x - ext, x + ext, -ext, y + ext, -ext,
                       z + ext, ext2);

        For_All_Grid(0, 0, 0, 0, 0, 0, 0)
            state->level4_particle_grid_z[Grid_Z()] += npgt[The_Grid()];
    }
}

static void reduce_population(struct oh_state* state) {
    const int ft = state->n_of_fields - 1;
    const int base = state->field_desc[ft].red.base;
    const int* size = state->field_desc[ft].red.size;
    struct S_mycommc* mycomm = state->my_comm;

    if (mycomm->black) {
        if (mycomm->prime != MPI_COMM_NULL)
            MPI_Reduce(state->level4_particle_grid[0][0] + base,
                       state->level4_particle_grid_total[0][0] + base, size[0],
                       MPI_LONG_LONG_INT, MPI_SUM, mycomm->rank, mycomm->prime);
        if (mycomm->sec != MPI_COMM_NULL)
            MPI_Reduce(state->level4_particle_grid[1][0] + base,
                       state->level4_particle_grid_total[1][0] + base, size[1],
                       MPI_LONG_LONG_INT, MPI_SUM, mycomm->root, mycomm->sec);
    } else {
        if (mycomm->sec != MPI_COMM_NULL)
            MPI_Reduce(state->level4_particle_grid[1][0] + base,
                       state->level4_particle_grid_total[1][0] + base, size[1],
                       MPI_LONG_LONG_INT, MPI_SUM, mycomm->root, mycomm->sec);
        if (mycomm->prime != MPI_COMM_NULL)
            MPI_Reduce(state->level4_particle_grid[0][0] + base,
                       state->level4_particle_grid_total[0][0] + base, size[0],
                       MPI_LONG_LONG_INT, MPI_SUM, mycomm->rank, mycomm->prime);
    }
    if (mycomm->prime == MPI_COMM_NULL)
        memcpy(state->level4_particle_grid_total[0][0] + base,
               state->level4_particle_grid[0][0] + base,
               size[0] * sizeof(dint));
}

static void add_population(struct oh_state* state, dint* npd, const int xl,
                           const int xu, const int yl, const int yu,
                           const int zl, const int zu, const int src) {
    dint* nps = npd + src;
    Decl_For_All_Grid();

    For_All_Grid_Abs(0, xl, yl, zl, xu, yu, zu)
        npd[The_Grid()] += nps[The_Grid()];
}

static void make_recv_list(const int currmode, const int level, const int reb,
                           const int oldp, const int newp, const int stats) {
    struct oh_state* state = oh4s_state();
    const int me = state->my_rank, ns = state->n_of_species;
    const int nn = state->n_of_nodes, nnns = nn * ns;
    const int nn2 = nn << 1;
    struct S_node* nodes = reb ? state->nodes_next : state->nodes;
    struct S_node* mynode = nodes + me;
    struct S_node* ch;
    int* first_neighbor = state->level4_first_neighbor;
    int* rl_index = state->rl_index;
    int* sec_rl_index = state->level4_sec_rl_index;
    int* alt_sec_rl_index = state->level4_alt_sec_rl_index;
    MPI_Comm comm = state->comm;
    struct S_recvsched_context
        context = { 0, 0, 0, state->comm_list };
    int rlsize, rlidx;
    const int ft = state->n_of_fields - 1;
    const int npgbase = state->field_desc[ft].bc.base;
    const int* npgsize = state->field_desc[ft].bc.size;
    const int zmax = state->level4_grid_desc[0].z - 1;
    struct S_commlist* lastrl;
    int i;

    for (ch = mynode->child; ch; ch = ch->sibling)
        sched_recv(state, reb, ch->get.sec, ch->stay.sec, ch->id, nnns,
                   &context);
    sched_recv(state, 0, mynode->get.prime, mynode->stay.prime, me, 0,
               &context);

    rlidx = rlsize = context.cptr - state->comm_list;  lastrl = context.cptr - 1;
    if (rlsize == 0) {
        struct S_commlist* rl = state->comm_list;
        rl->rid = me;  rl->tag = 0;  rl->sid = 0;  rl->count = 0;
        rl->region = zmax;
        rlidx = rlsize = 1;
    } else
        lastrl->region = zmax;

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
    rl_index[OH_NEIGHBORS] = rlidx;  sec_rl_index[OH_NEIGHBORS] = 0;
    AltSecRList = state->level4_alt_sec_recv_list =
        state->sec_recv_list = SecRList = state->comm_list + rlidx;
    alt_sec_rl_index[OH_NEIGHBORS] = 0;
    if (Mode_PS(currmode)) {
        oh1_broadcast_state(state, rl_index, sec_rl_index,
                            OH_NEIGHBORS + 1, OH_NEIGHBORS + 1,
                            MPI_INT, MPI_INT);
        oh1_broadcast_state(state, state->comm_list, state->sec_recv_list,
                            rlidx, sec_rl_index[OH_NEIGHBORS],
                            T_Commlist, T_Commlist);
        AltSecRList = state->level4_alt_sec_recv_list =
            state->level4_alt_sec_recv_list + sec_rl_index[OH_NEIGHBORS];
    }
    if (reb) {
        build_new_comm_state(state, currmode, -level, 2, stats);
        update_descriptors(state, oldp, newp);
        set_grid_descriptor(state, 2, newp);
        update_real_neighbors(state, URN_TRN, Mode_PS(currmode), oldp, newp);
        oh1_broadcast_state(state, rl_index, alt_sec_rl_index,
                            OH_NEIGHBORS + 1, OH_NEIGHBORS + 1,
                            MPI_INT, MPI_INT);
        oh1_broadcast_state(state, state->comm_list,
                            state->level4_alt_sec_recv_list,
                            rl_index[OH_NEIGHBORS],
                            alt_sec_rl_index[OH_NEIGHBORS],
                            T_Commlist, T_Commlist);
    }
    oh1_broadcast_state(state,
                        state->level4_particle_grid_total[0][0] + npgbase,
                        state->level4_particle_grid_total[1][0] + npgbase,
                        npgsize[0], npgsize[1],
                        MPI_LONG_LONG_INT, MPI_LONG_LONG_INT);
}

static void sched_recv(struct oh_state* state, const int reb, const int get,
                       const int stay, const int nid, const int tag,
                       struct S_recvsched_context* context) {
    const int z0 = context->z;
    dint nptotal = context->nptotal;
    dint nplimit = context->nplimit;
    struct S_commlist* cptr = context->cptr;
    const int ns = state->n_of_species;
    dint* npgz = state->level4_particle_grid_z;
    int z;
    const int zz = state->level4_grid_desc[0].z;

    if (reb)
        nplimit += get;
    else
        nplimit += get + stay;

    context->nplimit = nplimit;
    if (nptotal >= nplimit)  return;
    cptr->rid = nid;  cptr->tag = tag;  cptr->sid = 0;  cptr->count = 0;
    for (z = z0; z < zz; z++) {
        nptotal += npgz[z];
        if (nptotal >= nplimit) {
            cptr->region = z;  context->z = z + 1;
            context->nptotal = nptotal;  context->cptr = cptr + 1;
            return;
        }
    }
    local_errstop("per-plane histogram total %d is less than the total particle "
                  "population %d up to node %d",
                  nptotal, nplimit, nid);
}

static void make_send_sched(const int reb, const int pcode, const int oldp,
                            const int newp, struct S_commlist* rlist[2],
                            int* rlidx[2], int* nacc, int* nsendptr) {
    struct oh_state* state = oh4s_state();
    const int psold = Parent_Old(pcode) ? 1 : 0;
    const int psnew = Parent_New(pcode) ? 1 : 0;
    const int ns = state->n_of_species, ns2 = ns << 1, nn = state->n_of_nodes;
    const int tagt = OH_NBR_TCC * ns, tagb = OH_NBR_BCC * ns;
    const int tag1 = OH_NEIGHBORS * ns;
    int s, ps, n;
    int nsend = 0;
    int (*z_bound)[2] = (int (*)[2])state->level4_z_bound;
    int (*subdomains)[OH_DIMENSION][2] = state->subdomains;
    struct S_hplane (*hplanes)[2] =
        (struct S_hplane (*)[2])state->level4_horizontal_planes;
    int* alt_sec_rl_index = state->level4_alt_sec_rl_index;

    for (s = 0; s < ns2; s++)  state->total_particles_next[s] = 0;
    for (ps = 0; ps <= psold; ps++) {
        const int root = ps ? oldp : state->my_rank;
        for (n = 0; n < OH_NEIGHBORS; n++) {
            const int nrev = OH_NEIGHBORS - 1 - n;
            int sdid = state->neighbors[ps][n];
            if (sdid < 0)  sdid = -(sdid + 1);
            if (sdid < nn && (n == OH_NBR_SELF || sdid != root))
                nsend += make_send_sched_body(state, ps, n, sdid,
                                              rlist[ps] + rlidx[ps][nrev]);
        }
    }
    nacc[0] = nacc[1] = 0;
    for (ps = 0; ps <= psnew; ps++) {
        int psor2;
        int* nbors, * ri;
        struct S_commlist* rl;
        struct S_hplane* hp = hplanes[ps];
        if (ps && Parent_New_Diff(pcode)) {
            psor2 = 2;  nbors = state->neighbors[2];
            rl = state->level4_alt_sec_recv_list;  ri = alt_sec_rl_index;
        } else {
            psor2 = ps;  nbors = state->neighbors[ps];
            rl = rlist[ps];  ri = rlidx[ps];
        }
        make_send_sched_self(state, psor2, rl + ri[OH_NBR_SELF], nacc + ps);
        if (z_bound[ps][OH_UPPER] == 0)  continue;
        if (hp[OH_LOWER].nbor == nn) {
            int sdid = nbors[OH_NBR_BCC];
            if (sdid < 0)  sdid = -(sdid + 1);
            if (sdid < nn) {
                const int zmax = (subdomains[sdid][OH_DIM_Z][OH_UPPER] -
                                  subdomains[sdid][OH_DIM_Z][OH_LOWER]) - 1;
                struct S_commlist* rlb = rl + ri[OH_NEIGHBORS - 1 - OH_NBR_BCC];
                int rlz = rlb->region;
                while (rlz < zmax)  rlz = (++rlb)->region;
                hp[OH_LOWER].nbor = rlb->rid;
                hp[OH_LOWER].stag = (rlb->tag) ? tagb + tag1 : tagb;
            } else {
                hp[OH_LOWER].nbor = MPI_PROC_NULL;
                hp[OH_LOWER].stag = tagb;
            }
        }
        if (hp[OH_UPPER].nbor == nn) {
            int sdid = nbors[OH_NBR_TCC];
            struct S_commlist* rlt = rl + ri[OH_NEIGHBORS - 1 - OH_NBR_TCC];
            if (sdid < 0)  sdid = -(sdid + 1);
            if (sdid < nn) {
                hp[OH_UPPER].nbor = rlt->rid;
                hp[OH_UPPER].stag = (rlt->tag) ? tagt + tag1 : tagt;
            } else {
                hp[OH_UPPER].nbor = MPI_PROC_NULL;
                hp[OH_UPPER].stag = tagt;
            }
        }
        if (!ps)  nacc[1] = nacc[0];
    }
    *nsendptr = nsend;
}

#define For_All_Grid_Z(PS, X0, Y0, Z0, X1, Y1, Z1)\
  For_Z((fag_zidx=(Z0), fag_x1=Level4_Grid_Desc(PS).x+(X1),\
         fag_y1=Level4_Grid_Desc(PS).y+(Y1),\
         fag_z1=Level4_Grid_Desc(PS).z+(Z1),\
         fag_w=Level4_Grid_Desc(PS).w, fag_dw=Level4_Grid_Desc(PS).dw,\
         fag_gz=Coord_To_Index(X0,Y0,Z0,fag_w,fag_dw)),\
        (fag_zidx<fag_z1), (fag_zidx++,fag_gz+=fag_dw))

#define For_All_Grid_XY(PS, X0, Y0, X1, Y1)\
  For_Y((fag_yidx=(Y0), fag_gy=fag_gz),\
        (fag_yidx<fag_y1), (fag_yidx++,fag_gy+=fag_w))\
    for (fag_xidx=(X0),fag_gx=fag_gy; fag_xidx<fag_x1; fag_xidx++,fag_gx++)

#define Grid_Exterior_Boundary(N, GS, PL, PU) {\
  const int e = OH_PGRID_EXT;\
  if (N==0)      { PL = -e;    PU = -(GS); }\
  else if (N==1) { PL = 0;     PU = 0; }\
  else           { PL = (GS);  PU = e; }\
}

#define Grid_Interior_Boundary(N, GS, PL, PU) {\
  const int e = OH_PGRID_EXT;\
  if (N==0)      { PL = 0;       PU = -(GS)+e; }\
  else if (N==1) { PL = 0;       PU = 0; }\
  else           { PL = (GS)-e;  PU = 0; }\
}

#define Make_Send_Sched_Body(MYSELF) {\
  int s, nofsidx=nofsbase;\
  for (s=0; s<ns; s++,nofsidx+=nn) {\
    dint *npg = state->level4_particle_grid[ps][s];\
    int nsendofs=0;\
    For_All_Grid_XY(ps, xl, yl, xu, yu) {\
      const int g = The_Grid();\
      if (MYSELF)  npg[g] = 0;\
      else {\
        nsendofs += npg[g];  npg[g] = nofsidx + 1;\
      }\
    }\
    nsend += nsendofs;  state->n_of_send[nofsidx] += nsendofs;\
  }\
}

static int make_send_sched_body(struct oh_state* state, const int ps,
                                const int n, const int sdid,
                                struct S_commlist* rlist) {
    const int me = state->my_rank, ns = state->n_of_species;
    const int nn = state->n_of_nodes;
    const int nx = n % 3, ny = n / 3 % 3, nz = n / 9;
    struct S_griddesc* GridDesc = state->level4_grid_desc;
    int xl, xu, yl, yu, zl, zu, zn;
    int rlz = rlist->region, rid, ridp = -1, ridn = -1, nofsbase;
    int nsend = 0;
    const int zmax = (state->subdomains[sdid][OH_DIM_Z][OH_UPPER] -
                      state->subdomains[sdid][OH_DIM_Z][OH_LOWER]) - 1;
    Decl_For_All_Grid();

    Grid_Exterior_Boundary(nx, GridDesc[ps].x, xl, xu);
    Grid_Exterior_Boundary(ny, GridDesc[ps].y, yl, yu);
    Grid_Exterior_Boundary(nz, GridDesc[ps].z, zl, zu);
    zn = (nz == 0) ? zmax + 1 - OH_PGRID_EXT : 0;
    while (rlz < zn)  rlz = (++rlist)->region;
    rid = rlist->rid;  nofsbase = rlist->tag + rid;

    For_All_Grid_Z(ps, xl, yl, zl, xu, yu, zu) {
        if (n == OH_NBR_SELF && rid == me) {
            Make_Send_Sched_Body(1);
        } else {
            Make_Send_Sched_Body(0);
        }
        if (++zn > rlz && zn <= zmax) {
            rlz = (++rlist)->region;  rid = rlist->rid;  nofsbase = rlist->tag + rid;
        }
    }
    return(nsend);
}

static void make_send_sched_self(struct oh_state* state, const int psor2,
                                 struct S_commlist* rlist, int* naccptr) {
    const int me = state->my_rank, nn = state->n_of_nodes;
    const int ns = state->n_of_species;
    const int tag1 = OH_NEIGHBORS * ns;
    const int tagt = OH_NBR_TCC * ns, tagb = OH_NBR_BCC * ns;
    const int ps = psor2 == 0 ? 0 : 1, rtag = ps ? tag1 : 0;
    const int exti = OH_PGRID_EXT;
    struct S_griddesc* GridDesc = state->level4_grid_desc;
    int (*z_bound)[2] = (int (*)[2])state->level4_z_bound;
    const int zmax = GridDesc[psor2].z - 1;
    int rlz = -1, rid = nn, ridp = -1, ridn, stag = 0;
    struct S_hplane (*hplanes)[2] =
        (struct S_hplane (*)[2])state->level4_horizontal_planes;
    struct S_hplane* hp = hplanes[ps];
    int* zb = z_bound[ps];
    int np = *naccptr, * tpn = state->total_particles_next + (ps ? ns : 0);
    int s;
    Decl_For_All_Grid();

    hp[OH_LOWER].nbor = hp[OH_UPPER].nbor = MPI_PROC_NULL;
    For_All_Grid_Z(psor2, -exti, -exti, -exti, exti, exti, exti) {
        const int z = Grid_Z();
        ridn = (z == rlz) ? (z < zmax ? rlist->rid : nn) : -1;
        if (ridp == me) {
            zb[OH_UPPER] = z;  hp[OH_UPPER].nbor = rid;
            hp[OH_UPPER].stag = stag + tagt;
            hp[OH_UPPER].rtag = rtag + tagb;
        } else if (ridn == me) {
            zb[OH_LOWER] = z + 1;  hp[OH_LOWER].nbor = rid;
            hp[OH_LOWER].stag = stag + tagb;
            hp[OH_LOWER].rtag = rtag + tagt;
        }
        if (rid == me) {
            if (ridp >= 0) {
                make_send_sched_hplane(state, psor2, z, naccptr,
                                       hp[OH_LOWER].nsend, hp[OH_LOWER].sbuf);
                if (ridn >= 0) {
                    for (s = 0; s < ns; s++) {
                        hp[OH_UPPER].nsend[s] = hp[OH_LOWER].nsend[s];
                        hp[OH_UPPER].sbuf[s] = hp[OH_LOWER].sbuf[s];
                    }
                }
            } else if (ridn >= 0)
                make_send_sched_hplane(state, psor2, z, naccptr,
                                       hp[OH_UPPER].nsend, hp[OH_UPPER].sbuf);
            else
                make_send_sched_hplane(state, psor2, z, naccptr, NULL, NULL);
        } else {
            if (ridp == me)
                make_send_sched_hplane(state, psor2, z, naccptr,
                                       hp[OH_UPPER].nrecv, hp[OH_UPPER].rbuf);
            else if (ridn == me)
                make_send_sched_hplane(state, psor2, z, naccptr,
                                       hp[OH_LOWER].nrecv, hp[OH_LOWER].rbuf);
            else {
                for (s = 0; s < ns; s++) {
                    int* npgo = state->level4_particle_grid_out[ps][s];
                    For_All_Grid_XY(psor2, -exti, -exti, exti, exti)
                        npgo[The_Grid()] = 0;
                }
            }
        }
        ridp = -1;
        if (z == rlz) {
            ridp = rid;
            if (z < zmax) {
                rlz = rlist->region;  rid = rlist->rid;
                stag = rlist->tag ? tag1 : 0;  rlist++;
            } else {
                rlz++;  rid = nn;  stag = 0;
            }
        }
    }
    for (s = 0; s < ns; s++) {
        hp[OH_LOWER].sbuf[s] += np;  hp[OH_LOWER].rbuf[s] += np;
        hp[OH_UPPER].sbuf[s] += np;  hp[OH_UPPER].rbuf[s] += np;
        np += tpn[s];
    }
}

#define For_All_Grid_XY_At_Z(PS, X0, Y0, X1, Y1, Z0)\
  For_Z((fag_zidx=(Z0), fag_x1=Level4_Grid_Desc(PS).x+(X1),\
         fag_y1=Level4_Grid_Desc(PS).y+(Y1), fag_z1=(Z0)+1,\
         fag_w=Level4_Grid_Desc(PS).w, fag_dw=Level4_Grid_Desc(PS).dw,\
         fag_gz=Coord_To_Index(X0,Y0,Z0,fag_w,fag_dw)),\
        (fag_zidx<fag_z1), (fag_zidx++,fag_gz+=fag_dw))\
    For_Y((fag_yidx=(Y0), fag_gy=fag_gz),\
          (fag_yidx<fag_y1), (fag_yidx++,fag_gy+=fag_w))\
      for (fag_xidx=(X0),fag_gx=fag_gy; fag_xidx<fag_x1; fag_xidx++,fag_gx++)

static void make_send_sched_hplane(struct oh_state* state, const int psor2,
                                   const int z, int* naccptr, int* np,
                                   int* buf) {
    const int ns = state->n_of_species, exti = OH_PGRID_EXT;
    const int ps = psor2 == 0 ? 0 : 1, nsor0 = ps ? ns : 0;
    struct S_griddesc* GridDesc = state->level4_grid_desc;
    int nacc = *naccptr, s;
    Decl_For_All_Grid();

    for (s = 0; s < ns; s++) {
        dint* npgt = state->level4_particle_grid_total[ps][s];
        int* npgo = state->level4_particle_grid_out[ps][s];
        int npofs = 0;
        if (buf)  buf[s] = state->total_particles_next[nsor0 + s];
        For_All_Grid_XY_At_Z(psor2, -exti, -exti, exti, exti, z) {
            const int g = The_Grid();
            npofs += (npgo[g] = npgt[g]);
        }
        nacc += npofs;  state->total_particles_next[nsor0 + s] += npofs;
        if (np)  np[s] = npofs;
    }
    *naccptr = nacc;
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
  (N==0 ? 0 : (N<0 ? state->subdomains[SD][D][OH_LOWER] - \
                         state->subdomains[SD][D][OH_UPPER] :\
                     GridDesc[ps].XYZ))

static void update_neighbors(struct oh_state* state, const int ps) {
    int n, nx, ny, nz;
    const int nn = state->n_of_nodes;
    int (*abs_neighbors)[OH_NEIGHBORS] = state->abs_neighbors;
    int* grid_offset = state->level4_grid_offset;
    struct S_griddesc* GridDesc = state->level4_grid_desc;
    struct S_commlist (*primary_comm_list)[OH_NEIGHBORS] =
        (struct S_commlist (*)[OH_NEIGHBORS])state->level4_primary_comm_list;
    struct S_commlist* cl = primary_comm_list[ps];

    for (nz = -1, n = 0; nz < 2; nz++) {
        for (ny = -1; ny < 2; ny++) {
            for (nx = -1; nx < 2; nx++, n++) {
                int nbr = state->neighbors[ps][n];
                const int nrev = OH_NEIGHBORS - 1 - n;
                nbr = abs_neighbors[ps][n] = nbr < 0 ? -(nbr + 1) : nbr;
                cl[nrev].rid = nbr;  cl[nrev].tag = cl[nrev].sid = cl[nrev].count = 0;
                if (nbr >= nn) {
                    grid_offset[ps * OH_NEIGHBORS + n] = 0;  cl[nrev].region = 0;
                } else {
                    grid_offset[ps * OH_NEIGHBORS + n] =
                        Coord_To_Index(Neighbor_Grid_Offset(ps, nx, nbr, OH_DIM_X, x),
                                       Neighbor_Grid_Offset(ps, ny, nbr, OH_DIM_Y, y),
                                       Neighbor_Grid_Offset(ps, nz, nbr, OH_DIM_Z, z),
                                       GridDesc[0].w, GridDesc[0].dw);
                    cl[nrev].region =
                        state->subdomains[nbr][OH_DIM_Z][OH_UPPER] -
                        state->subdomains[nbr][OH_DIM_Z][OH_LOWER] - 1;
                }
            }
        }
    }
}

static void set_grid_descriptor(struct oh_state* state, const int idx,
                                const int nid) {
    const int exti6 = OH_PGRID_EXT * 6;
    struct S_griddesc* GridDesc = state->level4_grid_desc;
    struct S_grid* grid = state->grid;
    int (*subdomains)[OH_DIMENSION][2] = state->subdomains;
    const int w = GridDesc[idx].w = grid[OH_DIM_X].size + (exti6);
    const int d = GridDesc[idx].d =
        If_Dim(OH_DIM_Y, grid[OH_DIM_Y].size + (exti6), 1);

    GridDesc[idx].h = If_Dim(OH_DIM_Z, grid[OH_DIM_Z].size + (exti6), 1);
    GridDesc[idx].dw = d * w;
    if (nid >= 0) {
        GridDesc[idx].x = subdomains[nid][OH_DIM_X][OH_UPPER] -
            subdomains[nid][OH_DIM_X][OH_LOWER];
        GridDesc[idx].y = If_Dim(OH_DIM_Y,
                                 subdomains[nid][OH_DIM_Y][OH_UPPER] -
                                 subdomains[nid][OH_DIM_Y][OH_LOWER], 0);
        GridDesc[idx].z = If_Dim(OH_DIM_Z,
                                 subdomains[nid][OH_DIM_Z][OH_UPPER] -
                                 subdomains[nid][OH_DIM_Z][OH_LOWER], 0);
    } else {
        GridDesc[idx].x = GridDesc[idx].y = GridDesc[idx].z = -exti6;
        /* to ensure, e.g., x+3*(OH_PGRID_EXT)<=-3*(OH_PGRID_EXT) */
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

static void exchange_xfer_amount(const int trans, const int psnew, const int nextmode) {
    state_exchange_xfer_amount4s(oh4s_state(), trans, psnew, nextmode);
}

static void state_exchange_xfer_amount4s(struct oh_state* state,
                                         const int trans, const int psnew,
                                         const int nextmode) {
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
    for (ps = 0, tag = 0; ps <= nextmode; ps++, tag += nnns) {
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

static void make_bxfer_sched(struct oh_state* state, const int trans,
                             const int psnew, struct S_commlist* rlist[2],
                             int* rlidx[2]) {
    const int nn = state->n_of_nodes;
    int d, ps, du;
    int(*vph)[2][2] = (int(*)[2][2])state->level4_vertical_plane_head;
    int nsendib = 0, nrecveb = 0, vpidx = 0;
    int (*z_bound)[2] = (int (*)[2])state->level4_z_bound;

    for (d = OH_DIM_X; d <= OH_DIM_Y; d++) {
        for (ps = 0; ps < 2; ps++) {
            const int psor2 = ps ? trans + 1 : 0;
            struct S_commlist* rl;
            int* ri;
            if (ps > psnew || z_bound[ps][OH_UPPER] == 0) {
                vph[d][ps][0] = vph[d][ps][1] = vpidx;
                continue;
            }
            if (psor2 == 2) {
                rl = state->level4_alt_sec_recv_list;
                ri = state->level4_alt_sec_rl_index;
            } else {
                rl = rlist[ps];  ri = rlidx[ps];
            }
            for (du = OH_LOWER; du <= OH_UPPER; du++) {
                const int vpisave = vpidx;
                const int nx = (d == OH_DIM_X) ? du << 1 : 1;
                const int ny = (d == OH_DIM_X) ? 1 : du << 1;
                const int n = 3 * 3 + 3 * ny + nx;
                const int nrev = OH_NEIGHBORS - 1 - n;
                int nbor = state->neighbors[psor2][n];
                vph[d][ps][du] = vpidx;
                if (nbor < 0)  nbor = -(nbor + 1);
                if (nbor < nn) {
                    make_bsend_sched(state, psor2, n, nx, ny, rl + ri[nrev],
                                     &nsendib, &vpidx);
                    make_brecv_sched(state, psor2, n, nx, ny, rl + ri[nrev],
                                     &nrecveb, vpisave);
                }
            }
        }
    }
    vph[2][0][0] = vpidx;
}

#define Add_Pillar_Voxel(I)     (((dint)(I))<<32)
#define Is_Pillar_Voxel(V)      (V>=((dint)1)<<32)
#define Pillar_Lower(V)         (V&INT_MAX)
#define Pillar_Upper(V)         ((V)>>32)

static void make_bsend_sched(struct oh_state* state, const int psor2,
                             const int n, const int nx, const int ny,
                             struct S_commlist* rlist, int* nsendptr,
                             int* vpptr) {
    const int ns = state->n_of_species;
    const int ps = psor2 == 0 ? 0 : 1;
    const int tag1 = OH_NEIGHBORS;
    const int stag = n;
    const int rtag = ((ps ? OH_NEIGHBORS : 0) + (OH_NEIGHBORS - 1 - n));
    struct S_commlist* rl = rlist;
    int rlz = rl->region;
    int nsend = *nsendptr, nsendsave = nsend, vpidx = *vpptr;
    struct S_griddesc* GridDesc = state->level4_grid_desc;
    struct S_vplane* vplanes = state->level4_vertical_planes;
    int (*z_bound)[2] = (int (*)[2])state->level4_z_bound;
    int xl, xu, yl, yu;
    const int zbl = z_bound[ps][OH_LOWER];
    const int zbu = z_bound[ps][OH_UPPER] - GridDesc[psor2].z;
    const int zmax = z_bound[ps][OH_UPPER] - 1;
    const int xtop = GridDesc[psor2].x;
    int s;
    Decl_For_All_Grid();

    if (ny == 1) {
        Grid_Interior_Boundary(nx, GridDesc[psor2].x, xl, xu);
    } else {
        xl = -OH_PGRID_EXT;  xu = OH_PGRID_EXT;
    }
    Grid_Interior_Boundary(ny, GridDesc[psor2].y, yl, yu);

    while (rlz < zbl)  rlz = (++rl)->region;
    vplanes[vpidx].nbor = rl->rid;
    vplanes[vpidx].stag = (rl->tag ? tag1 : 0) + stag;
    vplanes[vpidx].rtag = rtag;
    vplanes[vpidx].sbuf = nsend;
    For_All_Grid_Z(psor2, xl, yl, zbl, xu, yu, zbu) {
        const int z = Grid_Z();
        for (s = 0; s < ns; s++) {
            dint* npg = state->level4_particle_grid[ps][s];
            int* npgo = state->level4_particle_grid_out[ps][s];
            For_All_Grid_XY(psor2, xl, yl, xu, yu) {
                const int g = The_Grid();
                const dint dst = npg[g];
                if (Grid_X() < 0 || Grid_X() >= xtop)
                    npg[g] += Add_Pillar_Voxel(nsend + 1);
                else if (dst >= 0)
                    npg[g] = -(nsend + 1);
                else
                    npg[g] -= Add_Pillar_Voxel(nsend + 1);
                nsend += npgo[g];
            }
        }
        if (z == rlz && z < zmax) {
            vplanes[vpidx++].nsend = nsend - nsendsave;
            rlz = (++rl)->region;
            vplanes[vpidx].nbor = rl->rid;
            vplanes[vpidx].stag = (rl->tag ? tag1 : 0) + stag;
            vplanes[vpidx].rtag = rtag;
            vplanes[vpidx].sbuf = nsendsave = nsend;
        }
    }
    vplanes[vpidx].nsend = nsend - nsendsave;
    *nsendptr = nsend;  *vpptr = vpidx + 1;
}

static void make_brecv_sched(struct oh_state* state, const int psor2,
                             const int n, const int nx, const int ny,
                             struct S_commlist* rlist, int* nrecvptr,
                             int vpidx) {
    const int ns = state->n_of_species;
    const int ps = psor2 == 0 ? 0 : 1;
    int nrecv = *nrecvptr, nrecvsave = nrecv;
    struct S_commlist* rl = rlist;
    int rlz = rl->region;
    struct S_griddesc* GridDesc = state->level4_grid_desc;
    struct S_vplane* vplanes = state->level4_vertical_planes;
    int (*z_bound)[2] = (int (*)[2])state->level4_z_bound;
    int xl, xu, yl, yu;
    const int zbl = z_bound[ps][OH_LOWER];
    const int zbu = z_bound[ps][OH_UPPER] - GridDesc[psor2].z;
    const int zmax = z_bound[ps][OH_UPPER] - 1;
    int s;
    Decl_For_All_Grid();

    if (ny == 1) {
        Grid_Exterior_Boundary(nx, GridDesc[psor2].x, xl, xu);
    } else {
        xl = -OH_PGRID_EXT;  xu = OH_PGRID_EXT;
    }
    Grid_Exterior_Boundary(ny, GridDesc[psor2].y, yl, yu);

    while (rlz < zbl)  rlz = (++rl)->region;
    vplanes[vpidx].rbuf = nrecv;
    For_All_Grid_Z(psor2, xl, yl, zbl, xu, yu, zbu) {
        const int z = Grid_Z();
        for (s = 0; s < ns; s++) {
            int* npgo = state->level4_particle_grid_out[ps][s];
            For_All_Grid_XY(psor2, xl, yl, zu, yu)
                nrecv += npgo[The_Grid()];
        }
        if (z == rlz && z < zmax) {
            vplanes[vpidx++].nrecv = nrecv - nrecvsave;
            rlz = (++rl)->region;
            vplanes[vpidx].rbuf = nrecvsave = nrecv;
        }
    }
    vplanes[vpidx].nrecv = nrecv - nrecvsave;  *nrecvptr = nrecv;
}

#define Move_Or_Do(P, PS, MYSD, TOSB, ACT, PIL) {\
  const OH_nid_t nid = level4_particle_region(state, P, PS);\
  int g = level4_grid_position(state, nid);\
  int sdid;\
  dint dst;\
  if (nid<0)  continue;\
  sdid = level4_neighbor_subdomain_id(state, nid, PS);\
  if (sdid!=(MYSD)) g = level4_local_grid_position(state, g, nid, PS);\
  dst = npg[g];\
  if (dst==0)  { ACT; }\
  else if (!PIL) {\
    if (TOSB) {\
      level4_copy_particle_to_buffer(state, sb, state->n_of_send[dst-1]++, P);\
    }\
  }\
  else if (dst>0)\
    level4_copy_particle_to_buffer(\
        state, sb, state->n_of_send[Pillar_Lower(dst)-1]++, P);\
  else {\
    ACT;\
    const dint bsbidx = -dst;\
    if (!Is_Pillar_Voxel(bsbidx)) {\
      level4_copy_particle_to_buffer(state, state->level4_boundary_send_buffer,\
                                     bsbidx - 1, P);\
      npg[g] = dst - 1;\
    }\
    else {\
      level4_copy_particle_to_buffer(state, state->level4_boundary_send_buffer,\
                                     Pillar_Lower(bsbidx) - 1, P);\
      level4_copy_particle_to_buffer(state, state->level4_boundary_send_buffer,\
                                     Pillar_Upper(bsbidx) - 1, P);\
      npg[g] = dst - (Add_Pillar_Voxel(1) + 1);\
    }\
  }\
}

static void move_to_sendbuf_4s(const int nextmode, const int psold, const int psnew,
                               const int trans, const int oldp, const int* nacc,
                               const int nsend, const int stats) {
    struct oh_state* state = oh4s_state();
    const int me = state->my_rank, ns = state->n_of_species;
    const int nn = state->n_of_nodes;
    const int ninj = state->n_of_injections;
    const int nplim = state->n_of_local_particles_limit;
    struct S_realneighbor (*real_src)[2] =
        (struct S_realneighbor (*)[2])state->level4_real_src_neighbors;
    int ps, s, t, i;
    int* nofr;
    int ninjp = 0, ninjs = nplim;
    struct S_particle* sb = state->send_buffer, * p;
    struct S_interiorp* interior_parts = state->level4_interior_parts;

    if (stats) oh1_stats_time(STATS_TB_MOVE, nextmode);
    state_set_sendbuf_disps4s(state, nextmode, trans);

    for (ps = 0, t = 0, nofr = state->n_of_recv; ps < 2; ps++) {
        const int nnbr = real_src[trans][ps].n;
        const int* rnbr = real_src[trans][ps].nbor;
        if (ps <= psnew) {
            for (s = 0; s < ns; s++, t++, nofr += nn) {
                int n, nrec;
                for (n = 0, nrec = 0; n < nnbr; n++)  nrec += nofr[rnbr[n]];
                interior_parts[t].size = nrec;
            }
        } else
            for (s = 0; s < ns; s++, t++)  interior_parts[t].size = 0;
    }
    for (i = 0, p = level4_particle_at(state, state->total_parts);
         i < ninj;
         i++, p = oh_particle_buffer_at(state->particle_adapter, p, 1)) {
        const int s = level4_particle_species(state, p);
        const OH_nid_t nid = level4_particle_region(state, p, 0);
        const int ps = level4_secondary_injected(state, nid) ? 1 : 0;
        dint* npg = state->level4_particle_grid[ps][s];
        if (nid < 0) continue;
        if (ps) {
            level4_primarize_particle_only(state, p);
            Move_Or_Do(p, ps, oldp, 1,
                       (level4_copy_particle_to_buffer(state, sb, --ninjs, p),
                        interior_parts[ns + s].size++), 0);
        } else
            Move_Or_Do(p, ps, me, 1,
                       (level4_copy_particle_to_buffer(state, sb,
                                                       nsend + ninjp++, p),
                        interior_parts[s].size++), 0);
    }
    move_to_sendbuf_uw4s(state, 0, me, 0, 0);
    if (psold) {
        move_to_sendbuf_uw4s(state, 1, oldp, state->primary_parts, nacc[0]);
        move_to_sendbuf_dw4s(state, 1, oldp, state->total_parts, nacc[1]);
    } else {
        int rbb_index = nacc[0];
        int s;
        for (s = 0; s < ns; s++) {
            state->recv_buffer_bases[ns + s] =
                level4_particle_at(state, rbb_index);
            interior_parts[ns + s].head = rbb_index;
            rbb_index += state->total_particles_next[ns + s];
        }
    }
    move_to_sendbuf_dw4s(state, 0, me, state->primary_parts, nacc[0]);

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

static void move_to_sendbuf_uw4s(struct oh_state* state, const int ps,
                                 const int mysd, const int cbase,
                                 const int nbase) {
    const int ns = state->n_of_species;
    const int nsor0 = ps ? ns : 0;
    const int* ctp = state->total_particles + nsor0;
    const int* ntp = state->total_particles_next + nsor0;
    struct S_interiorp* ip = state->level4_interior_parts + nsor0;
    struct S_particle* p;
    void** rbb = state->recv_buffer_bases + nsor0;
    struct S_particle* sb = state->send_buffer;
    int s, c, d, cn, dn;

    for (s = 0, c = cbase, d = nbase; s < ns; s++, c = cn, d = dn) {
        dint* npg = state->level4_particle_grid[ps][s];
        cn = c + ctp[s];  dn = d + ntp[s];
        ip[s].head = d;
        if (d <= c) {
            for (; c < cn; c++) {
                p = level4_particle_at(state, c);
                Move_Or_Do(p, ps, mysd, 1,
                           level4_copy_particle_to_buffer(
                               state, state->particles, d++, p), 0);
            }
            rbb[s] = level4_particle_at(state, d);
            ip[s].size += d - ip[s].head;
        } else if (dn <= cn) {
            const int cb = c;
            int cm, dm;
            for (; c < d; c++) {
                p = level4_particle_at(state, c);
                Move_Or_Do(p, ps, mysd, 0, (d++), 0);
            }
            cm = c - 1;  dm = d - 1;
            for (; c < cn; c++) {
                p = level4_particle_at(state, c);
                Move_Or_Do(p, ps, mysd, 1,
                           level4_copy_particle_to_buffer(
                               state, state->particles, d++, p), 0);
            }
            rbb[s] = level4_particle_at(state, d);
            ip[s].size += d - ip[s].head;
            for (c = dm, d = dm; c >= cb; c--) {
                p = level4_particle_at(state, c);
                Move_Or_Do(p, ps, mysd, 1,
                           level4_copy_particle_to_buffer(
                               state, state->particles, d--, p), 0);
            }
        }
    }
}

static void move_to_sendbuf_dw4s(struct oh_state* state, const int ps,
                                 const int mysd, const int ctail,
                                 const int ntail) {
    const int ns = state->n_of_species;
    const int nsor0 = ps ? ns : 0;
    const int* ctp = state->total_particles + nsor0;
    const int* ntp = state->total_particles_next + nsor0;
    struct S_interiorp* ip = state->level4_interior_parts + nsor0;
    struct S_particle* sb = state->send_buffer, * p;
    void** rbb = state->recv_buffer_bases + nsor0;
    int s, c, d, cn, dn;

    cn = ctail;  dn = ntail;
    for (s = ns - 1, c = cn - 1, d = dn - 1; s >= 0; s--, c = cn - 1, d = dn - 1) {
        dint* npg = state->level4_particle_grid[ps][s];
        const int dd = d;
        cn -= ctp[s];  dn -= ntp[s];
        if (c >= d || cn >= dn)  continue;
        for (; c >= cn; c--) {
            p = level4_particle_at(state, c);
            Move_Or_Do(p, ps, mysd, 1,
                       level4_copy_particle_to_buffer(
                           state, state->particles, d--, p), 0);
        }
        ip[s].head = d - ip[s].size + 1;  ip[s].size += dd - d;
        rbb[s] = level4_particle_at(state, ip[s].head);
    }
}

#define Sort_Particle(P) {\
  const int g = level4_grid_position(\
      state, level4_particle_region(state, P, ps));\
  const dint dst = npg[g];\
  level4_copy_particle_to_buffer(state, sb, npgt[g]++, P);\
  if (dst<0) {\
    const dint bsbidx = -dst;\
    if (!Is_Pillar_Voxel(bsbidx)) {\
      level4_copy_particle_to_buffer(state, state->level4_boundary_send_buffer,\
                                     bsbidx - 1, P);\
      npg[g] = dst - 1;\
    }\
    else {\
      level4_copy_particle_to_buffer(state, state->level4_boundary_send_buffer,\
                                     Pillar_Lower(bsbidx) - 1, P);\
      level4_copy_particle_to_buffer(state, state->level4_boundary_send_buffer,\
                                     Pillar_Upper(bsbidx) - 1, P);\
      npg[g] = dst - (Add_Pillar_Voxel(1) + 1);\
    }\
  }\
}

static void sort_particles(struct oh_state* state, const int nextmode,
                           const int psnew, const int stats) {
    const int ns = state->n_of_species;
    struct S_particle* p, * sb = state->send_buffer;
    int ps, s, t, i;

    if (stats) oh1_stats_time(STATS_TB_SORT, nextmode);
    for (ps = 0, t = 0; ps <= psnew; ps++) {
        for (s = 0; s < ns; s++, t++) {
            dint* npg = state->level4_particle_grid[ps][s];
            dint* npgt = state->level4_particle_grid_total[ps][s];
            const int ips = state->level4_interior_parts[t].size;
            const int head = state->level4_interior_parts[t].head;
            for (i = 0; i < ips; i++) {
                p = level4_particle_at(state, head + i);
                Sort_Particle(p);
            }
        }
    }
}

static void move_and_sort(const int nextmode, const int psold, const int psnew,
                          const int oldp, const int* nacc, const int stats) {
    struct oh_state* state = oh4s_state();
    const int me = state->my_rank, ns = state->n_of_species;
    const int nn = state->n_of_nodes;
    const int mysubdom[2] = { me, oldp }, ninj = state->n_of_injections;
    struct S_realneighbor (*real_src)[2] =
        (struct S_realneighbor (*)[2])state->level4_real_src_neighbors;
    struct S_particle* p;
    struct S_particle* sb = oh_particle_buffer_at(state->particle_adapter,
                                                  state->send_buffer,
                                                  nacc[1]);
    int* nofr;
    int ps, s, t, i, pidx, rbb_index;
    Decl_For_All_Grid();

    if (stats) oh1_stats_time(STATS_TB_MOVE, nextmode);
    state_set_sendbuf_disps4s(state, nextmode, 0);
    for (ps = 0, t = 0, nofr = state->n_of_recv, rbb_index = 0;
         ps <= psnew; ps++) {
        const int nnbr = real_src[0][ps].n;
        const int* rnbr = real_src[0][ps].nbor;
        for (s = 0; s < ns; s++, t++, nofr += nn) {
            int n, nrec;
            for (n = 0, nrec = 0; n < nnbr; n++)  nrec += nofr[rnbr[n]];
            state->recv_buffer_bases[t] = level4_particle_at(state, rbb_index);
            rbb_index += nrec;
        }
    }
    state->recv_buffer_bases[t] = level4_particle_at(state, rbb_index);

    for (ps = 0, pidx = 0, t = 0; ps <= psold; ps++) {
        const int mysd = mysubdom[ps];
        for (s = 0; s < ns; s++, t++) {
            dint* npg = state->level4_particle_grid[ps][s];
            dint* npgt = state->level4_particle_grid_total[ps][s];
            const int itail = state->total_particles[t];
            for (i = 0; i < itail; i++, pidx++) {
                p = level4_particle_at(state, pidx);
                Move_Or_Do(p, ps, mysd, 1,
                           level4_copy_particle_to_buffer(
                               state, state->send_buffer, npgt[g]++, p), 1);
            }
        }
    }
    for (i = 0; i < ninj; i++, pidx++) {
        p = level4_particle_at(state, pidx);
        const int s = level4_particle_species(state, p);
        const OH_nid_t nid = level4_particle_region(state, p, 0);
        const int ps = level4_secondary_injected(state, nid) ? 1 : 0;
        const int mysd = mysubdom[ps];
        dint* npg = state->level4_particle_grid[ps][s];
        dint* npgt = state->level4_particle_grid_total[ps][s];
        if (nid < 0) continue;
        if (ps)  level4_primarize_particle_only(state, p);
        Move_Or_Do(p, ps, mysd, 1,
                   level4_copy_particle_to_buffer(state, state->send_buffer,
                                                  npgt[g]++, p), 1);
    }
    primaryParts = state->primary_parts = *state->secondary_base = nacc[0];
}

static void sort_received_particles(struct oh_state* state, const int nextmode,
                                    const int psnew, const int stats) {
    const int ns = state->n_of_species;
    int ps, s, pidx = 0;
    struct S_particle* sb = state->send_buffer;
    void** rbb = state->recv_buffer_bases + 1;

    if (stats) oh1_stats_time(STATS_TB_SORT, nextmode);
    for (ps = 0; ps <= psnew; ps++) {
        for (s = 0; s < ns; s++, rbb++) {
            dint* npg = state->level4_particle_grid[ps][s];
            dint* npgt = state->level4_particle_grid_total[ps][s];
            const int rbtail = level4_particle_index(state, *rbb);
            for (; pidx < rbtail; pidx++) {
                struct S_particle* p = level4_particle_at(state, pidx);
                Sort_Particle(p);
            }
        }
    }
}

static void state_set_sendbuf_disps4s(struct oh_state* state,
                                      const int nextmode, const int trans) {
    const int nn = state->n_of_nodes, ns = state->n_of_species;
    struct S_realneighbor (*real_dst)[2] =
        (struct S_realneighbor (*)[2])state->level4_real_dst_neighbors;
    int ps, s, i, np, * sbd;

    for (ps = 0, sbd = state->n_of_send, np = 0; ps <= nextmode; ps++) {
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

static void xfer_particles(const int trans, const int psnew, const int nextmode,
                           struct S_particle* sbuf) {
    state_xfer_particles4s(oh4s_state(), trans, psnew, nextmode, sbuf);
}

static void state_xfer_particles4s(struct oh_state* state, const int trans,
                                   const int psnew, const int nextmode,
                                   struct S_particle* sbuf) {
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
                    rbuf = oh_particle_buffer_at(state->particle_adapter,
                                                 rbuf, nrecv);
                }
            }
        }
    }
    for (ps = 0, t = 0, sdisp = 0, nofs = state->n_of_send; ps <= nextmode; ps++) {
        const int n = real_dst[trans][ps].n;
        const int* nbor = real_dst[trans][ps].nbor;
        for (s = 0; s < ns; s++, t++, nofs += nn) {
            for (i = 0; i < n; i++) {
                const int nid = nbor[i];
                const int sdnxt = nofs[nid];
                const int nsend = sdnxt - sdisp;
                nofs[nid] = 0;
                if (nsend) {
                    MPI_Isend(oh_particle_buffer_at(state->particle_adapter,
                                                    sbuf, sdisp),
                              nsend, state->particle_mpi_type, nid, t,
                              state->comm, state->requests + req++);
                }
                sdisp = sdnxt;
            }
        }
    }
    MPI_Waitall(req, state->requests, state->statuses);
}

static void xfer_boundary_particles_v(struct oh_state* state, const int psnew,
                                      const int trans, const int d) {
    const int ns = state->n_of_species;
    int vphi = d * 2 * 2;
    int* vplane_head = state->level4_vertical_plane_head;
    struct S_vplane* vplanes = state->level4_vertical_planes;
    const int vphead = vplane_head[vphi], vptail = vplane_head[vphi + 2 * 2];
    struct S_griddesc* GridDesc = state->level4_grid_desc;
    int (*z_bound)[2] = (int (*)[2])state->level4_z_bound;
    struct S_particle* boundary_send_buffer = state->level4_boundary_send_buffer;
    int i, s, req = 0, ps;
    struct S_vplane* vp;
    struct S_particle* p;
    Decl_For_All_Grid();

    if (vphead == vptail)  return;

    for (i = vphead, vp = vplanes + vphead; i < vptail; i++, vp++) {
        const int nrecv = vp->nrecv;
        if (nrecv)
            MPI_Irecv(level4_particle_at(state, vp->rbuf), nrecv,
                      state->particle_mpi_type, vp->nbor, vp->rtag,
                      state->comm, state->requests + req++);
    }
    for (i = vphead, vp = vplanes + vphead; i < vptail; i++, vp++) {
        const int nsend = vp->nsend;
        if (nsend)
            MPI_Isend(oh_particle_buffer_at(state->particle_adapter,
                                            boundary_send_buffer, vp->sbuf),
                      nsend,
                      state->particle_mpi_type, vp->nbor, vp->stag,
                      state->comm, state->requests + req++);
    }
    if (req == 0)  return;
    MPI_Waitall(req, state->requests, state->statuses);

    p = level4_particle_at(state, vplanes[vphead].rbuf);
    for (ps = 0; ps <= psnew; ps++) {
        const int psor2 = ps ? trans + 1 : 0;
        const int zl = z_bound[ps][OH_LOWER];
        const int zu = z_bound[ps][OH_UPPER] - GridDesc[psor2].z;
        int du;
        for (du = OH_LOWER; du <= OH_UPPER; du++, vphi++) {
            int ny;
            int xl, yl, xu, yu;
            if (vplane_head[vphi] == vplane_head[vphi + 1])  continue;
            if (d == OH_DIM_X) {
                ny = 1;
                Grid_Exterior_Boundary(du << 1, GridDesc[psor2].x, xl, xu);
            } else {
                ny = du << 1;
                xl = -OH_PGRID_EXT;  xu = OH_PGRID_EXT;
            }
            Grid_Exterior_Boundary(ny, GridDesc[psor2].y, yl, yu);
            For_All_Grid_Z(psor2, xl, yl, zl, xu, yu, zu) {
                for (s = 0; s < ns; s++) {
                    dint* npg = state->level4_particle_grid[ps][s];
                    int* npgo = state->level4_particle_grid_out[ps][s],
                        * npgi = state->level4_particle_grid_index[ps][s];
                    For_All_Grid_XY(psor2, xl, yl, xu, yu) {
                        const int g = The_Grid(), tail = npgi[g] + npgo[g];
                        const dint dst = npg[g];
                        int i;
                        if (Is_Pillar_Voxel(dst)) {
                            struct S_particle* q = p;
                            int j = Pillar_Upper(dst) - 1;
                            for (i = npgi[g]; i < tail; i++) {
                                level4_copy_particle_to_buffer(
                                    state, boundary_send_buffer, j++, q);
                                q = oh_particle_buffer_at(
                                    state->particle_adapter, q, 1);
                            }
                        }
                        for (i = npgi[g]; i < tail; i++) {
                            struct S_particle* sp =
                                oh_particle_buffer_at(state->particle_adapter,
                                                      state->send_buffer, i);
                            level4_copy_particle(state, sp, p);
                            level4_set_particle_region(state, sp, -2, ps);
                            p = oh_particle_buffer_at(state->particle_adapter,
                                                      p, 1);
                        }
                    }
                }
            }
        }
    }
}

static void xfer_boundary_particles_h(struct oh_state* state, const int psnew) {
    const int ns = state->n_of_species;
    struct S_hplane (*hplanes)[2] =
        (struct S_hplane (*)[2])state->level4_horizontal_planes;
    int ps, ud, s, req = 0;

    for (ps = 0; ps <= psnew; ps++) {
        for (ud = OH_LOWER; ud <= OH_UPPER; ud++) {
            struct S_hplane* hp = hplanes[ps] + ud;
            int* nrecv = hp->nrecv, * rbuf = hp->rbuf;
            const int nbor = hp->nbor, tag = hp->rtag;
            if (nbor != MPI_PROC_NULL) {
                for (s = 0; s < ns; s++) {
                    if (nrecv[s])
                        MPI_Irecv(oh_particle_buffer_at(state->particle_adapter,
                                                        state->send_buffer,
                                                        rbuf[s]),
                                  nrecv[s],
                                  state->particle_mpi_type, nbor, tag + s,
                                  state->comm, state->requests + req++);
                }
            }
        }
    }
    for (ps = 0; ps <= psnew; ps++) {
        for (ud = OH_LOWER; ud <= OH_UPPER; ud++) {
            struct S_hplane* hp = hplanes[ps] + ud;
            int* nsend = hp->nsend, * sbuf = hp->sbuf;
            const int nbor = hp->nbor, tag = hp->stag;
            if (nbor != MPI_PROC_NULL) {
                for (s = 0; s < ns; s++) {
                    if (nsend[s])
                        MPI_Isend(oh_particle_buffer_at(state->particle_adapter,
                                                        state->send_buffer,
                                                        sbuf[s]),
                                  nsend[s],
                                  state->particle_mpi_type, nbor, tag + s,
                                  state->comm, state->requests + req++);
                }
            }
        }
    }
    if (req == 0)  return;
    MPI_Waitall(req, state->requests, state->statuses);

    for (ps = 0; ps <= psnew; ps++) {
        for (ud = OH_LOWER; ud <= OH_UPPER; ud++) {
            struct S_hplane* hp = hplanes[ps] + ud;
            int* nrecv = hp->nrecv, * rbuf = hp->rbuf;
            if (hp->nbor != MPI_PROC_NULL) {
                for (s = 0; s < ns; s++) {
                    const int tail = rbuf[s] + nrecv[s];
                    int i;
                    for (i = rbuf[s]; i < tail; i++)
                        level4_set_particle_region(
                            state,
                            oh_particle_buffer_at(state->particle_adapter,
                                                  state->send_buffer, i),
                            -2, ps);
                }
            }
        }
    }
}

void oh4s_exchange_border_data_(void* buf, void* sbuf, void* rbuf, int* type) {
    oh4s_exchange_border_data(buf, sbuf, rbuf, MPI_Type_f2c(*type));
}

void oh4s_exchange_border_data(void* buf, void* sbuf, void* rbuf,
                               MPI_Datatype type) {
    struct oh_state* state = oh4s_state();
    MPI_Aint esize, lb;

    MPI_Type_get_extent(type, &lb, &esize);
    exchange_border_data_v(state, buf, sbuf, rbuf, type, esize, 0);
    exchange_border_data_v(state, buf, sbuf, rbuf, type, esize, 1);
    exchange_border_data_h(state, buf, type, esize);
}

static void exchange_border_data_v(struct oh_state* state, void* buf,
                                   void* sbuf, void* rbuf, MPI_Datatype type,
                                   const MPI_Aint esize, const int d) {
    char* b = (char*)buf, * sb = (char*)sbuf, * rb = (char*)rbuf;
    const int ns = state->n_of_species;
    const int pscurr = state->region_id[1] < 0 ? 0 : 1;
    const int vphi = d * 2 * 2;
    int* vplane_head = state->level4_vertical_plane_head;
    struct S_vplane* vplanes = state->level4_vertical_planes;
    const int vphead = vplane_head[vphi], vptail = vplane_head[vphi + 2 * 2];
    struct S_griddesc* GridDesc = state->level4_grid_desc;
    int (*z_bound)[2] = (int (*)[2])state->level4_z_bound;
    struct S_vplane* vp;
    int ps, s, i, req = 0;
    Decl_For_All_Grid();

    if (vphead == vptail)  return;

    for (ps = 0, i = vphi; ps <= pscurr; ps++) {
        int du;
        const int zl = z_bound[ps][OH_LOWER];
        const int zu = z_bound[ps][OH_UPPER] - GridDesc[ps].z;
        for (du = OH_LOWER; du <= OH_UPPER; du++, i++) {
            int ny;
            int xl, yl, xu, yu;
            if (vplane_head[i] == vplane_head[i + 1])  continue;
            if (d == OH_DIM_X) {
                ny = 1;
                Grid_Interior_Boundary(du << 1, GridDesc[ps].x, xl, xu);
            } else {
                ny = du << 1;
                xl = -OH_PGRID_EXT;  xu = OH_PGRID_EXT;
            }
            Grid_Interior_Boundary(ny, GridDesc[ps].y, yl, yu);
            For_All_Grid_Z(ps, xl, yl, zl, xu, yu, zu) {
                for (s = 0; s < ns; s++) {
                    int* npgo = state->level4_particle_grid_out[ps][s],
                        * npgi = state->level4_particle_grid_index[ps][s];
                    For_All_Grid_XY(ps, xl, yl, xu, yu) {
                        const int g = The_Grid(), nbyte = npgo[g] * esize;
                        memcpy(sb, b + npgi[g] * esize, nbyte);
                        sb += nbyte;
                    }
                }
            }
        }
    }
    rb -= vplanes[vphead].rbuf * esize;
    for (i = vphead, vp = vplanes + vphead; i < vptail; i++, vp++) {
        const int nrecv = vp->nrecv;
        if (nrecv)
            MPI_Irecv(rb + vp->rbuf * esize, nrecv, type, vp->nbor, vp->rtag,
                      state->comm, state->requests + req++);
    }
    sb = (char*)sbuf - vplanes[vphead].sbuf * esize;
    for (i = vphead, vp = vplanes + vphead; i < vptail; i++, vp++) {
        const int nsend = vp->nsend;
        if (nsend)
            MPI_Isend(sb + vp->sbuf * esize, nsend, type, vp->nbor, vp->stag,
                      state->comm, state->requests + req++);
    }
    if (req == 0)  return;
    MPI_Waitall(req, state->requests, state->statuses);

    rb = (char*)rbuf;
    for (ps = 0, i = vphi; ps <= pscurr; ps++) {
        const int zl = z_bound[ps][OH_LOWER];
        const int zu = z_bound[ps][OH_UPPER] - GridDesc[ps].z;
        int du;
        for (du = OH_LOWER; du <= OH_UPPER; du++, i++) {
            int ny;
            int xl, yl, xu, yu;
            if (vplane_head[i] == vplane_head[i + 1])  continue;
            if (d == OH_DIM_X) {
                ny = 1;
                Grid_Exterior_Boundary(du << 1, GridDesc[ps].x, xl, xu);
            } else {
                ny = du << 1;
                xl = -OH_PGRID_EXT;  xu = OH_PGRID_EXT;
            }
            Grid_Exterior_Boundary(ny, GridDesc[ps].y, yl, yu);
            For_All_Grid_Z(ps, xl, yl, zl, xu, yu, zu) {
                for (s = 0; s < ns; s++) {
                    int* npgo = state->level4_particle_grid_out[ps][s],
                        * npgi = state->level4_particle_grid_index[ps][s];
                    For_All_Grid_XY(ps, xl, yl, xu, yu) {
                        const int g = The_Grid(), nbyte = npgo[g] * esize;
                        memcpy(b + npgi[g] * esize, rb, nbyte);
                        rb += nbyte;
                    }
                }
            }
        }
    }
}

static void exchange_border_data_h(struct oh_state* state, void* buf,
                                   MPI_Datatype type, const MPI_Aint esize) {
    char* b = (char*)buf;
    const int ns = state->n_of_species;
    const int pscurr = state->region_id[1] < 0 ? 0 : 1;
    struct S_hplane (*hplanes)[2] =
        (struct S_hplane (*)[2])state->level4_horizontal_planes;
    int ps, ud, s, req = 0;
    Decl_For_All_Grid();

    for (ps = 0; ps <= pscurr; ps++) {
        for (ud = OH_LOWER; ud <= OH_UPPER; ud++) {
            struct S_hplane* hp = hplanes[ps] + ud;
            int* nrecv = hp->nrecv, * rbuf = hp->rbuf;
            const int nbor = hp->nbor, tag = hp->rtag;
            if (nbor != MPI_PROC_NULL) {
                for (s = 0; s < ns; s++) {
                    if (nrecv[s])
                        MPI_Irecv(b + rbuf[s] * esize, nrecv[s], type, nbor,
                                  tag + s, state->comm,
                                  state->requests + req++);
                }
            }
        }
    }
    for (ps = 0; ps <= pscurr; ps++) {
        for (ud = OH_LOWER; ud <= OH_UPPER; ud++) {
            struct S_hplane* hp = hplanes[ps] + ud;
            int* nsend = hp->nsend, * sbuf = hp->sbuf;
            const int nbor = hp->nbor, tag = hp->stag;
            if (nbor != MPI_PROC_NULL) {
                for (s = 0; s < ns; s++) {
                    if (nsend[s])
                        MPI_Isend(b + sbuf[s] * esize, nsend[s], type, nbor,
                                  tag + s, state->comm,
                                  state->requests + req++);
                }
            }
        }
    }
    if (req)  MPI_Waitall(req, state->requests, state->statuses);
}

static void check_particle_location4s(struct oh_state* state,
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
  const double gsize = state->grid[DIM].gsize;\
  const double lb = state->grid[DIM].fcoord[OH_LOWER];\
  const double gf =\
    (G = (xyz-lb)*state->grid[DIM].rgsize +\
         state->grid[DIM].coord[OH_LOWER]) * gsize;\
  if (gf>xyz) G--;\
  else if (gf+gsize<=xyz) G++;\
  G -= state->subdomains[MYSD][DIM][OH_LOWER];  IDX += G;\
  if (G<0) {\
    K -= INC;\
    if (xyz<lb) {\
      if (state->boundaries[MYSD][DIM][OH_LOWER]) {\
        level4_set_particle_region(state, P, -1, ps);  return(-1);\
      }\
      XYZ += state->grid[DIM].fcoord[OH_UPPER] - lb;\
    }\
    if (G<-OH_PGRID_EXT)  K = -OH_NEIGHBORS;\
  } else if (G>=UB) {\
    double ub = state->grid[DIM].fcoord[OH_UPPER];\
    K += INC;\
    if (xyz>=ub) {\
      if (state->boundaries[MYSD][DIM][OH_UPPER]) {\
        level4_set_particle_region(state, P, -1, ps);  return(-1);\
      }\
      XYZ -= ub - lb;\
    }\
    G-=UB;\
    if (G>=OH_PGRID_EXT)  K = -OH_NEIGHBORS;\
  }\
}

#define Adjust_Neighbor_Grid(G, N, DIM)\
  if (G<0) G += state->subdomains[N][DIM][OH_UPPER] - \
                 state->subdomains[N][DIM][OH_LOWER];

int oh4s_map_particle_to_neighbor_(struct S_particle* part, const int* ps,
                                   const int* s) {
    return(oh4s_map_particle_to_neighbor(part, *ps, *s - 1));
}

int oh4s_map_particle_to_neighbor(void* particle, const int ps,
                                  const int s) {
    struct oh_state* state = oh4s_state();
    struct S_particle* part = (struct S_particle*)particle;
    const int ns = state->n_of_species, nn = state->n_of_nodes;
    const int inj = level4_particle_is_injected(state, part);
    struct S_griddesc* grid_desc = state->level4_grid_desc;
    int (*abs_neighbors)[OH_NEIGHBORS] = state->abs_neighbors;
    int x, y, z, w, d, dw, mysd;
    const int psnn = ps ? (s + ns) * nn : s * nn;
    int k = OH_NBR_SELF, idx = 0;
    double *xpos, *ypos, *zpos;
    int gz, gy, gx;
    int sd;

    check_particle_location4s(state, part, ps, s, inj);
    x = grid_desc[ps].x;  y = grid_desc[ps].y;  z = grid_desc[ps].z;
    w = grid_desc[ps].w;  d = grid_desc[ps].d;  dw = grid_desc[ps].dw;
    mysd = state->region_id[ps];
    xpos = level4_particle_position(state, part, OH_DIM_X);
    ypos = level4_particle_position(state, part, OH_DIM_Y);
    zpos = level4_particle_position(state, part, OH_DIM_Z);
    Do_Z(Map_Particle_To_Neighbor(part, *zpos, OH_DIM_Z, mysd, k, 9, z, gz,
                                  idx));
    Do_Z(idx *= d);
    Do_Y(Map_Particle_To_Neighbor(part, *ypos, OH_DIM_Y, mysd, k, 3, y, gy,
                                  idx));
    Do_Y(idx *= w);
    Map_Particle_To_Neighbor(part, *xpos, OH_DIM_X, mysd, k, 1, x, gx, idx);

    if (k == OH_NBR_SELF) {
        state->level4_particle_grid[ps][s][idx]++;
        state->n_of_particles_local[psnn + mysd]++;
        level4_set_particle_region(
            state, part, level4_combine_subdomain_position(state, k, idx), ps);
        if (inj) {
            if (ps) {
                state->injected_particles[ns + s]++;
                level4_secondarize_particle(state, part);
            } else {
                state->injected_particles[s]++;
            }
        }
        return(mysd);
    } else if (k < 0)
        return(oh4s_map_particle_to_subdomain(part, ps, s));
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
            state, part,
            level4_combine_subdomain_position(state, OH_NBR_SELF, idx), ps);
        if (inj)  state->injected_particles[ps ? ns + s : s]++;
    } else {
        state->level4_particle_grid[ps][s][idx]++;
        level4_set_particle_region(
            state, part,
            level4_combine_subdomain_position(
                state, k, Coord_To_Index(gx, gy, gz, w, dw)),
            ps);
    }
    if (inj && ps)  level4_secondarize_particle(state, part);
    return(sd);
}

#define Map_To_Grid(P, PXYZ, XYZ, DIM, GG, LG) {\
  const double gsize = state->grid[DIM].gsize;\
  const double lb = state->grid[DIM].fcoord[OH_LOWER];\
  const double ub = state->grid[DIM].fcoord[OH_UPPER];\
  double gf;\
  XYZ = PXYZ;\
  LG = 0;\
  if (XYZ<lb) {\
    if (Level4_Boundary_Condition(DIM, OH_LOWER)) {\
      level4_set_particle_region(state, P, -1, ps);  return(-1);\
    }\
    XYZ += (ub - lb);  PXYZ = XYZ;\
    LG = state->grid[DIM].coord[OH_LOWER] -\
         state->grid[DIM].coord[OH_UPPER];\
  }\
  else if (XYZ>=ub) {\
    if (Level4_Boundary_Condition(DIM, OH_UPPER)) {\
      level4_set_particle_region(state, P, -1, ps);  return(-1);\
    }\
    XYZ -= (ub - lb);  PXYZ = XYZ;\
    LG = state->grid[DIM].coord[OH_UPPER] -\
         state->grid[DIM].coord[OH_LOWER];\
  }\
  GG = (XYZ-lb)*state->grid[DIM].rgsize +\
       state->grid[DIM].coord[OH_LOWER];\
  gf = GG * gsize;\
  if (gf>XYZ) GG--;\
  else if (gf+gsize<=XYZ) GG++;\
  LG += GG;\
}

#define Map_Particle_To_Subdomain(XYZ, DIM, SDOM) {\
  double thresh = state->grid[DIM].light.thresh;\
  if (XYZ<thresh)\
    SDOM = (XYZ - state->grid[DIM].coord[OH_LOWER]) /\
           state->grid[DIM].light.size;\
  else\
    SDOM = (XYZ - thresh)/ (state->grid[DIM].light.size + 1) +\
           state->grid[DIM].light.n;\
}

#define Local_Coordinate(N, MYSD, GG, LG, DIM, K, INC, AA) {\
  GG -= state->subdomains[N][DIM][OH_LOWER];\
  if (N==MYSD)  LG = GG;\
  else {\
    const int ub = state->subdomains[MYSD][DIM][OH_UPPER];\
    if (LG>=ub+OH_PGRID_EXT)  AA = 1;\
    else {\
      const int inc = LG<ub ? 0 : INC;\
      LG -= state->subdomains[MYSD][DIM][OH_LOWER];\
      if (LG<-OH_PGRID_EXT)  AA = 1;\
      k += LG<0 ? -INC : inc;\
    }\
  }\
}

int oh4s_map_particle_to_subdomain_(struct S_particle* part, const int* ps,
                                    const int* s) {
    return(oh4s_map_particle_to_subdomain(part, *ps, *s - 1));
}

int oh4s_map_particle_to_subdomain(void* particle, const int ps,
                                   const int s) {
    struct oh_state* state = oh4s_state();
    struct S_particle* part = (struct S_particle*)particle;
    const int ns = state->n_of_species, nn = state->n_of_nodes;
    const int inj = level4_particle_is_injected(state, part);
    struct S_subdomdesc* subdomain_desc = state->subdomain_desc;
    struct S_griddesc* grid_desc = state->level4_grid_desc;
    const int nx = state->grid[OH_DIM_X].n;
    const int nxy = If_Dim(OH_DIM_Y, nx * state->grid[OH_DIM_Y].n, 0);
    const int t = ps ? ns + s : s;
    int w, dw, mysd;
    int sd;
    double x, y, z;
    double *xpos, *ypos, *zpos;
    int px, py, pz;
    int gx, gy, gz;
    int lx, ly, lz;
    int k = OH_NBR_SELF, aacc = 0;

    check_particle_location4s(state, part, ps, s, inj);
    w = grid_desc[ps].w;  dw = grid_desc[ps].dw;  mysd = state->region_id[ps];
    xpos = level4_particle_position(state, part, OH_DIM_X);
    ypos = level4_particle_position(state, part, OH_DIM_Y);
    zpos = level4_particle_position(state, part, OH_DIM_Z);
    Map_To_Grid(part, *xpos, x, OH_DIM_X, gx, lx);
    Do_Y(Map_To_Grid(part, *ypos, y, OH_DIM_Y, gy, ly));
    Do_Z(Map_To_Grid(part, *zpos, z, OH_DIM_Z, gz, lz));
    if (subdomain_desc) {
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
            level4_combine_subdomain_position(
                state, sd + OH_NEIGHBORS,
                Coord_To_Index(gx, gy, gz, w, dw)),
            ps);
    } else {
        state->level4_particle_grid[ps][s][Coord_To_Index(lx, ly, lz, w, dw)]++;
        level4_set_particle_region(
            state, part,
            level4_combine_subdomain_position(
                state, k, Coord_To_Index(gx, gy, gz, w, dw)),
            ps);
    }
    if (inj) {
        if (sd == mysd)  state->injected_particles[t]++;
        if (ps)  level4_secondarize_particle(state, part);
    }
    return(sd);
}

int oh4s_inject_particle_(const struct S_particle* part, const int* ps) {
    return(oh4s_inject_particle(part, *ps));
}

int oh4s_inject_particle(const void* particle, const int ps) {
    struct oh_state* state = oh4s_state();
    const struct S_particle* part = (const struct S_particle*)particle;
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
    sd = oh4s_map_particle_to_neighbor(p, ps, s);
    if (sd < 0) {
        state->n_of_injections--;
        nOfInjections = state->n_of_injections;
    }
    return(sd);
}
void oh4s_remove_mapped_particle_(struct S_particle* part, const int* ps,
                                  const int* s) {
    oh4s_remove_mapped_particle(part, *ps, *s - 1);
}
void oh4s_remove_mapped_particle(void* particle, const int ps,
                                 const int s) {
    struct oh_state* state = oh4s_state();
    struct S_particle* part = (struct S_particle*)particle;
    const int nn = state->n_of_nodes, ns = state->n_of_species;
    const int inj = level4_particle_is_injected(state, part);
    int sd, g, psreal = ps, mysd, t;
    OH_nid_t nid = level4_particle_region(state, part, psreal);

    check_particle_location4s(state, part, psreal, s, inj);
    if (nid < 0)  return;
    sd = level4_subdomain_id(state, nid, psreal);
    g = level4_grid_position(state, nid);
    if (sd >= nn) {
        psreal = 1;  sd = level4_primarize_particle(state, part);
        nid = level4_particle_region(state, part, psreal);
    }
    mysd = state->region_id[psreal];
    level4_set_particle_region(state, part, -1, psreal);
    t = psreal ? ns + s : s;
    state->n_of_particles_local[t * nn + sd]--;
    if (inj && sd == mysd)  state->injected_particles[t]--;
    if (Mode_Acc(state->curr_mode))  return;
    if (sd != mysd)  g = level4_local_grid_position(state, g, nid, psreal);
    state->level4_particle_grid[psreal][s][g]--;
}

int oh4s_remap_particle_to_neighbor_(struct S_particle* part, const int* ps, const int* s) {
    return(oh4s_remap_particle_to_neighbor(part, *ps, *s - 1));
}

int oh4s_remap_particle_to_neighbor(void* part, const int ps, const int s) {
    oh4s_remove_mapped_particle(part, ps, s);
    return(oh4s_map_particle_to_neighbor(part, ps, s));
}

int oh4s_remap_particle_to_subdomain_(struct S_particle* part, const int* ps, const int* s) {
    return(oh4s_remap_particle_to_subdomain(part, *ps, *s - 1));
}

int oh4s_remap_particle_to_subdomain(void* part, const int ps, const int s) {
    oh4s_remove_mapped_particle(part, ps, s);
    return(oh4s_map_particle_to_subdomain(part, ps, s));
}
