# 4.13 C Source File ohhelp4s.c - Part 2

Source: `doc/original/ohhelp.pdf`, pages 494-525.

<!-- Page 494 -->

Now, for each s ∈[0, S), we gather sending planes of all 2D = 6 neighbors to the local
node’s receiving planes by oh3_exchange_borders() giving it the base per-grid histogram
NOfPGridTotal[0][s][] and C−1 being the entry for per-grid histograms in BorderExc[][][][].
Its second argument for the secondary subdomain’s array is NULL because we don’t broadcast
the receiving planes to the helpers as indicated its forth argument bcast = 0.
Then we add each of 2D receiving planes to each halo planes of d-th dimensional from
d = D −1 = 2 to 0 to have the complete per-grid histogram. The addition is performed by
a series of calls of add_population() for each receiving/halo plane pair. More specifically,
the d-th dimensional halo planes to which we add receiving planes are specified as [βl0, βu0 )×
[βl1, βu1 ) × [βl2, βu2 ) where [βlk, βuk ) is specified as follows.

[−3eg, δk(n) + 3eg)     k < d                         
[−eg, eg)              k = d and lower
[βlk, βuk ) =
 [δk(n) −eg, δk(n) + eg)  k = d and upper
[−eg, δk(n) + eg)       k > d

The function is given the arguments for the base per-grid histogram, the lower and upper
bound of the halo planes in each axis shown above, and the offset of the receiving planes
from boundary planes namely;

d−1∏
gidx(β0, · · · , βd ± 2eg, · · · , βD−1) −gidx(β0, · · · , βd, · · · , βD−1) = ±2eg    (δmaxk  + 6eg)
k=0

where −/+ for lower/upper boundary. The method for the addition is same as that we
used in the sample code’s function add_boundary_current() shown in §3.13.
∑δy(n) ∑δx(n)
After the addition, we add   y=0    x=0 PT (0, s, gidx(x, y, z)) to NOfPGridZ[z] to have
per-plane histogram PZ(z) in it when we finish this function.

for (zz=0; zz<z; zz++)  NOfPGridZ[zz] = 0;
for (s=0; s<ns; s++) {
dint *npgt = npg[s];
oh3_exchange_borders(npgt, NULL, ct, 0);
add_population(npgt, -ext3, x+ext3, -ext3, y+ext3, -ext, ext,  -dw*ext2);
add_population(npgt, -ext3, x+ext3, -ext3, y+ext3, z-ext, z+ext, dw*ext2);
add_population(npgt, -ext3, x+ext3, -ext, ext, -ext, z+ext, -w*ext2);
add_population(npgt, -ext3, x+ext3, y-ext, y+ext, -ext, z+ext,  w*ext2);
add_population(npgt, -ext,  ext,   -ext, y+ext, -ext, z+ext, -ext2);
add_population(npgt, x-ext, x+ext, -ext, y+ext, -ext, z+ext,  ext2);

For_All_Grid(0, 0, 0, 0, 0, 0, 0)
NOfPGridZ[Grid_Z()] += npgt[The_Grid()];
}
}


#### 4.13.17 reduce_population()

reduce_population()  The function reduce_population(), called solely from exchange_population(), performs
reduce communications in primary (p = 0) and secondary (p = 1) family members to sum
up NOfPGrid[p][][] to have the sum in NOfPGridTotal[0][][].


<!-- Page 495 -->

The function works equivalently to its level-4p counterpart described in §4.10.18 when
the counterpart is given MPI_Reduce() through its argument87.  That  is, the function
performs red-black reduction as done in oh1_reduce() but keeping the source array
NOfPGrid[][][] rather than overwriting it by MPI_IN_PLACE option, and if the prime element
of MyComm is MPI_COMM_NULL, copies NOfPGrid[0][][] into NOfPGridTotal[0][][] explicitly by
memcpy(). Also equivalent to the level-4p counterpart, the base index and the number of
elements to be reduced are specified in FieldDesc[F−1].red.base and its element size[p]
for the per-grid histogram.


static void
reduce_population() {
const int ft=nOfFields-1;
const int base = FieldDesc[ft].red.base;
const int *size = FieldDesc[ft].red.size;

if (MyComm->black) {
if (MyComm->prime!=MPI_COMM_NULL)
MPI_Reduce(NOfPGrid[0][0]+base, NOfPGridTotal[0][0]+base, size[0],
MPI_LONG_LONG_INT, MPI_SUM, MyComm->rank, MyComm->prime);
if (MyComm->sec!=MPI_COMM_NULL)
MPI_Reduce(NOfPGrid[1][0]+base, NOfPGridTotal[1][0]+base, size[1],
MPI_LONG_LONG_INT, MPI_SUM, MyComm->root, MyComm->sec);
} else {
if (MyComm->sec!=MPI_COMM_NULL)
MPI_Reduce(NOfPGrid[1][0]+base, NOfPGridTotal[1][0]+base, size[1],
MPI_LONG_LONG_INT, MPI_SUM, MyComm->root, MyComm->sec);
if (MyComm->prime!=MPI_COMM_NULL)
MPI_Reduce(NOfPGrid[0][0]+base, NOfPGridTotal[0][0]+base, size[0],
MPI_LONG_LONG_INT, MPI_SUM, MyComm->rank, MyComm->prime);
}
if (MyComm->prime==MPI_COMM_NULL)
memcpy(NOfPGridTotal[0][0]+base, NOfPGrid[0][0]+base,
size[0]*sizeof(dint));
}


#### 4.13.18 add_population()

add_population()  The function add_population(), called solely from exchange_population() but 2DS
times, adds the elements in a receiving plane set in per-grid histogram for a species, spec-
ified by its argument npd being NOfPGridTotal[0][s][], to the halo plane (set) specified by
arguments as [xl, xu)×[yl, yu)×[zl, zu), whose values are shown in §4.13.16. The function
is perfectly equivalent to its level-4p counterpart described in §4.10.16 but its arguments
given from the caller are different as discussed in §4.13.16.


static void
add_population(dint *npd, const int xl, const int xu, const int yl,
const int yu, const int zl, const int zu,
const int src) {

87  Therefore,   the  function   is  very  similar  to  the  counterpart   literally  and  thus  have
NOfPGridTotal[1][0]+base as the second argument of MPI Reduce() for the reduction in secondary family
knowing the argument is meaningless.


<!-- Page 496 -->

dint *nps=npd+src;
Decl_For_All_Grid();

For_All_Grid_Abs(0, xl, yl, zl, xu, yu, zu)
npd[The_Grid()] += nps[The_Grid()];
}


#### 4.13.19 make_recv_list()

make_recv_list()  The function make_recv_list(), called solely from exchange_particles4s() when we will
be in secondary mode in the next step, scans per-plane histogram to build primary receiving
block, and then exchanges the block between neighbors to have primary sending block
and broadcast them for secondary receiving/sending and alternative secondary receiving/
sending blocks for helpers.  Its arguments currmode, level, reb and stats are perfectly
equivalent to those of the caller exchange_particles4s(), while oldp = npold and newp =
npnew are parents in the last and next simulation step.
The function is similar to its level-4p counterpart described in §4.10.19 but has various
differences as follows.

- Since we scan per-plane histogram rather than per-grid histogram to build the re-
ceiving schedule, the arguments set of the callee sched_recv() and its context are
different from those of the counterpart.

- Since we do not have hot-spots, we can be unaware of the possiblity the primary
receiving block generated by sched_recv() has hot-spot records especially at its tail.

- Since we need primary sending and secondary sending blocks even with anywhere ac-
commodation for halo paticle transfer scheduling, the function works without respect
to the accommodation pattern.

- On the helpand-helper reconfiguration, level-4s’s alternative secondary receiving block
is followed by alternative secondary sending block being the copy of npnew’s primary
sending block necessary for halo particle transfer.

- This function does not return anything because we do not have hot-spot sending block
whose head pointer is returned by the counterpart.


static void
make_recv_list(const int currmode, const int level, const int reb,
const int oldp, const int newp, const int stats) {
const int me = myRank, ns=nOfSpecies, nn=nOfNodes, nnns=nn*ns;
const int nn2 = nn<<1;
struct S_node *nodes = reb ? NodesNext : Nodes;
struct S_node *mynode = nodes + me;
struct S_node *ch;
struct S_recvsched_context
context = {0, 0, 0, CommList};
int rlsize, rlidx;
const int ft=nOfFields-1;
const int npgbase = FieldDesc[ft].bc.base;
const int *npgsize = FieldDesc[ft].bc.size;
const int zmax = GridDesc[0].z-1;
struct S_commlist *lastrl;


<!-- Page 497 -->

int i;


First, the function builds primary receiving block by calling sched_recv() for the local
node’s helpers and then the node itself to determine the subcuboid assigned to each node.
Unlike level-4p, we may scan the family members in either of helper-first or helpand-first
order because of no hot-spots, but we followed the level-4s’s convention, i.e., helper-first.
The arguments for the function are as follows, where NN  is Nodes[] if reb is false, or
NodesNext[] otherwise, for new helpands.

- Unlike level-4p, the function does not have the argument currmode, and reb is dif-
ferent from the counterpart because it is true iffwe have normal accommodation88,
helpand-helper reconfiguration is taking place, and the call is for a helper. That is,
reb is true when get has the expected number of particles the node will accommodate,
or false the number is calculated by get + stay as discussed in §4.10.20.

- As in level-4p, get is Rgetn = NN [n].get.prime for the local node n, or Qgetm =
NN [m].get.sec for its helper m, to specify the baseline number of receiving  (if
positive) or sending (if negative) primary/secondary particles of n or m.

- As in level-4p, stay is Qnn = NN [n].stay.prime for the local node n, or Qnm =
NN [m].stay.sec for its helper m, to specify the number of primary/secondary par-
ticles currently accommodated by n or m. The value is not useful if reb passed to
the function is true as described above.

- As in level-4p, nid is the node identifier of the local node n or its helper m.

- As in level-4p, tag is 0 for the local node, or NS for its helpers, to distinguish helpand
and helpers and to be set into S_commlist record element tag.

- Unlike level-4p, context is a S_recvsched_context structure whose difference from
level-4p’s counterpart is discussed in §4.12.4. Similarly to level-4p, however, its ele-
ments z, nptotal and nplimit are 0 at initial, while cptr is initialized to point the
head of CommList[].

Similarly to level-4p, we have primary receiving block by the sequence of calls, but its
last record does not necessary has the largest z-coordinate of the local node n’s primary
subdomain, δz(n) −1, because the topmost xy-plane can have no particles. If so, we need
to make the last record’s region have δz(n) −1 but we cannot simply do it by overwriting
the record in the case that the subdomain has no particles at all and thus the primary
receiving block is empty. In this case, we add a record to assign all xy-planes to the local
node n letting region element be δz(n) −1.

for (ch=mynode->child; ch; ch=ch->sibling)
sched_recv(reb, ch->get.sec, ch->stay.sec, ch->id, nnns, &context);
sched_recv(0, mynode->get.prime, mynode->stay.prime, me, 0, &context);

rlidx = rlsize = context.cptr - CommList;  lastrl = context.cptr - 1;
if (rlsize==0) {
struct S_commlist *rl = CommList;
rl->rid = me;  rl->tag = 0;  rl->sid = 0;  rl->count = 0;

88Even in level-4p, it is assured that reb is false if we have anywhere accommodation but this fact is not
exploited in the implementation.


<!-- Page 498 -->

rl->region = zmax;
rlidx = rlsize = 1;
} else
lastrl->region = zmax;


Next, unlike level-4p, the local node n exchanges its primary receiving block between its
neighbors to have primary sending blocks, regardless of the accommodation pattern because
we need the blocks always for halo particle transfer. The procedure to do that is, however,
exactly same as that shown in §4.10.19.

for (i=0; i<OH_NEIGHBORS; i++) {
const int dst=DstNeighbors[i], src=SrcNeighbors[i];
int rc;
MPI_Status st;
if (dst==me) {
RLIndex[i] = 0;  continue;
}
if (src>=0) {
RLIndex[i] = rlidx;
if (dst>=0)
MPI_Sendrecv(CommList, rlsize, T_Commlist, dst, 0,
CommList+rlidx, nn2, T_Commlist, src, 0, MCW, &st);
else
MPI_Recv(CommList+rlidx, nn2, T_Commlist, src, 0, MCW, &st);
MPI_Get_count(&st, T_Commlist, &rc);  rlidx += rc;
} else {
if (dst>=0)
MPI_Send(CommList, rlsize, T_Commlist, dst, 0, MCW);
RLIndex[i] = (src<-nn) ? rlidx : RLIndex[FirstNeighbor[i]];
}
}

The next and final step of the function, in which we broadcast primary receiving and
primary sending blocks to helpers including those after the helpand-helper reconfiguration
if it took place, is similar to level-4p. However there are two differences from level-4p. The
major one is that alternative secondary receiving block of a node is followed by alterna-
tive secondary sending block being the copy of primary sending block of the new helpand.
Therefore, we have AltSecRLIndex[3D + 1] whose element [k] (k < 3D) is the index of
the first record for the k-th neighbor of the helpand and [3D] is the size of the alterna-
tive secondary receiving/sending block, and broadcast this array to new helpers when the
helpand-helper reconfiguration took place instead of the size of primary receiving block.
The minor one is that this function does not return anything to its caller and thus it is
unnecessary to keep track of the tail of CommList[].

RLIndex[OH_NEIGHBORS] = rlidx;  SecRLIndex[OH_NEIGHBORS] = 0;
AltSecRList = SecRList = CommList + rlidx;  AltSecRLIndex[OH_NEIGHBORS] = 0;
if (Mode_PS(currmode)) {
oh1_broadcast(RLIndex, SecRLIndex, OH_NEIGHBORS+1, OH_NEIGHBORS+1,
MPI_INT, MPI_INT);
oh1_broadcast(CommList, SecRList, rlidx,
SecRLIndex[OH_NEIGHBORS], T_Commlist, T_Commlist);
AltSecRList += SecRLIndex[OH_NEIGHBORS];
}


<!-- Page 499 -->

if (reb) {
build_new_comm(currmode, -level, 2, stats);
update_descriptors(oldp, newp);
set_grid_descriptor(2, newp);
update_real_neighbors(URN_TRN, Mode_PS(currmode), oldp, newp);
oh1_broadcast(RLIndex, AltSecRLIndex, OH_NEIGHBORS+1, OH_NEIGHBORS+1,
MPI_INT, MPI_INT);
oh1_broadcast(CommList, AltSecRList, RLIndex[OH_NEIGHBORS],
AltSecRLIndex[OH_NEIGHBORS], T_Commlist, T_Commlist);
}
oh1_broadcast(NOfPGridTotal[0][0]+npgbase, NOfPGridTotal[1][0]+npgbase,
npgsize[0], npgsize[1], MPI_LONG_LONG_INT, MPI_LONG_LONG_INT);
}


#### 4.13.20 sched_recv()

sched_recv()  The function sched_recv(), called solely from make_recv_list() but |F(n)| times for
the local node n, scans per-plane histogram to determine the set of xy-planes,  i.e., the
subcuboid to be assigned to a node nid = mf being the f-th member of the local node’s
primary family, whose expected number of accommodating primary (tag = 0) or secondary
(tag = NS) particles is determined by the argumetns reb, get and stay. The scanning
and assignment context is kept in the S_recvsched_context structure argument context
whose elements and their definitions were given in §4.13.19.  This function is somewhat
similar to its level-4p counterpart shown in §4.10.20 but much simpler than it because
there are no hot-spots and the assignment unit is a xy-plane rather than a grid-voxel.


static void
sched_recv(const int reb, const int get, const int stay, const int nid,
const int tag, struct S_recvsched_context *context) {
const int z0=context->z;
dint nptotal=context->nptotal;
dint nplimit=context->nplimit;
struct S_commlist *cptr=context->cptr;
const int ns=nOfSpecies;
int z;
const int zz = GridDesc[0].z;

∑f
First, the function calculate PΛ(f) = PΛ(f −1) + Qnmf =   i=0 Qnmi where PΛ(f −1)
is given through the nplimit element of context, just depending on the reb argument
unlike the level-4p counterpart, but the calculation is logically equivalent to that shown
in §4.10.20 because Qnmf = get if reb or Qnmf = get + stay otherwise, as discussed in
§4.13.19.

if (reb)
nplimit += get;
else
nplimit += get + stay;

∑z0−1
Then after writing PΛ(f) back to context, we examine if PΣ(z0) =   z=0 PZ(z) ≥
PΛ(f) where z0 and PΣ(z0) are given through z and nptotal elements of context. If this


<!-- Page 500 -->

ineqaulity holds to mean that we don’t have any xy-planes to assign mf, we simply return
from this function without adding S_commlist record89.
Now we have some xy-planes to assign to mf and thus set S_commlist record elements,
rid to mf and tag to that given as the argument90.

context->nplimit = nplimit;
if (nptotal>=nplimit)  return;
cptr->rid = nid;  cptr->tag = tag;  cptr->sid = 0;  cptr->count = 0;

Now we scan per-plane histogram entries until we find z such that PΣ(z) ≥PΛ(f)91, to
let region element of the S_commlist record be such z, and then return to the caller writing
z + 1, PΣ(z) and the pointer to the next record back to context’s elements z, nptotal
and cptr respectively. Note that z found here is not necessary to satisfy z −z0 + 1 ≥eg
when eg > 1 to mean that the height of mf’s subcuboid can be less than eg and thus mf’s
horizontal exterior halo planes can be horizontal interior halo planes in two or more nodes.
Therefore, this function should be modified if we cope with eg > 1 cases with the second
solution shown in §4.13.6.

for (z=z0; z<zz; z++) {
nptotal += NOfPGridZ[z];
if (nptotal>=nplimit) {
cptr->region = z;  context->z = z + 1;
context->nptotal = nptotal;  context->cptr = cptr + 1;
return;
}
}
local_errstop("per-plane histogram total %d is less than the total particle "
"population %d up to node %d",
nptotal, nplimit, nid);
}


#### 4.13.21 make_send_sched()

make_send_sched()  The function make_send_sched(), called solely from exchange_particles4s(), scans pri-
mary receiving/sending and secondary receiving/sending blocks in PrimaryCommList[][] if
we will be in primary mode, or those in CommList[] possibly together with alternative sec-
ondary receiving/sending blocks otherwise, to determine the node to which the local node
n send the particles in each grid-voxel and the transfer schedule of those in horizontal halo
planes. The function is given the following arguments; reb being helpand-helper reconfigu-
ration indicator; the parent status code pcode; the identifiers of the old and (possibly) new
helpand oldp = npold and newp = npnew; the pointer pair to the head of primary receiving
and secondary receiving blocks rlist[2] being {PrimaryCommList[0], PrimaryCommList[1]}
or {CommList, SecRList}; the pair of index arrays rlidx[2] for primary receiving/sending
and secondary receiving/sending blocks being {PrimaryRLIndex[], PrimaryRLIndex[]} or
{RLIndex[], SecRLIndex[]}; an array nacc[2] to accumulate the number of primary parti-
cles ([0] = Qnn) and whole particles ([1] = Qn) to be accommodated by the local node;

89If the local node’s primary subdomain has no particles, this inequality holds at the first call of
sched recv() with z = 0 and f = 0, because PΣ(z) = PΛ(f) = 0. Therefore, the caller make recv list()
of sched recv() will have no S commlist records in primary receiving block in this case as discussed in
§4.13.19.
90The elements sid and count are meaningless but we let them be 0 to avoid to leave them undefined.
91Such z must be found because PΣ(δz(n)) = PΛ(|F(n)| −1).  Therefore,  if not found, we abort the
execution with local error stop()


<!-- Page 501 -->

and the pointer nsendptr to return the number of particles P nsend to be sent from the local
node.
What this function does  is somewhat similar to the level-4p counterpart shown in
§4.10.21, but its implementation is substantially different from the counterpart because
we don’t have hot-spots but have halo particle transfer.


static void
make_send_sched(const int reb, const int pcode, const int oldp,
const int newp, struct S_commlist *rlist[2],
int *rlidx[2], int *nacc, int *nsendptr) {
const int psold = Parent_Old(pcode) ? 1 : 0;
const int psnew = Parent_New(pcode) ? 1 : 0;
const int ns = nOfSpecies, ns2 = ns<<1,  nn = nOfNodes;
const int tagt = OH_NBR_TCC * ns,  tagb = OH_NBR_BCC * ns;
const int tag1 = OH_NEIGHBORS * ns;
int s, ps, n;
int nsend=0;


First, after clearing all elements of TotalPNext[2][S] as done in the level-4p counter-
part, we scan primary receiving/sending blocks in rlist[0] with indices in rlidx[0] always,
and then secondary receiving/sending blocks in rlist[1] with rlidx[1] if the local node has
helpand in the last step, i.e., if Parent Old(pcode) is true, to determine the node to accom-
modate particles in each xy-plane and thus each grid-voxel, by the following mechanisms to
visit each neighbor of the local node and its helpand by make_send_sched_body() also as
done in the level-4p counterpart; the head index of a sub-block for k-th neighbor is rlidx[k′]
where k′ = 3D −1−k because it corresponds to SrcNeighbors[] rather than Neighbors[p][];
we visit a neighbor twice or more if it occurred multiple times in Neighbors[p][] unless the
neighbor is the local node itself or its helpand; we explicitly skip inexistent neighbors such
that Neighbors[p][k] < −(N +1) to keep make_send_sched_body() from processing empty
sub-block. On the other hand, the procedure in make_send_sched_body() is quite simpler
than that in the level-4p counterpart as discussed later, because we don’t have hot-spots
and the almost all operations on grid-voxels in the local node’s subcuboid are performed by
other functions shown later together with those for halo particle transfer. Another differ-
ence is that the function returns the number of particles to be sent to other nodes, which
is accumulated in Pnsend .

for (s=0; s<ns2; s++)  TotalPNext[s] = 0;
for (ps=0; ps<=psold; ps++) {
const int root = ps ? oldp : myRank;
for (n=0; n<OH_NEIGHBORS; n++) {
const int nrev = OH_NEIGHBORS - 1 - n;
int sdid = Neighbors[ps][n];
if (sdid<0)  sdid = -(sdid+1);
if (sdid<nn && (n==OH_NBR_SELF || sdid!=root))
nsend += make_send_sched_body(ps, n, sdid, rlist[ps]+rlidx[ps][nrev]);
}
}

The next part is very level-4s’s own and is for subcuboids assigned to the local node
and halo particle transfer. We perform the following for primary subdomain and particles
(p = 0), and for secondary subdomain and particles (p = 1) in the next step if the local


<!-- Page 502 -->

node will have its helpand in the step, after initializing nacc[0] = nacc[1] = 0 as the base
of accumulation92.
First, we call make_send_sched_self() with arguments psor2 = p′, rlist = λ +
χ[⌊3D/2⌋] and naccptr pointing nacc[p].  If p = 1, npnew ̸= npold and npnew exists, p′ = 2
and λ = AltSecRList[] indexed by χ = AltSecRLIndex[] for the transitional state of the
helpand-helper configuration. Otherwise, i.e., p = 0 or p = 1 but npnew = npold or npnew is
inexistent, p′ = p and λ = rlist[p] whose indices are in χ = rlidx[p]. By this call, we
obtain the followings by the scan of primary receiving, secondary receiving or alternative
secondary receiving block and the per-grid histogram for interior grid-voxels; lower and
upper boundary of primary or secondary subcuboid in ZBound[p][]; halo particle transfer
schedule for the bottom/top surface of the subcuboid in HPlane[p][]; the number of particles
the local nodes will accommodate as primary ones or as the whole in nacc[p], for each species
in TotalPNext[p][], and for each species and grid-voxel in NOfPGridOut[p][][]. Note that we
give λ + χ[⌊3D/2⌋] instead of λ for the receiving block because it is not at the head of λ
when λ is PrimaryCommList[p].
Also Note that the local node can have no primary/secondary particles at all and, if
so, ZBound[p][β] = {0, 0}, HPlane[p][β].nbor = MPI_PROC_NULL93, nacc[p] = {0, nacc[0]}[p]
and TotalPNext[p][s] = 0 for all β ∈{0, 1} and s ∈[0, S) because they remain unchanged,
while NOfPGridOut[p][s][g]  is explicitly let be 0 for all g in the local node’s primary/
secondary subdomain including its exterior.  If this emptiness happens, we skip the fol-
lowing procedures for halo particle transfer because the local node does not do anything
for it.
Next we check if HPlane[p][β].nbor = N to mean the bottom/top surface of the local
node’s subcuboid is that of its subdomain.  If it holds for the bottom surface (β = 0) and
the corresponding true-bottom neighbor mb at the index kb = 31 +30 exsits, we replace the
HPlane[p][0].nbor with the rid element m′b of the last S_commlist record, whose region is
δz(mb)−1, of the sub-block for kb of the primary sending, secondary sending or alternative
secondary sending block in λ indexed by χ[k′b]. We let HPlane[p][0].stag be (p′ · 3D + kb)S
according to the tag element of the record being 0 (p′ = 0) or not (NS, p′ = 1) to indicate
that particles in horizontal interior halo plane of the local node’s subcuboid are sent to
m′b as its primary/secondary particles respectively.  If such neighbor does not exist, on
the other hand, because the bottom of the local node’s subdomain is also the non-periodic
bottom boundary of the system domain, we replace HPlane[p][0].nbor with MPI_PROC_NULL
to mean no halo particle transfer takes place through the bottom surface94.  Similarly, if
HPlane[p][1].nbor = N holds for the top surface, we let its nbor and stag have values
shown above, but with neighbor indices kt = 2 · 32 + 31 + 30, and referring to the first S_
commlist record of the kt’s sub-block in primary sending, secondary sending or alternative
secondary sending block.
The last operation of the loop for p is to let nacc[1] = nacc[0] if p = 0 to give nacc[1]
the base of accumulation, which can be its eventual value when the local node will not have
helpand in the next step.

nacc[0] = nacc[1] = 0;
for (ps=0; ps<=psnew; ps++) {

92Letting nacc[1] = 0 is necessary because the last operation of the loop for p to let nacc[1] be nacc[0]
can be skipped if the local node does not have primary subcuboid.
93As set by make send sched self(). Although MPI PROC NULL can be N with some MPI implementation,
no confusion should happen because iffZBounhd[p][1] = 0 then HPlane[p][β].nbor = MPI PROC NULL means
no communmication will take place for particles in horizontal halo planes rather than that the local node’s
subcuboid is the bottom/top one in its subdomain.
94And let .stag be kbS knowing it is not referred to but to avoid confusions in debugging etc.


<!-- Page 503 -->

int psor2;
int *nbors, *ri;
struct S_commlist *rl;
struct S_hplane *hp = HPlane[ps];
if (ps && Parent_New_Diff(pcode)) {
psor2 = 2;  nbors = Neighbors[2];  rl = AltSecRList;  ri = AltSecRLIndex;
} else {
psor2 = ps;  nbors = Neighbors[ps];  rl = rlist[ps];  ri = rlidx[ps];
}
make_send_sched_self(psor2, rl+ri[OH_NBR_SELF], nacc+ps);
if (ZBound[ps][OH_UPPER]==0)  continue;
if (hp[OH_LOWER].nbor==nn) {
int sdid = nbors[OH_NBR_BCC];
if (sdid<0)  sdid = -(sdid+1);
if (sdid<nn) {
const int zmax = (SubDomains[sdid][OH_DIM_Z][OH_UPPER] -
SubDomains[sdid][OH_DIM_Z][OH_LOWER]) - 1;
struct S_commlist *rlb = rl + ri[OH_NEIGHBORS-1-OH_NBR_BCC];
int rlz = rlb->region;
while (rlz<zmax)  rlz = (++rlb)->region;
hp[OH_LOWER].nbor = rlb->rid;
hp[OH_LOWER].stag = (rlb->tag) ? tagb + tag1 : tagb;
} else {
hp[OH_LOWER].nbor = MPI_PROC_NULL;
hp[OH_LOWER].stag = tagb;
}
}
if (hp[OH_UPPER].nbor==nn) {
int sdid = nbors[OH_NBR_TCC];
struct S_commlist *rlt = rl + ri[OH_NEIGHBORS-1-OH_NBR_TCC];
if (sdid<0)  sdid = -(sdid+1);
if (sdid<nn) {
hp[OH_UPPER].nbor = rlt->rid;
hp[OH_UPPER].stag = (rlt->tag) ?  tagt + tag1 : tagt;
} else {
hp[OH_UPPER].nbor = MPI_PROC_NULL;
hp[OH_UPPER].stag = tagt;
}
}
if (!ps)  nacc[1] = nacc[0];
}

Finally, we return P nsend through the argument pointer nsendptr.

*nsendptr = nsend;
}


4.13.22  Macros For_All_Grid_Z(), For_All_Grid_XY(),
Grid_Exterior_Boundary() and Grid_Interior_Boundary()

Here we show four macros used in the functions called from make_send_sched().

For_All_Grid_Z()  The macro pair of For All Grid Z(p, x0, y0, z0, x1, y1, z1) and For All Grid XY(p, x0, y0, x1,
For_All_Grid_XY()  y1) constracts triply nested for-loops as For_All_Grid() does, but the outermost z-loop


<!-- Page 504 -->

and inner double xy-loops are constructed seperatedly so that we have some codes special
to each z coordinate. The implementation of the macros is just a cut-and-paste of the body
of For_All_Grid(); its first For_Z() is in the former and the remaining For_Y() and for
construct are in the latter. The macro pair is used in make_send_sched_body() (the latter
is through the macro Make_Send_Sched_Body()), make_send_sched_self(), make_bsend_
sched(), make_brecv_sched(), xfer_boundary_particles_v() and exchange_border_
data_v().


#define For_All_Grid_Z(PS, X0, Y0, Z0, X1, Y1, Z1)\
For_Z((fag_zidx=(Z0), fag_x1=GridDesc[PS].x+(X1),\
fag_y1=GridDesc[PS].y+(Y1), fag_z1=GridDesc[PS].z+(Z1),\
fag_w=GridDesc[PS].w, fag_dw=GridDesc[PS].dw,\
fag_gz=Coord_To_Index(X0,Y0,Z0,fag_w,fag_dw)),\
(fag_zidx<fag_z1), (fag_zidx++,fag_gz+=fag_dw))
#define For_All_Grid_XY(PS, X0, Y0, X1, Y1)\
For_Y((fag_yidx=(Y0), fag_gy=fag_gz),\
(fag_yidx<fag_y1), (fag_yidx++,fag_gy+=fag_w))\
for (fag_xidx=(X0),fag_gx=fag_gy; fag_xidx<fag_x1; fag_xidx++,fag_gx++)


Grid_Exterior_Boundary()  Grid Exterior Boundary(νd, δd(n′), xld, xud) and Grid Interior Boundary(νd, δd(n′), xld,
Grid_Interior_Boundary()  xud) give d-th dimensional lower (xld) and upper (xud) bounds of an exterior or interior
region repectively of the local node n’s primary/secondary subdomain whose d-th dimen-
sional size is δd(n′) where n′ = n or n′ = parent(n), shared with a neighbor whose d-th
dimensional process coordinate relative to the subdomain is νd −1 ∈{−1, 0, 1}. Note that
the upper bound xud is relative to the subdomain’s upper bound δd(n′). Also note that the
former macro may gives the bounds of the subdomain itself rather than its exterior.
The lower and upper bounds xld and xlu + δd(n′) for an exterior resion are;

 (−eg, 0)             νd −1 = −1
(xld, xud + δd(n′)) =   (0, δd(n′))           νd −1 = 0                                
(δd(n′), δd(n′) + eg)  νd −1 = 1

while thier interior couterparts are;

 (0, eg)               νd −1 = −1
(xld, xud + δd(n′)) =   (0, δd(n′))           νd −1 = 0                                
(δd(n′) −eg, δd(n′))  νd −1 = 1

The macro Grid_Exterior_Boundary()  is used  in make_send_sched_body(), make_
brecv_sched(), xfer_boundary_particles_v() and exchange_border_data_v(), while
Grid_Interior_Boundary() is used in make_bsend_sched() and exchange_border_data_
v().


#define Grid_Exterior_Boundary(N, GS, PL, PU) {\
const int e = OH_PGRID_EXT;\
if (N==0)      { PL = -e;    PU = -(GS); }\
else if (N==1) { PL = 0;     PU = 0; }\
else           { PL = (GS);  PU = e; }\
}
#define Grid_Interior_Boundary(N, GS, PL, PU) {\


<!-- Page 505 -->

const int e = OH_PGRID_EXT;\
if (N==0)      { PL = 0;       PU = -(GS)+e; }\
else if (N==1) { PL = 0;       PU = 0; }\
else           { PL = (GS)-e;  PU = 0; }\
}



#### 4.13.23 make_send_sched_body()

Make_Send_Sched_Body()  Prior to discussing the function make_send_sched_body(), we show a macro Make Send
Sched Body(µ) used solely in the function. This macro scans NOfPGrid[p][s][g] for all s ∈
[0, S) and g ∈Sz = [xl, xu) × [yl, yu) × {z} in a xy-subplane at z in the exterior or interior
specified by (xl, yl) and (xu, yu) of the local node n’s primary (p = 0) or secondary (p = 1)
subdomain, where p, z, xl, xu, yl and yu are given from the invoker function implicitly
as its local variable ps, (invisible) fag zidx, xl, xu, yl and yu. For each s and g, we let
NOfPGrid[p][s][g] = 0 if the macro’s argument µ is true to mean the xy-subplane is in n’s
subcuboid.
Otherwise, since all particles in the subplane are sent to a node mf being the f-
th member of a neighbor family, we accumulate the sum of NOfPGrid[p][s][g] so that
NOfSend[pf][s][mf] = NOfSend[(pfS + s)N + mf] has the number of particles to be sent to
the node mf as its primary (pf = 0) or secondary(pf = 1) particles, where pfSN + mf is
given from the invoker function implicitly as its local variable nofsbase, and so that P send,′
given implicitly as nsend too, has the total number of particles sent from n to nodes in the
neighbor family. Then we let NOfPGrid[p][s][g] = (pfS + s)N + mf + 1 so that functions
referring to it finds NOfSend[pf][s][mf] quickly.
The reason why we define this macro is to avoid explicitly having two versions of similar
codes for µ = 0 and µ = 1 in make_send_sched_body() and at the same time to avoid
examining µ for each s and g. That is, the function just has two invocations of this macro
with µ = 0 and µ = 1 expecting that the two versions are eventually produced by macro
expansion and then unnecessary conditional construct examining µ for each s and g is
eliminated by compilers because µ in the construct is a constant 0 or 1.


#define Make_Send_Sched_Body(MYSELF) {\
int s, nofsidx=nofsbase;\
for (s=0; s<ns; s++,nofsidx+=nn) {\
dint *npg = NOfPGrid[ps][s];\
int nsendofs=0;\
For_All_Grid_XY(ps, xl, yl, xu, yu) {\
const int g = The_Grid();\
if (MYSELF)  npg[g] = 0;\
else {\
nsendofs += npg[g];  npg[g] = nofsidx + 1;\
}\
}\
nsend += nsendofs;  NOfSend[nofsidx] += nsendofs;\
}\
}


make_send_sched_body()  The function make_send_sched_body(), called solely from make_send_sched() but up to
2 · 3D times, scans a sub-block λ = rlist of primary receiving/sending (p = ps = 0)


<!-- Page 506 -->

or secondary receiving/sending (p = 1) block in PrimaryCommList[][] or CommList[]. The
sub-block is for a neighbor subdomain m = sdid having index k = n of the local node n’s
primary (p = 0) or secondary (p = 1) subdomain np = {n, parent(n)}[p]. The function
also scans the per-grid histogram elements in the exterior region of np being a part of its
neighbor subdomain m, or in np itself, to find the node in which particles in each grid-voxel
are accommodated and to return the number of particles in the region to be sent from n.
The function implements a part of what its level-4p counterpart shown in §4.10.22 does,
but the implementation is quite different from the counterpart.


static int
make_send_sched_body(const int ps, const int n, const int sdid,
struct S_commlist *rlist) {
const int me=myRank, ns=nOfSpecies, nn=nOfNodes;
const int nx = n % 3, ny = n/3 % 3, nz = n/9;
int xl, xu, yl, yu, zl, zu, zn;
int rlz = rlist->region, rid, ridp=-1, ridn=-1, nofsbase;
int nsend = 0;
const int zmax = (SubDomains[sdid][OH_DIM_Z][OH_UPPER] -
SubDomains[sdid][OH_DIM_Z][OH_LOWER]) - 1;
Decl_For_All_Grid();


At first, we determine the exterior or interior region of the subdomain np to be scanned,
∑D−1
S = [xl, xu) × [yl, yu) × [zl, zu) for the neighbor m having index k =   d=0 νd3d invok-
ing macro Grid_Exterior_Boundary() for each dimension d ∈[0, D). Then we skip S_
commlist records in λ until we find the record whose region element ζupf (mf) −1 satisfies
the following where mf being its rid element and pf is 0 or 1 according to its tag element
being 0 or NS respectively.
{
δz(m) −eg  νz −1 = −1
ζupf (mf) −1 ≥z′l =                                     0            νz −1 ∈{0, 1}

That is,  if m is located below np, we start from the node mf whose subcuboid contains
m’s top-side horizontal interior halo plane95. Otherwise, we simply start the node whose
subcuboid is the lowest in the family of m. Note that such record for the particle in the
xy-plane at zl should be found because the last record has δz(m) −1 ≥z′l.

Grid_Exterior_Boundary(nx, GridDesc[ps].x, xl, xu);
Grid_Exterior_Boundary(ny, GridDesc[ps].y, yl, yu);
Grid_Exterior_Boundary(nz, GridDesc[ps].z, zl, zu);
zn = (nz==0) ? zmax + 1 - OH_PGRID_EXT : 0;
while (rlz<zn)  rlz = (++rlist)->region;
rid = rlist->rid;  nofsbase = rlist->tag + rid;

Now we scan each xy-plane at z ∈[zl, zu) in the subdomain np and z′ = (z −zl) + z′l
of the subdomain m invoking Make_Send_Sched_Body() with µ = 1 if k = ⌊3D/2⌋and
mf = n to mean the plane is in n’s subcuboid, or µ = 0 otherwise. When µ = 1, the macro
lets NOfPGrid[p][s][g] = 0 for all s ∈[0, S) and g in the plane to mean that particles in
grid-voxels stay in n.

95The node mf  is the last one if eg = 1, but can be non-last when eg > 1 and must be the one whose
subcuboid is the lowest one among those having intersection with the planes.


<!-- Page 507 -->

When µ = 0, on the other hand, the macro accumulates the sum of NOfPGrid[p][s][g] for
each s and for all g so that, at the end of this function, NOfSend[pf][s][mf] has the number
of particles of species s to be sent to mf as its primary (pf = 0) or secondary (pf = 1)
particles, and so that Psend′   being the return value to the caller make_send_sched() has
the total number of particles to be sent from n to the family members of m. The macro
also lets NOfPGrid[p][s][g] = (pfS + s)N + mf + 1 after the accumulation so that functions
referring to it quickly find the entry NOfSend[pf][s][mf] = NOfSend[(pfS +s)N +mf] when
they move the particles in the grid-voxel to SendBuf[].
Then we visit the next S_commlist record for mf+1  if z′ = ζupf (mf) −1 and z′ <
δz(m) −1, or in other words z′ + 1 > ζupf (mf) −1 and z′ + 1 ≤δz(m) −1, updating pf and
mf according to the record.

For_All_Grid_Z(ps, xl, yl, zl, xu, yu, zu) {
if (n==OH_NBR_SELF && rid==me) {
Make_Send_Sched_Body(1);
}
else {
Make_Send_Sched_Body(0);
}
if (++zn>rlz && zn<=zmax) {
rlz = (++rlist)->region;  rid = rlist->rid;  nofsbase = rlist->tag + rid;
}
}
return(nsend);
}


#### 4.13.24 make_send_sched_self()

make_send_sched_self()  The function make_send_sched_self(), called solely from make_send_sched() but up to
twice, scans primary receiving (psor2 = p′ = p = 0), secondary receiving (p′ = p = 1)
or alternative secondary receiving (p′ = 2, p = 1) block in λ = rlist being a part of
PrimaryCommList[][] or CommList[], in order to have the lower/upper bound of the local
node n’s subcuboid in ZBound[p][β] = ζβp (n′) where n′ = {n, npold, npnew}[p′], and to have
the schedule of halo particle transfer through the horizontal surface of the subcuboid in
HPlane[p][β]. It also scans NOfPGridTotal[p][s][g] for all s ∈[0, S) and g in n’s subdomain
including its exterior, so that NOfPGridOut[p][s][g], TotalPNext[p][s] and Q = {Qnn, Qn}[p]
pointed by naccptr have appropriate values.
This function is very level-4s own and thus has no counterpart in level-4p.


static void
make_send_sched_self(const int psor2, struct S_commlist *rlist, int *naccptr) {
const int me=myRank, nn=nOfNodes, ns=nOfSpecies;
const int tag1 = OH_NEIGHBORS * ns;
const int tagt = OH_NBR_TCC * ns,  tagb = OH_NBR_BCC * ns;
const int ps = psor2==0 ? 0 : 1,  rtag = ps ? tag1 : 0;
const int exti = OH_PGRID_EXT;
const int zmax = GridDesc[psor2].z - 1;
int rlz = -1, rid = nn, ridp = -1, ridn, stag = 0;
struct S_hplane *hp = HPlane[ps];
int *zb = ZBound[ps];
int np = *naccptr,  *tpn = TotalPNext + (ps ? ns : 0);


<!-- Page 508 -->

int s;
Decl_For_All_Grid();


At first we initialize HPlane[p][β].nbor = MPI_PROC_NULL for both β ∈{0, 1} in case
n’s subcuboid is empty and thus no halo particle transfer takes place.  Then we scan
each xy-plane at z ∈[−eg, δz(n′) + eg), with initial setting m−1 = N, m−2 = −1 and
ζup−1(m−1) −1 = −1 to mean that bottom exterior of n’s subdomain up to z = −1 is not
assigned to any n’s family members but to a family member of the subdomain below it
if exist (m−1 = N) and we don’t care about the second-top member of the bottom-side
subdomain (m−2 = −1).
In the scanning loop body, we first examine if z = ζupf (mf) −1 (f = −1 at initial) to
mean we reach to the top surface of mf’s subcuboid. If so, we let mf+1 be rid element of
the record we will visit next if z < δz(n′)−1, or N to mean the subcuboid above the surface
is for a node in the family for the subdomain above n’s one. Otherwise, we let mf+1 = −1
to mean we don’t yet care about the subcuboid above that we are now visiting.
Next we examine mf−1 = n or mf+1 = n.   If the former holds to mean we are
now visiting the bottom surface of a subcuboid of mf just above n’s subcuboid, we let
ZBound[p][1] = ζup (n) = z and HPlane[p][1].nbor = mf to mean the particles in the upper
horizontal halo planes are exchanged with mf. We also let stag and rtag elements of
HPlane[p][1] be (pf · 3D + (2 · 32 + 31 + 30))S and (p · 3D + (31 + 30))S so that the sending/
receiving communmication for species s is associated with a unique point in a conceptual
5-dimensional space of [2][3][3][3][S] being [pf][2][1][1][s] for the former and [p][0][1][1][s] for
latter respectively, where pf is 0 or 1 according to tag element of currently visiting record
being 0 or NS respectively, or 0  if mf = N to mean the current xy-plane is in upper
exterior of n’s subdomain.
On the other hand,  if mf+1 = n holds to mean we are now visiting the top surface
of a subcuboid of mf  just below n’s subcuboid, we let ZBound[p][0] = ζlp(n) = z + 1
and HPlane[p][0].nbor = mf to mean the particles in the lower horizontal halo plane are
exchanged with mf. We also let stag and rtag elements of HPlane[p][0] be (pf ·3D+(2·32+
31 + 30))S and (p · 3D + (31 + 30))S to associate communicatetions for s with [pf][0][1][1][s]
and [p][2][1][1][s] respectively. Note that pf = 0 if mf = N too but this means the current
xy-plane is in lower exterior of n’s subdomain.

hp[OH_LOWER].nbor = hp[OH_UPPER].nbor = MPI_PROC_NULL;
For_All_Grid_Z(psor2, -exti, -exti, -exti, exti, exti, exti) {
const int z = Grid_Z();
ridn = (z==rlz) ? (z<zmax ? rlist->rid : nn) : -1;
if (ridp==me) {
zb[OH_UPPER] = z;  hp[OH_UPPER].nbor = rid;
hp[OH_UPPER].stag = stag + tagt;
hp[OH_UPPER].rtag = rtag + tagb;
} else if (ridn==me) {
zb[OH_LOWER] = z + 1;  hp[OH_LOWER].nbor = rid;
hp[OH_LOWER].stag = stag + tagb;
hp[OH_LOWER].rtag = rtag + tagt;
}

Then we examine if mf = n to mean we are visiting a xy-plane in n’s subcuboid. If so
we further examine if mf−1 ≥0 or mf+1 ≥0 to mean the plane is the bottom or top surface
of the subcuboid and, if either of them holds, call make_send_sched_hplane() giving p′,
z, the pointer to Q, and the element arrays nsend and sbuf of HPlane[p][β] where β = 0


<!-- Page 509 -->

for the former and β = 1 for the latter. By this call, besides it lets PO(p, s, g) = PT (p, s, g)
for all s ∈[0, S) and g ∈Sz where

Sz = [−eg, δx(n′)+eg) × [−eg, δy(n′)+eg) × {z}

we have the followings.
∑
nsend[s] = P′Z(p, s, z) =    PT (p, s, g)
g∈Sz
z0 = ζlp(n′) −1
∑z
TotalPNext[p][s] = Q(p, s, z) =     P′Z(p, s, z′)
z′=z0
sbuf[s] = Q(p, s, z−1)
S−1∑
Q = {0, Qnn}[p] +    Q(p, s, z)
s=0

Note that sbuf[s] above is so far the offset to the head of hbuf sh(p, β, s) from the head of
pbuf (p, s) in SendBuf[].
Also note that mf−1 ≥0 and mf+1 ≥0 may hold at the same time if n’s subcuboid
is one-grid thick. Even if so, we call make_send_sched_hplane() just once for β = 0 and
then copy all elements in nsend and sbuf elements of HPlane[p][0] into those of HPlane[p][1]
because the amount of particles to be sent to mf−1 and mf+1 and their locations in the
particle buffer are equivalent.
If neither mf−1 ≥0 nor mf+1 ≥0 holds, on the other hand, we  call make_
send_sched_hplane() too but giving NULL instead of nsend and sbuf just for updating
NOfPGridOut[p][][], TotalPNext[p][] and Q.

if (rid==me) {
if (ridp>=0) {
make_send_sched_hplane(psor2, z, naccptr,
hp[OH_LOWER].nsend, hp[OH_LOWER].sbuf);
if (ridn>=0) {
for (s=0; s<ns; s++) {
hp[OH_UPPER].nsend[s] = hp[OH_LOWER].nsend[s];
hp[OH_UPPER].sbuf[s] = hp[OH_LOWER].sbuf[s];
}
}
}
else if (ridn>=0)
make_send_sched_hplane(psor2, z, naccptr,
hp[OH_UPPER].nsend, hp[OH_UPPER].sbuf);
else
make_send_sched_hplane(psor2, z, naccptr, NULL, NULL);

If mf  ̸= n, again we examine  if mf−1 = n or mf+1 = n to mean the xy-plane we
are visiting is just above or below the subcuboid of n and thus upper or lower horizontal
exterior halo plane.  If either of them holds, we call make_send_sched_hplane() to have
P′Z(p, s, z) in nrecv[s] and Q(p, s, z−1) in rbuf[s] of HPlane[p][β] where β = 1 if mf−1 = n
and β = 0 mf+1 = n, because n receives halo particles into hbuf rh(p, β, s) and thus they
are accommodated by n.


<!-- Page 510 -->

If neither mf−1 = n nor mf+1 = n holds, on the other hand, we simply  let
NOfPGridOut[p][s][g] = 0 for all s ∈[0, S) and g in the plane, because n does not have
any particles in the plane.

} else {
if (ridp==me)
make_send_sched_hplane(psor2, z, naccptr,
hp[OH_UPPER].nrecv, hp[OH_UPPER].rbuf);
else if (ridn==me)
make_send_sched_hplane(psor2, z, naccptr,
hp[OH_LOWER].nrecv, hp[OH_LOWER].rbuf);
else {
for (s=0; s<ns; s++) {
int *npgo=NOfPGridOut[ps][s];
For_All_Grid_XY(psor2, -exti, -exti, exti, exti)
npgo[The_Grid()] = 0;
}
}
}

At the end of the scanning loop body, we examine if z = ζupf (mf) −1 again and,  if
so to mean the next xy-plane is for the bottom of mf+1’s subcuboid, we let mf−1 = mf,
and step to the next f letting mf be rid element of the record in λ and pf be 0 or 1
according to its tag element being 0 or NS respectively if z < δz(n′) −1, or let mf = N
and pf = 0 otherwise.  If z ̸= ζupf (mf) −1, on the other hand, the next xy-plane is still in
mf’s subcuboid and thus we let mf−1 = −1.

ridp = -1;
if (z==rlz) {
ridp = rid;
if (z<zmax) {
rlz = rlist->region;  rid = rlist->rid;
stag = rlist->tag ? tag1 : 0;  rlist++;
} else {
rlz++;  rid = nn;  stag = 0;
}
}
}

After the scanning loop, HPlane[p][β].{sbuf, rbuf}[s] has the offset from the head of
∑s−1
pbuf (p, s) whose index in SendBuf[] is Q0 +   t=0 TotalPNext[p][s] where Q0 = {0, Qnn}[p]
and is Q at the begining of this function. Therefore, we calculate the index of pbuf (p, s) for
each s and add it to HPlane[p][β].b[s] for each β ∈{0, 1} and b ∈{sbuf, rbuf} so that they
have the indices of SendBuf[] for the send/receive buffers, hbuf sh(p, β, s) and hbuf rh(p, β, s)
respectively.

for (s=0; s<ns; s++) {
hp[OH_LOWER].sbuf[s] += np;  hp[OH_LOWER].rbuf[s] += np;
hp[OH_UPPER].sbuf[s] += np;  hp[OH_UPPER].rbuf[s] += np;
np += tpn[s];
}
}


<!-- Page 511 -->

#### 4.13.25 make_send_sched_hplane()

For_All_Grid_XY_At_Z()  Prior to discussing make_send_sched_hplane(), we show the macro For All Grid XY At Z
(p, x0, y0, x1, y1, z) solely used in the function. This macro is a relative of For_All_Grid_
XY() but the z-coordinate value of the xy-plane to be scanned is given explicitly by z.
Therefore, this macro is equivalent to For All Grid(p, x0, y0, z, x1, y1, z′) where z′ = z +
1 −δz({n, parent(n)}[p]) for the local node n.


#define For_All_Grid_XY_At_Z(PS, X0, Y0, X1, Y1, Z0)\
For_Z((fag_zidx=(Z0), fag_x1=GridDesc[PS].x+(X1),\
fag_y1=GridDesc[PS].y+(Y1), fag_z1=(Z0)+1,\
fag_w=GridDesc[PS].w, fag_dw=GridDesc[PS].dw,\
fag_gz=Coord_To_Index(X0,Y0,Z0,fag_w,fag_dw)),\
(fag_zidx<fag_z1), (fag_zidx++,fag_gz+=fag_dw))\
For_Y((fag_yidx=(Y0), fag_gy=fag_gz),\
(fag_yidx<fag_y1), (fag_yidx++,fag_gy+=fag_w))\
for (fag_xidx=(X0),fag_gx=fag_gy; fag_xidx<fag_x1; fag_xidx++,fag_gx++)


make_send_sched_hplane()  The function make_send_sched_hplane(), called solely from make_send_sched_self()
but possibly for each xy-plane in the local node n’s subdomain and its exterior, scans
PT (p, s, g) = NOfPGridTotal[p][s][g] for all s ∈[0, S) and

g ∈Sz = [−eg, δx(n′)+eg) × [−eg, δy(n′)+eg) × {z = z}

where p = {0, 1, 1}[p′ = psor2] and n′ = {n, npold, npnew}[p′], and copies each of them into
PO(p, s, g) = NOfPGridOut[p][s][g] because z  is in the subcuboid of n′ or its horizontal                                  ∑
exterior halo planes.  It also calculate P′Z(p, s, z) =   g∈Sz PT (p, s, g) for each s to let
HPlane[p][β].{nsend, nrecv}[s] be the sum if the argument np is not NULL and thus points
it. The sum is also added to Q(p, s, z) being the so-far value of TotalPNext[p][s] and to
Q being Qnn or Qn pointed by naccptr.  In addition,  if the argument buf is not NULL
and thus points HPlane[p][β].{sbuf, rbuf}[s], it is let be Q(p, s, z−1) or in other words
TotalPNext[p][s] before the addition and thus have the offset from pbuf (p, s) to the send/
receive buffer of halo particles, i.e., hbuf sh(p, β, s) or hbuf rh(p, β, s) respectively.
This function is very level-4s’s own and thus has no counterpart in level-4p.


static void
make_send_sched_hplane(const int psor2, const int z, int *naccptr,
int *np, int *buf) {
const int ns=nOfSpecies, exti=OH_PGRID_EXT;
const int ps = psor2==0 ? 0 : 1,  nsor0 = ps ? ns : 0;
int nacc = *naccptr, s;
Decl_For_All_Grid();

for (s=0; s<ns; s++) {
dint *npgt = NOfPGridTotal[ps][s];
int *npgo = NOfPGridOut[ps][s];
int npofs = 0;
if (buf)  buf[s] = TotalPNext[nsor0+s];
For_All_Grid_XY_At_Z(psor2, -exti, -exti, exti, exti, z) {
const int g = The_Grid();
npofs += (npgo[g] = npgt[g]);


<!-- Page 512 -->

}
nacc += npofs;  TotalPNext[nsor0+s] += npofs;
if (np)  np[s] = npofs;
}
*naccptr = nacc;
}


#### 4.13.26 update_descriptors()

update_descriptors()  The function update_descriptors(), called from exchange_particles4s() when we
had  anywhere accommodation and  from make_recv_list()  otherwise,  reinitializes
BorderExc[][1][][].{send, recv} for the old secondary subdomain given through the argu-
ment npold = oldp by clear_border_exchange(), and update FieldDesc[].{bc,red}.
size[1] for the new secondary subdomain given through the argument npnew = newp by
calling set_field_descriptors() giving it arrays FieldTypes[][ and SubDomains[m][][].
It also calls adjust_field_descriptor() to modify FieldDesc[F−1].{bc,red}.size[1]
for per-grid histograms giving it ps = 1.
Note that we do above if the old and new parents are different, and call clear_border_
exchange() if the old one exists, while other two functions are called if the new one exists.
Also note that this function is perfectly equivalent to its level-4p counterpart shown in
§4.10.29


static void
update_descriptors(const int oldp, const int newp) {
int n;

if (oldp!=newp) {
if (oldp>=0)  clear_border_exchange();
if (newp>=0) {
set_field_descriptors(FieldTypes, SubDomains[newp], 1);
adjust_field_descriptor(1);
}
}
}


#### 4.13.27 update_neighbors()

Neighbor_Grid_Offset()  The macro Neighbor Grid Offset(p, νd−1, m, d, c), used three times  in the function
update_neighbors() solely as discussed later in this section, calculates

 δld(m) −δud(m) = −δd(m)  νd = 0
x0d(m, np) = δld(m) −δld(np) =    δld(np) −δld(np) = 0        νd = 1                                 
δud(np) −δld(np) = δd(np)   νd = 2

where np = {n, parent(n)}[p] for the local node n, to update GridOffset[p][k] where k =
∑D−1
d=0 νd3d, as discussed in §4.9.5 and in §4.10.30 for the level-4p counterpart perfectly
equivalent to this macro.


#define Neighbor_Grid_Offset(PS, N, SD, D, XYZ)\
(N==0 ? 0 : (N<0 ? SubDomains[SD][D][OH_LOWER]-SubDomains[SD][D][OH_UPPER] :\
GridDesc[ps].XYZ))


<!-- Page 513 -->

update_neighbors()  The function update_neighbors() is called from init4s() with p = ps = 0, and from
rebalance4s() or exchange_particles4s() with p = 1 when we had normal or anywhere
accommodation respectively. The function initializes/updates AbsNeighbors[p][k] for all
k ∈[0, 3D) to let it have;
{
Neighbors[p][k]         Neighbors[p][k] ≥0
AbsNeighbors[p][k] = mk =
−(Neighbors[p][k] + 1)  Neighbors[p][k] < 0

and  lets GridOffset[p][k] =  gidx(x00(mk, np), . . .) where np =  {n, parent(n)}[p] and
x0d(mk, np) is given by Neighbor_Grid_Offset(), as discussed in §4.9.5 and in §4.10.30
for the level-4p coutnerpart of the function. However in addition to those initializations,
the function has level-4s’s own ones for PrimaryCommList[p][k′] where k′ = 3D −1 −k for
each k to let its elements rid = mk, tag = 0, and region = δz(mk) −196. Another small
difference from the counterpart is in the outermost two loops whose coding is simplified
exploiting the fact D = 3.


static void
update_neighbors(const int ps) {
int n, nx, ny, nz;
const int nn = nOfNodes;
struct S_commlist *cl = PrimaryCommList[ps];

for (nz=-1,n=0; nz<2; nz++) {
for (ny=-1; ny<2; ny++) {
for (nx=-1; nx<2; nx++,n++) {
int nbr = Neighbors[ps][n];
const int nrev = OH_NEIGHBORS - 1 - n;
nbr = AbsNeighbors[ps][n] = nbr<0 ? -(nbr+1) : nbr;
cl[nrev].rid = nbr;  cl[nrev].tag = cl[nrev].sid = cl[nrev].count = 0;
if (nbr>=nn) {
GridOffset[ps][n] = 0;  cl[nrev].region = 0;
} else {
GridOffset[ps][n] =
Coord_To_Index(Neighbor_Grid_Offset(ps, nx, nbr, OH_DIM_X, x),
Neighbor_Grid_Offset(ps, ny, nbr, OH_DIM_Y, y),
Neighbor_Grid_Offset(ps, nz, nbr, OH_DIM_Z, z),
GridDesc[0].w, GridDesc[0].dw);
cl[nrev].region = SubDomains[nbr][OH_DIM_Z][OH_UPPER] -
SubDomains[nbr][OH_DIM_Z][OH_LOWER] - 1;
}
}
}
}
}


#### 4.13.28 set_grid_descriptor()

set_grid_descriptor()  The function set_grid_descriptor() is called from init4s() with idx = i = 0 and
nid = m = n arguments for the local node n for the initialization, from rebalance4s() or

96We also let unused elements sid and count be 0 to avoid leaving them undefined.


<!-- Page 514 -->

exchange_particles4s() with i = 1 and m = parent(n) when we had normal or anywhere
accommodation respectively, and from make_recv_list() with i = 2 and m = parent(n),
when helpand-helper reconfiguration is taking place assigning m to the local node as its
secondary subdomain. The function lets GridDesc[i] have the shape information of the
per-grid histogram for the subdomain m. Note that GridDesc[2] is used to have the shape
information of new parent(n) due to helpand-helper reconfiguration while [1] keeps that of
old parent(n).
The function is very similar to its level-4p counterpart shown in §4.10.31, but is different
from it because the per-grid histogram has 3-grid thick exterior, one for sending and two
for receiving as discussed in §4.12.3. Therefore, the elements w, d and h of GridDesc[i] is
let be δmaxd  + 6eg = Grid[d].size + 6 · OH_PGRID_EXT with d = 0, 1 and 2 respectively for
the physical array size, and dw be d × w, if D = 3. Similarly, the elements of x, y and z for
non-existent secondary subdomain are let be −6eg.


static void
set_grid_descriptor(const int idx, const int nid) {
const int exti6 = OH_PGRID_EXT*6;
const int w = GridDesc[idx].w = Grid[OH_DIM_X].size+(exti6);
const int d = GridDesc[idx].d =
If_Dim(OH_DIM_Y, Grid[OH_DIM_Y].size+(exti6), 1);

GridDesc[idx].h = If_Dim(OH_DIM_Z, Grid[OH_DIM_Z].size+(exti6), 1);
GridDesc[idx].dw = d * w;
if (nid>=0) {
GridDesc[idx].x = SubDomains[nid][OH_DIM_X][OH_UPPER] -
SubDomains[nid][OH_DIM_X][OH_LOWER];
GridDesc[idx].y = If_Dim(OH_DIM_Y,
SubDomains[nid][OH_DIM_Y][OH_UPPER] -
SubDomains[nid][OH_DIM_Y][OH_LOWER], 0);
GridDesc[idx].z = If_Dim(OH_DIM_Z,
SubDomains[nid][OH_DIM_Z][OH_UPPER] -
SubDomains[nid][OH_DIM_Z][OH_LOWER], 0);
} else {
GridDesc[idx].x = GridDesc[idx].y = GridDesc[idx].z = -exti6;
/* to ensure, e.g., x+3*(OH_PGRID_EXT)<=-3*(OH_PGRID_EXT) */
}
}


#### 4.13.29 adjust_field_descriptor()

adjust_field_descriptor()  The function adjust_field_descriptor() is called from init4s() with argument ps =
p = 0 for the initialization of the local node n’s primary subdomain, and from update_
descriptors() with p = 1 for n’s secondary subdomain newly assigned to it by helpand-
helper reconfiguration. The function is perfectly equivalent to its level-4p counterpart and
thus modifies FieldDesc[F−1].{bc, red}.size[p] as discussed in §4.10.32.


static void
adjust_field_descriptor(const int ps) {
const int f = nOfFields - 1,  ns = nOfSpecies;
int d, fs;


<!-- Page 515 -->

for (d=0,fs=1; d<OH_DIMENSION; d++)  fs *= FieldDesc[f].size[d];
fs *= ns-1;
FieldDesc[f].bc.size[ps] += fs;    FieldDesc[f].red.size[ps] += fs;
}


#### 4.13.30 update_real_neighbors()

update_real_neighbors()  The  function update_real_neighbors(),  called from init4s(),  try_primary4s(),
exchange_particles4s() and make_recv_list(), updates RealDstNeighbors[][] and
RealSrcNeighbors[][] according to its mode argument to specify the elements to be up-
dated, dosec to specify whether nodes have helpers or not, and oldp and newp being old
and new parent(n) of the local node n on helpand-helper reconfiguration.
This function is perfectly equivalent to its level-4p counterpart shown in §4.10.33


static void
update_real_neighbors(const int mode, const int dosec, const int oldp,
const int newp) {
const int me=myRank, nn=nOfNodes, nn4=nn<<2;
const int dosec0 = mode != URN_PRI;
int i, nbridx, ps, *doccur[2], *soccur[2];

for (i=0; i<nn4; i++)  TempArray[i] = 0;
doccur[0] = TempArray;       doccur[1] = doccur[0] + nn;
soccur[0] = doccur[1] + nn;  soccur[1] = soccur[0] + nn;

if (mode==URN_TRN) {
int *tmp = RealSrcNeighbors[1][0].nbor;
RealSrcNeighbors[1][0].n = RealSrcNeighbors[0][0].n;
RealSrcNeighbors[1][0].nbor = RealSrcNeighbors[0][0].nbor;
RealSrcNeighbors[0][0].nbor = tmp;
}
RealDstNeighbors[0][0].n = RealDstNeighbors[0][1].n = 0;
RealSrcNeighbors[0][0].n = RealSrcNeighbors[0][1].n = 0;
upd_real_nbr(me, 0, 1, 0, dosec0, Nodes, RealDstNeighbors[0], doccur);
upd_real_nbr(me, 0, 0, 0, dosec0, Nodes, RealSrcNeighbors[0], soccur);
if (mode==URN_PRI)  return;

nbridx = mode==URN_TRN ? 2 : 1;
upd_real_nbr(newp, 0, 1, nbridx, 1, Nodes, RealDstNeighbors[0], doccur);
upd_real_nbr(newp, 1, 1, nbridx, 1, Nodes, RealSrcNeighbors[0], soccur);
if (mode!=URN_TRN)  return;

for (ps=0; ps<2; ps++) {
const int nd = RealDstNeighbors[0][ps].n;
const int ns = RealSrcNeighbors[0][ps].n;
for (i=0; i<nd; i++)  doccur[ps][RealDstNeighbors[0][ps].nbor[i]] = 0;
for (i=0; i<ns; i++)  soccur[ps][RealSrcNeighbors[0][ps].nbor[i]] = 0;
}
RealDstNeighbors[1][0].n = RealDstNeighbors[1][1].n = 0;
RealSrcNeighbors[1][1].n = 0;
upd_real_nbr(me,   0, 1, 0, 1,     Nodes,     RealDstNeighbors[1], doccur);
upd_real_nbr(oldp, 0, 1, 1, 1,     Nodes,     RealDstNeighbors[1], doccur);
upd_real_nbr(newp, 1, 1, 2, dosec, NodesNext, RealSrcNeighbors[1], soccur);


<!-- Page 516 -->

}


#### 4.13.31 upd_real_nbr()

upd_real_nbr()  The function upd_real_nbr(), called solely from update_real_neighbors() but up to
seven times, to add members to RealDstNeighbors[][] or RealSrcNeighbors[][]. The func-
tion is perfectly equivalent to its level-4p counterpart shown in §4.10.34.


static void
upd_real_nbr(const int root, const int psp, const int pss,
const int nbr, const int dosec, struct S_node *nodes,
struct S_realneighbor rnbrptr[2], int *occur[2]) {
const int me=myRank;
struct S_realneighbor *pnbr = rnbrptr+psp,  *snbr = rnbrptr+pss;
int *poccur = occur[psp],  *soccur = occur[pss];
int i;

if (root<0)  return;
if (root!=me && !poccur[root]) {
pnbr->nbor[pnbr->n++] = root;  poccur[root] = 1;
}
if (dosec) {
struct S_node *ch;
for (ch=nodes[root].child; ch; ch=ch->sibling) {
const int nid = ch->id;
if (nid!=me && !soccur[nid]) {
snbr->nbor[snbr->n++] = nid;  soccur[nid] = 1;
}
}
}
for (i=0; i<OH_NEIGHBORS; i++) {
const int nid = Neighbors[nbr][i];
struct S_node *ch;
if (nid<0 || nid==root)  continue;
if (!poccur[nid]) {
pnbr->nbor[pnbr->n++] = nid;  poccur[nid] = 1;
}
if (dosec) {
for (ch=nodes[nid].child; ch; ch=ch->sibling) {
const int cid = ch->id;
if (!soccur[cid]) {
snbr->nbor[snbr->n++] = cid;  soccur[cid] = 1;
}
}
}
}
}


#### 4.13.32 exchange_xfer_amount()

exchange_xfer_amount()  The function exchange_xfer_amount(), called solely from exchange_particles4s(), ex-
changes NOfSend[][][] in the nodes responsible of a subdomain and its neighbors as the


<!-- Page 517 -->

nodes’ primary/secondary subdomain to have NOfRecv[][][] for position-aware particle trans-
fer. The function is very similar to its level-4p counterpart shown in §4.10.35, but slightly
different from it because this function is called not only when we will be in secondary mode
but also for primary mode. Therefore, the function is given an argument p′n = nextmode
to indicate we will be in primary (p′n = 0) or secondary (p′n = 1) mode in addition to two
arguments equivalent to the counterpart; trans = t ∈{0, 1} to mean we have stable (t = 0)
or transitional (t = 1) state of helpand-helper configuration; and psnew = pn ∈{0, 1} being
1 iffthe local node will have a secondary subdomain and thus will receive some particles.


static void
exchange_xfer_amount(const int trans, const int psnew, const int nextmode) {
const struct S_realneighbor *snbr = RealSrcNeighbors[trans];
const struct S_realneighbor *dnbr = RealDstNeighbors[trans];
const int nnns = nOfNodes * nOfSpecies;
int ps, tag, req;


The first part of this function  is perfectly equivalent to that in the level-4p coun-
terpart, and thus posts MPI_Irecv() to receive NOfRecv[p][s][m] from  all nodes m ∈
RealSrcNeighbors[t][p].nbor[] for p ∈{0, pn}.

for (ps=0,tag=0,req=0; ps<=psnew; ps++,tag+=nnns) {
const int n = snbr[ps].n;
const int *nbor = snbr[ps].nbor;
int i,  *nrbase = NOfRecv + tag;
for (i=0; i<n; i++,req++) {
const int nid = nbor[i];
MPI_Irecv(nrbase+nid, 1, T_Hgramhalf, nid, tag, MCW, Requests+req);
}
}

The next part is slightly different from that of the counterpart because it sends local
node n’s NOfSend[p][s][m] to all nodes m ∈RealDstNeighbors[t][p].nbor[] for p ∈{0, p′n},
instead of p ∈{0, 1}, by MPI_Isend().

for (ps=0,tag=0; ps<=nextmode; ps++,tag+=nnns) {
const int n = dnbr[ps].n;
const int *nbor = dnbr[ps].nbor;
int i,  *nsbase = NOfSend + tag;
for (i=0; i<n; i++,req++) {
const int nid = nbor[i];
MPI_Isend(nsbase+nid, 1, T_Hgramhalf, nid, tag, MCW, Requests+req);
}
}

The last part is perfectly equivalent to that of the counterpart again, and thus confirms
the completion of all MPI_Irecv() and MPI_Isend() calls recorded in Requests[] by MPI_
Waitall() to have their completion status in Statuses[] (but not referring to).

MPI_Waitall(req, Requests, Statuses);
}


<!-- Page 518 -->

#### 4.13.33 make_bxfer_sched()

make_bxfer_sched()  The function make_bxfer_sched(), called solely from exchange_particles4s(), scans
primary receiving/sending and secondary receiving/sending blocks in PrimaryCommList[][]
if we will be in primary mode, or those in CommList[] possibly together with alternative
secondary receiving/sending blocks otherwise, to build the halo particle transfer sched-
ule for those in vertical halo planes.  The function  is given the following arguments;
trans = t ∈{0, 1} is 1 iffwe have helpand-helper reconfiguration with normal accom-
modation; psnew = pn ∈{0, 1} is 1 iffthe local node will have helpand in the next step; the
pointer pair to the head of primary receiving and secondary receiving blocks rlist[2] = λ[2]
being {PrimaryCommList[0], PrimaryCommList[1]} or {CommList, SecRList}; and the pair
of index arrays rlidx[2] = χ[2] for primary receiving/sending and secondary receiving/
sending blocks being {PrimaryRLIndex[], PrimaryRLIndex[]} or {RLIndex[], SecRLIndex[]}.
This function is very level-4s own and thus has no counterpart in level-4p.

static void
make_bxfer_sched(const int trans, const int psnew, struct S_commlist *rlist[2],
int *rlidx[2]) {
const int nn = nOfNodes;
int d, ps, du;
int (*vph)[2][2] = (int(*)[2][2])VPlaneHead;
int nsendib=0, nrecveb=0, vpidx=0;


This function calls make_bsend_sched() for sending schedule and make_brecv_sched()
for receiving schedule up to eight times for each in a triply nested loop for all d ∈{0, 1},
p ∈{0, pn} and β ∈{0, 1} in this order, to have the transfer schedule for j-th node
responsible of contacting subcuboid in a subdomain being lower (β = 0) or upper (β = 1)
neighbor of the local node n’s primary (p = 0) or secondary (p = 1) subdomain along
x (d = 0) or y (d = 1) axis in VPlane[i(d, p, β, j)]. The functions are called if the local
node has primary/secondary subcuboid and the neighbor exists, and commonly given the
following arguments.

- psor2 = p′ = 2 if p = 1 and t = 1, or p′ = p otherwise.

- nx = νx = {2β, 1}[d], ny = νy = {1, 2β}[d], and n = k = 32 + νy · 31 + νx · 30 for
neighbor index.

- rlist = λ[p][χ[p][k′]] if p′ ̸= 2, or AltSecRList[AltSecRLIndex[k′]] otherwise, where
k′ = 3D −k −1.

The function make_bsend_sched() is also given pointers nsendptr and vpptr pointing
local variables to let it know the index Hs of BoundarySendBuf[] for hbuf sv(d, p, β, m0) and
the index V of VPlane[] being i(d, p, β, 0), and to let it add the total size of the buffers and
the number of contacting nodes in the neighbor family to the variables. Similarly, make_
brecv_sched() is given a pointer nrecvptr pointing a local varible of Particles[]’s index
Hr for hbuf rv(d, p, β, m0) and for the addition of total size to it, but the other argument
vpidx simply carries i(d, p, β, 0).
In addition to build the schedules in VPlane[], make_bsend_sched() lets NOfPGrid[][][]
for grid-voxels in vertical interior halo planes have negative values representing indices
of BoundarySendBuf[], while make_brecv_sched() lets array elements for grid-voxels in
exterior pillars have values greater than 232 for indices of the buffer.


<!-- Page 519 -->

In addition to calling two functions, this function lets VPlaneHead[d][p][β] = i(d, p, β, 0)
for all d ∈{0, 1}, p ∈{0, 1} and β ∈{0, 1}, as well as its last element [2][0][0]. This assign-
ment is done even for p such that p > pn or ZBound[p][1] = 0 to mean inexistent subcuboid,
as well as for inexistent neighbors. Therefore, we can keep xfer_boundary_particles_v()
and exchange_border_data_v() from scanning grid-voxels in a vertical exterior halo plane
for copying particles from hbuf rv(d, p, β, m) for efficiency, and keep the latter from scanning
those in vertical interior halo plane for copying to hbuf sv(p, d, β, m) for logical correctness,
because these functions recognizes that the particle copying is unnecessary/inhibitive by
the fact VPlaneHead[j] = VPlaneHead[j + 1] where j = 4d + 2p + β for [d][p][β].


for (d=OH_DIM_X; d<=OH_DIM_Y; d++) {
for (ps=0; ps<2; ps++) {
const int psor2 = ps ? trans+1 : 0;
struct S_commlist *rl;
int *ri;
if (ps>psnew || ZBound[ps][OH_UPPER]==0) {
vph[d][ps][0] = vph[d][ps][1] = vpidx;
continue;
}
if (psor2==2) {
rl = AltSecRList;  ri = AltSecRLIndex;
} else {
rl = rlist[ps];  ri = rlidx[ps];
}
for (du=OH_LOWER; du<=OH_UPPER; du++) {
const int vpisave = vpidx;
const int nx = (d==OH_DIM_X) ? du<<1 : 1;
const int ny = (d==OH_DIM_X) ? 1 : du<<1;
const int n = 3*3 + 3*ny + nx;
const int nrev = OH_NEIGHBORS - 1 - n;
int nbor = Neighbors[psor2][n];
vph[d][ps][du] = vpidx;
if (nbor<0)  nbor = -(nbor+1);
if (nbor<nn) {
make_bsend_sched(psor2, n, nx, ny, rl+ri[nrev], &nsendib, &vpidx);
make_brecv_sched(psor2, n, nx, ny, rl+ri[nrev], &nrecveb, vpisave);
}
}
}
}
vph[2][0][0] = vpidx;
}


4.13.34  Macros Add_Pillar_Voxel(),  Is_Pillar_Voxel(), Pillar_Lower() and
Pillar_Upper()

Add_Pillar_Voxel()  Here we show macros related to a value v in NOfPGrid[][][] for a grid-voxel in an interior pillar
Is_Pillar_Voxel()  or exterior pillar. As shown in §4.12.3, v for a grid-voxel can have −(i + 1) · 232 −(j + 1)· ≤
Pillar_Lower()  −232 when it is in an interior pillar, or (i + 1) · 232 + σ ≥232 when in an exterior pillar, to
Pillar_Upper()  represent the index i of BoundarySendBuf[] to which the particle in the grid-voxel is copied,
together with another index j of the buffer or σ of NOfSend[].


<!-- Page 520 -->

The macro Add Pillar Voxel(l) gives l · 232 so that we have (i + 1) · 232 with l = i + 1
to subtract it from v = −(j + 1) or add it to v = σ, or have 232 to subtract it from v
to increment its −232 component. The macro Is Pillar Voxel(v) is true iffv ≥232 to
indicate −v or v has (i + 1) component. The macro Pillar Lower(v) gives v mod 231 and
thus (j + 1) component of −v or σ component of v. The macro Pillar Upper(v) gives
⌊v/232⌋and thus (i + 1) component of −v or v.
All of four macros are used in Move_Or_Do() when it is invoked from move_and_sort(),
and in Sort_Particle() used in sort_particles() and sort_received_particles().
The macro Add_Pillar_Voxel() is also used in make_bsend_sched(), while Is_Pillar_
Voxel() and Pillar_Upper() are also used in xfer_boundary_particles_v().


#define Add_Pillar_Voxel(I)     (((dint)(I))<<32)
#define Is_Pillar_Voxel(V)      (V>=((dint)1)<<32)
#define Pillar_Lower(V)         (V&INT_MAX)
#define Pillar_Upper(V)         ((V)>>32)



#### 4.13.35 make_bsend_sched()

make_bsend_sched()  The function make_bsend_sched(), called solely from make_bxfer_sched() but up to 2 ×
2 × 2 = 8 times, scans PO(p, s, g) = NOfPGridOut[p][s][g] in a vertical interior halo plane
of the subcuboid in the subdomain n′ = {n, npold, npnew}[p′] (p′ = psor2) assigned to the
local node n as its primary (p = p′ = 0) or secondary (p = 1, p′ ∈{1, 2}) one, in order to
build the sending schedule for halo particles in the plane to be sent to the nodes in the k-th
(k = n) neighbor family of n′ according to k′-th (k′ = 3D −1 −k) sub-block λ = rlist
in primary sending (p′ = 0), secondary sending (p′ = 1) or alternative secondary sending
(p′ = 2) blocks. Note that k = 32 + νy · 31 + νx · 30, where νx = nx = {2β, 0}[d] and
νy = ny = {0, 2β}[d] for d-th dimensional lower (β = 0) or upper (β = 1) neighbor. The
function also accumulates Hs pointed by nsendptr being the numbder of halo particles to
be sent, and V pointed by vpptr being the number of VPlane[] enties consumed for bulding
sending schedules.
This function is very level-4s’s own and thus has no counterpart in level-4p.


static void
make_bsend_sched(const int psor2, const int n, const int nx, const int ny,
struct S_commlist *rlist, int *nsendptr, int *vpptr) {
const int ns=nOfSpecies;
const int ps = psor2==0 ? 0 : 1;
const int tag1 = OH_NEIGHBORS;
const int stag = n;
const int rtag = ((ps ? OH_NEIGHBORS : 0) + (OH_NEIGHBORS - 1 - n));
struct S_commlist *rl = rlist;
int rlz = rl->region;
int nsend = *nsendptr, nsendsave = nsend,  vpidx = *vpptr;
int xl, xu, yl, yu;
const int zbl = ZBound[ps][OH_LOWER];
const int zbu = ZBound[ps][OH_UPPER] - GridDesc[psor2].z;
const int zmax = ZBound[ps][OH_UPPER] - 1;
const int xtop = GridDesc[psor2].x;
int s;
Decl_For_All_Grid();


<!-- Page 521 -->

At first, we specify Sxy = [xl, xu) × [yl, yu) being each xy-subplane (or a line segment
perpendicular to z-axis in usual cases with eg = 1) in the vertical interior halo plane. The
values yl and yu are always determined by Grid_Interior_Boundary(), and xl and xu
are as well if νy = 1 for west or east vertical interior halo plane. However xl = −eg and
xu = δx(n′)+eg if νy ̸= 1 for south or north vertical interior halo plane, because it includes
exterior pillars at its west and east ends. Then we do the followings after keeping Hs in
H0s .
First we skip records in λ until we find the first family member mf such that ζupf (mf) ≥
ζlp(n) being the node which receives the particles in the bottom xy-plane of n’s subcuboid.
Then, we let elements of VPlane[V ] have the followings; mf in nbor; pf · 3D + k in stag;
p · 3D + (3D −1 −k) in rtag; and Hs in sbuf because Hs has the so-far total number
of halo particles to be sent and thus the head index of the send buffer hbuf sv(d, p, β, mf)
in BoundarySendBuf[] into which particles sent to mf are copied from corresponding grid-
voxels in SendBuf[]. The assignments to stag and rtag are to make the communication
between mf and n for halo particles in the plane and its exterior counterpart unique, with
a concept similar to that discussed in §4.13.24 but reflecting that particles of all species are
transferred at once.
Next, we visit each xy-subplane at z for all z ∈[ζlp(n), ζup (n)) to scan PO(p, s, g) =
NOfPGridOut[p][s][g] for all s ∈[0, S) and g ∈Sxy × {z} adding it to Hs. We also modify
v = NOfPGrid[p][s][g] as follows.

1. If g = gidx(x, y, z) is in an exterior pillar, i.e., x < 0 or x ≥δx(n′) and thus d = 1
definitely, we modify v = σ as v = (Hs + 1) · 232 + σ to indicate that the halo
particles in the grid-voxel received from a west/east neighbor should be copied into
hbuf sv(d, p, β, mf) starting from BoundarySendBuf[Hs] so that they are relayed to a
south/north neighbor. We keep σ in new v because it is necessary for move_and_
sort() to send particles originally in the grid-voxel to other node.

2. Otherwise and v is non-negative, we let v = −(Hs + 1) so that particles in the grid-
voxel are copied into hbuf sv(d, p, β, mf) starting from BoundarySendBuf[Hs]. Note
that keeping the original v is unnecessary because it is definitely 0 for move_and_
sort() (and sort_received_particles()) or it is no longer meaningful for sort_
particles().

3. Otherwise, i.e., v is negative to mean v has −(H′+1) with some H′ given in the case 2,
the grid-voxel has already been visited and thus is in an interior pillar. Therefore, we
let v = −(Hs+1)·232−(H′+1) so that particles in the grid-voxel are copied into both
of hbuf sv(d, p, β, mf) and hbuf sv(d′, p′, β′, m′f) starting from BoundarySendBuf[Hs] and
BoundarySendBuf[H′] respectively.

Then each time we compelete the scan of a xy-plane for all s ∈[0, S), we examine  if
z = ζupf (mf) −1 to mean z  is at the top surface of mf’s subcuboid.   If so, we let
VPlane[V ].nsend = Hs −H0s being the number of halo particles to be sent to mf,
i.e., the size of hbuf sv(d, p, β, mf). Now the buffer hbuf sv(d, p, β, mf) is formed as a con-
ceptual 5-dimensional array whose elements  [z][s][y][x][i] are for z ∈[ζlp(n), ζup (n)) ∩
[ζlpf (mf), ζupf (mf)), s ∈[0, S), y ∈[yl, yu), x ∈[xl, xu) and i ∈[0, PO(p, s, gidx(x, y, z))).
Then, if z < ζup (n)−1, we step to the next mf fetching the next record in λ and increment-
ing V , and then let nbor, stag, rtag and sbuf elements of VPlane[V ] be mf, pf · 3D + k,
p · 3D + (3D −1 −k) and Hs respectively, updating H0s to keep Hs.


<!-- Page 522 -->

At the end of the loop for z ∈[ζlp(n), ζup (n)), the series of the buffers hbuf sv(d, p, β, mf)
for f ∈{0, . . .}  is formed as a conceptual 5-dimensional array as a whole whose ele-
ments [z][s][y][x][i] are for z ∈[ζlp(n), ζup (n)), s ∈[0, S), y ∈[yl, yu), x ∈[xl, xu) and
i ∈[0, PO(p, s, gidx(x, y, z))), since ζupf (mf) = ζlpf+1(mf+1) for all f. Therefore, the whole
series of hbuf sv(d, p, k, mf) for d ∈{0, 1}, p ∈{0, pn} and β ∈{0, 1} established by the
series of the calls of this function from make_bxfer_sched() is formed as a 8-dimensional
array whose elements [d][p][β][z][s][y][x][i] are for those shown above but yl, yu, xl and xu
are dependent on d, p and β. Moreover, the index of BoundarySendBuf[] for the element
[d][p][β][z][s][y][x][0] is given by NOfPLocal[p][s][gidx(x, y, z)].
Finally, we return Hs and V + 1 to the caller make_bxfer_sched() for the successive
calls of this function itself, after letting VPlane[V ].nsend = Hs −H0s . Note that since V
has, at the end of the loop, the index of the last entry consumed by the function, we return
V + 1 rather than V .

if (ny==1) {
Grid_Interior_Boundary(nx, GridDesc[psor2].x, xl, xu);
} else {
xl = -OH_PGRID_EXT;  xu = OH_PGRID_EXT;
}
Grid_Interior_Boundary(ny, GridDesc[psor2].y, yl, yu);

while (rlz<zbl)  rlz = (++rl)->region;
VPlane[vpidx].nbor = rl->rid;
VPlane[vpidx].stag = (rl->tag ? tag1 : 0) + stag;
VPlane[vpidx].rtag = rtag;
VPlane[vpidx].sbuf = nsend;
For_All_Grid_Z(psor2, xl, yl, zbl, xu, yu, zbu) {
const int z = Grid_Z();
for (s=0; s<ns; s++) {
dint *npg = NOfPGrid[ps][s];
int *npgo = NOfPGridOut[ps][s];
For_All_Grid_XY(psor2, xl, yl, xu, yu) {
const int g = The_Grid();
const dint dst = npg[g];
if (Grid_X()<0 || Grid_X()>=xtop)
npg[g] += Add_Pillar_Voxel(nsend+1);
else if (dst>=0)
npg[g] = -(nsend+1);
else
npg[g] -= Add_Pillar_Voxel(nsend+1);
nsend += npgo[g];
}
}
if (z==rlz && z<zmax) {
VPlane[vpidx++].nsend = nsend - nsendsave;
rlz = (++rl)->region;
VPlane[vpidx].nbor = rl->rid;
VPlane[vpidx].stag = (rl->tag ? tag1 : 0) + stag;
VPlane[vpidx].rtag = rtag;
VPlane[vpidx].sbuf = nsendsave = nsend;
}
}
VPlane[vpidx].nsend = nsend - nsendsave;


<!-- Page 523 -->

*nsendptr = nsend;  *vpptr = vpidx + 1;
}


#### 4.13.36 make_brecv_sched()

make_brecv_sched()  The function make_brecv_sched(), called solely from make_bxfer_sched() but up to 2 ×
2 × 2 = 8 times, scans PO(p, s, g) = NOfPGridTotal[p][s][g] in a vertical exterior halo plane
surrounding the subcuboid in the subdomain n′ = {n, npold, npnew}[p′] (p′ = psor2) assigned
to the local node n as its primary (p = p′ = 0) or secondary (p = 1, p′ ∈{1, 2}) one, in order
to build the receiving schedule for halo particles in the plane to be received from the nodes
in the k-th (k = n) neighbor family of n′ according to the k′-th (k′ = 3D −1 −k) sub-block
λ = rlist in primary sending (p′ = 0), secondary sending (p′ = 1) or alternative secondary
sending (p′ = 2) blocks. Note that k = 32 + νy · 31 + νx · 30, where νx = nx = {2β, 0}[d]
and νy = ny = {0, 2β}[d] for d-th dimensional lower (β = 0) or upper (β = 1) neighbor.
The function also accumulates Hr pointed by nrecvptr being the number of particles to
be received, and the index V = vpidx of VPlane[] from which make_bsend_sched() has
built the sending schedule for the corresponding vertical interior halo plane and thus this
function will do as well.
This function is very level-4s’s own and thus has no counterpart in level-4p.


static void
make_brecv_sched(const int psor2, const int n, const int nx, const int ny,
struct S_commlist *rlist, int *nrecvptr, int vpidx) {
const int ns=nOfSpecies;
const int ps = psor2==0 ? 0 : 1;
int nrecv = *nrecvptr,  nrecvsave = nrecv;
struct S_commlist *rl = rlist;
int rlz = rl->region;
int xl, xu, yl, yu;
const int zbl = ZBound[ps][OH_LOWER];
const int zbu = ZBound[ps][OH_UPPER] - GridDesc[psor2].z;
const int zmax = ZBound[ps][OH_UPPER] - 1;
int s;
Decl_For_All_Grid();


At first, we specify Sxy = [xl, xu) × [yl, yu) being each xy-subplane (or a line segment
perpendicular to z-axis in usual cases with eg = 1) in the vertical exterior halo plane. The
values yl and yu are always determined by Grid_Exterior_Boundary(), and xl and xu
are as well if νy = 1 for west or east vertical exterior halo plane. However xl = −eg and
xu = δx(n′)+eg if νy ̸= 1 for south or north vertical exterior halo plane, because it includes
pillars, being the intersection of west/east and south/north vertical exterior halo planes at
its west and east ends. Then we do the followings after keeping Hr in H0r .
First we skip records in λ until we find the first family member mf such that ζupf (mf) ≥
ζlp(n) being the node which sends the particles in the bottom xy-plane of n’s subcuboid.
Then, we let VPlane[V ].rbuf = Hr because Hr has the so-far total number of halo particles
to be received and thus the head index of the receive buffer hbuf rv(d, p, β, mf) in Particles[]
from which particles received from mf are copied to corresponding grid-voxels in SendBuf[].
Next, we visit each xy-subplane at z for all z ∈[ζlp(n), ζup (n)) to scan PO(p, s, g) =
NOfPGridOut[p][s][g] for all s ∈[0, S) and g ∈Sxy × {z} adding it to Hr. Then each
time we compelete the scan of a xy-plane, we examine if z = ζupf (mf) −1 to mean z is


<!-- Page 524 -->

at the top surface of mf’s subcuboid.  If so, we let VPlane[V ].nrecv = Hr −H0r being
the number of halo particles to be received from mf, i.e., the size of hbuf rv(d, p, β, mf).
Now the buffer hbuf rv(d, p, β, mf) is formed as a conceptual 5-dimensional array whose
elements [z][s][y][x][i] are for z ∈[ζlp(n), ζup (n)) ∩[ζlpf (mf), ζupf (mf)), s ∈[0, S), y ∈[yl, yu),
x ∈[xl, xu) and i ∈[0, PO(p, s, gidx(x, y, z))). Then, if z < ζup (n) −1, we step to the next
mf fetching the next record in λ and incremening V , and then let VPlane[V ].rbuf = Hr,
as we did before the loop, updating H0r to keep Hr.
At the end of the loop for z ∈[ζlp(n), ζup (n)), the series of the buffers hbuf rv(d, p, β, mf)
for f ∈{0, . . .}  is formed as a conceptual 5-dimensional array as a whole whose ele-
ments [z][s][y][x][i] are for z ∈[ζlp(n), ζup (n)), s ∈[0, S), y ∈[yl, yu), x ∈[xl, xu) and
i ∈[0, PO(p, s, gidx(x, y, z))), since ζupf (mf) = ζlpf+1(mf+1) for all f. Therefore, the whole
series of hbuf rv(d, p, β, m) for d ∈{0, 1}, p ∈{0, pn} and β ∈{0, 1} established by the series
of the calls of this function from make_bxfer_sched() is formed as a 8-dimensional array
whose elements [d][p][β][z][s][y][x][i] are for those shown above but yl, yu, xl and xu are
dependent on d, p and β.
Finally, we return Hr to the caller make_bxfer_sched() for the successive calls of this
function itself, after letting VPlane[V ].nrecv = Hr −H0r .

if (ny==1) {
Grid_Exterior_Boundary(nx, GridDesc[psor2].x, xl, xu);
} else {
xl = -OH_PGRID_EXT;  xu = OH_PGRID_EXT;
}
Grid_Exterior_Boundary(ny, GridDesc[psor2].y, yl, yu);

while (rlz<zbl)  rlz = (++rl)->region;
VPlane[vpidx].rbuf = nrecv;
For_All_Grid_Z(psor2, xl, yl, zbl, xu, yu, zbu) {
const int z = Grid_Z();
for (s=0; s<ns; s++) {
int *npgo = NOfPGridOut[ps][s];
For_All_Grid_XY(psor2, xl, yl, zu, yu)
nrecv += npgo[The_Grid()];
}
if (z==rlz && z<zmax) {
VPlane[vpidx++].nrecv = nrecv - nrecvsave;
rlz = (++rl)->region;
VPlane[vpidx].rbuf = nrecvsave = nrecv;
}
}
VPlane[vpidx].nrecv = nrecv - nrecvsave;  *nrecvptr = nrecv;
}


#### 4.13.37 Macros Local_Grid_Position() and Move_Or_Do()

Local_Grid_Position()  The macro Local Grid Position(g, k·2Γ+g, p), used in the macro Move_Or_Do() and the
function oh4s_remove_mapped_particle() directly, transforms a particle’s grid-position g
in the k-th neighbor of the local node n’s primary (p = 0) or secondary (p = 1) subdomain
into its corresponding index g′ in the n’s subdomain by g + GridOffset[p][k]. The macro
is perfectly equivalent to its level-4p counterpart shown in §4.13.37.


<!-- Page 525 -->

#define Local_Grid_Position(G, NID, PS)  ((G) + GridOffset[PS][NID>>loggrid])


Move_Or_Do()  The macro Move Or Do(π, p, n′, µ, a, η), used in the particle scanning loop in move_to_
sendbuf_4s(), move_to_sendbuf_uw4s() and move_to_sendbuf_dw4s() with η = 0, and
move_and_sort() with η = 1, examines a primary (p = 0) or secondary (p = 1) particle
π of species s in Particles[] and moves/copies the particle in Particles[], to SendBuf[]
and/or to BoundarySendBuf[]. This macro is somewhat similar to its level-4p counterpart
shown in §4.10.40 but significantly different from it because we have no hot-spots but have
halo particles.
The first part is, however, quite similar to the counterpart. That is, If π.nid < 0 to
mean the particle was eliminated, we skip the iteration of the loop by continue. Otherwise
we obtain its subdomain identifier m by Neighbor_Subdomain_Id() and grid-position g
by Grid_Position() if m = n′, or by transforming what the macro gives into the local
coordinate g by Local_Grid_Position() otherwise, where n′  is the primary/secondary
subdomain of the local node.


#define Move_Or_Do(P, PS, MYSD, TOSB, ACT, PIL) {\
const OH_nid_t nid = P->nid;\
int g = Grid_Position(nid);\
int sdid;\
dint dst;\
if (nid<0)  continue;\
sdid = Neighbor_Subdomain_Id(nid, PS);\
if (sdid!=(MYSD)) g = Local_Grid_Position(g, nid, PS);\

Then if v = npg[g] = NOfPGrid[p][s][g] = 0, where the pointer to NOfPGrid[p][s][0] is
given through the implicit argument npg, to mean that the particle should stay in the
local node, we perform the operation a specified by the macro invoker, which is to move π
in Particles[] or to SendBuf[], except for the invocation in a loop of move_to_sendbuf_
uw4s() skipping particles.
If v > 0, on the other hand, it means that v mod 232 = σ = ((p′S + s)N + m′) + 1
representing the one-dimensional index of [p′][s][m′] of NOfSend[2][S][N] and π is to be sent
to m′ as its (not the local node’s) primary (p′ = 0) or secondary (p′ = 1) particle. Therefore,
we move π to SendBuf[β +NOfSend[p′][s][m′]] and then increment NOfSend[p′][s][m′] for the
next particle to be sent to m′ if µ ̸= 0. Note that SendBuf[β] is given through the implicit
argument sb, and β = Qn if the macro is invoked in move_and_sort(), or β = 0 otherwise.
Also note that it can be v ≥232 only when η ̸= 0 requiring us to extract σ by Pillar_
Lower(), and it cannot be σ = 0 if v ≥232 because the grid-voxel having π is in a vertical
exterior halo plane97. In addition, the conditionals examining η and µ should be eliminated
by compilers because they are constant in functions using this macro.
Otherwise, v < 0 to mean that π is in a vertical interior halo plane and definitely η ̸= 0.
Since π stays in the local node, the operation a takes place. Then  if v > −232 having
−(i + 1), we copy π to BoundarySendBuf[i], and then increment i by decrementing v for
the particle next to π. Otherwise, i.e., v = −(i + 1) · 232 −(j + 1) ≤−232 to mean g is
in an interior pillar, we copy π to BoundarySendBuf[i] and BoundarySendBuf[j], and then
increment i and j by decrementing v by 232 + 1 for the particle next to π.

97This does not means a grid-voxel cannot have v = (i + 1)232 with η ̸= 0. In fact, if a neighbor of n′
is n′ itself, grid-voxels in an exterior pillar in a vertical exterior halo plane for halo particles received from
the self neighbor definitely has such value since the grid-voxels do not have any particles. However, since
no particles in such grid-voxels, they are not accessed in Move Or Do().
