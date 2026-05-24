# 4.10 C Source File ohhelp4p.c - Part 1

Source: `doc/v1/original/ohhelp.pdf`, pages 346-374.

<!-- Page 346 -->

## 4.10 C Source File ohhelp4p.c

#### 4.10.1 Header File Inclusion

The first job done in ohhelp4p.c is the inclusion of the header files ohhelp1.h, ohhelp2.h,
ohhelp3.h and ohhelp4p.h. Before the inclusion of ohhelp1.h, ohhelp2.h and ohhelp3.h, we
#define the macro EXTERN as extern so as to make variables declared in the files external,
but after that we make it #undef’iend and then #define it as empty so as to provide
variables declared in ohhelp4p.h with their homes, as discussed in §4.2.3.

#define EXTERN extern
#include "ohhelp1.h"
#include "ohhelp2.h"
#include "ohhelp3.h"
#undef  EXTERN
#define EXTERN
#include "ohhelp4p.h"



#### 4.10.2 Function Prototypes

The next and last job to do prior to macro and function definitions is to declare the
prototypes of the following functions private for the level-4p library.

- The function init4p() is the body of oh4p_init().

- The function transbound4p() is the body of oh4p_transbound().

- The function try_primary4p() performs position-aware particle transfer in primary
mode after calling its level-1 couterpart try_primary1() to check if we will be in
primary mode in the next step.

- The function try_stable4p() performs position-aware particle transfer in secondary
mode after calling its level-1 couterpart try_stable1() to check if we can keep the
helpand-helper configuration.

- The function rebalance4p() performs position-aware particle transfer in secondary
mode after calling its level-1 counterpart rebalance1() to establish a new helpand-
helper configuration.

- The function exchange_particles4p() is the core of position-aware particle transfer
in secondary mode.

- The function exchange_population() gathers particle population in each grid-voxel
to build per-grid histogram.

- The function add_population() adds particle populatoin in each grid-voxel in re-
ceiving planes to that in boundary planes.

- The function mpi_allreduce_wrapper() is the wrapper function of MPI_Allreduce()
to call it with the API of MPI_Reduce().

- The function reduce_population() sums per-grid histograms of the family members.


<!-- Page 347 -->

- The function make_recv_list() scans per-grid histogram to build primary receiving
block, and then exchanges the block between neighbors to have primary sending
block and broadcast them for secondary receiving, secondary sending and alternative
secondary receiving blocks for helpers.

- The function sched_recv() scans per-grid histogram to determine the set of grid-
voxels to be hosted by a node and to find a hot-spot.

- The function make_send_sched() scans primary receiving, primary sending, sec-
ondary receiving, secondary sending and alternative secondary receiving blocks to
determine the node to which the local node sends the particles in each grid-voxel and
processes all hot-spots after the scan.

- The function make_send_sched_body() scans a primary receiving block created by
the local node itself, or that given from a neighbor, the helpand or a neighbor of the
helpand to determine the node which accommodates the particles in each grid-voxel
and to find hot-spots in the block.

- The function gather_hspot_recv() initiates the gather reception to have all accom-
modation data of a hot-spot.

- The function gather_hspot_send() scans hot-spots having a specific ordinal in all
neighboring subdomains to send their accommodaion data.

- The function gather_hspot_send_body() sends the accommodation data of a hot-
spot and initates scatter receptions to have receivers of particles in it and, if the local
node should host it, the amount of particles to accommodate.

- The function scatter_hspot_send() builds the accommodations of a hot-spot and
sends its sending and receiving schedules to the nodes involoved.

- The function scatter_hspot_recv() scans hot-spots having a specific ordinal in all
neighboring subdomains to examine their sending and receiving schedules.

- The function scatter_hspot_recv_body() examines the sending and receiving
schedule of a hot-spot.

- The  function update_descriptors()  updates  elements  in  FieldDesc[][][] and
BorderExc[][][][] for the secondary subdomain newly assigned to the local node by
rebalancing.

- The function update_neighbors() initializes/updates AbsNeighbors[][] and Grid
Offset[][] for the local node’s primary or secondary subdomain.

- The function set_grid_descriptor() sets an element of GridDesc[] according to a
subdomain.

- The function adjust_field_descriptor() adjusts FieldDesc[F−1].{bc, red}.size
[] for the broadcast and reduction of per-grid histogram.

- The function update_real_neighbors() updates RealDstNeighbors[][] and RealSrc
Neighbors[][].

- The function upd_real_nbr() updates an element array of RealDstNeighbors[][] or
RealSrcNeighbors[][].


<!-- Page 348 -->

- The function exchange_xfer_amount() performs a hand-made all-to-all communica-
tion to send NOfSend[][][] and to receive it to NOfRecv[][][].

- The function count_population() accumulates the number of particles in each grid-
voxel in primary and secondary subdomains to have the local per-grid histogram.

- The function sort_particles() performs bucket sorting on Particles[] to have
sorted result in SendBuf[].

- The function move_and_sort_primary() moves  all  particles  in Particles[] to
SendBuf[] sorting those staying in the local node, if we are in primary mode in next
simulation step.

- The function sort_received_particles() moves  all received  particles in each
rbuf (p, s) to SendBuf[] sorting them.

- The function move_to_sendbuf_sec4p()  is the level-4p counterpart of move_to_
sendbuf_secondary() to move particles to be sent to SendBuf[] and pack those to
stay in the local node in Particles[].

- The function move_to_sendbuf_uw4p()  is the level-4p counterpart of move_to_
sendbuf_uw() to move particles to be sent to SendBuf[] and pack those to stay in the
local node shifting upward in Particles[].

- The function move_to_sendbuf_dw4p()  is the level-4p counterpart of move_to_
sendbuf_dw() to move particles to be sent to SendBuf[] and pack those to stay in the
local node shifting downward in Particles[].

- The function move_and_sort_secondary() moves all particles in Particles[] to
SendBuf[] sorting those staying in the local node,  if we are in secondary mode in
next simulation step.

- The function set_sendbuf_disps4p() is the level-4p counterpart of set_sendbuf_
disps() to updates entries of NOfSend[][][] so that each of its entry has the displace-
ment of the head of sbuf (p, s, m).

- The function xfer_particles() performs a hand-made all-to-all communication to
exchange particles.


static void init4p(int **sdid, const int nspec, const int maxfrac,
int **totalp, struct S_particle **pbuf, int **pbase,
int maxlocalp, struct S_mycommc *mycommc,
struct S_mycommf *mycommf, int **nbor, int *pcoord,
int **sdoms, int *scoord, const int nbound, int *bcond,
int **bounds, int *ftypes, int *cfields, const int cfid,
int *ctypes, int **fsizes,
const int stats, const int repiter, const int verbose);
static int  transbound4p(int currmode, int stats, const int level);
static int  try_primary4p(const int currmode, const int level,
const int stats);
static int  try_stable4p(const int currmode, const int level, const int stats);
static void rebalance4p(const int currmode, const int level, const int stats);
static void exchange_particles4p(const int currmode, const int level,
int reb, int oldp, const int newp,


<!-- Page 349 -->

const int stats);
static void exchange_population(const int currmode, const int nextmode);
static void add_population(dint *npd, const int xl, const int xu,
const int yl, const int yu, const int zl,
const int zu, const int src);
static int  mpi_allreduce_wrapper(void *sendbuf, void *recvbuf,
const int count, MPI_Datatype datatype,
MPI_Op op, const int root, MPI_Comm comm);
static void reduce_population(int (*mpired)(void*, void*, int,
MPI_Datatype, MPI_Op, int,
MPI_Comm));
static struct S_commlist* make_recv_list(const int currmode,
const int level, const int reb,
const int oldp, const int newp,
const int stats);
static void sched_recv(const int currmode, const int reb, const int get,
const int stay, const int nid, const int tag,
struct S_recvsched_context *context);
static void make_send_sched(const int currmode, const int reb, const int pcode,
const int oldp, const int newp,
struct S_commlist *hslist, int *nacc, int *nsend);
static void make_send_sched_body(const int psor2, const int n, const int sdid,
const int self, const int sender,
struct S_commlist *rlist, int *maxhs,
int *naccs, int *nsendptr);
static int  gather_hspot_recv(const int currmode, const int reb,
const struct S_hotspot *hs);
static void gather_hspot_send(const int hsidx, const int pcode, const int rreq,
const int nfrom, const int nto,
struct S_commlist **hslist, int *sreqptr);
static void gather_hspot_send_body(const int hsidx, const int psor2,
const int n, int dst, const int sender,
struct S_commlist **hslist,
MPI_Request *reqs, int *sreqptr);
static void scatter_hspot_send(const int rreq, int *nacc,
struct S_commlist **hslist);
static int  scatter_hspot_recv(const int hsidx, const int pcode,
const int rreq, const int sreq, const int nfrom,
const int nto, int *nacc, int *nsend);
static void scatter_hspot_recv_body(const int hsidx, const int psor2,
const int n, int *naccptr, int *nsendptr);
static void update_descriptors(const int oldp, const int newp);
static void update_neighbors(const int ps);
static void set_grid_descriptor(const int idx, const int nid);
static void adjust_field_descriptor(const int ps);
static void update_real_neighbors(const int mode, const int dosec,
const int oldp, const int newp);
static void upd_real_nbr(const int root, const int psp, const int pss,
const int nbr, const int dosec, struct S_node *node,
struct S_realneighbor rnbrptr[2], int *occur[2]);
static void exchange_xfer_amount(const int trans, const int psnew);
static void count_population(const int nextmode, const int psnew,
const int stats);
static void sort_particles(dint ***npg, const int nextmode, const int psnew,


<!-- Page 350 -->

const int stats);
static void move_and_sort_primary(dint ***npg, const int psold,
const int stats);
static void sort_received_particles(const int nextmode, const int psnew,
const int stats);
static void move_to_sendbuf_sec4p(const int psold, const int trans,
const int oldp, const int *nacc,
const int nsend, const int stats);
static void move_to_sendbuf_uw4p(const int ps, const int mysd, const int cbase,
const int nbase);
static void move_to_sendbuf_dw4p(const int ps, const int mysd, const int ctail,
const int ntail);
static void move_and_sort_secondary(const int psold, const int psnew,
const int trans, const int oldp,
const int *nacc, const int stats);
static void set_sendbuf_disps4p(const int trans);
static void xfer_particles(const int trans, const int psnew,
struct S_particle *sbuf);


In addition, we use the following lower level library functions.

- The function mem_alloc() allocates a memory space by malloc(). It is called from
init4p() directly or through the macro Allocate_NOfPGrid().

- The function mem_alloc_error() aborts the simulation due to the memory shortage
reporting its cause. It is called from oh4p_max_local_particles().

- The function errstop() aborts the simulation due to an error detected by all processes
reporting given error message. It is called from init4p().

- The function local_errstop() aborts the simulation due to an error detected
by the  local process reporting given  error message.    It  is  called from oh4p_
inject_particle() directly and from oh4p_map_particle_to_neighbor(), oh4p_
map_particle_to_subdomain() and oh4p_remove_mapped_particle() through the
macro Check_Particle_Location().

- The function transbound1() is the body of oh1_transbound().  It is called from
transbound4p().

- The function try_primary1() is to examine whether particle distribution among
subdomains is balanced well and thus we can perform the simulation in primary
mode. It is called from try_primary4p().

- The function try_stable1() is to examine whether particle distribution among nodes
is balanced well and thus we can keep the current helpand-helper configuration. It is
called from try_stable4p().

- The function rebalance1() is to (re)build the helpand-helper configuration to cope
with an unacceptable load imbalance. It is called from rebalance4p().

- The function build_new_comm() is to build communicators for the helpand-helper
families built by rebalance1(). It is called from make_recv_list().

- The function exchange_primary_particles() is the core of the particle transfer in
primary mode. It is called from try_primary4p().


<!-- Page 351 -->

- The function move_to_sendbuf_primary() moves particles to be transferred from
Particles[] to SendBuf[] and packs those remaining in Particles[] in primary mode.
It is called from try_primary4p().

- The function set_sendbuf_disps() calculates each entry of SendBufDisps[][]. It is
called from move_and_sort_primary().

- The function exchange_particles() is the core of the particle transfer in secondary
mode. It is called from exchange_particles4p().

- The function init3() is the body of oh3_init(). It is called from init4p().

- The function set_field_descriptors() sets FieldDesc[f].{bc, red}.size[p] for all
f ∈[0, F) and given p ∈{0, 1}. It is called from update_descriptors().

- The  function  clear_border_exchange()  initializes  BorderExc[c][1][d][β].{send,
recv} for all c ∈[0, C), d ∈[0, D) and β ∈{0, 1}, or reinitializes them for the
subdomain which the local node has had as the secondary one but discarded by re-
balancing. It is called from update_descriptors().

- The function map_irregular_subdomain() finds the subdomain of irregular process
coordinate in which a particle resides.  It is called from oh4p_map_particle_to_
subdomain().

4.10.3  Macros If_Dim(), For_Z(), For_Y(), Do_Z(), Do_Y(), Coord_To_Index() and
Index_To_Coord()

Before starting to define functions, we define macros generally used in level-4p functions.
The first group is for dimension dpendent operations.

If_Dim()  The macro If Dim(d, et, ef) gives the expression et if d < D, while ef  is given otherwise.
Though the macro is expanded to the trinary expression examining d < D, it is expected
compilers transform the macro into et or ef because d is a constant, OH_DIM_Y or OH_DIM_
Z. The macro is used in init4p(), gather_hspot_recv(), set_grid_descriptor() and
oh4p_map_particle_to_subdomain().


#define If_Dim(D, ET, EF)  (OH_DIMENSION>D ? (ET) : (EF))

The other macros in this group have dimension dependent definitions and thus defined
in #if/#else/#endif construct to examine OH_DIMENSION.

For_Y()  The macro For Y(i, c, n) and For Z(i, c, n) are expanded to the statement i if D < 2 and
For_Z() D < 3 respectively, while they are expanded to the for-loop header for(i;c;n) to construct
a for-loop for the dimension 2 (y) or 3 (z). They are used in the macros For_All_Grid(),
For_All_Grid_Abs() and For_All_Grid_From().

Do_Z()  The macro Do Y(a) and Do Z(a) are expanded to nothing  if D < 2 and D < 3 respec-
Do_Y()   tively, while they are expanded to a otherwise. They are used in oh4p_map_particle_to_
neighbor() and oh4p_map_particle_to_subdomain() for the shorthand of;

#if OH DIM d<OH_DIMENSION
a
#endif


<!-- Page 352 -->

where d = Y or d = Z.

Coord_To_Index()  The macro Coord To Index(x, y, z, w, d·w) is expanded to x+y ·w+z ·d·w to give the one-
dimensional index of the element [x], [y][x] or [z][y][x] in a (conceptual) D-dimensional array
of [w], [d][w] or [h][d][w], i.e. gidx(x, y, z). The array size paramenter d is assumed to be 0 if
D ≤2 and w to be 0 if D = 1. The macro is used in the macro For_All_Grid(), For_All_
Grid_Abs() and Allocate_NOfPGrid(), and functions sched_recv(), make_send_sched_
body(), oh4p_map_particle_to_neighbor() and oh4p_map_particle_to_subdomain().

Index_To_Coord()  The macro Index To Coord(i, x, y, z, w, d·w) assigns values to x, y, z such that i = x +
y · w + z · d · w to translate the one-dimensional index i to its corresponding element
[x], [y][x] or [z][y][x] in a (conceptual) D-dimensional array of [w], [d][w] or [h][d][w], i.e.,
i = gidx(x, y, z). The variable y and/or z will have 0 if D < 2 or D < 3 respectively. The
macro is used in gather_hspot_recv().


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


4.10.4  Macros Decl_For_All_Grid(), For_All_Grid(), For_All_Grid_Abs(),
The_Grid(), Grid_X(), Grid_Y() and Grid_Z()

The next group of generally used macros are for traversing per-grid histogram.


<!-- Page 353 -->

Decl_For_All_Grid()  The macro Decl_For_All_Grid() declares the following special local variables for For_All_
Grid(), For_All_Grid_Abs() and For_All_Grid_From(), whose names have a common
prefix fag , for the traversal of grid-voxels (x, y, z) where x ∈[x0, x1), y ∈[y0, y1) and
z ∈[z0, z1);

- x1, y1 and z1 have x1, y1 and z1 respecively.

- xidx, yidx and zidx have x, y and z respectively.

- gx, gy and gz have gidx(x, y, z), gidx(x0, y, z) and gidx(x0, y0, z) respectively.

- w has GridDesc[].w = δmaxx  +4eg and dw has GridDesc[].dw = (δmaxx  +4eg)(δmaxy  +4eg).

The macro is used in functions that use For_All_Grid(), For_All_Grid_Abs() or For_
All_Grid_From().


#define Decl_For_All_Grid()\
int fag_x1, fag_y1, fag_z1;\
int fag_xidx, fag_yidx, fag_zidx, fag_gx, fag_gy, fag_gz;\
int fag_w, fag_dw;


For_All_Grid()  The macro For All Grid(p, x0, y0, z0, x1, y1, z1) constracts nested for-loops to traverse grid-
For_All_Grid_Abs()  voxels (x, y, z) in the per-grid histogram of local node n’s primary (p = 0) or secondary
(p = 1) subdomains, where x ∈[x0, δx(m)+x1), y ∈[y0, δy(m)+y1) and z ∈[z0, δz(m)+z1),
and m = n or m = parent(n). The macro For All Grid Abs(p, x0, y0, z0, x1, y1, z1) acts
similarly but the ranges are x ∈[x0, x1), y ∈[y0, y1) and z ∈[z0, z1).  Though their
definitions are fairly complicated due to the special variable names with fag and dimension
dependent definitions, the expansion results are not so jumbled as shown in the For_All_
Grid()’s example with D = 3 below.

for(z = z0, x1 = δx(m) + x1,  y1 = δy(m) + y1, z1 = δz(m) + z1,
w = δmaxx  (m) + 4eg, d = δmaxy  (m) + 4eg, gz = gidx(x0, y0, z0);
z < z1; z++, gz = gz + d · w)
for(y = y0, gy = gz; y < y1; y++, gy = gy + w)
for(x = x0, gx = gy; x < x1; x++, gx++)

Note that if D < 3, the outer for-loops are replaced with their initialization part by For_Z()
and For_Y(), and thus D = 1 case of the example above is as follows effectively.

x1 = δx(m) + x1;
for(x = x0, gx = gidx(x0); x < x1; x++, gx++)

The macro For_All_Grid()  is  used  in transbound4p(),  exchange_population(),
make_send_sched_body(), count_population(), sort_particles(), move_and_sort_
primary() and move_and_sort_secondary(), while For_All_Grid_Abs() is used solely
in add_population(). Note that we have a relative For_All_Grid_From() defined after-
ward and solely used in sched_recv().


#define For_All_Grid(PS, X0, Y0, Z0, X1, Y1, Z1)\
For_Z((fag_zidx=(Z0), fag_x1=GridDesc[PS].x+(X1),\
fag_y1=GridDesc[PS].y+(Y1), fag_z1=GridDesc[PS].z+(Z1),\
fag_w=GridDesc[PS].w, fag_dw=GridDesc[PS].dw,\
fag_gz=Coord_To_Index(X0,Y0,Z0,fag_w,fag_dw)),\


<!-- Page 354 -->

(fag_zidx<fag_z1), (fag_zidx++,fag_gz+=fag_dw))\
For_Y((fag_yidx=(Y0), fag_gy=fag_gz),\
(fag_yidx<fag_y1), (fag_yidx++,fag_gy+=fag_w))\
for (fag_xidx=(X0),fag_gx=fag_gy; fag_xidx<fag_x1; fag_xidx++,fag_gx++)
#define For_All_Grid_Abs(PS, X0, Y0, Z0, X1, Y1, Z1)\
For_Z((fag_zidx=(Z0), fag_x1=(X1), fag_y1=(Y1), fag_z1=(Z1),\
fag_w=GridDesc[PS].w, fag_dw=GridDesc[PS].dw,\
fag_gz=Coord_To_Index(X0,Y0,Z0,fag_w,fag_dw)),\
(fag_zidx<fag_z1), (fag_zidx++,fag_gz+=fag_dw))\
For_Y((fag_yidx=(Y0), fag_gy=fag_gz),\
(fag_yidx<fag_y1), (fag_yidx++,fag_gy+=fag_w))\
for (fag_xidx=(X0),fag_gx=fag_gy; fag_xidx<fag_x1; fag_xidx++,fag_gx++)


The_Grid()  The macro The_Grid() is to use in the body part of For_All_Grid() and its relatives to
Grid_X()  give gidx(x, y, z) stored in fag gx but without referring to the special variable name. The
Grid_Y()  other special variables fag xidx, fag yidx and fag zidx for x, y and z can be also referred
Grid_Z()  to by the macros Grid_X(), Grid_Y() and Grid_Z(). The macro The_Grid() is used in all
functions using For_All_Grid(), For_All_Grid_Abs() or For_All_Grid_From(), while
Grid_X(), Grid_Y() and Grid_Z() are used solely in the macro Sched_Recv_Return() in
sched_recv() which uses its own variation For_All_Grid_From().


#define The_Grid()  (fag_gx)
#define Grid_X()  (fag_xidx)
#define Grid_Y()  (fag_yidx)
#define Grid_Z()  (fag_zidx)



#### 4.10.5 Constants URN_PRI, URN_SEC and URN_TRN

URN_PRI  The last group of macro definitions is for constants of the operation mode given to update_
URN_SEC  real_neighbors(). It updates only RealDstNeighbors[0][0] and RealSrcNeighbors[0][0]
URN_TRN   if its mode argument is URN_PRI = 0 to mean the execution mode changes to primary mode
from other (including undefined) and it is called from init4p() or try_primary4p(). If it
is called from exchange_particles4p() with URN_SEC = 1 to mean helper-helpand recon-
figuration took place but its transitional state is not necessary to be aware of because of
anywhere accommodatation, both of [0][0] and [0][1] are updated but not for [1][]. Other-
wise, i.e., if it is called from make_recv_list() with URN_TRN = 2 to mean we have to be
aware of transitional state of helper-helpand configuration, all array elements are updated.
The contants are referred to by the functions stated above.


#define URN_PRI 0
#define URN_SEC 1
#define URN_TRN 2



#### 4.10.6 oh4p_init() and init4p()

oh4p_init_()  The API functions oh4p_init_() for Fortran and oh4p_init() for C receive a set of
oh4p_init()  array/structure variables through which level-1 to level-4p library functions communicate
with the simulator body, and a few integer parameters to specify the behavior of the library.


<!-- Page 355 -->

The functions have the same argument set as oh3_init[_]() but nphgram is excluded
as discussed in §4.9.3. Therefore the argument addition and modification for the call of
init4p() and setting specBase to 0 or 1 are almost same as those in oh3_init[_]()
discussed in §4.7.3, but init4p() has neither of nphgram, rcounts, scounts, nor skip2.


void
oh4p_init_(int *sdid, const int *nspec, const int *maxfrac, int *totalp,
struct S_particle *pbuf, int *pbase, const int *maxlocalp,
struct S_mycommf *mycomm, int *nbor, int *pcoord, int *sdoms,
int *scoord, const int *nbound, int *bcond, int *bounds,
int *ftypes, int *cfields, int *ctypes, int *fsizes,
const int *stats, const int *repiter, const int *verbose) {
specBase = 1;
init4p(&sdid, *nspec, *maxfrac, &totalp, &pbuf, &pbase, *maxlocalp, NULL,
mycomm, &nbor, pcoord, &sdoms, scoord, *nbound, bcond, &bounds,
ftypes, cfields, -1, ctypes, &fsizes,
*stats, *repiter, *verbose);
}
void
oh4p_init(int **sdid, const int nspec, const int maxfrac, int **totalp,
struct S_particle **pbuf, int **pbase, const int maxlocalp,
void *mycomm, int **nbor, int *pcoord, int **sdoms, int *scoord,
const int nbound, int *bcond, int **bounds, int *ftypes,
int *cfields, int *ctypes, int **fsizes,
const int stats, const int repiter, const int verbose) {
specBase = 0;
init4p(sdid, nspec, maxfrac, totalp, pbuf, pbase, maxlocalp,
(struct S_mycommc*)mycomm, NULL, nbor, pcoord, sdoms, scoord, nbound,
bcond, bounds, ftypes, cfields, 0, ctypes, fsizes,
stats, repiter, verbose);
}


Allocate_NOfPGrid()  Prior to give the definition of init4p(), we have to define the macro Allocate NOfPGrid
(π, h, t, σ, ν) used in the function to allocate and initialize a per-grid histogram, namely
NOfPGrid[2][S][σ] and NOfPGridTotal[2][S][σ]. The macro allocates the body of the per-
grid histogram, pointed by π and having 2·S ·σ elements, by mem_alloc(). It also allocates
a pointer array of [2][S], whose element [p][s] points the element [p][s][gidx(2eg, 2eg, 2eg)]
corresponding to (0, 0, 0) of the body array, by mem_alloc(). The arguments to call mem_
alloc() have element type t and the name ν of the array to be included in the error
message in case of memory shortage. Then it zero-clears all elements in the body array and
makes the pointers h[0] and h[1] points [0][] and [1][] of the pointer array respectively. The
argument t is dint to allocate NOfPGrid[][][] and NOfPGridTotal[][][] in init4p(), but int
for the allocation of NOfPGridOut[][][] done in oh4p_per_grid_histogram(), the sole user
other than init4p().


#define Allocate_NOfPGrid(BODY, NPG, TYPE, SIZE, MSG) {\
const int ns2 = nOfSpecies<<1;\
const int gridsize = SIZE;\
TYPE *npg = BODY;\
TYPE **npgp = (TYPE**)mem_alloc(sizeof(TYPE*), ns2, MSG);\
int s, g, exto=OH_PGRID_EXT<<1;\


<!-- Page 356 -->

const int base = Coord_To_Index(exto, exto, exto,\
GridDesc[0].w, GridDesc[0].dw);\
if (!npg)\
BODY = npg = (TYPE*)mem_alloc(sizeof(TYPE), ns2*gridsize, MSG) + base;\
for (s=0; s<ns2; s++,npg+=gridsize) {\
npgp[s] = npg;\
for (g=0; g<gridsize; g++)  npg[g-base] = 0;\
}\
NPG[0] = npgp;  NPG[1] = npgp + nOfSpecies;\
}


nOfLocalPLimitShadow  Yet another declaration prior to init4p() is for the variable nOfLocalPLimitShadow, which
keeps the return value of oh4p_max_local_particles() or has −1 if it has not been called
prior to oh4p_init[_](). Then init4p() examines this variable to confirm that oh4p_
max_local_particles() has been called and thus gridOverflowLimit = 2Phot has been
defined, and that init4p()’s argument maxlocalp = Plim is not less than the return value
kept in the variable. This variable is private to ohhelp4p.c unlike other global variables,
because it is just used for the communication between init4p() and oh4p_max_local_
particles() and it must have the initial value −1 prior to the execution.

init4p()  The function init4p(), called from oh4p_init[_]() implements the initialization for those
API functions. The arguments of the function are almost same as oh4p_init() but its
mycomm is split into two arguments mycommc and mycommf and there is an additional argu-
ment cfid as discussed in §4.7.3.


static int nOfLocalPLimitShadow = -1;
static void
init4p(int **sdid, const int nspec, const int maxfrac, int **totalp,
struct S_particle **pbuf, int **pbase, int maxlocalp,
struct S_mycommc *mycommc, struct S_mycommf *mycommf,
int **nbor, int *pcoord, int **sdoms, int *scoord,
const int nbound, int *bcond, int **bounds, int *ftypes, int *cfields,
const int cfid, int *ctypes, int **fsizes,
const int stats, const int repiter, const int verbose) {
int nn, me, nnns, nnns2, n;
int (*ft)[OH_FTYPE_N] = (int(*)[OH_FTYPE_N])ftypes;
int *cf = cfields;
int (*ct)[2][OH_CTYPE_N] = (int(*)[2][OH_CTYPE_N])ctypes;
int nf, ne, c, b, size, ps, tr;
int *nphgram = NULL;
int *hsr, *rnbr;
dint *npgdummy = NULL,  *npgtdummy = NULL;
int loggrid;
dint idmax;


We need a few operations prior to call init3() for the initialization of lower level data
structures, because we modify some of them for level-4p extension. At first, we get N by
MPI_Comm_size(), and then allocate TempArray[4][N] by mem_alloc(), whose size is four
times as large as that required in lower level libraries as discussed in §4.9.5. We also allocate
Particles[Plim] and SendBuf[Plim] as a contiguous array of [2Plim] if pbuf points NULL.
Otherwise, what pbuf points is set to the pointer Particles and SendBuf is let point the
head of the second half of pbuf.


<!-- Page 357 -->

MPI_Comm_size(MCW, &nn);  nnns = nn * nspec;  nnns2 = nnns << 1;
TempArray = (int*)mem_alloc(sizeof(int), nn<<2, "TempArray");
if (*pbuf)
Particles = *pbuf;
else
Particles = *pbuf =
(struct S_particle*)mem_alloc(sizeof(struct S_particle),
maxlocalp<<1, "Particles");
SendBuf = Particles + maxlocalp;


Next, we intercept arguments ftypes[][], cfields[] and ctypes[][][][] to add one ele-
ment to each of them for per-grid histogram. At first, we scan ftypes[][] and cfields[]
to find their terminators and thus their numbers of elements which are F −1 and
C −1 respectively.  Then we allocate FieldTypes[F+1][], BoundaryCommFields[C+1]
and BoundaryCommTypes[C][][][] by mem_alloc(), and then copy the elements in the ar-
gument arrays into them by memcpy() for the first and last while by a for-loop for
BoundaryCommFields[] to adjust the indices of FieldTypes[][].
Then we add the last elements of them as follows and as discussed in §4.9.3.

FieldTypes[F−1][] = {1, 0, 0, 0, 0, −eg, eg}
BoundaryCommFields[C−1] = F −1
{
{{−eg, eg, eg}, {0, −2eg, eg}}  b = 0
BoundaryCommTypes[C−1][b][][] =
{{0, 0, 0}, {0, 0, 0}}            b > 0

We also add the terminator FieldTypes[F][0] = −1 and BoundaryCommFields[C] = −1.

for (nf=0; ft[nf][OH_FTYPE_ES]>0; nf++);
for (ne=0; cf[ne]+cfid>=0; ne++);
FieldTypes = (int(*)[OH_FTYPE_N])
mem_alloc(sizeof(int), (nf+2)*OH_FTYPE_N, "FieldTypes");
BoundaryCommFields = cf =
(int(*))mem_alloc(sizeof(int), ne+2, "BoundaryCommFields");
BoundaryCommTypes = (int(*)[2][OH_CTYPE_N])
mem_alloc(sizeof(int), (ne+1)*nbound*2*OH_CTYPE_N,
"BoundaryCommTypes");
memcpy(FieldTypes, ft, sizeof(int)*nf*OH_FTYPE_N);
for (c=0; c<ne; c++)  cf[c] = cfields[c] + cfid;
memcpy(BoundaryCommTypes, ct, sizeof(int)*ne*nbound*2*OH_CTYPE_N);
ft = FieldTypes;  ct = BoundaryCommTypes + ne * nbound;
ft[nf][OH_FTYPE_ES] = 1;
ft[nf][OH_FTYPE_LO] = 0;  ft[nf][OH_FTYPE_UP] = 0;
ft[nf][OH_FTYPE_BL] = 0;  ft[nf][OH_FTYPE_BU] = 0;
ft[nf][OH_FTYPE_RL] = -OH_PGRID_EXT;  ft[nf][OH_FTYPE_RU] = OH_PGRID_EXT;
ft[nf+1][OH_FTYPE_ES] = -1;
cf[ne] = nf;  cf[ne+1] = -1;
ct[0][OH_LOWER][OH_CTYPE_FROM] = -OH_PGRID_EXT;
ct[0][OH_LOWER][OH_CTYPE_TO]   = OH_PGRID_EXT;
ct[0][OH_LOWER][OH_CTYPE_SIZE] = OH_PGRID_EXT;
ct[0][OH_UPPER][OH_CTYPE_FROM] = 0;
ct[0][OH_UPPER][OH_CTYPE_TO]   = -(OH_PGRID_EXT<<1);
ct[0][OH_UPPER][OH_CTYPE_SIZE] = OH_PGRID_EXT;


<!-- Page 358 -->

for (b=1; b<nbound; b++)
ct[b][OH_LOWER][OH_CTYPE_FROM] =
ct[b][OH_LOWER][OH_CTYPE_TO]   =
ct[b][OH_LOWER][OH_CTYPE_SIZE] =
ct[b][OH_UPPER][OH_CTYPE_FROM] =
ct[b][OH_UPPER][OH_CTYPE_TO]   =
ct[b][OH_UPPER][OH_CTYPE_SIZE] = 0;


Now we call init3() passing almost all arguments of init4p() but with the following
exceptions.

- nphgram is the pointer to init4p()’s local variable of the same name which has NULL
to let init3() allocate NOfPLocal[][][], because init4p() does not have the argument.

- rcounts and scounts are NULL because they are unnecessary.

- ftypes,  cfields and ctypes  are  FieldTypes[][],  BoundaryCommFields[] and
BoundaryCommTypes[][][][] respectively, and the arrays themselves are neither allo-
cated nor initialized by init3().

- skip2 is 0 because we need level-2 initialization.

Note that cfid is passed unmodified because we need the original value for initialization
of data structures other than BoundaryCommFields[]. As for its use to scan the terminator
of BoundaryCommFields[], init_fields() ignores it when OH_POS_AWARE is defined.

init3(sdid, nspec, maxfrac, &nphgram, totalp, NULL, NULL, pbuf, pbase,
maxlocalp, mycommc, mycommf, nbor, pcoord, sdoms, scoord, nbound,
bcond, bounds, (int*)ft, cf, cfid, (int*)BoundaryCommTypes, fsizes,
stats, repiter, verbose, 0);


Next, we confirm that nOfLocalPLimitShadow is non-negative and not greater than
maxlocalp = Plim, or in other words oh4p_max_local_particles() has been called and
its return value is passed (possibly after incremented) to maxlocalp.  If not, we stop the
execution by errstop() with an appropriate error message.

if (nOfLocalPLimitShadow<0)
errstop("oh4p_max_local_particles() has to be called before oh4p_init()");
else if (maxlocalp<nOfLocalPLimitShadow)
errstop("argument maxlocalp %d given to oh4p_init() is less than that "
"calculated by oh4p_max_local_particles() %d",
maxlocalp, nOfLocalPLimitShadow);


Next we allocate and initialize per-grid histograms and related variables.  First, we
initialize PbufIndex to be NULL so that the macro Check_Particle_Location() will not
refer to it until the first call of transbound4p() by which the array is allocated and is
given meaningful values. Then, we allocate NOfPGrid[2][S][G] and NOfPGridTotal[2][S][G]
by Allocate_NOfPGrid() after setting GridDesc[0] for the local node’s primary subdomain
by set_grid_descriptor(). Note that the first argument of Allocate_NOfPGrid() is a
dummy pointer variable having NULL to let the macro allocate the body arrays.


<!-- Page 359 -->

Then we check gidx(δmaxx  −1, δmaxy  −1, δmaxz  −1) being the largest possible one-dimen-
sional index of subdomains is small enough to represent it by int when combined with the
largest possible subdomain code. That is, we calculate;

Γ = ⌊log2 gidx(δmaxx  −1, δmaxy  −1, δmaxz  −1)⌋+ 1

and examines if (2(N + 3D) −1) · 2Γ  is not greater than INT_MAX or OH_nid_t is dint.
If this examination fails to mean OH_nid_t is int but not large enough to represent the
combination of the largest possible grid-position index and subdomain code, we stop the
execution by errstop() with an appropriate message suggesting to define OH_BIG_SPACE .
Otherwise, Γ and 2Γ −1 are set into logGrid and gridMask respectively.
Finally, we call adjust_field_descriptor() to modify FieldDesc[F−1].{bc, red}.
size[0] so that collective communications of NOfPGrid[p][][] or NOfPGridTotal[p][][] are
performed on the whole of [S][G] rather than [s][G] with a specific s.

me = myRank;
PbufIndex = NULL;
set_grid_descriptor(0, me);
size = GridDesc[0].dw * GridDesc[0].h;
Allocate_NOfPGrid(npgdummy, NOfPGrid, dint, size, "NOfPGrid");
Allocate_NOfPGrid(npgtdummy, NOfPGridTotal, dint, size, "NOfPGridTotal");

size = Coord_To_Index(Grid[OH_DIM_X].size-1,
If_Dim(OH_DIM_Y, Grid[OH_DIM_Y].size-1, 0),
If_Dim(OH_DIM_Z, Grid[OH_DIM_Z].size-1, 0),
GridDesc[0].w, GridDesc[0].dw);
for (loggrid=0; size; loggrid++,size>>=1);
idmax = (dint)(((nn+OH_NEIGHBORS)<<1)-1)<<loggrid;
if (idmax>INT_MAX && sizeof(OH_nid_t)==sizeof(int)) {
#if OH_DIMENSION==1
errstop("local grid size (%d+%d) times number of nodes %d "
"is too large for OH_nid_t=int and thus OH_BIG_SPACE should be "
"defined.",
GridDesc[0].w-(OH_PGRID_EXT<<2), OH_PGRID_EXT<<2, nn);
#elif OH_DIMENSION==2
errstop("local grid size (%d+%d)*(%d+%d) times number of nodes %d "
"is too large for OH_nid_t=int and thus OH_BIG_SPACE should be "
"defined.",
GridDesc[0].w-(OH_PGRID_EXT<<2), OH_PGRID_EXT<<2,
GridDesc[0].d-(OH_PGRID_EXT<<2), OH_PGRID_EXT<<2, nn);
#else
errstop("local grid size (%d+%d)*(%d+%d)*(%d+%d) times number of nodes %d "
"is too large for OH_nid_t=int and thus OH_BIG_SPACE should be "
"defined.",
GridDesc[0].w-(OH_PGRID_EXT<<2), OH_PGRID_EXT<<2,
GridDesc[0].d-(OH_PGRID_EXT<<2), OH_PGRID_EXT<<2,
GridDesc[0].h-(OH_PGRID_EXT<<2), OH_PGRID_EXT<<2, nn);
#endif
}
logGrid = loggrid;  gridMask = (1 << loggrid) - 1;
adjust_field_descriptor(0);


The next targets of allocation and initialization are data structures for particle transfer
scheduling. We allocate HotSpotList[2N + 2 · 3D + 1], and the body of HSRecv[3D][N][S],


<!-- Page 360 -->

HSSend[S], HSRecvFromParent[S] and HSReceiver[S] by mem_alloc(). For HSRecv[][][], we
initialize the pointer array of [3D] so that each [k] of them points the element [k][0][0] in
the body to have two-dimensional array of [3D][NS] in reality. Then we create MPI data-
type T_Hgramhalf by MPI_Type_vector() and MPI_Type_commit() for a slice [p][∗][m] in
NOfSend[][][] and NOfRecv[][][] as a vector having S elements of MPI_INT with a stride N.
Finally, all elements in NOfSend[][][] are zero-cleared as the base of accumulation of sending
particle counts in the first call of oh4p_transbound().

HotSpotList = (struct S_hotspot*)mem_alloc(sizeof(struct S_hotspot),
2*nn+2*OH_NEIGHBORS+1,
"HotSpotList");
hsr = (int*)mem_alloc(sizeof(int), OH_NEIGHBORS*nn*nspec, "HSRecv");
for (n=0; n<OH_NEIGHBORS; n++,hsr+=nnns)  HSRecv[n] = hsr;
HSSend = (int*)mem_alloc(sizeof(int), nspec*3, "HSSend");
HSRecvFromParent = HSSend + nspec;  HSReceiver = HSRecvFromParent + nspec;
MPI_Type_vector(nspec, 1, nn, MPI_INT, &T_Hgramhalf);
MPI_Type_commit(&T_Hgramhalf);
for (n=0; n<nnns2; n++)  NOfSend[n] = 0;


The next allocation and initialization are for data structures of neighboring information.
First we initiaize FirstNeighbor[k] to k itself if m = SrcNeighbors[k] ≥062, otherwise
the index k′ such that SrcNeighbors[k′] = −(m+1) which is kept in TempArray[−(m+1)].
Then we call update_neighbors() to initialize AbsNeighbors[0][] and GridOffset[0][]
based on the values in Neighbors[0][].
Then we  allocate  RealDstNeighbors[2][2].nbor[N] and  RealSrcNeighbors[2][2].
nbor[N] and call update_real_neighbors() with the code URN_PRI to initialize their el-
ements in [0][0] so that they have subdomain identifiers neighboring to the local node’s
primary subdomain63.

for (n=0; n<OH_NEIGHBORS; n++) {
const int snbr = SrcNeighbors[n];
if (snbr>=0)        FirstNeighbor[n] = TempArray[snbr] = n;
else if (snbr<-nn)  FirstNeighbor[n] = n;
else                FirstNeighbor[n] = TempArray[-(snbr+1)];
}
update_neighbors(0);
rnbr = (int*)mem_alloc(sizeof(int), nn*2*2*2, "RealNeighbors");
for (tr=0; tr<2; tr++)  for (ps=0; ps<2; ps++,rnbr+=nn) {
RealDstNeighbors[tr][ps].n = RealSrcNeighbors[tr][ps].n = 0;
RealDstNeighbors[tr][ps].nbor = rnbr;
RealSrcNeighbors[tr][ps].nbor = rnbr + nn*2*2;
}
update_real_neighbors(URN_PRI, 0, -1, -1);


Finally, we copy the contents of bcond[D][2] to its substance BoundaryCondition[][] by
memcpy(), if SubDomainDesc is NULL to mean regular process coordinate.

if (!SubDomainDesc)
memcpy(BoundaryCondition, bcond, sizeof(int)*OH_DIMENSION*2);
}

62Or if m = −N −1 < −N for neighbors not existing but the value k is never used in this case.
63The second, third and fourth argument of update real neighbors(), dosec, oldp and newp, are mean-
ingless in this call.


<!-- Page 361 -->

#### 4.10.7 oh4p_max_local_particles()

oh4p_max_local_particles_()  The API functions oh4p_max_local_particles_() for Fortran and oh4p_max_local_
oh4p_max_local_particles()  particles() for C calculate Plim being the maximum number of particles a local node
can accommodate and return it to the simulator body calling them. The function takes
the arguments for its level-2 counterpart oh2_max_local_particles() and call it to the
baseline of Plim, and then add 4Phot to Plim where Phot is given through its own last argu-
ment hsthresh to allow a node have 2Phot extra number of particles for each of primary
and secondary particle set above those expected by the load balancing algorithm.  This
2Phot allowance means the last grid-voxel allocated to a node could have up to 2Phot −1
particles because the grid-voxel cannot be split as a hot-spot. Therefore, the function sets
2Phot into gridOverflowLimit. The function also examines Plim is less than the maximum
positive int-type number INT_MAX = 231 −1, and if it finds excess it stops the execution
by mem_alloc_error().  Finally, Plim  is stored into nOfLocalPLimitShadow to indicate
the function is called and for the consistency check in init4p() against its maxlocalp
argument.


int
oh4p_max_local_particles_(const dint *npmax, const int *maxfrac,
const int *minmargin, const int *hsthresh) {
return(oh4p_max_local_particles(*npmax, *maxfrac, *minmargin, *hsthresh));
}
int
oh4p_max_local_particles(const dint npmax, const int maxfrac,
const int minmargin, const int hsthresh) {
const dint npl = (dint)oh2_max_local_particles(npmax, maxfrac, minmargin) +
((gridOverflowLimit = hsthresh<<1)<<1);
const int nplint = npl;

if (npl>INT_MAX) mem_alloc_error("Particles", 0);
return((nOfLocalPLimitShadow=nplint));
}


#### 4.10.8 oh4p_per_grid_histogram()

oh4p_per_grid_histogram_()  The API functions oh4p_per_grid_histogram_() for for Fortran and oh4p_per_grid_
oh4p_per_grid_histogram()  histogram() for C associate the per-grid histogram NOfPGridOut[][][] to that in the simu-
lator body. The Fortran coded simulator must allocate a (D+2)-dimensional array whose
leading D-dimensional size is specified in fsizes[F−1][][] given through the argument of
oh4p_init_(), and give the origin element of the array (0, . . . , 0, 1, 1) through the func-
tion’s sole argument pghgram. On the other hand, C coded simulator may let the function
allocate the array by giving a double pointer to NULL to pghgram, or allocate the array by
itself and give the double pointer to the array’s origin element to pghgram.
The function invokes the macro Allocate_NOfPGrid() to allocate NOfPGridOut[2][S][G]
where;

G = GridDesc[0].dw × GridDesc[0].h = ((δmaxx  + 4eg)(δmaxy  + 4eg)) × (δmaxz  + 4eg)

and returns the pointer to the conceptual element NOfPGridOut[0][0][0] · · · [0] through
*pghgram if it is NULL, to allocate the pointer array for it for the use in library functions,
and to zero-clear its body.


<!-- Page 362 -->

void
oh4p_per_grid_histogram_(int *pghgram) {
oh4p_per_grid_histogram(&pghgram);
}
void
oh4p_per_grid_histogram(int **pghgram) {
Allocate_NOfPGrid(*pghgram, NOfPGridOut, int, GridDesc[0].dw*GridDesc[0].h,
"NOfPGridOut");
}


#### 4.10.9 oh4p_transbound() and transbound4p()

oh4p_transbound_()  The API function oh4p_transbound_() for Fortran and oh4p_transbound() for C provide
oh4p_transbound()  the simulator body calling them with the load-balanced particle transfer mechanism of level-
4p and lower level libraries. The meanings of their two arguments, currmode and stats,
and return value in {−1, 0, 1} are perfectly equivalent to those of the level-1 to level-3
counterparts oh1_transbound[_](), oh2_transbound[_]() and oh3_transbound[_]().
Also similarly to the counterparts, their bodies only have a simple call of transbound4p()
but the third argument level is 4 to indicate the function is called from level-4p API
functions.


int
oh4p_transbound_(int *currmode, int *stats) {
return(transbound4p(*currmode, *stats, 4));
}
int
oh4p_transbound(int currmode, int stats) {
return(transbound4p(currmode, stats, 4));
}


transbound4p()  The function transbound4p(), called from oh4p_transbound[_](), has a code structure
similar to its level-2 counterpart transbound2() especially in its first half. That is, at first it
calls its level-1 counterpart transbound1() to calculate NOfPrimaries[][], TotalPGlobal[],
nOfParticles and nOfLocalPMax from NOfPLocal[][][] of the local node and other nodes,
and to have currmode which indicates not only the current execution mode but also the
accommodation mode, i.e., normal or anywhere. The function also allocates and calculates
TotalP[][] from NOfPLocal[][][] at the first call of it (and thus of transbound4p()) and let
primaryParts and totalParts have the number of particles the local node accommodates,
i.e., the sum of TotalP[][].
Then  it  calls functions  for the heart  of balancing examination  also  similarly to
transbound2() but the functions are level-4p’s own try_primary4p(), try_stable4p()
and rebalance4p().


static int
transbound4p(int currmode, int stats, const int level) {
int ret=MODE_NORM_SEC;
const int nn=nOfNodes, ns=nOfSpecies, nnns2=2*nn*ns;
struct S_particle *tmp;
int i, ps, s, tp;
Decl_For_All_Grid();


<!-- Page 363 -->

stats = stats && statsMode;
currmode = transbound1(currmode, stats, level);

if (try_primary4p(currmode, level, stats))  ret = MODE_NORM_PRI;
else if (!Mode_PS(currmode) || !try_stable4p(currmode, level, stats)) {
rebalance4p(currmode, level, stats);  ret = MODE_REB_SEC;
}

After that, we allocate PbufIndex[2][S] together with an additional element [2][0] so
that the array has 2S + 1 elements, if it is NULL and thus this function has not been called.
Then similarly to transbound2() again, we clear NOfPLocal[][][]; copy TotalPNext[][] to
its substance TotalP[][]; set totalParts and its shadow pointed by totalLocalParticles
to the sum of TotalP[p][s] for all p ∈[0, 1] and s ∈[0, S) to memorize the total number of
particles the local node accommodates at the beginning of the next simulation step, i.e.,
before any injections and removals; and clear InjectedParticles[0][][] = qinj(n)[][] and
nOfInjections = Qinjn  to indicate we have no injected particles at the beginning of the
next simulation step. A difference is in the loop to copy TotalPNext[][] into TotalP[][] in
which we let PbufIndex[p][s] have the head index of pbuf (p, s). In addition, PbufIndex[2][0]
is let have totalParts as the head index of the region following pbuf (1, S−1).

if (!PbufIndex)
PbufIndex = (int*)mem_alloc(sizeof(int), (ns<<1)+1, "PbufIndex");
for (i=0; i<nnns2; i++) NOfPLocal[i] = 0;
for (s=0,tp=0; s<ns*2; s++) {
TotalP[s] = TotalPNext[s];  PbufIndex[s] = tp;  tp += TotalPNext[s];
}
PbufIndex[s] = totalParts = *totalLocalParticles = tp;  nOfInjections = 0;
for (s=0; s<ns*2; s++)  InjectedParticles[s] = 0;


The next part is very level-4p’s own. It zero-clears elements of NOfPGrid[p][s][gidx(x, y,
z)] for p = 0 if the next execution mode is primary or p ∈[0, 1] if secondary, for all s ∈[0, S)
and all (x, y, z) ∈[−keg, δx(m) + keg) × [−keg, δy(m) + keg) × [−keg, δz(m) + keg) where
m = n for p = 0 or m = parent(n) for p = 1 for the local node n, and k = 1 for p = 0
or the helper-helpand tree is kept, or k = 2 otherwise, by For_All_Grid(). Note that
this zero-clear is not only for the subdomain’s cuboid and sending planes to give the base
of the accumulation of particle population for each grid-voxel, but also includes receiving
planes for NOfPGrid[1][][] on helpand-helper reconfiguration.  This inclusion is to ensure
that the sum of each receiving plane in NOfPGridTotal[0][][] given by the reduction of
NOfPGrid[][][] in F(n) has always 0 for all grid-voxels in receiving planes for non-periodic
system boundaries through which no boundary communications are taken. That is, all
receiving planes in NOfPGrid[0][][] are never modified from its initial state with 0 given
by init4p(), but receiving planes in NOfPGrid[1][][] could have non-zero elements on the
reconfiguration because those in old secondary subdomain could have neighbors64.

for (ps=0; ps<=Mode_PS(ret); ps++) {
const int extio = (ps==1 && ret<0) ? OH_PGRID_EXT<<1 : OH_PGRID_EXT;
for (s=0; s<ns; s++) {
dint *npg = NOfPGrid[ps][s];
For_All_Grid(ps, -extio, -extio, -extio, extio, extio, extio)

64Moreover, the receiving planes could be old sending planes if the reconfiguration shrank the subdomain.


<!-- Page 364 -->

npg[The_Grid()] = 0;
}
}

Finally, we exchange the role of Particles[] and SendBuf[] as another level-4p’s own op-
eration, and return to the caller with the return value defined in §4.3.10 for transbound1().
The return value is also set into currMode unless it is negative, i.e., MODE_REB_SEC = −1
which is replaced with MODE_NORM_SEC = 1 because the library does not care the rebalancing
in the last step but will set bit-1 when it finds anywhere accommodation.

tmp = Particles;  Particles = SendBuf;  SendBuf = tmp;
currMode = ret<0 ? -ret : ret;
return(ret);
}


#### 4.10.10 try_primary4p()

try_primary4p()  The function try_primary4p(), called solely from transbound4p(), examines if we can
stay in or turn to primary mode.   If so, the local node gathers all the particles in its
primary subdomain from other nodes and sort them according to their grid-position. The
function has three arguments currmode, level and stats whose meanings are perfectly
equivalent to those of its level-1 counterpart try_primary1().
First we call the level-1 counterpart try_primary1() to examine if the next execution
mode is primary.  If not, we simply return to its caller transbound4p() with the return
value FALSE to indicate the mode will be secondary. Otherwise, i.e., we will be in primary
mode, at first we call update_real_neighbors() with the operation code URN_PRI to
reinitialize the elements RealDstNeighbors[0][0] and RealSrcNeighbors[0][0] so that they
have subdomain identifiers neighboring to the local node’s primary subdomains65,  if we
were in secondary mode.


static int
try_primary4p(const int currmode, const int level, const int stats) {
const int nn=nOfNodes, ns=nOfSpecies, nnns=nn*ns, me=myRank;
const int oldp = RegionId[1];
int i, s, nsend, *np;
dint ***npg = Mode_PS(currmode) ? NOfPGridTotal : NOfPGrid;

if (!try_primary1(currmode, level, stats)) return(FALSE);
if (Mode_PS(currmode))  update_real_neighbors(URN_PRI, 0, -1, -1);

Next, if we have anywhere accommodataion, we perform position-aware particle transfer
as follows.  First we perform non-position-aware transfer by calling move_to_sendbuf_
primary() and exchange_primary_particles() to have all particles to be accommodated
by the local node in its primary subdomain. Then we call count_population() to have the
complete per-grid histogram in NOfPGrid[0][][] only for the primary subdomain by telling
it to the function through the argument psnew = 066.  Finally, based on the per-grid
histogram, we sort particles by sort_particles() telling it that the per-grid histogram is
in NOfPGrid[][][] which should be copied into NOfPGridOut[][][] (nextmode = 0), particles

65The second, third and fourth argument of update real neighbors(), dosec, oldp and newp, are mean-
ingless in this call.
66The first argument nextmode = 0 is only for the statistics and thus meaningless in this call.


<!-- Page 365 -->

to be sorted are primary only (psnew = 0), and execution time measurement has already
started in count_population() even if specified (stats = 0).

if (Mode_Acc(currmode)) {
move_to_sendbuf_primary(Mode_PS(currmode), stats);
exchange_primary_particles(currmode, stats);
count_population(0, 0, stats);
sort_particles(NOfPGrid, 0, 0, 0);

If the accommodation  is normal, on the other hand, we at first  call exchange_
population() to gather the per-grid histograms in neighbors’ sending planes, possibly
with reduction of those in helpers of the local node and the neighbors, to have the complete
per-grid histogram in NOfPGrid[0][][] or NOfPGridTotal[0][][] just depending on the current
execution mode currmode telling it to the function by nextmode = 0. Then we sum up the
number of particle to be sent to other nodes, namely P nsend , as follows.

S−1∑ N−1∑              S−1∑ N−1∑
P nsend =          q(n)[0][s][m] +         q(n)[1][s][m]
s=0 m=0               s=0 m=0
m̸=n
∑1 S−1∑ N−1∑
=             q(n)[p][s][m] −q(n)[0][s][n]

p=0 s=0 m=0
∑1 S−1∑ N−1∑
=            NOfPLocal[p][s][m] −NOfPLocal[0][s][n]

p=0 s=0 m=0

Note that the accidental particle travels from the local node’s secondary subdomain to
primary subdomain are considered as sending them as in lower level libraries.
Then, if Pn + P nsend = TotalPGlobal[n] + Pnsend > Plim = nOfLocalPLimit to mean
that we cannot move all particles in Particles[] to SendBuf[] with sorting, we perform
non-position-aware transfer by move_to_sendbuf_primary() and exchange_primary_
particles() to have all primary particles to be accommodated by the local node in
Particles[], and then sort them by sort_particles() telling it that the per-grid his-
togram is in NOfPGrid[][][] if we were in primary mode or NOfPGridTotal[][][] otherwise,
the histogram should be copied into NOfPGridOut[][][] (nextmode = 0), and particles to be
sorted are primary only (psnew = 0).
Otherwise,  i.e., Pn + P nsend ≤Plim, we move particles in Particles[] to SendBuf[]
sorting those staying in the primary subdomain by move_and_sort_primary() giving it
NOfPGrid[][][] or NOfPGridTotal[][][] depending on whether we are in primary mode and
telling it whether it needs to scan secondary particles (parent(n) ≥0 in the last step) or
not. Then, after letting SendBuf points SendBuf[Pn] for sbuf (0, 0) temporarily, we send
particles in sbuf (s, m) and receive particles to rbuf (0, s) for all s ∈[0, S) and m ∈[0, N)
by exchange_primary_particles().  Finally, we move received particles in rbuf (0, s) to
SendBuf[] sorting them by sort_received_particles() telling  it that particles to be
sorted are primary only (psnew = 0)67.

} else {
exchange_population(currmode, 0);
for (s=0,nsend=0,np=NOfPLocal; s<ns; s++,np+=nn) {
for (i=0; i<nn; i++)  nsend += np[i] + np[i+nnns];

67The first argument nextmode = 0 is only for the statistics.


<!-- Page 366 -->

nsend -= np[me];
}
if (TotalPGlobal[me]+nsend>(dint)nOfLocalPLimit) {
move_to_sendbuf_primary(Mode_PS(currmode), stats);
exchange_primary_particles(currmode, stats);
sort_particles(npg, 0, 0, stats);
} else {
struct S_particle *sbuf=SendBuf;
move_and_sort_primary(npg, (oldp>=0 ? 1 : 0), stats);
SendBuf += TotalPGlobal[me];
exchange_primary_particles(currmode, stats);
SendBuf = sbuf;
sort_received_particles(0, 0, stats);
}
}

Finally we finish this function and return to transbound4p() with TRUE to tell it we will
be in primary mode in the next simulation step, after setting primaryParts and its shadow
pointed by secondaryBase to the total number of particles the local node n accommodates,
i.e., the number of particles in the primary subdomain, TotalPGlobal[n].

primaryParts = *secondaryBase = TotalPGlobal[me];
return(TRUE);
}


#### 4.10.11 try_stable4p()

try_stable4p()  The function try_stable4p(), solely called from transbound4p(), examines if the current
helpand-helper configuration sustains the particle movements crossing subdomain bound-
aries which can bring intolerable load imbalance. The examination is done by calling its
level-1 counterpart try_stable1() simply passing all the arguments of the function itself
to the counterpart if we have anywhere accommodation, because the meanings of them are
perfectly equivalent to those of the counterpart. Otherwise, i.e., with normal accommoda-
tion, we pass the level argument making it negative to keep try_stable1() from making
particle transfer schedule which we will build in exchange_particles4p().
If the examination passes, we perform an all-to-all type particle transfer by calling
exchange_particles4p() with arguments of the function itself. In addition to them, the
third argument reb = 0 tells it that helpand-helper configuration is kept, and the fourth
and fifth arguments oldp = newp = RegionId[1] show the both of old and new helpand
of the local node n are parent(n). Then the function returns to transbound4p() with the
return value of TRUE.


static int
try_stable4p(const int currmode, const int level, const int stats) {
if (!try_stable1(currmode, (Mode_Acc(currmode) ? level : -level), stats))
return(FALSE);
exchange_particles4p(currmode, level, 0, RegionId[1], RegionId[1], stats);
return(TRUE);
}


<!-- Page 367 -->

#### 4.10.12 rebalance4p()

rebalance4p()  The function rebalance4p(), solely called from transbound4p(), builds the new family
tree to rebalance the load among nodes by calling its level-1 counterpart rebalance1()
simply passing all the arguments of the function itself to the counterpart if we have anywhere
accommodation, because the meanings of them are perfectly equivalent to those of the
counterpart.  Otherwise, i.e., with normal accommodation, we pass the level argument
making it negative to keep rebalance1() from making particle transfer schedule which we
will build in exchange_particles4p(), and from building new communicators for the new
tree because we still need the old ones.
Then,  before the  particle  transfer by exchange_particles4p(),  it modifies  ele-
ments of InjectedParticles[0][1][s] for  all s ∈[0, S),  if some particles are injected
(nOfInjections =  Qinjn >  0), we have anywhere accommodation, and old and new
parent(n) for the local node n are different, because we have to take care the secondary
particles injected into new secondary subdomain accidentally. That is,  if we have any-
where accommodation, we will at first perform non-position-aware particle transfer in
which secondary particles accidentally in the new secondary subdomain are considered
to be staying. On the other hand, InjectedParticles[0][1][s] has the number of sec-
ondary particles injected into the old secondary subdomain.  Therefore, we have to scan
all injected particles to let InjectedParticles[0][1][s] have the number of secondary par-
ticles (Secondary_Injected() is true) in the new secondary subdomain so that move_
to_sendbuf_secondary() picks some of them and let them stay in pbuf (1, s). Note that
we use Primarize_Id() to get the subdomain identifier of each secondary injected particle
but immediately recover its original form by Secondarize_Id() so that move_to_sendbuf_
secondary() correctly decode its subdomain code68. Also note that this operation is level-
4p specific because in lower levels particles injected into a subdomain other than the old
secondary one are considered primary.
Another remark is that the old parent(n) is obtained from RegionId[1] before the call
of rebalance1() which may update the element. Further, the new parent(n) is obtained
from NodesNext[n] in the new family tree if we have normal accommodation, while we have
to refer to Nodes[n] with anywhere accommodation because rebalance1() has exchanged
Nodes[] and NodesNext[] to let the former has the new tree.
Then we call exchange_particles4p() with arguments of this function itself. In addi-
tion to them, the third argument reb = 1 tells it helpand-helper reconfiguration, and the
fourth and fifth arguments oldp and newp have the old and new parent(n). Then after
the call, we do the followings for (potentially) new helpand and secondary subdomain as-
signed to the local node by rebalancing, if we had normal accommodation; call set_grid_
descriptor() to update GridDesc[1][] for the secondary subdomain; move Neighbors[2][k]
for the neighbors of the new parent(n), which were temporalily stored to keep the old par-
ent’s neighbors in transitional state of helpand-helper reconfiguration, into Neighbors[1][k]
for all k ∈[0, 3D) as the stable state information; and finally call update_neighbors()
telling it to update elements in AbsNeighbors[1][] and GridOffset[1][] for the secondary
subdomain. Note that the field descriptors for the secondary subdomain has already been
updated in make_recv_list(). Also note that, if we had anywhere accommodation, these
operations have been performed by exchange_particles4p().


static void
rebalance4p(const int currmode, const int level, const int stats) {

68Of course we can examine the subdomain identifier in a non-destructive manner but to design a specific
macro for it is tiresome.


<!-- Page 368 -->

const int me=myRank, ns=nOfSpecies;
const int oldp = RegionId[1],  amode = Mode_Acc(currmode);
const int ninj = nOfInjections;
int s, n, newp;

rebalance1(currmode, (amode ? level : -level), stats);
newp = amode ? Nodes[me].parentid : NodesNext[me].parentid;
if (ninj && amode && oldp!=newp) {
int *sinj = InjectedParticles + ns;
const int sbase=specBase;
int i;
struct S_particle *p;
Decl_Grid_Info();
for (s=0; s<ns; s++)  sinj[s] = 0;
if (newp>=0) {
for (i=0,p=Particles+totalParts; i<ninj; i++,p++) {
const OH_nid_t nid = p->nid;
int sdid;
if (Secondary_Injected(nid)) {
Primarize_Id(p, sdid);  Secondarize_Id(p);
if (sdid==newp)  sinj[Particle_Spec(p->spec-sbase)]++;
}
}
}
}
exchange_particles4p(currmode, level, 1, oldp, newp, stats);
if (!amode) {
set_grid_descriptor(1, newp);
for (n=0; n<OH_NEIGHBORS; n++)  Neighbors[1][n] = Neighbors[2][n];
update_neighbors(1);
}
}


4.10.13  Macros Parent_Old(), Parent_New(), Parent_New_Same() and
Parent_New_Diff()

Parent_Old()  Before giving the definition of the function exchange_particles4p(), we define four
Parent_New()  macros to examine the statuses of old and (possibly) new parents of the local node n
Parent_New_Same()  in the last and next simulation steps encoded in the least significant 3 bits of the function’s
Parent_New_Diff()  local variable pcode = π as follows.

- Bit-2 is 1 and thus Parent Old(π) is true iffthe old parent(n) ≥0 meaning the old
parent exists.

- Bit-1 is 1 and thus Parent New(π) is true iffthe new parent(n) ≥0 meaning the new
parent exists.

- Bit-0 is 1 iffthe old and new parent(n) is equivalent. Therefore, Parent New Same(π)
is true iffthe bit-1 and bit-0 are 1 to mean that the local node has the new parent
and unmodified. On the other hand, Parent New Diff(π) is true iffthe bit-1 is 1 but
bit-0 is 0 to mean that the local node has the really new parent.

The macros are used in exchange_particles4p() and its callees; Parent_Old() and
Parent_New_Diff() in make_send_sched(), gather_hspot_send() and scatter_hspot_


<!-- Page 369 -->

recv(), while Parent_New() solely in exchange_particles4p() and Parent_New_Same()
solely in make_send_sched().


#define Parent_Old(PCODE)       ((PCODE) & 4)
#define Parent_New(PCODE)       ((PCODE) & 2)
#define Parent_New_Same(PCODE)  (((PCODE) & 3) == 3)
#define Parent_New_Diff(PCODE)  (((PCODE) & 3) == 2)


#### 4.10.14 exchange_particles4p()

exchange_particles4p()  The function exchange_particles4p(), called from try_stable4p() and rebalance4p(),
performs an all-to-all type position-aware particle transfer when we will be in secondary
mode in the next simulation step.  The function is given arguments currmode, level
and stats whose meanings are same as those of the callers, reb being 0  if called from
try_stable4p() while 1 if from rebalance4p(), and oldp and newp are the local node’s
parent(n) in the last and next simulation step respectively.
At first in the variable declaration part, the function determines whether we have to
take care of the transitional state of helpand-helper configuration, i.e., whetehr we have
normal accommodation and rebalancing took place, and let its local variable trans be true
iffso. It also sets the parent status code discussed in §4.10.13 according to the arguments
oldp and newp.


static void
exchange_particles4p(const int currmode, const int level, int reb,
int oldp, const int newp, const int stats) {
const int trans = !Mode_Acc(currmode) && reb ? 1 : 0;
int pcode =
(oldp>=0 ? 4 : 0) + (newp>=0 ? 2 : 0) + (oldp==newp ? 1 : 0);
int psold, psnew;
int nacc[2]={0,0}, nsend=0;
struct S_commlist *hslist;


If we have anywhere accommodation, try_stable1() called from try_stable4p() or
rebalance1() called from rebalance4p() built particle transfer schdule in CommList[] and
set SecRList, SecRLSize and SLHeadTail[] to appropriate values by schedule_particle_
exchange() as done in lower level non-position-aware particle transfer.  Therefore, we
can call exchange_particles() as we do in try_stable2() or rebalance2() with any-
where accommodation69, to have primary and secondary particles of the local node without
position-aware manner. Then if rebalanced, we call the followings for the (potentially) new
parent(n) assigned to the local node by rebalancing; update_descriptors() giving old and
new parent(n) to update elements in FieldDesc[] for the new parent(n) and to reinitial-
ize BorderExc[][1][][] for old parent(n); set_grid_descriptor() to update GridDesc[1] for
the new parent(n); update_neighbors() to update AbsNeighbors[1][] and GridOffset[1][];
and update_real_neighbors() with the operation code URN_SEC and the new parent(n)
to update RealDstNeighbors[0][p] and RealSrcNeighbors[0][p] for p ∈[0, 1] to reflect the
new helpand-helper configuration. Note that the second argument dosec = 0 of update_

69The third argument oldparent is the oldp argument of this function, but it is not referred to in
exchange particles when neighboring is 0 to mean anywhere accommodation.


<!-- Page 370 -->

real_neighbors() does not have any effect because it is meaningful when the operation
code is URN_TRN.
Then we reinitialize NOfSend[][][] by zero-clearing its all elements as the base of count-
ing in callee functions of make_send_sched(), because make_comm_count() called from
try_stable1() or rebalance1() let it have the number of sending particles in the non-
position-aware particle transfer so that exchange_particles() refers to it in the all-to-all
communication for anywhere accommodation.  After that, we call count_population()
to have local per-grid histogram in NOfPGrid[p][][] telling it to do for both p ∈[0, 1]  if
the local node has new parent(n) (psnew = 1), and then reduce_population() with the
function pointer to mpi_allreduce_wrapper() to have the complete per-grid histogram in
NOfPGridTotal[p][][] for p ∈[0, 1], i.e., not only for the histogram of primary subdomain
but also of secondary one by all-reduce communication. We also let reb be 0 because we
do not have to take care the helpand-helper reconfiguration and thus make old and new
parent same letting pcode have the corresponding code just depending on the existance of
the new parent.
On the other hand, if we have normal accommodation, we call exchange_population()
forcing it to bulid the complete per-grid histogram in NOfPGridTotal[0][][] regardless the
current execution mode currmode.

if (Mode_Acc(currmode)) {
int i;
const int nnns2 = nOfNodes * nOfSpecies * 2;
if (reb) {
exchange_particles(SecRList, SecRLSize, oldp, 0, currmode, stats);
update_descriptors(oldp, newp);
set_grid_descriptor(1, newp);
update_neighbors(1);
update_real_neighbors(URN_SEC, 0, -1, newp);
}
else
exchange_particles(CommList+SLHeadTail[1], SecSLHeadTail[0], oldp, 0,
currmode, stats);
for (i=0; i<nnns2; i++)  NOfSend[i] = 0;
count_population(1, (Parent_New(pcode) ? 1 : 0), 0);
reduce_population(mpi_allreduce_wrapper);
reb = 0;  oldp = newp;  pcode = newp>=0 ? 7 : 0;
} else
exchange_population(currmode, 1);


Now, regardless of the accommodation mode, we have the complete per-grid histogram
in NOfPGridTotal[0][][] and particles in Particles[] which will stay in the local node’s
primary or secondary subdomain or travel to one of their neighbors. Therefore with this
common setting, we call make_recv_list() with arguments currmode, level, (possibly
modified) reb and stats, together with the old and (possibly diffenet) new parents to build
the receiver-side particle transfer schudule in CommList[] and to obtain its tail and thus the
head of hot-spot sending block as the return value. The head is passed to the next call of
make_send_sched() with other arguments including the parent status code pcode. The
function builds the per-receiver sending histogram in NOfSend[][][] and gives us the number
of primary/secondary particles to be accommodated by the local node in the local array
nacc[2] and the number of sending particles P nsend in the local variable nsend. Finally we
exchange NOfSend[][][] by a hand-made all-to-all communicaion in neighboring families to


<!-- Page 371 -->

have NOfRecv[p][][] for p ∈{0, 1} if the new parent exists or only for p = 0 otherwise, by
exchange_xfer_amount() which takes care of the transitional helpand-helper configuration
if trans is true.

psold = Parent_Old(pcode) ? 1 : 0;
psnew = Parent_New(pcode) ? 1 : 0;
hslist = make_recv_list(currmode, level, reb, oldp, newp, stats);
make_send_sched(currmode, reb, pcode, oldp, newp, hslist, nacc, &nsend);
exchange_xfer_amount(trans, psnew);

Now we start position-aware particle transfer. If Qn+P nsend > Plim = nOfLocalPLimit,
where Qn = nacc[0] + nacc[1], to mean we cannot move all particles in Particles[] to
SendBuf[] with sorting, we perform a partially position-aware transfer only takeing care of
the node to accommodate particles in each grid-voxel. This is done by move_to_sendbuf_
sec4p() being the level-4p version of move_to_sendbuf_secondary(), to which we give an
argument psold to indicate whether the local node’s old parent exists and thus it should
take care secondary particles. Then we call xfer_particles(), whose second argument
psnew is true iffthe local node’s new parent exists and thus we need to receive secondary
particles, and third argument sbuf = SendBuf means sbuf (0, 0, 0) is located at the head
of SendBuf[] as usual, to have particles to accommodate in Particles[]. Finally, we move
them with sorting to SendBuf[] by sort_particles() telling it the per-grid histogram
is in NOfPGridOut[][][] and,  if the local node’s new parent exists (newp = 1), both of
primary and secondary particles have to be sorted referring to GridDesc[1] or GridDesc[2]
depending on whether helpand-helper configuration is stable (nextmode = 1) or transitional
(nextmode = 2). Note that the first argument of sort_particles() is NULL but not referred
to in the function.
Otherwise, i.e., Qn+Pnsend ≤Plim, we move particles in Particles[] to SendBuf[] sorting
those staying in the primary/secondary subdomains by move_and_sort_secondary(). Its
argument psold indicates whether it should scan old secondary particles, while psnew in-
dicates whether NOfPGridOut[1][][], NOfPGridTotal[1][][] and RecvBufBases[1][] should be
built for new secondary particles. Then we transfer particles by xfer_particles() telling
it that sbuf (0, 0, 0) is at SendBuf[Qn]. Finally, we move received particles in rbuf (p, s) to
SendBuf[] sorting them by sort_received_particles() telling it that it should sort for
both of p ∈[0, 1] (psnew = 1) or only for p = 0 (psnew = 0).

if ((dint)nacc[0]+(dint)nacc[1]+nsend>(dint)nOfLocalPLimit) {
move_to_sendbuf_sec4p(psold, trans, oldp, nacc, nsend, stats);
xfer_particles(trans, psnew, SendBuf);
sort_particles(NULL, trans+1, psnew, stats);
} else {
move_and_sort_secondary(psold, psnew, trans, oldp, nacc, stats);
xfer_particles(trans, psnew, SendBuf+nacc[0]+nacc[1]);
sort_received_particles(1, psnew, stats);
}
}


#### 4.10.15 exchange_population()

exchange_population()  The function exchange_population(),  called from try_primary4p() and exchange_
particles4p() when we have normal accomodation, sums up local per-grid histograms
in the local node’s primary family  if we were in secondary mode, and then gathers the


<!-- Page 372 -->

per-grid histograms in neighbors’ sending planes to have the complete per-grid histogram
in NOfPGrid[0][][] if we were and will be in primary mode, or in NOfPGridTotal[0][][] oth-
erwise. The execution mode of the last step is given by the argument currmode while that
of the next step is given by nextmode.


static void
exchange_population(const int currmode, const int nextmode) {
const int ns=nOfSpecies;
int s;
dint **npg = NOfPGrid[0];
const int ct=nOfExc-1;
const int exti = OH_PGRID_EXT,  exto = exti<<1;
const int x = GridDesc[0].x,  y = GridDesc[0].y,  z = GridDesc[0].z;
const int w = GridDesc[0].w,  dw = GridDesc[0].dw;
Decl_For_All_Grid();


At  first,   if we were  in secondary mode, we sum up  local  per-grid histograms
NOfPGrid[p][][] in the local node’s primary family, where p = 0 for the local node while
p = 1 for its helpers, to have the sum in NOfPGridTotal[0][][] by reduce_population()
and its argument function MPI_Reduce(). On the other hand, if we were in primary mode
but will be in secondary mode, we copy elements NOfPGrid[0][s][gidx(x, y, z)] to the cor-
resonding elements in NOfPGridTotal[][][] using For_All_Grid() for all s ∈[0, S) and
(x, y, z) ∈[−eg, δx(n)+eg) × [−eg, δy(n)+eg) × [−eg, δz(n)+eg) for the local node n, be-
cause we need to keep NOfPGrid[][][] unchanged for the secondary mode particle transfer.
Therefore, if we were or will be in secondary mode, the base per-grid histogram is built in
NOfPGridTotal[0][][], while untouched NOfPGrid[0][][] is used as the base otherwise.

if (Mode_PS(currmode)) {
reduce_population(MPI_Reduce);  npg = NOfPGridTotal[0];
} else if (nextmode) {
npg = NOfPGridTotal[0];
for (s=0; s<ns; s++) {
dint *npgs = NOfPGrid[0][s],  *npgt = npg[s];
For_All_Grid(0, -exti, -exti, -exti, exti, exti, exti)
npgt[The_Grid()] = npgs[The_Grid()];
}
}

Now, for each s ∈[0, S), we gather sending planes of all 2D neighbors to the local
node’s receiving planes by oh3_exchange_borders() giving it the base per-grid histogram
NOfPGrid[0][s][] or NOfPGridTotal[0][s][] and C−1 being the entry for per-grid histograms
in BorderExc[][][][]. Its second argument for the secondary subdomain’s array is NULL be-
cause we don’t broadcast the receiving planes to the helpers as indicated its forth argument
bcast = 0.
Then we add each of 2D receiving planes to each boundary plane(s) of d-th dimensional
from d = D −1 to 0 to have the complete per-grid histogram. The addition is performed
by a series of calls of add_population() for each receiving/boundary plane pair. More
specifically, the d-th dimensional boundary plane(s) to which we add receiving plane(s) is


<!-- Page 373 -->

specified as [βl0, βu0 ) × · · · × [βlD−1, βuD−1) where [βlk, βuk ) is specified as follows.

[−2eg, δk(n) + 2eg)  k < d                        
[0, eg)             k = d and lower
[βlk, βuk ) =
 [δk(n) −eg, δk(n))   k = d and upper
[0, δk(n))          k > d

The function is given the arguments for the base per-grid histogram, the lower and upper
bound of the boundary plane(s) in each axis shown above, and the offset of the receiving
plane(s) from boundary plane(s) namely;

d−1∏
gidx(β0, · · · , βd ± 2eg, · · · , βD−1) −gidx(β0, · · · , βd, · · · , βD−1) = ±2eg    (δmaxk  + 4eg)
k=0

where −/+ for lower/upper boundary. The method for the addition is same as that we
used in the sample code’s function add_boundary_current() shown in §3.13.

for (s=0; s<ns; s++) {
oh3_exchange_borders(npg[s], NULL, ct, 0);
if (OH_DIMENSION>OH_DIM_Z) {
add_population(npg[s], -exto, x+exto, -exto, y+exto, 0, exti, -dw*exto);
add_population(npg[s], -exto, x+exto, -exto, y+exto, z-exti, z, dw*exto);
}
if (OH_DIMENSION>OH_DIM_Y) {
add_population(npg[s], -exto, x+exto, 0, exti, 0, z, -w*exto);
add_population(npg[s], -exto, x+exto, y-exti, y, 0, z, w*exto);
}
add_population(npg[s], 0, exti, 0, y, 0, z, -exto);
add_population(npg[s], x-exti, x, 0, y, 0, z, exto);
}
}


#### 4.10.16 add_population()

add_population()  The function add_population(), called solely from exchange_population() but 2DS
times, adds the elements in a receiving plane (set) of per-grid histogram, specified by its
argument npd being NOfPGrid[0][s][] or NOfPGridTotal[0][s][], to the boundary plane (set)
specified by arguments as [xl, xu) × [yl, yu) × [zl, zu), whose values are shown in §4.10.15.
The location of the receiving plane (set) is specified as the distance between corresponding
∏d−1
elements in receiving/boundary plane (sets) by the argument src being ±2eg  k=0(δmaxk  +
4eg) as discussed in §4.10.15.  The function simply performs the addition for specified
elements by For_All_Grid_Abs().


static void
add_population(dint *npd, const int xl, const int xu, const int yl,
const int yu, const int zl, const int zu,
const int src) {
dint *nps=npd+src;
Decl_For_All_Grid();

For_All_Grid_Abs(0, xl, yl, zl, xu, yu, zu)
npd[The_Grid()] += nps[The_Grid()];
}


<!-- Page 374 -->

#### 4.10.17 mpi_allreduce_wrapper()

mpi_allreduce_wrapper()  The function mpi_allreduce_wrapper(), appearing solely in exchange_particles4p() as
the argument of reduce_population(), accepts the same argument set as MPI_Reduce()
and calls MPI_Allreduce() passing all arguments except for root in the set of MPI_
Reduce() but not in that of MPI_Allreduce(). This function is just for using reduce_
population() for all-reduce communication rather than simple non-all type reduction.


static int
mpi_allreduce_wrapper(void *sendbuf, void *recvbuf, int count,
MPI_Datatype datatype, MPI_Op op, int root,
MPI_Comm comm) {
return(MPI_Allreduce(sendbuf, recvbuf, count, datatype, op, comm));
}


#### 4.10.18 reduce_population()

reduce_population()  The function reduce_population(), called from exchange_particles4p() and exchange_
population(), receives its sole argument mpired being mpi_allreduce_wrapper() for the
former and MPI_Reduce() for the latter, and performs (all-)reduce communication using
it in primary (p = 0) and secondary (p = 1) family members to sum up NOfPGrid[p][][] to
have the sum in NOfPGridTotal[0][][] and, if all-reducing, NOfPGridTotal[1][][].
The function is almost equivalent to oh1_all_reduce() and oh1_reduce() but we have
to have this variation because the source array NOfPGrid[][][] must be kept unchanged rather
than being overwritten it by MPI_IN_PLACE option that level-1 relatives specify. Therefore, if
the prime element of MyComm is MPI_COMM_NULL to mean that the local node has no children
and thus (all-)reduce operation is not performed, we have to copy NOfPGrid[0][][] into
NOfPGridTotal[0][][] explicitly by memcpy(). The base index and the number of elements
to be (all-)reduced are specified in FieldDesc[F−1].red.base and its element size[p] for
the per-grid histogram.


static void
reduce_population(int (*mpired)(void*, void*, int, MPI_Datatype, MPI_Op, int,
MPI_Comm)) {
const int ft=nOfFields-1;
const int base = FieldDesc[ft].red.base;
const int *size = FieldDesc[ft].red.size;

if (MyComm->black) {
if (MyComm->prime!=MPI_COMM_NULL)
mpired(NOfPGrid[0][0]+base, NOfPGridTotal[0][0]+base, size[0],
MPI_LONG_LONG_INT, MPI_SUM, MyComm->rank, MyComm->prime);
if (MyComm->sec!=MPI_COMM_NULL)
mpired(NOfPGrid[1][0]+base, NOfPGridTotal[1][0]+base, size[1],
MPI_LONG_LONG_INT, MPI_SUM, MyComm->root, MyComm->sec);
} else {
if (MyComm->sec!=MPI_COMM_NULL)
mpired(NOfPGrid[1][0]+base, NOfPGridTotal[1][0]+base, size[1],
MPI_LONG_LONG_INT, MPI_SUM, MyComm->root, MyComm->sec);
if (MyComm->prime!=MPI_COMM_NULL)
mpired(NOfPGrid[0][0]+base, NOfPGridTotal[0][0]+base, size[0],
