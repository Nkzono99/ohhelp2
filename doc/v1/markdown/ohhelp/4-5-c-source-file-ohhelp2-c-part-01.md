# 4.5 C Source File ohhelp2.c - Part 1

Source: `doc/v1/original/ohhelp.pdf`, pages 234-265.

<!-- Page 234 -->

## 4.5 C Source File ohhelp2.c

#### 4.5.1 Header File Inclusion

The first job done in ohhelp2.c is the inclusion of the header files ohhelp1.h and ohhelp2.h.
Before the inclusion of ohhelp1.h, we #define the macro EXTERN as extern so as to make
variables declared in the file external, but after that we make it #undef’iend and then
#define it as empty so as to provide variables declared in ohhelp2.h with their homes, as
discussed in §4.2.3.

#define EXTERN extern
#include "ohhelp1.h"
#undef  EXTERN
#define EXTERN
#include "ohhelp2.h"



#### 4.5.2 Function Prototypes

The next and last job to do prior to function definitions is to declare the prototypes of the
following functions private for the level-2 library.

- The function try_primary2() performs the particle transfer in primary mode after
calling its level-1 couterpart try_primary1() to check if we will be in primary mode
in the next step.

- The function try_stable2() performs the particle transfer in secondary mode after
calling its level-1 couterpart try_stable1() to check  if we can keep the helpand-
helper configuration.

- The function rebalance2() performs the particle transfer in secondary mode after
calling its level-1 counterpart rebalance1() to establish a new helpand-helper con-
figuration.

- The function move_to_sendbuf_secondary() moves particles to be transferred from
Particles[] to SendBuf[] and packs those remaining in Particles[] in secondary
mode.

- The functions move_to_sendbuf_uw() and move_to_sendbuf_dw() move particles
to be transferred from Particles[] to SendBuf[] and packs those remaining in
Particles[] for a block pbuf (p, s) whose region is shifted upward and downward,
i.e. to the direction of smaller/greater addresses, respectively.

- The function move_injected_to_sendbuf() moves injected particles from the bot-
tom of Particles[] to SendBuf[].

- The function move_injected_from_sendbuf() moves particles injected into the pri-
mary/secondary subdomain of the local node from SendBuf[] back to Particles[].

- The function receive_particles() performs receive (and send in special cases) com-
munication of particle transfer.

- The funciton send_particles() performs send communication of particle transfer.


<!-- Page 235 -->

/* Prototypes for private functions. */
static int   try_primary2(int currmode, int level, int stats);
static int   try_stable2(int currmode, int level, int stats);
static void  rebalance2(int currmode, int level, int stats);
static void  move_to_sendbuf_secondary(int secondary, int stats);
static void  move_to_sendbuf_uw(int ps, int me, int *putmes, int cbase,
int *ctp, int nbase, int *ntp,
struct S_particle **rbb);
static void  move_to_sendbuf_dw(int ps, int me, int *putmes, int ctail,
int *ctp, int ntail, int *ntp);
static void  move_injected_to_sendbuf();
static void  move_injected_from_sendbuf(int *injected, int mysd,
struct S_particle **rbb);
static void  receive_particles(struct S_commlist *rlist, int rlsize, int *req);
static void  send_particles(struct S_commlist *slist, int slsize, int myregion,
int parentregion, int *req);



#### 4.5.3 oh2_init() and init2()

oh2_init_()  The API functions oh2 init () for Fortran and oh2 init() for C receive a set of ar-
oh2_init()  ray/structure variables through which level-1 and level-2 library functions communicate
with the simulator body, and a few integer parameters to specify the behavior of the li-
brary. The functions have the following arguments.

- sdid
nspec
maxfrac
nphgram
totalp
The five arguments above are perfectly equivalent to those of the level-1 countgerparts
oh1_init[_]().

- The argument pbuf should be the (double) pointer to an arrray of [Plim] of S_
particle type particles which is referred to as Particles[] in the library functions.
The array itself is allocated by init2() if pbuf points NULL.

- The argument pbase should be the (double) pointer to an array of three elements
having displacements of the regions in Particles[]. The first element pbase[0] is the
displacement of the head of the first part of Particles[] for primary particles and
thus is always zero. The second element pbase[1] is the displacement of the head
of its second part for secondary particles thus is equal to the number of primary
particles Qnn for the local node n. The third element pbase[] is the displacement of
the unused part of Particles[] and thus is equal to the total number of particles
Qn the local node n accommodates. Therefore, pbase[1] and pbase[2] is pointed by
secondaryBase and totalLocalParticles so as to make these elements the shadows
of primaryParts and totalParts.

- The argument maxlocalp should have the value of Plim being the size of Particles[]
and SendBuf[].

- mycomm


<!-- Page 236 -->

nbor
pcoord
stats
repiter
verbose
The six arguments above are perfectly equivalent to those of the level-1 countgerparts
oh1_init[_]().

Note that oh2_init[_]() does not have arguments rcounts and scounts which oh1_
init[_]() has, because we do not need to report the transfer schedule.
The API functions almost simply call init2() passing all given arguments to it except
for the followings.

- oh2 init () passes the pointers to sdid, nphgram, totalp, pbuf, pbase and nbor
rather than themselves.

- oh2 init () passes mycomm to mycommf of init2() while NULL is passed through
mycommc of init2() to keep it from allocation of MyCommC.

- oh2 init() passes mycomm to mycommc of init2() while NULL  is passed through
mycommf of init2() telling it that the body of MyCommF is not required.  It also
casts the argument as S_mycommc pointer type, because mycomm is declared as a void
pointer to allow the simulator body to be completely unaware of the structure.

- Prior to calling init2(), oh2 init () lets specBase be 1 to indicate the species in
S_particle structures is represented by one-origin manner, while oh2 init() lets it
be 0 to indicate zero-origin numbering.


void
oh2_init_(int *sdid, int *nspec, int *maxfrac, int *nphgram,
int *totalp, struct S_particle *pbuf, int *pbase, int *maxlocalp,
struct S_mycommf *mycomm, int *nbor, int *pcoord,
int *stats, int *repiter, int *verbose) {
specBase = 1;
init2(&sdid, *nspec, *maxfrac, &nphgram, &totalp,
&pbuf, &pbase, *maxlocalp, NULL, mycomm, &nbor, pcoord,
*stats, *repiter, *verbose);
}
void
oh2_init(int **sdid, int nspec, int maxfrac, int **nphgram,
int **totalp, struct S_particle **pbuf, int **pbase, int maxlocalp,
void *mycomm, int **nbor, int *pcoord,
int stats, int repiter, int verbose) {
specBase = 0;
init2(sdid, nspec, maxfrac, nphgram, totalp,
pbuf, pbase, maxlocalp, (struct S_mycommc*)mycomm, NULL, nbor, pcoord,
stats, repiter, verbose);
}

init2()  The function init2() implements the initialization for its caller API functions, oh2_init_
() and oh2_init(), or part of that for its higher level counterpart init3(). The arguments
of this function are almost same as those of oh2_init() but its mycomm is split into two
arguments mycommc and mycommf, which are NULL if called from oh2_init_() or oh2_init()
respectively.


<!-- Page 237 -->

void
init2(int **sdid, int nspec, int maxfrac, int **nphgram,
int **totalp, struct S_particle **pbuf, int **pbase, int maxlocalp,
struct S_mycommc *mycommc, struct S_mycommf *mycommf,
int **nbor, int *pcoord, int stats, int repiter, int verbose) {
int ns, nn, nnns, s;


At first the function calls its level-1 counterpart init1() to initialize data structures specific
to level-1 and common to level-2. The arguments passed to init1() are almost simply those
given to init2() but NULL is passed to rcounts and scounts because level-2 library does
not show the particle transfer schedule to the simulator body.

init1(sdid, nspec, maxfrac, nphgram, totalp, NULL, NULL,
mycommc, mycommf, nbor, pcoord, stats, repiter, verbose);


Then, after obtaining S = nOfSpecies and N = nOfNodes, we set maxlocalp into Plim =
nOfLocalPLimit and allocate Particles[Plim] by mem_alloc() to set its base pointer into
*pbuf if it was NULL, while the pointer to the array allocated by the simulator body is
simply set into Particles otherwise.  In addition, we initialize totalParts = Plim so
that, prior to the first call of oh2_transbound() (or one of its higher-level counterparts) or
oh2_set_total_particles(), any injections are judged causing particle buffer overflow,
and level-4p functions such as oh4p_map_particle_to_neighbor() correctly judge their
argument particles are not injected one.

ns = nOfSpecies;  nn = nOfNodes;  nnns = nn * ns;

nOfLocalPLimit = totalParts = maxlocalp;
if (*pbuf)
Particles = *pbuf;
else
Particles = *pbuf =
(struct S_particle*)mem_alloc(sizeof(struct S_particle),
maxlocalp, "Particles");


Next we define the MPI data-type for a S_particle structure, which is simply a MPI_BYTE
sequence of sizeof(struct␣S_particle), by MPI_Type_contiguous()43, and commit its
use by MPI_Type_commit().

MPI_Type_contiguous(sizeof(struct S_particle), MPI_BYTE, &T_Particle);
MPI_Type_commit(&T_Particle);


We continue the conditional allocation of the interface array *pbase with mem_alloc(), and
then clear all of its three elements with zero to assume that the local node has no particles at
initial. We also let secondaryBase and totalLocalParticles point the elements *pbase[1]
and *pbase[2] respecitively to let library functions know where these shadow variables are.


if (!*pbase)  *pbase = (int*)mem_alloc(sizeof(int), 3, "ParticleBase");

43Because we ignore endian problem which could arise if an OhHelp’ed simulator were executed on a
heterogeneous parallel system.


<!-- Page 238 -->

(*pbase)[0] = (*pbase)[1] = (*pbase)[2] = 0;
secondaryBase = *pbase + 1;  totalLocalParticles = *pbase + 2;


Finally we  allocate  the  following  library’s own  global  variables by  mem_alloc();
SendBuf[Plim] unless OH_POS_AWARE is defined to mean it is allocated by level-4p initializer
init4p(), RecvBufBases[2][S]44, SendBufDisps[S][N], and RecvBufDisps[N]. We also al-
locate Requests[4SN + 2 · 3D] and Statuses[4SN + 2 · 3D]. Note that the size is larger by
2 · 3D than 4SN discussed in §4.4.2, because that if we have position-aware particle man-
agement we could need have 4N + 2 · 3D entries for them as discussed in §4.9.4. We also
initialize nOfInjections = Qinjn  clearling it with zero to indicate no particles are injected.


#ifndef OH_POS_AWARE
SendBuf = (struct S_particle*)mem_alloc(sizeof(struct S_particle), maxlocalp,
"SendBuf");
#endif
RecvBufBases = (struct S_particle**)mem_alloc(sizeof(struct S_particle*),
2*ns+1, "RecvBufBases");
SendBufDisps = (int*)mem_alloc(sizeof(int),  nnns, "SendBufDisps");
RecvBufDisps = (int*)mem_alloc(sizeof(int),  nn, "RecvBufDisps");
nOfInjections = 0;

Requests = (MPI_Request*)mem_alloc(sizeof(MPI_Request),
nnns*4+OH_NEIGHBORS*2, "Requests");
Statuses = (MPI_Status*) mem_alloc(sizeof(MPI_Status),
nnns*4+OH_NEIGHBORS*2, "Statuses");
}


#### 4.5.4 oh2_transbound() and transbound2()

oh2_transbound_()  The API functions oh2 transbound () for Fortran and oh2 transbound() for C provide
oh2_transbound()  a simulator body calling them with the core mechanism of level-2 library. The meanings
of their two arguments, currmode and stats, and return value in {−1, 0, 1} are perfectly
equivalent to those of the level-1 counterparts oh1_transbound[_](). Also similarly to the
counterparts, their bodies only have a simple call of transbound2() but the third argument
level is 2 to indicate the function is called from level-2 API functions.


int
oh2_transbound_(int *currmode, int *stats) {
return(transbound2(*currmode, *stats, 2));
}
int
oh2_transbound(int currmode, int stats) {
return(transbound2(currmode, stats, 2));
}


transbound2()  The function transbound2(), called from oh2_transbound_(), oh2_transbound() or
the level-3 counterpart transbound3(), first calls its level-1 counterpart transbound1()
to calculate NOfPrimaries[][], TotalPGlobal[], nOfParticles and nOfLocalPMax from

44It has one extra element [2][0].


<!-- Page 239 -->

NOfPLocal[][][] of the local node and other nodes, and to have currmode which indicates
not only the current execution mode but also the accommodation mode, i.e., normal or
anywhere. The function also allocates and calculates TotalP[][] from NOfPLocal[][][] at the
first call of it (and thus of transbound2()) and let primaryParts and totalParts have
the number of particles the local node accommodates, i.e., the sum of TotalP[][].


int
transbound2(int currmode, int stats, int level) {
int ret=MODE_NORM_SEC, nn=nOfNodes, ns=nOfSpecies, nnns2=2*nn*ns;
int i, s, tp;

stats = stats && statsMode;
currmode = transbound1(currmode, stats, level);


The next part being the heart  of balancing examination  is very similar to that  of
transbound1() but the functions called in it are level-2’s ones, try_primary2(), try_
stable2() and rebalance2() which perform particle transfer in addition to the schedul-
ing of it.

if (try_primary2(currmode, level, stats))  ret = MODE_NORM_PRI;
else if (!Mode_PS(currmode) || !try_stable2(currmode, level, stats)) {
rebalance2(currmode, level, stats);  ret = MODE_REB_SEC;
}

Finally, also as done in transbound1(), we clear NOfPLocal[][][] and copy TotalPNext[][] to
its substance TotalP[][]. In addition, we also clear InjectedParticles[0][][] = qinj(n) and
nOfInjections = Qinjn  to indicate we have no injected particles at the beginning of the next
simulation step, and set totalParts and its shadow pointed by totalLocalParticles to
the sum of TotalP[p][s] for all p ∈[0, 1] and s ∈[0, S−1] to memorize the total number
of particles the local node accommodates at the beginning of the next simulation step,
i.e., before any injections and removals. Then we return to the simulator body with the
return value defined in §4.3.10 for transbound1(), setting it to currMode also as done in
transbound1().

for (i=0; i<nnns2; i++) NOfPLocal[i] = 0;
for (s=0,tp=0; s<ns*2; s++) {
TotalP[s] = TotalPNext[s];  tp += TotalPNext[s];
}
for (s=0; s<ns*2; s++)  InjectedParticles[s] = 0;
totalParts = *totalLocalParticles = tp;  nOfInjections = 0;
return((currMode=ret));
}


#### 4.5.5 try_primary2()

try_primary2()  The function try primary2(), called solely from transbound2(), examines if we can stay
in or turn to primary mode.  If so, the local node gathers all the particles in its primary
subdomain from other nodes. The function has three arguments currmode, level and
stats whose meanings are perfectly equivalent to those of its level-1 counterpart try_
primary1().
First we call the level-1 counterpart try_primary1() to examine if the next execution
mode is primary. If not, we simply return to its caller transbound2() with the return value


<!-- Page 240 -->

FALSE to indicate the mode will be secondary. Then we start particle transfer at first calling
move_to_sendbuf_primary() which moves the particles outside of the primary subdomain
of the local node and thus to be sent to other nodes, from the particle buffer Particles[]
to the send buffer SendBuf[], while primary particles are packed in Particles[] to form
pbuf (0, s) for each species s together with the receive buffer rbuf (0, s) for s located at the
head or tail of pbuf (0, s).  It also sets SendBufDisps[s][m] to be the displacement of the
block sbuf (s, m) from the head of SendBuf[], and RecvBufBases[0][s] to point rbuf (0, s).
Then we call exchange_primary_particles() to send particles in SendBuf[] to other
nodes and to receive particles into rbuf (0, s) in Particles[] from other nodes.
Finally we finish this function and return to transbound2() with TRUE to tell it we will
be in primary mode in the next simulation step, after setting primaryParts and its shadow
pointed by secondaryBase to the total number of particles the local node n accommodates,
i.e., the number of particles in the primary subdomain, TotalPGlobal[n].


static int
try_primary2(int currmode, int level, int stats) {

if (!try_primary1(currmode, level, stats))  return(FALSE);
move_to_sendbuf_primary(Mode_PS(currmode), stats);
exchange_primary_particles(currmode, stats);
primaryParts = *secondaryBase = TotalPGlobal[myRank];
return(TRUE);
}


#### 4.5.6 exchange_primary_particles()

exchange_primary_particles()  The function exchange_primary_particles(), called from try_primary2() and some
functions of level-4 or higher, sends and receives particles to/from other nodes when we
will be in primary mode in the next simulation step. The particles of species s to be sent
to the node m are in sbuf (s, m) of SendBuf[] pointed by SendBufDisps[s][m], while those
to be received are in rbuf (0, s) of Particles[] pointed by RecvBufBases[0][s].


void
exchange_primary_particles(int currmode, int stats) {
int i, s, nn=nOfNodes, ns=nOfSpecies, nnns=nn*ns, me=myRank;
int *np, *rnp, *sbd;


We perform the particle transfer communications, after starting timing measurement
of this process by oh1_stats_time() with the key STATS_TB_COMM.  If we are already in
primary mode and the accommodation mode is normal, all the particles the local node has to
receive should be found in its neighborinhg nodes and those the local node has to send should
be destined for the neighboring nodes too. Thus, for each s ∈[0, S−1], we pick neigboring
nodes nd(k) = DstNeighbors[k] and their oppositional ones ns(k) = SrcNeighbors[k] for
all k ∈[0, 3D−1] so that the local node sends its accommodating particles of species s in
the subdomain nd(k) to the node nd(k) simultaneously receiving its primary ones of species
s accommodated by the node ns(k) from it by MPI_Sendrecv(). More accurately, the local
node n performs neighboring communication exactly one send and receive for each neighbor
node as follows.


<!-- Page 241 -->

1. If nd(k) = n, it must be ns(k) = n by definition and symmetricity of the self neigh-
boring. Thus we skip the k-th neighbor to avoid self communication.

2. If nd(k) ≥0 and ns(k) ≥0 meaning they are first appearence in the neighbor arrays,
particles are transferred by MPI_Sendrecv().

3. If either nd(k) < 0 or ns(k) < 0, particles are transferred by a one-way communica-
tion MPI_Send() or MPI_Recv() respectively, because we have already sent/received
particles to/from the node correspoinding to nd(k) < 0 or ns(k) < 0. This situation
may occur if the neighboring configuration is explicitly given by the simulator body
through the argument nbor of oh1_init() (and thus oh2_init()).

4. Otherwise, i.e., nd(k) < 0 and ns(k) < 0, we skip the communication for k-th neigh-
bors because we have already performed it.

Note that since it is assured that the local node n is nd(k) (ns(k)) of the node ns(k) (nd(k)),
the blocking communications should be safely performed without deadlock.
As for the arguments of the communication functions, we give the followings for the
local node n and a receiver dest = nd(k) and a sender source = ns(k).

- sendbuf is the pointer to sbuf (s, nd(k)) and thus SendBuf + SendBufDisps[s][nd(k)].

- sendcount is the number of particles of species s in the subdomain nd(k) accom-
modated by the local node n as  its primary ones and thus q(n)[0][s][nd(k)] =
NOfPLocal[0][s][nd(k)].

- recvbuf is the pointer to the receive buffer rbuf (0, s)/ns(k) for the particles of species
s receiving from the node ns(k) and thus;
∑
RecvBufBases[0][s] +         q(ns(i))[0][s][n]

i<k
ns(i)≥0
∑
= RecvBufBases[0][s] +        NOfPrimaries[0][s][ns(i)]

i<k
ns(i)≥0

- recvcount is the number of particles of species s in the subdomain n accommo-
dated by the node ns(k) and thus q(ns(k))[0][s][n] = NOfPrimaries[0][s][ns(k)].
Therefore, the displacement of the receive buffer rbuf (0, s)/ns(k) from the top of
rbuf (0, s) pointed by RecvBufBases[0][s] is obtained by summing up the values given
to recvcount as shown above.

- sendtype and recvtype are commmonly T_Particle.

- sendtag and recvtag are commonly zero because we have no reason to distinguish
the communications each other.

Note that we do not skip the communication even if sendcount or recvcount is zero to avoid
coding complication, although skipping could give small but non-negligible performance
improvement and should be safely implemented.

if (stats) oh1_stats_time(STATS_TB_COMM, 0);
np = NOfPLocal;                       /* &NOfPLocal[0][0][0] */
rnp = NOfPrimaries;                   /* &NOfPrimaries[0][0][0] */


<!-- Page 242 -->

sbd = SendBufDisps;                   /* SendBufDisps[0][0] */
if (currmode==MODE_NORM_PRI) {
for (s=0; s<ns; s++,np+=nn,rnp+=nn,sbd+=nn) {
/* np=&NOfPLocal[0][s][0] */
/* rnp=&NOfPrimaries[0][s][0] */
/* sbd=&SendBufDisps[s][0] */
struct S_particle *rb;
rb = RecvBufBases[s];             /* RecvBufBases[0][s] */
for (i=0; i<OH_NEIGHBORS; i++) {
int dst=DstNeighbors[i];
int src=SrcNeighbors[i];
int rc;
MPI_Status st;
if (dst==me) continue;
if (src>=0) {
rc = rnp[src];                /* NOfPrimaries[0][s][src] */
if (dst>=0)
MPI_Sendrecv(SendBuf+sbd[dst], np[dst], T_Particle, dst, 0,
rb, rc, T_Particle, src, 0, MCW, &st);
else
MPI_Recv(rb, rc, T_Particle, src, 0, MCW, &st);
rb += rc;
} else if (dst>=0)
MPI_Send(SendBuf+sbd[dst], np[dst], T_Particle, dst, 0, MCW);
}
}
On the other hand, if the mode has turned from secondary to primary or the accommodation
mode is anywhere, some particles in the primary subdomain of the local node n can be found
in any nodes. Therefore, for each species s, we perform an all-to-all communication by MPI_
Alltoallv() with the following arguments.

- sendbuf is always SendBuf regardless of s because we specify each of sbuf (s, m) for
the node m by sdispls = SendBufDisps[].

- sendcounts is NOfPLocal[0][s][] but each element of it is incremented by the sec-
ondary counterpart NOfPLocal[1][s][] so that it has q(n)[0][s][m] + q(n)[1][s][m] for
all m ∈[0, N−1] in case of we are in secondary mode.  Note that the local
node’s own NOfPLocal[0][s][n] is set to zero by move_to_sendbuf_primary() but
NOfPLocal[1][s][n] is not to move particles which resides in the subdomain n and was
secondary from SendBuf[] back to Particles[] by the self communication taken in
MPI_Alltoallv().

- sdispls is SendBufDisps[s][] to specify sbuf (s, m) for each node m.

- recvbuf  is  RecvBufBases[0][s]  which  points  the  receive  buffer  rbuf (0, s)  in
Particles[] for s.

- recvcounts is TempArray[] whose element [m] has the following to represent the
number of particles of species s in the subdomain n currently accommodated by the
node m, excepting n’s own primary particles45.
{
q(m)[1][s][n]         m = n
TempArray[m] =
q(m)[0][s][n] + q(m)[1][s][n] m ̸= n

45Instead of TempArray[], we may use NOfPrimaries[0][s][] destructively adding its secondary counterparts
to it, but dare to use TempArray[] by some historical reason of the implementaion.


<!-- Page 243 -->

{
NOfPrimaries[1][s][m]              m = n
=
NOfPrimaries[0][s][m] + NOfPrimaries[1][s][m] m ̸= n

∑         ∑
- rdispls  is RecvBufDisps[] whose element m has   i<m TempArray[m] =   i<m
rcounts[m] for the block rbuf (0, s)/m in the receive buffer rbuf (0, s) to which we
receive the particles from the node m.

- sendtype and recvtype are commmonly T_Particle.


} else {
for (s=0; s<ns; s++,np+=nn,rnp+=nn,sbd+=nn) {
/* np=&NOfPLocal[0][s][0] */
/* sbd=&SendBufDisps[s][0] */
/* rnp=&NOfPrimaries[0][s][0] */
int rdisp=0;
rnp[me] = 0;                      /* &NOfPrimaries[0][s][me] */
for (i=0; i<nn; i++) {
int rc = rnp[i] + rnp[i+nnns];
/* NOfPrimaries[0][s][i]+ NofPrimaries[1][s][i] */
TempArray[i] = rc;
RecvBufDisps[i] = rdisp;
rdisp += rc;
np[i] += np[i+nnns];            /* += NOfPLocal[1][s][i] */
}
MPI_Alltoallv(SendBuf, np, sbd, T_Particle,
RecvBufBases[s], TempArray, RecvBufDisps, T_Particle, MCW);
}
}
}


#### 4.5.7 try_stable2()

try_stable2()  The function try stable2(), solely called from transbound2(), examines if the current
helpand-helper configuration sustains the particle movements crossing subdomain bound-
aries which can bring intolerable load imbalance. The examination is done by calling its
level-1 counterpart try_stable1() simply passing all the arguments of the function itself to
the counterpart, because the meanings of them are perfectly equivalent to those of the coun-
terpart. If the examination passes, we perform an all-to-all type particle transfer by calling
exchange_particles() with the following arguments, before returning to transbound2()
with the return value of TRUE.

- secrlist  is the pointer to the secondary receiving block and thus CommList +
SLHeadTail[1] because helpand-helper configuration has been kept.  Similarly, the
size of the block secrlsize is given by SecSLHeadTail[0].

- oldparent  is the current helpand  of the  local node n and thus parent(n) =
Nodes[n].parentid.

- neighboring is true if and only if the argument currmode of the function is equal to
MODE_NORM_SEC indicating normal accommodation.

- currmode and stats are simply those passed to the function.


<!-- Page 244 -->

static int
try_stable2(int currmode, int level, int stats) {

if (!try_stable1(currmode, level, stats)) return(FALSE);
exchange_particles(CommList+SLHeadTail[1], SecSLHeadTail[0],
Nodes[myRank].parentid, currmode==MODE_NORM_SEC,
currmode, stats);
return(TRUE);
}


#### 4.5.8 rebalance2()

rebalance2()  The function rebalance2(), solely called from transbound2(), builds the new family tree
to rebalance the load among nodes by calling its level-1 counterpart rebalance1() simply
passing all the arguments of the function itself to the counterpart, because the meanings
of them are perfectly equivalent to those of the counterpart.  Then, before the particle
transfer by exchange_particles(), it clears InjectedParticles[0][1][] = qinj(n)[1][]  if
some particles are injected (nOfInjections = Qinjn > 0), the local node n had a parent,
and the old parent and new one are different, because particles injected into old secondary
subdomain are simply slown away to the old parent or its new children. Note that the
particles injected into the new secondary subdomain accidentally are regarded as primary
particles and thus they are transferred from the primary subdomain to secondary one.
Then it performs an all-to-all type particle transfer by calling exchange_particles()
with the following arguments.

- secrlist and secrlsize are SecRList and SecRLSize which are set to the head and
size of secondary receiving or alternative secondary receiving block broadcasted from
the new helpand by make_comm_count() called in rebalance1().

- oldparent is the helpand of the local node n in the old helpand-helper configuration
if we are in secondary mode with normal accommodation.  That  is,  if currmode
argument of the function is MODE_NORM_SEC, oldparent is NodesNext[n].parentid
because the old configuration is kept in NodesNext[] by rebalance1(). Otherwise,
oldparent is −1 to indicate no information was given from the old helpand because
we are in primary mode or with anywhere accommodataion.

- neighboring is true if and only if Mode_Is_Norm() for the argument currmode of
the function is true indicating normal accommodation.

- currmode and stats are simply those passed to the function.



static void
rebalance2(int currmode, int level, int stats) {
int me=myRank, ns=nOfSpecies, s, oldp, newp;

rebalance1(currmode, level, stats);
oldp = NodesNext[me].parentid;  newp=Nodes[me].parentid;
if (nOfInjections && oldp>=0 && oldp!=newp)
for (s=0; s<ns; s++)  InjectedParticles[ns+s] = 0;
if (Mode_Is_Norm(currmode))


<!-- Page 245 -->

exchange_particles(SecRList, SecRLSize,
Mode_PS(currmode) ? oldp : -1,
1, currmode, stats);
else
exchange_particles(SecRList, SecRLSize, -1, 0, currmode, stats);
}


#### 4.5.9 move_to_sendbuf_primary()

move_to_sendbuf_primary()  The function move to sendbuf primary(), called from try_primary2() and some library
functions of level-4 or higher, moves particles not residing in the primary subdomain of
the local node from the particle buffer Particles[] to the send buffer SendBuf[]. The
particles residing in the primary subdomain are also moved in Particles[] so that they are
contiguously packed in each pbuf (0, s) for each species s ∈[0, S−1], each pbuf (0, s) has a
receive buffer rbuf (0, s) of an appropriate size at its head or tail, and all pbuf (0, s) are also
contiguously aligned. The function also takes care of particles injected by oh2_inject_
particle() or its higher level counterparts and located at the tail of Particles[], by at
first moving them to SendBuf[] regardless of their residing subdomains and then moving
primary ones from SendBuf[] back to Particles[]. The function is also responsible to let
SendBufDisps[s][m], RecvBufBases[0][s] and TotalPNext[0][s] have appropriate values for
all s ∈[0, S−1] and m ∈[0, N−1].
The function is given two arguments, secondary being true (1) if and only if we were
in the secondary mode in the last step, and stats being true (non-zero) if and only if the
execution time spent in the process to move particles has to be measured.


void
move_to_sendbuf_primary(int secondary, int stats) {
int me=myRank, ns=nOfSpecies, nn=nOfNodes, nnns=nn*ns;
int s, i, j, *pp;


First we call oh1_stats_time() with the key STATS_TB_MOVE to measure the time
spent in the function  if the argument stats is true.  Then, for each s ∈[0, S−1], we
clear NOfPLocal[0][s][n] for the local node n to indicate primary particles of the node
are not moved to SendBuf[] but stay in Particles[]. Note that we keep NOfPLocal[1][s][n]
unchanged so that secondary particles in the primary subdomain, which are accommodated
by the local node and incidentally moved from the subdomain which the local node was
responsible for as its secondary subdomain in the last step, are moved to SendBuf[] and
then sent to the local node itself.
We also set TotalPNext[0][s] as follows;

∑ N−1∑        ∑ N−1∑
TotalPNext[0][s] =           NOfPrimaries[p][s][m] =            q(m)[p][s][n]
p∈{0,1} m=0                            p∈{0,1} m=0

to indicate all the particles of species s in the primary subdomain of the local node is
accommodated by the node, while TotalPNext[1][s] is cleared with 0 because we will have
no secondary particles.

if (stats) oh1_stats_time(STATS_TB_MOVE, 0);
for (s=0,i=me,pp=NOfPrimaries; s<ns; s++,i+=nn,pp+=nn) {
int t = 0;


<!-- Page 246 -->

NOfPLocal[i] = 0;                           /* NOfPLocal[0][s][me] */
for (j=0; j<nn; j++) t += pp[j] + pp[j+nnns];
/* NOfPrimaries[0][s][j] + NOfPrimaries[1][s][j] */
TotalPNext[s] = t;                          /* TotalPNext[0][s] */
TotalPNext[ns+s] = 0;                       /* TotalPNext[1][s] */
}

Next, we call set_sendbuf_disps(), with the first argument secondary and second −1
meaning the local node does not have parent, to calculate values of SendBufDisps[S][N].
We then call move_injected_to_sendbuf() to move injected particles to SendBuf[] if we
have any of them, i.e., nOfInjections = Qinjn > 0.

set_sendbuf_disps(secondary, -1);
if (nOfInjections)  move_injected_to_sendbuf();


Now we move the primary particles to SendBuf[] or pack them in Particles[] by calling
move_to_sendbuf_uw() for the species whose block pbuf (0, s) will not have head and tail
addresses larger than their current ones and thus have some particles moving upward, or
toward smaller addresses. That is, these blocks can be scanned in ascending order and be
packed safely without any hazard to destroy the contents of other (following) blocks. On
the other hand, the blocks which will have head and tail addresses larger than their current
ones will be moved downward and thus are skipped by the function because they should be
scanned in descending order after we process all of the upward blocks.
The arguments to be given to the function are as follows.

- ps is 0 to scan primary particles.

- me is the rank of the local node n to identify the primary particles in the next step.

- putmes is the slice NOfPLocal[0][S][n] whose elements have been zero-cleared to indi-
cates no particles in the primary subdomain are sent.

- cbase is 0 to tell the function to start the scan from the head of Particles[].

- ctp is TotalP[0][S] to show the size of the current pbuf (0, s) is TotalP[0][s].

- nbase is 0 to tell the function to pack particles staying in the local node from the
head of Particles[].

- ntp is TotalPNext[0][S] to show the size of the next pbuf (0, s) is TotalPNext[0][s].

- rbb is RecvBufBases[0][S] to tell the function that the head of rbuf (0, s) should be
set into RecvBufBases[0][s].


move_to_sendbuf_uw(0, me, NOfPLocal+me, 0, TotalP, 0, TotalPNext,
RecvBufBases);


If we were in secondary mode in the last step, i.e. the argument secondary is 1, we move all
secondary particles stored in Particles[primaryParts] and below to SendBuf[] regardless
the subdomain where they reside by calling move_to_sendbuf_uw() again but with the
following arguments.

- ps is 1 to scan secondary particles.


<!-- Page 247 -->

- me is −1 to force no particles to be judged to stay in Particles[].

- putmes is NULL to indicate no subdomains are specially treated as the primary sub-
domain.

- cbase is primaryParts to tell the function to start the scan from the head of pbuf (1, 0)
for secondary particles.

- ctp is TotalP[1][S] to show the size of the current pbuf (1, s) is TotalP[1][s].

- nbase is 0 to make it sure that all the blocks pbuf (1, s) is judged to move upward,
while no particles are moved in Particles[] actually.

- ntp is TotalPNext[1][S] to show the size of the next pbuf (0, s) is TotalPNext[1][s] = 0
to make it sure that all the blocks pbuf (1, s) is judged to move upward.

- rbb is RecvBufBases[1][S] so that the function safely set the base of rbuf (1, s) =
Particles into RecvBufBases[1][s], which will not be referred to though.


if (secondary) move_to_sendbuf_uw(1, -1, NULL, primaryParts, TotalP+ns,
0, TotalPNext+ns, RecvBufBases+ns);


Now we revisit the primary particle blocks pbuf (0, s) skipped by move_to_sendbuf_uw()
due to their downward moving direction. We move particles in these blocks by calling move_
to_sendbuf_dw() with arguments similar to its upward counterparts but different from it
as follows.

- The third argument is named ctail instead of cbase and we give primaryParts to
show the tail of the current primary particle buffer from which the function starts the
scan.

- The fifth argument is named ntail instead of nbase and we give the number of
primary particles of the local node n in the next step, TotalPGlobal[n], to show the
tail of the next primary particle buffer.

- The function does not have rbb argument because setting RecvBufBases[][] is solely
done by move_to_sendbuf_uw().

Note that it is unnecessary to call move_to_sendbuf_dw() for secondary particles even if we
were in secondary mode in the last step, because all of them have been moved to SendBuf[]
by move_to_sendbuf_uw() for them.

move_to_sendbuf_dw(0, me, NOfPLocal+me, primaryParts, TotalP,
TotalPGlobal[me], TotalPNext);


Finally, we call set_sendbuf_disps() again to regain the values of SendBufDisps[][] which
have been set by the first call of it, because move_to_sendbuf_uw() and move_to_sendbuf_
dw() have modified them for moving particles from Particles[] to SendBuf[]. Then we
call move_injected_from_sendbuf() giving InjectedParticles[0][0][] = qinj(n)[0][], n
and RecvBufBases[0][S] to its arguments to move injected primary particles, which have
been moved to SendBuf[] by move_injected_to_sendbuf(), from sbuf (s, n) in SendBuf[]
for the local node n back to pbuf (0, s) in Particles[], if we have injected particles, i.e.,
nOfInjections = Qinjn > 0.


<!-- Page 248 -->

set_sendbuf_disps(secondary, -1);
if (nOfInjections)
move_injected_from_sendbuf(InjectedParticles, me, RecvBufBases);
}


#### 4.5.10 move_to_sendbuf_secondary()

move_to_sendbuf_secondary()  The function move to sendbuf secondary(),  called from exchange_particles() and
some functions of level-4 or higher, moves particles to be sent to other nodes from the
particle buffer Particles[] to the send buffer SendBuf[], in a similar manner move_to_
sendbuf_primary() does. However, the particles to be sent are not only those residing in
the subdmains other than primary or secondary ones of the local node, but also some of them
resinding these responsible subdomains but being overflown from the node. The particles
staying in the local node as its primary (p = 0) or secondary (p = 1) ones are also moved in
Particles[] so that they are contiguously packed in each pbuf (p, s) for each p ∈{0, 1} and
species s ∈[0, S−1], each pbuf (p, s) has a receive buffer rbuf (p, s) of an appropriate size at
its head or tail, and all pbuf (p, s) are also contiguously aligned. The function also takes care
of particles injected by oh2_inject_particle() and located at the tail of Particles[], by
at first moving them to SendBuf[] regardless of their residing subdomains and then moving
some of primary ones from SendBuf[] back to Particles[]. The function is also responsi-
ble to let SendBufDisps[s][m], RecvBufBases[p][s] and TotalPNext[p][s] have appropriate
values for all p ∈{0, 1}, s ∈[0, S−1] and m ∈[0, N−1].
The function is given two arguments, secondary being true (1) if and only if we have
already been in the secondary mode in the last step, and stats being true (non-zero) if
and only if the execution time spent in the process to move particles has to be measured.


static void
move_to_sendbuf_secondary(int secondary, int stats) {
int me=myRank, ns=nOfSpecies, ns2=ns<<1, nn=nOfNodes;
struct S_node *node = Nodes+me;
int put[2]={-node->get.prime, -node->get.sec}, pnext[2];
int sec=node->parentid;
int nnns=nn*ns;
int *mynp[2]={NOfPLocal+me,           /* &NOfPLocal[0][0][me] */
sec<0 ? NULL : NOfPLocal+nnns+sec};
/* &NOfPLocal[1][0][sec] */
int *mynps;
int ps, s, i;


First we call oh1_stats_time() with the key STATS_TB_MOVE to measure the time spent
in the function if the argument stats is true. Then, for each p ∈{0, 1} and s ∈[0, S−1], we
modify NOfPLocal[p][s][n] so that it has the number of particles in primary and secondary
subdomains sent from the local node n due to overflow. We also calculate each element of
TotalPNext[p][s] by modifying its base value given by count_next_particles() or make_
recv_count(), and the sum for all s to have Qnn and Qparent(n)n        (or pnext[p]) in the next
step.
More specifically, the operations above are performed as follows. Let m = n for p = 0
and m = parent(n) for p = 1, and put(p) = putme be the number of particles resinding its


<!-- Page 249 -->

primary (p = 0) or secondary (p = 1) subdomain, accommodated by the local node and
being sent to other nodes due to overflow. That is;
{
−Rgetn = −Nodes[n].get.prime  p = 0                   put(p) =
−Qgetn = −Nodes[n].get.sec    p = 1

If put(p) ≤0 indicating that the node have to get some particles as its primary/secondary
particles, NOfPLocal[p][s][m] are cleared with zero for all s to mean that no particles in
the responsible subdomains are sent to other nodes. In this case, since TotalPNext[p][s]
was set to the total number of particles of species s to be recieved from other nodes, it
is incremented by the original (before clearing) value of NOfPLocal[p][s][m] to have the
number of particles to be accommodated by the local node in the next step.
Otherwise,  i.e.,  put(p) >  0,  TotalPNext[p][s] was  set to zero  for  all s because
no particles are received from other nodes, but some particles have to be sent out.
We choose particles to be sent from leading species by emptying first  t species such      ∑
that Σ(t) =    s<t NOfPLocal[p][s][m] ≤put(p) and thus both NOfPLocal[p][s][m] and
TotalPNext[p][s] (= 0) remain unchanged for all s < t. As for the species t, it has some
number of particles to be sent namely put(p) −Σ(t), and remainders to stay namely
Σ(t + 1) −put(p).   Therefore, NOfPLocal[p][t][m]  is set to the former number while
TotalPNext[p][t] to the latter by adding it to the original value 0. The particles of re-
mainder species of s > t simply stay in the local node and thus NOfPLocal[p][s][m] is
cleared with zero moving its original value to TotalPNext[p][s] (by adding it to the original
value 0) as done in the case of put(p) ≤0.
After that, if we have particles injected into the primary/secondary subdomain of the
local node, we have to take care of them by further modifying the value to be set into
NOfPLocal[p][s][m] discussed above, because it is the number of particles in the primary/
secondary subdomain to be sent including those of injected, but it should have the num-
ber of ordinary non-injected particles to be moved from Particles[] to SendBuf[]. Since
all injected particles are moved to SendBuf[] regardless of their residing subdomains, we
have decrease NOfPLocal[p][s][m] by the amount of the particles injeted into the primary/
secondary subdomain, namely InjectedParticles[0][p][s] = qinj(n)[p][s]. More specifi-
cally, we perform the following.

NOfPLocal[p][s][m] ←max(0, NOfPLocal[p][s][m] −qinj(n)[p][m])

We also let InjectedParticles[1][p][s] be the following to  tell move_injected_from_
sendbuf() how many particles should be moved from SendBuf[] back to Particles[].

InjectedParticles[1][p][s] = max(0, qinj(n)[p][s] −NOfPLocal[p][s][m])

That is, if NOfPLocal[p][s][m] > qinj(n)[p][s], we send all injected particles and make non-
injected ones of the same amount stay. Otherwise, we send some of injected particles while
all non-injected are made stay.
In any cases, Qnn and Qparent(n)n        are obtained by summing up the updated values of
TotalPNext[p][s] for all s. Note that, however, parent(n) can be −1 to indicate the lo-
cal node  is the root of the helpand-helper tree.  In this case, we skip the update of
NOfPLocal[1][][] and TotalPNext[1][] (= 0) to make them remain unchanged because all
the particles which was secondary in the last step, if any, have to be sent to other nodes,
and the local node will not have any secondary particles in the next step. In addtion, the
sum of secondary particles, conceptually Q−1n  , is set to zero.


<!-- Page 250 -->

if (stats) oh1_stats_time(STATS_TB_MOVE, 1);
for (ps=0,i=0; ps<2; ps++) {
int putme=put[ps], npnext=0;
mynps=mynp[ps];
if (mynps==NULL) {
pnext[ps] = 0;  break;
}
if (putme<0) putme = 0;
for (s=0; s<ns; s++,i++,mynps+=nn) {        /* i=ps*ns+s */
int stay=*mynps;                          /* NofPLocal[ps][s][me/sec] */
int tpni=TotalPNext[i];                   /* TotalPNext[ps][s] */
int inj=InjectedParticles[i];             /* InjectedParticles[ps][s] */
if (putme<stay) {
TotalPNext[i] = tpni = tpni + stay - putme;
stay = putme;
putme = 0;
} else
putme -= stay;
if (stay>inj) {
InjectedParticles[ns2+i] = 0;  *mynps = stay - inj;
} else {
InjectedParticles[ns2+i] = inj - stay;  *mynps = 0;
}
npnext += tpni;
}
pnext[ps] = npnext;
}

Next, we  call set_sendbuf_disps() with the first argument secondary and second
parent(n) of the local node n meaning we will be in secondary mode with the parent
(or without if negative), to calculate values of SendBufDisps[S][N]. We then call move_
injected_to_sendbuf() to move injected particles to SendBuf[] if we have any of them,
i.e., nOfInjections = Qinjn > 0.

set_sendbuf_disps(secondary, sec);
if (nOfInjections)  move_injected_to_sendbuf();


Now we move the primary particles to SendBuf[] or pack them in Particles[] by calling
move_to_sendbuf_uw() for the species whose block pbuf (0, s) will not have head and tail
addresses larger than their current ones, as done in move_to_sendbuf_primary() with the
following arguements.

- ps is 0 to scan primary particles.

- me is the rank of the local node n to identify the primary particles in the next step.

- putmes is the slice NOfPLocal[0][S][n] whose elements have been set to the number of
particles residing in the primary subdomain but being moved to SendBuf[] and sent
to other nodes.

- cbase is 0 to tell the function to start the scan from the head of Particles[].

- ctp is TotalP[0][S] to show the size of the current pbuf (0, s) is TotalP[0][s].


<!-- Page 251 -->

- nbase is 0 to tell the function to pack particles staying in the local node from the
head of Particles[].

- ntp is TotalPNext[0][S] to show the size of the next pbuf (0, s) is TotalPNext[0][s].

- rbb is RecvBufBases[0][S] to tell the function that the head of rbuf (0, s) should be
set into RecvBufBases[0][s].


move_to_sendbuf_uw(0, me, mynp[0],            /* &NOfPLocal[0][0][me] */
0, TotalP, 0, TotalPNext, RecvBufBases);


If we have already been in secondary mode,  i.e.  secondary = 1, we move secondary
particles stored in Particles[primaryParts] and below to SendBuf[] or pack them in the
region from Particles[Qnn] to Particles[Qnn+Qparent(n)n      −1]. This is done by calling move_
to_sendbuf_uw() and move_to_sendbuf_dw() giving them the following arguments.

- ps is 1 to scan secondary particles.

- me is parent(n) = Nodes[n].parentid for the rank of the local node’s helpand or −1
if the local node is the root of the helpand-helper tree.

- putmes is the slice NOfPLocal[1][S][n] whose elements have been set to the number
of particles residing in the secondary subdomain but being moved to SendBuf[] and
sent to other nodes, if parent(n) ≥0. Otherwise, i.e., if parent(n) = −1 for the local
node rooting the tree, this argument is NULL.

- cbase for move_to_sendbuf_uw() is primaryParts to tell the function to start the
forward scan from the head of pbuf (1, 0), while ctail for move_to_sendbuf_dw() is
totalParts for the tail (plus one) of pbuf (1, S−1) from which it scans reversely.

- ctp is TotalP[1][S] to show the size of the current pbuf (1, s) is TotalP[1][s].

- nbase for move_to_sendbuf_uw() is Qnn to tell the function to pack secondary par-
ticles staying in the local node following those of primary, while ntail for move_to_
sendbuf_dw() is Qnn + Qparent(n)n       to show the tail (plus one) of the particle buffer in
the next step.

- ntp is TotalPNext[1][S] to show the size of the next pbuf (1, s) is TotalPNext[1][s].

- rbb, only for move_to_sendbuf_uw(), is RecvBufBases[1][S] to tell the function that
the head of rbuf (1, s) should be set into RecvBufBases[1][s].


if (secondary) {
move_to_sendbuf_uw(1, sec, mynp[1],         /* &NOfPLocal[1][0][sec] */
primaryParts, TotalP+ns, pnext[0], TotalPNext+ns,
RecvBufBases+ns);
move_to_sendbuf_dw(1, sec, mynp[1], totalParts, TotalP+ns,
pnext[0]+pnext[1], TotalPNext+ns);

Otherwise, i.e., if we were in primary mode in the last step, we have no secondary particles to
be sent but will get them from other nodes. Since the number of particles to be received for
each species s has been set in TotalPNext[1][s], we set RecvBufBases[1][s] as follows so that


<!-- Page 252 -->

rbuf (1, 0) follows pbuf (0, S−1) of the next step and rbuf (1, s) is as large as TotalPNext[1][s]
for each s.
∑s−1
RecvBufBases[1][s] = Qnn +    TotalPNext[1][t]
t=0

} else {
struct S_particle *rbb=Particles+pnext[0];
for (s=0; s<ns; s++) {
RecvBufBases[ns+s] = rbb;                 /* RecvBufBases[1][s] */
rbb += TotalPNext[ns+s];                  /* TotalPNext[1][s] */
}
}

Now we revisit the primary particle blocks pbuf (0, s) skipped by move_to_sendbuf_uw()
due to their downward moving direction. We move particles in these blocks by calling
move_to_sendbuf_dw() as done in move_to_sendbuf_primary(), giving its ctail and
ntail arguements the indices of the tail (plus one) of pbuf (0, S−1) in the current and next
step, namely primaryParts and Qnn respectively.

move_to_sendbuf_dw(0, me, mynp[0], primaryParts, TotalP, pnext[0],
TotalPNext);


Then, we  call set_sendbuf_disps() again to regain the values of SendBufDisps[][].
Then we call move_injected_from_sendbuf() giving InjectedParticles[1][0][S], n and
RecvBufBases[0][S] to its arguments to move (some of) injected primary particles of the
local node n from SendBuf[] back to Particles[] if nOfInjections = Qinjn > 0. We also call
it again with InjectedParticles[1][1][[S], parent(n) and RecvBufBases[1][S] for injected
secondary particles if the local node n is not the root. Finally, we set primaryParts and
its shadow pointed by secondaryBase to Qnn.

set_sendbuf_disps(secondary, sec);
if (nOfInjections) {
move_injected_from_sendbuf(InjectedParticles+ns2, me, RecvBufBases);
if (sec>=0)
move_injected_from_sendbuf(InjectedParticles+ns2+ns, sec,
RecvBufBases+ns);
}
primaryParts = *secondaryBase = pnext[0];
}


#### 4.5.11 set_sendbuf_disps()

set_sendbuf_disps()  The function set_sendbuf_disps(), called from move_to_sendbuf_primary(), move_to_
sendbuf_secondary() and higher level library functions such as those in level-4p, calculates
values of SendBufDisps[S][N] so that its element [s][m] has the displacement to the head of
sbuf (s, m) in SendBuf[]. Since the number of primary particles of the species s to be sent to
the node m is in NOfPLocal[0][s][m] and, if we are in secondary mode and thus the argument
of the function secondary is true, that of secondary particles is in NOfPLocal[1][s][m],
SendBufDisps[s][m] is fundamentally defined as follows.

¬secondary                                     0                                    m−1∑                                                         m−1                       ∑                      SendBufDisps[s][m] =     NOfPLocal[0][s][i] +
     NOfPLocal[1][s][i]  secondary                                                 i=0
i=0


<!-- Page 253 -->

However, we have to take care of an additional factor, injected particles which have to be
moved to sbuf (s, m) for the local node m = n or its parent m = parent(n) and whose count
for a species s is recorded in InjectedParticles[0][p][s] = qinj(n)[p][s] where p = 0 for
primary and p = 1 for secondary particles respectively. Therefore, the equation above is
modified as follows, where parent(n) is given through the argument parent.

n′ = {n, parent(n)}[p]
{
0      m ̸= n′
q(p, s, m) = NOfPLocal[p][s][m] +                                                                                               qinj(n)[p][s] m = n′
{
0        ¬secondary
q(s, m) = q(0, s, m) +
q(1, s, m)  secondary
m−1∑
SendBufDisps[s][m] =     q(s, m)

i=0


The equation above implies that NOfPLocal[0][s][n] has the number of primary parti-
cles to be sent to helpers (thus should be 0  if we  will be in primary mode) while
NOfPLocal[1][s][parent(n)] has that to be sent to the helpand and/or sibling helpers.  It
also means that NOfPLocal[1][s][n] and NOfPLocal[0][s][parent(n)] are not special but sim-
ply represents number of particles visiting to primary/secondary domains including those
will be accommodated by the local node by self communication.


void
set_sendbuf_disps(int secondary, int parent) {
int nn=nOfNodes, ns=nOfSpecies, me=myRank;
int i, j, k, s, disp;

for (s=0,i=0,disp=0; s<ns; s++) {
for (k=0; k<nn; k++,i++) {
SendBufDisps[i] = disp;                   /* SendBufDisps[s][k] */
disp += NOfPLocal[i];                     /* NOfPLocal[0][s][k] */
if (k==me)  disp += InjectedParticles[s]; /* InjectedParticles[0][s] */
}
}
if (secondary) {
for (s=0,j=0,disp=0; s<ns; s++) {
for (k=0; k<nn; k++,i++,j++) {
SendBufDisps[j] += disp;                /* SendBufDisps[s][k] */
disp += NOfPLocal[i];                   /* NOfPLocal[1][s][k] */
if (k==parent)  disp += InjectedParticles[ns+s];
}                                         /* InjectedParticles[1][s] */
}
}
}


#### 4.5.12 exchange_particles()

exchange_particles()  The function exchange particles(), called from try_stable2(), rebalance2() and
higher level library functions such as those in level-4p, performs an all-to-all type parti-
cle transfer when we will be in secondary mode in the next simulation step. The function
is given the following arguments.


<!-- Page 254 -->

- secrlist points the secondary receiving or alternative secondary receiving block in
CommList[], which is given from the helpand of the local node in the next simulation
step, and secrlsize is its size.

- oldparent is the helpand of the local node in the last simulation step. More specifi-
cally, it has the followings.

  - If the function is called from try_stable2() and thus the last simulation step is
in secondary mode and the helpand-helper configuration is kept, it has the rank
of the helpand of the current configuration regardless of the accommodation
mode46.

  - In the case that the function is called from rebalance2() by which helpand-
helper configuration is rebuild, it has the rank of the helpand in the old configu-
ration if the last simulation step is in secondary mode. Otherwise, i.e., if we were
in primary mode, the argument has −1 to indicate no meaningful information is
given from the configuration before rebuilding47.

- neighboring is true (non-zero) if the accommodation mode is normal. Otherwise,
i.e., if anywhere accommodation mode, it is false (zero).

- currmode is referred to only for examining the execution mode in the last simulation
step is primary or secondary48.

- stats is true (non-zero) if and only if timing measurements for the process of particle
movement and transfer are required.


void
exchange_particles(struct S_commlist *secrlist, int secrlsize, int oldparent,
int neighboring, int currmode, int stats) {
int me=myRank, nn=nOfNodes, ns=nOfSpecies, nnns=nn*ns;
int newparent=Nodes[me].parentid;
int s, i, req;


The first job of the function is to call move_to_sendbuf_secondary() to move particles to
be sent to other nodes from the particle buffer Particles[] to the send buffer SendBuf[],
and to pack primary and secondary particles which are kept accommodated in the lo-
cal node’s Particles[] in which it reserves receive buffers for particle reception.  It also
sets SendBufDisps[s][m] to be the displacement of the block sbuf (s, m) from the head of
SendBuf[], and RecvBufBases[p][s] to point rbuf (p, s).
Then we call oh1_stats_time() with the key STATS_TB_COMM, if the argument stats
is true, to measure the time spent for the particle transfer communication.

move_to_sendbuf_secondary(Mode_PS(currmode), stats);
if (stats) oh1_stats_time(STATS_TB_COMM, 1);

If the argument neighboring is true to indicate that the accommodation mode is normal, we
perform particle transfer scanning the transfer schedules stored in CommList[] by receive_
particles() and send_particles() as follows.

46If the accommodataion mode is anywhere, this argument is not referred to by the function.
47If the accommodation mode is anywhere, the argument also has −1 but is not referred to by the
function.
48Since currmode has the information of neighboring, it is simply redundant to have two arguments
separately, but we have both of them due to some historical reason of the implementaion.


<!-- Page 255 -->

- The primary receiving block, which is located at the top of CommList[] and whose size
is SLHeadTail[0], is scanned by receive_particles() to receive primary particles
and/or to send them to the helpers of the local node if it is required to push down the
primary particles to them in order to make room to accommodate secondary particles.
This scan and transfer are performed always.

- If the local node n  is not the root of the helpand-helper tree,  i.e., parent(n) =
Nodes[n].parentid ≥0, the secondary receiving or alternative secondary receiving
block specified by the arguments secrlist and secrlsize is scanned by receive_
particles() to receive secondary particles and/or to send them to the members of
the family to which the local node belongs to as a helper due to secondary particle
overflow.

- If the helpand-helper configuration is rebuild by rebalance2() in secondary mode and
the local node has new helpand different from the old one, the secondary receiving
block given by the old helpand, which starts from CommList + SLHeadTail[1] and
whose size is SecSLHeadTail[0], is scanned by receive_particles() to send (not to
receive) particles in the subdomain which were secondary one of the local node.

- The primary sending block, which starts from CommList + SLHeadTail[0] and whose
size is SLHeadTail[1]−SLHeadTail[0], is scanned by send_particles() to send par-
ticles to the nodes responsible for the subdomain neigboring to the primary of the
local node. We have to be aware of the possibility that the primary sending block
is given from the node which is the helpand of the local node in the current (new)
or old configuration and its primary subdomain is neighboring to that of the local
node simultaneously. In this case, the primary sending block may have an intersec-
tion with the secondary receiving and alternative secondary receiving blocks which
have already been scannded by receive_particles() and thus simply scaninng the
primary sending blcok could cause duplicated transmissions. Therefore, the ranks of
new and old helpands are given to send_particles() to avoid the duplication so
that it skips S_commlist records having region elements maching to the ranks. This
scan and transfer are performed always.

- If we were in secondary mode in the  last simulation step and the local node
had the helpand in the configuration in the step, the secondary sending block,
which starts from CommList + SLHeadTail[1] + SecSLHeadTail[0] and whose size
is SecSLHeadTail[1] −SecSLHeadTail[0] is scanned by send_particles() to send
particles to the node responsible for the subdomain neighboring to the secondary sub-
domain of the local node in the configuration. We have to be aware of the possibility
that the secondary sending block is given from the node which was the helpand of
the local node and its primary subdomain is neighboring to that of the local node
itself or of its helpand in the new configuration. In this case, the secondary sending
block may have an intersection with the primary receiving or alternative secondary
receiving block which have already been scanned by receive_particles() and thus
simply scaninng the secondary sending blcok could cause duplicated transmissions.
Therefore, the rank of the local node itself and that of new helpand are given to
send_particles() to avoid the duplication so that it skips S_commlist records hav-
ing region elements maching to the ranks.

The functions receive_particles() and send_particles() receive and send particles
updating RecvBufBases[p][s] to let it point to the recieve buffer rbuf (p, s)/m for primary
or secondary particles of species s to be received from the node m, and SendBufDisps[s][k]


<!-- Page 256 -->

to let it has the displacement of sbuf (s, k)/m for particles of species s in the subdomain k to
be sent to the node m. They also increment their last argument req to count the number
of calls of MPI_Irecv() and MPI_Isend(), with which we confirm thier completions by
MPI_Waitall() giving it the arrays of non-blocking transfer requests Requests[] and their
completion stauses Statuses[].

if (neighboring) {
req = 0;
receive_particles(CommList, SLHeadTail[0], &req);
if (newparent>=0)
receive_particles(secrlist, secrlsize, &req);
if (oldparent!=newparent && oldparent>=0)
receive_particles(CommList+SLHeadTail[1], SecSLHeadTail[0], &req);
send_particles(CommList+SLHeadTail[0], SLHeadTail[1]-SLHeadTail[0],
newparent, oldparent, &req);
if (oldparent>=0)
send_particles(CommList+SLHeadTail[1]+SecSLHeadTail[0],
SecSLHeadTail[1]-SecSLHeadTail[0], me, newparent, &req);
MPI_Waitall(req, Requests, Statuses);
}

On the other hand, if the argument neighboring is false to indicate that the accommodation
mode is anywhere, we perform MPI_Alltoallv() for each p ∈{0, 1} and s ∈[0, S−1] giving
it the following arguments.

- sendbuf  is always SendBuf regardless  of p and s because we specify each  of
sbuf (s, k)/m for the node m responsible for the subdomain k by sdispls[m].

- sendcounts is NOfSend[p][s][] which is set by make_comm_count() called from try_
stable2() via try_stable1() or rebalance2() via rebalance1().

- sdispls is SendBufDisps[s][] for sbuf (s, m) for each node m if p = 0. Otherwise,
for the node m whose secondary subdomain is k = parent(m) ≥0, sdispls[m] =
TempArray[m] should have the following.
∑
TempArray[m] = SendBufDisps[s][k] + NOfSend[0][s][k] +    NOfSend[1][s][i]

i<m
parent(i)=k

This means that sbuf (s, k) for a subdomain k which has h = |H(k)| helpers is split
into h+1 consecutive sub-blocks sbuf (s, k)/k, sbuf (s, k)/m1, . . . , sbuf (s, k)/mh where
sbuf (s, k)/k has NOfSend[0][s][k] particles while sbuf (s, k)/m has NOfSend[1][s][m]
particles.

- recvbuf is RecvBufBases[p][s] to point rbuf (p, s).

- recvcounts is NOfRecv[p][s][] which is set by make_comm_count() called from try_
stable2() via try_stable1() or rebalance2() via rebalance1().

- rdispls is RecvBufDisps[] whose element [m] is defined as follows.

m−1∑
RecvBufDisps[m] =     NOfRecv[p][s][i]

i=0


<!-- Page 257 -->

- sendtype and recvtype are commmonly T_Particle.


else {
int ps;
int *rcount=NOfRecv;
int *scount=NOfSend;
struct S_particle **rbb=RecvBufBases;
for (ps=0; ps<2; ps++,rbb+=ns) {            /* rbb=&RecvBufBases[p][0] */
int *sbd0=SendBufDisps, *sbd;
for (s=0; s<ns; s++,rcount+=nn,scount+=nn,sbd0+=nn) {
/* rcount=&NOfRecv[ps][s][0] */
/* sbd0=&SendBufDisps[s][0] */
int rdisp=0;
for (i=0; i<nn; i++) {
RecvBufDisps[i] = rdisp;  rdisp += rcount[i];
}
if (ps==0) sbd = sbd0;                  /* &SendBufDisps[s][0] */
else {
sbd = TempArray;
for (i=0; i<nn; i++) {
int r=Nodes[i].parentid;
if (r>=0) {
sbd[i] = sbd0[r];
sbd0[r] += scount[i];
}
else sbd[i] = 0;            /* not necessary becasuse scount[i]=0
but ... */
}
}
MPI_Alltoallv(SendBuf, scount, sbd, T_Particle,
rbb[s], rcount, RecvBufDisps, T_Particle, MCW);
if (ps==0)
for (i=0; i<nn; i++) sbd0[i] += scount[i];
}
}
}
}


#### 4.5.13 move_to_sendbuf_uw()

move_to_sendbuf_uw()  The  function move_to_sendbuf_uw(),  called from move_to_sendbuf_primary() and
move_to_sendbuf_secondary(), scans primary (p = 0) or secondary (p = 1) particles
in Particles[] to move its contents to SendBuf or to pack them in Particles[] itself. The
function is given the following arguments according to the caller’s context defined by p.

- ps = p is used as the argument of Subdomain_Id() to extract the subdomain identifier
of a particle when it is in a neighbor of the local node’s primary/secondary subdomain.

- me = n′ is the rank of the local node n if p = 0, while it is parent(n) otherwise.  It
is used to identify particles which will be reside in the local node and thus is to be
moved in Particles[] by packing operation. Note that n′ = parent(n) can be −1
meaning that all secondary particles should be moved to SendBuf[] because the local
node will not have its helpand in the next step.


<!-- Page 258 -->

- putmes is the slice NOfPLocal[p][S][n′] whose element [p][s][n′] has the number of
primary (p = 0) or secondary (p = 1) particles residing in the primary/secondary
subdomain but being moved to SendBuf[] and sent to other nodes in the primary/
secondary family of the local node.

- cbase is 0 if p = 0 or primaryParts otherwise, to specify the starting point of the
scan, i.e., pbuf (p, 0).

- ctp is TotalP[p][S] to show the size of the current pbuf (p, s) is ctp[s].

- nbase is 0 if p = 0 or Qnn otherwise, to specify the head of pbuf (p, 0) for the next
step. That is, particles staying in the local node is packed to the region from nbase.

- ntp is TotalPNext[p][S] to show the size of the next pbuf (0, s) is ntp[s].

- rbb is RecvBufBases[p][S] to specify that the head of rbuf (p, s) should be set into
rbb[s].



static void
move_to_sendbuf_uw(int ps, int me, int *putmes, int cbase, int *ctp,
int nbase, int *ntp, struct S_particle **rbb) {
int i, in, j, jn, k, s;
int ns=nOfSpecies, nn=nOfNodes, *sbd=SendBufDisps;
Decl_Grid_Info();


The function moves particles in pbuf (p, s), by scanning them in the ascending order of
s, as follows. Let i and j be the followings being the head of the current and next pbuf (p, s)
respectively.

∑s−1     ∑s−1
i = cbase +    ctp[t] = cbase +    TotalP[p][t]

t=0                  t=0
∑s−1     ∑s−1
j = nbase +    ntp[t] = nbase +    TotalPNext[p][t]

t=0                  t=0

1. If j ≤i, all the particles staying in pbuf (p, s) can be moved upward,  i.e., toward
smaller locations from their current locations, because the number of particles to be
moved is at most ctp[s]. In this case, we move particles in Particles[i+k] for all
k ∈[0, ctp[s]) as follows.

(a) If the subdomain identifier m of Particles[i+k] obtained by Subdomain_Id()
is not equal to  n′,  i.e., the particle  is not in the subdomain specified by
n′ but in m,  it is moved to SendBuf[SendBufDisps[s][m]] post-incrementing
SendBufDisps[s][m]. Note that m can be −1 to mean the particle disappears
from the simulation domain and thus we simply discard it. Also note that n′ can
be −1 to mean all particles in the subdomain m ≥0 are unconditionally moved
to SendBuf[].

(b) Otherwise,  if putmes  ̸= NULL and k <  putmes[s][0] to mean the particle
Particles[i+k] is in the leading region to be sent to a family member, the
particle is moved to SendBuf[] as done in (a).


<!-- Page 259 -->

(c) Otherwise, i.e., after the movement of the particles done by (b), the reminders
in the subdomain n′ are moved upward to the next pbuf (p, s) starting from
Particles[j].

Finally, we set rbb[s] = RecvBufBases[p][s] to point Particles[j + l] where l is the
number of particles staying in this block pbuf (p, s) so that particles from other nodes
are received to the bottom of the block.


for (s=0,i=cbase,j=nbase,k=0; s<ns; s++,i=in,j=jn,sbd+=nn,k+=nn) {
int putme = putmes ? putmes[k] : 0; /* NOfPLocal[0/1][s][me/sec] */
in = i + ctp[s];  jn = j + ntp[s];
if (j<=i) {                         /* upward move only */
for (; putme>0; i++) {            /* throw my particles to send buf */
int dst=Subdomain_Id(Particles[i].nid, ps);
if (dst<0) continue;
SendBuf[sbd[dst]++] = Particles[i];
if (dst==me) putme--;
}
for (; i<in; i++) {               /* move upward */
int dst=Subdomain_Id(Particles[i].nid, ps);
if (dst<0) continue;
if (dst==me) Particles[j++] = Particles[i];
else         SendBuf[sbd[dst]++] = Particles[i];
}
rbb[s] = Particles + j;           /* receive to bottom */


2. If j > i and j + ntp[s] > i + ctp[s], all the particles to stay in pbuf (p, s) can be
moved downward by a bottom-up scan of the block.  Since this movement must be
performed after the movement of the succeeding blocks, we leave it to the counterpart
function move_to_sendbuf_dw(). Note that rbb[s] = RecvBufBases[p][s] is set to
point Particles[j] so that particles from other nodes are received to the top of the
block because particles staying are packed to the bottom.


} else if (jn>in) {                 /* downward only and thus skip */
rbb[s] = Particles + j;           /* receive to top */


3. Otherwise, i.e., j > i but j + ntp[s] ≤i + ctp[s], the upper half of the block must
be moved downward while the lower half have to be moved upward. Therefore, after
we move particles as done in 1(a) and 1(b) and record the source location ib of the
first succeeding particle, we skip particles which should move downward (if any still),
i.e., those staying in the subdomain n′ ≥049, recording the source and destination
locations, namely im and jm, of the last particle skipped. Then we move the remaining
l particles upward in the way of 1(a) and 1(c), and set rbb[s] = RecvBufBases[p][s]
to point Particles[jm + 1 + l] to receive particles from other nodes at the bottom
of pbuf (p, s).  Finally, we move skipped particles Particles[k] for all k such that
ib ≤k ≤im  (if any,  i.e.,  ib ≤im) downward scanning them descendingly from
Particles[im] in the way of 1(a) and 1(c) to the subblock whose tail is Particles[jm].

49We need to check the subdomain identifier obtained by Subdomain Id() is non-negative because n′ can
be −1.


<!-- Page 260 -->

} else {                            /* downward and upward */
int ib, im, jm;
for (; putme>0; i++) {            /* throw my particles to send buf */
int dst=Subdomain_Id(Particles[i].nid, ps);
if (dst<0) continue;
SendBuf[sbd[dst]++] = Particles[i];
if (dst==me) putme--;
}
ib = i;
for (; i<j; i++) {                 /* skip downward movers if any */
int dst=Subdomain_Id(Particles[i].nid, ps);
if (dst==me && dst>=0)  j++;
}
im = i-1; jm = j-1;
for (; i<in; i++) {               /* move remainders upward */
int dst=Subdomain_Id(Particles[i].nid, ps);
if (dst<0) continue;
if (dst==me) Particles[j++] = Particles[i];
else         SendBuf[sbd[dst]++] = Particles[i];
}
rbb[s] = Particles + j;           /* receive to bottom */
for (i=im,j=jm; i>=ib; i--) {     /* move first half downward if any */
int dst=Subdomain_Id(Particles[i].nid, ps);
if (dst<0) continue;
if (dst==me) Particles[j--] = Particles[i];
else         SendBuf[sbd[dst]++] = Particles[i];
}
}
}
}


#### 4.5.14 move_to_sendbuf_dw()

move_to_sendbuf_dw()  The  function move_to_sendbuf_dw(),  called from move_to_sendbuf_primary() and
move_to_sendbuf_secondary(), scans primary (p = 0) or secondary (p = 1) particles
in Particles[] to move its contents to SendBuf or to pack them in Particles[] itself. The
scanning and moving are performed only on the blocks pbuf (p, s) which are skipped by
the upward counterpart move_to_sendbuf_uw() because they are shifted down as a whole.
The function is given the following arguments according to the caller’s context defined by
p.

- ps, me, putmes, ctp and ntp are equivalent to those for move_to_sendbuf_uw().

- ctail is primaryParts  if p = 0 or totalParts otherwise, to specify the starting
point of the scan, i.e., the tail (plus one) of pbuf (p, S−1).

- ntail is Qnn  if p = 0 or Qnn + Qparent(n)n        otherwise, to specify the tail (plus one) of
pbuf (p, S−1) in the next step.



static void
move_to_sendbuf_dw(int ps, int me, int *putmes, int ctail, int *ctp, int ntail,


<!-- Page 261 -->

int *ntp) {
int i, in, j, jn, k, s, ns=nOfSpecies, nn=nOfNodes, nnnsm1=nn*(ns-1);
int *sbd=SendBufDisps+nnnsm1;
Decl_Grid_Info();


The function moves particles in pbuf (p, s), by scanning them in the descending order of
s, as follows. Let i and j be the followings being the tails of the current and next pbuf (p, s)
respectively.

S−1∑                   S−1∑
i = ctail −1 −     ctp[t] = ctail −1 −     TotalP[p][t]
t=s                       t=s
S−1∑                   S−1∑
j = ntail −1 −     ntp[t] = ntail −1 −    TotalPNext[p][t]
t=s                       t=s

If i < j and i −ctp[t] < j −ntp[s], the particles in the block pbuf (p, s) are scanned and
moved in the way shown in 1(a)–(c) in §4.5.13.

in = ctail;  jn = ntail;
for (s=ns-1,i=in-1,j=jn-1,k=nnnsm1; s>=0; s--,i=in-1,j=jn-1,sbd-=nn,k-=nn) {
int putme = putmes ? putmes[k] : 0; /* NOfPLocal[0/1][s][me/sec] */
in -= ctp[s];  jn -= ntp[s];
if (i>=j || in>=jn) continue;       /* not downward only and thus skip */
for (; putme>0; i--) {              /* throw my particles to send buf */
int dst=Subdomain_Id(Particles[i].nid, ps);
if (dst<0) continue;
SendBuf[sbd[dst]++] = Particles[i];
if (dst==me) putme--;
}
for (; i>=in; i--) {                /* move downward */
int dst=Subdomain_Id(Particles[i].nid, ps);
if (dst<0) continue;
if (dst==me) Particles[j--] = Particles[i];
else         SendBuf[sbd[dst]++] = Particles[i];
}
}
}


#### 4.5.15 move_injected_to_sendbuf()

move_injected_to_sendbuf()  The function move_injected_to_sendbuf(), called from move_to_sendbuf_primary()
and move_to_sendbuf_secondary(), moves particles injected by oh2_inject_particle()
from the tail of Particles[] to SendBuf[]. It scans the block for the injected particles which
starts from Particles[totalParts] and has nOfInjections = Qinjn  particles. Each parti-
cle whose nid, or its part for subdmain identifier extracted by Subdomain_Id(), and spec
are m and s0 respectively is moved to sbuf (s, m) where s is given by Particle_Spec() as
follows, if nid is non-negative.
{
s0 −specBase  OH_HAS_SPEC is defined                                                 s =
0               otherwise

That is, if S_particle has the element spec, s is the element offset by specBase which
is 0  if the library is initialized by oh2_init() called from a C-coded simulator, while


<!-- Page 262 -->

1  if initialized by oh2_init_() called from a Fortran-coded simulator. The location in
sbuf (s, m) to which a particle is moved is given by SendBufDisps[s][m] which was set to
the head of sbuf (s, m) by set_sendbuf_disps() prior to the call of this function. Then
it is post-incremented at each move to make it point the head of the buffer for ordinary
not-injected particles to be sent to the other nodes.
Note that this function moves particles injected into primary/secondary subdomain of
the local node to SendBuf[].  These particles or part of them, however, will be moved
back to rbuf (p, s) in Particles[] by move_injected_from_sendbuf(). Also note that the
subdomain identifier of a particle injected into a subdomain m can have (N + 3D) + d or
(N +3D)+m+3D if the subdomain is secondary one of the local node or is its d-th neighbor,
if OH_POS_AWARE is defined to mean we employ position-aware particle management. If so,
we let the identifier be m by the macro Primarize_Id().


static void
move_injected_to_sendbuf() {
struct S_particle *pbuf=Particles+totalParts;
int ninj=nOfInjections, nn=nOfNodes, sb=specBase;
int i;
Decl_Grid_Info();

for (i=0; i<ninj; i++) {
int dst = Subdomain_Id(pbuf[i].nid, 0);
int s = Particle_Spec(pbuf[i].spec-sb);
if (dst<0) continue;
#ifdef OH_POS_AWARE
if (dst>=nn)  Primarize_Id(pbuf+i, dst);
#endif
SendBuf[SendBufDisps[dst+s*nn]++] = pbuf[i];
}
}


#### 4.5.16 move_injected_from_sendbuf()

move_injected_from_sendbuf()  The function move_injected_from_sendbuf(), called from move_to_sendbuf_primary()
and move_to_sendbuf_secondary(), moves particles injected by oh2_inject_particle()
or  its higher  level counterparts and then temporarily moved to SendBuf[] by move_
injected_to_sendbuf() back to Particles[]. The function has three arguments the first
of which is an array injected[S] = InjectedParticles[π][p][S] whose element [s] = qs
is the number of primary (p = 0) or secondary (p = 1) particles of species s to be
moved back, where π = 0  if called from move_to_sendbuf_primary() while π = 1
if from move_to_sendbuf_secondary(), from the leading part of sbuf (s, m) to that of
rbuf (p, s) where m = n for the local node n  if p = 0 or m = parent(n)  if p = 1 and
m is given through its argument mysd. Therefore, for each s ∈[0, S), we move particles
SendBuf[SendBufDisps[s][m] + k] to the location pointed by RecvBufBases[p][s] + k for all
k ∈[0, qs), where RecvBufBases[p] is given through the argument rbb. Then we increment
SendBufDisps[s][m] and RecvBufBases[p][s] by qs so that they point the heads of send/
receive buffers for particles to be sent/received.


static void
move_injected_from_sendbuf(int *injected, int mysd, struct S_particle **rbb) {


<!-- Page 263 -->

int nn=nOfNodes, ns=nOfSpecies;
int *sdisp=SendBufDisps+mysd;
int s, i;

for (s=0; s<ns; s++,sdisp+=nn) {
struct S_particle *rbuf=rbb[s];
struct S_particle *sbuf=SendBuf+*sdisp;
int inj=injected[s];
for (i=0; i<inj; i++)  rbuf[i] = sbuf[i];
rbb[s] += inj;  *sdisp += inj;
}
}


#### 4.5.17 receive_particles()

receive_particles()  The function receive_particles(), called solely from exchange_particles(), scans the
S_commlist sequence, the primary or (alternative) secondary receiving block of CommList,
whose head and size are given through the arguments rlist and rlsize.  It posts MPI_
Irecv() and MPI_Isend() each time a record for receiving/sending to/from the local node
n is found. The arguments count, source, dest and tag for these MPI functions are simply
given by the CommList record, and type and comm are obvious and invariant, but buf is a
little bit complicated.
For MPI_Irecv(), buf for the k-th record with tag = pS + s starts from qr(p, s, k)-th
particle in the buffer whose head is pointed by RecvBufBases[p][s] when the function is
called, where qr(p, s, k) is defined as follows.

Cr(p, s, k) = {i | i < k, rlist[i].(rid, tag) = (n, pS + s)}
∑
qr(p, s, k) =    rlist[i].count
i∈Cr(p,s,k)

Therefore each time we find a record with rid = n, we increment RecvBufBases[p][s] =
RecvBufBases[tag] by count of the record after letting buf be RecvBufBases[p][s].
For MPI_Isend(), buf for the k-th record with tag = pS + s and region = m
starts from qs(s, m, k)-th particle in the buffer whose head’s displacement of SendBuf[] is
SendBufDisps[s][m] = SendBufDisps[sN +m] when the function is called, where qs(s, m, k)
is defined as follows.

Cs(s, m, k) = {i | i < k, rlist[i].(sid, tag, region) = (n, pS + s, m), p ∈{0, 1}}
∑
qs(s, m, k) =    rlist[i].count
i∈Cs(s,m,k)

Therefore each time we find a record with sid = n, we increment SendBufDisps[s][m] by
count of the record after letting buf be SendBuf + SendBufDisps[s][m].
We also give the MPI functions the pointer to Requests[r] to let them store an MPI_
Request structure in it, where r’s initial value is given through the argument req of this
function, r is incremented each time the MPI functions are called, and then r’s final value
is returned to the caller through req to be used for the successive calls of this function and
send_particles().


static void


<!-- Page 264 -->

receive_particles(struct S_commlist *rlist, int rlsize, int *req) {
int me=myRank, i, r=*req, nn=nOfNodes, ns=nOfSpecies, sdisp;
struct S_particle *rbuf;

for (i=0; i<rlsize; i++) {
if (rlist[i].rid==me) {
int count=rlist[i].count, tag=rlist[i].tag;
rbuf = RecvBufBases[tag];  RecvBufBases[tag] = rbuf + count;
MPI_Irecv(rbuf, count, T_Particle, rlist[i].sid, tag, MCW,
Requests+(r++));
}
if (rlist[i].sid==me) {
int count=rlist[i].count, tag=rlist[i].tag, region=rlist[i].region;
region += nn * (tag<ns ? tag : tag-ns);
sdisp = SendBufDisps[region];  SendBufDisps[region] = sdisp + count;
/* SendBufDisps[s][region] */
MPI_Isend(SendBuf+sdisp, count, T_Particle, rlist[i].rid, tag, MCW,
Requests+(r++));
}
}
*req = r;
}


#### 4.5.18 send_particles()

send_particles()  The function send_particles(), called solely from exchange_particles(), scans the S_
commlist sequence, the primary or secondary sending block of CommList, whose head
and size are given through the arguments slist and slsize.  It posts MPI_Isend() for
records for sending from the local node, giving it arguments in the same way as receive_
particles() does for sending records to send particles from buffers in SendBuf[] whose
displacement is specified by SendBufDisps[][]. However, the records to be processed are
not just those having sid = n for the local node n, but those having region matching
to myregion or parentregion argument are excluded from the processing. That is, as
explained in §4.5.12, since such a record should have been already processed by receive_
particles() because it should be in a receiving block, we have to exclude it to avoid
duplicated transmission. The function also has an argument req to receive/return the entry
number of Requests[] for MPI_Request structures also as done in receive_particles().


static void
send_particles(struct S_commlist *slist, int slsize, int myregion,
int parentregion, int *req) {
int me=myRank, i, r=*req, nn=nOfNodes, ns=nOfSpecies, sdisp, region;

for (i=0; i<slsize; i++) {
if (slist[i].sid==me && (region=slist[i].region)!=myregion &&
region != parentregion) {
int count=slist[i].count, tag=slist[i].tag;
region += nn * (tag<ns ? tag : tag-ns);
sdisp = SendBufDisps[region];  SendBufDisps[region] = sdisp + count;
/* SendBufDisps[s][region] */
MPI_Isend(SendBuf+sdisp, count, T_Particle, slist[i].rid, tag, MCW,
Requests+(r++));


<!-- Page 265 -->

}
}
*req = r;
}


#### 4.5.19 oh2_inject_particle()

oh2_inject_particle_()  The API function oh2_inject_particle_() for Fortran and oh2_inject_particle() for
oh2_inject_particle() C provide a simulator body calling them with the way to inject a particle the pointer to
which is given by the argument part. Before it moves the particle to the tail of the particle
buffer, namely Particles[i] where i = totalParts+nOfInjections = Qn+Qinjn  , it checks
if OH_HAS_SPEC is defined or S = 1 to mean Particle_Spec() correctly gives the spec s
of the particle from its spec element offset by specBase or s = 0 unconditionaly because
of S = 1, and abort the execution by local_errstop() if not satisfied.  It also checks if
i < Plim = nOfLocalPLimit whose violation also causes abort by local_errstop() due to
the overflow of Particles[].
Then the function moves the particle of nid = m to Particles[i] incrementing Qinjn
to show the number of injections as well as the entry for the next injection.  After
that, if m = parent(n) for local node n, it increments NOfPLocal[1][s][m] to incorporate
the injected particle to the secondary particle population histogram, and also increments
InjectedParticles[0][1][s] = qinj(n)[1][s] to count the number of particles injected into
n’s secondary subdomain.  Otherwise, the injected particle is regarded as primary, and
thus NOfPLocal[0][s][m] is incremented if m ≥0. Then,  if m = n for the local node n,
it increments InjectedParticles[0][0][s] = qinj(n)[0][s] to count the number of particles
injected into n’s primary subdomain50.


void
oh2_inject_particle_(struct S_particle *part) {
oh2_inject_particle(part);
}
void
oh2_inject_particle(struct S_particle *part) {
const int ns=nOfSpecies, nn=nOfNodes;
int inj = totalParts + nOfInjections++;
int s = Particle_Spec(part->spec - specBase);
int n=part->nid;

#ifndef OH_HAS_SPEC
if (ns!=1)
local_errstop("particles cannot be injected when S_particle does not "
"have ’spec’ element and you have two or more species");
#endif
if (inj>=nOfLocalPLimit)
local_errstop("injection causes local particle buffer overflow");
Particles[inj] = *part;
if (n<0)  return;
if (n==RegionId[1]) {

50Note that the particle residence subdomain is just part->nid instead of that with Subdomain Id()
because we have other function for particle injection with position-aware particle management. Also note
that regarding particles injected into secondary subdomain as secondary should work almost perfectly well
unless a particle is injected at the boundary of secondary subdomain.
