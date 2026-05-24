# 4.12 Header File ohhelp4s.h

Source: `doc/v1/original/ohhelp.pdf`, pages 442-461.

<!-- Page 442 -->

## 4.12 Header File ohhelp4s.h

The header file of level-4s library, ohhelp4s.h, is very similar to the level-4p counterpart
ohhelp4p.h but has a few additions, deletions and modifications for introducing subcuboid-
based particle assignment and halo particle transfer, and for eliminating hot-spot-related
functions.


#### 4.12.1 Constants

At first we define the following constants as done in ohhelp4p.h (§4.9.1).

OH_PGRID_EXT     • The constant OH_PGRID_EXT = eg = 1 is exactly equivalent to that in the level-4p
library, but the level-4s library cannot cope with the case of eg > 1 and thus init4s()
aborts the execution if eg > 1.
∑D−1
OH_NBR_SELF     • The constant OH_NBR_SELF =   d=0 3d = ⌊3D/2⌋is exactly equivalent to that in the
level-4p library.

Then we define the following level-4s’s own constants for neighbor indices.

OH_NBR_BCC     • The constants OH_NBR_BCC = 1 · 30 + 1 · 31 + 0 · 32 = 2 and OH_NBR_TCC = 1 · 30 +
OH_NBR_TCC       1 · 31 + 2 · 32 = 20 are the neighbor indices of the subdomains contacting with the
local node’s subdomain at its bottom and top surfaces. They are used in make_send_
sched() to know neighbor nodes whose subcuboids contact with the local subcuboid
at its bottom and top surfaces.

Here we revisit a few other constants/switches related to level-4s extension.

OH_LIB_LEVEL_4S     • The switch OH_LIB_LEVEL_4S can be defined in oh config.h to declare that the OhHelp
OH_LIB_LEVEL_4PS        library should be configured with level-4s extension, as discussed in §3.3. The switch
OH_LIB_LEVEL_4PS is defined iffOH_LIB_LEVEL_4S or OH_LIB_LEVEL_4P is defined.

OH_POS_AWARE     • The switch OH_POS_AWARE and the constant STATS_TB_SORT can be defined in
STATS_TB_SORT        ohhelp1.h and oh stats.h respectively, for position-awareness and sorting time mea-
surement, as discussed in §4.9.1.

- With other library levels, D = OH_DIMENSION can be less than 3 for 1- or 2-dimensional
simulations. However, the level-4s library is applicable only to 3-dimensional simula-
tions and thus init4s() aborts the execution if D ̸= 3.


#define OH_PGRID_EXT    1
#define OH_NBR_SELF     (OH_NEIGHBORS>>1)
#define OH_NBR_BCC      (1+1*3+0*3*3)
#define OH_NBR_TCC      (1+1*3+2*3*3)



#### 4.12.2 Macros for Grid-Position

The level-4s mechanism to represent grid-position, the one-dimensional index of each grid-
voxel, in the nid element of S_particle is almost equivalent to that of level-4p, but the


<!-- Page 443 -->

grid-position given by gidx(x, y, z) and the size of per-grid histogram G are different from
those in level-4p as follows.

gidx(x, y, z) = x + (δmaxx  + 6eg)(y + (δmaxy  + 6eg)z)

D−1∏
G =    (δmaxd  + 6eg)
d=0

The difference comes from that the per-grid histogram has 2eg thick sending and receiving
planes to exchange particle populations.  Since the inside portion of sending plane of eg
thick is in the interior of the local node’s subdomain, while its outside portion of eg thick
and 2eg thick receiving plane are in the exterior, the per-grid histogram has 3eg thick
exterior at its lower and upper boundaries to make its width, depth and height δmaxd  + 6eg.
The reason why the sending/receiving planes are 2eg thick is that the per-grid histogram
NOfPGridTotal[][][] needs to have the population of grid-voxels in exterior halo planes,
corresponding to the outside sending planes of per-grid histogram, to know halo particle
population.
The difference above, however, does not affect the switch OH_BIG_SPACE, the data
type OH_nid_t, the global variables logGrid, gridMask and AbsNeighbors, and macros
Decl_Grid_Info(), Subdomain_Id() and Primarize_Id(), which header files other than
ohhelp4s.h define. Therefore and since their definitions (but not necessarily their values)
are perfectly equivalent to those discussed in §4.9.2, here we just show the level-4s functions
which access the variables and use the macros.

- The integer variables logGrid and gridMask are initialized by init4s().

- The two dimensional array AbsNeighbors[2][3D] are initialized/updated by update_
neighbors() called from init4s(), rebalance4s() and exchange_particles4s(),
and is referred to by oh4s_map_particle_to_neighbor() directly and other func-
tions through the macro Subdomain_Id() or Neighbor_Subdomain_Id().

- The macro Decl_Grid_Info() is used in the following functions;

rebalance4s(), count_population(), move_to_sendbuf_4s(),
move_to_sendbuf_uw4s(), move_to_sendbuf_dw4s(), sort_particles(),
move_and_sort(), sort_received_particles(),
oh4s_map_particle_to_neighbor(), oh4s_map_particle_to_subdomain(),
oh4s_remove_mapped_particle().

- The macro Subdomain Id(i, p) is used in oh4s_remove_mapped_particle().

- The macro Primarize Id(π, m) is used in rebalance4s() and oh4s_remove_mapped_
particle().

The equivalence to level-4p also holds for the following macros defined in ohhelp4s.h and
thus we just show the level-4s functions using them.

Grid_Position()     • The macro Grid Position(i) is used in count_population() and oh4s_remove_
mapped_particle() directly, in move_to_sendbuf_4s(), move_to_sendbuf_uw4s(),
move_to_sendbuf_dw4s() and move_and_sort() through the macro Move_Or_Do(),
and in sort_particles() and sort_received_particles() through the macro
Sort_Particle().


<!-- Page 444 -->

Combine_Subdom_Pos()     • The macro Combine Subdom Pos(σ, g) is used in count_population(), oh4s_map_
particle_to_neighbor() and oh4s_map_particle_to_subdomain().

Primarize_Id_Only()     • The macro Primarize Id Only(π) is used in move_to_sendbuf_4s() and move_and_
sort().

Secondarize_Id()     • The macro Secondarize Id(π) is used in rebalance4s(), oh4s_map_particle_to_
neighbor() and oh4s_map_particle_to_subdomain().

Secondary_Injected()     • The macro Secondary Injected(i) is used in rebalance4s(). move_to_sendbuf_
4s() and move_and_sort().

Neighbor_Subdomain_Id()     • The macro Neighbor Subdomain Id(i, p) is used in move_to_sendbuf_4s(), move_
to_sendbuf_uw4s(), move_to_sendbuf_dw4s() and move_and_sort() through the
macro Move_Or_Do().

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



#### 4.12.3 Per-Grid Histograms and Related Variables

Next, we declare the following arrays of per-grid histograms and related variables, some of
which are perfectly equivalent to those declared in the level-4p library.

PbufIndex     • PbufIndex[2][S] is perfectly equivalent to that declared in the level-4p library,  is
initialized by init4s() with NULL, is allocated by the first call of transbound4s()
which also sets all elements in each call, and is referred to by oh4s_map_particle_
to_neighbor(), oh4s_map_particle_to_subdomain() and oh4s_remove_mapped_
particle() through the macro Check_Particle_Location().

NOfPGrid     • As in level-4p, NOfPGrid[2] and NOfPGridTotal[2] are double pointer arrays to im-
NOfPGridTotal       plement three-dimensional dint arrays of [2][S][G] whose element [p][s][g] has the
number of primary (p = 0) or secondary (p = 1) particles of species s in the grid-
voxel whose index is g, namely PL(p, s, g) and PT (p, s, g) respectively. Besides the
fact that the size G is larger than that in level-4p as discussed in §4.12.2, their roles
are little bit different from those in level-4p.

The allocation and initialization of both arrays in init4s(),  reinitialization of
NOfPGrid[][][] in transbound4s(), and its counting up/down in oh4s_map_particle_
to_neighbor(),  oh4s_map_particle_to_subdomain()  or oh4s_remove_mapped_
particle() are equivalent to those in level-4p, except for the reinitialization in
transbound4s() which enlarging receiving plane thickness affects.
Their first roles in transbound4s() are fundamentally equivalent to those in level-4p,
but the process to build the complete per-grid histogram is a little bit different as
follows.


<!-- Page 445 -->

  - The complete histogram is always built in NOfPGridTotal[][][] regardless the
mode in the next simulation step, by exchange_population().  Therefore,
NOfPGrid[0][][] is copied into NOfPGridTotal[0][][]  if the current mode is pri-
mary, while NOfPGrid[][][] of the primary family members are summed up by
reduce_population() if the mode is secondary.

  - Regardless the accommodation pattern, the sending planes of NOfPGridTotal[0]
[][] are exchanged among neighbor nodes to have receiving planes in exchange_
population(), because we always need to have halo particle population. There-
fore, even if we have anywhere accommodation, NOfPGridTotal[0[][] is built in
the same way of normal accomodation but after the local histogram is built in
NOfPGrid[][][] by count_population().

  - Regardless of the current/next mode and accommodation pattern, NOfPGrid[][][]
is referred to by make_send_sched_body() through the macro Make_Send_
Sched_Body() to know the number of particles to be sent to other nodes, and
NOfPGridTotal[][][] is referred to by make_send_sched_hplane() to know the
number of particles to be accommodated and transferred for halo particle ex-
change.

  - The function sched_recv() does not refers to NOfPGridTotal[][][] but to the                     ∑ ∑
per-plane histogram NOfPGridZ[z] = PZ(z) =    s   x,y PT (0, s, gidx(x, y, z)) for
each z to determine the subcuboids to be assigned to primary members.

Note that NOfPGridTotal[0][][] is broadcasted in make_recv_list() to primary fam-
ily members so that they have it as NOfPGridTotal[1][][], as done in level-4p.

Then NOfPGrid[][][] is modified by make_send_sched_body() (through the macro
Make_Send_Sched_Body()) to have one of the followings after its original contents
are examined.

(a) 0 means that the particles in the grid-voxel stays in the local node, as in level-4p.
(b) A positive number (pS + s)N + m + 1 < 232 means that the particles of species
s in the grid-voxel is sent to the node m as its primary (p = 0) or secondary
(p = 1) particle, as in level-4p.

Further, the array is modified by make_bsend_sched() to have one of the following
level-4s’s own values in addition to the values above.

(c) A positive number (i + 1) · 232 + σ ≥232 means the particles in the grid-voxel
stay in the local node (σ = 0) or is sent to other node (σ > 0) as in above two
cases, and are copied to BoundarySendBuf[i] after the reception of halo particles
since the grid-voxel is in an exterior pillar being an intersection of a west/east
vertical exterior halo plane and a south/north vertical interior halo plane.
(d) A negative number −(i + 1) > −232 means that the grid-voxel is in a vertical
interior halo plane and thus particles in it definitely stay in the local node. The
copy of the first particle in it goes to BoundarySendBuf[i] to be sent to a node
as its halo particle.
(e) A negative number −(i + 1) · 232 −(j + 1) ≤−232 means that the grid-voxel
is in a interior pillar being the intersection of two vertical interior halo plane,
and thus particles in it definitely stay in the local node too. In this case, the
first copied particle goes to both BoundarySendBuf[i] and BoundarySendBuf[j]
to send it to the south/north and west/east neighbors respectively.


<!-- Page 446 -->

The functions move_to_sendbuf_4s(), move_to_sendbuf_uw4s() and move_to_
sendbuf_dw4s() refer to NOfPGrid[][][] having values (a) or (b) because they are
called before make_bsend_sched(), while sort_particles() may see (a), (b), (d)
or (e)77, but (a) and (b) are not distinguish because the grid-voxel is definitely in
a subcuboid of the local node. On the other hand, move_and_sort() may see all
values but it is assured that a grid-voxel having (d) or (e) should have had (a) before
make_bsend_sched() let it have them, because, without helpand-helper reconfigura-
tion, the old and new secondary subdomain are same. Note that a grid-voxel having
(c) can have had (a) because the neighbor sharing the vertical exterior halo plane
as its vertical interior halo plane can be the local node itself with periodic boundary
condition.  Finally, sort_received_particles() can see (a), (d) or (e) because it
should visit grid-voxels only in subcuboid of the local nodes.

On the other hand, NOfPGridTotal[][][] is modified by exchange_particles4s() to
have the sorting index of SendBuf[] to which the first particle in the grid-voxel is
moved, as in level-4p. A small difference from level-4p is that SendBuf[] will have
halo particles and thus the sorting index must takes them into account. That is,
for the local node n, it is initialized as follows and then its element is incremented
each time a particle in the grid-voxel is moved by the functions sort_particles(),
move_and_sort() or sort_received_particles().
{
n          q = 0
nq =
parent(n)  q = 1

δd(m) = δud(m) −δld(m)
Rd(q) = [−eg, δd(nq) + eg)
G(q) = {g | g = gidx(x, y, z), x ∈Rx(q), y ∈Ry(q), z ∈Rz(q)}
NOfPGridTotal[p][s][g] =
∑p−1 S−1∑ ∑
NOfPGridOut[q][t][h] +
q=0 t=0 h∈G(q)
∑s−1 ∑        ∑
NOfPGridOut[p][t][h] +    NOfPGridOut[p][s][h]
t=0 h∈G(p)                      h∈G(p), h<g


NOfPGridOut     • As in level-4p, NOfPGridOut[2]  is a double pointer array to implement a three-
NOfPGridOutShadow       dimensional int array of [2][S][G] whose element [p][s][g] has the number of primary
NOfPGridIndex        (p = 0) or secondary (p = 1) particles of species s in the grid-voxel whose index is g,
NOfPGridIndexShadow       namely PO(p, s, g). Besides the enlargement of G again, it has a few different features
from that in level-4p as follows.

  - NOfPGridOut[][][] has a shadow NOfPGridOutShadow[][][] whose body array
is  given  or  returned through  the argument pghgram  of oh4s_per_grid_
histogram(), because  it  is referred to outside of transbound4s() and by
oh4s_exchange_border_data()  while no  level-4p  library  function  outside
transbound4p() accesses the array. The substance array is allocated by oh4s_
per_grid_histogram() using Allocate_NOfPGrid(), but the body of shadow is

77It cannot see (c) because it scans interior particles only, but can see (b) if helpand-helper reconfiguration
takes place to let particles in a grid-voxel in the old secondary subdomain go to other nodes while the grid-
voxel is in the new secondary subcuboid of the local node.


<!-- Page 447 -->

allocated if pghgram point NULL while its pointer array are allocated uncondition-
ally. Copying the substance to the shadow is done by exchange_particles4s().

  - NOfPGridOut[][][] has a correlated index array NOfPGridIndex[][][] and its shadow
NOfPGridIndexShadow[][][] whose body is given or returned through the argu-
ment pgindex of oh4s_per_grid_histogram(), which allocates these index ar-
rays in a manner similar to that of per-grid histograms.  The substance in-
dex array has values same as the initial ones of NOfPGridTotal[][][] in its sec-
ond role discussed in the previous item, but is let have values in exchange_
particles4s(). The function also makes the shadow has the copy of the sub-
stance with C coded simulators, while each shadow element is made greater by
one than the substance’s with Fortran coded ones.

  - NOfPGridOut[][][] is let have particle populations by make_send_sched_self()
or make_send_sched_hplane() regardless of the next execution mode.  Also
regardless of the mode,  it is referred to by exchange_particles4s() to let
NOfPGridTotal[][][] have sorting indices.

  - NOfPGridOut[][][] is also referred to by make_bsend_sched() and make_brecv_
sched() to build the halo particle transfer schedules.  Then, it is referred to
by xfer_boundary_particles_v() and exchange_border_data_v() together
with NOfPGridIndex[][][] to have the population and the index of SendBuf[] for
particles, or of the buffer for particle-associated data in halo planes.

NOfPGridZ     • The one dimensional dint array NOfPGridZ[δmaxz    ] is level-4s’s own per-plane histogram
to have the number of particles of each xy-plane at z-coordinate z of the local node’s
primary subdomain in its element [z], namely PZ(z). More specifically, the element
PZ(z) = NOfPGridZ[z] has the following for the local node n.

S−1∑ δy(n)−1∑  δx(n)−1∑
PZ(z) = NOfPGridZ[z] =              PT (0, s, gidx(x, y, z))
s=0  y=0   x=0

After allocated by init4s(), the array is let have the values above by exchange_
population(). Then it is referred to by sched_recv() to determine the primary
family member node to which each xy-plane is assigned as a part of the subcuboid of
the node.

ZBound     • The int array ZBound[2][2] is level-4s’s own to have the local z-coordinate of the
ZBoundShadow        lower (β = 0) or upper (β = 1) surface of the primary (p = 0) or secondary (p =
1) subcuboid ζβp (n) of the local node n in ZBound[p][β].  It can be ZBound[p][0] =
ZBound[p][1] = 0 to mean the corresponding subcuboid is empty because n is not
assigned any particles, as (re)initialized by transbound4s() for p ∈{0, 1}.  After
that, ZBound[p][β] is let have ζβp (n) by make_send_sched_self(), and then is referred
to by make_send_sched() to know if n has subcuboid, by make_bsend_sched() and
make_brecv_sched() to examine if other node’s subcuboid shares a vertical surface
of n’s subcuboid, and by xfer_boundary_particles_v() and exchange_border_
data_v() to scan vertical halo planes.

Since exchange_border_data_v() is outside of transbound4s() and the simulator
body need ζβp (n), ZBound[][] has its shadow ZBoundShadow[2][2]. The shadow is given
through the zbound argument of oh4s_init(), or is allocated by init4s() and re-
turned to the simulator body through the argument. The function init4s() also
initializes the shadow letting ζl0(n) = 0 and ζu0 = δz(n) to mean the local node n’s


<!-- Page 448 -->

primary subdomain itself is n’s primary subcuboid, and ζl1(n) = ζu1 (n) = 0 to mean
n does not have secondary subcuboid. Then all elements in the substance ZBound[][]
are copied into those in the shadow by transbound4s() as a part of its post process.

S_hplane     • The array HPlane[2][2] of S_hplane structure with the following elements are level-
HPlane         4s’s own to have per-node transfer schedules of particles and particle-associated data
in horizontal halo planes.

  - nbor is the rank m of the node to/from which the particles in a plane is sent/
received. This element can be MPI_PROC_NULL to mean the corresponding sub-
cuboid is empty because the local node is not assigned any particles or its sub-
cuboid’s surface is at non-periodic system boundary.
  - stag has (p · 3D + k)S to indicate that the particles are primary (p = 0) or
secondary (p = 1) for the k-th neighbor node m. rtag also has (p · 3D + k)S
but indicates that the particles are primary (p = 0) or secondary (p = 1) for
the local node and it is m’s k-th neighbor. From both elements, the tag of MPI_
Isend() and MPI_Irecv() to transfer halo particles of species s is generated as
(p · 3D + k)S + s to make each send/receive pair for each s unique for the local
node even when a node occurs multiple times as receivers/senders of the transfer
due to periodic boundary condition and/or two roles as helpand and a helper.

  - nsend[S] and nrecv[S] have the number of particles of species s to be sent/
received in their element [s].

  - sbuf[S] and rbuf[S] have the head indices of the send/receive buffer in their ele-
ment [s]. For particle transfer, they are indices of hbuf sh(p, β, s) and hbuf rh(p, β, s)
in SendBuf[], while they are those of buf[] argument array of oh4s_exchange_
border_data() for particle-associated data transfer performed by the function.

HPlane[p][β] has the transfer schedule for the lower (β = 0) or upper (β = 1) hori-
zontal halo plane of primary (p = 0) or secondary (p = 1) subcuboid.

The array elements of S of HPlane[][] are allocated by init4s() which also initializes
nbor being MPI_PROC_NULL in order to keep oh4s_exchange_border_data() from
doing anything before the first call of oh4s_transbound(). Besides the initialization,
HPlane[][] is let have the transfer schedule by make_send_sched() and its callees
make_send_sched_self() and make_send_sched_hplane(), and then referred to by
xfer_boundary_particles_h() and exchange_border_data_h() to transfer parti-
cles and particle-associated data respectively.

S_vplane     • The array VPlane[2N + 6] of S_vplane structure, similar to S_hplane but a little
VPlane         bit different from it, with the following elements are level-4s’s own to have per-node
VPlaneHead        transfer schedules of particles and particle-associated data in vertical halo planes.

  - nbor is same as that of S_hplane to have rank m of the node to/from which the
particles in a plane is sent/received.
  - stag and rtag are similar to those of S_hplane, but have p · 3D + k because
particles in all species are trasferred at once.

  - nsend and nrecv have the number of particles to be sent/received to/from nbor.
Since particles in all species are transferred to/from nbor at once, they are scalars
rather than arrays of S elements.


<!-- Page 449 -->

  - sbuf and rbuf have  the head  indices  of  the  send/receive  buffer,  being
hbuf sv(d, p, β, m) in BoundarySendBuf[] and hbuf rv(d, p, β, m) in Particles[] re-
spectively in particle transfer for nbor = m and stag or rtag being p · 3D + k
where k is defined as follows for west (d, β) = (0, 0), east (0, 1), south (1, 0) and
north (1, 1) neighbors.
{
32 + 31 + 2β · 30  d = 0    k = 32+(1+d(2β−1))·31+(1+(1−d)(2β−1))·30 =
32 + 2β · 31 + 30  d = 1

In particle-associated data transfer by by oh4s_exchange_border_data(), they
are indices of the argument array sbuf[] and rbuf[] of the function respectively.

An element of VPlane[] has the following for the local node n. Let np (p ∈{0, 1})
be n’s primary (n0 = n) or secondary (n1 = parent(n)) subdomain, npd,β be the d-th
dimensional (d ∈{0, 1}) lower (β = 0) or upper (β = 1) neighbor subdomain of np.
We define that n’s primary/secondary subcuboid has contact with that of m ∈F(npd,β)
iff[ζlp(n), ζup (n)) ∩[ζlq(m), ζuq (m)) ̸= ∅where q = 0 if m = npd,β or q = 1 otherwise,
and denote the subset of F(npd,β) whose member has such contacting subcuboids as
F n(npc   d,β) = {f0, . . .} such that j < j′ iffζlq(fj) < ζlq(fj′). With these definitions,
the element VPlane[i(d, p, β, j)] has the transfer schedule for fj ∈F n(npc   d,β) where
i(d, p, β, j) is defined as follows and pn ∈{0, 1} is 1 iffthe local node n will have
secondary subdomain in the next step.

∑d−1 ∑pn ∑1    ∑p−1 ∑1             β−1∑
i(d, p, β, j) =         Fn(nqc   e,γ) +      F n(nqc   d,γ) +   F n(npc   d,γ) + j
e=0 q=0 γ=0            q=0 γ=0            γ=0

Note that the concepually 3-dimensional int array VPlaneHead[2 × 2 × 2 + 1] has
i(d, p, β, 0) in its element [d][p][β] including the last one [2][0][0] being the number of
elements in VPlane[].

VPlane[] is allocated by init4s() which also initializes VPlaneHead[d][p][β] to be 0
for all d ∈{0, 1}, p ∈{0, 1} and β ∈{0, 1} as well as the last element [2][0][0], in order
to keep oh4s_exchange_border_data() from doing anything before the first call of
oh4s_transbound().

The size of VPlane[] being NV = 2N + 6 is determined as follows. It is assured that
n0d,β ̸= n1d,β for all d and β, but it can be npd,β = nqe,γ  if (d, β) ̸= (e, γ), because a
neighbor of primary subdomain may be also a neighbor of secondary one with different
index and, more complicatedly, a subdomain may have two or more neighbor indecies
with periodic system boundary.  If npd,β = np, |Fn(npc   d,β)| = 1 because n’s subcuboid
has only one contacting subcuboid being itself.  Otherwise, |Fn(npc   d,β)| ≤|F(npd,β)|                        ∑ ∑ ∑
obviously but they can be equal. Since VPlane[] must have   d   p   β |F n(npc   d,β)| and∪ ∪ ∪
d   p  β Fn(npc   d,β) can be the set of all nodes N, it looks NV = N +2×4−1 because∪ ∪ ∪
d   p  β Hcn(npd,β) can be N −{r}, where Hcn(npd,β) is the set of helpers in Fn(npc   d,β)
and r is the root of the family tree, and we have four, i.e., west, east, south and north,
neighbors for each of primary and secondary subdomains.

However as mentioned above, we can have neighbor duplication to make NV larger.
Let M(n) be the set of non-self neighbors of n, i.e.,

M(n) = {npd,β | d ∈{0, 1}, p ∈{0, 1}, β ∈{0, 1}, npd,β ̸= np} = {m1, . . .}


<!-- Page 450 -->

occ(mi) be the number of occurrence of mi as the neighbor of n0 and n1,  i.e.,
occ(mi) = |{npd,β | mi = npd,β  ̸= np}|, and occ(m1) be∑the largest without loss of
generality. Then NV must be the maximum of NVn =     i occ(mi)|F n(mi)|c      because
the                             transfer schedule for npd,β is different from nqe,γ even if mi = npd,β = nqe,γ. Since             ∪
i Hcn(mi) ⊆N −{r} again, Hcn(mi) ∩Hcn(mj) = ∅for any i ̸= j, and F n(mi)c     can be
N  if mi = npd,β ̸= np, NVn is maximized with given M(n) when Fn(m1)c   = N to let
∑
NV = occ(m1)N +    occ(mi) + {npd,β | npd,β = np} = occ(m1)N + (2 × 4 −occ(m1))
i>1

because Fn(mi)c   = {mi} for all i > 1 due to Fn(m1)c   = N.
Therefore, NVn is maximized by maximizing occ(m1) letting it be 2 with Πx × Πy = 2
and periodic system boundaries perpendicular to z-axis. Suppose Πx = 2 and Πy = 1
to let n’s primary subdomain has west and east neighbors commonly m1, while n’s
north and south neighbors are commonly n itself.  Since F(m1) = F n(m1)c   = N
to include n in it, n’s secondary subdomain is m1 having m1 itself as north and
south neighbors and n as west and east neighbors. Therefore, occ(m1) = 2 to make
NV = 2N + (2 × 4 −2) = 2N + 6.
Besides the initialization, VPlane[] is let have the transfer schedule by make_bsend_
sched() and make_brecv_sched(), while their caller make_bxfer_sched() deter-
mines VPlaneHead[], and then referred to by xfer_boundary_particles_v() and
exchange_border_data_v() together with VPlaneHead[] to transfer particles and
particle-associated data.

BoundarySendBuf     • The array BoundarySendBuf[K] of S_particle structure where

K = 2Dδmaxz   ((δmaxy  + 2eg)(δmaxx  + 2eg) −δmaxx  δmaxy   )
is for the set of hbuf sv(d, p, β, m) for primary (p = 0) or secondary (p = 1) particles
accmmodated by the local node n and sent to the node m in the family for the d-th
dimensional (d ∈{0, 1}) lower (β = 0) or upper (β = 1) neighbor of n’s primary/
secondary subdomain as m’s halo particles. The size K represents the maximum num-
ber of particles in vertical exterior halo plane (not interior) of primary and secondary
subcuboids, because we need to have two copies of each particle in interior pillars and
a copy of each in exterior pillar. That is, by shifting each vertical interior halo plane
by eg grids outward for a neighbor contacting at a surface, and each exterior pillar by
eg grids along y-axis outward for relaying particles in them from a west/east neighbor
to a south/north one, we have a grid-voxel set whose cardinality is equal to those in
vertical exterior halo planes. To the array, particles are copied by sort_particles()
and sort_received_particles() through the macro Sort_Particle(), by move_
and_sort() through the macro Move_Or_Do(), and by xfer_boundary_particles_
v() for exterior pillars. Then the particles in the array is sent to other nodes by
xfer_boundary_particles_v() using the indices and number of particles recorded
in VPlane[].

S_griddesc     • The array GridDesc[3] of the S_griddesc structure is foundamentally equivalent to
GridDesc        that in level-4p described in §4.9.3. However, its elements w, d and h are enlarged
from δmaxd  + 4eg to δmaxd  + 6eg because of the outside portion of sending planes of eg
thick and receiving planes of 2eg thick, as discussed in §4.12.2.

As in level-4p, each array element is set by set_grid_descriptor() called from
init4s() for [0], rebalance4s() or exchange_particles4s() for [1], and make_


<!-- Page 451 -->

recv_list() for [2]. Then the array is referred to by the following functions directly
(*1), through macros Allocate_NOfPGrid() (*2), For_All_Grid() (*3), For_All_
Grid_Abs() (*4), For_All_Grid_Z() (*5), For_All_Grid_XY_At_Z()(*6), and/or
Neighbor_Grid_Offset() (*7).

init4s()(*1,*2), oh4s_per_grid_histogram()(*1,*2), transbound4s()(*3),
exchange_particles4s()(*3), exchange_population()(*3),
add_population()(*4), make_recv_list()(*1), sched_recv()(*1),
make_send_sched_body()(*1,*5), make_send_sched_self()(*1,*5),
make_send_sched_hplane()(*6), update_neighbors()(*1,*7),
count_population()(*3), make_bsend_sched()(*1,*5),
make_brecv_sched()(*1,*5), sort_particles()(*3), move_and_sort()(*3),
xfer_boundary_particles_v()(*1,*3), exchange_border_data_v()(*1,*3),
oh4s_map_particle_to_neighbor()(*1),
oh4s_map_particle_to_subdomain()(*1).

S_interiorp     • The array InteriorParts[2][S] of the S_interiorp structure is level-4s’s own and its
InteriorParts       element [p][s] specifies the contiguos subregion pbuf i(p, s) in pbuf (p, s) in which non-
halo particles are stored after the packing by move_to_sendbuf_4s() and particle
reception by xfer_particles() when partially position-aware particle transfer takes
place. The structure elements head and size have the head index and the size of
pbuf i(p, s) respectively. The reason why we need this array is that, unlike level-4p,
pbuf (p, s) may not be fully filled because halo particles are not yet received and thus
sort_particles() cannot scan all particles in pbuf (p, s) but should do only those in
pbuf i(p, s).
After allocated by init4s(), the array is let have the values shown above by move_
to_sendbuf_4s() and its callees move_to_sendbuf_uw4s() and move_to_sendbuf_
dw4s() to define each of pbuf i(p, s), and then referred to by sort_particles() to
scan the particles in pbuf i(p, s).


EXTERN int *PbufIndex;                                  /* [2*ns+1] */
EXTERN dint **NOfPGrid[2], **NOfPGridTotal[2];          /* [2][ns][z][y][x] */
EXTERN int **NOfPGridOut[2], **NOfPGridOutShadow[2];    /* [2][ns][z][y][x] */
EXTERN int **NOfPGridIndex[2], **NOfPGridIndexShadow[2];/* [2][ns][z][y][x] */
EXTERN dint *NOfPGridZ;                                 /* [z] */
EXTERN int ZBound[2][2], (*ZBoundShadow)[2];
struct S_hplane {
int nbor, stag, rtag;
int *nsend, *nrecv, *sbuf, *rbuf;                     /* [ns] */
};
EXTERN struct S_hplane HPlane[2][2];                    /* [2][2] */
struct S_vplane {
int nbor, stag, rtag;
int nsend, nrecv, sbuf, rbuf;
};
EXTERN struct S_vplane *VPlane;                         /* [2*nn+6] */
EXTERN int VPlaneHead[2*2*2+1];
EXTERN struct S_particle *BoundarySendBuf;

struct S_griddesc {
int x, y, z, w, d, h, dw;
};


<!-- Page 452 -->

EXTERN struct S_griddesc GridDesc[3];

struct S_interiorp {
int head, size;
};
EXTERN struct S_interiorp *InteriorParts;


In addition, we use the following variables a little bit diffenently from their usages in
lower level libraries but similarly to level-4p.

Particles     • As in level-4p, Particles[Plim] and SendBuf[Plim] are combined, and are used al-
SendBuf        ternately. However the size of each Plim  is now calculated by init4s() a little bit
differently as discussed later, and thus receiving the buffer from the simulator body
or allocating it is done by level-4s’s own function oh4s_particle_buffer().

The following functions refer to and/or modify both Particles[] and SendBuf[] di-
rectly or through macros.

move_to_sendbuf_4s(), move_to_sendbuf_uw4s(),
move_to_sendbuf_dw4s(), sort_particles(), move_and_sort(),
sort_received_particles(), xfer_particles(),
xfer_boundary_particles_v().

The function xfer_boundary_particles_h() refers to and modifies only SendBuf[],
while the following functions refer to and/or modify only Particles[].

count_population() oh4s_map_particle_to_neighbor(),
oh4s_map_particle_to_subdomain(), oh4s_inject_particle(),
oh4s_remove_mapped_particle().

nOfLocalPLimit     • Unlike  level-4p,  the  variable nOfLocalPLimitShadow = P lim′     is  calculated by
nOfLocalPLimitShadow       init4s() to let it be as follows.

Phalo = D((δmaxx  + 2)(δmaxy  + 2)(δmaxz  + 2) −δmaxx  δmaxy  δmaxz   )
Pmgn = Dδmaxx  δmaxy
⌈               ⌉
Plim′ = max( P(100 + α)/100  , P + minmargin) + 2(Phalo + Pmgn)

This calculation ensures that each node can have up to 2Phalo halo particles for largest
possible primary and secondary subcuboid, and that we have the margins of Pmgn
for each of primary and secondary particle sets due to the coarse unit of particle
assignment being those in a xy-plane of subdomain. The calculated value is also
reported to the simulator body through the maxlocalp argument of oh4s_init().

Then Plim = nOfLocalPLimit is given by the simulator body through the argument
maxlocalp of oh4s_particle_buffer() to let the library know the real amount and
confirm that it is not less than Plim.′   The variable is referred to by level-4s functions
exchange_particles4s(), move_to_sendbuf_4s() and oh4s_inject_particle().

NOfPLocal     • As in level-4p, NOfPLocal[2][S][N]  is private to level-4s library and maintained
by oh4s_map_particle_to_neighbor(), oh4s_map_particle_to_subdomain() and
oh4s_remove_mapped_particle(). In level-4s library NOfPLocal[][][] is also referred
to by transbound4s().


<!-- Page 453 -->

RecvBufBases     • As in level-4p, the pointer array RecvBufBases[2][S] has one extra element concep-
tually [2][0] so that sort_received_particles() can know the tail of rbuf (p, s).
This extra element is set by move_and_sort() or move_and_sort_secondary() and
referred to by sort_received_particles(), while other elements are referred to
also by them and move_to_sendbuf_4s(), move_to_sendbuf_uw4s() and xfer_
particles().

- Besides Particles[], SendBuf[], nOfLocalPLimit, NOfPLocal[][][] and RecvBufBases
[][], some other variables for particle buffers and population are also used in the level-4s
functions in their original meanings as follows.

  - TotalP[][] in transbound4s(), move_and_sort_primary(),
move_to_sendbuf_uw4s(), move_to_sendbuf_dw4s() and move_and_sort().

  - TotalPNext[][] in transbound4s(), count_population(),
make_send_sched(), make_send_sched_hplane(), move_to_sendbuf_4s(),
move_to_sendbuf_uw4s(), move_to_sendbuf_dw4s() and sort_particles().

  - primaryParts in move_to_sendbuf_4s() and move_and_sort() together with
the pointer to its shadow secondaryBase, while in count_population()
solitarily.

  - totalParts in oh4s_particle_buffer(), transbound4s(), rebalance4s(),
count_population(), move_to_sendbuf_4s(),
oh4s_map_particle_to_neighbor(), oh4s_map_particle_to_subdomain(),
oh4s_inject_particle() and oh4s_remove_mapped_particle() directly, and
through the macro Check_Particle_Location(). The function
transbound4s() also refers to the pointer to the shadow
totalLocalParticles.

  - nOfInjections in transbound4s(), rebalance4s(), count_population(),
move_to_sendbuf_4s(), move_and_sort(), and oh4s_inject_particle()
directly, and in oh4s_map_particle_to_neighbor() and
oh4s_map_particle_to_subdomain() through the macro
Check_Particle_Location().

  - InjectedParticles[][] in transbound4s(), rebalance4s(),
oh4s_map_particle_to_neighbor(), oh4s_map_particle_to_subdomain()
and oh4s_remove_mapped_particle().

nOfFields     • As in level-4p, init4s() intercepts  its argument ftypes to make  its substance
FieldTypes        FieldTypes[][] have the following additional entry f = F −1 to its tail for per-grid
FieldDesc        histogram.

  - [0] = ε(f) = 1 means that an entry of per-grid histogram has one (always 64-bit
integer, unlike level-4p) element.

  - [1:2] = {el(f), eu(f)} = {0, 0} means per-grid histogram does not have any
special extensions, as in level-4p.
  - [3:4] = {ebl(f), ebu(f)} = {−eg, eg} means the broadcast of per-grid histogram
should include eg thick extensions, unlike level-4p. These extensions are neces-
sary to let helpers have helpand’s sending or in other words exterior halo planes.
  - [5:6] = {erl (f), eru(f)} = {−eg, eg} means the reduction of per-grid histogram
should include its sending planes of eg thick, as in level-4p.


<!-- Page 454 -->

On the other hand, the mechanism of initialization and adjustment of FieldDesc[]
is same as that of level-4p.  That  is, FieldDesc[]  is allocated and initialized by
init_fields() called from init3() called from init4s(). Then adjust_field_
∏D−1
descriptor() adds (S−1)  d=0 Φd(F−1) = (S−1)G to FieldDesc[F−1].{bc,
red}.size[0] so that the broadcast and reduction for per-grid histogram are per-
formed on the whole of [p][S][G] rather than the one array element [p][s][G]. The
function is called from init4s() and update_descriptors(), the latter of which
is called from exchange_particles4s() and make_recv_list() on the helpand-
helper reconfiguration with anywhere and normal accommodation respectively.  It
is also same as level-4p that FieldDesc[F−1].{bc, red} are referred to by reduce_
population() and make_recv_list() for reduction and broadcast of per-grid his-
togram respectively.

nOfExc     • As in level-4p, init4s() intercepts its arguments cfields and ctypes to make their
BoundaryCommFields        substances BoundaryCommFields[] and BoundaryCommTypes[][][][] have one additional
BoundaryCommTypes        entry C−1 for each, to have F−1 for the former and the followings for the latter.
BorderExc
  - Unlike level-4p,  [0][0][] = {−eg, eg, 2eg} to mean that the sending planes of
the downward communication are 2eg thick and consist of the lower bound-
ary plane(s) and the plane(s) just below it (or them), while receiving planes are
just above the upper sending planes.
  - Also unlike level-4p, [0][1][] = {−eg, −3eg, 2eg} means that the sending planes
of the upward communication are 2eg thick and consist of the upper boundary
plane(s) and the plane(s) just above it (or them), while receiving planes are just
below the lower sending planes.

On the other hand, BoundaryCommTypes[C−1][b][][] for all b ∈[1, B) are set to 0 to
mean no communication for non-periodical system boundaries, as in level-4p.

It is also same as level-4p that BorderExc[][][][] is allocated and initialized by init_
fields() called from init3() called from init4s(), and its element [C−1][][][] is
referred to by oh3_exchange_borders() called from exchange_population().


#### 4.12.4 Variables for Particle Transfer Scheduling

The next variable group is for the particle transfer scheduling. Before showing them, we
revisit the following variables whose usages are slightly different from those in lower level
libraries and from level-4p in some of them.

S_commlist     • As done in the level-1 and level-4p library, we build the secondary mode particle
CommList        transfer schedule in the array of S_commlist structure CommList[]. However, some of
SecRList       the structure elements have meanings different from those in level-1 and level-4p as
RLIndex         follows.

  - rid is the ID r of the node by which particles specified by the record should be
accommodated, as in level-1 and level-4p.

  - Unlike level-4p, region is the z-coordinate of the topmost xy-plane of the sub-
cuboid assigned to r. That is, r will accommodate particles in the subcuboid
whose z-coordinates are in (z′, z] where z′ is region of the previous record or
−1 if the record in question is the first one.

  - tag is 0 for primary particles of r or NS for its secondary ones, as in level-1 and
level-4p.


<!-- Page 455 -->

  - count is always 0, unlike level-4p because we don’t have hot-spot records.
  - sid is always 0, unlike level-4p because we don’t have hot-spot records.

As for the blocks in CommList[], they are very similar to those in level-4p but the sizes
are differnt from them and the alternative secondary receiving block is followed by
alternative secondary sending block.

primary receiving block is build by each node for particles in its primary subdo-
main to be accommodated by the node itself or its helpers. Since for a subdomain
n, the records for each member of F(n) appear at most once, the size of this
block is at most |F(n)| ≤N.
primary sending block is exchanged by neighboring node (subdomain) pairs. A
node receives the whole primary receiving block from each neighbor for particles
sent from the family members rooted by the node to the family members rooted
by the neighbor.  Since we avoid receiving a primary receiving block twice or
more from a neighbor and a node can appear at most two primary receiving
blocks as a helpand and a helper, the size of this block is at most 2N.
As in level-4p, the element [k] (k < 3D) of the integer array RLIndex[3D + 1]
has the CommList’s index of the first record of primary receiving block received
from the k-th neighbor node, or 0 if the node is the local node itself. Also as in
level-4p, all elements for a node having multiple neighbor indices commonly have
the index of the sole primary receiving block of the node, and RLIndex[3D] has
the index of the record just following the primary sending block, or the combined
size of primary receiving and primary sending blocks in other words.
secondary receiving and secondary sending blocks for a node are the copies of
the primary receiving and primary sending block of its helpand which broadcasts
the blocks to the helpers to show them particle accommodations for helpand’s
primary subdomain and its neighbors and thus helper’s secondary subdomain
and its neighbors, as in level-4p.  Therefore, the size of this block is at most
3N. The pointer SecRList points the head of the secondary receiving block,
and SecRLIndex[3D + 1] shown later is the copy of RLIndex[] of the helpand,
also as in level-4p.
alternative secondary receiving block for a node is the copy of primary receiving
block given from the new helpand on helpand-helper reconfiguration as in level-1
and level-4p, but is different from them because it is followed by alternative
secondary sending block being the copy of the primary sending block of the
helpand. The reason why we need alternative secondary sending block is that
the node should know subcuboids of the neighbor family members of its new
helpand to build the transfer schedule of halo particles. The combined size of
these blocks is at most 3N. The pointer AltSecRList shown later points the
head of alternative secondary receiving block as in level-4p, but we also have
AltSecRLIndex[3D + 1] being the copy of RLIndex[] of the new helpand and
having indices in alternative secondary receiving/sending blocks.

Note that we don’t have hot-spot sending block because we have no hot-spots. There-
fore, the total of the maximum size of each block is 9N being definitely less than
2 · 3D(NS + 1) + N(S + 3) ≥10N that level-1 requires. Therefore, init1() may be
unaware of the amount required for level-4s78.

78The function init1() is still aware of the amount required for level-4p, but in 3-dimensional simulations
only to which the level-4s library can be applied, level-1’s requirement is always larger than level-4p’s.


<!-- Page 456 -->

NOfSend     • As in level-4p, we use NOfSend[2][S][N] for the per-receiver sending histogram so
NOfRecv        that its element [p][s][m] has the number of particles of species s which the local node
should send to the receiver node m as m’s (not the local node’s) primary (p = 0) or sec-
ondary (p = 1) particles. Each element is accumulated by make_send_sched_body()
using the macro Make_Send_Sched_Body(). Then we perform a hand-made all-to-all
communication among neighboring family members in exchange_xfer_amount() to
exchange NOfSend[] to have the per-sender receiving histogram in NOfRecv[2][S][N]
so that its element [p][s][m] has the number of particles of species s which the local
node should receive from the sender node m as the local node’s (not m’s) primary
(p = 0) or secondary (p = 1) particles.

Also as in level-4p, NOfSend[p][s][m] then acts as the index of a portion of SendBuf[],
sbuf (p, s, m), to which a particle of species s to be sent to m as m’s primary (p = 0)
or secondary (p = 1) particle is moved. This role change is done by set_sendbuf_
disps4s(), and then the macro Move_Or_Do() used in move_to_sendbuf_4s(),
move_to_sendbuf_uw4s(), move_to_sendbuf_dw4s() and move_and_sort() incre-
ments an element each time a particle is moved from Particles[] to SendBuf[] for
sending.

Then particles are sent in xfer_particles() referring to NOfSend[][][] for the send
count, each element referred to and thus possibly having non-zero is zero-cleared for
the accumulation in the next call of transbound4s().  All the entries, in addtion,
are also zero-cleared in init4s() at the very beginning and before the first call of
transbound4s(), and in exchange_particles4s() when we have anywhere accom-
modation with which NOfSend[] are modified by exchange_particles().

On the other hand, NOfRecv[][][] does not have any other roles because of no hot-spots.

Requests     • The usage of Requests[] and Statuses[] to keep the requests/statuses of asynchronous
Statuses     MPI communications  is not changed, and their required sizes for level-4s’s own
communications for halo particle (or particle-associated data) transfer is less than
4SN + 2 · 3D, because those for particles in vertical halo planes and horizontal halo
planes are up to 2 × (2N + 6) and (2 × 2 × 2)S respectively as discussed in §4.12.379.

Requests[] and Statuses[] are referred to in;

exchange_xfer_amount(), xfer_particles(),
xfer_boundary_particles_v(), xfer_boundary_particles_h(),
exchange_border_data_v() and exchange_border_data_h().

Now we show the variables and struct data types for the particle transfer scheduling.

AltSecRList     • As in level-4p, the S_commlist-type pointer AltSecRList is let point the head of
alternative secondary receiving block in CommList[] by make_recv_list(), but the
block is followed by alternative secondary sending block unlike level-4p. Then it is
referred to by make_send_sched() and make_bxfer_sched() directly, and by make_
send_sched_self(), make_bsend_sched() and make_brecv_sched() through their
arguments rlist.

PrimaryCommList     • The array of S_commlist structure PrimaryCommList[2][3D] is level-4s’s own to have
the trivial receiving record for the k-th neighbor in its element [p][k] to be used when
we will in primary mode in the next step to determine the node to which we send

798S can be larger than 4SN when N = 1 but, since the very single node cannot have secondary
subdomain if so, the required amount is 4S = 4SN.


<!-- Page 457 -->

primary (p = 0) and secondary (p = 1) particles. That is, PrimaryCommList[p][k]
has mk = Neighbors[p][3D −k −1] in rid, δz(mk) −1 in region to mean mk’s
subcuboid is the primary subdomain of mk itself, and 0 in other elements particularly
for tag to mean the record is for primary particles for mk. This list is required to use
exchange_particles4s() regardless of the next execution mode and thus even in the
mode is primary. The array is let have the elements above in update_neighbors(),
and is referred to by exchange_particles4s() directly and by make_send_sched(),
make_bxfer_sched() and their callees through their rlist arguments.

SecRLIndex     • As in level-4p, the integer array SecRLIndex[3D + 1] has the index of secondary
receiving and secondary sending blocks in CommList[] in its element [k] for the k-th
neighbor of the local node’s helpand if k < 3D, while the element [3D] has the index
of the block following secondary sending block, i.e., alternative secondary receiving
block, or the combined size of secondary receiving and secondary sending blocks in
other words. The array is obtained from the helpand by its broadcast of RLIndex[] in
make_recv_list(), and then is referred to by exchange_particles4s() directly and
by make_send_sched() and make_bxfer_sched() through their rlidx arguments.

AltSecRLIndex     • The integer array AltSecRLIndex[3D + 1] is level-4s’s own and has the index of alter-
native secondary receiving/sending blocks in CommList[] in its element [k] (k < 3D)
for the k-th neighbor of the local node’s new helpand when helpand-helper reconfig-
uration takes place, while the element [3D] has the size of the block. The necessity
of this array was shown in the discussion of CommList[] in this section. The array is
obtained from the helpand by its broadcast of RLIndex[] in make_recv_list(), and
then is referred to by make_send_sched() and make_bxfer_sched().

PrimaryRLIndex     • The integer array PrimaryRLIndex[3D] is level-4s’s own and has the trivial index k in
its element [k] to show the record for the k-th neighbor is in PrimaryCommList[p][k]
as discussed above. The array is let have the elements above in init4s(), and is
referred to by exchange_particles4s() directly and by make_send_sched() and
make_bxfer_sched() through their rlidx arguments.

S_recvsched_context     • The struct named S_recvsched_context is to keep the execution context of the
function sched_recv() as in level-4p, but its elements are different from level-4p as
follows.

  - z are the local z-coordinate of the xy-plane of per-grid histogram to be vistied.

  - nptotal is the number of particles which have already processed, as in level-4p.

  - nplimit is the total number of particles which the nodes already visited are
expected to accommodate by the balancing algorithm, as in level-4p.

  - cptr is the pointer to a record in CommList[] to be built, as in level-4p.

The structure is initialized by the caller make_recv_list() and then referred to and
updated by sched_recv().

T_Hgramhalf     • The MPI_Datatype variable T Hgramhalf is perfectly equivalent to its level-4p coun-
terpart, and thus has the MPI data-type for a slice [p][∗][m] in NOfSend[][][] and
NOfRecv[][][] to send/receive the particle populations the node m should accommo-
date as its primary (p = 0) or secondary (p = 1) particles. The value of this variable
is created by MPI_Type_vector() called in init4s() so that the type has S elements
with the stride of N, and is used in exchange_xfer_amount().


<!-- Page 458 -->

Note that we have neither variables nor structures for hot-spots because we have none of
them.

EXTERN struct S_commlist *AltSecRList, PrimaryCommList[2][OH_NEIGHBORS];
EXTERN int SecRLIndex[OH_NEIGHBORS+1], AltSecRLIndex[OH_NEIGHBORS+1];
EXTERN int PrimaryRLIndex[OH_NEIGHBORS];

struct S_recvsched_context {
int z;
dint nptotal, nplimit;
struct S_commlist *cptr;
};
EXTERN MPI_Datatype T_Hgramhalf;



#### 4.12.5 Variables for Neighboring Information

Next, we declare arrays to hold neighboring information. Since they are perfectly equivalent
to their level-4p counterparts discussed in §4.9.5, we briefly discuss them focusing on the
functions referring to them.

FirstNeighbor     • The  element  [k]  of  the  integer  array  FirstNeighbor[3D]  has  k   if m =
SrcNeighbors[k] ≥0 or m = −N −1 to mean the first occurrence, or k′ such that
m = −(SrcNeighbors[k′] + 1). The array is let have these values by init4s() and
then is referred to by make_recv_list().

GridOffset     • The array GridOffset[2][3D] has the offset goff(k) to translate a grid-position of the
k-th neighbor m of the local node n’s primary (p = 0) or secondary (p = 1) subdomain
np = {n, parent(n)}[p] into the corresponding grid-position of np in the element [p][k].
The values [p][] are initialized/updated when Neighbors[p][] is initialized/updated by
the function update_neighbors(), and then is referred to through the macro Local_
Grid_Position() invokded in the macro Move_Or_Do() and in the function oh4s_
remove_mapped_particle().

S_realneighbor     • The arrays RealDstNeighbors[2][2] and RealSrcNeighbors[2][2] of S_realneighbor
RealDstNeighbors        structure have the sets of nodes in the neighboring families of the local node. The
RealSrcNeighbors        structure element nbor[N] is the array of a node set and n has its cardinality. The
arrays are allocated by init4s(), are updated by update_real_neighbors() and
its callee upd_real_nbr(), and then are referred to by exchange_xfer_amount(),
move_and_sort(), set_sendbuf_disps4s() and xfer_particles().

On the other hand, we modified the definitions of the following arrays declared in level-1
library, not only in the manner done in level-4p but also in a level-4p’s own way.

Neighbors     • The element array [2][] of Neighbors[3][N] temporarily has the neighbors of the local
DstNeighbors        node’s helpand by build_new_comm(), as in level-4p. The added element is referred
SrcNeighbors        to by upd_real_nbr() to construct RealSrcNeighbors[1][1] and then copied into
Neighbors[1][] by rebalance4s(). Besides this extra role, level-4s’s Neighbors[0][] =
DstNeighbors[] and SrcNeighbors[] are different from that in lower levels and level-
4p, because their elements should have −(N + 1) if the corresponding neighbors in
level-1’s definition are beyond non-periodic system boundaries. This modification is
done by init4s() referring to the values set by init1(), while letting Neighbors[1][]
have the helpand’s Neighbors[0][] is done in the way perfectly equivalent to other


<!-- Page 459 -->

levels, i.e., by build_new_comm(). Besides these assignments, Neighbors[][] is referred
to by make_send_sched(), update_neighbors() and make_bxfer_sched(), while
DstNeighbors[] and SrcNeighbors[] are referred to by make_recv_list().

TempArray     • The array TempArray[] has 4N elements for update_real_neighbors() and its callee
upd_real_nbr(), while init4s() allocates it and uses its first 2N elements to build
DstNeighbors[] = Neighbors[0][], SrcNeighbors[] and FirstNeighbor[].


EXTERN int FirstNeighbor[OH_NEIGHBORS], GridOffset[2][OH_NEIGHBORS];
struct S_realneighbor {
int n, *nbor;
};
EXTERN struct S_realneighbor RealDstNeighbors[2][2], RealSrcNeighbors[2][2];



#### 4.12.6 Variable for Boundary Condition

BoundaryCondition  The last variable BoundaryCondition[D][2] is perfectly equivalent to level-4p’s discussed in
§4.9.6, and thus is the substance of the oh4s_init()’s argument bcond to have the system
boundary conditions. The array is initialized in init4s() and is referred to by the macro
Map_To_Grid() used in oh4s_map_particle_to_subdomain().


EXTERN int BoundaryCondition[OH_DIMENSION][2];



#### 4.12.7 Function Prototypes

The next and last block is to declare the prototypes of the API function pairs each of which
consists of API for Fortran and C, as listed below with marks “[E]” for those equivalent to
level-4p’s, “[M]” for those different from level-4p’s, and “[N]” for those newly introduced
for level-4s.

- The function oh4s_init[_]() [M] initializes data strucutures of the level-4s and lower
level libraries.

- The function oh4s_particle_buffer[_]() [N] defines Plim and particle buffers
Particles[] and SendBuf[].

- The function oh4s_per_grid_histogram[_]() [M] defines arrays for per-grid his-
togram and per-grid index.

- The function oh4s_transbound[_]() [M] at first performs what its level-1 counter-
part oh1_transbound[_]() does to have the fundamental particle assignment for
load balancing, and then modifies it to have position-aware particle distribution by
the level-4s’s own particle transfer.

- The function oh4s_exchange_border_data[_]() [N] transfers halo part of particle-
associated array data.

- The function oh4s_map_particle_to_neighbor[_]() [E] finds the subdomain in
which a given particle resides, providing that the subdomain is a neighbor of the
primary/secondary subdomain of the local node, and maintains per-subdomain and
per-grid histograms of particle population.


<!-- Page 460 -->

- The function oh4s_map_particle_to_subdomain[_]() [E] finds the subdomain in
which a given particle resides, allowing that the subdomain is not necessary to be
a neighbor of the primary/secondary subdomain of the local node, and maintains
per-subdomain and per-grid histograms of particle population.

- The function oh4s_inject_particle[_]() [E] injects a particle and place it at the
bottom of Particles[] maintaining per-subdomain and per-grid histograms of particle
population.

- The function oh4s_remove_mapped_particle[_]() [E] removes a particle which has
been mapped to a subdomain or been injected into a subdomain.

- The function oh4s_remap_particle_to_neighbor[_]() [E] does what functions
oh4s_remove_mapped_particle() and oh4s_map_particle_to_neighbor() do.

- The function oh4s_remap_particle_to_subdomain[_]() [E] does what functions
oh4s_remove_mapped_particle() and oh4s_map_particle_to_subdomain() do.

As done in §4.2.11, §4.4.5, §4.6.6, and §4.9.7, prior to showing the function prototypes,
we show the fifth and (so far) last part of the header files ohhelp c.h for C-coded simulators
and ohhelp f.h for Fortran-coded ones, which defines the aliases of level-4s API functions.
In the #else part of #ifdef␣OH_LIB_LEVEL_4P, at first they #define the aliases of API
functions.

#else
#define \
oh_init(A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,A16,A17,A18,
A19,A20,A21,A22,A23,A24,A25,A26) \
oh4s_init(A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,A16,A17,A18,
A19,A20,A21,A22,A23,A24,A25,A26)
#define oh_particle_buffer(A1,A2) oh4s_per_grid_histogram(A1,A2)
#define oh_per_grid_histogram(A1,A2) oh4s_per_grid_histogram(A1,A2)
#define oh_transbound(A1,A2)      oh4s_transbound(A1,A2)
#define oh_exchange_border_data(A1,A2,A3,A4) \
oh4s_exchange_border_data(A1,A2,A3,A4)
#define oh_map_particle_to_neighbor(A1,A2,A3) \
oh4s_map_particle_to_neighbor(A1,A2,A3)
#define oh_map_particle_to_subdomain(A1,A2,A3) \
oh4s_map_particle_to_subdomain(A1,A2,A3)
#define oh_inject_particle(A1,A2) oh4s_inject_particle(A1,A2)
#define oh_remove_mapped_particle(A1,A2,A3) \
oh4s_remove_mapped_particle(A1,A2,A3)
#define oh_remap_particle_to_neighbor(A1,A2,A3) \
oh4s_remap_particle_to_neighbor(A1,A2,A3)
#define oh_remap_particle_to_subdomain(A1,A2,A3) \
oh4s_remap_particle_to_subdomain(A1,A2,A3)

Then ohhelp c.h gives the prototypes of the functions above, which are also given in
ohhelp4s.h80, while their Fortran versions are given in oh mod4s.F90 as shown in §3.8.

void oh4s_init(int **sdid, const int nspec, const int maxfrac,
const long long int npmax, const int minmargin,

80Prototypes of oh4s init() in ohhelp c.h and ohhelp4s.h are slightly different, i.e., the type of its fourth
argument npmax is long long int in the former, while in the latter is dint.


<!-- Page 461 -->

const int maxdensity, int **totalp, int **pbase,
int *maxlocalp, int *cbufsize, void *mycomm, int **nbor,
int *pcoord, int **sdoms, int *scoord, const int nbound,
int *bcond, int **bounds, int *ftypes, int *cfields,
int *ctypes, int **fsizes, int **zbound,
const int stats, const int repiter, const int verbose);
void oh4s_particle_buffer(const int maxlocalp, struct S_particle **pbuf);
void oh4s_per_grid_histogram(int **pghgram, int **pgindex);
int  oh4s_transbound(int currmode, int stats);
void oh4s_exchange_border_data(void *buf, void *sbuf, void *rbuf,
MPI_Datatype type);
int  oh4s_map_particle_to_neighbor(struct S_particle *part, const int ps,
const int s);
int  oh4s_map_particle_to_subdomain(struct S_particle *part, const int ps,
const int s);
int  oh4s_inject_particle(const struct S_particle *part, const int ps);
void oh4s_remove_mapped_particle(struct S_particle *part, const int ps,
const int s);
int  oh4s_remap_particle_to_neighbor(struct S_particle *part, const int ps,
const int s);
int  oh4s_remap_particle_to_subdomain(struct S_particle *part, const int ps,
const int s);

Then ohhelp4s.h continues prototype declaration for Fortran API functions.


void oh4s_init_(int *sdid, const int *nspec, const int *maxfrac,
const dint *npmax, const int *minmargin, const int *maxdensity,
int *totalp, int *pbase, int *maxlocalp, int *cbufsize,
struct S_mycommf *mycomm, int *nbor, int *pcoord, int *sdoms,
int *scoord, const int *nbound, int *bcond, int *bounds,
int *ftypes, int *cfields, int *ctypes, int *fsizes,
int *zbound,
const int *stats, const int *repiter, const int *verbose);
void oh4s_particle_buffer_(const int *maxlocalp, struct S_particle *pbuf);
void oh4s_per_grid_histogram_(int *pghgram, int *pgindex);
int  oh4s_transbound_(int *currmode, int *stats);
void oh4s_exchange_border_data_(void *buf, void *sbuf, void *rbuf, int *type);
int  oh4s_map_particle_to_neighbor_(struct S_particle *part, const int *ps,
const int *s);
int  oh4s_map_particle_to_subdomain_(struct S_particle *part, const int *ps,
const int *s);
int  oh4s_inject_particle_(const struct S_particle *part, const int *ps);
void oh4s_remove_mapped_particle_(struct S_particle *part, const int *ps,
const int *s);
int  oh4s_remap_particle_to_neighbor_(struct S_particle *part, const int *ps,
const int *s);
int  oh4s_remap_particle_to_subdomain_(struct S_particle *part, const int *ps,
const int *s);
