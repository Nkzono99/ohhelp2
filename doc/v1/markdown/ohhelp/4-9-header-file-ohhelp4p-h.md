# 4.9 Header File ohhelp4p.h

Source: `doc/v1/original/ohhelp.pdf`, pages 325-345.

<!-- Page 325 -->

## 4.9 Header File ohhelp4p.h

The header file of level-4p library, ohhelp4p.h, #defines a few macros for per-grid histogram
and grid-position. Then it declares global variables and their structures used in level-4p
library codes to keep per-grid histograms and their shapes, to make the particle transfer
schedule, to store neighboring node information, and to check the boundary conditions of
the system domain. Finally it gives prototypes of API functions.


#### 4.9.1 Constants

At first we define the following constants.

OH_PGRID_EXT     • The constant OH_PGRID_EXT = eg = 1 is the inner extension of per-grid histogram to
mean that particles in a subdomain can move at most one grid outside the subdomain.
Though it is very unlikely that we have a PIC simulator in which paticles can travel
two or more grids with position-aware particle management, modifying this definition
may cope with such a imaginary implementation. Note that the outer extension of
per-grid histogram is 2eg to have additional receiving plane(s) outside the sending
plane(s) both of which are eg thick.
∑D−1
OH_NBR_SELF     • The constant OH_NBR_SELF =   d=0 3d = ⌊3D/2⌋is the index of Neighbors[3D] of a
node and its relatives for the node itself. It is used to know whether we refer to the
node itself when, for example, we scan its neighbors.

Here we revisit a few other constants/switches related to level-4p extension.

OH_LIB_LEVEL_4P     • The switch OH_LIB_LEVEL_4P can be defined in oh config.h to declare that the OhHelp
library should be configured with level-4p extension, as discussed in §3.3.

OH_POS_AWARE     • The switch OH_POS_AWARE can be defined in ohhelp1.h to make lower level libraries
position-aware.  So  far  it  is equivalent to OH_LIB_LEVEL_4P because ohhelp1.h
#defines it iffOH_LIB_LEVEL_4P is defeined as well as discussed in §4.2.2, but may
be not in future with further or another extensions.

STATS_TB_SORT     • The constant STATS_TB_SORT can be defined in oh stats.h as the timing statistics key
to measure the time for particle sorting as discussed in §3.10.1.


#define OH_PGRID_EXT 1
#define OH_NBR_SELF (OH_NEIGHBORS>>1)



#### 4.9.2 Macros for Grid-Position

For the particle sorting, we need that the particle structure S_particle has the grid-
position of the particle.  Moreover, with normal accommodation, we need to know the
neighbor index k of the subdomain m where the particle resides. To minimize the spatial
impact on the particle buffers, we encode the grid-position and k or m in the nid element of
S_particle, rather than add an element for grid-position nor k. More specifically, the nid
element i of a particle residing in the subdomain m, which can be the k-th neighbor of the


<!-- Page 326 -->

local node n’s primary/secondary subdomain n′ ∈{n, parent(n)}, and in the subdomain’s
grid-voxel of index g has the following.

 x                   D = 1
gidx(x, y, z) =  x + (δmaxx  + 4eg)y           D = 2
                                    x + (δmaxx  + 4eg)(y + (δmaxy  + 4eg)z) D = 3

Γ = ⌊log2 gidx(δmaxx  −1, δmaxy  −1, δmaxz  −1)⌋+ 1
{
k    m is k-th neighbor of n′ definitely
σ =
m + 3D  otherwise
i = σ · 2Γ + g

That is, Γ has the minimum number of bits to represent the largest possible one-dimensional
index of per-grid histogram whose origin is for (0, · · · , 0) of the local coordinate of a sub-
∏D−1
domain and size is G =   d=0 (δmaxd  + 4eg) including eg thick sending and receiving planes
at lower and upper boundaries of the subdomain.  Therefore, the subdomain code σ for
nid i can be obtined by ⌊i/2Γ⌋or by shifting right i by Γ bits, while g is (i mod Γ) or is
extracted by a bitwise-AND of i and 2Γ −1. Then the subdomain identifier m is obtained
from σ by;                            {
AbsNeighbors[p][σ]  σ < 3D                     m =
σ −3D           σ ≥3D

where AbsNeighbors[p][k] is the subdomain identifier of k-th neighbor of the local node’s
primary (p = 0) or secondary (p = 1) subdomain.
In addition, the nid element i can temporarily have a special value in its subdomain
code part, (N +3D)+σ, for secondary particles injected into the subdomain m represented
by σ, to distinguish them from primary injected particles. Therefore, the largest possible i
is (2(N + 3D) −1) · 2Γ which should not be greater than the largest int value, 231 −1, or
we need to represent nid by 64-bit integer long␣long␣int.
Prior to showing the macros defined in ohhelp4p.h, here we revisit a switch, a data type,
variables and macros defined in other header files.

OH_BIG_SPACE     • The switch OH_BIG_SPACE can be defined in oh config.h to declare that per-grid his-
tograms are too large to represent nid by int and thus it should be long␣long␣int.

OH_nid_t     • The data type OH_nid_t  is int  if OH_BIG_SPACE  is undefined by  default,  or
long␣long␣int otherwise. The typedef of this data type is in oh part.h, while its
Fortran counterpart oh type.F90 just has #ifdef/#else/#endif construct to declare
the nid element with corresponding size.

logGrid     • The integer variable logGrid and gridMask declared in ohhelp2.h have Γ and 2Γ −1
gridMask        respectively. The values are assigned to them by init4p() and the variables are
referred to only through the macros discussed below.

AbsNeighbors     • The two dimensional array AbsNeighbors[2][3D] declared in ohhelp2.h has the follow-
ing value in its element [p][k].
{
Neighbors[p][k]         Neighbors[p][k] ≥0
AbsNeighbors[p][k] =
−(Neighbors[p][k] + 1)  Neighbors[p][k] < 0

That is the array is an always-positive version of Neighbors[][] unaware of multiple oc-
currences of subdomains in a neighbor set. Therefore, the elements [0][] are initialized


<!-- Page 327 -->

by init4p() through its callee update_neighbors() after Neighbors[0][] are initial-
ized, and [1][] are initialized/updated in rebalance4p() or exchange_particles4p()
when Neighbors[1][] are set to neighbors of the newly assigned local node’s sec-
ondary subdomain through their callee update_neighbors().  Besides the initial-
izer/updater update_neighbors(), the array is reffered to by oh4p_map_particle_
to_neighbor() directly and other functions through the macro Subdomain_Id() or
Neighbor_Subdomain_Id().

Decl_Grid_Info()     • The macro Decl_Grid_Info() with no arguments, defined in ohhelp2.h, declares local
variables named gridmask and loggrid to cache the corresponding global variables
gridMask and logGrid in them respectively, and subdomid to have σ temporarily, so
that the local variables are referred to by the related macros. The macro is used in
the following functions;

rebalance4p(), count_population(), sort_particles(),
move_and_sort_primary(), sort_received_particles(),
move_to_sendbuf_sec4p(), move_to_sendbuf_uw4p(),
move_to_sendbuf_dw4p(), move_and_sort_secondary(),
oh4p_map_particle_to_neighbor(), oh4p_map_particle_to_subdomain(),
oh4p_remove_mapped_particle().

Subdomain_Id()     • The macro Subdomain Id(i, p), defined in ohhelp2.h, extracts the subdomain code σ
of nid element i of a primary (p = 0) or secondary (p = 1) particle by calculating
σ = ⌊i/2Γ⌋and then translates it to the subdoamin identifier represented by σ. This
macro is used in move_and_sort_primary() and oh4p_remove_mapped_particle(),
when it is not sure that σ ∈[0, 3D) due to secondary injected particles for the former
and anywhere-accommodated ones for the latter.

Primarize_Id()     • The macro Primarize Id(π, m), defined in ohhelp2.h, removes the secondary particle
flag attached to the particle pointed by π by subtracting (N + 3D) · 2Γ from its nid.
It also gives the identifier m of the subdomain into which the particle is injected.
This macro is used in rebalance4p(), move_and_sort_primary() and oh4p_remove_
mapped_particle(), which need m as well as the primarization.

Note that the last three macros neither do anything nor are defined at all if OH_POS_AWARE
is not defined.
Now we show a few macros used only by functions in level-4p library.

Grid_Position()     • The macro Grid Position(i)  extracts the  grid-position part  of  i by perform-
ing  bitwise-AND  on  it  with  2Γ −1 =  gridMask.    This  macro  is  used
in rebalance4p(),  count_population(),  sort_particles(),  move_and_sort_
primary(), sort_received_particles(), oh4p_remove_mapped_particle()  di-
rectly, and  in move_to_sendbuf_sec4p(), move_to_sendbuf_uw4p(), move_to_
sendbuf_dw4p() and move_and_sort_secondary() through the macro Move_Or_
Do().

Combine_Subdom_Pos()     • The macro Combine Subdom Pos(σ, g) combines the subdomain code σ and the grid-
position g to have σ · 2Γ + g. This macro is used in count_population(), oh4p_map_
particle_to_neighbor() and oh4p_map_particle_to_subdomain().

Primarize_Id_Only()     • The macro Primarize Id Only(π) removes the flag for secondary injected particles
as its relative Primarize_Id() does, but does not give the subdomain identifier to its


<!-- Page 328 -->

invoker which does not care that. This macro is used in move_to_sendbuf_sec4p()
and move_and_sort_secondary().

Secondarize_Id()     • The macro Secondarize Id(π) flags that the particle pointed by π is injected to the
secondary subdomain of the local node, or other (neighbor of secondary subdomain,
very likely) subdomain as a secondary particle, by adding (N +3D)·2Γ to its nid. This
macro is used in rebalance4p(), oh4p_map_particle_to_neighbor() and oh4p_
map_particle_to_subdomain().

Secondary_Injected()     • The macro Secondary Injected(i) is replaced with true iffi being the nid of a
particle has a subdomain code not less than N + 3D, i.e., the particle is injected as
a secondary one. This macro is used in rebalance4p(). move_to_sendbuf_sec4p()
and move_and_sort_secondary().

Neighbor_Subdomain_Id()     • The macro Neighbor Subdomain Id(i, p) is a simplified version of Subdomain_Id()
to be used  if it is assured that the subdomain code part in i is in [0, 3D),  i.e.,  if
the macro is used with normal accommodation or after the first phase non-position-
aware particle transfer with anywhere one, and  i  is of a particle not being sec-
ondary injected or that primarized.  Therefore, the macro is simply replaced with
AbsNeighbors[p][⌊i/2Γ⌋]. This macro is used in move_and_sort_primary() directly,
and in move_to_sendbuf_sec4p(), move_to_sendbuf_uw4p(), move_to_sendbuf_
dw4p() and move_and_sort_secondary() through the macro Move_Or_Do().


#define Grid_Position(ID)         ((ID)&gridmask)
#define Combine_Subdom_Pos(ID, G) (((OH_nid_t)(ID)<<loggrid) + (G))
#define Primarize_Id_Only(P) \
(P)->nid -= (OH_nid_t)(nOfNodes+OH_NEIGHBORS)<<loggrid
#define Secondarize_Id(P) \
(P)->nid += (OH_nid_t)(nOfNodes+OH_NEIGHBORS)<<loggrid
#define Secondary_Injected(ID) \
((ID>>loggrid)>=nOfNodes+OH_NEIGHBORS)
#define Neighbor_Subdomain_Id(ID, PS) \
AbsNeighbors[PS][(ID)>>loggrid]



#### 4.9.3 Per-Grid Histograms and Related Variables

Next, we declare the following arrays of per-grid histograms and related variables.

PbufIndex     • The element of [p][s] of the integer array PbufIndex[2][S] has the head index of
pbuf (p, s) in Particles[]. That is, each of its element has the following value for
the local node n.

∑p−1 S−1∑     ∑s−1
PbufIndex[p][s] =        TotalP[q][t] +    TotalP[p][t]

q=0 t=0               t=0

Note that the array has an additional conceptual element [2][0] defined by the equation
above and thus having Qn = totalParts. The array is initialized by oh4p_init()
with NULL and then allocated by the first call of transbound4p() which also sets all el-
ements in each call, while referred to by oh4p_map_particle_to_neighbor(), oh4p_
map_particle_to_subdomain() and oh4p_remove_mapped_particle() through the
macro Check_Particle_Location().


<!-- Page 329 -->

NOfPGrid     • The dint type double pointer arrays NOfPGrid[2] and NOfPGridTotal[2] are for the
NOfPGridTotal        per-grid histograms of conceptually three-dimensional arrays of [2][S][G] whose ele-
ment [p][s][g] has the number of primary (p = 0) or secondary (p = 1) particles of
species s in the grid-voxel whose index is g, namely PL(p, s, g) and PT (p, s, g) re-
spectively.  Their body dint arrays and pointer arrays are allocated by init4p()
using Allocate_NOfPGrid() which also zero-clears their bodies.  NOfPGrid[][][] is
also zero-cleared by transbound4p() as one of its post-process to give the base of
the counting. Then oh4p_map_particle_to_neighbor() or oh4p_map_particle_
to_subdomain() increments entries of NOfPGrid[][][] for particles accommodated by
the local node, while oh4p_remove_mapped_particle() may decrement some of them
to cancel the increments for particles which were once recoginized existing but then
vanish. Therefore at the call of oh4p_transbound(), NOfPGrid[][][] should have the
per-grid histogram after the one-step travel of particles.

Then in the functions called from transbound4p(), these two arrays play the following
roles depending on the mode in the last and next simulation steps (mc and mn) and
the accommodation pattern a.

  - a = normal, mn = primary, mc = primary
The complete per-grid histogram  is  bulit  in NOfPGrid[0][][] by exchange_
population() through the neighboring communication of its boundary plane.
Then NOfPGrid[][][] is referred to by sort_particles() or move_and_sort_
primary() for sorting.

  - a = normal, mn = primary, mc = secondary
The complete per-grid histogram is built in NOfPGridTotal[0][][] by summing
up NOfPGrid[][][] in the primary family members and by exchanging bound-
ary planes of neighbors in exchange_population() and  its callee reduce_
population(). Then NOfPGridTotal[0][][] is referred to by sort_particles()
or move_and_sort_primary() for sorting, while NOfPGrid[][][] is kept unchanged
but not referred to.

  - a = normal, mn = secondary, mc = primary
The contents of NOfPGrid[0][][]  is copied into NOfPGridTotal[0][][] in which
the complete per-grid histogram is built by exchange_population().  Then
NOfPGridTotal[0][][] is referred to by sched_recv() to determine the distribu-
tion of primary particles among primary family members, and then broadcased
to new helpers’ NOfPGridTotal[1][][] in make_recv_list() so that helpers knows
the number of particles in each grid-voxel which they have to host in make_send_
sched_body(). On the other hand, NOfPGrid[0][][] is kept unchanged to be re-
ferred to know the number of particles in each grid-voxel, by make_send_sched_
body(), gather_hspot_send_body(), scatter_hspot_send() and scatter_
hspot_recv_body(), in case that they are sent out to other nodes.

  - a = normal, mn = secondary, mc = secondary
The complete per-grid histogram is built in NOfPGridTotal[0][][] by summing
up NOfPGrid[][][] in the primary family members and by exchanging bound-
ary planes of neighbors in exchange_population() and  its callee reduce_
population(). Then both arrays are referred to as discussed above for mc =
primary.

  - a = anywhere, mn = primary
Since the contents of NOfPGrid[0][][] is not useful, the complete per-grid his-


<!-- Page 330 -->

togram is rebuilt in it by count_population() after the non-position-aware
particle transfer. Then it is referred to by sort_particles() for sorting.
  - a = anywhere, mn = secondary
Since the contents of NOfPGrid[][][] is not useful, the local per-grid histogram
is rebuilt in it by count_population() after the non-position-aware particle
transfer. Then the complete per-grid histogram is built in NOfPGridTotal[0][][]
by reduce_population() keeping NOfPGrid[][][] unchanged. Then both arrays
are referred to as discussed in the case of a = normal, mn = secondary and
mc = primary.

In addition, each of two arrays has another role.  If the next step is in secondary
mode, NOfPGrid[][][] is modified by make_send_sched_body() and scatter_hspot_
recv_body() to have one of the followings after its original contents are examined.

  - 0 means that the particles in the grid-voxel stays in the local node.

  - A positive number (pS + s)N + m + 1 means that the particles of species s in
the grid-voxel is sent to the node m as its primary (p = 0) or secondary (p = 1)
particle.
  - A negative number −(i + 1) means that the hot-spot sending schedule for the
particles in the grid-voxel is found in CommList[i].

The  functions move_to_sendbuf_sec4p(),  move_to_sendbuf_uw4p(), move_to_
sendbuf_dw4p() and move_and_sort_secondary() refer to NOfPGrid[][][] with this
role.

On the other hand, NOfPGridTotal[][][] is modified by sort_particles(), move_
and_sort_primary() or move_and_sort_secondary() to have the sorting index of
SendBuf[] to which the particle in the grid-voxel is moved. That is, for the local node
n, it is initialized as follows and then its element is incremented each time a particle
in the grid-voxel is moved by the functions above and sort_received_particles().
{
n            p′ = 0
n(p′) =
parent(n)   p′ = 1

δd(m) = δud(m) −δld(m)
Rd(p′) = [0, δd(n(p′)))
G(p′) = {g | g = gidx(x, y, z), x ∈Rx(p′), y ∈Ry(p′), z ∈Rz(p′)}
NOfPGridTotal[p][s][g] =
∑p−1 S−1∑ ∑
NOfPGridOut[p′][s′][g′] +
p′=0 s′=0 g′∈G(p′)
∑s−1 ∑        ∑
NOfPGridOut[p][s′][g′] +    NOfPGridOut[p][s][g′]
s′=0 g′∈G(p)                        g′∈G(p), g′<g

NOfPGridOut     • The int type double pointer array NOfPGridOut[2] is for the local per-grid histogram
of conceptually three-dimensional array of [2][S][G] whose element [p][s][g] has the
number of primary (p = 0) or secondary (p = 1) particles of species s in the grid-
voxel whose index is g, namely PO(p, s, g). The per-grid histogram reflects the particle
transfer and is given to the simulator body so that it captures particles in a particular
grid-voxel.


<!-- Page 331 -->

The body int array of NOfPGridOut[][][] is given by the simulator body through the
double-pointer argument of pghgram of oh4p_per_grid_histogram(), or is allocated
by the function using Allocate_NOfPGrid() if the argument points NULL. In both
cases, its pointer arrays are allocated by the function and the body is zero-cleared.
Then if the next step is in primary mode, each entry is set to the number of parti-
cles by sort_particles() or move_and_sort_primary(). Otherwise, each entry is
set by make_send_sched_body(), scatter_hspot_send() or scatter_hspot_recv_
body(), and then referred to by sort_particles(), move_and_sort_primary() or
move_and_sort_secondary() to build the sorting index in NOfPGridTotal[][][].

S_griddesc     • The array GridDesc[3] of the S_griddesc structure with the following elements has
GridDesc       the shape information of per-grid histogram for a subdomain m.

  - x, y and z have δd(m) = δud(m)−δld(m) where d = 0 for x, d = 1 for y, and d = 2
for z. That is, each element has the upper bound of m’s local coordinate for the
corresponding dimension. Note that if D < 3, y and/or z is set to 0. Also note
that if the sudomain m does not exist because, for example, it is the secondary
subdomain of the root of helpand-helper tree, these elements are set to −4eg to
ensure that adding any possible upper bound offset in [0, 2eg) to it should give
a result not greater than −2eg, the absolute lower bound of the D-dimensional
index of per-grid histogram.
  - w, d and h have δmaxd  + 4eg where d = 0 for w, d = 1 for d, and d = 2 for h. That
is, each element has the size of conceptual D-dimensional element array of the
per-grid histogram. The values of the elements are common for all GridDesc[]
array elements. Note that if D < 3, d and/or h is set to 1.

  - dw is w×d. Therefore, the one dimensional per-grid histogram index gidx(x, y, z)
is x + d · y + dw · z.

Each array element is set by set_grid_descriptor() called by one of the following
functinos.

  - [0] for the primary subdomain is permanently set by the call from init4p().

  - [1] for the secondary subdomain  is set by the call from rebalance4p() or
exchange_particles4p() to reflect the helpand-helper reconfiguration with
normal or anywhere accommodation respectively.

  - [2] for the newly assigned secondary subdomain due to the helpand-helper re-
configuration is set by the call from make_recv_list(), keeping [1] unchanged
for the transitional state.

Then  the  array   is  referred  to  by  oh4p_per_grid_histogram(),  exchange_
population(), sched_recv(), make_send_sched_body(), gather_hspot_recv(),
oh4p_map_particle_to_neighbor(), oh4p_map_particle_to_subdomain() directly,
and in transbound4p(), add_population(), count_population(), sort_particles
(), move_and_sort_primary() and move_and_sort_secondary() through the macro
For_All_Grid() or For_All_Grid_Abs().


EXTERN int *PbufIndex;                                  /* [2*ns+1] */
EXTERN dint **NOfPGrid[2], **NOfPGridTotal[2];          /* [2][ns][z][y][x] */
EXTERN int **NOfPGridOut[2];                            /* [2][ns][z][y][x] */


<!-- Page 332 -->

struct S_griddesc {
int x, y, z, w, d, h, dw;
};
EXTERN struct S_griddesc GridDesc[3];


In addition, we use the following variables a little bit diffenently from their usages in
lower level libraries.

Particles     • Each of two particle buffers Particles[Plim] and SendBuf[Plim] is now a half of the
SendBuf        larger buffer of 2Plim particles. Therefore, the simulator body must give the (dou-
ble) pointer to this larger buffer through the argument pbuf of oh4p_init(), or will
receive the pointer to the buffer allocated by oh4p_init(). Moreover, Particles[]
and SendBuf[] exchanges their roles each time oh4p_transbound() is called. That is,
Particles[] has all partcles accommodated by the local node at the call of the func-
tion, but those which stay in the local node in the next step are moved to SendBuf[]
together with those received from other nodes to make SendBuf[] becomes Particles[]
in the next step by transbound4p(). Therefore, simulator body must switch the pat-
icle buffer which it processes from/to the first half to/from the second half each time
it calls oh4p_transbound().
The  functions sort_particles(),  move_and_sort_primary(), sort_received_
particles(),  move_to_sendbuf_sec4p(),  move_to_sendbuf_uw4p(),  move_to_
sendbuf_dw4p(), move_and_sort_secondary() and xfer_particles()  refer to
and/or  modify  both  Particles[] and  SendBuf[]  directly  or  indirectly,  while
count_population(), oh4p_map_particle_to_neighbor(), oh4p_map_particle_
to_subdomain(), oh4p_inject_particle() and oh4p_remove_mapped_particle()
refer to and/or modify only Particles[] directly or indirectly.

nOfLocalPLimit     • The  variable nOfLocalPLimit =  Plim  is now calculated by oh4p_max_local_
particles() to let it
⌈               ⌉
Plim = max( P(100 + α)/100  , P + minmargin) + 4Phot

to ensure we have the margins of 2Phot for each of primary and secondary particle
sets. Moreover, oh4p_max_local_particles() keeps its calculation result in a vari-
able nOfLocalPLimitShadow private to level-4p functions so that init4p() ensures
that oh4p_max_local_particles() had been called and its result is not less than
that given by maxlocalp arguement of init4p(). The variable is referred to by
level-4p functions try_primary4p(), exchange_particles4p() and oh4p_inject_
particle().

NOfPLocal     • The per-subdomain local particle population histogram NOfPLocal[2][S][N] does
not change  its role but  it  is now private to level-4p library.  Therefore, oh4p_
init() does not has the argument nphgram which lower-level’s counterparts have,
and counting the per-subdomain population  is perfectly up to the library func-
tions, oh4p_map_particle_to_neighbor(), oh4p_map_particle_to_subdomain()
and oh4p_remove_mapped_particle(). In level-4p library NOfPLocal[][][] is referred
to by transbound4p(), try_primary4p() and move_and_sort_primary().

RecvBufBases     • The pointer array RecvBufBases[2][S] each element [p][s] of which points rbuf (p, s)
does not change its role but it is now has one extra element conceptually [2][0] so
that sort_received_particles() can know the tail of rbuf (p, s) by referring to the
element at (real) one-dimensional index [pS+s+1].


<!-- Page 333 -->

This  extra  element  is  set by move_and_sort_primary()  or move_and_sort_
secondary() and referred to by sort_received_particles(), while other elements
are referred to also by them and move_to_sendbuf_sec4p(), move_to_sendbuf_
uw4p() and xfer_particles().

- Besides Particles[], SendBuf[], nOfLocalPLimit, NOfPLocal[][][] and RecvBufBases
[][], some other variables for particle buffers and population are also used in the level-
4p functions in their original meanings as follows.

  - NOfPrimaries[][] in make_recv_list(), sched_recv() and
move_and_sort_primary().
  - TotalPGlobal[] in try_primary4p().
  - TotalP[][] by transbound4p(), move_and_sort_primary(),
move_to_sendbuf_uw4p(), move_to_sendbuf_dw4p() and
move_and_sort_secondary().
  - TotalPNext[][] by transbound4p(), make_send_sched(),
make_send_sched_body(), scatter_hspot_send(),
scatter_hspot_recv_body(), count_population(), sort_particles(),
move_and_sort_primary(), move_to_sendbuf_uw4p() and
move_to_sendbuf_dw4p().
  - primaryParts in try_primary4p(), move_to_sendbuf_sec4p() and
move_and_sort_secondary() together with the pointer to its shadow
secondaryBase, while in count_population() solitarily.
  - totalParts in transbound4p() with totalLocalParticles, in
move_to_sendbuf_sec4p(), oh4p_map_particle_to_subdomain(),
oh4p_inject_particle() and oh4p_remove_mapped_particle() directly, and
in oh4p_map_particle_to_neighbor() through the macro
Check_Particle_Location().
  - SendBufDisps[][] in move_and_sort_primary().
  - nOfInjections in transbound4p(), rebalance4p(),
move_and_sort_primary(), move_to_sendbuf_sec4p(),
move_and_sort_secondary(), oh4p_inject_particle() and
oh4p_remove_mapped_particle() directly, and in
oh4p_map_particle_to_neighbor() and
oh4p_map_particle_to_subdomain() through the macro
Check_Particle_Location().
  - InjectedParticles[][] in transbound4p(), rebalance4p(),
move_and_sort_primary(), oh4p_map_particle_to_neighbor(),
oh4p_map_particle_to_subdomain() and oh4p_remove_mapped_particle().

nOfFields     • The integer/structure arrays for field-arrays, FieldTypes[F+1][7] and FieldDesc[F],
FieldTypes      and their size F = nOfFields are extended to have one extra element for per-grid
FieldDesc        histogram. That is, init4p() intercepts its argument ftypes to make its substance
FieldTypes[][] have the following additional entry f = F −1 to its tail.

  - [0] = ε(f) = 1 means that an entry of per-grid histogram has one (32-bit or
64-bit integer) element.
  - [1:2] = {el(f), eu(f)} = {0, 0} means per-grid histogram does not have any
special extensions.


<!-- Page 334 -->

  - [3:4] = {ebl(f), ebu(f)} = {0, 0} means the broadcast of per-grid histogram does
not have any extensions.
  - [5:6] = {erl (f), eru(f)} = {−eg, eg} means the reduction of per-grid histogram
should include its sending planes of eg thick.

Then the FieldTypes[][] is passed to init3(), which calls init_fields() to allocate
and initialize FieldDesc[] including the extra field of per-grid histogram but not
to allocate FieldTypes[][] because it is allocated by init4p(). This call of init_
fields() also makes fsizes[F−1][][] has the D-dimensional size information of per-
grid histogram by which the simulator body can allocate and access NOfPGridOut[][][]
associated with the argument pghgram of oh4p_per_grid_histogram().

In addition, init4p() calls adjust_field_descriptor() after the call of init3()
∏D−1
to add (S −1)  d=0 Φd(F−1) = (S −1)G to FieldDesc[F−1].{bc, red}.size[0] so
that the broadcast and reduction for per-grid histogram are performed on the whole of
[p][S][G] rather than the one array element [p][s][G]. This adjustment is also made by
update_descriptors() called from exchange_particles4p() or make_recv_list()
when the helpand-helper reconfiguration gives a new helpand to the local node and
we had anywhere or normal accommodation respectively.  Note that the elements
of FieldDesc[F−1].{bc, red} are not referred to by the level-3 API functions of
collective communication, but by level-4p functions reduce_population() and make_
recv_list() because the element data type of per-grid histogram is MPI_LONG_LONG_
INT rather than MPI_DOUBLE59.

nOfExc     • The  integer/structure  arrays  for  the boundary communication  of  field-arrays,
BoundaryCommFields       BoundaryCommFields[C+1], BoundaryCommTypes[C][B][2][3] and BorderExc[C][2][D]
BoundaryCommTypes          [2], and their size C = nOfExc are extended to have one extra element for per-grid
BorderExc        histogram. That is, init4p() intercepts its arguments cfields and ctypes to make
their substances BoundaryCommFields[] and BoundaryCommTypes[][][][] have one ad-
ditional entry C−1 for each, to have F−1 for the former and the followings for the
latter.

  - [0][0][] = {−eg, eg, eg} means that the sending plane(s) of the downward com-
munication is just below the lower boundary plane and receiving plane(s) is just
above the upper sending plane(s).
  - [0][1][] = {0, −2eg, eg} means that the sending plane(s) of the upward commu-
nication is just above the upper boundary plane and receiving plane(s) is just
below the lower sending plane(s).

Note that BoundaryCommTypes[C−1][b][][] for all b ∈[1, B) are set to 0 to mean no
communication is performed for non-periodical system boundaries.

Then  the  BoundaryCommFields[] and  BoundaryCommTypes[][][][]  are  passed  to
init3(), which calls init_fields() to allocate and initialize BorderExc[][][][] but
not to allocate BoundaryCommFields[] and BoundaryCommTypes[][][][] because they
are allocated by init4p().   In addition, init_fields() takes special care  of
BorderExc[C−1][][][] to make the base type of the boundary communication MPI_
LONG_LONG_INT rather than MPI_DOUBLE.  Therefore, we may use oh3_exchange_
borders() in exchange_population() for the boundary communication of per-grid

59The difference  is essential  for the reduction but maybe not  for the broadcast because both of
MPI LONG LONG INT and MPI DOUBLE are 64-bit wide. However daring to use oh3 bcast field() is not very
attractive because oh1 broadcast() is simple enough.


<!-- Page 335 -->

histogram knowing that oh3_exchange_borders() assumes the buffer pointers are
double but this erroneous assumption is not harmful because sizeof(double) =
sizeof(dint) = 8.


#### 4.9.4 Variables for Particle Transfer Scheduling

The next variable group is for the particle transfer scheduling. Before showing them, we
revisit the following variables whose usages are slightly different from those in lower level
libraries.

S_commlist     • As done in the level-1 library, we build the secondary mode particle transfer schedule
CommList        in the array of S_commlist structure CommList[]. However, some of the structure
SecRList       elements have meanings different from those in level-1 as follows.
RLIndex
  - rid is the ID r of the node by which particles specified by the record should be
accommodated.

  - region is the grid-position g of the last member of the grid-voxel set, the particles
in which should be accommodated by r. That is, r will accommodate particles
in the grid-voxels whose indices are in (g′, g] where g′ is region of the previous
record or −1 if the record in question is the first one.

  - tag is 0 for primary particles of r or NS for its secondary ones. In addition the
element can be −1 if the record is in hot-spot sending block to indicate that the
hot-spot record is for the particles to be accommodated by the local node.

  - count is 0  if the last grid-voxel at g is not a hot-spot.  Otherwise it has the
number of particles in a hot-spot at g to be accommodated by r, and the record
is followed by records with the same g and non-zero count to specify the set
of nodes which also accommodates particles in the hot-spot and the number of
particles for them.

  - sid is meaningful only for a hot-spot record and has the zero-origin ordinal of
the hot-spot in a subdomain, unless the record is the tail of the sequence of hot-
spot receivers and has −1 to indicate it is the tail60. In hot-spot sending block
introduced later, however, each record has the number of receivers to receive a
hot-spot particles of a species accommodated by a node.

As for the blocks in CommList[], they are similar to those in level-1 but are differnt
from them in various aspects as follows.

primary receiving block is build by each node for particles in its primary subdo-
main to be accommodated by the node itself or its helpers. For a subdomain n,
the records for each member of F(n) appear at most twice, as the last host of a
hot-spot and the first host of the subsequent grid-voxels. Therefore, the size of
this block is at most 2|F(n)| ≤2N.

primary sending block is exchanged by neighboring node (subdomain) pairs. A
node receives the whole primary receiving block from each neighbor for particles
sent from the family members rooted by the node to the family members rooted
by the neighbor. Since we avoid receiving a primary receiving block twice or more
from a neighbor and a node can appear at most two primary receiving blocks as
a helpand and a helper, the size of this block is at most 2 × 2 × N = 4N.

60The ordinal is meaningful only in the head record of the sequence. For non-hot-spot records, the ordinal
is of course meaningless but has that of the next hot-spot if any.


<!-- Page 336 -->

The integer array RLIndex[3D + 1] has the CommList’s index of the first record
of primary receiving block received from k-th neighbor unless the neighbor is
the local node itself for which the index is 0 to mean the primary receiving
block built by the local node itself.  In addition,  if a node appears twice or
more as neighbors of the local node, all the elements of RLIndex[] for the node
commonly have the index of the sole primary receiving block received from the
node.  Another remark is that RLIndex[3D] has the index of the record just
following the primary sending block, or the combined size of primary receiving
and primary sending blocks in other words.
secondary receiving block for a node is the copy of primary receiving block of its
helpand which broadcasts the block to its helpers to show them particle accom-
modations for its primary subdomain and thus helper’s secondary subdomain.
Therefore, the size of this block is at most 2N. The pointer SecRList points the
head of this block as done in level-1 library.
secondary sending block for a node is the copy of primary sending block of its hel-
pand which broadcasts the block to its helpers to show them particle transmis-
sions to the subdomains neighboring to its primary subdomain and thus helper’s
secondary subdomain. Therefore, the size of this block is at most 4N.
The integer array SecRLIndex[3D +1] shown later is the copy of RLIndex[] of the
helpand and thus has the offset from SecRList of each primary receiving block
which the helpand receives from its neighbor.
alternative secondary receiving block for a node is the copy of primary receiving
block of its helpand which broadcasts the block to its helpers which become
its family members by rebalancing. A node must refer to both of secondary
receiving blocks gotten from its old and new helpand because the former may
have particle transmissions for its old secondary subdomain. The size of this
block is at most 2N. The pointer AltSecRList shown later points the head of
this block.
hot-spot sending block for a node is for the records by which the node sends par-
ticles in the hot-spots in its primary or secondary subdomain or a subdomain
neighboring them. Unlike other blocks, a record in this block is bulit for a species.
Since a node can have all hot-spots in its primary and secondary subdomains
including their sending planes and every node can host four hot-spots, two for its
primary subdomain and other two for secondary one, this block can have 4NS
records.

The total of the maximum size of each block is (14 + 4S)N and can be greater than
2 · 3D(NS + 1) + N(S + 3) that level-1 requires  if D = 1 and S < 4.  Therefore
init1() takes care of the possiblity and allocates CommList[(14+4S)N] if the former
is greater.

NOfSend     • Unlike non-position-aware case, the particle transfer schedule built in CommList[] is
NOfRecv       not complete because it lacks sender information. This is due to that the node re-
sponsible of a subdomain as its primary one does not know the individual particle
population of each grid-voxel and each node having it. On the other hand, scanning
CommList[] makes each node know which node should accommodate (a part of) parti-
cles in a grid-voxel the node has. Therefore, at first we build a per-receiver histogram
of sending particles in each node and then exchange them among nodes in neighbor-
ing family members to build a per-sender histogram of receiving particles to have the
complete sending/receiving schedule.


<!-- Page 337 -->

We use NOfSend[2][S][N] for the per-receiver sending histogram so that its element
[p][s][m] has the number of particles of species s which the local node should send to
the receiver node m as m’s (not the local node’s) primary (p = 0) or secondary (p = 1)
particles. Each element is accumulated by make_send_sched_body() for particles in
non-hot-spot grid-voxels and scatter_hspot_recv_body() for those in hot-spots.
Then we perform a hand-made all-to-all communication among neighboring family
members in exchange_xfer_amount() to exchange NOfSend[] to have the per-sender
receiving histogram in NOfRecv[2][S][N] so that its element [p][s][m] has the number
of particles of species s which the local node should receive from the sender node m
as the local node’s (not m’s) primary (p = 0) or secondary (p = 1) particles.

In addition, NOfSend[p][s][m] then acts as the index of a portion of SendBuf[],
sbuf (p, s, m), to which a particle of species s to be sent to m as m’s primary (p = 0)
or secondary (p = 1) particle is moved. This role is similar to SendBufDisps[s][m]
for sbuf (s, m) but we need to the additional dimension for [p] because the lower
level’s per-subdomain configuration would shuffle particles of different destinations.
The role change is done by set_sendbuf_disps4p(), and then move_to_sendbuf_
uw4p(), move_to_sendbuf_dw4p() and move_and_sort_secondary() increments an
element each time a particle is moved from Particles[] to SendBuf[] for sending.

Then particles are sent in xfer_particles() referring to NOfSend[][][] for the send
count, each element referred to and thus possibly having non-zero is zero-cleared for
the accumulation in the next call of transbound4p().  All the entries, in addtion,
are also zero-cleared in init4p() at the very beginning and before the first call of
transbound4p().

NOfRecv[][][] also has another role in which its first half is used as an array of [N][S].
In the scattering communication for the particle populations in a hot-spot in the
local node’s primary subdomain, the node sets the element [k][s] to the number of
particles the node itself or its helper should accommodate, where k is the ordinal of the
accommodating node in the hot-spot member nodes. This map is locally manipulated
by scatter_hspot_send().

Requests     • The usage of Requests[] and Statuses[] to keep the requests/statuses of asynchronous
Statuses     MPI communications is not changed but its required size could be a little bit larger
than that for particle transfer 4NS discussed in §4.4.2.  That  is, in the hot-spot
gathering communication, a node can post MPI_Irecv() from 2D neighbors the half
of which can be identital each other. Since a neighbor m may have a large family of
F(m) = N members, the total number of posted MPI_Irecv() can be (2D/2)N =
2D−1N and thus 4N in three-dimensional simulation with D = 3. At the same time,
the node may also be involved in the hot-spot scattering communication with 3D −1
neighbors of its primary and secondary subdomains, and its helpand from which it
can receive two messages. Therefore, the maximum number of pending MPI_Irecv()
is 4N + 2 · (3D −1) + 2 = 4N + 2 · 3D which can be larger than 4NS with S = 1
and/or a small N. Therfore, init2() allocates the arrays of 4NS + 2 · 3D instead of
4NS.

Requests[] is referred to in;

gather_hspot_recv(), gather_hspot_send(), scatter_hspot_send(),
scatter_hspot_recv(), exchange_xfer_amount() and xfer_particles(),

while Statuses[] is referred to in;


<!-- Page 338 -->

scatter_hspot_send(), scatter_hspot_recv(), exchange_xfer_amount()
and xfer_particles().

Now we show the variables and struct data types for the particle transfer scheduling.

gridOverflowLimit     • The  integer  variable gridOverflowLimit  is  set to 2Phot by oh4p_max_local_
particles() based on its argument hsthresh = Phot. Since Phot  is the minimum
cardinality of each subset split from the set of particles in a hot-spot grid-voxel, we
cannot split the set if its cardinality is less than 2Phot. In other words, we may split
the hot-spot set if adding it to the set for a member of primary family of the local node
makes its accommodating particle population exceed what the balancing mechanism
expect by 2Phot, and must do it to keep the excess over Pmax is less than 4Phot, i.e.,
2Phot for each of primary and secondary particle sets. This variable is referred to
solely in sched_recv() through the macro Sched_Recv_Check().

AltSecRList     • The S_commlist-type pointer AltSecRList is let point the head of alternative sec-
ondary receiving block in CommList[] by make_recv_list(). Then it is referred to
by make_send_sched().

SecRLIndex     • The integer array SecRLIndex[3D + 1] has the index of secondary receiving and sec-
ondary sending blocks in CommList[] in its element [k] for the k-th neighbor of the
local node’s helpand if k < 3D, while the element [3D] has the index of the block
following secondary sending block, i.e., alternative secondary receiving or hot-spot
sending block, or the combined size of secondary receiving and secondary sending
blocks in other words. The array is obtained from the helpand by its broadcast of
RLIndex[] in make_recv_list(), and then is referred to by make_send_sched().

S_recvsched_context     • The struct named S_recvsched_context is to keep the execution context of the
function sched_recv() with the following elements, which are initialized by the caller
make_recv_list() and then referred to and updated by sched_recv().

  - x, y and z are the local coordinate of the grid-voxel in per-grid histogram to be
vistied, while g is its one-dimensional index.

  - hs is the number of hot-spots which have already visted.

  - nptotal is the number of particles which have already processed.

  - nplimit is the total number of particles which the nodes already visited are
expected to accommodate by the balancing algorithm.

  - carryover is the number of particles in the visiting hot-spot which have not
been assigned to nodes yet.

  - cptr is the pointer to a record in CommList[] to be built.

S_hotspot     • The array HotSpotList[2N +2·3D +1] of the S_hotspot structure with the following
HotSpotList       elements keeps all hot-spots which the local node is involved in the last and/or the
HotSpotTop       next simulation steps.

  - g is the one-dimensional index of the hot-spot.

  - n is the number of nodes which should accommodate the particles in the hot-spot.

  - lev is the zero-origin ordinal of the hot-spot in the subdomain to which it be-
longs.


<!-- Page 339 -->

  - self is true iffthe hot-spot is in the interior of the primary/secondary subdomain
of the local node, i.e., not in exterior being its sending planes/edges/vertices, and
the local node must accommodate some particles in the hot-spot.

  - comm is the pointer to the head record for the hot-spot in primary receiving block
at first then that in hot-spot sending block of CommList[].

  - next is the pointer to the succeeding S_hotspot element in the subdomain to
which the hot-spot is belongs.

A node can be involved in all hot-spots in the simulated space domain, while the
number of hot-spots is at most 2N because a node can be the first node of at most
two hot-spots in its primary and secondary subdomains.  Since the local node has
one dummy element in HotSpotList[] for each neighbor of its primary and secondary
subdomains including themselves and the newly assigned secondary subdomain in
transitional state of helpand-helper reconfiguration, we need 2·3D+1 dummy elements.
Therefore, the size of HotSpotList[] is 2N +2·3D +1, with which init4p() allocates
the array.

The S hotspot  type  pointer HotSpotTop  points  the  first unused  element  of
HotSpotList[]. Therefore, make_send_sched() lets it be equal to HotSpotList to
mean all elements in it are available. The function also increments the variable to
acquire the dummy element for a subdomain. Then make_send_sched_body() incre-
ments it too when the function enqueues a hot-spot for a subdomain.

S_hotspotbase     • The array HotSpot[3][3D] of the S_hotspotbase structure holds queues of S_hotspot
HotSpot       elements in HotSpotList[] for each neighboring subdomain of the local node. The
array element [p][k] is for the hot-spots in the k-th neighbor of the local node’s primary
(p = 0) or secondary (p = 1) subdomain, in which the local node is involved.  In
addition the element [2][⌊3D/2⌋] is for the hot-spots in the newly assigned secondary
subdomain in transitional state of helpand-helper reconfiguration. The structure has
the following elements.

  - head is the pointer to the queue head on which the functions gather_hspot_
recv(), gather_hspot_send_body(), scatter_hspot_send() and scatter_
hspot_recv_body() operate.

  - tail is the pointer to the dummy element at queue tail. The enqueue operation
by make_send_sched_body() takes place by making the dummy element active
and acquiring a new dummy elemnent from HotSpotTop.

HSRecv     • The integer pointer array HSRecv[3D] is for a conceptually three-dimensional array
of [3D][N][S] whose element [k][m][s] is to receive the number of particles of species
s in a hot-spot in the k-th boundary plane of the local node’s primary subdomain
accomodated by the node m being its helper, a neighbor or a helper of the neighbor.
Note that ⌊3D/2⌋-th boundary plane is not actually boundary plane but the inner
cuboid of the primary subdomain excluding real boundary planes.

This array looks too large just for one hot-spot but is designed to cope with compli-
cated situations that one single node involved in the hot-spot belonging to a subdo-
main in multiple aspects. That is, a node can be responsible of a neighbor subdomain
as its primary one and another neighbor subdomain as its secondary one. More com-
plicatedly, one single subdomain can acts as multiple neighbors of the local node’s
subdomain when we have periodic system-boundary condition and just one or two


<!-- Page 340 -->

nodes rank along an axis, in the case of which the hot-spot appears multiple times in
different location of the per-grid histogram of the subdomain.
The body array of [3D][N][S] is allocated by init4p() which also initializes the
pointer array so that its elements point appropriate elements. Then gather_hspot_
recv() initiates the receiving of the hand-made gathering communication to the array,
while scatter_hspot_send() examines received particle populations.

HSSend     • The integer array HSSend[S] acts as the send buffer for the gathering communication
for the particle population of a hot-spot. For each hot-spot at g, gather_hspot_send_
body() copies PL(p, s, g) = NOfPGrid[p][s][g] into HSSend[s] and sends the whole of
HSSend[] to the node responsible of the subdomain including the hot-spot as its pri-
mary subdomain, if the local node accommodates primary (p = 0) and/or secondary
(p = 1) particles in the hot-spot. The array is allocated by init4p().

HSRecvFromParent     • The integer array HSRecvFromParent[S] acts as the receive buffer for the scattering
communication of the particle population in a hot-spot. That is, the local node having
a hot-spot in its secondary subdomain receives the number of particels of species s
which the node has to accommodate in the element of [s]. The receiving operation is
initiated by gather_hspot_send_body() and the received populations are examined
by scatter_hspot_recv_body(). The array is allocated by init4p().

HSReceiver     • The integer array HSReceiver[S] is locally used by scatter_hspot_send() to re-
member the ordinal of the hot-spot receiver to which a node sends its particle of
species s in the element [s]. The array is allocated by init4p().

T_Hgramhalf     • The MPI_Datatype variable T Hgramhalf has the MPI data-type for a slice [p][∗][m]
in NOfSend[][][] and NOfRecv[][][] to send/receive the particle populations the node m
should accommodate as its primary (p = 0) or secondary (p = 1) particles. The value
of this variable is created by MPI_Type_vector() called in init4p() so that the type
has S elements with the stride of N, and is used in exchange_xfer_amount().


EXTERN int gridOverflowLimit;
EXTERN struct S_commlist *AltSecRList;
EXTERN int SecRLIndex[OH_NEIGHBORS+1];

struct S_recvsched_context {
int x, y, z, g, hs;
dint nptotal, nplimit, carryover;
struct S_commlist *cptr;
};
struct S_hotspot {
int g, n, lev, self;
struct S_commlist *comm;
struct S_hotspot *next;
};
EXTERN struct S_hotspot *HotSpotList, *HotSpotTop;      /* [2*nn+2*3^D+1] */
struct S_hotspotbase {
struct S_hotspot *head, *tail;
};
EXTERN struct S_hotspotbase HotSpot[3][OH_NEIGHBORS];

EXTERN int *HSRecv[OH_NEIGHBORS];                       /* [3^D][nn][ns] */


<!-- Page 341 -->

EXTERN int *HSSend, *HSRecvFromParent, *HSReceiver;     /* [ns] */
EXTERN MPI_Datatype T_Hgramhalf;



#### 4.9.5 Variables for Neighboring Information

Next, we declare arrays to hold neighboring information.

FirstNeighbor     • When make_recv_list() receives primary receiving blocks from neighbors, we need
to know not only a node appears twice or more in SrcNeighbors[] but also the
ordinal of its first occurrence so that RLIndex[k] for the second or following oc-
currence of k-th neighbor has RLIndex[k′] where k′ < k and SrcNeighbors[k] =
−(SrcNeighbors[k′] + 1). The array FirstNeighbor[3D] is for this and its element
[k] has k if m = SrcNeighbors[k] ≥0 or m = −N −1 to mean the first occurrence,
or k′ such that m = −(SrcNeighbors[k′] + 1). The array is let have these values by
init4p().

GridOffset     • The array GridOffset[2][3D] has the offset goff(k) to translate a grid-position of the
k-th neighbor m of the local node n’s primary (p = 0) or secondary (p = 1) subdo-
main n′ ∈{n, parent(n)} into the corresponding grid-position of n′ in the element
[p][k]. That is, when (x, y, z) in m’s local coordinate corresponds to (x′, y′, z′) of n′,
gidx(x′, y′, z′) = gidx(x, y, z) + goff(k). The d-th dimensional origin x0d(m) of the
subdomain m is at δld(m) and thus x0d(m) is at x0d(m, n′) = δld(m)−δld(n′) in the local
coordinate of n′. Therefore, for the k-th neighbor m of n′, x0d(m, n′) is calculated by;

 δld(m) −δlu(m) = δd(m)  νd = 0
x0d(m, n′) = δld(m) −δld(n′) =    δld(n′) −δld(n′) = 0       νd = 1                                
δud(n′) −δld(n′) = δd(n′)  νd = 2
∑D−1
where k =   d=0 νd3d, and thus goff(k) = gidx(x00(m, n′), . . .).  The values  [p][]
are initialized/updated when Neighbors[p][]  is initialized/updated by the func-
tion update_neighbors(), called from init4p() for [0][] and from rebalance4p()
and exchange_particles4p() for  [1][].  Besides the initializer/updater update_
neighbors(), the array is reffered to through the macro Local_Grid_Position()
invokded in the macro Move_Or_Do() and in the function oh4p_remove_mapped_
particle().

S_realneighbor     • The arrays RealDstNeighbors[2][2] and RealSrcNeighbors[2][2] of S_realneighbor
RealDstNeighbors        structure have the sets of nodes in the neighboring families of the local node. The
RealSrcNeighbors        structure element nbor[N] is the array of a node set and n has its cardinality.

The element RealDstNeighbors[0][p] has the nodes responsible of the subdomain
neighboring the local node’s primary and secondary subdomains as their primary (p =
0) or secondary (p = 1) subdomains. This means that particles accommodated by the
local node will be sent to them as their primary (p = 0) or secondary (p = 1) particles.
On the other hand, the element RealSrcNeighbors[0][p] has the nodes responsible
of the subdomain neighboring the local node’s primary (p = 0) or secondary (p = 1)
subdomain as their primary and secondary subdomains.  This means that particle
that the local node will accommodate as its primary (p = 0) or secondary (p = 1)
ones are sent from them.


<!-- Page 342 -->

The elements RealDstNeighbors[1][p] and RealSrcNeighbors[1][p] have same mean-
ings as their  [0][p] counterparts but they are for transitional state of helpand-
helper reconfiguration.   Therefore, RealDstNeighbors[1][p] should have the hel-
pand (p = 0) or its new helpers (p = 1) of the neighbor subdomains of the local
node’s primary subdomain and its old secondary subdomain. Similarly but inversely,
RealSrcNeighbors[1][p] should have the helpand and its old helpers of the neighbor
subdomains of the local node’s primary subdomain (p = 0) or its new secondary
subdomain (p = 1).
The arrays are allocated by init4p(), are updated by update_real_neighbors()
and its callee upd_real_nbr(), and are referred to by exchange_xfer_amount(),
set_sendbuf_disps4p() and xfer_particles().

In addition we slightly modified the definitions of a few arrays declared in level-1 library
as follows.

Neighbors     • We add the element array [2][] to Neighbors[3][N] so that it temporarily has the
neighbors of the local node’s helpand by build_new_comm(). The added element
Neighbors[2][] is referred to by upd_real_nbr() to construct RealSrcNeighbors[1][1]
and then copied into Neighbors[1][] by rebalance4p().
Besides this extra role, Neighbors[0][] and Neighbors[1][] are used with the original
meaning in make_send_sched(), update_neighbors() and gather_hspot_send().

TempArray     • In update_real_neighbors(), we have to keep track the occurrence of the nodes in
the set RealDstNeighbors[k][p].nbor[] and RealSrcNeighbors[k][p].nbor[] for each
particular k but for all p ∈[0, 1]. Therefore, we need an array of [4][N] and thus let
init4p() allocate 4N elements for TempArray[] for this purpose. The whole part of
TempArray[4N] is referred to by update_real_neighbors() and its callee upd_real_
nbr(), while init4p() uses the first N elements to build FirstNeighbor[].

- Besides two arrays above, we use the following neighbor arrays in the orignial mean-
ings.

  - DstNeighbors[] in make_recv_list() and gather_hspot_recv().
  - SrcNeighbors[] in init4p() and make_recv_list().


EXTERN int FirstNeighbor[OH_NEIGHBORS], GridOffset[2][OH_NEIGHBORS];
struct S_realneighbor {
int n, *nbor;
};
EXTERN struct S_realneighbor RealDstNeighbors[2][2], RealSrcNeighbors[2][2];



#### 4.9.6 Variable for Boundary Condition

BoundaryCondition  The last variable is BoundaryCondition[D][2] being the substance of the oh4p_init()’s
argument bcond to have the boundary condition of the lower (β = 0) or upper (β =
1) system boundary plane parpendicular to d-th dimensional axis in the element [d][β].
The array is initialized in init4p() and is referred to by the macro Map_Particle_To_
Subdomain() used in oh4p_map_particle_to_subdomain().


EXTERN int BoundaryCondition[OH_DIMENSION][2];


<!-- Page 343 -->

#### 4.9.7 Function Prototypes

The next and last block is to declare the prototypes of the API function pairs each of which
consists of API for Fortran and C, as listed below.

- The function oh4p_init[_]() initializes data strucutures of the level-4p and lower
level libraries.

- The function oh4p_max_local_particles[_]() defines Phot, the minimum cardinal-
ity of a subset split from a hot-spot, and calculates Plim, the size of the particle buffer
Particles[], based on Phot and other parameters.

- The function oh4p_per_grid_histogram[_]() defines an array to be associated with
per-grid histogram.

- The function oh4p_transbound[_]() at first performs what its level-1 counterpart
oh1_transbound[_]() does to have the fundamental particle assignment for load
balancing, and then modifies it to have position-aware particle distribution by the
level-4p’s own particle transfer.

- The function oh4p_map_particle_to_neighbor[_]() finds the subdomain in which
a given particle resides, providing that the subdomain is a neighbor of the primary/
secondary subdomain of the local node, and maintains per-subdomain and per-grid
histograms of particle population.

- The function oh4p_map_particle_to_subdomain[_]() finds the subdomain in which
a given particle resides, allowing that the subdomain is not necessary to be a neighbor
of the primary/secondary subdomain of the local node, and maintains per-subdomain
and per-grid histograms of particle population.

- The function oh4p_inject_particle[_]() injects a particle and place it at the bot-
tom of Particles[] maintaining per-subdomain and per-grid histograms of particle
population.

- The function oh4p_remove_mapped_particle[_]() removes a particle which has
been mapped to a subdomain or been injected into a subdomain.

- The function oh4p_remap_particle_to_neighbor[_]() does what functions oh4p_
remove_mapped_particle() and oh4p_map_particle_to_neighbor() do.

- The function oh4p_remap_particle_to_subdomain[_]() does what functions oh4p_
remove_mapped_particle() and oh4p_map_particle_to_subdomain() do.

As done in §4.2.11, §4.4.5 and §4.6.6, prior to showing the function prototypes, we
show the fourth part of the header files ohhelp c.h for C-coded simulators and ohhelp f.h for
Fortran-coded ones, which define the aliases of level-4p API functions. In the #else part of
#if␣OH_LIB_LEVEL=3, at first they #define the aliases of API functions if OH_LIB_LEVEL_
4P is defined.

#else
#ifdef OH_LIB_LEVEL_4P
#define \
oh_init(A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,A16,A17,A18,
A19,A20,A21,A22) \
oh4p_init(A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,A16,A17,A18,
A19,A20,A21,A22)


<!-- Page 344 -->

#define oh_max_local_particles(A1,A2,A3,A4) \
oh4p_max_local_particles(A1,A2,A3,A4)
#define oh_per_grid_histogram(A1) oh4p_per_grid_histogram(A1)
#define oh_transbound(A1,A2)      oh4p_transbound(A1,A2)
#define oh_map_particle_to_neighbor(A1,A2,A3) \
oh4p_map_particle_to_neighbor(A1,A2,A3)
#define oh_map_particle_to_subdomain(A1,A2,A3) \
oh4p_map_particle_to_subdomain(A1,A2,A3)
#define oh_inject_particle(A1,A2) oh4p_inject_particle(A1,A2)
#define oh_remove_mapped_particle(A1,A2,A3) \
oh4p_remove_mapped_particle(A1,A2,A3)
#define oh_remap_particle_to_neighbor(A1,A2,A3) \
oh4p_remap_particle_to_neighbor(A1,A2,A3)
#define oh_remap_particle_to_subdomain(A1,A2,A3) \
oh4p_remap_particle_to_subdomain(A1,A2,A3)

Then ohhelp c.h gives the prototypes of the functions above, which are also given in
ohhelp4p.h61, while their Fortran versions are given in oh mod4p.F90 as shown in §3.7.

void oh4p_init(int **sdid, const int nspec, const int maxfrac, int **totalp,
struct S_particle **pbuf, int **pbase, const int maxlocalp,
void *mycomm, int **nbor, int *pcoord, int **sdoms, int *scoord,
const int nbound, int *bcond, int **bounds, int *ftypes,
int *cfields, int *ctypes, int **fsizes,
const int stats, const int repiter, const int verbose);
int  oh4p_max_local_particles(const long long int npmax, const int maxfrac,
const int minmargin, const int hsthresh);
void oh4p_per_grid_histogram(int **pghgram);
int  oh4p_transbound(int currmode, int stats);
int  oh4p_map_particle_to_neighbor(struct S_particle *part, const int ps,
const int s);
int  oh4p_map_particle_to_subdomain(struct S_particle *part, const int ps,
const int s);
int  oh4p_inject_particle(const struct S_particle *part, const int ps);
void oh4p_remove_mapped_particle(struct S_particle *part, const int ps,
const int s);
int  oh4p_remap_particle_to_neighbor(struct S_particle *part, const int ps,
const int s);
int  oh4p_remap_particle_to_subdomain(struct S_particle *part, const int ps,
const int s);

Then ohhelp4p.h continues prototype declaration for Fortran API functions.


void oh4p_init_(int *sdid, const int *nspec, const int *maxfrac, int *totalp,
struct S_particle *pbuf, int *pbase, const int *maxlocalp,
struct S_mycommf *mycomm, int *nbor, int *pcoord, int *sdoms,
int *scoord, const int *nbound, int *bcond, int *bounds,
int *ftypes, int *cfields, int *ctypes, int *fsizes,
const int *stats, const int *repiter, const int *verbose);
int  oh4p_max_local_particles_(const dint *npmax, const int *maxfrac,
const int *minmargin, const int *hsthresh);
void oh4p_per_grid_histogram_(int *pghgram);

61Prototypes of oh4p max local particles() in ohhelp c.h and ohhelp4p.h are slightly different, i.e., the
type of its first argument is long long int in the former, while in the latter is dint.


<!-- Page 345 -->

int  oh4p_transbound_(int *currmode, int *stats);
int  oh4p_map_particle_to_neighbor_(struct S_particle *part, const int *ps,
const int *s);
int  oh4p_map_particle_to_subdomain_(struct S_particle *part, const int *ps,
const int *s);
int  oh4p_inject_particle_(const struct S_particle *part, const int *ps);
void oh4p_remove_mapped_particle_(struct S_particle *part, const int *ps,
const int *s);
int  oh4p_remap_particle_to_neighbor_(struct S_particle *part, const int *ps,
const int *s);
int  oh4p_remap_particle_to_subdomain_(struct S_particle *part, const int *ps,
const int *s);
