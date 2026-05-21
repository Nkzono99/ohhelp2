# 4.2 Header File ohhelp1.h

Source: `doc/original/ohhelp.pdf`, pages 132-156.

<!-- Page 132 -->

## 4.2 Header File ohhelp1.h

The header file of level-1 library, ohhelp1.h, #define’s a few basic constants and shorthands,
defines structured data types, declares global variables used in level-1 and higher level C
codes, and gives prototypes of API functions and those called by higher level codes.


#### 4.2.1 Header File Inclusion

The first part of ohhelp1.h has a few lines to include the following standard headers.

- stdio.h for printing debug and statistics messages.

- stdlib.h for malloc() for allocate data allocations and qsort() for balanced particle
distribution in a family.

- string.h for strcat() to create debug messages.

- limits.h to refer to INT_MAX for statistics calculation.

- float.h to refer to DBL_MAX for statistics calculation.

- stdarg.h for vprintf() and other variable-number-argument stufffor verbose and
debug messaging.

- mpi.h for MPI functions and constants.

In addtion to them, we also include our own header files, oh config.h to define the number of
dimensions D = OH_DIMENSION of simulated space and the library level as discussed in §3.3,
and oh stats.h to define keys and identification strings for timing measurement intervals as
discussed in §3.10.1.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <stdarg.h>
#include <mpi.h>

#include "oh_config.h"
#include "oh_stats.h"



#### 4.2.2 Constants and Shorthands

Next stuffis a sequence of a few constant and shorthand definitions. We define the following
constants.

TRUE     • If TRUE and FALSE have not been defined in any standard header files we have included,
FALSE       they are simply defined as integer constants 1 and 0.

OH_POS_AWARE     • The switch OH_POS_AWARE is defined if OH_LIB_LEVEL_4PS is also defined to mean
that level-4p/4s extension and thus position-aware particle management are in effect.


<!-- Page 133 -->

OH_DIM_X     • The constants OH DIM X = 0, OH DIM Y = 1 and OH DIM Z = 2 are used as indices of
OH_DIM_Y        arrays having dimension-dependent values such as that keeping the grid size of the
OH_DIM_Z       simulated space domain.

OH_NEIGHBORS     • The constant OH NEIGHBORS is defined as 3D using #if/#elif/#else/#endif con-
struct examining OH_DIMENSION. This number is one more than the number of sub-
domain cuboids contacted with a subdomain, in order to include the subdomain itself
into its neighbors.

Then we define the following shorthands.

MCW     • The constant MCW is the shorthand of the MPI constant MPI_COMM_WORLD. Since almost
all MPI function requires communicator arguments and they are almost always the
lengthy MPI_COMM_WORLD, we introduced this shorthand to save typing. Also it might
be useful if your simulator has some MPI processes not correspoing to decomposed
subdomains, because you may replace MPI_COMM_WORLD with a global variable having
the communicator for prosseses working with subdomains. That is, for example, you
may modify the initializer function oh1_init[_]() (§4.3.3) to give it an additional
argument having your own communicator and to assign it to the global variable
equivalent to MCW.  If you do so and you create the communicator in Fortran code,
remember that you have to transform it into C’s counterpart by MPI_Comm_f2c() in
the modified oh1_init[_]().

dint     • The data-type dint is the short hand of 64-bit integer long␣long␣int. Note that
long␣int can be used with 64-bit compliant C compilers but long␣long␣int is safe
for both of 32-bit and 64-bit compilers (so far).

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifdef  OH_LIB_LEVEL_4PS
#define OH_POS_AWARE
#endif

/* constants for D-dimensional simulation */
#define OH_DIM_X        0
#define OH_DIM_Y        1
#define OH_DIM_Z        2
#if OH_DIMENSION==1
#define OH_NEIGHBORS    3
#elif OH_DIMENSION==2
#define OH_NEIGHBORS    (3*3)
#else
#define OH_NEIGHBORS    (3*3*3)
#endif

#define MCW MPI_COMM_WORLD      /* shorthand of MPI_COMM_WORLD */

typedef long long int dint;     /* shorthand of 64-bit integer */


<!-- Page 134 -->

#### 4.2.3 Basic Process Configuration Variables

Here we start the declarations of global variables and structured data-types. Before list-
ing the first variable group for basic configuration, we note a well-known trick to avoid
duplicated declarations.

EXTERN  Each global variable declaration has a prefix EXTERN which will be defined as an empty string
or C keyword extern. That is, among C source files in which we include ohhelp1.h, only
one file, namely ohhelp1.c, defines EXTERN as empty to give the variables their home, while
other files, namely ohhelp2.c and higher level library sources, let EXTERN be #undef’ined to
make ohhelp1.h #define it as extern and to refer to the variables defined in the other C
file.

Now here is the list of global variables to represent the basic process configuration of
simulation.

nOfNodes     • The integer variable nOfNodes has the number of computation nodes (MPI processes)
involved in the parallel simulation. That is, nOfNodes is the size of MPI_COMM_WORLD
given by MPI_Comm_size() and is set by init1(). This variable is referred to in many
functions. Hereafter the value of nOfNodes is denoted by N.

myRank     • The integer variable myRank has the rank of the local node (process). That is, myRank
is the rank of MPI_COMM_WORLD given by MPI_Comm_rank() and is set by init1().
This variable is referred to in many functions.

RegionId     • The integer array RegionId[2] has identifiers of primary (0) and secondary (1) sub-
SubdomainId       domains. Since a subdomain identifier is that of the node which is responsible for the
subdomain as its primary one, RegionId[0] is always equivalent to myRank and thus
is set by init1(). On the other hand, RegionId[1] has the rank of the local node n’s
parent being parent(n) and thus it may be −1 if n is the root of the helpand-helper tree
or we are in primary mode. Therefore, it is set to −1 by init1() and try_primary1()
to indicate that the local node does not have the secondary subdomain as any other
nodes, while rebalance1() sets it to parent(n) when it (re)builds the helpand-helper
relationship. The array is referred to by count_stay(), transbound3(), oh3_map_
particle_to_neighbor(), oh3_exchange_borders() and set_border_exchange(),
while the simulator body does so through the shadow of the array pointed by
SubdomainId to protect RegionId from accidental modifications. That is, the body
of SubdomainId is allocated by the simulator body which gives (double) pointer to
it oh1_init() through the argument sdid, or by init1() if sdid points NULL, and
init1(), try_primary1() and rebalance1() update the body when they update
RegionId[].

currMode     • The integer variable currMode has one of the following values, which are usually
returned from oh1_transbound(), or of its level-2/3 counterparts oh2_transbound()
or oh3_transbound(), but can be modified by other functions to force anywhere
accommodation indicated by bit-1.

MODE_NORM_PRI           0: (MODE NORM PRI) The next simulation step is executed in primary mode.

MODE_NORM_SEC           1: (MODE NORM SEC) The next simulation step is executed in secondary mode keeping
the helper-tree unchanged from the last step.

MODE_REB_SEC       −1: (MODE REB SEC) The next simulation step is executed in secondary mode with
the reconfiguration of the helper-tree.


<!-- Page 135 -->

MODE_ANY_PRI           2: (MODE ANY PRI) The current simulation step is executed in primary mode with
anywhere accommodation regardless of the real accommodation status.

MODE_ANY_SEC           3: (MODE ANY SEC) The current simulation step is executed in secondary mode with
anywhere accommodation regardless of the real accommodation status.

After initialized to MODE_NORM_PRI by init1(), currMode is set to MODE_NORM_PRI,
MODE_NORM_SEC, or MODE_REB_SEC by transbound1() or transbound2() and possibly
modified by functions such as those for position-aware particle management. Then
the variable is referred to by three functions; transbound1() to check the simulator
body and the library agree the execution mode; set_total_particles() to know if
NOfPLocal[1][][] are valid; and the level-3 API oh3_exchange_borders() to decide
whether it broadcasts exchanged boundary values of a field-array to helpers.

The functions above and others called from them with the argument currmode as the
local version of currMode use the following macros to examine and/or modify the bit-0
of primary/secondary mode indicator and bit-1 of normal/anywhere accommodation
indicator.

Mode_PS()        – Mode PS(M) examines primary/secondary mode indicator of M.

Mode_Acc()        – Mode Acc(M) examines normal/anywhere accommodation indicator of M.

Mode_Set_Pri()        – Mode Set Pri(M) is to set mode indicator of M to primary.

Mode_Set_Sec()        – Mode Set Sec(M) is to set mode indicator of M to secondary.

Mode_Set_Norm()        – Mode Set Norm(M) is to set accommodation indicator of M to normal.

Mode_Set_Any()        – Mode Set Any(M) is to set accommodation indicator of M to anywhere.

Mode_Is_Norm()        – Mode Is Norm(M) is true iffM indicates normal accommodation including rebal-
ancing.

Mode_Is_Any()        – Mode Is Any(M) is true iffM indicates anywhere accommodation.

accMode     • The integer variable accMode has 0 for normal accommodation or 1 for anywhere one
given by oh1_transbound(). It is also initialized by init1() to be 0. The variable
is referred to by oh1_accom_mode() to give its value to the caller of the function as
the return value.


#ifndef EXTERN
#define EXTERN extern
#endif

/* Basic process configuration variables */
EXTERN int nOfNodes;
EXTERN int myRank;
EXTERN int RegionId[2], *SubdomainId;
#define MODE_NORM_PRI (0)
#define MODE_NORM_SEC (1)
#define MODE_REB_SEC  (-1)
#define MODE_ANY_PRI  (2)
#define MODE_ANY_SEC  (3)
#define Mode_PS(M)       (M&1)
#define Mode_Acc(M)      (M&2)
#define Mode_Set_Pri(M)  (M&2)
#define Mode_Set_Sec(M)  (M|1)


<!-- Page 136 -->

#define Mode_Set_Norm(M) (M&1)
#define Mode_Set_Any(M)  (M|2)
#define Mode_Is_Norm(M)  (M<2)
#define Mode_Is_Any(M)   (M>=2)
EXTERN int currMode, accMode;



#### 4.2.4 Particle Histograms

The next variable group is for particle histograms. We have the followings to count the
number of particles.

nOfSpecies     • The integer variable nOfSpecies has the number of species of particles. This number
is not necessary to mean the real number of species, e.g., the number of variations
of particle mass and charge. Instead, this variable must have the number of memory
regions each of which accommodates particles of a species, as discussed in §3.2.1.
This variable should be given by the simulator body through the argument nspec of
oh1_init(), and is referred to in many functions. Hereafter the value of nOfSpecies
is denoted by S.

maxFraction     • The integer variable maxFraction has the tolerance factor percentage. This variable
should be given by the simulator body through the argument maxfrac of oh1_init(),
and is referred to in transbound1() to calculate nOfLocalPMax. Hereafter the value
of maxFraction is denoted by α.

NOfPLocal     • The element [p][s][m] of the integer array NOfPLocal[2][S][N] has the number of
primary (p = 0) or secondary (p = 1) particles of species s residing in the subdomain m
and accommodated by the local node. The simulator body should give the (double)
pointer to the array through the argument nphgram of oh1_init(), or a pointer
to NULL to allocate its body by init1(), and should set each element by counting
particles before calling oh1_transbound() which then clears all elements in the array
to 0 upon its return. This array is also referred to in many other functions. Hereafter
NOfPLocal of the node n is denoted by q(n).

NOfPrimaries     • The element [p][s][m] of the integer array NOfPrimaries[2][S][N] has the number
of particles of species s in the local node’s primary subdomain and accommodated
by the node m as  its primary (p = 0) or secondary (p = 1) particles.  Since
NOfPrimaries[p][s][m] of the local node n is equal to q(m)[p][s][n], NOfPrimaries is
built by collecting q(m)[∗][∗][n] using MPI_Alltoall() in transbound1(). This array
is allocated by init1() and is referred to in try_primary1(), schedule_particle_
exchange(), sched_comm(), stats_primary_comm(), try_primary2() and move_
to_sendbuf_primary().

TotalPGlobal     • The element [m] of the 64-bit integer array TotalPGlobal[N+1] has the system-
wide total number of particles residing in the subdomain m, namely Pm.  Since
TotalPGlobal[m] for m < N is defined as

N−1∑ ∑  S−1∑
Pm = TotalPGlobal[m] =                 q(k)[p][s][m]
k=0 p∈{0,1} s=0

the values of this array are calculated by MPI_Allreduce() in transbound1(). On the
other hand, the element [N] is used to detect non-neighboring particle transfer and is


<!-- Page 137 -->

non-zero if so, in transbound1(). This array is allocated by init1() and is referred to
in try_primary1(), try_stable1(), rebalance1(), push_heap(), remove_heap(),
try_primary2() and move_to_sendbuf_primary().

nOfParticles     • The 64-bit integer variable nOfParticles has the total number of particles residing
in the simulated space domain. Thus its value, denoted by P hereafter, is calcu-
∑N−1
lated by transbound1() such that P =   m=0 Pm. The variable is referred to in
rebalance1().

nOfLocalPMax     • The integer variable nOfLocalPMax have the maximum number of particles which a
local node can accommodate. Its value Pmax is calculated by transbound1() by;

Pmax = ⌊P(100 + α)/(100N)⌋

This variable is referred to in try_primary1(), try_stable1(), assign particles
() and try_primary2().

NOfPToStay     • The element [m] of the 64-bit integer array NOfPToStay[N] has the number of particles
residing in the subdomain m and accommodated by nodes responsible for m as their
primary or secondary subdomains, excluding those injected in the subdomain by the
nodes themselves. That is, NOfPToStay[m] is defined as

q′(m)[0][s][m] = q(m)[0][s][m] −qinj(m)[0][s]
q′(c)[0][s][parent(c)] = q(c)[1][s][parent(c)] −qinj(c)[1][s]
                
S−1∑      ∑
NOfPToStay[m] =    q′(m)[0][s][m] +     q′(c)[1][s][m] 
s=0               c∈H(m)

where qinj(m)[p][s] is InjectedParticles[p][s] of m, and H(m) is the set of helpers
of m, or H(m) = {c | parent(c) = m} in other word.  Therefore the values of this
array are calculated by each local node and then gathered by MPI_Allgather() in
count_stay(). Then its caller try_stable1() refers to it possibly decrementing the
element [parent(m)] if the node m has to throw a part of its secondary particles away.
The array is allocated by init1().

TotalP     • The element [p][s] of the integer array TotalP[2][S] has the number of primary (p = 0)
or secondary (p = 1) particles of species s accommodated by the local node n. Set-
ting its elements is done in transbound1() by copying corresponding elements from
TotalPNext[][]. The function also allocates the body of the array on its first call                                         ∑N−1
and initializes its primary elements [0][s] by  m=0 q(n)[0][s][m] and clears secondary
elements [1][s] with 0 by set_total_particles().  This implies that TotalP[p][s]
does not the sum of q(n)[p][s][m] for all m in the second and successive calls, because
q(n) = NOfPLocal may reflect particle injections and/or removals and thus may not
represent the layout in particle buffer. The copying from TotalPNext is also done
by the level-2 counterpart transbound2(), and the array is referred to in move_to_
sendbuf_primary() and move_to_sendbuf_secondary().

TotalPNext     • The element [p][s] of the integer array TotalPNext[2][S] has the value to be set
to TotalP[p][s] at the end of transbound1() or transbound2().  The values of
this array are calculated by try_primary1() or move_to_sendbuf_primary() if the
next mode is primary, or by make_comm_count(), make_recv_count(), count_next_
particles(), and/or move_to_sendbuf_secondary() otherwise. This array is shown


<!-- Page 138 -->

to the simulator body, which gives the (double) pointer to it through totalp argu-
ment of oh1_init(), or the pointer to NULL to alllocate it by init1(), and thus
works as the shadow of TotalP.

primaryParts     • The integer variable primaryParts is calculated by set_total_particles() on the
first call of oh1_transbound() or an explicit call of oh2_set_total_particles()
∑S−1
to have Qnn =   s=0 TotalP[0][s],  i.e., the number of primary particles which the
local node n initially accommodates. After that, the variable is calculated by try_
primary2() or move_to_sendbuf_secondary() to show the size/base of priamry/
secondary particle buffer in the next step, so that it can be referred to by move_to_
sendbuf_secondary() itself and move_to_sendbuf_primary().

totalParts     • The integer variable totalParts is made equal to primaryParts by set_total_
particles() in the first call of transbound1() or an explicit call of oh2_set_total_
particles() to show the number of (primary) particles which the local node initially
accommodates.  After that, it is calculated at the end of transbound2() to show              ∑   ∑S−1
Qn =    p∈0,1   s=0 TotalP[p][s],  i.e., the total number of particles which the lo-
cal node currently accommodates, to the functions move_to_sendbuf_secondary(),
move_injected_to_sendbuf() and oh2_inject_particle() which need to know the
bottom of the particle buffer in the next simulation step.

NOfRecv     • The element [p][s][m] of the integer array NOfRecv[2][S][N] is calculated by try_
RecvCounts       primary1() if we will be in primary mode in the next step, or make_comm_count() and
make_recv_count() otherwise, to have the number of primary (p = 0) or secondary
(p = 1) particles of species s which the local node should received from the node m.
The shadow of this array RecvCounts replicated by transbound1() is an API for the
simulator body to notify it how particles are received, and thus the (double) pointer
to the array, or a pointer to NULL to allocate its body by init1() as well as NOfRecv,
is given through the argument rcounts of oh1_init(). We need the shadow because
NOfRecv is referred to by stats_secondary_comm().

NOfSend     • The element [p][s][m] of the integer array NOfSend[2][S][N] is calculated by try_
SendCounts       primary1() if we will be in primary mode in the next step, or make_comm_count(),
make_recv_count() and make_send_count() otherwise, to have the number of parti-
cles of species s which the local node should send to the node m as its primary (p = 0)
or secondary (p = 1) particles. The shadow of this array SendCounts replicated by
transbound1() is an API for the simulator body to notify it how particles are sent,
and thus the (double) pointer to the array, or a pointer to NULL to allocate its body by
init1() as well as NOfSend, is given through the argument scounts of oh1_init().
We need the shadow because NOfSend is referred to by stats_secondary_comm().

InjectedParticles     • The  element  of  [0][p][s]  of  the  integer  array  InjectedParticles[2][2][S]  has
qinj(n)[p][s] being the number of particles of species s injected by the local node n into
its primary (p = 0) or secondary (p = 1) subdomain using oh2_inject_particle()
or its higher level counterparts.  That  is, after allocated by init1() and cleared
by it and transbound2() at its end, each element is incremented by oh2_inject_
particle() if the injected particle is in the primary/secondary subdomain to have
the number of them at the call of transbound2(). Then the elements in [0][p][s] are at
first referred by count_stay() so that injected particles are excluded from the stay-
ing primary/secondary particle count NOfPToStay[] so that they are considered to be
floating. After that, the elements in [0][p][s] are referred to by set_sendbuf_disps()


<!-- Page 139 -->

to keep the space for injected particles in SendBuf[], and,  if we will be in primary
mode in the next step, by move_injected_from_sendbuf() to move back injected
primary particles into Particles[] from SendBuf[] into which move_injected_to_
sendbuf() moved them. On the other hand if the next mode is secondary, the sec-
ond half elements [1][p][s] are set to the number of primary/secondary particles which
are injected and stay in the local node by move_to_sendbuf_secondary(), and are
referred to by move_injected_from_sendbuf() for moving back.

TempArray     • The integer array TempArray[N] is used for a temporay store in the functions count_
stay(), assign_particles(), schedule_particle_exchange(),  sched_comm(),
rebalance1(), try_primary2() and exchange_particles(). The array is allocated
by init1() which also uses it, or by its level-4p counterpart due to the necessity of
a larger store.

T_Histogram     • The MPI_Datatype variable T Histogram has the MPI data-type for a slice [∗][∗][m]
in a array of [2][S][N] for MPI_Alltoall() communications to exchange histograms
of particle amounts. The value of this variable is created by MPI_Type_vector()
and MPI_Type_struct() called in init1(), and  is used for MPI_Alltoall() in
transbound1() and make_comm_count().


/* Number of particles and related variables */
EXTERN int  nOfSpecies;
EXTERN int  maxFraction;
EXTERN int  *NOfPLocal;                 /* [2][nOfSpecies][nOfNodes] */
EXTERN int  *NOfPrimaries;              /* [2][nOfSpecies][nOfNodes] */
EXTERN dint *TotalPGlobal;              /* [nOfNodes+1] */
EXTERN dint nOfParticles;
EXTERN int  nOfLocalPMax;
EXTERN dint *NOfPToStay;                /* [nOfNodes] */
EXTERN int  *TotalP;                    /* [2][nOfSpecies] */
EXTERN int  *TotalPNext;                /* [2][nOfSpecies] */
EXTERN int  primaryParts, totalParts;
EXTERN int  *NOfRecv, *RecvCounts;      /* [2][nOfSpecies][nOfNodes] */
EXTERN int  *NOfSend, *SendCounts;      /* [2][nOfSpecies][nOfNodes] */
EXTERN int  *InjectedParticles;         /* [2][2][nOfSpecies] */
EXTERN int  *TempArray;                 /* [nOfNodes] */
EXTERN MPI_Datatype T_Histogram;



#### 4.2.5 Node Descriptors

S_node  The next variable group is for S node type data structures to keep various information
of each node (MPI process) for load balancing. The S_node structure has the following
elements.

- stay.prime is set to the number of primary particles accommodated by the node,
Qnn for the node n, by count_stay().

- stay.sec is set to the number of secondary particles accommodated by the node,
Qparent(n)n        for the node n, by count_stay().


<!-- Page 140 -->

- get.prime has the maximum number of primary particles that the helpers of the
node can accommodate further, or 0 if the node must get all the primary particles
other than those can stay in the helpers, before the node is visited by the bottom-up
traversal of the family tree in try_stable1(). That is, it has the following Pnput for
a node n.

 Pm                      H(m) = ∅
P mmin =     ∑          min
c   ))  H(m) ̸= ∅             max(0, Pm −   (Pmax −P
c∈H(m)
Qgetm = Pmax −(P mmin + Qparent(m)m        )
∑
Pnput =    max(0, Qgetm )
m∈H(n)

Then, when the node is visited, it is set to the minimum number of primary particles
the node must get from other nodes if positive, or the reversed maximum number of
the node can put out to its helpers if negative. That is, it is set to the folllowing Pnget
for a node n.
∑
Qstayn  = Qnn +   min(Qnm, Pmax −P mmin )
m∈H(n)
Pnget = max(Pn −Qstayn −Pnput , −Qnn)

Finally, when the node is visited again by the top-down traversal in try_stable1(),
it is set to the exact number of the primary particles the node gets from others if
positive, or the reversed one the node puts to its helpers otherwise.

- get.sec is set to the reversed number of secondary particles the node must put out in
order to accommodate its primaries, or 0 otherwise, by the bottom-up traversal of the
family tree in try_stable1(). That is, it is set to min(0, Qgetn  ) for a node n. Then,
when the helpand of the node is visited in the top-down traversal in try_stable1(),
it is set to the exact number to get from other nodes if it was 0 meaning the node
can accommodate some secondary particles further.

- comm.prime is set to the index of the MPI communicator array Comms.body[N] for
the family rooted by the node if it has helpers, or −1 otherwise, by rebalance1().

- comm.sec is set to the index of the MPI communicator array Comms.body[N] for the
family rooted by the node if it has the helpand, or −1 otherwise, by rebalance1().

- comm.black is set to 0 if the family rooted by the node is in red group, or 1 otherwise
(i.e., black group), by rebalance1().

- comm.rank is set to the MPI rank of the node in the communicator for the family
rooted by the node by rebalance1().

- parent is set to the pointer to the S_node structure for the helpand of the node, or
NULL if the node is the root of the family-tree, by rebalance1().

- sibling is set to the pointer to the S_node structure for the sibling in the family to
which the node belongs as a helper, or NULL if the node is the last helper or is the
root of the family tree, by rebalance1().


<!-- Page 141 -->

- child is set to the pointer to the S_node structure for the first helper of the family
rooted by the node, or NULL if the node is a leaf of the family tree, by rebalance1().

- id is set to the MPI rank of the node by init1().

- parentid is set to the MPI rank of the helpand of the node, or −1 if the node is the
root of the family tree, by rebalance1().

Then we have three array variables of S_node and its pointer types.

Nodes     • The element [n] of the array Nodes[N] has the S_node structure whose MPI rank is
n. That is, init1() makes Nodes[n].id = n for all n when it allocates the array.
This array is referred to by many functions.

NodesNext     • The array NodesNext[N] temporaily has Nodes[] for the next simulation step when
rebalancing is performed. This array, allocated by init1() and copied from Nodes[]
by rebalance1(),  is necessary because schedule_particle_exchange(), sched_
comm() and rebalance2() access both family tree for the current (before rebalancing)
and next (after rebalancing) steps. That is, the former is accessed to find neighboring
and own family members who may accommodate particles residing the subdomain
in question, while the latter is accessed to find new family menbers to whom those
particles are distributed.

NodeQueue     • The array NodeQueue[N] has the pointers to all Nodes[] elements in the order of a
bottom-up traversal of the family tree. It is assured that for 0 ≤∀i < ∀j < N, the
node pointed by the element [i] is not an ancestor of the node pointed by [j]. That
is, the helpand of the node pointed by [i] is pointed from some [j] such that i < j.
The array is allocated by init1(), while its elements are set by rebalance1() and
referred to by try_stable1().


/* Computation node descriptors */
struct S_node {
struct {int prime, sec;} stay;
struct {dint prime, sec;} get;
struct {int prime, sec, black, rank;} comm;
struct S_node *parent, *sibling, *child;
int id, parentid;
};
EXTERN struct S_node *Nodes, *NodesNext, **NodeQueue;



#### 4.2.6 Heap Structures for Rebalancing

S_heap  The next variable group is for S heap type data structures to keep subdomain ID’s in as-
cending and descending order of particle populations in subdomains. The S heap structure
has the following elements.

- n has the number of elements registered in a heap.

- node[N+1] has subdomain ID’s. The element [1] is the root and has the ID of sub-
domain whose number of particles is minimum (maximum).  For other element [i]
(i > 1), it is assured that the number of particles in the corresponding subdomain is
not less than (not greater than) that in the subdomain whose ID is registered in the


<!-- Page 142 -->

parent element [⌊i/2⌋]. Note that the element [0] is never referred to and thus this
element is not allocated.

- index[N] has indices of node[]. The element [n] has the index of node[] at which
the subdomain ID n is registered, or 0 if the subdomain in not registered in the heap.
That is, node[index[n]] = n if index[n] ̸= 0.

Then we have the following two S_heap variables, which are allocated by init1() and
manipulated by rebalance1() directly or through push_heap(), remove_heap() and pop_
heap().

LessHeap     • The variable LessHeap is the S_heap structure to keep ID’s of subdomains whose
particle populations are less than average in ascending (minimum first) order.

GreaterHeap     • The variable GreaterHeap is the S_heap structure to keep ID’s of subdomains whose
particle populations are not less than average in descending (maximum first) order.


/* Heap structure for load rebalancing */
struct S_heap {
int n, *node, *index;
};
EXTERN struct S_heap LessHeap, GreaterHeap;



#### 4.2.7 Variables for Particle Transfer Scheduling

S_commlist  The next variable group is for an array of S commlist type structure named CommList
and variables related to the array. Each elelemnt of the array represents a secondary-mode
particle transfer which the local node or its family members have to perform. An array
element of S commlist type has the following integer elements whose values are set by
sched_comm().

- sid is the ID (MPI rank) of the node from which particles are transferred.

- rid is the ID (MPI rank) of the node to which particles are transferred.

- region is the ID of the subdomain in which the transferred particles reside.

- count is the number of particles to be transferred.

- tag has the number pS + s to indicate the species s of the transferred particles and
whether they are primary (p = 0) or secondary (p = 1) ones for the receiver. By
using this element for the tag of MPI point-to-point communication, the receiver can
recognize where the received particles should be placed in its particle store. Moreover,
the tag can be used for the one-dimensional index of a (conceptually) two dimensional
array of [2][S].

Then we have the following variables, CommList array, arrays having indices of CommList,
a pointer and size variable for a subarray of CommList, and an MPI data-type to transfer
S_commlist data.

CommList     • The array CommList[2 · 3D(NS + 1) + N(S + 3)] of S_commlist type is conceptually
divided into following five blocks.


<!-- Page 143 -->

primary receiving block is built by each node for particles in its primary subdomain
to be received by the node itself or its helpers. Its size is at most N +NS because
the block corresponds to a shortest path in a conceptual two-dimensional array
of |F(n)| × #s(n) · S, where F(n) = H(n) ∪{n} is the set of family members for
the subdomain n and #s(n) is the number of sender nodes which has particles
in n, from its south-west corner to north-east corner and |F(n)| and #s(n) are
at most (but may be) N.

primary sending block is exchanged by neighboring node (subdomain) pairs. A
node receives a part of the primary receiving block from each neighbor for par-
ticles sent from the family members rooted by the node to the family members
rooted by the neighbor. The size of the block B(n) for a node n is given by;
∑       ∑
B(n) ≤    (|F(m)| + |F(n)|S) ≤    |F(m)| + (3D −1)|F(n)|S

m∈nbor(n)                m∈nbor(n)

where nbor(n) is the set of nodes neighboring to n and thus |nbor(n)| ≤3D −1.
Since we have;
∑
|F(m)| + |F(n)| ≤3D + N     |F(n)| ≤N

m∈nbor(n)

because the sets of helpers of n and its neighbors are exclusive each other, we
can bound B(n) as follows.
∑
B(n) ≤    |F(m)| + (3D −1)|F(n)|S

m∈nbor(n)
≤3D + N −|F(n)| + (3D −1)|F(n)|S
≤3D + (3D −1)NS

secondary receiving block for a node is the copy of primary receiving block of
its helpand which broadcasts the block to its helpers to show them particle
receptions for its primary subdomain and thus helper’s secondary subdmain.
Therefore, the size of this block is at most N + NS.

secondary sending block for a node is the copy of primary sending block of its
helpand which broadcasts the block to its helpers to show them particle trans-
missions for its primary subdomain and thus helper’s secondary subdomain.
Therefore, the size of this block is at most 3D + (3D −1)NS.

alternative secondary receiving block for a node is the copy of primary receiving
block of its helpand which broadcasts the block to its helpers which become
its family members by rebalancing. A node must refer to both of secondary
receiving blocks gotten from its old and new helpand because the former may
have particle transmissions for its old secondary subdomain. The size of this
block is at most N + NS.

The  array  is  allocated by  init1(),  while  its  elements  are  set by  sched_
comm().   The  whole  or  a  part  of  the  array  or  its  elements  are  referred
to by schedule_particle_exchange(), make_comm_count(), make_recv_count(),
make_send_count(), count_next_particles(), exchange_particles(), receive_
particles() and send_particles().


<!-- Page 144 -->

SecRList     • The S_commlist type pointer SecRList points the head of the block of CommList
SecRLSize         for the secondary particle transfers, and integer variable SecRLSize has its size. The
block is either of the secondary receiving block if we continue secondary mode without
rebalancing, or the alternative secondary receiving block if rebalanced. The variables
are set by make_comm_count() and referred to by rebalance2().

RLIndex     • The element k of the integer array RLIndex[3D+1] has the index of CommList from
which particle receptions from k-th neighbor are recorded. Therefore, we send records
from CommList[RLIndex[k]] to CommList[RLIndex[k+1]−1] to k-th neighbor as a part
of its primary sending block.  This array is manipulated by schedule_particle_
exchange() and sched_comm().

SLHeadTail     • The first element ([0]) of the integer array SLHeadTail[2] has the head index of the
primary sending block of CommList, while its second element ([1]) has the tail index
plus one, or the head of the secondary receiving block. The elements of this array are
set by schedule_particle_exchange() and are referred to by make_comm_count()
and try_stable2().

SecSLHeadTail     • The first element ([0]) of the integer array SecSLHeadTail[2] has the head index of
the secondary sending block of CommList, while its second element ([1]) has the tail
index plus one, or the head of the alternative secondary receiving block. Both indices
are displacements from the head of secondary receiving block. The elements of this
array are set by schedule_particle_exchange() and are referred to by make_comm_
count() and try_stable2().

T_Commlist     • The MPI_Datatype type variable T Commlist has the MPI data-type for a S_commlist
type record, a contiguous data-type whose size is sizeof(struct S_commlist) in
bytes  . The value of this variable is created by MPI_Type_contiguous() called in
init1(), and used for MPI communications in schedule_particle_exchange() and
make_comm_count().

S_commsched_context  In addition, we have another struct for particle transfer named S commsched context
to keep the execution context of the function sched_comm() with the following elements,
which are intialized by the caller schedule_particle_exchange() and are updated and
referred to by sched_comm().

- neighbor is the neighboring index of the node which sched_comm() is visiting as the
root of a sender family.

- sender is the ID of the node which sched_comm() is examining its particles to be
sent to the local node’s family.

- spec is the particle species which sched_comm() is examining for sending particles
from sender to the local node’s family.

- dones is the number of particles of the species spec, which sched_comm() has already
processed for sending from sender to the local node’s family.

- donen is the number of particles which sched_comm() has already processed for send-
ing from sender belonging to the local node’s family.


/* Structured variables for particle transfer */
struct S_commlist {


<!-- Page 145 -->

int sid, rid, region, count, tag;     /* tag = spec + nOfSpecies*sec */
};
EXTERN struct S_commlist *CommList, *SecRList;
EXTERN int RLIndex[OH_NEIGHBORS+1];
EXTERN int SLHeadTail[2], SecSLHeadTail[2], SecRLSize;
EXTERN MPI_Datatype T_Commlist;
struct S_commsched_context {
int neighbor, sender, spec, comidx, dones, donen;
};



#### 4.2.8 Variables for Family Communicators

The next variable group is for the MPI communicators of helpand-helper families.

GroupWorld     • The MPI_Group type variable GroupWorld has the group structure of MPI-processes
belonging to MPI_COMM_WORLD (or whatever MCW refers to). It is initialized by init1()
and is referred to by rebalance1() to extract processes to build communicators of
families.

Comms     • The struct variable Comms has the following elements to store family communicators.

  - n has the number of family communicators, i.e., the number of non-leaf nodes in
the family tree.

  - body[N] has family communicators. More specifically its element  [i] has the
communicator of the family rooted by the node in NodeQueue[N−i−1].

The elements above are initialized by init1(), and are updated and referred to by
rebalance1().

MyComm     • The variable MyComm is the pointer to a struct named S mycommc to have information
MyCommC         of the family communicators for the local node with the following elements.
S_mycommc
  - prime has the communicator of the family which the local node belongs to as
the helpand, or MPI_COMM_NULL if it is a leaf.

  - sec has the communicator of the family which the local node belongs to as a
helper, or MPI_COMM_NULL if it is the root.

  - rank is the MPI rank of the local node in the communicator prime, or −1 if it
is a leaf.

  - root is the MPI rank of the local node’s helpand in the communicator sec, or
−1 if the local node is the root.

  - black indicates whether the communicator prime is in red group (0) or black
group (1). The red (black) color is given to families rooted by nodes which
belong to black (red) families as helpers. The coloring is necessary to perform
collective communications in families without serialization. That is, we perform
collective communications in black families at first in parallel, and then do in
red families also in parellel.

The structure is allocated by init1() and its elements are set by rebalance1(). The
elements are referred to by oh1_broadcast(), oh1_all_reduce() and oh1_reduce().


<!-- Page 146 -->

A C-coded simulator body may allocate a S_mycommc structure and give the pointer to
the structure through the argument mycomm of oh1_init() which set it into MyCommC,
or simply passes NULL through mycomm to show the unawareness of the structure. In
the former case, rebalance1() copies the updated values of MyComm into MyCommC
to make it the shadow. To make it possible for the simulator body to access the
S_mycommc structure, the C header file ohhelp c.h has the declaration of S mycommc
same as that in ohhelp1.h at its very beginning but just following;

#include <mpi.h>

to obtain the type declaration of MPI_Comm.

MyCommF     • The variable MyCommF is the pointer to a struct named S mycommf being the Fortran
S_mycommf        counterpart of MyComm. The S_mycommf structure has elements same as S_mycommc but
its prime and sec are not MPI_Comm type but integers. A Fortran-coded simulator
body should allocate a S_mycommf structure, or more accurately, oh_mycomm type
defined in oh type.F90 and shown in §3.4.1, and give the pointer to it through the
argument mycomm of oh1_init_(). The values of the structure are copied from MyComm
with C-to-Fortran translation by rebalance1().


/* Structured variables for MPI communicator */
EXTERN MPI_Group GroupWorld;
EXTERN struct {
int n;
MPI_Comm *body;       /* [nOfNodes] */
} Comms;
struct S_mycommc {
MPI_Comm prime, sec;
int rank, root, black;
};
EXTERN struct S_mycommc *MyComm, *MyCommC;
EXTERN struct S_mycommf {
int prime, sec;
int rank, root, black;
} *MyCommF;


In addition to the global variables shown above, the C source file ohhelp1.c has two
global but private arrays of integers FamIndex[N + 1] and FamMembers[2N −1] being the
arguments of oh1_families() to report the configurations of all families to the simulator
body, as discussed in §4.3.8.


#### 4.2.9 Variables for Neighboring Information

DstNeighbors  Next, we declare the integer arrays DstNeighbors[3D] and SrcNeighbors[3D] whose ele-
SrcNeighbors  ment [k] has the k-th neighbor of the local node. More specifically, let k be as follows,
Neighbors
D−1∑
k =     νd3d  (νd ∈{0, 1, 2})

d=0

and let (π0, . . . , πD−1) be the coordinate of the local node in the conceptual D-dimensional
coordinate system in which MPI processes (or equivalently their primary subdomains) are


<!-- Page 147 -->

laid out. The element DstNeighbors[k] basically has the MPI rank r of the process at
(π0+ν0−1, . . . , πD−1+νD−1−1), but must have −(r + 1)  if there  is k′ < k such that
DstNeighbors[k′] = r. Note that the local node itself is in the element k = 1+3+· · ·+3D−1
because νd = 1 for all d ∈[0, D−1]. Similarly, SrcNeighbors[3D−1−k] has r or −(r + 1)
for k defined above with k′ > k. For both arrays, a special identifier −(N + 1) means that
no MPI process is at the corresponding neighboring location. Therefore, when we perform
a neighboring communication along the direction defined by k without multiple sending/
receiving to/from an existing neighboring process, we send a data to DstNeighbors[k] if
non-negative and receive a data from SrcNeighbors[k] if non-negative.
The contents of the arrays are initialized by init1() based on the neighboring informa-
tion given by the simulator body through the argument nbor[3D] of oh1_init(), or on the
process grid size given by the argument pcoord[D]. In the latter case, init1() initialzes
the array nbor simply with (π0+ν0−1, . . . , πD−1+νD−1−1) assuming that πd ∈[0, Πd−1]
where Πd = pcoord[d] and the rank r of the process at (π0, . . . , πD−1) is defined as follows.

rD−1 = πD−1     rd = rd+1Πd + πd     r = r0

The elements of arrays are referred to by schedule_particle_exchange(), sched_comm(),
try_primary2() and init3().
The array DstNeighbors[3D] is the element [0] of the array Neighbors[3][3D] in real-
ity.  Its elements [1] and [2] are DstNeighbors for the helpand of the local node which
broadcasts one of them in build_new_comm(). More specifically, the element [2] is used
transitionally when we need both sets of neighbors of the helpands before ([1]) and after
([2]) rebalancing for position-aware particle management etc., while [1] is for non-transional
use in transbound1() and oh3_map_particle_to_neighbor(), which also refer to [0].


/* Neighboring information */
EXTERN int Neighbors[3][OH_NEIGHBORS], SrcNeighbors[OH_NEIGHBORS];
/* <BSW,BS,BSE,BW,B,BE,BNW,BN,BNE,    : 00..04..08
SW, S, SE, W,O  E, NW, N, NE,    : 09..13..17
TSW,TS,TSE,TW,T,TE,TNW,TN,TNE>    : 18..22..26 */
EXTERN int *DstNeighbors;


In addition to the global variables shown above, the C source file ohhelp1.c has two global
but private arrays of integers NeighborsShadow[3][3D] and NeighborsTemp[3D] being the
argument nbor of oh1_neighbors() and init1() respectively to report the neighbors of
the local node’s primary/secondary subdomains to the simulator body, as discussed in
§4.3.3.


#### 4.2.10 Variables for Statistics and Verbose Messaging

The next variable group is for statistics reporting and verbose messaging. Before explaning
the group, we revisit the definition in the header file oh stats.h explained in §3.10.1. The file
#define’s integer keys to identify execution intervals whose execution times are measured.
We name each key as STATS key where key is a sequence of uppercase letters and underscores
unique to the key but should not start with PART . Each key must be #define’d as a unique
integer in the range [0, Kt−1] where Kt should be the definition of the special key STATS_
TIMINGS.
The prototype of oh stats.h defines the following keys.

STATS_TRANSBOUND for the interval from the beginning of transbound1().


<!-- Page 148 -->

STATS_TRY_STABLE for the interval from the beginning of try_stable1().

STATS_REBALANCE for the interval from the beginning of rebalance1().

STATS_REB_COMM for the interval from the beginning of the family communicator creation
in rebalance1().

STATS_TB_MOVE for the particle packing in the particle store and move those to be sent to
the send buffer in move_to_sendbuf_primary() or move_to_sendbuf_secondary().

STATS_TB_COMM for particle transfer in try_primary2() or exchange_particles().

The header file oh stats.h also has the declaration and initialization of the array of
character strings StatsTimeStrings[2Kt] whose elements [2k] and [2k+1] are strings to be
printed with the timing statistics of the execution intervals for the simulation of primary and
secondary particles and/or subdomains identified by the key numbered k. The declaration
is surrounded by C’s macro construct #ifdef␣OH_DEFINE_STATS and #endif so that only
ohhelp1.c includes this declaration. The array is referred to by print_stats().
Now we come back to ohhelp1.h and #define following keys for the statistics of particle
transfers.

STATS_PART_MOVE_PRI_MIN     • The keys STATS PART MOVE x y where x ∈{PRI, SEC} and y ∈{MIN, MAX, AVE} are
STATS_PART_MOVE_PRI_MAX         for MINnimum, MAXimum and AVErage numbers of PRImary and SECondary particles
STATS_PART_MOVE_PRI_AVE       between a sender/receiver pair.
STATS_PART_MOVE_SEC_MIN
STATS_PART_MOVE_SEC_MAX
STATS_PART_MOVE_SEC_AVE

STATS_PART_GET_PRI_MIN     • The keys STATS PART GET x y where x ∈{PRI, SEC} and y ∈{MIN, MAX} are for
STATS_PART_GET_PRI_MAX      MINimum and MAXimum numbers of PRImary and SECondary particles received by a
STATS_PART_GET_SEC_MIN       node.
STATS_PART_GET_SEC_MAX

STATS_PART_PUT_PRI_MIN     • The keys STATS PART PUT x y where x ∈{PRI, SEC} and y ∈{MIN, MAX} are for
STATS_PART_PUT_PRI_MAX      MINimum and MAXimum numbers of PRImary and SECondary particles sent by a node.
STATS_PART_PUT_SEC_MIN
STATS_PART_PUT_SEC_MAX

STATS_PART_PG_PRI_AVE     • The keys STAS PART PG x AVE where x ∈{PRI, SEC} are for average numbers of
STATS_PART_PG_SEC_AVE      PRImary and SECondary particles received (or sent equivalently) by a node.

STATS_PART_PRIMARY     • The key STATS PART PRIMARY is for the number of transitions to (or staying at) pri-
mary mode.

STATS_PART_SECONDARY     • The key STATS PART SECONDARY is for the number of transitions to (or staying at)
secondary mode.

STATS_PARTS     • The macro constant STATS PARTS is #define’d to be the number of keys of particle
transfer statistics, Kp.

StatsPartStrings  Then we declare and initialize the array of character strings StatsPartStrings[Kp] whose
elements [k] is the string to be printed with the particle transfer statistics identified by
the key numbered k. The declaration is surrounded by C’s macro construct #ifdef␣OH_
DEFINE_STATS and #endif so that only ohhelp1.c includes this declaration. The array is
referred to by print_stats().


<!-- Page 149 -->

/* Structures and variables for statistics and verbose messaging */
#define STATS_PART_MOVE_PRI_MIN 0
#define STATS_PART_MOVE_PRI_MAX 1
#define STATS_PART_MOVE_PRI_AVE 2
#define STATS_PART_GET_PRI_MIN  3
#define STATS_PART_GET_PRI_MAX  4
#define STATS_PART_PUT_PRI_MIN  5
#define STATS_PART_PUT_PRI_MAX  6
#define STATS_PART_PG_PRI_AVE   7
#define STATS_PART_MOVE_SEC_MIN 8
#define STATS_PART_MOVE_SEC_MAX 9
#define STATS_PART_MOVE_SEC_AVE 10
#define STATS_PART_GET_SEC_MIN  11
#define STATS_PART_GET_SEC_MAX  12
#define STATS_PART_PUT_SEC_MIN  13
#define STATS_PART_PUT_SEC_MAX  14
#define STATS_PART_PG_SEC_AVE   15
#define STATS_PART_PRIMARY      16
#define STATS_PART_SECONDARY    17
#define STATS_PARTS             (STATS_PART_SECONDARY+1)

#ifdef OH_DEFINE_STATS
static char *StatsPartStrings[STATS_PARTS] = {
"p2p transfer[pri,min]",
"p2p transfer[pri,max]",
"p2p transfer[pri,ave]",
"get[pri,min]",
"get[pri,max]",
"put[pri,min]",
"put[pri,max]",
"put&get[pri,ave]",
"p2p transfer[sec,min]",
"p2p transfer[sec,max]",
"p2p transfer[sec,ave]",
"get[sec,min]",
"get[sec,max]",
"put[sec,min]",
"put[sec,max]",
"put&get[sec,ave]",
"transition to pri",
"transition to sec",
};
#endif


The next part is the sequence of the following struct declarations.

S_statscurr     • The structure S statscurr is for the statistics of the currently executed simulation
step and has the following elements.

  - time.value has the double-float wall-clock time at the call of oh1_stats_time()
which starts an interval to be measured.
  - time.val[2Kt+2] is a double-float array whose element [2k+p] has the time
spent in the interval identified by k for the simulation of primary (p = 0) or


<!-- Page 150 -->

secondary (p = 1) particles and/or subdomains. The elements [2Kt] and [2Kt+1]
are special entries to eliminate the time spent from the statistics processing.

  - time.key has the identifier of the interval currently measured.
  - time.ev[2Kt+2] is an integer array whose element [2k+p] is 1 if and only if the
interval identified by k for the simulation of primary (p = 0) or secondary (p = 1)
particles and/or subdomains is executed.

  - part[Kp] is a 64-bit integer array whose element [k] has the statistics count of
the particle transfer identified by k.

The elements belonging to time are cleared by oh1_init_stats() and updated
by oh1_stats_time() while the elements of part are set and modified by stats_
primary_comm(), stats_secondary_comm() and stats_comm(). Then they are re-
ferred to and partly reinitialized by update_stats().

S_statstime     • The structure S statstime is for the timing statistics for each key and has following
elements.

  - min has the double-float hitherto-minimum measured time.
  - max has the double-float hitherto-maximum measured time.
  - total has the double-float total of measured times.
  - ev has the number of executions of the measured intervals.

S_statspart     • The structure S statspart is for the particle transfer statistics for each key and has
following elements.

  - min has the 64-bit integer hitherto-minimum measured count.
  - max has the 64-bit integer hitherto-maximum measured count.
  - total has the 64-bit integer total of measured counts.

S_statstotal     • The structure S statstotal has the following arrays of structures S_statstime and
S_statspart structures for statistics keys.

  - time[2Kt] is the array of S_statstime structures to keep timing statistics of
the interval identified by k for the simulation of primary (p = 0) or secondary
(p = 1) particles and/or subdomains in its element [2k+p].

  - part[Kp] is the array of S_statspart structures to keep particle transfer statis-
tics identified by k in its element [k].

The elements are initialized by clear_stats() and updated by update_stats(),
while they are referred to by print_stats().

Stats     • The variable Stats of S stats structure has the following elements to keep measured
S_stats         statistics.

  - curr is a S_statscurr structure to keep the statistics measured in the most
recent simulation time-step.

  - subtotal is a S_statstotal structure to keep the hitherto total statistics in the
current time-steps for subtotal measurement.

  - total is a S_statstotal structure to keep the hitherto total statistics from the
beginning of the simulation.


<!-- Page 151 -->

struct S_statscurr {
struct {
double value, val[2*STATS_TIMINGS+2];
int key, ev[2*STATS_TIMINGS+2];
} time;
dint part[STATS_PARTS];
};
struct S_statstime {
double min, max, total;
int ev;
};
struct S_statspart {
dint min, max, total;
};
struct S_statstotal {
struct S_statstime time[2*STATS_TIMINGS];
struct S_statspart part[STATS_PARTS];
};
EXTERN struct S_stats {
struct S_statscurr curr;
struct S_statstotal subtotal, total;
} Stats;


Finally, we declare a few variables related to staticstics and verbose messaging as follows.

T_StatsTime     • The MPI_Datatype variable T StatsTime is for the MPI communication to reduce S_
statstime statistics data. The value of this variable is created by oh1_init_stats()
and used for MPI_Reduce() called in print_stats().

Op_StatsTime     • The MPI_Op variables Op StatsTime and Op StatsPart are for the MPI communica-
Op_StatsPart        tions to reduce S_statstime and S_statspart statistics data by the functions stats_
reduce_time() and stats_reduce_part() respectively. The variables are initialized
by oh1_init_stats() and used for MPI_Reduce() called in update_stats() and
print_stats().

statsMode     • If and only if the variable statsMode has a non-zero value (1 or 2, in a usual sense)
statistics data are measured and reported. If it is 2, the reporting is repeated every r
simulation steps where r = reportIteration, while the report is made only at the end
of simulation otherwise. The simulator body must gives its value through the argu-
ment stats of oh1_init() so that init1() copies it into statsMode. The variable is
referred to in transbound1(), oh1_init_stats[_](), oh1_stats_time[_](), oh1_
show_stats[_](), update_stats(), oh1_print_stats[_]() and transbound2().

reportIteration     • The variable reportIteration specifies the number of simulation steps at the every
end of which the statistics are reported if statsMode = 2. The simulator body must
gives its value through the argument repiter of oh1_init() so that init1() copies
it into reportIteration. The variable is referred to in oh1_show_stats().

verboseMode     • The variable verboseMode specifies the level of verbose execution as follows.

  - 0 means to execute silently.

  - 1 means to execute reasonably verbosely.


<!-- Page 152 -->

  - 2 means to execute very verbosely.

  - 3 (or larger) means to execute with very verbose messages from all processes.

The simulator body must gives the value of verboseMode through the argument
verbose of oh1_init() so that init1() copies it into verboseMode. The variable
is referred to in transbound1(), try_primary1(), try_stable1(), rebalance1(),
init1() and oh1_verbose[_]() through the functional macro Verbose(), and in
vprint() directly.


EXTERN MPI_Datatype T_StatsTime;
EXTERN MPI_Op Op_StatsTime, Op_StatsPart;
EXTERN int statsMode, reportIteration, verboseMode;



#### 4.2.11 Function Prototypes

The next block is to declare function prototypes.  First we declare the prototypes of the
API function pairs each of which consists of API for Fortran and C. An API for Fortran
has name ending with an underscore while its C counterpart is named by removing the tail
underscore. The API functions are listed below.

- The function oh1_init[_]() initializes data strucutures of the level-1 library.

- The function oh1_neighbors[_]() is to specify the array into which the neighborhood
information of the local node is given by the library.

- The function oh1_families[_]() is to specify the array pair into which the configu-
ration of all family is given by the library.

- The function oh1_transbound[_]() examines whether particles distribution among
nodes are balanced well and, if imbalanced, reconfigures helpand-helper relationships.
Then it notifies the simulator body how the local node should receive and send par-
ticles through RecvCounts and SendCounts.

- The function oh1_accom_mode[_]() is to show its caller whether the particle accom-
modation mode is normal or anywhere by its return value.

- The functions oh1_broadcast[_](), oh1_all_reduce[_]() and oh1_reduce[_]()
performs collective communications in the families which the local node belongs to as
the helpand and a helper.

- The function oh1_init_stats[_]() starts statistics measurement.

- The function oh1_stats_time[_]() declares the beginning of the interval whose ex-
ecution time is measured.

- The function oh1_show_stats[_]() notifies the library of the end of a simulation
step so as to let it update statistics with those measured in the step. It also reports
the statistics if the step is at the end of iterations defined by reportIteration and
statsMode = 2.

- The function oh1_print_stats[_]() notifies the library of the end of the simulation
so as to let it report the statistics.


<!-- Page 153 -->

- The function oh1_verbose[_]() prints given message if verboseMode is not zero.

Before showing the source code for the prototypes, we show the first part of the header
files ohhelp c.h for C-coded simulators and ohhelp f.h for Fortran-coded ones. At first these
files #include’s oh config.h to #define D = OH_DIMENSION and the constant OH_LIB_LEVEL
for the default library level.

#include "oh_config.h"


Then they #define the aliases of level-1 API functions which do not have higher level
counterparts.

#define oh_neighbors(A1) \
oh1_neighbors(A1)
#define oh_families(A1,A2) \
oh1_families(A1,A2)
#define oh_accom_mode() \
oh1_accom_mode()
#define oh_broadcast(A1,A2,A3,A4,A5,A6) \
oh1_broadcast(A1,A2,A3,A4,A5,A6)
#define oh_all_reduce(A1,A2,A3,A4,A5,A6,A7,A8) \
oh1_all_reduce(A1,A2,A3,A4,A5,A6,A7,A8)
#define oh_reduce(A1,A2,A3,A4,A5,A6,A7,A8) \
oh1_reduce(A1,A2,A3,A4,A5,A6,A7,A8)
#define oh_init_stats(A1,A2)    oh1_init_stats(A1,A2)
#define oh_stats_time(A1,A2)    oh1_stats_time(A1,A2)
#define oh_show_stats(A1,A2)    oh1_show_stats(A1,A2)
#define oh_print_stats(A1)      oh1_print_stats(A1)
#define oh_verbose(A1)          oh1_verbose(A1)

Then ohhelp c.h gives the prototypes of the functions above, which are also given in
ohhelp1.h.

void oh1_neighbors(int **nbor);
void oh1_families(int **famindex, int **members);
int  oh1_accom_mode();
void oh1_broadcast(void *pbuf, void *sbuf, int pcount, int scount,
MPI_Datatype ptype, MPI_Datatype stype);
void oh1_all_reduce(void *pbuf, void *sbuf, int pcount, int scount,
MPI_Datatype ptype, MPI_Datatype stype,
MPI_Op pop, MPI_Op sop);
void oh1_reduce(void *pbuf, void *sbuf, int pcount, int scount,
MPI_Datatype ptype, MPI_Datatype stype,
MPI_Op pop, MPI_Op sop);
void oh1_init_stats(int key, int ps);
void oh1_stats_time(int key, int ps);
void oh1_show_stats(int step, int currmode);
void oh1_print_stats(int nstep);
void oh1_verbose(char *message);


Then both headers #define the aliases level-1 specific API functions if OH_LIB_LEVEL
is 1.

#if OH_LIB_LEVEL==1


<!-- Page 154 -->

#define oh_init(A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13) \
oh1_init(A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13)
#define oh_transbound(A1,A2) oh1_transbound(A1,A2)

Finally, the prototypes of these functions are given in ohhelp c.h and ohhelp1.h.

void oh1_init(int **sdid, int nspec, int maxfrac, int **nphgram,
int **totalp, int **rcounts, int **scounts, void *mycomm,
int **nbor, int *pcoord, int stats, int repiter, int verbose);
int  oh1_transbound(int currmode, int stats);


Oh the other hand, the prototypes of Fortran API functions are solely given in ohhelp1.h,
while their Fortran versions are given in oh mod1.F90 as shown in §3.4.

void oh1_neighbors_(int *nbor);
void oh1_families_(int *famindex, int *members);
int  oh1_accom_mode_();
void oh1_broadcast_(void *pbuf, void *sbuf, int *pcount, int *scount,
int *ptype, int *stype);
void oh1_all_reduce_(void *pbuf, void *sbuf, int *pcount, int *scount,
int *ptype, int *stype, int *pop, int *sop);
void oh1_reduce_(void *pbuf, void *sbuf, int *pcount, int *scount,
int *ptype, int *stype, int *pop, int *sop);
void oh1_init_stats_(int *key, int *ps);
void oh1_stats_time_(int *key, int *ps);
void oh1_show_stats_(int *step, int *currmode);
void oh1_print_stats_(int *nstep);
void oh1_verbose_(char *message);
void oh1_init_(int *sdid, int *nspec, int *maxfrac, int *nphgram,
int *totalp, int *rcounts, int *scounts,
struct S_mycommf *mycomm, int *nbor, int *pcoord, int *stats,
int *repiter, int *verbose);
int  oh1_transbound_(int *currmode, int *stats);


Next we declare the prototypes of the following functions used in level-2 and level-3
libraries.

- The function init1() is the body of oh1_init().

- The function mem_alloc() allocates a memory space by malloc().

- The function mem_alloc_error() aborts the simulation due to the memory shortage
reporting its cause.

- The function errstop() aborts the simulation due to an error detected by all processes
reporting given error message.

- The function local_errstop() aborts the simulation due to an error detected by the
local process reporting given error message.

- The function set_total_particles()  is to initialize TotalP, primaryParts and
totalParts with NOfPLocal upon the first call of oh1_transbound() or an explict
call of oh2_set_total_particles().

- The function transbound1() is the body of oh1_transbound().


<!-- Page 155 -->

- The function try_primary1() is to examine whether particle distribution among
subdomains is balanced well and thus we can perform the simulation in primary
mode.

- The function try_stable1() is to examine whether particle distribution among nodes
is balanced well and thus we can keep the current helpand-helper configuration.

- The function rebalance1() is to (re)build the helpand-helper configuration to cope
with an unacceptable load imbalance.

- The function build_new_comm() is to build communicators for the helpand-helper
families built by rebalance1().

- The function vprint() prints a verbose message specified by its variable number of
arguments.

- The function dprint() prints a message for debugging. This function is not used in
the production version of the library.


/* Prototypes for the functions called from higher-level library code */
void  init1(int **sdid, int nspec, int maxfrac, int **nphgram,
int **totalp, int **rcounts, int **scounts,
struct S_mycommc *mycommc, struct S_mycommf *mycommf, int **nbor,
int *pcoord, int stats, int repiter, int verbose);
void* mem_alloc(int esize, int count, char* varname);
void  mem_alloc_error(char* varname, size_t size);
void  errstop(char* format, ...);
void  local_errstop(char* format, ...);
void  set_total_particles();
int   transbound1(int currmode, int stats, int level);
int   try_primary1(int currmode, int level, int stats);
int   try_stable1(int currmode, int level, int stats);
void  rebalance1(int currmode, int level, int stats);
void  build_new_comm(int currmode, int level, int nbridx, int stats);
void  vprint(char* format, ...);
void  dprint(char* format, ...);



#### 4.2.12 Macro Verbose()

Verbose()  The last block #define’s a macro named Verbose(L,VP) for verbose messaging. This macro
examines whether the given verbose level L conforms the level defined by verboseMode.
That is, if L ≥verboseMode, it prints a message using the expression given by VP which
should be a call of vprint(), after performing global barrier synchronization by MPI_
Barrier(). The printing is always done if the rank of the local node is 0, or verboseMode
is 3 (or larger).
This macro is used in init1(), transbound1(), try_primary1(), try_stable1(),
rebalance1() and oh1_verbose[_]().


/* Macro for verbose messaging. */
#define Verbose(L,VP) {\
if (verboseMode>=L) {\


<!-- Page 156 -->

MPI_Barrier(MCW);\
if (myRank==0 || verboseMode>=3) VP;\
}\
}
