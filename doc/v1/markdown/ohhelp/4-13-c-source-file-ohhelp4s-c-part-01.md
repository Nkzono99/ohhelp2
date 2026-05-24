# 4.13 C Source File ohhelp4s.c - Part 1

Source: `doc/v1/original/ohhelp.pdf`, pages 462-493.

<!-- Page 462 -->

## 4.13 C Source File ohhelp4s.c

#### 4.13.1 Header File Inclusion

The first job done in ohhelp4s.c is the inclusion of the header files ohhelp1.h, ohhelp2.h,
ohhelp3.h and ohhelp4s.h. Before the inclusion of ohhelp1.h, ohhelp2.h and ohhelp3.h, we
#define the macro EXTERN as extern so as to make variables declared in the files external,
but after that we make it #undef’iend and then #define it as empty so as to provide
variables declared in ohhelp4p.h with their homes, as discussed in §4.2.3.

#define EXTERN extern
#include "ohhelp1.h"
#include "ohhelp2.h"
#include "ohhelp3.h"
#undef  EXTERN
#define EXTERN
#include "ohhelp4s.h"



#### 4.13.2 Function Prototypes

The next and last job to do prior to macro and function definitions is to declare the
prototypes of the following functions private for the level-4s library. Note that the marks
“[E]”, “[M]” and “[N]” in the list are for the indications as same as those in §4.12.7.

- The function init4s() [M] is the body of oh4s_init().

- The function transbound4s() [M] is the body of oh4s_transbound().

- The function try_primary4s() [M] performs position-aware particle transfer in pri-
mary mode after calling its level-1 couterpart try_primary1() to check if we will be
in primary mode in the next step.

- The function try_stable4s() [M] performs position-aware particle transfer in sec-
ondary mode after calling its level-1 couterpart try_stable1() to check if we can
keep the helpand-helper configuration.

- The function rebalance4s() [M] performs position-aware particle transfer in sec-
ondary mode after calling its level-1 counterpart rebalance1() to establish a new
helpand-helper configuration.

- The function exchange_particles4s() [M] is the core of position-aware particle
transfer in both primary and secondary modes.

- The function count_population() [E] accumulates the number of particles in each
grid-voxel in primary and secondary subdomains to have the local per-grid histogram.

- The function exchange_population() [M] gathers particle population in each grid-
voxel to build per-grid histogram.

- The function reduce_population() [M] sums per-grid histograms of the family mem-
bers.

- The function add_population() [E] adds particle populatoin in each grid-voxel in
receiving planes to that in boundary planes.


<!-- Page 463 -->

- The function make_recv_list() [M] scans per-plane histogram to build primary
receiving block, and then exchanges the block between neighbors to have primary
sending block and broadcast them for secondary receiving/sending and alternative
secondary receiving/sending blocks for helpers.

- The function sched_recv() [M] scans per-plane histogram to determine the sub-
cuboid for each node.

- The function make_send_sched() [M] scans primary receiving/sending, secondary re-
ceiving/sending and alternative secondary receiving/sending blocks to determine the
node to which the local node sends the particles which the node currently accommo-
dates, the head index of SendBuf[] for particles in each grid-voxel and in the next
step, and the transfer schedulde of particles in horizontal halo planes.

- The function make_send_sched_body() [M] scans a primary receiving block created
by the local node itself, or that given from a neighbor, the helpand or a neighbor of the
helpand to determine the node which accommodates the particles in each xy-plane.

- The function make_send_sched_self() [N] scans the primary receiving or secondary
receiving block to determine the particle population in each grid-voxel in the local
node’s primary and secondary subcuboids, and the transfer schedule of halo particles
in horizontal halo planes.

- The function make_send_sched_hplane() [N] scans grid-voxels in a xy-plane in the
local node’s subcuboid in the next step to have PO(p, s, g) in the plane, and to deter-
mine the head index and the size of the send/receive buffer for halo particle transfer
if the plane is a horizontal halo plane.

- The function update_descriptors() [E] updates elements in FieldDesc[][][] and
BorderExc[][][][] for the secondary subdomain newly assigned to the local node by
rebalancing.

- The function update_neighbors() [M] initializes/updates AbsNeighbors[][], Grid
Offset[][] and PrimaryCommList[][] for the local node’s primary or secondary subdo-
main.

- The function set_grid_descriptor() [M] sets an element of GridDesc[] according
to a subdomain.

- The function adjust_field_descriptor() [E] adjusts FieldDesc[F−1].{bc, red}.
size[] for the broadcast and reduction of per-grid histogram.

- The function update_real_neighbors() [E] updates RealDstNeighbors[][] and Real
SrcNeighbors[][].

- The function upd_real_nbr() [E] updates an element array of RealDstNeighbors[][]
or RealSrcNeighbors[][].

- The function exchange_xfer_amount() [M] performs a hand-made all-to-all commu-
nication to send NOfSend[][][] and to receive it to NOfRecv[][][].

- The function make_bxfer_sched() [N] scans primary receiving blocks created by
neighbors of the local node or its helpand and grid-voxels in vertical halo planes to
build transfer schedule for halo particles of the local node, its helpand, the neighbors
of them, and the helpers of those nodes.


<!-- Page 464 -->

- The function make_bsend_sched() [N] scans a primary receiving block created by a
neighbor of the local node or its helpand and grid-voxels in a vertical interior halo
plane corresponding to the neighbor to build the sending schedule for halo particles
of the neighbor and its helpers.

- The function make_brecv_sched() [N] scans a primary receiving block created by a
neighbor of the local node or its helpand and grid-voxels in a vertical exterior halo
plane corresponding to the neighbor to build the receiving schedule for halo particles
of the local node from the neighbor and its helpers.

- The function move_to_sendbuf_4s() [M] is the level-4s counterpart of move_to_
sendbuf_primary() and move_to_sendbuf_secondary() to move particles to be sent
to SendBuf[] and pack those to stay in the local node in Particles[].

- The function move_to_sendbuf_uw4s() [M] is the level-4s counterpart of move_to_
sendbuf_uw() to move particles to be sent to SendBuf[] and pack those to stay in the
local node shifting upward in Particles[].

- The function move_to_sendbuf_dw4s() [M] is the level-4s counterpart of move_to_
sendbuf_dw() to move particles to be sent to SendBuf[] and pack those to stay in the
local node shifting downward in Particles[].

- The function sort_particles() [M] performs bucket sorting on Particles[] to have
sorted result in SendBuf[].

- The function move_and_sort() [M] moves all particles in Particles[] to SendBuf[]
sorting those staying in the local node.

- The function sort_received_particles() [M] moves all received particles in each
rbuf (p, s) to SendBuf[] sorting them.

- The function set_sendbuf_disps4s() [M]  is the  level-4s counterpart  of set_
sendbuf_disps() to updates entries of NOfSend[][][] so that each of its entry has
the displacement of the head of sbuf (p, s, m).

- The function xfer_particles() [M] performs a hand-made all-to-all communication
to exchange particles.

- The function xfer_boundary_particles_v() [N] transfers particles in vertical halo
planes.

- The function xfer_boundary_particles_h() [N] transfers particles in horizontal
halo planes.

- The function exchange_border_data_v() [N] transfers particle-associated data in
vertical halo planes.

- The function exchange_border_data_h() [N] transfers particle-associated data in
horizontal halo planes.


static void init4s(int **sdid, const int nspec, const int maxfrac,
const dint npmax, const int minmargin, const int maxdensity,
int **totalp, int **pbase, int *maxlocalp, int *cbufsize,
struct S_mycommc *mycommc, struct S_mycommf *mycommf,


<!-- Page 465 -->

int **nbor, int *pcoord, int **sdoms, int *scoord,
const int nbound, int *bcond, int **bounds, int *ftypes,
int *cfields, const int cfid, int *ctypes, int **fsizes,
int **zbound,
const int stats, const int repiter, const int verbose);
static int  transbound4s(int currmode, int stats, const int level);
static int  try_primary4s(const int currmode, const int level,
const int stats);
static int  try_stable4s(const int currmode, const int level, const int stats);
static void rebalance4s(const int currmode, const int level, const int stats);
static void exchange_particles4s(int currmode, const int nextmode,
const int level, int reb, int oldp, int newp,
const int stats);
static void count_population(const int nextmode, const int psnew,
const int stats);
static void exchange_population(const int currmode);
static void reduce_population();
static void add_population(dint *npd, const int xl, const int xu,
const int yl, const int yu, const int zl,
const int zu, const int src);
static void make_recv_list(const int currmode, const int level, const int reb,
const int oldp, const int newp, const int stats);
static void sched_recv(const int reb, const int get, const int stay,
const int nid, const int tag,
struct S_recvsched_context *context);
static void make_send_sched(const int reb, const int pcode, const int oldp,
const int newp, struct S_commlist *rlist[2],
int *rlidx[2], int *nacc, int *nsendptr);
static int  make_send_sched_body(const int ps, const int n, const int sdid,
struct S_commlist *rlist);
static void make_send_sched_self(const int psor2, struct S_commlist *rlist,
int *naccptr);
static void make_send_sched_hplane(const int psor2, const int z, int *naccptr,
int *np, int *buf);
static void update_descriptors(const int oldp, const int newp);
static void update_neighbors(const int ps);
static void set_grid_descriptor(const int idx, const int nid);
static void adjust_field_descriptor(const int ps);
static void update_real_neighbors(const int mode, const int dosec,
const int oldp, const int newp);
static void upd_real_nbr(const int root, const int psp, const int pss,
const int nbr, const int dosec, struct S_node *node,
struct S_realneighbor rnbrptr[2], int *occur[2]);
static void exchange_xfer_amount(const int trans, const int psnew,
const int nextmode);
static void make_bxfer_sched(const int trans, const int psnew,
struct S_commlist *rlist[2], int *rlidx[2]);
static void make_bsend_sched(const int psor2, const int n, const int nx,
const int ny, struct S_commlist *rlist,
int *nsendptr, int *vpptr);
static void make_brecv_sched(const int psor2, const int n, const int nx,
const int ny, struct S_commlist *rlist,
int *nrecvptr, int vpidx);
static void move_to_sendbuf_4s(const int nextmode, const int psold,


<!-- Page 466 -->

const int psnew, const int trans,
const int oldp, const int *nacc,
const int nsend, const int stats);
static void move_to_sendbuf_uw4s(const int ps, const int mysd, const int cbase,
const int nbase);
static void move_to_sendbuf_dw4s(const int ps, const int mysd, const int ctail,
const int ntail);
static void sort_particles(const int nextmode, const int psnew,
const int stats);
static void move_and_sort(const int nextmode, const int psold, const int psnew,
const int oldp, const int *nacc, const int stats);
static void sort_received_particles(const int nextmode, const int psnew,
const int stats);
static void set_sendbuf_disps4s(const int nextmode, const int trans);
static void xfer_particles(const int trans, const int psnew,
const int nextmode, struct S_particle *sbuf);
static void xfer_boundary_particles_v(const int psnew, const int pcode,
const int d);
static void xfer_boundary_particles_h(const int psnew);
static void exchange_border_data_v(void *buf, void *sbuf, void *rbuf,
MPI_Datatype type, const MPI_Aint esize,
const int d);
static void exchange_border_data_h(void *buf, MPI_Datatype type,
const MPI_Aint esize);


In addition, we use the following lower level library functions, the set of which is equiv-
alent to level-4p’s.

- The function mem_alloc() allocates a memory space by malloc(). It is called from
init4s() directly or through the macro Allocate_NOfPGrid(), oh4s_per_grid_
histogram() through the macro, oh4s_particle_buffer() and transbound4s().

- The function mem_alloc_error() aborts the simulation due to the memory shortage
reporting its cause. It is called from init4s().

- The function errstop() aborts the simulation due to an error detected by all pro-
cesses reporting given error message. It is called from init4s() and oh4s_particle_
buffer().

- The function local_errstop() aborts the simulation due to an error detected by the
local process reporting given error message. It is called from sched_recv() and oh4s_
inject_particle() directly and from oh4s_map_particle_to_neighbor(), oh4s_
map_particle_to_subdomain() and oh4s_remove_mapped_particle() through the
macro Check_Particle_Location().

- The function transbound1() is the body of oh1_transbound().  It is called from
transbound4s().

- The function try_primary1() is to examine whether particle distribution among
subdomains is balanced well and thus we can perform the simulation in primary
mode. It is called from try_primary4s().

- The function try_stable1() is to examine whether particle distribution among nodes
is balanced well and thus we can keep the current helpand-helper configuration. It is
called from try_stable4s().


<!-- Page 467 -->

- The function rebalance1() is to (re)build the helpand-helper configuration to cope
with an unacceptable load imbalance. It is called from rebalance4s().

- The function build_new_comm() is to build communicators for the helpand-helper
families built by rebalance1(). It is called from make_recv_list().

- The function exchange_primary_particles() is the core of the particle transfer in
primary mode. It is called from exchange_particles4s().

- The function move_to_sendbuf_primary() moves particles to be transferred from
Particles[] to SendBuf[] and packs those remaining in Particles[] in primary mode.
It is called from exchange_particles4s().

- The function exchange_particles() is the core of the particle transfer in secondary
mode. It is called from exchange_particles4s().

- The function init3() is the body of oh3_init(). It is called from init4s().

- The function set_field_descriptors() sets FieldDesc[f].{bc, red}.size[p] for all
f ∈[0, F) and given p ∈{0, 1}. It is called from update_descriptors().

- The  function  clear_border_exchange()  initializes  BorderExc[c][1][d][β].{send,
recv} for all c ∈[0, C), d ∈[0, D) and β ∈{0, 1}, or reinitializes them for the
subdomain which the local node has had as the secondary one but discarded by re-
balancing. It is called from update_descriptors().

- The function map_irregular_subdomain() finds the subdomain of irregular process
coordinate in which a particle resides.  It is called from oh4s_map_particle_to_
subdomain().

4.13.3  Macros If_Dim(), For_Y(), For_Z(), Do_Y(), Do_Z() and Coord_To_Index()

Before starting to define functions, we define macros generally used in level-4s functions.
The first group is for dimension dpendent operations in level-4p, but the macros in this
group are different from level-4p’s and are definitely expanded to those for D = 3 in level-
4s. The reason why we keep these macro giving definitions (almost) equivalent to those of
D = 3 case in level-4p is to minimize the difference between level-4s and level-4p codes.

If_Dim()  The macro If Dim(d, et, ef) is always expanded to et regardless of d. The macro is used in
init4s(), set_grid_descriptor() and oh4s_map_particle_to_subdomain().

For_Y()  The macro For Y(i, c, n) and For Z(i, c, n) are expanded to the for-loop header for(i;c;n)
For_Z()  to construct a for-loop for the dimension 2 (y) or 3 (z). They are commonly used in macros
For_All_Grid(), For_All_Grid_Abs() and For_All_Grid_XY_At_Z(), while For_Y() is
used also in For_All_Grid_XY() and For_Z() is used in For_All_Grid_Z().

Do_Z()  The macro Do Y(a) and Do Z(a) are expanded to a. They are used in update_neighbors(),
Do_Y()  oh4s_map_particle_to_neighbor() and oh4s_map_particle_to_subdomain().

Coord_To_Index()  The macro Coord To Index(x, y, z, w, d·w) is expanded to x + y · w + z · d · w to give
the one-dimensional index of the element [z][y][x] in a (conceptual) D-dimensional array
of [h][d][w], i.e., gidx(x, y, z). The macro is used in macros For_All_Grid(), For_All_
Grid_Abs(), For_All_Grid_Z(), For_All_Grid_XY_At_Z() and Allocate_NOfPGrid(),
and functions init4s(), update_neighbors(), oh4s_map_particle_to_neighbor() and
oh4s_map_particle_to_subdomain().


<!-- Page 468 -->

#define If_Dim(D, ET, EF)  (ET)
#define For_Y(LINIT, LCONT, LNEXT) for(LINIT; LCONT; LNEXT)
#define For_Z(LINIT, LCONT, LNEXT) for(LINIT; LCONT; LNEXT)
#define Do_Y(ACT) ACT
#define Do_Z(ACT) ACT
#define Coord_To_Index(GX, GY, GZ, W, DW)  ((GX) + (GY)*(W) + (GZ)*(DW))


4.13.4  Macros Decl_For_All_Grid(), For_All_Grid(), For_All_Grid_Abs(),
The_Grid(), Grid_X(), Grid_Y() and Grid_Z()

The next group of generally used macros are for traversing per-grid histogram. Since they
are perfectly equivalent to those in level-4p, we briefly discuss them focusing on the functions
using them.

Decl_For_All_Grid()  The macro Decl_For_All_Grid() declares local variables fag v used in For_All_Grid(),
For_All_Grid_Abs(), For_All_Grid_Z(), For_All_Grid_XY() and For_All_Grid_XY_
At_Z(). The macro is used in functions that use the macros listed above.

For_All_Grid()  The macro For All Grid(p, x0, y0, z0, x1, y1, z1) constracts nested for-loops to traverse grid-
For_All_Grid_Abs()  voxels (x, y, z) in the per-grid histogram of local node n’s primary (p = 0) or secondary (p =
1) subdomains, where x ∈[x0, δx(np)+x1), y ∈[y0, δy(np)+y1) and z ∈[z0, δz(np)+z1), and
np = {n, parent(n)}[p]. The macro For All Grid Abs(p, x0, y0, z0, x1, y1, z1) acts similarly
but the ranges are x ∈[x0, x1), y ∈[y0, y1) and z ∈[z0, z1).
The macro For_All_Grid() is used in transbound4s(), exchange_particles4s(),
count_population(), exchange_population(), sort_particles(), move_and_sort(),
while For_All_Grid_Abs() is used solely in add_population(). Note that we have level-
4s’s own relatives For_All_Grid_Z(), For_All_Grid_XY() and For_All_Grid_XY_At_Z()
defined afterward.

The_Grid()  The macro The_Grid() is to use in the body part of For_All_Grid() and its relatives to
Grid_X()  give gidx(x, y, z) stored in fag gx but without referring to the special variable name. The
Grid_Y()  other special variables fag xidx, fag yidx and fag zidx for x, y and z can be also referred
Grid_Z()  to by the macros Grid_X(), Grid_Y() and Grid_Z(). The macro The_Grid() is used in
all functions using For_All_Grid() and its relatives except for For_All_Grid_Z(). The
functions exchange_population(), make_send_sched_self() and make_brecv_sched()
also uses Grid_Z(), while make_bsend_sched() uses Grid_X() and Grid_Z().


#define Decl_For_All_Grid()\
int fag_x1, fag_y1, fag_z1;\
int fag_xidx, fag_yidx, fag_zidx, fag_gx, fag_gy, fag_gz;\
int fag_w, fag_dw;
#define For_All_Grid(PS, X0, Y0, Z0, X1, Y1, Z1)\
For_Z((fag_zidx=(Z0), fag_x1=GridDesc[PS].x+(X1),\
fag_y1=GridDesc[PS].y+(Y1), fag_z1=GridDesc[PS].z+(Z1),\
fag_w=GridDesc[PS].w, fag_dw=GridDesc[PS].dw,\
fag_gz=Coord_To_Index(X0,Y0,Z0,fag_w,fag_dw)),\
(fag_zidx<fag_z1), (fag_zidx++,fag_gz+=fag_dw))\
For_Y((fag_yidx=(Y0), fag_gy=fag_gz),\
(fag_yidx<fag_y1), (fag_yidx++,fag_gy+=fag_w))\
for (fag_xidx=(X0),fag_gx=fag_gy; fag_xidx<fag_x1; fag_xidx++,fag_gx++)
#define For_All_Grid_Abs(PS, X0, Y0, Z0, X1, Y1, Z1)\


<!-- Page 469 -->

For_Z((fag_zidx=(Z0), fag_x1=(X1), fag_y1=(Y1), fag_z1=(Z1),\
fag_w=GridDesc[PS].w, fag_dw=GridDesc[PS].dw,\
fag_gz=Coord_To_Index(X0,Y0,Z0,fag_w,fag_dw)),\
(fag_zidx<fag_z1), (fag_zidx++,fag_gz+=fag_dw))\
For_Y((fag_yidx=(Y0), fag_gy=fag_gz),\
(fag_yidx<fag_y1), (fag_yidx++,fag_gy+=fag_w))\
for (fag_xidx=(X0),fag_gx=fag_gy; fag_xidx<fag_x1; fag_xidx++,fag_gx++)
#define The_Grid()  (fag_gx)
#define Grid_X()  (fag_xidx)
#define Grid_Y()  (fag_yidx)
#define Grid_Z()  (fag_zidx)



#### 4.13.5 Constants URN_PRI, URN_SEC and URN_TRN

URN_PRI  The last group of macro definitions for constants of update_real_neighbors()’s opera-
URN_SEC  tion mode is also equivalent to that in level-4p. To the function, URN_PRI = 0 to turn to
URN_TRN  primary mode is given by init4s() or try_primary4s(), URN_SEC = 1 on helper-helpand
reconfiguration with anywhere accommodatation is given by exchange_particles4s(),
and URN_TRN = 2 meaning the awareness of transitional state of helper-helpand configura-
tion is given by make_recv_list().


#define URN_PRI 0
#define URN_SEC 1
#define URN_TRN 2



#### 4.13.6 oh4s_init() and init4s()

oh4s_init_()  The API functions oh4s_init_() for Fortran and oh4s_init() for C receive a set of
oh4s_init()  array/structure variables through which level-1 to level-4s library functions communicate
with the simulator body, and a few integer parameters to specify the behavior of the library.
The argument set of the functions are different from that of level-4p counterparts oh4p_
init[_]() described in §4.10.6 as follows.

- New arguments npmax, minmargin and maxdensity = D are added to calculate
maxlocalp = Plim′   which is input argument in level-2/3/4p but is output in level-
4s.  Since the margin factors Phalo and Pmgn to determine P lim′  and thus Plim de-
pends on the largest subdomain size δmaxd    , we cannot calculate Plim′   prior to oh4s_
init() while it is done in level-2/3 with oh2_max_local_particles() and in level-4p
with oh4p_max_local_particles(). Therefore, we let init4s(), being the body of
oh4s_init(), calculate P lim′   adding these three arguments and changing the role of
maxlocalp. This also eliminates pbuf from the argument set because init4s() does
not have Plim possibly greater than P lim,′   and thus the association pbuf to Particles[]
and SendBuf[] is done by oh4s_particle_buffer() called by simulator body after
oh4s_init().

- The argument cbufsize is added to report Pcomm being the required size of send/
receive buffers for halo part transfer of particle-associated arrays.

- The argument zbound[2][2]  is added to associate an array in simulator body to
ZBoundShadow[2][2] for ζβp (n).


<!-- Page 470 -->

On the other hand, the argument addition and modification for the call of init4s() and
setting specBase to 0 or 1 are same as those in oh4s_init[_]() discussed in §4.10.6.


void
oh4s_init_(int *sdid, const int *nspec, const int *maxfrac, const dint *npmax,
const int *minmargin, const int *maxdensity, int *totalp,
int *pbase, int *maxlocalp, int *cbufsize, struct S_mycommf *mycomm,
int *nbor, int *pcoord, int *sdoms, int *scoord, const int *nbound,
int *bcond, int *bounds, int *ftypes, int *cfields, int *ctypes,
int *fsizes, int *zbound,
const int *stats, const int *repiter, const int *verbose) {
specBase = 1;
init4s(&sdid, *nspec, *maxfrac, *npmax, *minmargin, *maxdensity, &totalp,
&pbase, maxlocalp, cbufsize, NULL, mycomm, &nbor, pcoord, &sdoms,
scoord, *nbound, bcond, &bounds, ftypes, cfields, -1, ctypes, &fsizes,
&zbound,
*stats, *repiter, *verbose);
}
void
oh4s_init(int **sdid, const int nspec, const int maxfrac, const dint npmax,
const int minmargin, const int maxdensity, int **totalp,
int **pbase, int *maxlocalp, int *cbufsize, void *mycomm,
int **nbor, int *pcoord, int **sdoms, int *scoord, const int nbound,
int *bcond, int **bounds, int *ftypes, int *cfields, int *ctypes,
int **fsizes, int **zbound,
const int stats, const int repiter, const int verbose) {
specBase = 0;
init4s(sdid, nspec, maxfrac, npmax, minmargin, maxdensity, totalp,
pbase, maxlocalp, cbufsize, (struct S_mycommc*)mycomm, NULL, nbor,
pcoord, sdoms, scoord, nbound, bcond, bounds, ftypes, cfields, 0,
ctypes, fsizes, zbound,
stats, repiter, verbose);
}


Allocate_NOfPGrid()  As done  in  level-4p, we define  the macro Allocate NOfPGrid(π, h, t, σ, ν) used  in
init4s() to allocate and initialize a per-grid histogram, namely NOfPGrid[2][S][σ] and
NOfPGridTotal[2][S][σ] of t = dint. The definition is almost equivalent to that in level-4p
described in §4.10.6 but the offset from the first element of the body arrays of [p][s][σ] to
the element corresponding to (0, 0, 0) is differently given by gidx(3eg, 3eg, 3eg) rather than
gidx(2eg, 2eg, 2eg) for level-4p, because we have 3eg thick planes in the exterior of a subdo-
main for 2eg thick receiving planes and eg thick outside sending planes. Also as in level-4p,
the macro has the sole other user oh4s_per_grid_histogram() with t = int, but the
function uses the macro not only for NOfPGridOut[][][] but also for NOfPGridOutShadow[][][],
NOfPGridIndex[][][] and NOfPGridIndexShadow[][][].


#define Allocate_NOfPGrid(BODY, NPG, TYPE, SIZE, MSG) {\
const int ns2 = nOfSpecies<<1;\
const int gridsize = SIZE;\
TYPE *npg = BODY;\
TYPE **npgp = (TYPE**)mem_alloc(sizeof(TYPE*), ns2, MSG);\
int s, g, exto=OH_PGRID_EXT*3;\
const int base = Coord_To_Index(exto, exto, exto,\


<!-- Page 471 -->

GridDesc[0].w, GridDesc[0].dw);\
if (!npg)\
BODY = npg = (TYPE*)mem_alloc(sizeof(TYPE), ns2*gridsize, MSG) + base;\
for (s=0; s<ns2; s++,npg+=gridsize) {\
npgp[s] = npg;\
for (g=0; g<gridsize; g++)  npg[g-base] = 0;\
}\
NPG[0] = npgp;  NPG[1] = npgp + nOfSpecies;\
}

nOfLocalPLimitShadow  As in level-4p, we declare a global variable nOfLocalPLimitShadow private to ohhelp4s.c to
keep Plim.′   However, the functions referring to the variable are different; it is (re)initialized
by init4s(), and then referred to by oh4s_particle_buffer() to confirm that the func-
tion is called after oh4s_init() and the argument maxlocalp = Plim given to the function
is not less than Plim′   stored in the variable.

init4s()  The function init4s(), called from oh4s_init[_]() implements the initialization for those
API functions. The arguments of the function are almost same as oh4s_init() but its
mycomm is split into two arguments mycommc and mycommf and there is an additional argu-
ment cfid as discussed in §4.7.3.


static int nOfLocalPLimitShadow = -1;
static void
init4s(int **sdid, const int nspec, const int maxfrac, const dint npmax,
const int minmargin, const int maxdensity, int **totalp, int **pbase,
int *maxlocalp, int *cbufsize, struct S_mycommc *mycommc,
struct S_mycommf *mycommf, int **nbor, int *pcoord, int **sdoms,
int *scoord, const int nbound, int *bcond, int **bounds, int *ftypes,
int *cfields, const int cfid, int *ctypes, int **fsizes, int **zbound,
const int stats, const int repiter, const int verbose) {
int nn, me, nnns, nnns2, n;
int (*ft)[OH_FTYPE_N] = (int(*)[OH_FTYPE_N])ftypes;
int *cf = cfields;
int (*ct)[2][OH_CTYPE_N] = (int(*)[2][OH_CTYPE_N])ctypes;
int nf, ne, c, b, size, ps, s, tr, i, x, y, z;
int *nphgram = NULL;
int *rnbr, *iptr;
dint *npgdummy = NULL,  *npgtdummy = NULL;
int loggrid;
dint idmax;
const int ext = OH_PGRID_EXT, ext2 = ext<<1, ext3 = ext*3;
struct S_particle pbufdummy, *pbufdummyptr = &pbufdummy;
dint npl;


The structure of init4s() is similar to init4p() described in §4.10.6 and some of
its portions are equivalent to those of the counterpart. However the function has various
difference from the counterpart of course for level-4s’s own initialization, and the first
difference appears its very beginning. That is, at first we check if D = 3 because the level-
4s extension is only for 3-dimensional simulations, and if eg = 1 because having horizontal
halo planes of two or more grids thick is not easy to implement or we need;

- to send/receive particles to/from two or more nodes for the lower or upper set of
horizontal halo planes, or;


<!-- Page 472 -->

- to restrict the height of a subcuboid to be eg or larger in order to make the commu-
nication above performed with only one node.

Though removing the restriction on eg is not extremely tough especially with the second
solution above and only a few functions need the restriction, we so far abandon to cope
with cases of eg > 1 because it is very unlike that a simulator requires eg > 1. Therefore,
we confirm that both conditions are satisfied, or abort execution by errstop() if either of
them does not hold.

if (OH_DIMENSION!=3)
errstop("dimension size %d is not 3 which level-4s extension requires.",
OH_DIMENSION);
if (OH_PGRID_EXT!=1)
errsotp("boundary plane thickness %d is not 1 which level-4s extension "
"requires.", OH_PGRID_EXT);


The next part is equivalent to the first half of the corresponding part of init4p()
to get N by MPI_Comm_size() and then to allocate TempArray[4][N] by mem_alloc().
However, the second half is eliminated because the association of pbuf in simulator body
to Particles[] and SendBuf[] is done by oh4s_particle_buffer() in level-4s.

MPI_Comm_size(MCW, &nn);  nnns = nn * nspec;  nnns2 = nnns << 1;
TempArray = (int*)mem_alloc(sizeof(int), nn<<2, "TempArray");


The next part  is very similar to that in init4p() to intercept arguments ftypes,
cfields and ctypes so that their substances FieldTypes[], BoundaryCommFields[] and
BoundaryCommTypes[][][][] have one additional element for each for per-grid histogram.
However, their additional last elements are different from those in level-4p and have the
followings as discussed in §4.12.3.

FieldTypes[F−1][] = {1, 0, 0, −eg, eg, −eg, eg}
BoundaryCommFields[C−1] = F −1
{
{{−eg, eg, 2eg}, {−eg, −3eg, 2eg}}  b = 0
BoundaryCommTypes[C−1][b][][] =
{{0, 0, 0}, {0, 0, 0}}                 b > 0



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
ft[nf][OH_FTYPE_BL] = -ext;  ft[nf][OH_FTYPE_BU] = ext;


<!-- Page 473 -->

ft[nf][OH_FTYPE_RL] = -ext;  ft[nf][OH_FTYPE_RU] = ext;
ft[nf+1][OH_FTYPE_ES] = -1;
cf[ne] = nf;  cf[ne+1] = -1;
ct[0][OH_LOWER][OH_CTYPE_FROM] = -ext;
ct[0][OH_LOWER][OH_CTYPE_TO]   = ext;
ct[0][OH_LOWER][OH_CTYPE_SIZE] = ext2;
ct[0][OH_UPPER][OH_CTYPE_FROM] = -ext;
ct[0][OH_UPPER][OH_CTYPE_TO]   = -ext3;
ct[0][OH_UPPER][OH_CTYPE_SIZE] = ext2;
for (b=1; b<nbound; b++)
ct[b][OH_LOWER][OH_CTYPE_FROM] =
ct[b][OH_LOWER][OH_CTYPE_TO]   =
ct[b][OH_LOWER][OH_CTYPE_SIZE] =
ct[b][OH_UPPER][OH_CTYPE_FROM] =
ct[b][OH_UPPER][OH_CTYPE_TO]   =
ct[b][OH_UPPER][OH_CTYPE_SIZE] = 0;


Now we call init3() passing almost all arguments of init4s() but with the following
exceptions, some of them are different from init4p()’s.

- npmax, minmargin and maxdensity, cbufsize and zbound are not passed because
they are init4s()’s own.

- As in init4p(), nphgram is the pointer to init4s()’s local variable of the same name
which has NULL to let init3() allocate NOfPLocal[][][], because init4s() does not
have the argument.

- As in init4p(), rcounts and scounts are NULL because they are unnecessary.

- Unlike init4p(), pbuf is the double pointer to init4s()’s local variable pbufdummy
to avoid the particle buffer allocation in init3().  Another difference  is that
maxlocalp = 0 for init3() because this argument is meaningless.

- As in init4p(), ftypes, cfields and ctypes are FieldTypes[][], BoundaryComm
Fields[] and BoundaryCommTypes[][][][] respectively, and the arrays themselves are
neither allocated nor initialized by init3().

- As in init4p(), skip2 is 0 because we need level-2 initialization.

Note that, as in init4p(), cfid is passed unmodified.

init3(sdid, nspec, maxfrac, &nphgram, totalp, NULL, NULL, &pbufdummyptr,
pbase, 0, mycommc, mycommf, nbor, pcoord, sdoms, scoord, nbound,
bcond, bounds, (int*)ft, cf, cfid, (int*)BoundaryCommTypes, fsizes,
stats, repiter, verbose, 0);

The next few parts are very different from init4p()’s.  First the check of Plim′  =
nOfLocalPLimitShadow  is eliminated because the calculation of P lim′    is now done in
init4s() as follows. As done in oh4p_max_local_particles(), we call oh2_max_local_
particles() with npmax, maxfrac and minmargin to have the baseline of Plim,′   and then
add 2(Phalo+Pmgn) to it where Phalo and Pmgn are defined as follows as discussed in §4.12.3.

Phalo = D((δmaxx  + 2)(δmaxy  + 2)(δmaxz  + 2) −δmaxx  δmaxy  δmaxz   )
Pmgn = Dδmaxx  δmaxy


<!-- Page 474 -->

Then also as in oh4p_max_local_particles(), we examine Plim′    is not greater than
the maximum positive int-type number INT_MAX = 231 −1 to abort execution by mem_
alloc_error() unless it holds, and store P lim′   into the simulator body’s variable pointed
by maxlocalp and also into nOfLocalPLimitShadow for the consistency check in oh4s_
particle_buffer().
We also calculate K by;

K = 2Dδmaxz   ((δmaxx  + 2eg)(δmaxy  + 2eg) −δmaxx  δmaxy   )

to allocate BoundarySendBuf[] of K particles by mem_alloc(). On the other hand, the size
Pcomm of sbuf[] and rbuf[] arguments of oh4s_exchange_border_data(), being reported
through the argument cbufsize, can be smaller because these buffers does not need to
have four vertical halo planes but just to have a pair of them. Therefore, Pcomm is defined
as;
Pcomm = 2Dδmaxz   max(δmaxx  + 2eg, δmaxy   )

taking it into account that a xz-plane should include exterior pillars while a yz-plane may
exclude them.

size =
((Grid[OH_DIM_X].size+ext2)*(Grid[OH_DIM_Y].size+ext2)*
(Grid[OH_DIM_Z].size+ext2)-
Grid[OH_DIM_X].size*Grid[OH_DIM_Y].size*Grid[OH_DIM_Z].size) +
Grid[OH_DIM_X].size*Grid[OH_DIM_Y].size;
npl = (dint)oh2_max_local_particles(npmax, maxfrac, minmargin) +
2 * maxdensity * size;
if (npl>INT_MAX) mem_alloc_error("Particles", 0);
nOfLocalPLimitShadow = *maxlocalp = npl;
size =
2 * maxdensity * Grid[OH_DIM_Z].size *
((Grid[OH_DIM_X].size+ext2)*(Grid[OH_DIM_Y].size+ext2)-
Grid[OH_DIM_X].size*Grid[OH_DIM_Y].size);
BoundarySendBuf =
(struct S_particle*)mem_alloc(sizeof(struct S_particle), size,
"BoundarySendBuf");
size = Grid[OH_DIM_X].size + ext2;
if (size<Grid[OH_DIM_Y].size)  size = Grid[OH_DIM_Y].size;
*cbufsize = 2 * maxdensity * Grid[OH_DIM_Z].size * size;


The next part is alomost equivalent to init4p()’s. That is, we initialize PbufIndex to
be NULL to avoid the reference to it in Check_Particle_Location() before the first call
of transbound4s(); allocate NOfPGrid[2][S][G] and NOfPGridTotal[2][S][G] by Allocate_
NOfPGrid() after setting GridDesc[0] for the local node’s primary subdomain by set_grid_
descriptor(); calculate Γ to examine if gidx(δmaxx  −1, δmaxy  −1, δmaxz  −1) is small enough to
represent it by int when combined with the largest possible subdomain code; let logGrid
and gridMask have Γ and 2Γ −1 respectively; and call adjust_field_descriptor() to
modify FieldDesc[F−1].{bc, red}.size[0].
A difference is that we also allocate level-4s’s own NOfPGridZ[δmaxz    ] by mem_alloc().
The other difference is in the abortion by errstop() with too large Γ with OH_nid_t being
int. That is, codes for D < 3 cases are eliminated and the local grid size shown in the
error message reflects the fact that the planes in exterior are 3eg thick.

me = myRank;


<!-- Page 475 -->

PbufIndex = NULL;
set_grid_descriptor(0, me);
size = GridDesc[0].dw * GridDesc[0].h;
Allocate_NOfPGrid(npgdummy, NOfPGrid, dint, size, "NOfPGrid");
Allocate_NOfPGrid(npgtdummy, NOfPGridTotal, dint, size, "NOfPGridTotal");
NOfPGridZ = (dint*)mem_alloc(sizeof(dint), Grid[OH_DIM_Z].size,
"NOfPGridZ");

size = Coord_To_Index(Grid[OH_DIM_X].size-1,
If_Dim(OH_DIM_Y, Grid[OH_DIM_Y].size-1, 0),
If_Dim(OH_DIM_Z, Grid[OH_DIM_Z].size-1, 0),
GridDesc[0].w, GridDesc[0].dw);
for (loggrid=0; size; loggrid++,size>>=1);
idmax = (dint)(((nn+OH_NEIGHBORS)<<1)-1)<<loggrid;
if (idmax>INT_MAX && sizeof(OH_nid_t)==sizeof(int)) {
const int ext6 = ext3<<1;
errstop("local grid size (%d+%d)*(%d+%d)*(%d+%d) times number of nodes %d "
"is too large for OH_nid_t=int and thus OH_BIG_SPACE should be "
"defined.",
GridDesc[0].w-ext6, ext6,
GridDesc[0].d-ext6, ext6,
GridDesc[0].h-ext6, ext6, nn);
}
logGrid = loggrid;  gridMask = (1 << loggrid) - 1;
adjust_field_descriptor(0);


The next targets of allocation and initialization are data structures of level-4s’s own,
i.e., HPlane[2][2], VPlane[2N + 6], ZBoundShadow[δmaxz    ] and InteriorParts[2][S].  First,
we allocate 2 × 2 × 4 × S elements of int for four arrays of S integers in each element of
HPlane[2][2]. Then the pointer for each of arrays nsend, nrecv, sbuf and rbuf in each
element of HPlane[][] is let point appropriate portion of S integers, while nbor is initialized
to be MPI_PROC_NULL to indicate we have no transfer schedules for particle-associated array
data in horizontal halo planes so that oh4s_exchange_border_data() will do nothing even
if it is called before the first call of oh4s_transbound().
Next we  allocate 2N + 6 elements  of S_vplane structure  for VPlane[], and  let
VPlaneHead[d][p][β] = 0 for all d ∈{0, 1}, p ∈{0, 1} and β ∈{0, 1} as well as its last
element [2][0][0] to mean we have no transfer schedules for particle-associated array data
in vertical halo planes so that oh4s_exchange_border_data() will do nothing even if it is
called before the first call of oh4s_transbound() again.
Next, we allocate *zbound[2][2] by mem_alloc() if zbound argument points NULL, and
let ZBoundShadow point what *zbound points anyway. Then we initialize its elements as
ZBoundShadow[0][] = {0, δz(n)} for the local node n to mean its primary subcuboid is
the primary subdomain itself, and ZBoundShadow[1][] = {0, 0} to mean n does not have
secondary subcuboid, to allow the simulator body refers to the array before the first call
of oh4s_transbound(). Note that the substance ZBound[][] is not allocated because it has
its body on the declaration, and its elements are not initialized because they will not be
accessed before the first call of oh4s_transbound().
Next and at last of this part, we allocate InteriorParts[2][S] by mem_alloc().
Note that we eliminate the allocation and initalization of data structures for hot-spots
of level-4p, of course.

iptr = (int*)mem_alloc(sizeof(int), 2*2*4*nspec, "HPlane");


<!-- Page 476 -->

for (ps=0; ps<2; ps++)  for (i=OH_LOWER; i<=OH_UPPER; i++) {
HPlane[ps][i].nsend = iptr;  iptr += nspec;
HPlane[ps][i].nrecv = iptr;  iptr += nspec;
HPlane[ps][i].sbuf  = iptr;  iptr += nspec;
HPlane[ps][i].rbuf  = iptr;  iptr += nspec;
HPlane[ps][i].nbor  = MPI_PROC_NULL;
}
size = 2*nn + 2*2 + 2;
VPlane = (struct S_vplane*)mem_alloc(sizeof(struct S_vplane), size,
"VPlane");
VPlaneHead[0] = VPlaneHead[1] = VPlaneHead[2] = VPlaneHead[3] =
VPlaneHead[4] = VPlaneHead[5] = VPlaneHead[6] = VPlaneHead[7] =
VPlaneHead[8] = 0;

iptr = *zbound;
if (!iptr)  iptr = *zbound = mem_alloc(sizeof(int), 4, "ZBound");
ZBoundShadow = (int(*)[2])iptr;
ZBoundShadow[0][OH_LOWER] = 0;  ZBoundShadow[0][OH_UPPER] = GridDesc[0].z;
ZBoundShadow[1][OH_UPPER] = ZBoundShadow[1][OH_UPPER] = 0;

InteriorParts = mem_alloc(sizeof(struct S_interiorp), nspec*2,
"InteriorParts");

The next three lines are equivalent to those in init4p(); creating MPI data-type
T_Hgramhalf by MPI_Type_vector() and MPI_Type_commit() for a  slice [p][∗][m] in
NOfSend[][][] and NOfRecv[][][]; and zero-clearing of all elements in NOfSend[][][] for the
first call of oh4s_transbound().

MPI_Type_vector(nspec, 1, nn, MPI_INT, &T_Hgramhalf);
MPI_Type_commit(&T_Hgramhalf);
for (n=0; n<nnns2; n++)  NOfSend[n] = 0;


The next allocation and initialization for data structures of neighboring informa-
tion is very different from init4p()’s.  That  is, unlike level-4p nor other lower levels,
Neighbors[0][] = DstNeighbors[] and SrcNeighbors[] must be aware of system bound-
ary conditions. That is, in level-4p and lower levels, a subdomain may have its neighbor
beyond a non-periodic system boundary as if it is periodic, because no particle transfers
eventually take place through the boundary. However in level-4p, the subdomain cannot
have k-th neighbor if a system boundary separating them is non-periodic and thus it must
be Neighbors[k] = −(N + 1), or a node responsible of the subdomain would try to send
particles in its interior halo plane to the neighbor, while the neighbor correctly knows it
has no such particles in its exterior halo plane because exchange_population() is aware
of the system boundary condition.
Therefore we temporarily rebuild DstNeighbors[k] and SrcNeighbors[k′=3D−k−1] for
the local node n as follows, where nk is the value of DstNeighbors[k] given by init1(),
∑D−1
k =   d=0 νd3d, and bβd = Boundaries[n][d][β].
D−1∧
periodic(k) =     (νd = 1 ∨bνd/2d  = 0)
d=0        
 nk          periodic(k) ∧nk ≥0
DstNeighbors[k] = SrcNeighbors[k′] =  −(nk + 1)  periodic(k) ∧−N ≤nk < 0

−(N + 1)   ¬periodic(k) ∨nk = −(N + 1)


<!-- Page 477 -->

That is, we let DstNeighbors[k] = SrcNeighbors[k′] have non-negative subdomain iden-
tifier of the k-th neighbor  if it originally exists and is not beyond non-periodic system
boundary, or let them have −(N + 1) otherwise.
Then we modify DstNeighbors[k] and SrcNeighbors[k] so that they have the followings,
where ∆k and Σk are their original values, as done in init1() using TempArray[m] (m ∈
[0, N)) for the occurence check.

 −(N + 1)  ∆k < 0
DstNeighbors[k] =  ∆k       ∆k ≥0 ∧∀l < k : (∆l ̸= ∆k)

−(∆k + 1)  ∆k ≥0 ∧∃l < k : (∆l = ∆k)

 −(N + 1)  Σk < 0
SrcNeighbors[k] =  Σk        Σk ≥0 ∧∀l < k : (Σl ̸= Σk)

−(Σk + 1)  Σk ≥0 ∧∃l < k : (Σl = Σk)

At the same time, we let FirstNeighbor[k] as follows as done in init4p() but using
TempArray[N + m] (m ∈[0, N)) to remember minimum k such that m = Σk.
{
k                   Σk < 0
FirstNeighbor[k] =
min{l | l ≤k, Σl = Σk}  Σk ≥0

In  the  loop  doing  above,  we  also add  a  very  level-4s own  initialization  to  let
PrimaryRLIndex[k] = k which are referred to by make_send_sched() and its callees when
the next execution mode is primary as the trivial index of primary receiving, primary send-
ing, secondary receiving and secondary sending blocks of S_commlist records, while the
blocks are set in update_neighbors().
The remaining part  is literally equivalent to init4p()’s, but the call of update_
neighbors() is semantically different because its initialization based on Neighbors[0][]
is not only for AbsNeighbors[0][] and GridOffset[0][] but also for PrimaryCommList[0][].
On the other hand, other procedures, the allocation of RealDstNeighbors[2][2].nbor[N]
and RealSrcNeighbors[2][2].nbor[N] by mem_alloc() and the  call of update_real_
neighbors() with the code URN_PRI to initialize their elements in [0][0] so that they have
subdomain identifiers neighboring to the local node’s primary subdomain, are perfectly
equivalent to init4p()’s.

for (z=0,n=0; z<3; z++) {
int (*bd)[OH_DIMENSION][2] = Boundaries;
const int nonpz = z!=1 && bd[me][OH_DIM_Z][z>>1];
for (y=0; y<3; y++) {
const int nonpy = nonpz || (y!=1 && bd[me][OH_DIM_Y][y>>1]);
for (x=0; x<3; x++,n++) {
int dnbr = DstNeighbors[n];
const int nrev = OH_NEIGHBORS - 1 - n;
if (nonpy || (x!=1 && bd[me][OH_DIM_X][x>>1]))
DstNeighbors[n] = SrcNeighbors[nrev] = -(nn+1);
else if (dnbr<0 && dnbr>=-nn)
DstNeighbors[n] = SrcNeighbors[nrev] = -(dnbr+1);
else
SrcNeighbors[nrev] = dnbr;
}
}
}
for (i=0; i<nn; i++)  TempArray[i] = 0;


<!-- Page 478 -->

for (n=0; n<OH_NEIGHBORS; n++) {
const int dnbr = DstNeighbors[n],  snbr = SrcNeighbors[n];
int *sfirst = TempArray + nn;
if (dnbr>=0) {
if (TempArray[dnbr]&1)  DstNeighbors[n] = -(dnbr+1);
else                    TempArray[dnbr] |= 1;
}
if (snbr>=0) {
if (TempArray[snbr]&2) {
SrcNeighbors[n] = -(snbr+1);
FirstNeighbor[n] = sfirst[snbr];
} else {
FirstNeighbor[n] = sfirst[snbr] = n;
TempArray[snbr] |= 2;
}
} else
FirstNeighbor[n] = n;
PrimaryRLIndex[n] = n;
}
update_neighbors(0);
rnbr = (int*)mem_alloc(sizeof(int), nn*2*2*2, "RealNeighbors");
for (tr=0; tr<2; tr++)  for (ps=0; ps<2; ps++,rnbr+=nn) {
RealDstNeighbors[tr][ps].n = RealSrcNeighbors[tr][ps].n = 0;
RealDstNeighbors[tr][ps].nbor = rnbr;
RealSrcNeighbors[tr][ps].nbor = rnbr + nn*2*2;
}
update_real_neighbors(URN_PRI, 0, -1, -1);


The  last  part,  in which we copy the  contents  of bcond[D][2]  to  its substance
BoundaryCondition[][] by memcpy()  if SubDomainDesc is NULL to mean regular process
coordinate, is also equivalent to init4p()’s.

if (!SubDomainDesc)
memcpy(BoundaryCondition, bcond, sizeof(int)*OH_DIMENSION*2);
}


#### 4.13.7 oh4s_particle_buffer()

oh4s_particle_buffer_()  The API functions oh4s_particle_buffer_() for Fortran and oh4s_particle_buffer()
oh4s_particle_buffer()  for C associate particle buffer pbuf[2Plim] to Particles[Plim] and SendBuf[Plim] where
Plim = maxlocalp. The function is level-4s’s own but performs what init4p() and init2()
do as follows.
First, we confirm that P lim′  = nOfLocalPLimitShadow is non-negative and not greater
than Plim = maxlocalp, or in other words oh4s_init() has been called and its argument
maxlocalp is passed (possibly after incremented) to maxlocalp of this function.  If not,
we stop the execution by errstop() with an appropriate error message. This part is very
similar to the corresponding part of init4p() but the error messages are different reflecting
the difference of the functions to calculate P lim′  and to check Plim > P lim.′
Next, as done in init4p(), we allocate Particles[Plim] and SendBuf[Plim] as a con-
tiguous array of [2Plim]  if pbuf points NULL. Otherwise, what pbuf points is set to the
pointer Particles, and SendBuf is let point the head of the second half of pbuf.


<!-- Page 479 -->

Finally, we let nOfLocalPLimit = Plim and totalParts = Plim as done in init2(),
because the assignments in init2() are meaningless with incorrect maxlocalp passed to it
by init4s() through init3().


void
oh4s_particle_buffer_(const int *maxlocalp, struct S_particle *pbuf) {
oh4s_particle_buffer(*maxlocalp, &pbuf);
}
void
oh4s_particle_buffer(const int maxlocalp, struct S_particle **pbuf) {

if (nOfLocalPLimitShadow<0)
errstop("oh4s_particle_buffer() has to be called after oh4s_init()");
else if (maxlocalp<nOfLocalPLimitShadow)
errstop("argument maxlocalp %d given to oh4s_particle_buffer() is less "
"than that calculated by oh4s_init() %d",
maxlocalp, nOfLocalPLimitShadow);
if (*pbuf)
Particles = *pbuf;
else
Particles = *pbuf =
(struct S_particle*)mem_alloc(sizeof(struct S_particle),
maxlocalp<<1, "Particles");
SendBuf = Particles + maxlocalp;
nOfLocalPLimit = totalParts = maxlocalp;
}


#### 4.13.8 oh4s_per_grid_histogram()

oh4s_per_grid_histogram_()  The API  functions oh4p_per_grid_histogram_()  for Fortran and oh4p_per_grid_
oh4s_per_grid_histogram()  histogram() for C associate the shadow per-grid histogram NOfPGridOutShadow[][][] and
per-grid index NOfPGridIndexShadow[][][] to those in the simulator body given through the
arguments pghgram and pgindex. The differnces between the functions and their coun-
terparts oh4p_per_grid_histogram[_]() described in §4.10.8 are as follows; level-4s’s has
an additional argument pgindex for per-grid index; level-4s’s associates arguments with
shadow arrays because substance ones are accessed outside oh4s_transbound(),  i.e., in
oh4s_exchange_border_data(); and thus level-4s’s allocates substance arrays.
As in the countepart of level-4p, the Fortran coded simulator must allocate 5-dimensional
arrays whose leading 3-dimensional sizes are commonly specified in fsizes[F−1][][] given
through the argument of oh4s_init_(), and give the origin element of the array (0, 0, 0, 1, 1)
through pghgram and pgindex. On the other hand, C coded simulator may let the function
allocate the arrays by giving a double pointers to NULL to the arguments, or allocate the
arrays by itself and give the double pointer to the array’s origin element to arguments.
The function invokes the macro Allocate_NOfPGrid() to allocate the shadow arrays of
[2][S][G] where;

G = GridDesc[0].dw × GridDesc[0].h = ((δmaxx  + 6eg)(δmaxy  + 6eg)) × (δmaxz  + 6eg)

and returns the pointer to the conceptual element [0][0][0][0][0] through *pghgram and
*pgindex if they are NULL, to allocate the pointer array for them for the use in library
functions, and to zero-clear their bodies. The macro is also used for the allocation and
initialization of substance arrays NOfPGridOut[2][S][G] and NOfPGridIndex[2][S][G].


<!-- Page 480 -->

void
oh4s_per_grid_histogram_(int *pghgram, int *pgindex) {
oh4s_per_grid_histogram(&pghgram, &pgindex);
}
void
oh4s_per_grid_histogram(int **pghgram, int **pgindex) {
int *npgo=NULL, *npgi=NULL;
const int size = GridDesc[0].dw*GridDesc[0].h;
Allocate_NOfPGrid(npgo, NOfPGridOut, int, size, "NOfPGridOut");
Allocate_NOfPGrid(*pghgram, NOfPGridOutShadow, int, size,
"NOfPGridOutShadow");
Allocate_NOfPGrid(npgi, NOfPGridIndex, int, size, "NOfPGridIndex");
Allocate_NOfPGrid(*pgindex, NOfPGridIndexShadow, int, size,
"NOfPGridIndexShadow");
}


#### 4.13.9 oh4s_transbound() and transbound4s()

oh4s_transbound_()  The API function oh4s_transbound_() for Fortran and oh4s_transbound() for C provide
oh4s_transbound()  the simulator body calling them with the load-balanced particle transfer mechanism of level-
4s and lower level libraries. The meanings of their two arguments, currmode and stats,
and return value in {−1, 0, 1} are perfectly equivalent to those of the level-1 to level-3
counterparts oh1_transbound[_](), oh2_transbound[_]() and oh3_transbound[_]().
Also similarly to the counterparts, their bodies only have a simple call of transbound4s()
but the third argument level is 4 to indicate the function is called from level-4s API
functions.


int
oh4s_transbound_(int *currmode, int *stats) {
return(transbound4s(*currmode, *stats, 4));
}
int
oh4s_transbound(int currmode, int stats) {
return(transbound4s(currmode, stats, 4));
}


transbound4s()  The function transbound4s(), called from oh4s_transbound[_](), is very similar to its
level-4p counterpart transbound4p() described in §4.10.9. The difference between them
are as follows; level-4s’s initializes ZBound[][] at its beginning and copies the array into its
shadow ZBoundShadow[][] at its end; and the range of NOfPGrid[][][] to be zero-cleared at
the end of level-4s’s is larger reflecting the fatter exterior.
Equivalently  to  transbound4p(),  at  first we  call  transbound1()  to  calculate
NOfPrimaries[][],  TotalPGlobal[],  nOfParticles  and  nOfLocalPMax  and  to  have
currmode always, and to calculate TotalP[][], primaryParts and totalParts on the first
call. Then we perform level-4s’s own operations to let ZBound[p][β] = 0 for all p ∈{0, 1}
and β ∈{0, 1} to mean the local node has neither of primary nor secondary subcuboids,
i.e., no particles at all, unless exchange_particles4s() finds some particles assigned to
the node very likely but not necessarily. Then we call functions for the heart of balancing
examination similarly to transbound4p() but there are difference that called functions are
level-4s’s own try_primary4s(), try_stable4s() and rebalance4s().


<!-- Page 481 -->

static int
transbound4s(int currmode, int stats, const int level) {
int ret=MODE_NORM_SEC;
const int nn=nOfNodes, ns=nOfSpecies, ns2=ns<<1, nnns2=nn*ns2;
struct S_particle *tmp;
int i, ps, s, tp;
Decl_For_All_Grid();

stats = stats && statsMode;
currmode = transbound1(currmode, stats, level);

ZBound[0][OH_LOWER] = ZBound[0][OH_UPPER] = 0;
ZBound[1][OH_UPPER] = ZBound[1][OH_UPPER] = 0;
if (try_primary4s(currmode, level, stats))  ret = MODE_NORM_PRI;
else if (!Mode_PS(currmode) || !try_stable4s(currmode, level, stats)) {
rebalance4s(currmode, level, stats);  ret = MODE_REB_SEC;
}

The next part is equivalent to transbound4p()’s. We allocate PbufIndex[2][S] and
its additional element [2][0] by mem_alloc()  if it is NULL to mean the first call of this
function. Then we clear NOfPLocal[][][]; copy TotalPNext[p]s] to its substance TotalP[p][s]
letting PbufIndex[p][s] have the index of pbuf (p, s); set totalParts, its shadow pointed by
totalLocalParticles and PbufIndex[2][0] to the sum of TotalP[p][s] for all p ∈[0, 1] and
s ∈[0, S); and clear InjectedParticles[0][][] = qinj(n)[][] and nOfInjections = Qinjn  .

if (!PbufIndex)
PbufIndex = (int*)mem_alloc(sizeof(int), ns2+1, "PbufIndex");
for (i=0; i<nnns2; i++) NOfPLocal[i] = 0;
for (s=0,tp=0; s<ns2; s++) {
TotalP[s] = TotalPNext[s];  PbufIndex[s] = tp;  tp += TotalPNext[s];
}
PbufIndex[s] = totalParts = *totalLocalParticles = tp;  nOfInjections = 0;
for (s=0; s<ns2; s++)  InjectedParticles[s] = 0;


The next part is almost equivalent to transbound4p()’s.  It zero-clears elements of
NOfPGrid[p][s][gidx(x, y, z)] for p = 0 if the next execution mode is primary or p ∈[0, 1] if
secondary, for all s ∈[0, S) and all (x, y, z) ∈[−keg, δx(m) + keg) × [−keg, δy(m) + keg) ×
[−keg, δz(m)+keg) where m = n for p = 0 or m = parent(n) for p = 1 for the local node n,
and k = 1 for p = 0 or the helper-helpand tree is kept, or k = 3 being different from k = 2
in transbound4p() because of 3eg thickness of the exterior otherwise, by For_All_Grid().


for (ps=0; ps<=Mode_PS(ret); ps++) {
const int extio = (ps==1 && ret<0) ? OH_PGRID_EXT*3 : OH_PGRID_EXT;
for (s=0; s<ns; s++) {
dint *npg = NOfPGrid[ps][s];
For_All_Grid(ps, -extio, -extio, -extio, extio, extio, extio)
npg[The_Grid()] = 0;
}
}

Then we copy ZBound[][] to its shadow ZBoundShadow[][] to let it be referred to by
the simulator body, as another level-4s’s own operation.  Finally and equivalently to


<!-- Page 482 -->

transbound4p(), we exchange the role of Particles[] and SendBuf[], and return to the
caller with the return value defined in §4.3.10 letting currMode have its absolute value in
order to replaced MODE_REB_SEC = −1 with MODE_NORM_SEC = 1.

ZBoundShadow[0][0] = ZBound[0][0];    ZBoundShadow[0][1] = ZBound[0][1];
ZBoundShadow[1][0] = ZBound[1][0];    ZBoundShadow[1][1] = ZBound[1][1];
tmp = Particles;  Particles = SendBuf;  SendBuf = tmp;
currMode = ret<0 ? -ret : ret;
return(ret);
}


#### 4.13.10 try_primary4s()

try_primary4s()  The function try_primary4s(), called solely from transbound4s(), examines if we can
stay in or turn to primary mode.   If so, the local node gathers all the particles in its
primary subdomain from other nodes, sort them according to their grid-position, and then
gather halo particles from its neighbor nodes. The function has three arguments currmode,
level and stats whose meanings are perfectly equivalent to those of its level-1 counterpart
try_primary1().
The code structure of this function is completely different from its level-4p counter-
part try_primary4p() described in §4.10.10, because it shares the particle transfer and
sorting mechanisms implemented in exchange_particles4s() with try_stable4s() and
rebalance4s(), while try_primary4p() has its own mechanisms for primary mode. The
reason why we made the mechanisms common is that we need to have halo particle transfer
and its scheduling which are easily implemented with S_commlist records even for primary
mode, i.e., those in PrimaryCommList[].
In this function, first we call the level-1 counterpart try_primary1() to examine if the
next execution mode is primary. If not, we simply return to its caller transbound4s() with
the return value FALSE to indicate the mode will be secondary.
Otherwise, i.e., if we will be in primary mode, we call exchange_particles4s() with
the arguments currmode, level and stats of this functions’s own, and nextmode = 0
meaning the next execution mode is primary, reb = 0 meaning no rebalancing took place,
oldp = parent(n) for the helpand of the local node in the last step being RegionId[1] before
the call of try_primary1() if any, and newp = −1 meaning the local node will not have
helpand of course.
After that,  if we were in secondary mode, we call update_real_neighbors() with
the operation code URN_PRI to  reinitialize the elements RealDstNeighbors[0][0] and
RealSrcNeighbors[0][0] so that they have subdomain identifiers neighboring to the local
node’s primary subdomains. Note that this call should be done after the call of exchange_
particles4s() because the elements has been kept to send n’s secondary particles to the
nodes whose primary subdomains are neighbors of n’s secondary subdomain, and to re-
ceive n’s primary particles in the next step from the nodes whose primary or secondary
subdomains are neighbors of n’s primary subdomain.
Finally the function returns to transbound4s() with the return value of TRUE.


static int
try_primary4s(const int currmode, const int level, const int stats) {
const int oldp = RegionId[1];

if (!try_primary1(currmode, level, stats)) return(FALSE);


<!-- Page 483 -->

exchange_particles4s(currmode, 0, level, 0, oldp, -1, stats);
if (Mode_PS(currmode))  update_real_neighbors(URN_PRI, 0, -1, -1);
return(TRUE);
}


#### 4.13.11 try_stable4s()

try_stable4s()  The function try_stable4s(), solely called from transbound4s(), examines if the cur-
rent helpand-helper configuration sustains by try_stable1() and, if examination passes,
performs particle transfer by exchange_particles4s().
The code structure of this function is very similar to that of its level-4p counterpart
try_stable4p() described in §4.10.11. That is, first we call the level-1 counterpart try_
stable1() passing all arguments or negating level argument according to the accommoda-
tion pattern is anywhere or normal respectively. Then if try_stable1() returns FALSE we
return to the caller transbound4s() with FALSE too, or we call exchange_particles4s()
for particle transfer otherwise.  The differnce  is in the latter case because exchange_
particles4s() is different from its counterpart exchange_particles4p() and it has an ad-
ditional argument nextmode being 1 for this call to mean we will be in secondary mode in the
next step. The other arguments, however, are same as those in try_stable4p(), and thus
reb = 0 meaning no rebalancing took place and oldp = newp = RegionId[1] = parent(n)
meaning the helpand of the local node n is unchanged. After the call, as in try_stable4p(),
we return to transbound4s() with TRUE.


static int
try_stable4s(const int currmode, const int level, const int stats) {
if (!try_stable1(currmode, (Mode_Acc(currmode) ? level : -level), stats))
return(FALSE);
exchange_particles4s(currmode, 1, level, 0, RegionId[1], RegionId[1], stats);
return(TRUE);
}


#### 4.13.12 rebalance4s()

rebalance4s()  The function rebalance4s(), solely called from transbound4s(), builds the new family
tree to rebalance the load among nodes by rebalance1(), and then performs particle
transfer by exchange_particles4s(). The code structure of this function is very similar
to that of its level-4p counterpart rebalance4p() described in §4.10.12. That is, first we call
the level-1 counterpart rebalance1() passing all arguments or negating level argument
according to the accommodation pattern is anywhere or normal respectively. Then, if npold
and npnew, being the parent(n) of the local node n in the last and next step respectiely, are
different and we have anywhere accommodation, we modify InjectedParticles[0][1][s] for
all s ∈[0, S) so that it has the number of secondary particles injected to the new secondary
subdomain npnew accidentally.
Then we call exchange_particles4s() differently from that in rebalance4p() because
the function is different from its counterpart exchange_particles4p() and it has an ad-
ditional argument nextmode being 1 for this call to mean we will be in secondary mode in
the next step. The other arguments, however, are same as those in rebalance4p(), and
thus reb = 1 meaning rebalancing took place, oldp = npold, and newp = npnew.


<!-- Page 484 -->

Then after the call, as in rebalance4p(), we do the followings if we had normal accom-
modation; call set_grid_descriptor() to update GridDesc[1][] for the secondary subdo-
main; move Neighbors[2][k] for the neighbors of npnew to Neighbors[1][k] for all k ∈[0, 3D);
and finally call update_neighbors() telling it to update elements in AbsNeighbors[1][] and
GridOffset[1][] for the secondary subdomain.


static void
rebalance4s(const int currmode, const int level, const int stats) {
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
exchange_particles4s(currmode, 1, level, 1, oldp, newp, stats);
if (!amode) {
set_grid_descriptor(1, newp);
for (n=0; n<OH_NEIGHBORS; n++)  Neighbors[1][n] = Neighbors[2][n];
update_neighbors(1);
}
}


4.13.13  Macros Parent_Old(), Parent_New(), Parent_New_Same() and
Parent_New_Diff()
Parent_Old()  The following four macros to examine the statuses of old and new parents npold and npnew
Parent_New()  and its encoding in the local variable pcode = π of exchange_particles4s() is perfectly
Parent_New_Same()  equivalent to those in level-4p.
Parent_New_Diff()
- Parenet Old(π) is true iffnpold ≥0.
- Parent New(π) is true iffnpnew ≥0.
- Parent New Same(π) is true iffnpnew ≥0 and npnew = npold.


<!-- Page 485 -->

- Parent New Diff(π) is true iffnpnew ≥0 and npnew ̸= npold.

These macros Parent_Old() and Parent_New() are used in exchange_particles4s() and
make_send_sched(), while Parent_New_Diff() is solely used in make_send_sched()81.


#define Parent_Old(PCODE)       ((PCODE) & 4)
#define Parent_New(PCODE)       ((PCODE) & 2)
#define Parent_New_Same(PCODE)  (((PCODE) & 3) == 3)
#define Parent_New_Diff(PCODE)  (((PCODE) & 3) == 2)


#### 4.13.14 exchange_particles4s()

exchange_particles4s()  The function exchange_particles4s(), called from try_primary4s(), try_stable4s()
and rebalance4s(), performs an all-to-all type position-aware particle transfer including
that for halo particles. Though the function has some similarity to its level-4p counterpart
exchange_particles4p() described in §4.10.14, it has various aspects different from the
counterpart as follows.

- This function is called not only from try_stable4s() and rebalance4s() but also
from try_primary4s(), because halo particle transfer can be implemented easily
by letting this function cover both primary and secondary modes in the next step,
rather than having a mechanism specific to primary mode which try_primary4p()
has.  Therefore the function has an argument nextmode = p′n additional to those
of exchange_particles4p() to indicate we will be in primary (p′n = 0) or sec-
ondary mode in the next step.  This difference also makes it unnecessary to have
level-4s counterparts of level-4p functions mpi_allreduce_wrapper() and move_and_
sort_primary(), while making the following callee functions responsible of primary
mode too; make_send_sched(), exchange_xfer_amount(), move_to_sendbuf_4s(),
move_and_sort(), and xfer_particles().

- Since we have no hot-spots in level-4s, this function and its callees make_recv_
list(), make_send_sched(), move_to_sendbuf_4s() and move_and_sort() are free
from hot-spot-related operations.  This also makes  it unnecessary to have level-
4s counterparts of level-4p functions gather_hspot_recv(), gather_hspot_send(),
gather_hspot_send_body(), scatter_hspot_send(), scatter_hspot_recv() and
scatter_hspot_recv_body().

- Since a node is responsible of particles in subcuboids rather than a general contiguos
set of grid-voxels, make_recv_list() and make_send_sched() works xy-plane-wise.

- Since we have to transfer halo particles, this function has calls of make_bxfer_
sched() for scheduling, and xfer_boundary_particles_v() and xfer_boundary_
particles_h() for transfer.  This also lets the following functions take care the
halo particle transfer; make_send_sched(), sort_particles(), move_and_sort()
and sort_received_particles()

- Since we have the shadow per-grid histogram NOfPGridOutShadow[][][] of NOfPGridOut
[][][] and the substance/shadow pair of per-grid index arrays NOfPGridIndex[][][] and
NOfPGridIndexShadow[][][], this function is responsible to let these new arrays have

81Parent New Same() is not used at all in level-4s but we keep this macro to make level-4s is similar to
level-4p as much as possible.


<!-- Page 486 -->

appropriate values, and also works on NOfPGridTotal[][][] to let  it have per-grid
index while sort_particles(), move_and_sort_primary() and move_and_sort_
secondary() do it in level-4p.

The arguments except for the new one nextmode shown above are equivalent to
exchange_particles4p(). That is, the meanings of currmode, level and stats are same
as those of the callers, and reb, oldp = npold, newp = npnew for the local node n are as
follows, where parent(n) means n’s helpand in the last step.
- try_primary4s() gives reb = 0, npold = parent(n) and npnew = −1.
- try_stable4s() gives reb = 0, npold = npnew = parent(n).
- rebalance4s() gives reb = 1, npold = parent(n) and npnew being the new helpand of
n.

As in exchange_particles4p(), at first in the variable declaration part, the function
determines whether we have to take care of the transitional state of helpand-helper con-
figuration, i.e., whetehr we have normal accommodation and rebalancing took place, and
let its local variable trans be true iffso.  It also sets the parent status code discussed in
§4.10.13 and §4.13.13 according to the arguments npold and npnew.


static void
exchange_particles4s(int currmode, const int nextmode, const int level,
int reb, int oldp, int newp, const int stats) {
const int ns=nOfSpecies, exti=OH_PGRID_EXT;
const int trans = !Mode_Acc(currmode) && reb ? 1 : 0;
int pcode =
(oldp>=0 ? 4 : 0) + (newp>=0 ? 2 : 0) + (oldp==newp ? 1 : 0);
int ps, psold, psnew, s;
int nacc[2], nsend, tp;
struct S_commlist *rlist[2];
int *rlidx[2];
Decl_For_All_Grid();

If we have anywhere accommodation and p′n = 1, we do the followings almost equiva-
lently to what we do in exchange_particles4p(). First we call exchange_particles()
as we do in try_stable2() or rebalance2() with anywhere accommodation to have pri-
mary and secondary particles of the local node without position-aware manner.  Then
if rebalanced, we  call the followings; update_descriptors() to update elements in
FieldDesc[] for npnew and to reinitialize BorderExc[][1][][] for npold; set_grid_descriptor()
to update GridDesc[1] for npnew; update_neighbors() to update AbsNeighbors[1][] and
GridOffset[1][]; and update_real_neighbors() with the operation code URN_SEC to up-
date RealDstNeighbors[0][p] and RealSrcNeighbors[0][p] for p ∈[0, 1]. Then we reini-
tialize NOfSend[][][] by zero-clearing its all elements because it has been updated by make_
comm_count() called in try_stable1() or rebalance1().
On the other hand, the anywhere accommodation case with p′n = 0 is similar to the corre-
sponding part in try_primary4p() and thus has the calls of move_to_sendbuf_primary()
and exchange_primary_particles() for non-position-aware particle transfer.
Then still as in exchange_particles4p() for p′n = 1 and try_primary4p() for p′n = 0,
we call count_population() to build the local per-grid histogram in NOfPGrid[][][], and
then let reb = 0 and npold = npnew with corresponding setting of pcode because we do


<!-- Page 487 -->

not have to take care the helpand-helper reconfiguration. We also let primary/secondary
mode indicator in currmode be nextmode, but keeping the accommodation pattern in it to
be anywhere82, so that the following process in this function assumes that we were in the
execution mode in which will be in, because all particles are now accommodated by nodes as
the next execution mode requires regardless of the mode we were in and the accommodation
pattern we had.
On the other hand, the local per-grid histogram is then used differently from level-4p. If
p′n = 1, we will call exchange_population() afterward rather than reduce_population()
because we need not only to sum up the local per-grid histograms in the local node’s primary
family but also exchange populations in halo planes with neighbor nodes. If p′n = 0 on the
other hand, we cannot sort partilces by sort_particles() because we need to build halo
particle transfer schedule before the sorting. Therefore, the operations specific to anywhere
accommodation is over here.

if (Mode_Acc(currmode)) {
if (nextmode) {
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
} else {
move_to_sendbuf_primary(Mode_PS(currmode), stats);
exchange_primary_particles(currmode, stats);
}
count_population(nextmode, (Parent_New(pcode) ? 1 : 0), 0);
currmode = Mode_Set_Any(nextmode);
reb = 0;  oldp = newp;  pcode = newp>=0 ? 7 : 0;
}

Now, regardless of the accommodation mode and p′n, we have the local per-grid his-
togram in NOfPGrid[][][] and particles in Particles[] which will stay in the local node’s pri-
mary or secondary subdomain or travel to one of their neighbors. Therefore with this com-
mon setting, we do the followings fairly differently from those in exchange_particles4p().
First we call exchange_population() always to have the complete per-grid histogram of
the local node’s primary subdomain and its halo planes in NOfPGridTotal[0][][] and per-
plane histogram in NOfPGridZ[] for the subdomain’s interior, giving it currmode to let it
know whether the reduction on local per-grid histograms is required.
Next, as done in level-4p, we define two execution-mode indicators namely pc = psold
and pn = psnew being 1 if the local node has or will have its secondary subdomain/particles
in the last or next step respectively, or 0 otherwise. They are referred to when we need to
know if a data structure for subdomain/particles of p ∈{0, 1} has the portion for p = 1,
because, for example, pn = 1 means p′n = 1 but it can be p′n = 1 and pn = 0.

82So far, there are no reasons to remember the accommodation pattern, but at least keeping it is safe.


<!-- Page 488 -->

Next,  if p′n = 1, we call make_recv_list(), with arguments currmode, level, reb,
oldp, newp and stats of this function itself or modified ones due to anywhere accom-
modation, to build the receiver-side particle transfer schudule in CommList[] together
with the pointers to secondary receiving and alternative secondary receiving blocks in
SecRList and AltSecRList, and indices of primary receiving/sending, secondary receiv-
ing/sending and alternative secondary receiving/sending blocks in RLIndex[], SecRLIndex[]
and AltSecRLIndex[] respectively. If p′n = 0 on the other hand, the transfer schedule is triv-
ial and PrimaryCommList[p][] has primary receiving/sending blocks (p = 0) and secondary
receiving/sending ones (p = 1) while indices for neighbors in both cases are commonly in
PrimaryRLIndex[].
Next, with the transfer schedule above, we call make_send_sched() with other input
arguments currmode, reb, oldp and newp. The function builds the per-receiver sending
histogram in NOfSend[][][], lets NOfPGrid[][][] act the second role shown in §4.12.3, lets
NOfPGridOut[][][] have the local per-grid histogram at the beginning of the next step, and
lets ZBound[] and HPlane[][] have the values defined in §4.12.3. The function also gives us
the number of primary particles including halo ones to be accommodated by the local node
in the local array element nacc[0], the sum of those numbers for primary and secondary
particles in nacc[1]83, and the number of sending particles P nsend in the local variable nsend,
through its output arguments.
Finally we exchange NOfSend[][][] by a hand-made all-to-all communicaion in neighbor-
ing families to have NOfRecv[][][] by exchange_xfer_amount(), which is slightly different
because it takes care of p′n = 0 case in which we have no particles to send as other nodes’s
secondary particles.

exchange_population(currmode);
psold = Parent_Old(pcode) ? 1 : 0;
psnew = Parent_New(pcode) ? 1 : 0;
if (nextmode) {
make_recv_list(currmode, level, reb, oldp, newp, stats);
rlist[0] = CommList;  rlist[1] = SecRList;
rlidx[0] = RLIndex;   rlidx[1] = SecRLIndex;
} else {
rlist[0] = PrimaryCommList[0];  rlist[1] = PrimaryCommList[1];
rlidx[0] = rlidx[1] = PrimaryRLIndex;
}
make_send_sched(reb, pcode, oldp, newp, rlist, rlidx, nacc, &nsend);
exchange_xfer_amount(trans, psnew, nextmode);


The next part is very level-4s’s own, in which we copy the per-grid histogram from
its substance NOfPGridOut[p][][] to its shadow NOfPGridOutShadow[p][][] for p ∈{0, pn}, in
order to show it to the simulator body but keeping its original version from being (acciden-
tally) tampered by the simulator body, by scannig the substance by For_All_Grid(). At
the same time and also for p ∈{0, pn}, we build the per-grid index in NOfPGridTotal[p][][]
for the use in sort_particles(), move_and_sort() and sort_received_particles(),
the substance array NOfPGridIndex[p][][] and shadow one NOfPGridIndexShadow[][][] by
accumulating the values of the per-grid histogram to let the former two array have the val-
ues defined in §4.12.3. One caution is that the shadow can have values greater by one than
the substance for Fortran coded simulator body, i.e., when specBase = 1. The other cau-
tion is that the scanning of the per-grid histogram includes exterior halo planes of eg thick,

83This is different from level-4p in which nacc[1] has the number of secondary particles rather than the
sum.


<!-- Page 489 -->

and that on secondary subdomain is for the next step whose size is in GridDesc[2] when
we have transitional state of helpand-helper configuration as given to the first argument of
For_All_Grid().

for (ps=0,tp=0; ps<=psnew; ps++) {
const int psor2 = ps ? trans + 1 : 0;
const int sb = specBase;
for (s=0; s<ns; s++) {
dint *npgt=NOfPGridTotal[ps][s];
int *npgo=NOfPGridOut[ps][s], *npgos=NOfPGridOutShadow[ps][s];
int *npgi=NOfPGridIndex[ps][s], *npgis=NOfPGridIndexShadow[ps][s];
For_All_Grid(psor2, -exti, -exti, -exti, exti, exti, exti) {
const int g = The_Grid(),  np = npgo[g];
npgos[g] = np;  npgt[g] = npgi[g] = tp;  npgis[g] = tp + sb;  tp += np;
}
}
}

Now we start position-aware transfer of non-halo particles, similarly to exchange_
particles4s() but differently from  it in some  details.    If Qn + P nsend >  Plim =
nOfLocalPLimit, where Qn = nacc[1], to mean we cannot move all particles in Particles[]
to SendBuf[] with sorting, we perform a partially position-aware transfer only takeing care
of the node to accommodate particles in each grid-voxel. In addition and differently from
level-4p, we perform this type of transfer when helpand-helper reconfiguration takes place
as disussed shortly.  The partially position-aware transfer is at first done by move_to_
sendbuf_4s() being the level-4s version of move_to_sendbuf_sec4p() but it works even
on the case of p′n = 0, while its argument set is almost equivalent to the level-4p counterpart
except that we add psnew = pn to know the size of rbuf (p, s) in pbuf i(p, s) and nacc[1]
has the sum of primary and secondary paticles. Then we call xfer_particles(), which is
slightly different from its level-4p version because it takes care of p′n = 0 case in which we
have no particles to send as other nodes’s secondary particles, to have non-halo particles to
accommodate in Particles[].
The next step is very level-4s’s own and calls make_bxfer_sched() to build the transfer
schedule of particles in vertical halo planes. Note that calling the function here means that
NOfPGrid[][][] for move_to_sendbuf_4s() should have non-negative values less than 232
indicating that particles in each grid-voxels stay in the local node or are sent to other node
which the array element specifies. This fact is important when helpand-helper reconfigura-
tion takes place, and thus a grid-voxel in the old secondary subdomain having particles for
other nodes can be included in the vertical halo planes of new secondary subcuboid. That
is, the particles in such grid-voxel are at first packed in Particles[] or moved to SendBuf[]
properly by move_to_sendbuf_4s() as NOfPGrid[][][] indicates and then transferred, and
after that the array elements for grid-voxels in vertical interior halo plane and exterior
pillar are let have special values to copy particles in them to BoundarySendBuf[] by sort_
particles(), being slightly different from its level-4p version because of this operation.
This is the reason why we perform the partially position-aware transfer on helpand-helper
reconfiguration.
On the other hand,  if helpand-helper reconfiguration does not take place and Qn +
Pnsend ≤Plim, we at first call make_bxfer_sched() to let NOfPGrid[][][] for grid-voxels
in vertical interior halo plane and exterior pillar have special values. Since particles in a
grid-voxel in a vertical interior halo plane definitely stays in the local node, it is safe to
let the element of NOfPGrid[][][] for the grid-voxel have a negative value because it should
have been 0. Then we move particles in Particles[] to SendBuf[] sorting those staying


<!-- Page 490 -->

in the primary/secondary subcuboid by move_and_sort() being the level-4s version of
move_and_sort_secondary() but it works even on the case of p′n = 0, while its argu-
ment set is almost equivalent to the level-4p counterpart except for nacc[1] as discussed
above with move_to_sendbuf_4s().  In addition,  it copies particles in vertical interior
halo planes to BoundarySendBuf[] as in sort_particles(). Then we transfer particles
by xfer_particles() and then move received particles in rbuf (p, s) to SendBuf[] sorting
them by sort_received_particles(), which is slightly different from its level-4p version
again because it also copies particles in vertical interior halo planes to BoundarySendBuf[].

if (trans || (dint)nacc[1]+(dint)nsend>(dint)nOfLocalPLimit) {
move_to_sendbuf_4s(nextmode, psold, psnew, trans, oldp, nacc, nsend,
stats);
xfer_particles(trans, psnew, nextmode, SendBuf);
make_bxfer_sched(trans, psnew, rlist, rlidx);
sort_particles(nextmode, psnew, stats);
} else {
make_bxfer_sched(0, psnew, rlist, rlidx);
move_and_sort(nextmode, psold, psnew, oldp, nacc, stats);
xfer_particles(trans, psnew, nextmode, SendBuf+nacc[1]);
sort_received_particles(nextmode, psnew, stats);
}

The last part of this function is very level-4s’s own. We call xfer_boundary_particles_
v() twice to transfer particles in vertical halo planes, for yz-planes with argument d = 0
at first and then for xz-planes with d = 1, relaying the particles in exterior pillars from
west/east neighbor to south/north one. Then we call xfer_boundary_particles_h() to
transfer those in horizontal halo planes including those received by the calls of xfer_
boundary_particles_v(). The argument psnew = pn is commonly passed to them to
specify whether secondary halo particles are transfered, and t = trans is given to the former
to specify that GridDesc[t + 1] is referred in For_All_Grid() to scan vertical exterior halo
planes in per-grid histogram and per-grid index to find the locations for received secondary
halo particles.

xfer_boundary_particles_v(psnew, trans, 0);
xfer_boundary_particles_v(psnew, trans, 1);
xfer_boundary_particles_h(psnew);
}

Here we revisit the issue that this function should work well not only in the case of
secondary mode in the next step as the level-4p counterpart works but also of primary mode,
showing data structures for position-aware particle transfer have followings consistent with
primary mode case.

- NOfPGrid[0][][] has been set according to PrimaryCommList[0][] so that all primary
particles in the interior of n’s primary subdomain stay in n while all in the exterior are
sent to n’s neighbors. As for those in vertical interior halo planes, they have indices of
BoundarySendBuf[] for n’s neighbors whose primary subdomains share vertical sur-
faces with n’s ones. Those in exterior pillars also have indices of BoundarySendBuf[]
for n’s south/north neighbors to relay particles from west/east ones.  If n has sec-
ondary particles, NOfPGrid[1][][] are set according to PrimaryCommList[1][] so that
all of them are sent to npold or its neighbors.  Since make_bxfer_sched() does not
modify anything in NOfPGrid[1][][], move_and_sort() should properly work on old
secondary particles to send them to the nodes.


<!-- Page 491 -->

- NOfPGridOut[0][][] has been set according to the primary receiving block in Primary
CommList[0][⌊3D/2⌋] and thus has NOfPGridTotal[0][][] for  all grid-voxels in n’s
primary subdomain and its horizontal exterior halo planes. On the other hand,
NOfPGridOut[1][][] is meaningless because it has not been modified when n will not
have secondary subdomain, but will not be referred to.

- ZBound[0][] has been set according to PrimaryCommList[0] and thus has {0, δz(n)},
while ZBound[1][] has not been modified after its reinitialization and thus has {0, 0}.

- HPlane[0][] has been set according to PrimaryCommList[0] and thus has transfer sched-
ules with n’s neighbors below and above its primary subdomain. HPlane[1][] is mean-
ingless because it has not been modified, but will not be referred to.

- VPlane[] has been set according to PrimaryCommList[0] and thus has transfer sched-
ules with n’s neighbors whose primary subdomains share vertical surfaces of n’s one.
VPlaneHead[d][p][β] = h4d+2p+β is also set properly according to the number of trans-
fer schedules, h0 = 0, hi = hi−1 + {0, 1} for i ∈{1, 4} according to the {0, 1, 2, 3} for
the first four elements and 4 for remaining 5 elements, in fact.

- GridDesc[0] has never been modified after the initilization for n’s primary subdomain
in init4s(), and,  if n has secondary subdomain, GridDesc[1] has also been kept
unchanged since the last helpand-helper reconfiguration having descriptors for the
subdomain.

- TotalPNext[0][] has been set according to PrimaryCommList[0][⌊3D/2⌋] and thus has
the number of primary particles n will accommodate in the next step. TotalPNext[1][]
has been zero-cleared and has not been modified after that.

- NOfSend[0][][] has been set according to PrimaryCommList[][] which tells us all par-
ticles sent to neighbors of n, and parent(n) if any, are primary for them. Therefore
NOfSend[1][][] has not been modified after zero-clearing in the last step or in this
function with anywhere accommodation. Since RealDstNeighbors[0][0] is kept un-
changed even when we were in secondary mode, elements in NOfSend[0][] have been
sent to the nodes whose primary subdomains are neighbors of n and parent(n) if any.

- NOfRecv[0][][] has been set according to RealSrcNeighbors[0][0] which is kept un-
changed even when we were in secondary mode.  Therefore, its elements have the
number of n’s primary particles currently accommodated by the nodes whose primary
or secondary subdomains are neighbors of n’s primary subdomain. NOfRecv[1][][] is
meaningless because it has not been modified since the last step, but will not be
referred to.

- GridOffset[0][] has never been modified after the initilization for n’s primary subdo-
main in init4s(), and, if n has secondary subdomain, GridOffset[1][] has also been
kept unchanged since the last helpand-helper reconfiguration having proper offsets for
neighbors of the subdomain.

- RealDstNeighbors[0][0] and RealSrcNeighbors[0][0] has been kept unchanged since
the last helpand-helper reconfiguration so that the former tells us the set of nodes
whose primary subdomains are neighboring to n’s primary and secondary subdomains
while the latter does those whose primary or secondary subdomains are neighboring
to n’s primary subdomain. Other elements [t][p] where t ̸= 0 or p ̸= 0 are meaningless
and thus will not be referred to.


<!-- Page 492 -->

#### 4.13.15 count_population()

count_population()  The function count_population(), called solely from exchange_particles4s() with any-
where accmmodation, counts the particle population in each grid-voxel to have the per-grid
histogram in NOfPGrid[][][] after we perform non-position-aware particle transfer.  It also
lets each of Particles[].nid have the subdomain code ⌊3D/2⌋and the grid-position in
the primary/secondary subdomain of the local node, copies TotalPNext[] to TotalP[], lets
primaryParts and totalParts have the numbers of primary and all particles, and lets
nOfInjection=0, as if we had the result of the non-position-aware particle transfer at the
call of oh4s_transbound().
The function is perfectly equivalent to its level-4p counterpart described in §4.10.36.
Only one difference is that the function is now called solely from exchange_particles4s()
regardless of the execution mode in next step, because try_primary4s() does not have its
own particle transfer mechanism which try_primary4p(), the other caller in level-4p, has84.


static void
count_population(const int nextmode, const int psnew, const int stats) {
int ps, s, t, i, j, tp;
const int ns=nOfSpecies, exti=OH_PGRID_EXT;
Decl_For_All_Grid();
Decl_Grid_Info();

if (stats) oh1_stats_time(STATS_TB_SORT, nextmode);
for (ps=0,t=0,j=0,tp=0; ps<=psnew; ps++) {
for (s=0; s<ns; s++,t++) {
dint *npgs = NOfPGrid[ps][s];
const int tpn = TotalP[t] = TotalPNext[t];
tp += tpn;
For_All_Grid(ps, -exti, -exti, -exti, exti, exti, exti)
npgs[The_Grid()]=0;
for (i=0; i<tpn; i++,j++) {
const int g = Grid_Position(Particles[j].nid);
npgs[g]++;
Particles[j].nid = Combine_Subdom_Pos(OH_NBR_SELF, g);
}
}
if (ps==0)  primaryParts = tp;
}
totalParts = tp;  nOfInjections = 0;
}


#### 4.13.16 exchange_population()

exchange_population()  The function exchange_population(), called solely from exchange_particles4s(), sums
up local per-grid histograms in the local node’s primary family if we have secondary mode
configuration85 indicated by currmode argument, and then gathers the per-grid histograms
in neighbors’ halo planes to have the complete per-grid histogram in NOfPGridTotal[0][][].

84Therefore, the argument stats is meaningless because it is always 0 and nextmode as well because it
is meaningful only when stats is non-zero, but we keep them to make this function perfectly equivalent to
the level-4p counterpart.
85That is, we were in secondary mode with normal accommodation or will be in secondary mode with
anywhere accommodation.


<!-- Page 493 -->

The function is similar to its level-4p counterpart described in §4.10.15, but it has various
differnces as follows.

- Since we always need per-grid histograms in neighbors’ halo planes for halo par-
ticle transfer,  this function  is called regardless of the current and next execu-
tion mode and accommodation pattern to have the complete per-grid histogram in
NOfPGridTotal[0][][] always.

- Since we need per-plane histogram in NOfPGridZ[], this function calculate the particle
population in each xy-plane of interior of the local node’s primary subdomain.

- Since the receiving planes to recieve the per-grid histograms in neighbors’ halo planes
are 2eg thick and they are added to those in local node’s halo planes being eg thick for
each of interior and exterior ones, the arguments of add_population() are level-4s
specific.

- Since the level-4s library is only for 3-dimentional simulations, dimension-dependent
constructs are elminated.


static void
exchange_population(const int currmode) {
const int ns=nOfSpecies;
int s, zz;
dint **npg = NOfPGridTotal[0];
const int ct=nOfExc-1;
const int ext = OH_PGRID_EXT,  ext2 = ext<<1,  ext3 = ext*3;
const int x = GridDesc[0].x,  y = GridDesc[0].y,  z = GridDesc[0].z;
const int w = GridDesc[0].w,  dw = GridDesc[0].dw;
Decl_For_All_Grid();


At first,  if we have secondary mode configuration, we sum up local per-grid his-
tograms NOfPGrid[p][][] in the local node’s primary family, where p = 0 for the local
node while p = 1 for its helpers, to have the sum in NOfPGridTotal[0][][] by reduce_
population(). On the other hand, if we have primary mode configuration, we copy ele-
ments NOfPGrid[0][s][gidx(x, y, z)] to the corresonding elements in NOfPGridTotal[][][] us-
ing For_All_Grid() for all s ∈[0, S) and (x, y, z) ∈[−eg, δx(n)+eg) × [−eg, δy(n)+eg) ×
[−eg, δz(n)+eg) for the local node n, because we need to keep NOfPGrid[][][] unchanged for
particle transfer86. Therefore, the base per-grid histogram is built in NOfPGridTotal[0][][]
always unlike in level-4p.

if (Mode_PS(currmode))  reduce_population();
else {
for (s=0; s<ns; s++) {
dint *npgs = NOfPGrid[0][s],  *npgt = npg[s];
For_All_Grid(0, -ext, -ext, -ext, ext, ext, ext)
npgt[The_Grid()] = npgs[The_Grid()];
}
}

86This copy cannot be done calling reduce population() blindly because the prime element of MyComm is
not meaningful with primary mode configuration and thus not necesarily be MPI COMM NULL
