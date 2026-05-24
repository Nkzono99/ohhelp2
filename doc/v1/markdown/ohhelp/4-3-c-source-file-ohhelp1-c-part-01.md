# 4.3 C Source File ohhelp1.c - Part 1

Source: `doc/v1/original/ohhelp.pdf`, pages 157-188.

<!-- Page 157 -->

## 4.3 C Source File ohhelp1.c

#### 4.3.1 Header File Inclusion

The first job done in ohhelp1.c is the inclusion of the header file ohhelp1.h.  Before the
inclusion, we #define the macro EXTERN as empty so as to provide variables declared in
ohhelp1.h with their homes, as discussed in §4.2.3.

OH_DEFINE_STATS We also #define the macro OH DEFINE STATS to have the private variable declarations of
StatsTimeStrings and StatsPartStrings as discussed in §4.2.10.


#define EXTERN
#define OH_DEFINE_STATS
#include "ohhelp1.h"



#### 4.3.2 Function Prototypes

The next and last job to do prior to function definitions is to declare the prototypes of the
following functions private for the level-1 library.

- The function count_stay() counts the number of primary and secondary particles
in each node.

- The function assign_particles() determines the number and destination of floating
particles for a family.

- The function compare_int() compares two integers given by qsort().

- The  function schedule_particle_exchange() determines  the  particle  transfer
schedule for the family rooted by the local node.

- The function count_real_stay() counts the number of primary/secondary particles
accommodated in the local node or its helper.

- The function sched_comm() determines the transfer schedule of the particle residing
in and visiting to the primary subdomain of the local node.

- The function make_comm_count() gives particle counts to TotalPNext, NOfRecv and
NOfSend.

- The function make_recv_count() counts particles for NOfRecv.

- The function make_send_count() counts particles for NOfSend.

- The function count_next_particles() counts particles for TotalPNext.

- The function push_heap() pushes an element to a heap structure.

- The function pop_heap() pops the top element from a heap structure.

- The function remove_heap() removes an element from a heap structure.

- The function clear_stats() clears statistics data in a S_statstotal structure.


<!-- Page 158 -->

- The function stats_primary_comm() calculates statistics data of the particle trans-
fers in primary mode.

- The function stats_secondary_comm() calculates statistics data of the particle trans-
fers in secondary mode.

- The function stats_comm() performs the particle transfer statistics calculation for
stats_primary_comm() and stats_secondary_comm()

- The function update_stats() update statistics data in a S_statstotal structure.

- The function stats_reduce_part() performs pairwise reduction for the particle
transfer statistics.

- The function print_stats() prints statitstics.

- The function stats_reduce_time() performs pairwise reduction for the timing statis-
tics.


/* Prototypes for private functions. */
static void  count_stay();
static dint  assign_particles(dint npr, dint npt, struct S_node *ch, int incgp,
int *nget);
static int   compare_int(const void* x, const void* y);
static void  schedule_particle_exchange(int reb);
static int   count_real_stay(int *np);
static void  sched_comm(int toget, int rid, int tag, int reb,
struct S_commsched_context *context);
static void  make_comm_count(int currmode, int level, int reb, int oldparent,
int stats);
static void  make_recv_count(struct S_commlist* rlist, int rlsize);
static void  make_send_count(struct S_commlist* slist, int slsize);
static void  count_next_particles(struct S_commlist* rlist, int rlsize);
static void  push_heap(int id, struct S_heap* heap, int greater);
static int   pop_heap(struct S_heap* heap, int greater);
static void  remove_heap(struct S_heap* heap, int greater, int rem);
static void  clear_stats(struct S_statstotal *stotal);
static void  stats_primary_comm(int currmode);
static void  stats_secondary_comm(int currmode, int reb);
static void  stats_comm(int* nrecv, int* nsend, dint* scp, int ns);
static void  update_stats(struct S_statstotal *stotal, int step, int currmode);
static void  stats_reduce_part(void* inarg, void* ioarg, int* len,
MPI_Datatype* type);
static void  print_stats(struct S_statstotal *stotal, int cstep, int n);
static void  stats_reduce_time(void* inarg, void* ioarg, int* len,
MPI_Datatype* type);



#### 4.3.3 oh1_init() and init1()

oh1_init_()  The API functions oh1 init () for Fortran and oh1 init() for C receive a set of ar-
oh1_init()  ray/structure variables through which level-1 library functions communicate with the sim-
ulator body, and a few integer parameters to specify the behavior of the library.  The
functions have the following arguments.


<!-- Page 159 -->

- The argument sdid is the (double) pointer to a two-element integer array, which is
referred to as SubdomainId[2] in the library funcions as the shadow of RegionId[2].
The array has the subdomain identifier of the local node’s primary subdomain in [0],
while the element [1] has that of the secondary subdomain.  Since the subdomain
identifier is equivalent to the MPI rank of the node responsible for the subdomain as
its primary one, [0] is always the rank of the local node and [1] is that of its helpand
unless the local node is the family tree root. The array is allocated by init1() if
sdid points NULL, and then initialized by init1() while its element [1] is updataed
by rebalance1().

- The integer input argument nspec should have the number of species of particles, i.e.
S. This number is not necessary to mean the real number of species, e.g., the number
of variations of particle mass and charge. Instead, this variable must have the number
of memory regions each of which accommodates particles of a species, as discussed in
§3.2.1. The value of the argument is set into nOfSpecies by init1().

- The integer input argument maxfrac should have the tolerance factor percentage.
The value of the argument is set into α = maxFraction by init1().

- The argument nphgram should be the (double) pointer to an integer array of [2][S][N]
(or of [2×S ×N]) which is referred to as NOfPLocal[][][] in the library functions. Each
time the simulator body calls oh1_transbound(), it should set the element [p][s][m]
to the number of primary (p = 0) or secondary (p = 1) particles of species s residing
in the subdomain m and accommodated by the local node. Then oh1_transbound()
clears all elements to zero upon its return to the caller. The array itself is allocated
by init1() if nphgram points NULL.

- The argument totalp should be the (double) pointer to an integer array of [2][S] (or
of [2×S]) which is referred to as TotalPNext[][] in the library functions as the shadow
of TotalP[][]. Each array element [p][s] is updated each time oh1_transbound() is
called to notify the simulator body of the number of primary (p = 0) or secondary
(p = 1) particles of species s which the local node will have to accommodate in the
next simulation step. The array itself is allocated by init1() if totalp points NULL.

- The argument rcounts should be the (double) pointer to an integer array of [2][S][N]
(or of [2 × S × N]) which is referred to as RecvCounts[][][] in the library functions as
the shadow of NOfRecv[][]. Each array element [p][s][m] is updated each time oh1_
transbound() is called to notify the simulator body of the number of primary (p = 0)
or secondary (p = 1) particles of species s which the local node will have to receive
from the node m. The array itself is allocated by init1() if rcounts points NULL.

- The argument scounts should be the (double) pointer to an integer array of [2][S][N]
(or of [2 × S × N]) which is referred to as SendCounts[][][] in the library functions as
the shadow of NOfSend[][]. Each array element [p][s][m] is updated each time oh1_
transbound() is called to notify the simulator body of the number of particles of
species s which the local node will have to send to the node m as m’s primary (p = 0)
or secondary (p = 1) ones. The array itself is allocated by init1() if scounts points
NULL.

- The argument mycomm should be the pointer to a S_mycommf or S_mycommc structure,
or NULL for C-coded simulators. The simulator body can be unaware of the contents
of the structure if it only uses API funtions for collective communications in each


<!-- Page 160 -->

family such as oh1_broadcast(). If so, it is solely required to allocate the structure
body, or C-coded body may be free from even the allocation by giving NULL through
this argument. The Fortran API argument is referred to as MyCommF in the library
functions, while C’s counterpart is MyCommC which acts as the shadow of MyComm.

- The argument nbor should be the (double) pointer to an integer array of [3D], which
the simulator body can fully specify to make element nbor[k] have the MPI rank of a
neighbor of the local node conceptually at (π0, . . . , πD−1) in a D-dimensional integer
coordinate system each grid point (π′d, . . . , π′D−1) of which has a MPI process whose
rank is rank(π′d, . . . , π′D−1).

D−1∑
k =     νd3d  (νd ∈{0, 1, 2})

d=0
nbor[k] = rank(π0 + ν0 −1,  . . . , πD−1 + νD−1 −1)


Note that rank(π′0, . . . , π′D−1) can be −2 or less to indicated that the grid point
(π′0, . . . , π′D−1) has no processes.
On the other hand, the simulator body may entrust the setup of the array elements
to init1() either by giving the pointer to NULL through nbor or the pointer to an
array of [3D] whose first element is −1. In this case, init1() consults the array of
[D] given through pcoord assuming that r = rank(π0, . . . , πD−1) is given as follows
where Πd is the element [d] of the array.

rD−1 = πD−1     rd = rd+1Πd + πd     r = r0

- The argument pcoord should be the pointer to an integer array of [D] to describe the
process coordinate space Π0 × · · · × ΠD−1 where Πd is its element [d], if the simulator
body entrusts the initialization of the array specified by nbor.

- The integer input argument stats should have 0, 1 or 2 to specify the mode of
statistics collection and reporting, and is set into statsMode by init1(). If and only
if it is 1 or 2, statistics data are collected and measured.  If it is 2, the reporting is
repeated every r simulation steps where r = repiter, while the report is made only
at the end of simulation otherwise.

- The integer input argument repiter should have a non-negative number to specify
the number of simulation steps at the every end of which statistics are reported if
stats = 2. The value is set into reportIteration by init1().

- The integer input argument verbose should have a number in [0, 3] to specify the
level of verbose execution as follows.

  - 0 means to execute silently.

  - 1 means to execute reasonably verbosely.

  - 2 means to execute very verbosely.

  - 3 (or larger) means to execute with very verbose messages from all processes.

The value is set into verboseMode by init1().


<!-- Page 161 -->

The API functions almost simply call init1() passing all given arguments to it except
for the followings.

- oh1 init () passes the pointers to sdid, nphgram, totalp, rcounts, scounts and
nbor rather than themselves.

- oh1 init () passes mycomm to mycommf of init1() while NULL is passed through
mycommc of init1() to keep it from allocation of MyCommC.

- oh1 init() passes mycomm to mycommc of init1() while NULL  is passed through
mycommf of init1() telling it that the body of MyCommF is not required.  It also
casts the argument as S_mycommc pointer type, because mycomm is declared as a void
pointer to allow the simulator body to be completely unaware of the structure.


void
oh1_init_(int *sdid, int *nspec, int *maxfrac, int *nphgram,
int *totalp, int *rcounts, int *scounts, struct S_mycommf *mycomm,
int *nbor, int *pcoord, int *stats, int *repiter, int *verbose) {
init1(&sdid, *nspec, *maxfrac, &nphgram, &totalp, &rcounts, &scounts,
NULL, mycomm, &nbor, pcoord, *stats, *repiter, *verbose);
}
void
oh1_init(int **sdid, int nspec, int maxfrac, int **nphgram,
int **totalp, int **rcounts, int **scounts, void *mycomm,
int **nbor, int *pcoord, int stats, int repiter, int verbose) {
init1(sdid, nspec, maxfrac, nphgram, totalp, rcounts, scounts,
(struct S_mycommc*)mycomm, NULL, nbor, pcoord, stats, repiter,
verbose);
}


NeighborsShadow  Prior  to  give the definition  of  init1(), we have  to  declare two pointer  variables
NeighborsTemp  NeighborsShadow[3][3D] and NeighborsTemp[3D] being global but private to ohhelp1.c for
the communiations among init1(), oh1_neighbors() and build_new_comm().
The former keeps what the argument nbor of oh1_neighbors() points, i.e, *nbor, so
that build_new_comm() lets its elements [1][] have what the helpand of the local node
have in [0][] after rebalancing, and [2][] have what the local node itself had in [1][] before
rebalancing. As for the elements [0][], they should be consistent with *nbor[] of init1().
Therefore, init1()’s *nbor is kept in NeighborsTemp so that oh1_neighbors() makes
NeighborsShadow[0][] consistent with NeighborsTemp[] by copying the elements unless the
two pointers are equivalent, if oh1_neighbors() is called after init1() is called as recom-
mended. Note that if *nbor was NULL upon the call of init1() requiring to allocate the
neighborhood array, we allocate an array of [3][3D] for *nbor instead of [3D] so that *nbor
can be passed to oh1_neighbors() without allocating two arrays.
If oh1_neighbors() is called before init1() is called, on the other hand, init1() no-
tices this order reversal by NeighborsShadow ̸= NULL and initializes NeighborsShadow[0][]
to make them consistent with its *nbor[] = NeighborsTemp[]. Note that if *nbor = NULL in
this case, init1() allocates an array of [3D] instead of [3][3D] because *nbor given to the
function is definitely different from that given to oh1_neighbors(). Also note that oh1_
neighbors() notices the reversal by NeighborsTemp = NULL to delegate the initialization
to init1().


<!-- Page 162 -->

static int (*NeighborsShadow)[OH_NEIGHBORS] = NULL;
static int *NeighborsTemp = NULL;


init1()  The function init1() implements the initialization for its caller API functions, oh1_init_
() and oh1_init(), or part of that for its higher level counterparts init2() and init3().
The arguments of this function are almost same as those of oh1 init() but its mycomm is
split into two arguments mycommc and mycommf, which are NULL if called from oh1 init ()
or oh1 init() respectively.


void
init1(int **sdid, int nspec, int maxfrac, int **nphgram,
int **totalp, int **rcounts, int **scounts, struct S_mycommc *mycommc,
struct S_mycommf *mycommf, int **nbor, int *pcoord,
int stats, int repiter, int verbose) {

int nn, ns, me, i, s, clsize;
int *nb = *nbor;
int bl[2]={1,1};
MPI_Datatype tmptype[2]={MPI_DATATYPE_NULL, MPI_UB};
MPI_Aint disp[2]={0, sizeof(int)};


First, we obtain the size of MPI_COMM_WORLD and the local node’s rank in it by MPI_Comm_
size() and MPI_Comm_rank() to set them into nOfNodes = N and myRank. Then, we ini-
tialize currMode to MODE_NORM_PRI and accMode to 0 because we are in primary mode and
normal accommodataion at initial, and set the argument verbose into verboseMode and
messaging verbosely. After that we set arguments into corresponding global variables as fol-
lows; *sdid into SubdomainId allocating its body if necessary; nspec into nOfSpecies = S;
maxfrac into maxFraction = α; *nphgram into NOfPLocal allocating its body if necessary;
*totalp into TotalPNext allocating its body if necessary. The allocation of the argument
array bodies are done by mem_alloc(). As for SubdomainId, its first element is set to
myRank while the second element is initialized to −1 to indicate the local node has no hel-
pand, together with its substance RegionId. On the other hand, TotalP, the substance
of TotalPNext, is initialized to NULL to indicate it should be allocated and initialized by
transbound1() on its first call.  Finally, NOfPLocal is zero-cleared for the first particle
counting.

MPI_Comm_size(MCW, &nn);  nOfNodes = nn;
MPI_Comm_rank(MCW, &me);  myRank = me;
currMode = MODE_NORM_PRI;  accMode = 0;

verboseMode = verbose;
Verbose(1, vprint("oh_init"));

if (!*sdid)  *sdid = (int*)mem_alloc(sizeof(int), 2, "SubdomainID");
SubdomainId = *sdid;
(*sdid)[0] = RegionId[0] = me;  (*sdid)[1] = RegionId[1] = -1;
ns = nOfSpecies = nspec;
maxFraction = maxfrac;

if (!*nphgram)


<!-- Page 163 -->

*nphgram = (int*)mem_alloc(sizeof(int), 2*ns*nn, "NOfPLocal");
NOfPLocal = *nphgram;
if (!*totalp)  *totalp = (int*)mem_alloc(sizeof(int), 2*ns, "TotalP");
TotalPNext = *totalp;
TotalP = NULL;
for(i=0; i<2*ns*nn; i++)  NOfPLocal[i] = 0;


Next, we allocate RecvCounts[2][S][N] and SendCounts[2][S][N] by mem_alloc(), un-
less their corresponding arguments rcounts and scounts are NULL meaning that init1() is
called from init2(), and if they point NULL. On the other hand, if rcounts and scounts
point non-NULL pointers, the pointers are set into RecvCounts and SendCounts.  In any
cases, their substance NOfRecv[2][S][N] and NOfSend[2][S][N] are allocated.

if (rcounts) {
if (!*rcounts)
*rcounts = (int*)mem_alloc(sizeof(int), 2*ns*nn, "RecvCounts");
RecvCounts = *rcounts;
}
if (scounts) {
if (!*scounts)
*scounts = (int*)mem_alloc(sizeof(int), 2*ns*nn, "SendCounts");
SendCounts = *scounts;
}
NOfRecv = (int*)mem_alloc(sizeof(int), 2*ns*nn, "NOfRecv");
NOfSend = (int*)mem_alloc(sizeof(int), 2*ns*nn, "NOfSend");


Next we allocate the body of the following global variables for particle population lo-
cally used in the library, by mem_alloc(); NOfPrimaries[2][S][N], TotalPGlobal[N+1],
NOfPToStay[N], and InjectedParticles[2][2][N]. We also allocate TempArray[N] unless
OH_POS_AWARE is defined to mean it is allocated by level-4p initializer init4p() with larger
amount.
We also define the MPI data-type for the communication of particle histograms, a slice
[2][S][1] of integer arrays of [2][S][N] as follows.  Since a slice is a strided vector having
2S elements separated by N −1 array elements, the basic type for the slice is constructed
by MPI_Type_vector(). However, the slices should be arrayed contiguously for, e.g., MPI_
Alltoall(), so that a slice [∗][∗][n] is followed by [∗][∗][n + 1]. Therefore, we need to use
MPI_Type_struct() to create a two-element structure with the strided vector and MPI_
UB to make the resulting type T_Histogram have the extent of sizeof(int). Finally, we
commit the use of the type by MPI_Type_commit().

NOfPrimaries = (int*) mem_alloc(sizeof(int),  2*ns*nn, "NOfPrimaries");
TotalPGlobal = (dint*)mem_alloc(sizeof(dint), nn+1, "TotalPGlobal");
NOfPToStay   = (dint*)mem_alloc(sizeof(dint), nn, "NOfPToStay");
InjectedParticles = (int*)mem_alloc(sizeof(int), 4*ns, "InjectedParticles");
for (s=0; s<ns*2; s++)  InjectedParticles[s] = 0;
#ifndef OH_POS_AWARE
TempArray    = (int*) mem_alloc(sizeof(int),  nn, "TempArray");
#endif

MPI_Type_vector(2*ns, 1, nn, MPI_INT, tmptype);
MPI_Type_struct(2, bl, disp, tmptype, &T_Histogram);
MPI_Type_commit(&T_Histogram);


<!-- Page 164 -->

Next, we allocate Nodes[N], NodesNext[N] and NodeQueue[N] by mem_alloc(). For
each of Nodes[n], we give the constant n to its id element.

Nodes = (struct S_node*)mem_alloc(sizeof(struct S_node), nn, "Nodes");
NodesNext = (struct S_node*)mem_alloc(sizeof(struct S_node), nn,
"NodesNext");
for (i=0; i<nn; i++) Nodes[i].id = i;
NodeQueue =(struct S_node**)mem_alloc(sizeof(struct S_node*), nn,
"NodeQueue");


The next allocation with mem_alloc() is done for LessHeap and GreaterHeap.  Al-
though their node elements are indexed in the range of [0, N], we only refer to the elements
in [1, N]. Therefore, we allocate a memory space for 2N integers for each heap structure,
N for node and the other N for index, and make the pointer node point its non-exsistent
element [0] at one-element behind the allocated space.

LessHeap.node     = (int*)mem_alloc(sizeof(int), nn*2, "LessHeap") - 1;
LessHeap.index    = LessHeap.node + nn + 1;
GreaterHeap.node  = (int*)mem_alloc(sizeof(int), nn*2, "GreaterHeap") - 1;
GreaterHeap.index = GreaterHeap.node + nn + 1;


Next we allocate CommList[] which could have 2 · 3D(NS + 1) + N(S + 3) elemets as
discussed in §4.2.7. The required size is, however, can be larger than it with position-aware
particle management, that could need (14 + 4S)N elements, when D = 1 and S < 4.
Therfore we allocate CommList[] using the larger one by mem_alloc(). We also define the
MPI data-type for its element, which is simply a MPI_BYTE sequence of sizeof(struct␣S_
commlist), by MPI_Type_contiguous()28.

clsize = 2*OH_NEIGHBORS*(nn*ns+1)+nn*(ns+3);
if (clsize<(14+4*ns)*nn)  clsize = (14+4*ns)*nn;
CommList = (struct S_commlist*)mem_alloc(sizeof(struct S_commlist), clsize,
"CommList");
MPI_Type_contiguous(sizeof(struct S_commlist), MPI_BYTE, &T_Commlist);
MPI_Type_commit(&T_Commlist);


Next we allocate Comms.body[N] and initialize Comms.n to be 0 because it has no com-
municators in it. Then, after obtaining the group corresponding to MPI_COMM_WORLD to be
set into GroupWorld by MPI_Comm_group(), we initialize MyComm after allocating it by mem_
alloc(). We initialize its prime and sec elements to be MPI_COMM_NULL and rank, root
and black elements to be 0, so that even an accidental invocation of oh1_broadcast() or
other collective communication functions before the first call of oh1_transbound() results
in just no-operation rather than an error. Then if mycommc is not NULL, we copy MyComm into
its body after setting MyCommC to be the pointer. The initialization of MyCommF = mycommf is
similarly done but MPI_COMM_NULL for prime and sec elements is translated into its Fortran
form by MPI_Comm_c2f().

Comms.body = (MPI_Comm*)mem_alloc(sizeof(MPI_Comm), nn, "Comms");
Comms.n = 0;

28Because we ignore endian problem which could arise if an OhHelp’ed simulator were executed on a
heterogeneous parallel system.


<!-- Page 165 -->

MPI_Comm_group(MCW, &GroupWorld);
MyComm = (struct S_mycommc*)mem_alloc(sizeof(struct S_mycommc), 1, "MyComm");
MyComm->prime = MyComm->sec  = MPI_COMM_NULL;
MyComm->rank  = MyComm->root = MyComm->black = 0;
if ((MyCommC=mycommc))  *mycommc = *MyComm;
if ((MyCommF=mycommf)) {
MyCommF->prime = MyCommF->sec = MPI_Comm_c2f(MPI_COMM_NULL);
MyCommF->rank = MyCommF->root = MyCommF->black = 0;
}

Next, if the argument nbor points NULL, we allocate an array of [3D] or [3][3D] by mem_
alloc() and returns the pointer to it through nbor, according to NeighborsShadow ̸= NULL
or not, i.e., oh1_neighbors() has been called beforehand or not. Then, if we allocate the
array or the simulator body gives the array whose first element is −1, we initialize nbor[k]
by the followings where Πd = pcoord[d] to specify Π0 × · · · × ΠD−1 integer coordinate
space in which MPI processes for π = (π0, . . . , πD−1) are laid out with the rank rank(π) =
rank(π0, . . . , πD−1) and the local node of rank n is at π(n), after checking if N = Π0 ×
· · · × ΠD−1 and abort the execution by errstop() if it is not satisfied.

rD−1(π) = πD−1    rd(π) = rd+1(π)Πd + πd    rank(π) = r0(π)
D−1∑
k =     νd3d  (νd ∈{0, 1, 2})

d=0
nbor[k] = rank(π(n) + ν −(1, . . . , 1))

The implementation assumes that D ≤3 and is comprehensive for the case of D = 3.
However, by making νd = 0, πd = 0 and Πd = 1 for all d s.t. D ≤d < 3, we can cope with
the cases with D < 3.

if (!nb) {
if (NeighborsShadow) {
nb = *nbor = (int*)mem_alloc(sizeof(int), OH_NEIGHBORS, "Neighbors");
} else {
nb = *nbor = (int*)mem_alloc(sizeof(int), 3*OH_NEIGHBORS, "Neighbors");
}
nb[0] = -1;
}
if (nb[0]==-1) {
int p=pcoord[0];
int q=(OH_DIMENSION>1)?pcoord[1]:1, r=(OH_DIMENSION>2)?pcoord[2]:1;
int j, k, l;
int yplus=(OH_DIMENSION>1)?2:0, zplus=(OH_DIMENSION>2)?2:0;
int xoff, yoff, zoff;
if (nn!=p*q*r || p<0 || q<0 || r<0)
errstop("<# of x-nodes>(%d) * <# of y-nodes>(%d) * <# of z-nodes>(%d) "
"should be equal to <# of nodes>(%d)", p, q, r, nn);
i = me % p;  j = (me/p) % q;  k = me / (p*q);
for (l=0,zoff=-1; zoff<zplus; zoff++) {
for (yoff=-1; yoff<yplus; yoff++) {
for (xoff=-1; xoff<2; xoff++,l++) {
nb[l] = (i+xoff+p)%p + (((j+yoff+q)%q) + ((k+zoff+r)%r)*q)*p;
}
}
}


<!-- Page 166 -->

On the other hand, if the simulator body gives the array nbor setting its elements, we
check its consistency. That is, we check inter-node consistency by sending k to the process
nbor[k] if non-negative and/or receivng it from nbor[3D−1−k] if non-negative to examine
the received index is k, with MPI_Sendrecv(), MPI_Send() and MPI_Recv() depeding on
the existance of neighbors.  This consistency check may cause a deadlock but it is less
harmful than occuring in later simulation phase.

} else {
for (i=0; i<OH_NEIGHBORS; i++) {
int n=nb[i], m=nb[(OH_NEIGHBORS-1)-i], k;
MPI_Status st;
if (m>=0) {
if (n>=0)
MPI_Sendrecv(&i, 1, MPI_INT, n, 0, &k, 1, MPI_INT, m, 0, MCW, &st);
else
MPI_Recv(&k, 1, MPI_INT, m, 0, MCW, &st);
if (k!=i)
local_errstop("rank-%d’s %d-th neighbor rank-%d says "
"rank-%d is not %d-th neighbor but %d-th",
me, (OH_NEIGHBORS-1)-i, m, me, i, k);
} else if (n>=0) {
MPI_Send(&i, 1, MPI_INT, n, 0, MCW);
}
}
}

Now we have neighbor information of the local node in *nbor[3D] and let NeighborsTemp
be the pointer  for  it so that oh1_neighbors()  will  refer to  it afterward to make
NeighborsShadow[0][] consistent with NeighborsTemp[].  On the other hand,  if oh1_
neighbors() has been called beforehand to let NeighborsShadow ̸= NULL and two neigh-
borhood arrays are different, we have to initialize NeighborsShadow[0][] copying all elements
in *nbor[] into them.

NeighborsTemp = nb;
if (NeighborsShadow && nb!=(int*)NeighborsShadow)
for (i=0; i<OH_NEIGHBORS; i++)  NeighborsShadow[0][i] = nb[i];


Now we initialize DstNeighbors[k] = Neighbors[0][k] and SrcNeighbors[3D−1−k] so
that they have the followings where µ[k] = nbor[k].

 −(N + 1)    µ[k] < 0
DstNeighbors[k] =   µ[k]          µ[k] ≥0 ∧∀k′ < k(µ[k′] ̸= µ[k])
                                  −(µ[k] + 1)  µ[k] ≥0 ∧∃k′ < k(µ[k′] = µ[k])

 −(N + 1)    µ[k] < 0
SrcNeighbors[3D −1 −k] =   µ[k]          µ[k] ≥0 ∧∀k′ > k(µ[k′] ̸= µ[k])
                                  −(µ[k] + 1)  µ[k] ≥0 ∧∃k′ > k(µ[k′] = µ[k])

For the occurence check whether there is k′ such that µ[k′] = µ[k], we use TempArray[N].
We make the bit-0 of the element of the array [n] be 1 if and only if there is k′ < k such
that n = µ[k′] and make its bit-1 be 1 if and only if there is k′ > k such that n = µ[k′],
and look it up when we have k such that nd = µ[k] and ns = µ[3D−1−k]. That is, after


<!-- Page 167 -->

initializing TempArray[m] to be 0 for all m ∈[0, N −1], we examine its elements [nd] and
[ns] whenever we have k such that nd = µ[k] ≥0 and ns = µ[3D−1−k] ≥0, and turn bit-0
and bit-1 of the elements to 1 respectively.

DstNeighbors = Neighbors[0];
for (i=0; i<nn; i++) TempArray[i] = 0;
for (i=0; i<OH_NEIGHBORS; i++) {
int dst=nb[i],  src=nb[(OH_NEIGHBORS-1)-i];
if (dst<0)  DstNeighbors[i] = -(nn+1);
else {
DstNeighbors[i] = (TempArray[dst]&1) ? -(dst+1) : dst;
TempArray[dst] |= 1;
}
if (src<0)
SrcNeighbors[i] = -(nn+1);
else {
SrcNeighbors[i] = (TempArray[src]&2) ? -(src+1) : src;
TempArray[src] |= 2;
}
}

Finally, we finish the function setting variables for statistics, stats into statsMode and
repiter into reportIteration.

statsMode = stats;
reportIteration = repiter;
}


#### 4.3.4 mem_alloc()

mem_alloc()  The function mem alloc(), called from init1(), transbound1(), init2(), init3(), init_
subdomain_passively() and init_fields(), allocates the memory region whose byte-
size is specified by its arguments, namely e × c where e = esize for the element size
and c = count for the number of elements, and returns the base pointer to the allocated
region. The allocation is done by malloc() whose failure stops the execution by mem_
alloc_error() which produces an error message containing the name of the variable to be
allocated given by varname and the required byte-size e × c.


void*
mem_alloc(int esize, int count, char* varname) {

size_t size = (size_t)esize*(size_t)count;
void* ptr = malloc(size);
if (!ptr) mem_alloc_error(varname, size);
return(ptr);
}


#### 4.3.5 mem_alloc_error()

mem_alloc_error()  The  function  mem alloc error(),  called  from  mem_alloc()  and  oh2_max_local_
particles(), aborts the execution due to the memory shortage with an error message
showing its cause given by the arguments, the variable name varname and the required
byte-size size, by errstop().


<!-- Page 168 -->

void
mem_alloc_error(char* varname, size_t size) {
errstop("out of virtual memory for %s(%lld)", varname, size);
}


#### 4.3.6 errstop() and local_errstop()

errstop()  The funtion errstop(), called from init1(), mem_alloc_error(), try_stable1(), oh2_
max_local_particles(), init_subdomain_actively(), init_subdomain_passively()
and init_fields(), stops the execution gracefully by MPI_Finalize() and exit() after
showing an error message given through its variable number arguments following format.
The message printing is done solely by the node of rank 0 by vprintf() and fprintf()
with macros for variable number arguments va_start() and va_end().

local_errstop() On the other hand, the funciton local errstop()  is for errors detected by the local
process in init1(), transbound1(), sched_comm(), oh2_inject_particle() and init_
subdomain_actively(). Therefore, the error message is printed by the local process itself,
and the execution is stopped ungracefully by MPI_Abort().


void
errstop(char* format, ...) {
va_list v;
va_start(v, format);

if (myRank==0) {
vfprintf(stderr, format, v);
fprintf(stderr, "\n");
}
va_end(v);
MPI_Finalize();  exit(1);
}
void
local_errstop(char* format, ...) {
va_list v;
va_start(v, format);

vfprintf(stderr, format, v);
fprintf(stderr, "\n");
va_end(v);
MPI_Abort(MCW, 1);
}


#### 4.3.7 oh1_neighbors()

oh1_neighbors_()  The API functions oh1_neighbors_() for Fortran and oh1_neighbors() for C provide a
oh1_neighbors()  simulator body calling them with an access to neighbor information kept in the library
through its argument nbor. The Fortran API simply calls its C counterpart to which it
gives the pointer to its argument nbor.


<!-- Page 169 -->

void
oh1_neighbors_(int *nbor) {
oh1_neighbors(&nbor);
}
void
oh1_neighbors(int **nbor) {
int *nb = *nbor;
int i;


First,  if *nbor = NULL requiring the allocation of the neighborhood array, we allo-
cate that of [3][3D] by mem_alloc() and return the pointer to it through nbor.  Then,
if NeighborsTemp  ̸= NULL to mean init1() has already been called as expected but
*nbor arguments of init1() and this function are different, we copy NeighborsTemp[]
into *nbor[0][] to make them consistent. Othewise we leave *nbor[0][] unchanged because
init1() will initialize them afterward referring to NeighborsShadow or, most usually, it
has done for its *nbor being equivalent to *nbor of this function. Finally, we save *nbor
into NeighborsShadow so that build_new_comm() will upadate elements in [1][] and [2][]
and, if init1() has not been called yet, it will initialize those in [0][].

if (!nb)
nb = *nbor = (int*)mem_alloc(sizeof(int), 3*OH_NEIGHBORS, "Neighbors");
if (NeighborsTemp && nb!=NeighborsTemp)
for (i=0; i<OH_NEIGHBORS; i++)  nb[i] = NeighborsTemp[i];
NeighborsShadow = (int(*)[OH_NEIGHBORS])nb;
}


#### 4.3.8 oh1_families()

FamIndex  Prior to give the definition of API functions oh1_families[_](), we have to declare two
FamMembers  pointer variables FamIndex and FamMembers being global but private to ohhelp1.c for the
communiations among oh1_families(), try_primary1() and build_new_comm(). These
pointers are made equivalent of the arrays pointed by famindex and members arguments
of oh1_families() by the function so that other functions updates the arrays to show the
family configuration to a simulator body through the arrays. More specifically, the element
[m] of FamIndex[N+1] has the index im of the array FamMembers[2N] whose elements
{[j] | j ∈[im, im+1)} are the ranks of the members in the family whose helpand is m, which
is always registered in FamMembers[im]. In addition FamMembers[2N−1] has the rank of the
root of the helpand-helper tree.

static int *FamIndex = NULL;
static int *FamMembers = NULL;


oh1_families_()  The API functions oh1_families_() for Fortran and oh1_families() for C provide a
oh1_families()  simulator body calling them with an access to family information kept in the library through
its arguments famindex and members. The Fortran API simply calls its C counterpart to
which it gives the pointers to its arguments famindex and members.


void
oh1_families_(int *famindex, int *members) {
oh1_families(&famindex, &members);


<!-- Page 170 -->

}
void
oh1_families(int **famindex, int **members) {
int *fidx = *famindex,  *fmem = *members;
int nn, i;


First, we call MPI_Comm_size() to obtain N because init1() may have not been called
yet and thus nOfNodes can be undefined, though very unlikely. Then if *famindex = NULL
and/or *members = NULL requiring the allocation of both or either of the arrays for family
configuration, we allocate those of [N+1] and/or [2N] by mem_alloc() and return the
pointers to them through famindex and/or members. Next we initialize the arrays so that
the elements [m] of them commonly have m for all m ∈[0, N) because F(m) = {m}
for all m in the initial primary mode. We also initialize *famindex[N] = N to make
*famindex[m+1] −*famindex[m] = |F(m)| = 1 for all m ∈[0, N) including m = N −1.
Finally, we save *famindex and *members into FamIndex and FamMembers so that try_
primary1() and build_new_comm() refer to them for update.

MPI_Comm_size(MCW, &nn);
if (!fidx)
fidx = *famindex = (int*)mem_alloc(sizeof(int), nn+1, "FamIndex");
if (!fmem)
fmem = *members = (int*)mem_alloc(sizeof(int), nn*2, "FamMembers");
for (i=0; i<nn; i++)  fidx[i] = fmem[i] = i;
fidx[nn] = nn;
FamIndex = fidx;  FamMembers = fmem;
}


#### 4.3.9 set_total_particles()

set_total_particles()  The function set_total_particles(), called from transbound1() and level-2 API func-
tions oh2_set_total_particles[_](), is to allocate TotalP[p][s] by mem_alloc() if it is
NULL, and to calculate TotalP[p][s], primaryParts and totalParts as follows.

N−1∑
TotalP[0][s] =     q(n)[0][s][m]
{m=0                               ∑N−1
q(n)[1][s][m]  currMode mod 2 ̸= 0 ∧parent(n) ≥0                                 TotalP[1][s] =    m=0
0                   otherwise
S−1∑
primaryParts =     TotalP[0][s]

s=0
S−1∑
totalParts = primaryParts +    TotalP[1][s]

s=0

The necessity of the function is based on the fact these substance variables are usually
calculated by transbound1() and/or transbound2() but they are undefined at or before
the first call of these functions. Therefore it is intended to call this function at or before the
first call of transbound1() to let it know the very initial state of the particles in the local
node, but the function is designed to work well in other occasions. The values calculated
for TotalP[p][s] is also stored in its shadow TotalPNext[p][s] as the reasonable output of
oh2_set_total_particles().


<!-- Page 171 -->

void
set_total_particles() {
int ns=nOfSpecies, nn=nOfNodes, nnns=nn*ns;
int cm=(Mode_PS(currMode))&&(RegionId[1]>=0);
int s, i, j, tpp, tps;

if (!TotalP)  TotalP = (int*)mem_alloc(sizeof(int), 2*ns, "TotalP");
primaryParts = 0;  totalParts = 0;
for (s=0,j=0; s<ns; s++) {
for (i=0,tpp=0,tps=0; i<nn; i++,j++) {
tpp += NOfPLocal[j];  tps += NOfPLocal[nnns+j];
}
if (!cm)  tps = 0;
TotalP[s] = TotalPNext[s] = tpp;  TotalP[ns+s] = TotalPNext[ns+s] = tps;
primaryParts += tpp;  totalParts += tps;
}
totalParts += primaryParts;
}


#### 4.3.10 oh1_transbound() and transbound1()

oh1_transbound_()  The API functions oh1 transbound () for Fortran and oh1 transbound() for C provide
oh1_transbound()  a simulator body calling them with the core mechanism of level-1 library. The functions
have the following arguments.

- The input argument currmode should be an integer in {0, 1} to mean one of the
followings29.

  - 0 means we are in primary mode.

  - 1 means we are in secondary mode.

- The input argument stats is usually non-zero (just 1 is sufficient) to mean that
statistics data will be collected  if statsMode is non-zero.  However, the simulator
body can give 0 through this argument to disable statistics processing temporalily
when it calls the function, for example, for the initial particle distribution.

The API functions simply call transbound1() and pass its return value in {−1, 0, 1} to
their callers to notify them that the next simulation step is performed in primary (0) or sec-
ondary mode with (−1) or without (1) (re)building the helpand-helper configuration. The
call of transbound1(), however, needs an additional argument level being 1 to indicate
that the function is called from level-1 API functions. This level argument can be 2 (or
larger) for the call from functions in level-2 (or higher) library for which setting NOfRecv/
RecvCounts and NOfSend/SendCounts are unnecessary because particles are transferred in
the library.

int
oh1_transbound_(int *currmode, int *stats) {

29In earlier versions, we needed information more detailed than simply showing the current execution
mode and thus this argument, but what we need in the current version is just the mode and it is stored
in the global variable currMode. However we are keeping this almost unnecessary argument for backward
compatibility, which also require us to check the consistency with currMode and to extract its bit-0 to obtain
the mode.


<!-- Page 172 -->

return(transbound1(*currmode, *stats, 1));
}
int
oh1_transbound(int currmode, int stats) {
return(transbound1(currmode, stats, 1));
}


transbound1()  When  the  function  transbound1()   is  called,  from  oh1_transbound_()  or  oh1_
transbound(), or higher level counterparts transbound2() or transbound3(), each ele-
ment of the histogranm array NOfPLocal[p][s][m] = q(n)[p][s][m] has the number of particles
accommodated by the local node n for each p ∈{0, 1}, s ∈[0, S−1] and d ∈[0, N−1].

The first job of the function, besides starting time measurement and verbose messaging,
is to call set_total_particles() if TotalP is NULL to indicate that it is the first call of the
function and thus TotalP, primaryParts and totalParts should be initialized according
to NOfPLocal. We also check  if LSB (mode indicator), extraced by Mode_PS(), of the
argument currmode is equal to that of currMode to confirm the simulator body and library
agree on the execution mode and abort execution by local_errstop() unless they are
equal30.

int
transbound1(int currmode, int stats, int level) {
int ret=MODE_NORM_SEC, nn=nOfNodes, ns=nOfSpecies, nnns=nn*ns, nnns2=2*nnns;
int i, j, k, s, p, tp, tpn, *nbor;
dint nofp;

Verbose(1,vprint("oh_transbound"));
if ((stats=statsMode&&stats)) oh1_stats_time(STATS_TRANSBOUND, 0);
if (!TotalP)  set_total_particles();
currmode = Mode_PS(currmode);
if (currmode!=Mode_PS(currMode))
local_errstop("currmode given to oh_transbound() does not match with "
"that the library maintains");


Next we calculate the followings to have temporary local values of TotalPGlobal for
the local node n.

∑  S−1∑
Qmn = TotalPGlobal[m] =            q(n)[p][s][m]
p∈{0,1} s=0

N−1∑
Qn =   Qmn
m=0
nbor(p) = {m | m ∈Neighbors[p], m ≥0}
∑   N−1∑   S−1∑
Q′n =                      q(n)[p][s][m]
p∈{0,1} m∈nbor(p) s=0
{
0  Qn = Q′n ∧currMode < 2                         TotalPGlobal[N] =
1  otherwise

30We can simply ignore the argument currmode and use currMode instead, but checking the consistency
could help to debug the simulator body.


<!-- Page 173 -->

That is, Qmn  is the number of particles in the subdomain m and currently accomadate by the
local node n which has Qn particles in total. On the other hand, Q′n is the total number
of primary and secondary particles residing in the primary and secondary subdomains
themselves or in their neighboring subdomains.  Therefore, TotalPGlobal[N] = 0 if and
only if the outgoing particles from the local node’s primary or secondary subdomain will be
transferred to their neighbors, providing that Mode_Is_Norm() for currMode is true to mean
unnatural particle accommodation is not forced. We refer to this natural accommodation
of particles as normal accommodation. Otherwise, i.e., if TotalPGlobal[N] = 1, the local
node has particles which should be transferred some distant subdomains and is in the state
of anywhere accommodation, or is forced to be considered so.
Then we gather q(m)[p][s][n], i.e., NOfPLocal[p][s][n] of node m to have NOfPrimaries[p]
[s][m] of the local node n by MPI_Alltoall() using the MPI data-type T_Histogram for
the slice NOfPLocal[∗][∗][n]31. We also obtain;

N−1∑
Pm = TotalPGlobal[m] =   Qmk
k=0

for all m ∈[0, N−1] by MPI_Allreduce() on TotalPGlobal which also give us;
{
= 0 ∀m : Qm = Q′m                    TotalPGlobal[N]
> 0 ∃m : Qm ̸= Q′m

to make bit-1 (accommodation indicator) of currmode indicate whether we have normal (0)
or anywhere (1) accommodation globally.

for (i=0; i<nn; i++) TotalPGlobal[i] = 0;
for (p=0,j=0,tp=0,tpn=0; p<=currmode; p++) {
for (s=0; s<ns; s++) {
for (i=0; i<nn; i++,j++) {
int np=NOfPLocal[j];
TotalPGlobal[i] += np;  tp += np;
}
}
for (i=0,nbor=Neighbors[p]; i<OH_NEIGHBORS; i++) {
int n=nbor[i];
if (n>=0)
for (s=0,k=(p==0)?n:n+nnns; s<ns; s++,k+=nn)  tpn+= NOfPLocal[k];
}
}
TotalPGlobal[nn] = (tp==tpn && Mode_Is_Norm(currMode)) ? 0 : 1;

MPI_Alltoall(NOfPLocal, 1, T_Histogram, NOfPrimaries, 1, T_Histogram, MCW);
#ifndef INTEL_MPI_BUG_FIXED
for (p=0,k=myRank; p<2; p++)  for (s=0; s<ns; s++,k+=nn)
NOfPrimaries[k] = NOfPLocal[k];
#endif
MPI_Allreduce(MPI_IN_PLACE, TotalPGlobal, nn+1, MPI_LONG_LONG_INT, MPI_SUM,
MCW);
if (TotalPGlobal[nn])  currmode = Mode_Set_Any(currmode);


31Since Intel MPI has a bug in MPI Alltoall() with 12 ≤N ≤16 and the data type T Histogram that
NOfPrimaries[p][s][n] of the local node n is not updated for p > 0 or s > 0, we have to copy NOfPLocal[p][s][n]
to it explicitly until the bug is fixed.


<!-- Page 174 -->

Then we calculate the followings.

N−1∑
P = nOfParticles =   Pm
m=0
Pmax = nOfLocalPMax = ⌊P(100 + α)/(100N)⌋

We also let accMode have the accommodation mode according to currmode by Mode_Is_
Any().

for (i=0,nofp=0; i<nn; i++)  nofp += TotalPGlobal[i];
nOfParticles = nofp;
nOfLocalPMax = nofp*(maxFraction+100)/100/nn;
accMode = Mode_Is_Any(currmode) ? 1 : 0;


Then here is the heart of the balancing examination, but we skip it if transbound1() is
called from level-2 or higher library, leaving the examination to level-2 counterparts of try_
primary1(), try_stable1() and rebalance1(),  i.e.  try_primary2(), try_stable2()
and rebalance2(), giving them currmode to show the current execution mode and accom-
modation type as the return value. Otherwise, we first call try_primary1() to examine if
we can stay in or turn to primary mode.  If so, the return value is set to MODE_NORM_PRI
to indicate we will be in primary mode in the next step. Otherwise, if we have been in
secondary mode, we check if the particle movement still allows to keep the helpand-helper
configuration by try_stable1(). If this examination fails and thus we need reconfiguration,
or we have been in primary mode and thus need to build the configuration from scratch, we
call rebalance1() to do that setting the return value to MODE_REB_SEC to show we have
new configuration, while the return value is MODE_NORM_SEC, being the initial default value
of the local variable ret, if success.

if (level>1) return(currmode);
if (try_primary1(currmode, 1, stats))  ret = MODE_NORM_PRI;
else if (!Mode_PS(currmode) || !try_stable1(currmode, 1, stats)) {
rebalance1(currmode, 1, stats);  ret = MODE_REB_SEC;
}

Finally, we clear NOfPLocal[][][] to give the histogram base  for the next simula-
tion step, and copy NOfRecv[][][] and NOfSend[][][] to their shadows RecvCounts[][][] and
SendCounts[][][], and also copy TotalPNext[][] to  its substance TotalP[][].  Note that
NOfPLocal[1][][] should be cleared even when we will be in primary mode in the next step,
because at least they are referred to by the rebalance1() itself. Then we return to the
simulator body with return value indicating that the mode in the next step is primary (0),
secondary without reconfiguration (1), or secondary with reconfiguration (−1), also setting
it into currMode.

for (i=0; i<nnns2; i++) {
NOfPLocal[i] = 0;  RecvCounts[i] = NOfRecv[i];  SendCounts[i] = NOfSend[i];
}
for (s=0; s<ns*2; s++) TotalP[s] = TotalPNext[s];
return((currMode=ret));
}


<!-- Page 175 -->

#### 4.3.11 try_primary1()

try_primary1()  The function try primary1(), called from transbound1() and try_primary2() being the
level-2 counterpart of this function, examines if we can stay in or turn to primary mode.
If so, we set NOfRecv[][][] and NOfSend[][][] from NOfPrimaries[][][] and NOfPLocal[][][]
respectively. The function has three arguments currmode, level and stats whose meanings
are almost as same as those of transbound1(), but a little bit different from them in the
following points; currmode has the particle accommodation type in its bit-1; and stats is
the logical conjunction of statsMode and that given to transbound1().


int
try_primary1(int currmode, int level, int stats) {
int nn=nOfNodes, ns=nOfSpecies, nnns=nn*ns, me=myRank, nlpmax=nOfLocalPMax;
int i, j, s;


After verbose messaging, we perform the main job of this function to check if Pm =
TotalPGlobal[m] ≤Pmax = nOfLocalPMax for all m. We continue the execution including
particle transfer statistics calculation and another verbose messaging if it is satisfied, or
return to the caller with FALSE to tell it to do try_stable1() and/or rebalance1().

Verbose(2,vprint("try_primary(%s,%s)",
Mode_PS(currmode)?"secondary":"primary",
Mode_Acc(currmode)?"anywhere":"normal"));
for (i=0; i<nn; i++) {
if (TotalPGlobal[i]>nlpmax) return(FALSE);
}
if (stats) stats_primary_comm(currmode);
Verbose(2,vprint("try_primary=TRUE"));


Then if the function is called from level-2 or higher library, we simply return to try_
primary2() with TRUE to let it perform primary mode particle transfers, after setting
RegionId[1] and its shadow SubdomainId[1] to −1 to indicate the local node does not have
a secondary subdomain, and letting FamIndex[m] = FamMembers[m] = m for all m ∈[0, N)
to represent F(m) = {m} if the previous mode is secondary and oh1_families() has been
called beforehand to make these pointers non-NULL.

SubdomainId[1] = RegionId[1] = -1;
if (Mode_PS(currmode) && FamIndex) {
int *fidx = FamIndex,  *fmem = FamMembers;
for (i=0; i<nn; i++)  fidx[i] = fmem[i] = i;
fidx[nn] = nn;
}
if (level>1) return(TRUE);

Otherwise, we set the element values of NOfSend, NOfRecv and TotalPNext of the local
node n as follows.
∑     ∑
NOfSend[0][s][m] =     q(n)[p][s][m] =    NOfPLocal[p][s][m]  (m ̸= n)

p∈{0,1}              p∈{0,1}
NOfSend[0][s][n] = q(n)[1][s][n] = NOfPLocal[1][s][n]
NOfSend[1][s][m] = 0


<!-- Page 176 -->

∑     ∑
NOfRecv[0][s][m] =     q(m)[p][s][n] =    NOfPrimaries[p][s][m]  (m ̸= n)

p∈{0,1}              p∈{0,1}
NOfRecv[0][s][n] = q(n)[1][s][n] = NOfPrimaries[1][s][n]
NOfRecv[1][s][m] = 0
N−1∑ ∑            N−1∑ ∑
TotalPNext[0][s] =           q(m)[p][s][n] =          NOfPrimaries[p][s][m]
m=0 p∈{0,1}           m=0 p∈{0,1}

The equations above mean that we simply send all particles accommodated by the local
node to other nodes responsible for subdomains they reside, while also simply gather all
particles residing in the local node’s subdomain from other nodes. Note that we consider the
particle amount q(n)[1][s][n] = NOfPLocal[1][s][n] = NOfPrimaries[1][s][n] as transferred
in the local node itself because it should require moves from the store of secondary particles
to that of primary.
Then we return to the caller with TRUE to indicate that we will be in primary mode in
the next simulation step.

for (i=0,s=0; s<ns; s++) {
int t = 0;
for (j=0; j<nn; j++,i++) {
NOfSend[i] = NOfPLocal[i] + NOfPLocal[i+nnns];
t += (NOfRecv[i] = NOfPrimaries[i] + NOfPrimaries[i+nnns]);
NOfSend[i+nnns] = NOfRecv[i+nnns] = 0;
if (j==me) {
NOfSend[i] -= NOfPLocal[i];  NOfRecv[i] -= NOfPrimaries[i];
}
}
TotalPNext[s] = t;  TotalPNext[ns+s] = 0;
}
return(TRUE);
}


#### 4.3.12 Macro Special_Pexc_Sched()

Special_Pexc_Sched()  The macro Special Pexc Sched(LEVEL), used in try_stable1() and rebalance1(), is
expanded to true iffthe argument LEVEL is negative to mean that these functions are called
from their higher-level counterparts which have their own particle exchange scheduling
mechanism (for position-aware particle management) and thus those in the level-1 functions
should be skipped.


#define Special_Pexc_Sched(LEVEL) (LEVEL<0)


#### 4.3.13 try_stable1()

try_stable1()  The function try stable1(), called from transbound1() and try_stable2() being the
level-2 counterpart of this function, examines if the current helpand-helper configuration
sustains the particle movements crossing subdomain boundaries which can bring intolerable
load imbalance.  If so, we make an all-to-all type particle transfer schedule to keep the
balanced situation and set NOfRecv[][][] and NOfSend[][][] according to the schedule. The
function has three arguments currmode, level and stats whose meanings are as same as
those of try_primary1().


<!-- Page 177 -->

int
try_stable1(int currmode, int level, int stats) {
int nn=nOfNodes;
int nlpmax=nOfLocalPMax;
struct S_node *node, *ch;
int i;


The first job of the function, besides starting time measurement and verbose messaging,
is to call count_stay() to have the followings for all n ∈[0, N−1].

S−1∑
Qnn = Nodes[n].stay.prime =     q(n)[0][s][n]
s=0
S−1∑
Qparent(n)n    = Nodes[n].stay.sec =     q(n)[1][s][parent(n)]
                s=0     
S−1∑     ∑
NOfPToStay[n] =     q(n)[0][s][n] +     q(c)[1][s][n]
s=0               c∈H(n)
Nodes[n].get.prime = Nodes[n].get.sec = 0

That is, Nodes[n].stay.{prime, sec} is set to the number of primary/secondary particles
currently accommodated by the node n including those injected into primary/secondary
subdomains, and NOfPToStay[n] is set to the sum of these staying particles in the family
of n (i.e., F(n)), while Nodes[n].get.{prime, sec} are intialized to 0.

if (stats) oh1_stats_time(STATS_TRY_STABLE, 0);
Verbose(2,vprint("try_stable"));
count_stay();


Now we examine if we can keep the helpand-helper configuration by traversing the family
tree in bottom-up (leaf-to-root) manner. The traversal is done by scanning NodeQueue[i]
for all i ∈[0, N−1] ascendingly because for any helpand-helper pair stored in NodeQueue[ip]
and NodeQueue[ic] it is guaranteed that ip > ic. Thus we perform the followings for each
node Nodes[n] = NodeQueue[i].

(1) At the visit of n, Nodes[n].get.prime is 0 if n is a leaf node, or the sum of the rooms
in n’s helpers to which n can move its primary particles if required. That is, this
value putprimemax = Pnput is defined as follows.

 Pm                      H(m) = ∅
P mmin =     ∑          min
c   ))  H(m) ̸= ∅               max(0, Pm −   (Pmax −P
c∈H(m)
Qgetm = Pmax −(P mmin + Qnm)
∑
Pnput =    max(0, Qgetm )
m∈H(n)


<!-- Page 178 -->

(2) NOfPToStay[n] has remained unchanged from its initial value if all helpers have some
such rooms, i.e., Qgetm ≥0 for all m ∈H(n). However, if some helper m has no rooms
but should put its secondary partiles out to other family members to make room for
its own primary particles, NOfPToStay[n] has been decremented by the number of the
overflown particles to be put out, namely Qnm −(Pmax −P mmin ). Therefore the current
value of NOfPToStay[n], namely Qstayn    is defined as follows.
∑
Qstayn  = Qnn +   min(Qnm, Pmax −P mmin )
m∈H(n)

Since Pn = TotalPGlobal[n] has the system-wide total number of particles in the
subdomain n, getprime = Pn −Qstayn    is the number of particles n’s family members
have to get not only from non-family members but possibly from n’s helpers having
overflown secondary particles.

(3) We calculate putprime = −P nget = min(Pnput −(Pn −Qstayn   ), Qnn) being the maximum
number of primary particles that the node n can put out to its helpers if positive,
or reversed miminum one that n have to get from its helpers if negative. Then we
record P nget in Nodes[n].get.prime so that we refer to it when we determine the real
number of particles to be put or gotten afterward.

(4) We calculate the maximum room in n, namely room = Qgetn = Pmax −(Qnn + P nget +
Qparent(n)n        ) which is equivalent to the defition Qgetn = Pmax −(P nmin + Qparent(n)n        )
because Qnn + P nget = P nmin as proven as follows.
Pnget = −min(P nput −(Pn −Qstayn   ), Qnn)
= max((Pn −Qstayn   ) −Pnput , −Qnn)
∑
= max((Pn −(Qnn +   min(Qnm, Pmax −P mmin ))) −P nput , −Qnn)
m∈H(n)
∑
P nget + Qnn = max(Pn −   min(Qnm, Pmax −P mmin ) −P nput , 0)
m∈H(n)
∑
min(Qnm, Pmax −P mmin ) + P nput
m∈H(n)
∑        ∑
=   min(Qnm, Pmax −Pmmin ) +    max(0, Qgetm )
m∈H(n)                    m∈H(n)
∑        ∑
=   min(Qnm, Pmax −Pmmin ) +    max(0, Pmax −Pmmin −Qnm)
m∈H(n)                    m∈H(n)
∑ (                                                    )
=    min(Qnm, Pmax −P mmin ) + max(0, Pmax −P mmin −Qnm)
m∈H(n)
∑ (                                                     )
=    min(Qnm, Pmax −P mmin ) + max(Qnm, Pmax −P mmin ) −Qnm
m∈H(n)
∑ (                        )
=    Qnm + Pmax −Pmmin −Qnm
m∈H(n)
∑ (            )
=    Pmax −Pmmin
m∈H(n)
∑
P nget + Qnn = max(Pn −   (Pmax −Pmmin ), 0) = P nmin
m∈H(n)


<!-- Page 179 -->

Note that Qgetn  can be negative even if P nget is negative to mean n can put out some
particles to decrease the number of particles currently accommodating which can be
larger than Pmax by one or more of the following reasons32.

(a) Some particles are injected into n’s primary/secondary subdomain to make Qnn
and/or Qparent(n)n        larger than those set in the previous call of transbound1().

(b) Some particles are removed to make Pmax less than that examined in the previous
call of transbound1().
(c) The position-aware particle management is in effect so that Qnn and/or Qparent(n)n
can be a little bit larger than those set in the previous call of try_primary1(),
try_stable1() or rebalance1().

On the other hand, Qgetn  can be positive with a positive Pnget of course, because Qnn
and/or Qparent(n)n       could be made small by the particle movement crossing subdomain
boundaries.

(5) If Qgetn  ≥0, i.e., Qgetn = max(0, Qgetn  ), we add it to Nodes[parent(n)].get.prime =
P parent(n)put      to show the contribution from n.

(6) If Qgetn < 0, the node n has to put a number of secondary particles out, namely −Qgetn  ,
to accommodate its floating primary particles. Thus we record the room Qgetn   itself
(i.e., a negative number) into Nodes[n].get.sec to indicate the negative amount to
be gotten. Then we also add Qgetn  to NOfPToStay[k], or decrement it by −Qgetn  in
other words, to state that the −Qgetn  secondary particles in the node n cannot stay in
it and thus have to be put out. It is proven that this operation correctly contributes
to Qstayk   by Qstayk,n = min(Qkn, Pmax −P nmin ), where k = parent(n), as follows.
{
Qkn         Qgetn ≥0                          Qstayk,n =                           Qkn + Qgetn   Qgetn < 0
= Qkn + min(Qgetn  , 0)
= Qkn + min(Pmax −(P nmin + Qkn), 0)
= min(Pmax −P nmin , Qkn)
= min(Qkn, Pmax −Pnmin )

Finally we check if −Qgetn ≤Qkn = Nodes[n].stay.sec meaning that the number of
overflown secondary particles is not greater than that of secondary particles the node
n has.  If it does not hold meaning that the node n cannot accommodate primary
particles the node should do, we stop the procedure returning FALSE to the caller
transbound1() or try_stable2(). The equivalence of this inequality to that more
comprehensible one, Pmax ≥P nmin being the negation of the failure condition shown
in §2.3, is proven as follows.

Qkn + Qgetn = Qkn + Pmax −(P nmin + Qkn) = Pmax −P nmin
⇒Qkn ≥−Qgetn  ↔Pmax ≥Pnmin

32The fix in v0.9.8 coped with the reason (a) by excluding injected particles from Qmn , but ignored (b)
and (c) which we cannot cope with by adjusting Qmn .


<!-- Page 180 -->

Note that adding to Nodes[parent(n)].get.prime in (3) and (4), and updating NOfPToStay
[parent(n)] in (5) should not be done for the node n being the root of the family tree, which
should be at the bottom of NodeQueue[]. However, checking the satisfaction of −Qgetn ≤Qkn,
the righthand side should be 0 for the root, in (5) must be done for the root as other non-root
nodes.

for (i=0; ; i++) {                    /* bottom up traversal of node tree */
int nid, stayprime, staysec;
dint putprimemax, floating, putprime, room, getsec=0;
struct S_node *parent;
node = NodeQueue[i];
nid = node->id;
putprimemax = node->get.prime;      /* 0 for leaf, or max number of p’s
children can accommodate for non
leaf */
stayprime = node->stay.prime;
staysec = node->stay.sec;
parent = node->parent;
floating = TotalPGlobal[nid] - NOfPToStay[nid];
putprime = putprimemax - floating;
if (putprime>stayprime)  putprime = stayprime;
node->get.prime = -putprime;
room = nlpmax - (stayprime + staysec - putprime);
if (room<0) {                       /* have to put some secondaries to get
primaries */
node->get.sec = getsec = room;    /* getsec is negative to mean to put */
if (room+staysec<0) return(FALSE);
/* nlpmax<stay.prime+getprime */
}
if (!parent) break;
if (getsec) {                       /* getsec is negative to mean to put
and thus number of parant’s to-stay
is decremented to make its getprime
larger in the result */
NOfPToStay[node->parentid] += getsec;
} else {                            /* getsec is 0 to mean the node has
some room to get secondaries */
parent->get.prime += room;
}
}
Verbose(2,vprint("try_stable=TRUE"));


Now we have confirmed that the helpand-helper configuration can be kept. In the confir-
mation process above, Nodes[n].{stay, get}.{prime, sec} have been set to the following
numbers of particles for each n ∈[0, N−1]

- stay.prime = Qnn is the number of currently staying in primary subdomain (excluding
secondary to primary movement).

- stay.sec = Qparent(n)n          is the number of currently staying in secondary subdomain
(exculding primary to secondary movement).

- get.prime = P nget is the minmum number to get from other nodes if positive, or the
reversed maximum number to put to helpers if negative.


<!-- Page 181 -->

- get.sec is the reversed number of secondary overflows if negative and thus Qgetn  , or
0 otherwise.

The elelemnt NOfPToStay[n] = Qstayn   has also been set to the number of particles in the
subdomain n, which can stay in the family nodes rooted by n excluding those overflown from
helpers. Now we perform a top-down (root-to-leaf) traversal of the family tree by scanning
NodeQueue[i] descendingly from NodeQueue[N−1] for the root, skipping leaf nodes. We
perform the followings for each non-leaf node Nodes[n] = NodeQueue[i].
(1) At the visit of n, Qgetn = Nodes[n].get.sec has been set to the real number of particles
the node should get (if positive) from other nodes or put (if negative) to its helpand
or sibling helpers. Thus we calculate the minimum number of particles which the
node n must accommodate, namely nproot = ρn by the following.

ρn = Nodes[n].stay.prime + Nodes[n].stay.sec + Nodes[n].get.sec
= Qnn + Qparent(n)n    + Qgetn = Qnn + Rn
Note that this number does not include positive Nodes[n].get.prime = P nget and thus
may be less than real must, but it is assured that ρn +P nget ≤Pmax by the calculation
of Qgetn  .
(2) ρn can be greater than Pmax with negative P nget .  This means that the node has
to put a number of particles namely ρn −Pmax which is assuredly not greater than
−P nget . Thus we set the reverse of this difference namely Rgetn = Pmax −ρn into
Nodes[n].get.prime to indicate the negative amount of particles to be gotten. We
also calculate floating = Rfltn by the following to have the number of floating particles
in the family including the movement from the node n to its chldren.

Rfltn = Pn −Qstayn  + ρn −Pmax
(  ∑                 )
= Pn − Qnn +   min(Qnm, Pmax −P mmin ) + (Qnn + Rn) −Pmax
m∈H(n)
( ∑  ∑       )  (  ∑                 )
=    Qnk +   Qnm + Qnn − Qnn +   min(Qnm, Pmax −P mmin )
k/∈F (n)  m∈H(n)               m∈H(n)
+ (Qnn + Rn −Pmax)
∑  ∑
=   Qnk +    max(0, Qnm + P mmin −Pmax) + (Qnn + Rn −Pmax)
k/∈F (n)  m∈H(n)
∑  ∑
=   Qnk +    max(0, Qnm + P mmin −Pmax) + max(0, Qnn + Rn −Pmax)
k/∈F (n)  m∈H(n)
(ρn = Qnn + Rn > Pmax)

We also do ρn ←Pmax to minimize the number of particle movement from the node
to its helpers.

(3) If ρn ≤Pmax, the node n has some room to get primary particles. In this case, the
number of floating particles Rfltn  is simply;
∑  ∑
Rfltn = Pn −Qstayn  =   Qnk +    max(0, Qnm + P mmin −Pmax)
k/∈F (n)  m∈H(n)
∑  ∑
=   Qnk +    max(0, Qnm + P mmin −Pmax) + max(0, Qnn + Rn −Pmax)
k/∈F (n)  m∈H(n)
(ρn = Qnn + Rn ≤Pmax)


<!-- Page 182 -->

and we initialize Rgetn = Nodes[n].get.prime = 0 because the node may not get
anything if it is heavily loaded and thus the oringinal Nodes[n].get.prime = P nget is
negative. In fact, in an extreme case with Rfltn = 0, we may skip the following because
Rgetn   is set to 0 correctly and it is assured that Qgetm  is 0 for any n’s helper nodes m.
(4) We call assign_particles() with ρn, positive Rfltn , the head of helpers of the node n
and the forth argument incgp = 0 meaing we try to keep helpers from putting their
own primary particles to grand-helpers. If it returns a positive number nptotal = Tn,
it successfuly found that floating particles can be assigned to kn lightly loaded nodes
in the family and the well balancing will be achieved by letting them have Tn particles
in total. Otherwise, we have to call assign particles() again setting incgp = 1
but keeping other arguments same to have Tn and kn allowing movements of helpers
to grand-helpers.

(5) Now we have the set of lightly loaded nodes Fl(n) ⊆F(n) in the family and kn =
|Fl(n)|. For each node m ∈F(n), we define its base-loads Bm as follows.
- Bn = ρn = min(Pmax, Qnn + Rn).
- If we called assign_particles() twice and thus incgp = 1, for all m ∈H(n);
Bm = Qmm + Qnm + min(0, Qgetm ) + Pmget
= (Qmm + P mget ) + Qnm + min(0, Pmax −(Pmmin + Qnm))
= Pmmin + min(Qnm, Pmax −P mmin )
= min(Qnm + P mmin , Pmax)

- If we called assign_particles() only once and thus incgp = 0;
  - For all m ∈{m ∈H(n) | P mget ≥0};
Bm = Qmm + Qnm + min(0, Qgetm ) + Pmget = min(Qnm + P mmin , Pmax)
  - For all min{m ∈H(n) | P mget < 0};
Bm = Qmm + Qnm + min(0, Qgetm ) = Qmm + min(Qnm, Pmax −P mmin )
Note that, since P mget < 0 means Qmm > P mmin , Bm = Qmm+Qnm if Bm ≤Pmax.

That is, Bm for m ∈H(n) is the sum of stay.prime, stay.sec and get.sec of
Nodes[m], possibly adding its get.prime  if positive or incgp = 1. Note that the
definitions above assure that a node m ∈F(n) really has room to have Pmax −Bm
particles if Bm < Pmax. We also know the followings.
∑
Tn = Rfltn +    Bl    ∀h /∈Fl(n) : Tn ≤knBh    Tn ≤knPmax
l∈Fl(n)
∀l ∈Fl(n) : Bl < Pmax

In other words, Tn/kn > Bl only for all l ∈Fl(n) and we can achieve a good balancing
by making the node l get particles of Tn/kn −Bl.
However, since Tn/kn is not necessary to be an integer, we have to make some rounding
as follows. Let npave = qa = ⌈(Tn/kn)−1⌉and npfac = qf = Tn−qakn which should
be in [1, kn]. Since Tn/kn > Bl is equivalent qa ≥Bl, we can find nodes in Fl(n) by
this condition.  Since some nodes, qf nodes more specfically, should have qa + 1
particles but it should be greater than neither any Bh such that h /∈Fl(n) nor Pmax,
we may allocate this slightly heavier load to arbitrary nodes in Fl(n) chosen by the
scanning order from the node n and through the sibling chain. Therefore, we let;


<!-- Page 183 -->

- Rgetn = Nodes[n].get.prime = qa + 1 −Bn if n ∈Fl(n)
- Qgetl = Nodes[l].get.sec = qa +1−Bl for first qf or qf −1 nodes l ∈Fl(n), and
- Qgetl = Nodes[l].get.sec = qa −Bl for remaining nodes l ∈Fl(n).
Now we have set Rgetn = Nodes[n].get.prime to be the real number of primary particles to
be gotten by (5) if it is lightly loaded, that to be put by (2) if overloaded, or 0 otherwise
by (3). We also have set Qgetm = Nodes[m].get.sec for all helper nodes m ∈H(n) to be
the real number of particles to be gotten by (5) if it is lightly loaded, that to be put by the
first bottom-up tree traveral if overloaded, or initial 0 unchanged otherwise.
Since each leaf node l has the exact number of primary particles to be gotten P lget in
Nodes[l].get.prime, and the root node r has of course no secondary particles to get and
thus Qgetr = Nodes[r].get.sec = 0 unchanged from its initial value, we have exact number
of primary/secondary particles to get/put to/from any node n in Nodes[n].get.prime and
Nodes[n].get.sec at the end of the traversal loop.

for (i=nn-1; i>=0; i--) {             /* top down traversal of node tree */
int nid, k, npfrac, incgp;
dint nproot, floating, nptotal, npave;
node = NodeQueue[i];
if (!(ch=node->child))  continue;   /* a leaf may reside below some non-
leaves in NodeQueue when its number
of primaries is equal to the
average */
nid = node->id;
floating = TotalPGlobal[nid] - NOfPToStay[nid];
/* # of transboundaries + overflows */
nproot = node->stay.prime + node->stay.sec + node->get.sec;
if (nproot>nlpmax) {                /* secondary assignment made primary
overflow */
dint getprime = nlpmax - nproot;  /* getprime<0 to mean to put */
node->get.prime = getprime;
floating -= getprime;
nproot = nlpmax;
} else {
node->get.prime = 0;
}
if (floating==0)  continue;
incgp = 0;
if ((nptotal=assign_particles(nproot, floating, ch, 0, &k))<0) {
/* try to avoid moving primaries of
children to their children */
incgp = 1;
if ((nptotal=assign_particles(nproot, floating, ch, 1, &k))<0)
/* allow moving primaries of
children to their children */
errstop("SECONDARY PARTICLE ASSIGNMENT STABILITY CHECK ERROR");
}
npave = nptotal / k;
npfrac = nptotal - npave*k;         /* should be faster than nptotal%k */
if (npfrac==0) {
npave--;  npfrac = k;
}                                   /* npave = ceil(average)-1 */
if (nproot<=npave) {


<!-- Page 184 -->

if (npfrac-- > 0)  nproot--;      /* get to have ceil(average) */
node->get.prime = npave - nproot;
}
for (ch=node->child; ch; ch=ch->sibling) {
int npch = ch->stay.prime + ch->stay.sec + ch->get.sec;
int gp = ch->get.prime;
if (gp>0 || incgp)  npch += gp;
if (npch<=npave) {
if (npfrac-- > 0)  npch--;
ch->get.sec = npave - npch;
}
}
}

Finally, if Special_Pexc_Sched() is true for the argument level meaning the caller of
try_stable1() has its own particle exchange scheduling mechanism, we simply return to
the caller with TRUE to indicate rebalancing is not necessary. Otherwise, we make particle
transfer schedule by calling schedule_particle_exchange() with an argument reb = 0
(or −1) if currmode = 1 (or 3) to mean floating partciles for a subdomain must be (are
not necessary to be) found only in the node responsible for the subdomain or its neighbors,
that is, we have normal (anywhere) accommodation. Then we call make_comm_count()
to build NOfRecv[][][] and NOfSend[][][] as the output of oh1_transbound() if level = 1,
or for non-neighboring particle transfers if currmode = 3. Otherwise, make_comm_count()
works to initialize TotalPNext[][] with the number of particles to be received. And then, we
return to the caller transbound1() or try_stable2() with TRUE to indicate rebalancing is
not necessary.

if (Special_Pexc_Sched(level)) return(TRUE);
schedule_particle_exchange(currmode==MODE_NORM_SEC ? 0 : -1);
make_comm_count(currmode, level, 0, Nodes[myRank].parentid, stats);
return(TRUE);
}


#### 4.3.14 count_stay()

count_stay()  The function count stay(), called only from try_stable1() without any arguments, cal-
culates the followings for all n ∈[0, N−1].

S−1∑
Qnn = Nodes[n].stay.prime =     q(n)[0][s][n]
s=0
S−1∑
Qparent(n)n    = Nodes[n].stay.sec =     q(n)[1][s][parent(n)]
s=0
S−1∑ (    ∑         )
NOfPToStay[n] =       q(n)[0][s][n] +     q(c)[1][s][n]
s=0              c∈H(n)


That is, Nodes[n].stay.{prime, sec} is set to the number of primary/secondary particles
currently accommodated by the node n, and NOfPToStay[n] is set to the sum of these
staying particles in the family of n, F(n).  The function also gives initial value 0 to
Nodes[n].get.{prime, sec}.


<!-- Page 185 -->

static void
count_stay() {
int nn=nOfNodes, ns=nOfSpecies, me=myRank;
int *np, *stay=TempArray;
struct S_node *node;
int i, s, sec;


For the calculation of Nodes[].stay.{prime, sec} and NOfPToStay[], first we make
∑S−1
TempArray[n] =   s=0 q(n)[0][s][n] for the local node n, and perform MPI_Allgather()
to have TempArray[m] for all m. Then TempArray[m] are copied to NOfPToStay[m] and
Nodes[m].stay.prime.

stay[me] = 0;
for (s=0,np=NOfPLocal; s<ns; s++,np+=nn)  stay[me] += np[me];
/* NOfPLocal[0][s][me] */
MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, stay, 1, MPI_INT, MCW);
for (i=0,node=Nodes; i<nn; i++,node++) {
node->stay.prime = NOfPToStay[i] = stay[i];
node->get.prime = 0;
}
∑S−1
Next, we make TempArray[n] =   s=0 q′(n)[1][s][parent(n)] for the local node n if it is
not the root of family tree, or TempArray[n] = 0 otherwise. Then and finally, we perform
MPI_Allgather() for TempArray[] again but this time gathered TempArray[m] is copied to
Nodes[m].stay.sec and added to NOfPToStay[parent(m)] if m is non-root.

sec = RegionId[1];
stay[me] = 0;
if (sec>=0)                                   /* np=&NOfPLocal[1][0][0] */
for (s=0; s<ns; s++,np+=nn)  stay[me] += np[sec];
/* NOfPLocal[1][s][me] */
MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, stay, 1, MPI_INT, MCW);
for (i=0,node=Nodes; i<nn; i++,node++) {
sec = node->parentid;
if (sec>=0)  NOfPToStay[sec] += (node->stay.sec = stay[i]);
else         node->stay.sec = 0;
node->get.sec = 0;
}
}


#### 4.3.15 assign_particles()

assign_particles()  This function, called only from try_stable1(), examines the load of the nodes in a helpand-
helper family, whose helpand n will have a number of particles specified by the argument
npr = ρn at least, and helpers are listed from the argument pointer ch to form H(n).
Then it finds the set of nodes, whose size is kn to be returned through the argument nget,
and to which we move a number of particles specified by the argument npt = Rfltn  in
the subdomain of the family from its member and other nodes to achieve a good tradeoff
between the load balancing and the communication cost reduction. More specifically, we
perform the followings.


<!-- Page 186 -->

First, we build a histogram of particle populations, or the base-load Bm for each member
m ∈F(n) as follows.

Bn = ρn = min(Pmax, Qnn + Rn)
{   get                      get
m   incgp = 1 ∨P m > 0   Bm = Qmm + Qnm + min(0, Qgetm ) +  P
0     otherwise
{
min(Pmax, Qnm + Pmmin )       incgp = 1 ∨P mget > 0     =                            min                     (m ∈H(n))        Qmm + min(Qnm, Pmax −Pm  )  othewise

where;

{Qmm, Qnm} = Nodes[m].stay.{prime, sec}
P mget = Nodes[m].get.prime     min(0, Qgetm ) = Nodes[m].get.sec

and incgp ∈{0, 1} is the argument given to the function.
With the definitions above, we prove if incgp = 1 and Rn ≤Pmax −Pnmin the following
is satisfied to assure the family has room to have Rfltn  particles in total.
∑
Rfltn +  Bm ≤Pmax|F(n)|
m∈F (n)

First we calulate the lefthand side of the inequality above.

max(0, Qnm + Pmmin −Pmax) + min(Pmax, Qnm + P mmin )
= (max(Pmax, Qnm + Pmmin ) −Pmax) + min(Pmax, Qnm + P mmin )
= (Pmax + (Qnm + Pmmin )) −Pmax = Qnm + P mmin
max(0, Qnn + Rn −Pmax) + min(Pmax, Qnn + Rn)
= Qnn + Rn
∑  ∑  ∑
Rfltn +  Bm =   Qnk +    max(0, Qnm + P mmin −Pmax) + max(0, Qnn + Rn −Pmax) +
m∈F (n)    k/∈F (n)  m∈H(n)
∑
min(Pmax, Qnm + Pmmin ) + min(Pmax, Qnn + Rn)
m∈H(n)
∑  ∑
=   Qnk +   (Qnm + Pmmin ) + Qnn + Rn
k/∈F (n)  m∈H(n)
∑
= Pn +   P mmin + Rn
m∈H(n)

Then the satisfaction of the inequality is proven as follows.
∑                     (  ∑
(  Bm + Rfltn ) −Pmax|F(n)| =  Pn +   Pmmin + Rn) −Pmax|F(n)|
m∈F (n)                           m∈H(n)
∑
= ((Pn + Rn) −Pmax) −   (Pmax −P mmin )
m∈H(n)
(  ∑           )
=  Pn −   (Pmax −P mmin ) + (Rn −Pmax)
m∈H(n)
∑
≤max(0, Pn −   (Pmax −P mmin )) + (Rn −Pmax)


<!-- Page 187 -->

= P nmin + (Rn −Pmax)
= Rn −(Pmax −P nmin ) ≤0
⇔Rn ≤Pmax −P nmin

Since Pmax ≥Pnmin is satsfied for all n and Rr = 0 for the root node r to assure Pmax −
Prmin ≥Rr = 0, if we can distribute Rfltn  particles in the members of F(n) keeping Rm ≤
Pmax −P mmin for all m ∈H(n) and keep resulting Qn not greater then Pmax, good load
balancing should be kept. In fact, the proven inequality assures that we have the following
subset F ′(n) of F(n), whose member m may receive at most r′m particles as a part of
distribution of Rfltn  particles.

F ′(n) = {m ∈F(n) | Bm < Pmax}      r′m = Pmax −Bm

Note that r′m is sufficient to cover Rfltn by its sum over the members in F ′(n), and Rm ≤
Pmax −P mmin for m ∈F ′(n) −{n} and Qn ≤Pmax for n ∈F ′(n) are satisfied as follows.
∑  ∑     ∑         ∑
r′m =   (Pmax −Bm) =   (Pmax −Bm) = Pmax|F(n)| −  Bm ≥Rfltn
m∈F ′(n)  m∈F ′(n)         m∈F (n)                    m∈F ′(n)
Rm ≤r′m + Qmn = (Pmax −Bm) + Qmn = Pmax −(Qmn + P mmin ) + Qmn = Pmax −P mmin
Qn ≤r′n + Bn = (Pmax −Bn) + Bn = Pmax

On the other hand, the required conditions of Rm and Qn are also satisfied for the members
in F(n) −F ′(n) as follows.

Rm = Qmn + min(0, Qgetn  ) = Qmn + Qgetn = Qmn + (Pmax −(P mmin + Qnm)) = Pmax −Pmmin
Qn = Qnn + Rn + P nget = Qnn + Rn + (Pmax −(Qnn + Rn)) = Pmax

Now we have assured existence of F ′(n) and;
∑              ∑
(Pmax −Bm) ≥Rfltn   or equivalently  Rfltn +  Bm ≤Pmax|F ′(n)|
m∈F ′(n)                               m∈F ′(n)

if incgp = 1. Therefore, we can find a subset Fl(n) of F ′(n) which satisfies;
∑
Rfltn +  Bm ≤Pmax|Fl(n)|
m∈Fl(n)

as follows. First we sort Bm in ascending order using TempArray[] as a temporary sorting
buffer and calling qsort() giving it the comparation funciont compare_int(), to have an
ascending sequence B′0, B′1, . . . , B′|F (n)|−1. Next we find minimum kn such that

kn−1∑
Rfltn +    B′i ≤knB′kn
i=0

or let kn = |F(n)|  if such kn does not exist, and let Fl(n) = {m | Bm < B′kn} where
B′|F (n)| = Pmax. Since Bm = Pmax for Bm ∈F(n) −F ′(n), it is assured that we can have
Fl(n) ⊂F(n) if F ′(n) ⊂F(n). Otherwise, i.e., if F ′(n) = F(n), it can be Fl(n) = F(n)
but is all right because it means Bm < Pmax for all m ∈F(n). This process is to find kn
nodes having small number of particles and to make them get some particles for resulting
loads balanced among them, while other heavily loaded nodes will not get any particles


<!-- Page 188 -->

(but may put some of them out if it is necessary not for load balancing but for keeping the
helpand-helper configuration).
On the other hand, If incgp = 0 this particle assignnment could make load overflow
resulting;
kn−1∑
Rfltn +    B′i > knPmax
i=0

(kn may be less than |F(n)|) because we make overesitimation for Bm neglecting negative
Pmget , and thus find we don’t have room enough to accommodate Rfltn  particles without
pushing helper’s primary particles down to their grand-helpers.  In this case we return
−1 to report the failure so that the caller try_stable1() call this function again with
incgp = 1. Otherwise we return

k−1∑    ∑
Tn = Rn +    B′i = Rn +  Bm
i=0          m∈Fl(n)

to report that its average for kn nodes is the target of the number of particles which lightly
loaded nodes will have in total. We also return the value of kn through the pointer argument
nget to the caller.


static dint
assign_particles(dint npr, dint npt, struct S_node *ch, int incgp, int *nget) {
int *np=TempArray;            /* used just for temporary sorting buffer */
int n, i;
dint nlpmax = nOfLocalPMax;

np[0] = npr;
for (n=1; ch; ch=ch->sibling, n++) {
int gp=ch->get.prime;
np[n] = ch->stay.prime + ch->stay.sec + ch->get.sec;
if (gp>0 || incgp)  np[n] += gp;
}
qsort(np, n, sizeof(int), compare_int);       /* sort ascendingly */
for (i=0; i<n; i++) {
dint npc=np[i];
if (npt<=npc*i)  break;
npt += npc;
}
*nget = i;
return(npt>nlpmax*i ? -1 : npt);
}


#### 4.3.16 compare_int()

compare_int()  The function compare int(), called from qsort() in assign_particles() with two void
poiter arguments x and y, returns the following r for the comparison of an integer pair X
and Y pointed by the arguments in the array to be sorted.
{
−1 X < Y
r =  0  X = Y
1  X > Y
