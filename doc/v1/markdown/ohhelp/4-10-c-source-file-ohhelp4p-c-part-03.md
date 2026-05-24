# 4.10 C Source File ohhelp4p.c - Part 3

Source: `doc/v1/original/ohhelp.pdf`, pages 404-435.

<!-- Page 404 -->

#### 4.10.33 update_real_neighbors()

update_real_neighbors()  The  function update_real_neighbors(),  called from init4p(),  try_primary4p(),
exchange_particles4p() and make_recv_list(), updates RealDstNeighbors[][] and
RealSrcNeighbors[][] according to its mode argument to specify the elements to be updated,
dosec to specify whether nodes have helpers or not, and oldp = npold and newp = npnew
being old and new parent(n) of the local node n on helpand-helper reconfiguration. Note
that npnew is just parent(n) in the stable state of helpand-helper configuration.
As discussed in §4.9.5, these two arrays have the neighbor node idententifiers as follows.

- RealDstNeighbors[t][p] has the set of nodes which will accommodate particles that
the local nodes is accommodating as its primary and secondary ones, as their primary
(p = 0) or secondary (p = 1) ones in the stable (t = 0) or transitional (t = 1) state of
helpand-helper configuration. Therefore, each element has the following, where N(m)
is the set of neighbors of a subdomain m including m itself, and Hnew(m) is the set of
m’s helpers in the stable helpand-helper configuration or new ones in the transitional
configuration.

RealDstNeighbors[0][0] = N(n) ∪N(npnew)
RealDstNeighbors[0][1] = Hnew(N(n)) ∪Hnew(N(npnew))
RealDstNeighbors[1][0] = N(n) ∪N(npold)
RealDstNeighbors[1][1] = Hnew(N(n)) ∪Hnew(N(npold))
Note that N(n) and N(npold) are always given by Neighbors[0][] and Neighbors[1][],
while N(npnew) is given by Neighbors[1][] or Neighbors[2][] depending on stable or
transitional state of helpand-helper configuration respectively.

- RealSrcNeighbors[t][p] has the set of nodes which is accommodating particles that
the local nodes will accommodate as its primary (p = 0) or secondary (p = 1) ones,
as their primary and secondary ones in the stable (t = 0) or transitional (t = 1) state
of helpand-helper configuration.  Therefore, each element has the following, where
Hold(m) is the set of m’s old helpers in transitional helpand-helper configuration.

RealSrcNeighbors[0][0] = N(n) ∪Hnew(N(n))
RealSrcNeighbors[0][1] = N(npnew) ∪Hnew(N(npnew))
RealSrcNeighbors[1][0] = N(n) ∪Hold(N(n))
RealSrcNeighbors[1][1] = N(npnew) ∪Hold(N(npnew))

In addition, as discussed in §4.10.5, the mode argument has one of the followings.

- URN_PRI to update  [0][0] only and given in the  calls from init4p() and try_
primary4p().

- URN_SEC to update [0][0] and [0][1] but not [1][], and given in the call from exchange_
particles4p() with anywhere accommodation in which we don’t need to care about
transitional helpand-helper configuration.

- URN_TRN to update all of [0][0], [0][1], [1][0] and [1][1], and given in the call from make_
recv_list() with normal accommodation in which we have to care about transitional
helpand-helper configuration.


<!-- Page 405 -->

static void
update_real_neighbors(const int mode, const int dosec, const int oldp,
const int newp) {
const int me=myRank, nn=nOfNodes, nn4=nn<<2;
const int dosec0 = mode != URN_PRI;
int i, nbridx, ps, *doccur[2], *soccur[2];


At  first we  zero-clear  all  elements  of  TempArray[2][2][N]  so  that  its  element
[σ][p][m]  is true iffm has already been in RealDstNeighbors[][p].nbor (σ =  0) or
RealSrcNeighbors[][p].nbor (σ = 1).

for (i=0; i<nn4; i++)  TempArray[i] = 0;
doccur[0] = TempArray;       doccur[1] = doccur[0] + nn;
soccur[0] = doccur[1] + nn;  soccur[1] = soccur[0] + nn;


Then if mode is URN_TRN, we exchange [0][0] and [1][0] of RealSrcNeighbors[][] so that
[1][0] has everything in [0][0] because Hold(N(n)) is the current (i.e., old) Hnew(N(n)).

if (mode==URN_TRN) {
int *tmp = RealSrcNeighbors[1][0].nbor;
RealSrcNeighbors[1][0].n = RealSrcNeighbors[0][0].n;
RealSrcNeighbors[1][0].nbor = RealSrcNeighbors[0][0].nbor;
RealSrcNeighbors[0][0].nbor = tmp;
}

Now we call upd_real_nbr() twice to build the first subsets of RealDstNeighbors[0][]
and the whole of RealSrcNeighbors[0][0], i.e., N(n) and Hnew(N(n)), after zero-clearing
their n elements for cardinality of the sets in thier nbor elements. The arguments given to
the function are as follows.

- root = n to mean we visit N(n) and their helpers.

- psp = 0 to mean N(n) are included in [0][0].nbor.

- pss = 1 for RealDstNeighbors[][] to mean Hnew(N(n)) are included in [0][1].nbor,
while pss = 0 for RealSrcNeighbors[][] to mean Hnew(N(n)) are included in
[0][0].nbor.

- nbr = 0 to mean N(n) is given by Neighbors[0][].

- dosec is true iffmode ̸= URN_PRI to mean we have to visit the elements in Hnew(N(n))
iffmode ̸= URN_PRI.

- nodes  is Nodes[] to mean Hnew(N(n)) are obtained by traversing the  list from
Nodes[m].child for m ∈N(n).

- rnbptr is RealDstNeighbors[0][] or RealSrcNeighbors[0][] according to the call.

- occur[2]  is  TempArray[0][][]  for  RealDstNeighbors[0][]  or  TempArray[1][][]  for
RealSrcNeighbors[0][].


<!-- Page 406 -->

Then we  return  to  the  caller   if  mode =  URN_PRI  because  what we  need  is
RealDstNeighbors[0][0] = RealSrcNeighbors[0][0] = N(n).

RealDstNeighbors[0][0].n = RealDstNeighbors[0][1].n = 0;
RealSrcNeighbors[0][0].n = RealSrcNeighbors[0][1].n = 0;
upd_real_nbr(me, 0, 1, 0, dosec0, Nodes, RealDstNeighbors[0], doccur);
upd_real_nbr(me, 0, 0, 0, dosec0, Nodes, RealSrcNeighbors[0], soccur);
if (mode==URN_PRI)  return;


Next we call upd_real_nbr() twice again to obtain the second subsets of RealDst
Neighbors[0][] and the whole of RealSrcNeighbors[0][1], i.e., N(npnew) and Hnew(N(npnew)).
The arguments given to the function are as follows.

- root = npnew to mean we visit N(npnew) and their helpers.
- psp = 0 for RealDstNeighbors[][] to mean N(npnew) are included in [0][0].nbor,
whiele psp = 1 for RealSrcNeighbors[][] to mean N(npnew) are included in [0][1].nbor.
- pss = 1 to mean Hnew(N(npnew)) are included in [0][1].nbor.
- nbr = 2 or 1 to mean N(npnew) is given by Neighbors[2][] if mode = URN_TRN, or
Neighbors[1][] otherwise, i.e., mode ̸= URN_TRN respectively. That is, N(npnew) has
neighbors of the stable helpand or newly assigned helpand.

- dosec  is unconditionally true to mean always we have to visit the elements in
Hnew(N(npnew)).
- nodes is Nodes[] to mean Hnew(N(npnew)) are obtained by traversing the list from
Nodes[m].child for m ∈N(npnew).

- rnbptr is RealDstNeighbors[0][] or RealSrcNeighbors[0][] according to the call.

- occur[2]  is  TempArray[0][][]  for  RealDstNeighbors[0][]  or  TempArray[1][][]  for
RealSrcNeighbors[0][].

Then if mode ̸= URN_TRN, we return to caller with;

RealDstNeighbors[0][] = {N(n) ∪N(npnew), Hnew(N(n)) ∪Hnew(N(npnew))}
RealSrcNeighbors[0][] = {N(n) ∪Hnew(N(n)), N(npnew) ∪Hnew(N(npnew))}


nbridx = mode==URN_TRN ? 2 : 1;
upd_real_nbr(newp, 0, 1, nbridx, 1, Nodes, RealDstNeighbors[0], doccur);
upd_real_nbr(newp, 1, 1, nbridx, 1, Nodes, RealSrcNeighbors[0], soccur);
if (mode!=URN_TRN)  return;


Finally, we call upd_real_nbr() thrice, twice for RealDstNeighbors[1][] and once for
RealSrcNeighbors[1][1], after zero-clearing TempArray[σ][p][m] for all m such that m ∈
RealDstNeighbors[0][p].nbor (σ = 0) and m ∈RealSrcNeighbors[0][p].nbor (σ = 1).
The first two calls for RealDstNeighbors[1][] are to obtain;
RealDstNeighbors[1][0] = N(n) ∪N(npold)
RealDstNeighbors[1][1] = Hnew(N(n)) ∪Hnew(N(npold))

for which the first and second calls build the first and second half of each element respec-
tively. To these two calls, we give the following arguments.


<!-- Page 407 -->

- root = n for the first call and npold for the second to visit N(n) or N(npold) and their
helpers respectively.
- psp = 0 to mean N(n) and N(npold) are included in RealDstNeighbors[1][0].nbor,
commonly.
- pss = 1 to mean Hnew(N(n)) and Hnew(N(npold)) are included in RealDstNeighbors
[1][1].nbor, commonly.
- nbr = 0 for the first call and 1 for the second to mean N(n) and N(npold) are given
by Neighbors[0][] and Neighbors[1][] respectively.
- dosec is true to visit Hnew(N(n)) and Hnew(N(npold)) unconditionally (as far as npold
exists.

- nodes is commonly Nodes[] to mean Hnew(N(m)) are obtained by traversing the list
from Nodes[m′].child for m′ ∈N(m) where m ∈{n, npold}.

- rnbptr is commonly RealDstNeighbors[1][].

- occur[2] is commonly TempArray[0][][].

On the other hand, the third and last call to obtain;

RealSrcNeighbors[1][1] = N(npnew) ∪Hold(N(npnew))

gives the following arguments to upd_real_nbr().
- root = npnew to visit N(npnew) and their helpers.
- psp =  pss =  1  to mean N(npnew) and  Hold(N(npnew))  are  included  in
RealSrcNeighbors[1][1].nbor, respectively.

- nbr = 2 to mean N(npnew) is given by Neighbors[2][].
- dosec is that in this function’s argument to mean we visit Hold(N(npnew)) if we were
in secondary mode.

- nodes is NodesNext[] to mean Hold(N(npnew)) is obtained by traversing the list from
NodesNext[m].child for m ∈N(npnew), becuse NodesNext[] is Nodes[] in the last step
when update_real_neighbors() is called.

- rnbptr is RealSrcNeighbors[1][].

- occur[2] is TempArray[1][][].


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
}


<!-- Page 408 -->

#### 4.10.34 upd_real_nbr()

upd_real_nbr()  The function upd_real_nbr(),  called  solely from update_real_neighbors() but up
to seven times, to add members of N(r) and,  if dosec argument  is ture, H(N(r))
to RN [pp] and RN [ps]  respectively where  r =  root ∈  {n, npnew, npold}, N(r)  is
given by Neighbors[nbr], H(m)  is  given by NN [m] =  nodes[m] being  either  of
Nodes[] or NodesNext[], RN [2] =  rnbrptr[2]  is  either  of RealDstNeighbors[t][] or
RealSrcNeighbors[t][] with t ∈{0, 1}, and pp = psp and ps = pss. Another argument
occur[2][N] being TempArray[σ][][] with σ ∈{0, 1} to indicate that a node m has already
been in RN [p] iffoccur[p][m] is true.


static void
upd_real_nbr(const int root, const int psp, const int pss,
const int nbr, const int dosec, struct S_node *nodes,
struct S_realneighbor rnbrptr[2], int *occur[2]) {
const int me=myRank;
struct S_realneighbor *pnbr = rnbrptr+psp,  *snbr = rnbrptr+pss;
int *poccur = occur[psp],  *soccur = occur[pss];
int i;

At first we check if r < 0 to mean npnew or npold does not exist, and return to the caller
doing nothing if so. Otherwise, we add r to RN [pp].nbor[] if r ̸= n and r is not in the
set. Then, if dosec is true, we traverse the list NN [r].child to add each of its member
m ∈H(r) to RN [ps].nbor[] if m ̸= n and m is not in the set. Note that we exclude n
itself from both sets because n does not communicate with itself in the communication in
its primary/secondary families. Also note that we visit H(r) even if occur[pp][r] is true
because it could have been visited as a helper and pp = ps.

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

Next we traverse Neighbors[nbr][k] for all k ∈[0, 3D) to have N(r) and add each
m ∈N(r) to RN [pp].nbor[] if m ̸= r and m is not in the set.  If so and dosec is true, we
also traverse the list NN [m].child to add each of its member m′ to RN [ps].nbor[] if m′ is
not in the set. Note that we exclude neither of n nor r from both sets because they can be a
helper of their (or n’s helpand’s) neighbors and, for n, it must perform self communication
for particle movement crossing the boundary of its primary/secondary subdomains. Also
note that we visit H(m) even if occur[pp][m] is true because it could have been visited as
a helper and pp = ps.

for (i=0; i<OH_NEIGHBORS; i++) {


<!-- Page 409 -->

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


#### 4.10.35 exchange_xfer_amount()

exchange_xfer_amount()  The function exchange_xfer_amount(), called solely from exchange_particles4p(), ex-
changes NOfSend[][][] in the nodes responsible of a subdomain and its neighbors as the
nodes’ primary/secondary subdomain to have NOfRecv[][][] for position-aware particle trans-
fer when we will be in secondary mode in the next step. The function is given two ar-
guments; trans = t ∈{0, 1} to mean we have stable (t = 0) or transitional (t = 1)
state of helpand-helper configuration and to be used to refer to RealSrcNeighbors[t][][]
and RealDstNeighbors[t][][] to find the senders/receivers of NOfSend[][][], respectively; and
psnew = pn ∈{0, 1} to mean the local node will have a secondary subdomain (pn = 1) and
thus will receive some particles, or not.


static void
exchange_xfer_amount(const int trans, const int psnew) {
const struct S_realneighbor *snbr = RealSrcNeighbors[trans];
const struct S_realneighbor *dnbr = RealDstNeighbors[trans];
const int nnns = nOfNodes * nOfSpecies;
int ps, tag, req;


First we  post  MPI_Irecv()  to  receive  NOfRecv[p][s][m]  from  all  nodes m  ∈
RealSrcNeighbors[t][p].nbor[] which  is m’s NOfSend[p][s][n] for the local node n, for
p ∈{0, pn}. The receiving data has the MPI’s data-type T_Hgramhalf for a slice [∗][N]
of an integer array [S][N]. Since a node m can appear in both of RealSrcNeighbors[t][0]
and RealSrcNeighbors[t][1], we give a tag 0 and NS for p = 0 and p = 1 respectively to
MPI_Irecv() to distinguish them.

for (ps=0,tag=0,req=0; ps<=psnew; ps++,tag+=nnns) {
const int n = snbr[ps].n;
const int *nbor = snbr[ps].nbor;
int i,  *nrbase = NOfRecv + tag;
for (i=0; i<n; i++,req++) {
const int nid = nbor[i];
MPI_Irecv(nrbase+nid, 1, T_Hgramhalf, nid, tag, MCW, Requests+req);
}
}


<!-- Page 410 -->

Next, we send local node n’s NOfSend[p][s][m] to all nodes m ∈RealDstNeighbors[t][p].
nbor[] so as to be received into m’s NOfRecv[p][s][n], for p ∈{0, 1} by MPI_Isend(). The
data-type and tag are same as the receiving counterpart MPI_Irecv(), i.e., T_Hgramhalf
and {0, NS}[p].

for (ps=0,tag=0; ps<2; ps++,tag+=nnns) {
const int n = dnbr[ps].n;
const int *nbor = dnbr[ps].nbor;
int i,  *nsbase = NOfSend + tag;
for (i=0; i<n; i++,req++) {
const int nid = nbor[i];
MPI_Isend(nsbase+nid, 1, T_Hgramhalf, nid, tag, MCW, Requests+req);
}
}

Finally, we confirm the completion of all MPI_Irecv() and MPI_Isend() calls recorded
in Requests[] by MPI_Waitall() to have their completion status in Statuses[] (but not
referring to).

MPI_Waitall(req, Requests, Statuses);
}


#### 4.10.36 count_population()

count_population()  The  function  count_population(),  called  from  try_primary4p() and  exchange_
particles4p() when they find we have anywhere accmmodation, counts the particle pop-
ulation in each grid-voxel to have the per-grid histogram in NOfPGrid[][][] after we perform
non-position-aware particle transfer. The function is given three arguments; nextmode be-
ing 0 or 1 for the call from try_primary4p() or exchange_particles4p() respectively
to mean we will be in primary/secondary mode respectively; pn = psnew = 1 iffthe lo-
cal node will have a secondary subdomain; and stats = 0 for the call from exchange_
particles4p() because count_population() is included in the previous region for the
timing measurement, while it is stats argument of the caller try_primary4p() because
count_population() starts a new region.


static void
count_population(const int nextmode, const int psnew, const int stats) {
int ps, s, t, i, j, tp;
const int ns=nOfSpecies, exti=OH_PGRID_EXT;
Decl_For_All_Grid();
Decl_Grid_Info();


After starting the new timing measurement with the key STATS_TB_SORT and nextmode
if required by stats ̸= 0, we do the followings for all p ∈[0, pn] and s ∈[0, S). First we
zero-clear the per-grid histogram NOfPGrid[ps][s][g] for all g = gidx(x, y, z) for (x, y, z) ∈
[0, δx(n)) × [0, δy(n)) × [0, δy(n)) by For_All_Grid() and The_Grid() to have g.
Then we scan all particles Particles[i] in pbuf (p, s), whose size is TotalPNext[p][s]
which has already been set by move_to_sendbuf_primary() called from try_primary4p(),
or  move_to_sendbuf_secondary()  called  from  exchange_particles()  called  from
exchange_particles4p().   The  size  is  also  set  into  TotalP[p][s]  for the  reference


<!-- Page 411 -->

in sort_particles(), move_to_sendbuf_sec4p() and  its callees, or move_and_sort_
secondary(), together with the sum of TotalPNext[0][s] for  all s ∈[0, S) set into
primaryParts and that of TotalPNext[p][s] for all p ∈{0, 1} set into totalParts.
For each Particles[i], we extract its grid-position g by Grid_Position() and increment
NOfPGrid[p][s][g] to let it have per-grid histogram finally. We also make nid of each particle
have ⌊3D/2⌋2Γ + g so that all particles look like staying in the local node’s primary/
secondary subdomain. This operation is necessary because a particle has traveled from a
neighbor subdomain can have a neighbor index k ̸= ⌊3D/2⌋with which other functions
should confuse that it should go out to a neighbor subdomain.
Finally, we let nOfInjections be 0 because all injected particles have been processed
by the first-phase non-position-aware particle transfer.

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


#### 4.10.37 sort_particles()

sort_particles()  The function sort_particles() is called from the following functions to move particles in
Particles[] to SendBuf[] with sorting.

- try_primary4p() when it finds we have anywhere accommodation with which we have
to gather primary particles at first and then sort them. It gives npg = NOfPGrid[][][] as
the complete per-grid histogram built by count_population(), nextmode = psnew =
0 because we will be in primary mode and thus we have only primary particles to
be sorted, and stats = 0 because the timing measurement with STATS_TB_SORT has
already been started by count_population() preceding the call.

- try_primary4p() when it finds we have normal accommodation but Pn+Pnsend > Plim
to mean we have to transfer particles among neighbors at first and then sort all
primary particles.   It gives npg = NOfPGrid[][][]  if we were in primary mode in
which the per-grid histogram was built by the neighbor communication by exchange_
population(), or npg = NOfPGridTotal[][][] if we were in secondary mode in which
we needed in-family reduction in exchange_population() to have the per-grid his-
togram.  It also gives nextmode = psnew = 0 because we will be in primary mode
and thus we have only primary particles to be sorted, and its own stats argument to
stats to start the timing measurement with STATS_TB_SORT if required by stats ̸= 0.


<!-- Page 412 -->

- exchange_particles4p() when it finds Pn+Pnsend > Plim to mean we have to transfer
particles among neighbors at first and then sort all primary and secondary particles.
Since the per-grid histogram is built in NOfPGridOut[][][] by make_send_sched() and
its element is int instead of dint of NOfPGrid[][][] and NOfPGridTotal[][][], the per-
grid histogram is not given through the argument npg, which has NULL but its use
is specified by nextmode ̸= 0. The argument nextmode also specifies the index of
GridDesc[] used for secondary particles, i.e., [1] or [2] for stable or transitional state
of helpand-helper configuration respectively. The argument psnew is 1 iffthe local
node will have a secondary subdomain, i.e., it is not the root of the family tree. The
argument stats is that of the caller to start the timing measurement with STATS_
TB_SORT if required by stats ̸= 0.


static void
sort_particles(dint ***npg, const int nextmode, const int psnew,
const int stats) {
const int ns=nOfSpecies;
struct S_particle *p = Particles;
int ps, s, t, i, npt;
Decl_For_All_Grid();
Decl_Grid_Info();


After starting the new timing measurement with the key STATS_TB_SORT if required by
stats ̸= 0, we do the followings for all p ∈[0, pn] and s ∈[0, S) where pn = psnew. First,
we build the index array for bucket sort in NOfPGridTotal[][][] as follows.

D−1∏
Gp = {g | g = gidx(x, y, z), (x, y, z) ∈      [0, δd(m)), m = {n, parent(n)}[p]}

d=0
∑p−1 S−1∑ ∑    ∑s−1 ∑    ∑
NOfPGridTotal[p][s][g] =            P(q, t, h) +        P(p, t, h) +    P(p, s, h)
q=0 t=0 h∈G0            t=0 h∈Gp           h<g

where P(p, s, h) is NOfPGridOut[p][s][h] = PO(p, s, h) if nextmode ̸= 0, or npg[p][s][h] =
NOfPGrid[p][s][g] = PL(p, s, h) or NOfPGridTotal[p][s][g] = PT (p, s, h) otherwise. In the
latter case, we let NOfPGridOut[p][s][h] = PO(p, s, h) have the value npg[p][s][h] as the
number of particles the local node accommodates in the grid-voxel at g.  The array
NOfPGridTotal[][][] is built scanning elements by For_All_Grid() whose first argument
is 0 when p = 0, or nextmode ∈{1, 2} when p = 1 to specify the index of GridDesc[].
Then we scan all particles Particles[i] in pbuf (p, s), whose size is TotalPNext[p][s]
which has already been set by move_to_sendbuf_primary() called from try_primary4p(),
or move_to_sendbuf_sec4p() called from exchange_particles4p(). For each Particles
[i], we  extract  its  grid-position  g by Grid_Position() and move  it  to SendBuf
[NOfPGridTotal[p][s][g]] and then increment NOfPGridTotal[p][s][g] for the next particle
in the grid-voxel at g.

if (stats) oh1_stats_time(STATS_TB_SORT, nextmode?1:0);
for (ps=0,t=0,npt=0; ps<=psnew; ps++) {
for (s=0; s<ns; s++,t++) {
int *npgo = NOfPGridOut[ps][s];
dint *npgt = NOfPGridTotal[ps][s];
const int tpn = TotalPNext[t];


<!-- Page 413 -->

if (nextmode) {
const int gdidx = ps ? nextmode : 0;
For_All_Grid(gdidx, 0, 0, 0, 0, 0, 0) {
const int np = npgo[The_Grid()];
npgt[The_Grid()] = npt;  npt += np;
}
} else {
dint *npgs = npg[ps][s];
For_All_Grid(0, 0, 0, 0, 0, 0, 0) {
const int np = npgo[The_Grid()] = npgs[The_Grid()];
npgt[The_Grid()] = npt;  npt += np;
}
}
for (i=0; i<tpn; i++,p++)
SendBuf[npgt[Grid_Position(p->nid)]++] = *p;
}
}
}


#### 4.10.38 move_and_sort_primary()

move_and_sort_primary()  The function move_and_sort_primary() is called solely from try_primary4p() when it
finds we have normal accommodation and Pn + Pnsend ≤Plim to mean we can move par-
ticles staying in and leaving from the local node toghther from Particles[] to SendBuf[]
with sorting. It receives the following three arguments; npg = NOfPGrid[][][] if we were in
primary mode or npg = NOfPGridTotal[][][] otherwise to show the complete per-grid his-
togram; psold = pc is 1 iffwe were in secondary mode and the local node had a secondary
subdomain; and stats to mean we have to start new timing measurement if required by
stats ̸= 0.

static void
move_and_sort_primary(dint ***npg, const int psold, const int stats) {
const int nn=nOfNodes, ns=nOfSpecies, nnns=nn*ns, me=myRank;
const int ninj=nOfInjections,  sbase=specBase;
struct S_particle *rbb, *p, *sbuf;
int ps, s, t, i, nacc, mysd, *sbd;
Decl_For_All_Grid();
Decl_Grid_Info();

After starting the new timing measurement with the key STATS_TB_MOVE if required by
stats ̸= 0, we do the followings for all s ∈[0, S). For the local node n, at first we let

N−1∑ ∑             N−1∑ ∑
TotalPNext[0][s] =            q(m)[p][s][n] =           NOfPrimaries[p][s][m]
m=0 p∈{0,1}            m=0 p∈{0,1}

and TotalPNext[1][s] = 0. We also let RecvBufBases[0][s] point Particles[r(s)] where;
                    
∑s−1   N−1∑ ∑
r(s) =               q(m)[p][t][n] −q(n)[0][t][n] 

t=0   m=0 p∈{0,1}

∑s−1
=    (TotalPNext[0][t] −NOfPLocal[0][t][n])

t=0


<!-- Page 414 -->

to mean rbuf (0, s) are continually ranked from Particles[0] and its size is the number
of particles of species s in n’s primary subdomain excluding those the local node has al-
ready accommodated as its primary particles. In addition, we let NOfPLocal[0][s][n] = 0
and InjectedParticles[0][p][s] = qinj(n)[p][s] = 0 for p ∈{0, 1} so that set_sendbuf_
disps() excludes particles staying in n and ignores injected particles when  it builds
SendBufDisps[s][].
Then we scan NOfPGrid[0][s][g] or NOfPGridTotal[0][s][g] given through the argument
npg by For_All_Grid() to copy it into NOfPGridOut[0][s][g] and build the index array for
sorting in NOfPGridTotal[0][s][g] as discussed in §4.10.37.

if (stats) oh1_stats_time(STATS_TB_MOVE, 0);
for (s=0,t=0,nacc=0,rbb=Particles; s<ns; s++,t+=nn) {
int n, tpn, *npgo=NOfPGridOut[0][s], *nprime=NOfPrimaries+t;
dint *npgs=npg[0][s], *npgt=NOfPGridTotal[0][s];
for (n=0,tpn=0; n<nn; n++)  tpn += nprime[n] + nprime[n+nnns];
TotalPNext[s] = tpn;  TotalPNext[ns+s] = 0;
RecvBufBases[s] = rbb;  rbb += tpn - NOfPLocal[t+me];
NOfPLocal[t+me] = 0;
InjectedParticles[s] = InjectedParticles[ns+s] = 0;
For_All_Grid(0, 0, 0, 0, 0, 0, 0) {
const int np = npgo[The_Grid()] = npgs[The_Grid()];
npgt[The_Grid()] = nacc;  nacc += np;
}
}

Then we let RecvBufBases[0][S] (or [1][0]) have the value for S defined above point
the entry next to the tail of pbuf (0, S−1). Then we call set_sendbuf_disps(), giving it
secondary = pc to let it take care of secondary particles if exist and parent = −1 to mean
the local node will not have helpand, to build SendBufDisps[s][m] for sbuf (s, m).
Next we perform the core part of the sorting by scanning pbuf (p, s) for all p ∈{0, pc}
and s ∈[0, S) whose size is TotalP[p][s]. For each Particles[i] having non-negative nid
element, we extract its subdomain identifer m and grid-position g by Neighbor_Subdomain_
Id() and Grid_Position() respectively. Then if p = 0 and m = n, we move it to SendBuf
[NOfPGridTotal[0][s][g]] and then increment NOfPGridTotal[0][s][g] for the next particle
in the grid-voxel at g. Otherwise, we move it to SendBuf[Pn + SendBufDisps[s][m]] and
then increment SendBufDisps[s][m] for the next particle to be sent to m. Note that we
move secondary particles of species s in the local node’s primary subdomain accidentally
to sbuf (s, n) rather than pbuf (0, s) because exchange_primary_particles() treats them
as the target of all-to-all communication.

RecvBufBases[s] = rbb;
sbuf = SendBuf + nacc;
set_sendbuf_disps(psold, -1);
for (ps=0,t=0,p=Particles,mysd=me; ps<=psold; ps++,mysd=-1) {
for (s=0,sbd=SendBufDisps; s<ns; s++,t++,sbd+=nn) {
dint *npgt = NOfPGridTotal[0][s];
const int itail = TotalP[t];
for (i=0; i<itail; i++,p++) {
const OH_nid_t nid = p->nid;
if (nid>=0) {
const int sdid = Neighbor_Subdomain_Id(nid, ps);
if (sdid==mysd)  SendBuf[npgt[Grid_Position(nid)]++] = *p;
else             sbuf[sbd[sdid]++] = *p;


<!-- Page 415 -->

}
}
}
}

Then we scan injected particles whose amount is nOfInjections = Qinjn  residing be-
yond the last pbuf (pc, S−1).  For each Particles[i] having non-negative nid element,
we extract its subdomain identifer m, grid-position g and species s by Subdomain_Id(),
Grid_Position(), Particle_Spec() respectively. Then if the nid of the particle is non-
negative and m = n, we move it to SendBuf[NOfPGridTotal[0][s][g]] and then increment
NOfPGridTotal[0][s][g] for the next particle in the grid-voxel at g. Note that the second
argument of Subdomain_Id() is 0 to let it to refer to AbsNeighbors[0][] if necessary. This
is correct for primary injected particles, and also for secondary ones because the subdomain
code is not less than 3D and thus the macro should give us m not less than N + 3D and
thus N without looking up AbsNeighbors[][].
Otherwise, m can be greater than N −1 to mean the particle was injected into or around
the local node’s old secondary subdomain.  If so, we perform Primarize_Id() to let the
particle has σ′ = σ −(N + 3D) in its subdomain code and to have real m. This operation
is subtle because Primarize_Id() may refers to AbsNeighbors[1][k] for the k-th neighbor
of the local node’s old secondary subdomain through the macro Subdomain_Id() invoked
in it. However, the array keeps correct values and thus the resulting subdomain identifier
m is also correct.
Then we move it to SendBuf[Pn+SendBufDisps[s][m]] and then increment SendBufDisps
[s][m] for the next particle to be sent to m. Note that we move particles of m = n to
sbuf (s, m) again.
Finally we call set_sendbuf_disps() again so that SendBufDisps[][] regains the dis-
placements of sbuf (s, m) for the reference in exchange_primary_particles().

for (i=0; i<ninj; i++,p++) {
const OH_nid_t nid = p->nid;
const int s = Particle_Spec(p->spec-sbase);
int sdid;
if (nid<0) continue;
sdid = Subdomain_Id(nid, 0);
if (sdid==me) SendBuf[NOfPGridTotal[0][s][Grid_Position(nid)]++] = *p;
else {
if (sdid>=nn) Primarize_Id(p, sdid);
sbuf[SendBufDisps[s*nn+sdid]++] = *p;
}
}
set_sendbuf_disps(psold, -1);
}


#### 4.10.39 sort_received_particles()

sort_received_particles()  The function sort_received_particles() is called from try_primary4p() with the ar-
gument nextmode = 0 and exchange_particles4p() with nextmode = 1, to sort particles
received from other nodes when we have normal accommodation and Qn + P nsend ≤Plim
and the local node will have secondary subdomain (psnew = pn = 1) or not (pn = 0) in the
next step. The other argument stats means we have to start new timing measurement if
required by stats ̸= 0.


<!-- Page 416 -->

After starting the new timing measurement with the key STATS_TB_SORT and nextmode
if required by stats ̸= 0, the function scans rbuf (p, s) for all p ∈{0, pn} and s ∈[0, S),
which are contiguously ranked from Particles[0] and are pointed by RecvBufBases[p][s].
For each Particles[i] in rbuf (p, s), we extract its grid-position g by Grid_Position() to
move it to SendBuf[NOfPGridTotal[p][s][g]] and then increment NOfPGridTotal[p][s][g] for
the next received particle in the grid-voxel at g. Note that all rbuf (p, s) are contiguously
ranked and thus RecvBufBases[pS + s + 1] points the entry next to the tail of rbuf (p, s)
including the case of p = 1 and s = S −1.


static void
sort_received_particles(const int nextmode, const int psnew, const int stats) {
const int ns=nOfSpecies;
int ps, s;
struct S_particle *p = Particles, **rbb = RecvBufBases+1;
Decl_Grid_Info();

if (stats) oh1_stats_time(STATS_TB_SORT, nextmode);
for (ps=0; ps<=psnew; ps++) {
for (s=0; s<ns; s++,rbb++) {
dint *npgt = NOfPGridTotal[ps][s];
const struct S_particle *rbtail = *rbb;
for (; p<rbtail; p++)  SendBuf[npgt[Grid_Position(p->nid)]++] = *p;
}
}
}


#### 4.10.40 Macros Local_Grid_Position() and Move_Or_Do()

Local_Grid_Position()  The macro Local Grid Position(g, k·2Γ+g, p), used in the macro Move_Or_Do() and the
function oh4p_remove_mapped_particle() directly, transforms a particle’s grid-position g
in the k-th neighbor of the local node n’s primary (p = 0) or secondary (p = 1) subdomain
into its corresponding index g′ in the n’s subdomain.  Note that it is assured that the
second argument nid of the particle should have k · 2Γ + g because Move_Or_Do() is used
in the functions called in normal accommodation or after the subdomain codes in nid of all
particles are let be ⌊3D/2⌋by count_population() with anywhere accommodation, and
oh4p_remove_mapped_particle() uses this macro if we are in normal accommodation.
Therefore, g′ is obtained by g + GridOffset[p][k] as discussed in §4.9.5.


#define Local_Grid_Position(G, NID, PS)  ((G) + GridOffset[PS][NID>>loggrid])


Move_Or_Do()  The macro Move Or Do(π, p, n′, µ, a), used in the particle scanning loop in move_to_
sendbuf_sec4p(), move_to_sendbuf_uw4p(), move_to_sendbuf_dw4p() and move_and_
sort_secondary(), examines a primary (p = 0) or secondary (p = 1) particle π of species
s in Particles[]. If nid of the particle is negative to mean the particle was eliminated, we
skip the iteration of the loop by continue. Otherwise we obtain its subdomain identifier m
by Neighbor_Subdomain_Id() and grid-position g by Grid_Position() if m = n′, or by
transforming what the macro gives into the local coordinate g by Local_Grid_Position()
otherwise, where n′ is the primary/secondary subdomain of the local node.


<!-- Page 417 -->

#define Move_Or_Do(P, PS, MYSD, MOVEIF, ACT) {\
const OH_nid_t nid = P->nid;\
int g = Grid_Position(nid);\
int sdid, dst;\
if (nid<0)  continue;\
sdid = Neighbor_Subdomain_Id(nid, PS);\
if (sdid!=(MYSD)) g = Local_Grid_Position(g, nid, PS);\
dst = npg[g];\

Then if c = npg[g] = NOfPGrid[p][s][g] = 0 to mean that the particle should stay in the
local node, we perform the operation a specified by the macro invoker. Note that npg is an
implicit argument given to the macro and has the pointer to NOfPGrid[p][s][0].
Otherwise and if µ ̸= 0, we do the followings. If c > 0 to mean that c = ((p′S + s)N +
m′)+1 for the one-dimensional index of [p′][s][m′] for sending π to the node m′, we move it to
SendBuf[β + NOfSend[p′][s][m′]] and then increment NOfSend[p′][s][m′] for the next particle
to be sent to m′, where β = Qn if the grand-invoker is move_and_sort_secondary(), or
β = 0 otherwise. Note that the pointer sb to SendBuf[β] is another implicit argument.
Otherwise, c is the negative index to the S_commlist record in hot-spot sending block
for a hot spot at g, namely C = CommList[−(c + 1)]. If t = C.tag < 0 to mean the record
is for the local node itself, we let NOfPGrid[p][s][g] = 0 and perform the action a for staying
particles.
Otherwise, we move the particle to SendBuf[β + NOfSend[p′][s][m′]] to send it to m′ =
C.rid and increment NOfSend[p′][s][m′] for the next particle to be sent to m′. Note that
the one-dimensional index of NOfSend[p′][s][m′] is obtained by t+m′ because t = (p′S+s)N.
Then C.count is decremented and if it becomes 0, c = NOfPGrid[p][s][g] is decremented to
let it have the negative index of the next record. We also let the nid element of the moved
particle −1 if µ < 076, so that it will be skipped when it is revisited in move_to_sendbuf_
uw4p() rather than mistakingly recognized as a staying particle because c can be 0 or be
pointing a record with t < 0.

if (dst==0)  { ACT; }\
else if (MOVEIF) {\
if (dst>0)  sb[NOfSend[dst-1]++] = *P;\
else {\
struct S_commlist *hs = CommList - (dst + 1);\
if (hs->tag<0) {\
npg[g] = 0;  ACT;\
} else {\
sb[NOfSend[hs->tag+hs->rid]++] = *P;\
if (MOVEIF<0)  P->nid = -1;\
if (--(hs->count)==0)  npg[g]--;\
}\
}\
}\
}


76We can do this operation always but do it only when it is really necessary for the sake of comprehen-
siveness. This restriction gives us a small performance benefit avoiding unnecessary memory write, while
the conditional operation should not cause any performance degradation because the condition (MOVEIF<0)
is replaced with (1<0) or (-1<0) which reasonably smart compilers must eliminate (together with the
assignment).


<!-- Page 418 -->

#### 4.10.41 move_to_sendbuf_sec4p()

move_to_sendbuf_sec4p()  The function move_to_sendbuf_sec4p(), called solely from exchange_particles4p()
when it finds Qn +Pnsend > Plim to mean we have to transfer particles among neighbors be-
fore sorting, is the position-aware counterpart of move_to_sendbuf_secondary() to move
particles to be sent to SendBuf[] and pack those to stay in the local nodes in Particles[].
It is given arguments psold = pc = 1 if the local node had a secondary subdomain or 0
otherwise, trans = t ̸= 0 iffwe have transitional state of helpand-helper configuration,
new },                       oldp = npold being the local node n’s helpand in the last step, nacc[2] = {Qnn, Qnpn
nsend = P nsend , and stats ̸= 0 if we have to start new timing measurement.
Note that having npold instead of npnew as an argument is essential for this function
and its callees move_to_sendbuf_uw4p() and move_to_sendbuf_dw4p(). They refer to
NOfPGrid[1][s][g] through the macro Move_Or_Do() to determine the fate of each secondary
particles, i.e., staying in the local node n or being sent to another node. Since this map
is corresponding to the secondary subdomain npold in the last step, g has to be calculated
based on npold. Therefore, Move_Or_Do() recognizes that a particle in the subdomain npold
is possibly to stay in the local node even if npold ̸= npnew due to helpand-helper reconfig-
uration, but the transfer schedule in NOfPGrid[1][][] definitely tells us all the secondary
particles should go out from the local node. This also means that a particle traveling to
the subdomain npnew being a helper of npold and thus being accommodated possibly by the
local node can be sent to the local node n itself. This subtle situation, however, should not
cause any problems because in this situation RealDstNeighbors[2][1] should have n as a
new helper of a neighbor of n’s old helpand npold and RealSrcNeighbors[2][1] should also
have n as a old helper of a neighbor of n’s new helpand npnew.


static void
move_to_sendbuf_sec4p(const int psold, const int trans, const int oldp,
const int *nacc, const int nsend, const int stats) {
const int me=myRank, ns=nOfSpecies, sbase=specBase;
const int ninj=nOfInjections, nplim=nOfLocalPLimit;
int ninjp=0, ninjs=nplim, i;
struct S_particle *sb = SendBuf,  *p;
Decl_Grid_Info();


After starting the new timing measurement with the key STATS_TB_MOVE if required by
stats ̸= 0, we call set_sendbuf_disps4p() to build the index array of sbuf (p, s, m) in
NOfSend[p][s][m] for m ∈RealDstNeighbors[t][p] giving it t as its argument.
Then we scan injected particles whose amount is nOfInjections = Qinjn  residing beyond
the last pbuf (pc, S−1), i.e., from Particles[totalParts]. For each Particles[i] having
non-negative nid, we extract its species s by Particle_Spec().  Then  if Secondary_
Injected() tells us that the particle was injected into or around the local node’s sec-
ondary subdomain in the last step, we perform Primarize_Id_Only() to let the particle
has σ′ = σ −(N + 3D) in its subdomain code. Then we move it in bottom-up manner
to the region from SendBuf[P nsend ] to SendBuf[Plim −1],  i.e., the empty region follwing
sbuf (1, S−1, N−1), if the injected subdomain m is npold and it was scheduled to stay, or to
sbuf (p′, s, m) otherwise by Move_Or_Do() which invokes Neighbor_Subdomain_Id() with
p = 1 and refers to NOfPGrid[1][s][g].
Otherwise, i.e., if Secondary_Injected() is false to mean the particle was injected into
or around the local node n’s primary subomain, we do the same as the secondary case, but


<!-- Page 419 -->

we move the particles to the top half of the region from top if m = n and scheduled to stay,
and let Move_Or_Do() acts for a primary particle.
Note that in each case we count the number of particles injected into primary/secondary
subdomain and staying in the local node to have qinjpri and qinjsec respectively.  Therefore,
injected and staying primary particles are moved to the region SendBuf[P nsend + i] where
i ∈[0, qinjpri), while secondary particles are moved to SendBuf[P nsend + Plim −i] where i ∈
[1, qinjsec]. Also note that the region for the injected particles should be large enough because
Plim ≥Qn ≥Pnsend + Qinjn ≥P nsend + (qinjpri + qinjsec).

if (stats) oh1_stats_time(STATS_TB_MOVE, 1);
set_sendbuf_disps4p(trans);

for (i=0,p=Particles+totalParts; i<ninj; i++,p++) {
const int s = Particle_Spec(p->spec-sbase);
const OH_nid_t nid = p->nid;
const int ps = Secondary_Injected(nid) ? 1 : 0;
dint *npg = NOfPGrid[ps][s];
if (nid<0) continue;
if (ps) {
Primarize_Id_Only(p);
Move_Or_Do(p, ps, oldp, 1, (sb[--ninjs]=*p));
} else
Move_Or_Do(p, ps, me  , 1, (sb[nsend+ninjp++]=*p));
}

Next we  call move_to_sendbuf_uw4p() to move primary  particles to be sent to
SendBuf[] and to pack those to stay upward giving it arguments ps = 0 for primary par-
ticles, mysd = n for the primary subdomain identifier, cbase = 0 to start the scan from
pbuf (0, 0), and nbase = 0 to mean the packed pbuf (0, 0) will be also at Particles[0].
Then if pc ̸= 0 to mean we have secondary particles, we call move_to_sendbuf_uw4p()
again but this time the arguments to be given are ps = 1 for secondary particles, mysd =
npold for the secondary subdomain identifier, cbase = primaryParts being Qnn in the last
step to start the scan from pbuf (1, 0), and nbase = nacc[0] being Qnn in the next step to
give the location of the packed pbuf (1, 0). We also call its downward counterpart move_to_
sendbuf_dw4p() with ps = 1 and mysd = npold too, and ctail = totalParts being Qn in
the last step to show the tail of pbuf (1, S−1), and ntail = nacc[0] + nacc[1] being Qn in
the next step and thus corresponding to the tail of the packed pbuf (1, S−1).
Otherwise, i.e., if pc = 0, we let;

∑s−1
RecvBufBases[1][s] = Particles + Qnn +    TotalPNext[1][t]
t=0

so that rbuf (1, s) = pbuf (1, s).
Finally, we call move_to_sendbuf_dw4p() (again) for the primary particles, with ps = 0
and mysd = n same as the upward counterpart, and ctail = primaryParts and ntail =
nacc[0] being the tail of pbuf (0, S−1) before and after packing respectively.

move_to_sendbuf_uw4p(0, me, 0, 0);
if (psold) {
move_to_sendbuf_uw4p(1, oldp, primaryParts, nacc[0]);
move_to_sendbuf_dw4p(1, oldp, totalParts, nacc[0]+nacc[1]);
} else {


<!-- Page 420 -->

struct S_particle *rbb=Particles+nacc[0];
int s;
for (s=0; s<ns; s++) {
RecvBufBases[ns+s] = rbb;  rbb += TotalPNext[ns+s];
}
}
move_to_sendbuf_dw4p(0, me, primaryParts, nacc[0]);


Then we move back injected and staying primary (p = 0) and secondary (p = 1)
particles whose amount is qinjpri and qinjsec respectively, from SendBuf[] to Particles[]. For
each particle of species s, obtained by Particle_Spec(), we move them to the location
pointed by RecvBufBases[p][s] and increment RecvBufBases[p][s] so that they are moved
into rbuf (p, s).
Finally, we let primaryParts and its shadow pointed by secondaryBase be Qnn in the
next step.

for (i=0,p=SendBuf+nsend; i<ninjp; i++,p++)
*(RecvBufBases[Particle_Spec(p->spec-sbase)]++) = *p;
for (i=ninjs,p=SendBuf+ninjs; i<nplim; i++,p++)
*(RecvBufBases[Particle_Spec(p->spec-sbase)+ns]++) = *p;

primaryParts = *secondaryBase = nacc[0];
}


#### 4.10.42 move_to_sendbuf_uw4p()

move_to_sendbuf_uw4p()  The function move_to_sendbuf_uw4p(), called solely from move_to_sendbuf_sec4p()
once or twice, scans particles in the local node n’s primary (ps = p = 0) or secondary (p = 1)
subdomain mysd = m from Particles[b−0 ] where b−0 = cbase argument for pbuf (p, 0) in
the last step.  It moves scanned particles to be sent to other nodes to SendBuf[], and
packs those to stay in the local node upward, i.e., to the direction of smaller indices of
Particles[] and to the region beginning Particles[b+0 ] where b+0 = nbase argument for
the packed pbuf (p, 0).


static void
move_to_sendbuf_uw4p(const int ps, const int mysd, const int cbase,
const int nbase) {
const int ns=nOfSpecies;
const int nsor0 = ps ? ns : 0;
const int *ctp = TotalP + nsor0,  *ntp = TotalPNext + nsor0;
struct S_particle *p,  **rbb = RecvBufBases + nsor0,  *sb = SendBuf;
int s, c, d, cn, dn;
Decl_Grid_Info();


We scan particles in each pbuf (p, s) for all s ∈[0, S) and determine the direction of
packing particles to stay in the local node as follows. Let us define b−s and b+s  recursively
as b−s+1 = b−s + TotalP[p][s] and b+s+1 = b+s + TotalPNext[p][s], where TotalPNext[p][s] =
|pbuf (p, s)| was accummulated by functions called by make_send_sched(), namely make_
send_sched_body(), scatter_hspot_send() and scatter_hspot_recv_body().
If b+s ≤b−s , it is assured that the packing direction is upward because, for the particle
being i-th and j-th in pbuf (p, s) in the last and next step respectively, it is assured j ≤i


<!-- Page 421 -->

and thus their indices b+s + j ≤b−s + i. Therefore, we move all particles in pbuf (p, s) from
its top to bottom by Move_Or_Do() to let it move Particles[b−s + i] to Particles[b+s + j]
for particles to stay. After that, we let RecvBufBases[p][s] point Particles[b+s + l] where
l is the number of particles staying in pbuf (p, s) so that rbuf (p, s) is placed at the bottom
of pbuf (p, s) for the next step.
If b+s+1 > b−s+1, on the other hand, we can pack staying particles in pbuf (p, s) by moving
downward, because it is assured b+s+1 −j > b−s+1 −i for j ≤i.  However we have to
postpone it after packing pbuf (p, s+1) and its successors.  Therefore, we leave them to
move_to_sendbuf_dw4p() but just let RecvBufBases[p][s] point Particles[b+s ], i.e., the
top of pbuf (p, s) for the next step.
Otherwise, i.e., if b+s > b−s but b+s+1 ≤b−s+1, we have to pack the second half upward
and then the first half downward. First we find the first staying particle being im-th and
jm-th in pbuf (p, s) such that b+s + jm ≤b−s + im, by Move_Or_Do() letting it to move
all particles to be sent to SendBuf[]. Note that this scan may make a hot-spot only have
particles staying in the local node if any letting NOfPGrid[p][s][g] in question be 0, so that
those staying particles is correctly reconginzed as staying in this scan and the backward
scan done afterward. At the same time, all scanned particles to be sent in the hot-spot are
eliminated by letting their nid be −1 because the fourth argument of the macro is −1, in
order to keep them from mistakingly recognized as staying particles in the backward scan.
Then we continue the scan from im and jm to move all remaining particles in pbuf (p, s)
by Move_Or_Do() to let it move Particles[b−s + i] to Particles[b+s + j] for particles to
stay. After that, we let RecvBufBases[p][s] point Particles[b+s + l] where l is the number
of particles staying in pbuf (p, s) so that rbuf (p, s) is placed at the bottom of pbuf (p, s)
for the next step. Then finally, we scan the first half again from the bottom im −1 and
jm −1 to the top by Move_Or_Do() to let it move particles to stay downward, but to let
it do nothing for particles with NOfPGrid[p][s][g] ̸= 0 because they have already moved to
SendBuf[] and NOfPGrid[p][s][g] = 0 for all staying particles including those in hot-spots.

for (s=0,c=cbase,d=nbase; s<ns; s++,c=cn,d=dn) {
dint *npg = NOfPGrid[ps][s];
cn = c + ctp[s];  dn  = d + ntp[s];
if (d<=c) {
for (p=Particles+c; c<cn; c++,p++)
Move_Or_Do(p, ps, mysd, 1, (Particles[d++]=*p));
rbb[s] = Particles + d;
} else if (dn>cn) {
rbb[s] = Particles + d;
} else {
const int cb = c;
int cm, dm;
for (p=Particles+c; c<d; c++,p++)  Move_Or_Do(p, ps, mysd, -1, (d++));
cm = c - 1;  dm = d - 1;
for (p=Particles+c; c<cn; c++,p++)
Move_Or_Do(p, ps, mysd, 1, (Particles[d++]=*p));
rbb[s] = Particles + d;
for (c=dm,d=dm,p=Particles+c; c>=cb; c--,p--)
Move_Or_Do(p, ps, mysd, 0, (Particles[d--]=*p));
}
}
}


<!-- Page 422 -->

#### 4.10.43 move_to_sendbuf_dw4p()

move_to_sendbuf_dw4p()  The function move_to_sendbuf_dw4p(), called solely from move_to_sendbuf_sec4p()
once or twice, scans particles in the local node n’s primary (ps = p = 0) or secondary
(p = 1) subdomain mysd = m from Particles[b−S −1] where b−S = ctail argument for
the element following pbuf (p, S−1) in the last step. It moves scanned particles to be sent
to other nodes to SendBuf[], and packs those to stay in the local node downward, i.e., to
the direction of greater indices of Particles[] and to the region ending Particles[b+S −1]
where b+S = ntail argument for the element following the packed pbuf (p, S−1).


static void
move_to_sendbuf_dw4p(const int ps, const int mysd, const int ctail,
const int ntail) {
const int ns=nOfSpecies;
const int nsor0 = ps ? ns : 0;
const int *ctp = TotalP + nsor0,  *ntp = TotalPNext + nsor0;
struct S_particle *sb = SendBuf,  *p;
int s, c, d, cn, dn;
Decl_Grid_Info();

We scan particles in pbuf (p, s) such that b+s+1 > b−s+1 where b−s and b+s  are defined
recursively as b−s+1 = b−s +TotalP[p][s] and b+s+1 = b+s +TotalPNext[p][s], or in other words
b−s = b−s+1 −TotalP[p][s] and b+s = b+s+1 −TotalPNext[p][s]. For such pbuf (p, s), we can
pack staying particles in it by moving downward, because it is assured b+s+1−j > b−s+1−i for
a particle being i-th and j-th in pbuf (p, s) from its tail in the last and next step respectively
and thus j ≤i. Therefore, we move all particles in pbuf (p, s) from its bottom to top by
Move_Or_Do() to let it move Particles[b−s+1 −i] to Particles[b+s+1 −j] for particles to
stay, i.e. those in grid-voxels at g such that NOfPGrid[p][s][g] = 0.

cn = ctail;  dn = ntail;
for (s=ns-1,c=cn-1,d=dn-1; s>=0; s--,c=cn-1,d=dn-1) {
dint *npg = NOfPGrid[ps][s];
cn -= ctp[s];  dn -= ntp[s];
if (c>=d || cn>=dn)  continue;
for (p=Particles+c; c>=cn; c--,p--)
Move_Or_Do(p, ps, mysd, 1, (Particles[d--]=*p));
}
}


#### 4.10.44 move_and_sort_secondary()

move_and_sort_secondary()  The function move_and_sort_secondary(), called solely from exchange_particles4p()
when it finds Qn +P nsend ≤Plim to mean we can move particles staying in and leaving from
the local node together from Particles[] to SendBuf[] with sorting. It is given the following
arguments; psold = pc = 1 iffthe local node had secondary particles; psnew = pn = 1 iff
it will have secondary particles; trans = t ∈{0, 1} being 1 iffwe have transitional state of
helpand-helper configuration; oldp = npold being the local node n’s helpand in the last step;
new }; and stats ̸= 0 if we have to start new timing measurement. The                            nacc[2] = {Qnn, Qnpn
reason why this function needs to have npold instead of npnew is same as what we discussed
in §4.10.41.


<!-- Page 423 -->

static void
move_and_sort_secondary(const int psold, const int psnew, const int trans,
const int oldp, const int *nacc, const int stats) {
const int me=myRank, ns=nOfSpecies, nn=nOfNodes, sbase=specBase;
const int mysubdom[2] = {me, oldp},  ninj = nOfInjections;
struct S_particle *p,  *rbb,  *sb = SendBuf + nacc[0] + nacc[1];
int *nofr;
int ps, s, t, npt, i;
Decl_For_All_Grid();
Decl_Grid_Info();


After starting the new timing measurement with the key STATS_TB_MOVE if required by
stats ̸= 0, we call set_sendbuf_disps4p() to build the index array of sbuf (p, s, m) in
NOfSend[p][s][m] for m ∈RealDstNeighbors[t][p] giving it t as its argument.
Then we do the followings for all p ∈{0, pn} (not {0, pc}) and s ∈[0, S). First we let
RecvBufBases[p][s] be as follows.

NS(n) = RealSrcNeighbors[t][0].nbor[]
NS(npnew) = RealSrcNeighbors[t][1].nbor[]
∑s−1 ∑
RecvBufBases[0][s] = Particles +             NOfRecv[0][s′][m]
s′=0 m∈NS(n)
RecvBufBases[1][s] = Particles +
S−1∑ ∑       ∑s−1 ∑
NOfRecv[0][s′][m] +                NOfRecv[1][s′][m]
s′=0 m∈NS(n)                     s′=0 m∈NS(npnew)
∑
That is, we let |rbuf (p, s)| =   m∈Ns(n′) NOfRecv[p][s][m] where n′ = {n, npnew}[p], rank
them contiguously from Particles[0], and  let RecvBufBases[p][s] point the head of
rbuf (p, s).  Note that we also  let RecvBufBases[1][S] be the value defined above so
that sort_received_particles() refers to  it pointing the entry following the tail of
rbuf (1, S−1).
Then we scan NOfPGridOut[p][s][g] to build the index array for sorting in NOfPGridTotal
[p][s][g] as discussed in §4.10.37, by For_All_Grid() giving 0 to its first argument if p = 0,
or t + 1 if p = 1 so that it refers to appropriate element of GridDesc[].

if (stats) oh1_stats_time(STATS_TB_MOVE, 1);
set_sendbuf_disps4p(trans);
for (ps=0,t=0,nofr=NOfRecv,rbb=Particles,npt=0; ps<=psnew; ps++) {
const int nnbr = RealSrcNeighbors[trans][ps].n;
const int *rnbr = RealSrcNeighbors[trans][ps].nbor;
const int gdidx = ps ? trans+1 : 0;
for (s=0; s<ns; s++,t++,nofr+=nn) {
int n, nrec;
dint *npgt = NOfPGridTotal[ps][s];
const int *npgo = NOfPGridOut[ps][s];
for (n=0,nrec=0; n<nnbr; n++)  nrec += nofr[rnbr[n]];
RecvBufBases[t] = rbb;  rbb += nrec;
For_All_Grid(gdidx, 0, 0, 0, 0, 0, 0) {
const int np = npgo[The_Grid()];
npgt[The_Grid()] = npt;  npt += np;


<!-- Page 424 -->

}
}
}
RecvBufBases[t] = rbb;


Then for all p ∈{0, pc} and s ∈[0, S), we scan all particles in pbuf (p, s) whose
size is TotalP[p][s] invoking Move_Or_Do() to move each of them in grid-voxel at g to
SendBuf[NOfPGridTotal[p][s][g]] and increment the index if it stays in the local node, or
to SendBuf[Qn+NOfSend[p′][s]] and increment the index too where p′ = 0 if the particle
becomes primary of m or p′ = 1 otherwise, according to NOfPGrid[p][s][g].

for (ps=0,p=Particles,t=0; ps<=psold; ps++) {
const int mysd = mysubdom[ps];
for (s=0; s<ns; s++,t++) {
dint *npg = NOfPGrid[ps][s],  *npgt = NOfPGridTotal[ps][s];
const int itail = TotalP[t];
for (i=0; i<itail; i++,p++)
Move_Or_Do(p, ps, mysd, 1, (SendBuf[npgt[g]++]=*p));
}
}

We continue the scan for injected particles whose amount is nOfInjections = Qinjn
residing beyond the last pbuf (pc, S−1).  For each Particles[i] having non-negative nid
element, we extract its species s by Particle_Spec() and examines if it was injected into
or around the local node’s primary (p = 0) or secondary (p = 1) subdomain in the last
step by Secondary_Injected(), performing Primarize_Id_Only() to let the particle has
σ′ = σ −(N + 3D) in its subdomain code in the secondary case. Then we move the particle
to SendBuf[] by Move_Or_Do() giving it n (p = 0) or npold (p = 1) for the subdomain
identifier and letting it refer to NOfPGrid[p][s][g] and NOfPGridTotal[p][s][g] where g is the
grid-position of the particle.
Finally, we let primaryParts and its shadow pointed by secondaryBase be Qnn in the
next step.

for (i=0; i<ninj; i++,p++) {
const int s = Particle_Spec(p->spec-sbase);
const OH_nid_t nid = p->nid;
const int ps = Secondary_Injected(nid) ? 1 : 0;
const int mysd = mysubdom[ps];
dint *npg = NOfPGrid[ps][s],  *npgt = NOfPGridTotal[ps][s];
if (nid<0) continue;
if (ps)  Primarize_Id_Only(p);
Move_Or_Do(p, ps, mysd, 1, (SendBuf[npgt[g]++]=*p));
}
primaryParts = *secondaryBase = nacc[0];
}


#### 4.10.45 set_sendbuf_disps4p()

set_sendbuf_disps4p()  The function set_sendbuf_disps4p(), called from move_to_sendbuf_sec4p() and move_
and_sort_secondary() prior to their particle scan, is the position-aware counterpart of
set_sendbuf_disps() to build the index array for SendBuf[] in NOfSend[][][] based on the
sending counts in itself. The function is given an argument trans = t ∈{0, 1} being 1


<!-- Page 425 -->

iffwe have transitional state of helpand-helper configuration and thus we need to refer to
RealDstNeighbors[1][].
The function  lets NOfSend[p][s][m] as follows so that sbuf (p, s, m) are ranked in
SendBuf[] contiguously and |sbuf (p, s, m)| is the original value of NOfSend[p][s][m] which
we refer to as qsend(p, s, m);

NOfSend[p][s][mi] =
∑p−1 S−1∑ c(q)−1∑      ∑s−1 c(p)−1∑      ∑i−1
qsend(q, t, mj(q)) +           qsend(p, t, mj(p)) +     qsend(p, s, mj(p))
q=0 t=0  j=0                     t=0  j=0                    j=0

where;

m ∈ND(n(p)) = {m0(p), . . . , mc(p)−1(p)} = RealDstNeighbors[t][p].nbor[]
n(p) = {n, npold}[p]
c(p) = |ND(n(p))| = RealDstNeighbors[t][p].n

Roughly speaking, the equation above is calculated recursively by;

NOfSend[p][s][mi] = NOfSend[p][s][mi−1] + qsend(p, s, mi−1)

if we may consider that NOfSend[p][s][m−1(p)] = NOfSend[p][s −1][mc(p)−1(p)] and so on.


static void
set_sendbuf_disps4p(const int trans) {
const int nn=nOfNodes, ns=nOfSpecies;
int ps, s, i, np, *sbd;

for (ps=0,sbd=NOfSend,np=0; ps<2; ps++) {
const int n = RealDstNeighbors[trans][ps].n;
const int *nbor = RealDstNeighbors[trans][ps].nbor;
for (s=0; s<ns; s++,sbd+=nn) {
for (i=0; i<n; i++) {
const int nid = nbor[i];
const int nsend = sbd[nid];
sbd[nid] = np;  np += nsend;
}
}
}
}


#### 4.10.46 xfer_particles()

xfer_particles()  The function xfer_particles(), called solely from exchange_particles4p() in two cases
with Qn + P nsend ≤Plim or not, sends particles in the local node to other nodes in
RealDstNeighbors[t][] and receives particles from other nodes in RealSrcNeighbors[t][],
where t = trans ∈{0, 1} argument being 1 iffwe have transitional state of helpand-helper
configuration. The other arguments are psnew = pn ∈{0, 1} being 1 iffthe local node will
have secondary subdomain in the next step and thus may have some secondary particles
to receive, and sbuf is the pointer to SendBuf[0] or SendBuf[Qn] to specify the location of
sbuf (0, 0, 0).


<!-- Page 426 -->

static void
xfer_particles(const int trans, const int psnew, struct S_particle *sbuf) {
const int nn=nOfNodes, ns=nOfSpecies;
int ps, s, t, i, req, sdisp, *nofr, *nofs;


First, we post MPI_Irecv() to receive particles, whose amount is NOfRecv[p][s][mi] > 0
and data-type is T_Particle, from the node mi = RealSrcNeighbors[t][p].nbor[i] for all
p ∈{0, pn}, s ∈[0, S) and i ∈[0, c(p)) where c(p) = RealSrcNeighbors[t][p].n. The loca-
tion of the receiving buffer is in rbuf (p, s) in which particles received from all sender nodes
∑i−1
are ranked contiguously, and thus it is from RecvBufBases[p][s] +   j=0 NOfRecv[p][s][j].
The tag of the communication is pS + s so that coupling it with mi makes each communi-
cation unique.

for (ps=0,t=0,nofr=NOfRecv,req=0; ps<=psnew; ps++) {
const int n = RealSrcNeighbors[trans][ps].n;
const int *nbor = RealSrcNeighbors[trans][ps].nbor;
for (s=0; s<ns; s++,t++,nofr+=nn) {
struct S_particle *rbuf = RecvBufBases[t];
for (i=0; i<n; i++) {
const int nid = nbor[i];
const int nrecv = nofr[nid];
if (nrecv) {
MPI_Irecv(rbuf, nrecv, T_Particle, nid, t, MCW, Requests+req++);
rbuf += nrecv;
}
}
}
}

Next, we post MPI_Isend() to send particles of T_Particle to the nodes mi =
RealDstNeighbors[t][p].nbor[i] for  all p ∈{0, 1}, s ∈[0, S) and  i ∈[0, c(p)) where
c(p) =  RealDstNeighbors[t][p].n.  The sending buffer  is sbuf (p, s, mi) whose offset
from sbuf (0, 0, 0)  is NOfSend[p][s][mi−1]  if we consider [p][s][m−1]  is [p][s−1][mc(p)−1]
etc., and since sbuf (p, s, mi) are ranked contiguously, the amount of sending particles is
NOfSend[p][s][mi] −NOfSend[p][s][mi−1]. The tag of the communication is pS + s again to
ensure the uniqueness of the communication. Note that we let NOfSend[p][s][mi] = 0 for
the use of the next step.

for (ps=0,t=0,sdisp=0,nofs=NOfSend; ps<2; ps++) {
const int n = RealDstNeighbors[trans][ps].n;
const int *nbor = RealDstNeighbors[trans][ps].nbor;
for (s=0; s<ns; s++,t++,nofs+=nn) {
for (i=0; i<n; i++) {
const int nid = nbor[i];
const int sdnxt = nofs[nid];
const int nsend = sdnxt - sdisp;
nofs[nid] = 0;
if (nsend) {
MPI_Isend(sbuf+sdisp, nsend, T_Particle, nid, t, MCW,
Requests+req++);
}
sdisp = sdnxt;


<!-- Page 427 -->

}
}
}

Finally, we confirm the completions of all MPI_Irecv() and MPI_Isend() by MPI_
Waitall() recorded in Requests[] to obtain their completion status in Statuses[] (but
not referring to).

MPI_Waitall(req, Requests, Statuses);
}


#### 4.10.47 Macro Check_Particle_Location()

Check_Particle_Location()  The macro Check Particle Location(π, p, s, S, i) checks the consistency of the arguments
π, p and s given to its invoker oh4p_map_particle_to_neighbor(), oh4p_map_particle_
to_subdomain() or oh4p_remove_mapped_particle(), if OH_NO_CHECK is not #defined.
It always checks  if p ∈{0, 1} and s ∈[0, S). Then  if PbufIndex ̸= NULL to mean
transbound4p() has already been called to give us meaningful values in it and totalParts,
we further check the following consistencies.  For injected particles (i ̸= 0), we checks
that parent(n) = RegionId[1] ≥0  if p = 1 and that π points a location not beyond
Particles[Qn + Qinjn  ], i.e., π < Particles + totalParts + nOfInjections. For ordinary
particles (i = 0) on the other hand, it checks π points a location in pbuf (p, s), i.e.

Particles + PbufIndex[p][s] ≤π < Particles + PbufIndex[p][s+1]

where we consider PbufIndex[p][S] = PbufIndex[p+1][0]. Then if one or more examinations
fail, we abort the execution by local_errstop() showing the particle index π−Particles
and values of p and s.
If OH_NO_CHECK is #defined, on the other hand, the macro replacement gives us nothing.


#ifndef OH_NO_CHECK
#define Check_Particle_Location(P, PS, S, NS, INJ) {\
const int t = (PS) ? (S)+(NS) : (S);\
const int pidx = (P) - Particles;\
if ((PS)<0 || (PS)>1 || (S)<0 || (S)>=(NS) ||\
(PbufIndex && ((INJ) ?\
(((PS)&&RegionId[1]<0) ||\
pidx>=totalParts+nOfInjections) :\
(pidx<PbufIndex[t] || pidx>=PbufIndex[t+1]))))\
local_errstop("’part’ argument pointing %c%d%c of the particle buffer is "\
"inconsistent with ’ps’=%d and ’s’=%d",\
specBase?’(’:’[’, pidx+specBase,\
specBase?’)’:’]’, PS, (S)+specBase);\
}
#else
#define Check_Particle_Location(P, PS, S, NS, INJ)
#endif


<!-- Page 428 -->

4.10.48  Macros Map_Particle_To_Neighbor() and Adjust_Neighbor_Grid()

Map_Particle_To_Neighbor()  The macro Map Particle To Neighbor(π, Xd, d, m, k, 3d, δd(m), xd, g), used solely in oh4p_
map_particle_to_neighbor() D-times, examines the d-th dimensional coordinate Xd of
the position of particle π in the subdomain m or its exterior to find the subdomain in which
π resides. It updates the neighbor index k by ±3d if π is in d-th dimensional exterior. It
also calculates d-th dimensional integer coordinate xd local to m for Xd, and updates grid-
voxel’s partial index g by adding xd, and then convert xd to the local coordinate of the
subdomain in whch π resides if it is in the d-th dimensional upper exterior of m.
First, we calculate x′d = (Xd −∆ld · γd)/γd + ∆ld, where ∆ld · γd = Grid[d].fcoord[0],
1/γd = Grid[d].rgsize, and ∆ld = Grid[d].coord[0], to have the global integer coordinate
x′d for Xd. Then we make the following correction for the floating point calculation error
so that x′dγd ≤Xd < (x′d + 1)γd, where γd = Grid[d].gsize.

 x′d −1  Xd < x′dγd
x′d ←   x′d       x′dγd ≤Xd < (x′d + 1)γd                              
x′d + 1   (x′d + 1)γd ≤Xd

Then we have the local coordinate xd for m by subtracting δln(m) = SubDomains[m][d][0]
from x′d, i.e. xd = x′d −δld(m), and update the partial index g = gidx(0, xd+1, . . .) by adding
xd to it, i.e., g ←g + xd = gidx(xd, xd+1, . . .).
Next if xd < 0 to mean π is in the d-th dimensional lower exterior of m, we update k =                  ∑d                               e=0 3e+∑D−1e=d+1 νe3e by subtracting 3d from it to have k ←k−3d = ∑d−1e=0 3e+∑D−1e=d νe3d
where νd = 0 for d-th dimensional lower neighbors. We also check if Xd < ∆ld · γd and,
if so, confirm that the d-th dimensional lower system boundary condition is periodic, i.e.,
Boundaries[m][d][0] = 0, to let Xd ←Xd+(∆ud −∆ld)γd where ∆ud ·γd = Grid[d].fcoord[1].
If this examination fails to mean π is crossing a non-periodic system boundary, we make
π eliminated by letting its nid be −1 and force oh4p_map_particle_to_neighbor() to
return to its caller. We also confirm xd ≥−eg, and if unsatisfied we let k = −3D to let
oh4p_map_particle_to_neighbor() to consult oh4p_map_particle_to_subdomain() to
have the subdomain to which π warped. Note that letting k = −3D makes it sure that the
series of the invocations of this macro results k < 0 because other invocations add at most
3D −2 to k.
If xd ≥δd(m), on the other hand, to mean π is in the d-th dimensional upper exterior
∑d−1   ∑D−1
of m, we let k ←k + 3d =   e=0 3e +   e=d νe3d where νd = 2 for d-th dimensional upper
neighbors. We also check if Xd ≥∆ud · γd and,  if so, confirm that the d-th dimensional
upper system boundary condition is periodic, i.e., Boundaries[m][d][1] = 0, to let Xd ←
Xd −(∆ud −∆ld)γd.  If this examination fails to mean π is crossing a non-periodic system
boundary, we let π.nid = −1 to mean π has gone away and force oh4p_map_particle_
to_neighbor() return to its caller with −1. Then, unlike the lower boundary case, we let
xd ←xd −δd(m) to make xd local to the neighbor because this conversion can be done
without knowing the neighbor subdomain’s shape. We also confirm, like lower boundary
case, xd < eg, and if unsatisfied we let k = −3D for consulting oh4p_map_particle_to_
subdomain() too.


#define Map_Particle_To_Neighbor(P, XYZ, DIM, MYSD, K, INC, UB, G, IDX) {\
const double xyz = XYZ;\
const double gsize = Grid[DIM].gsize;\
const double lb = Grid[DIM].fcoord[OH_LOWER];\
const double gf =\
(G = (xyz-lb)*Grid[DIM].rgsize + Grid[DIM].coord[OH_LOWER]) * gsize;\


<!-- Page 429 -->

if (gf>xyz) G--;\
else if (gf+gsize<=xyz) G++;\
G -= SubDomains[MYSD][DIM][OH_LOWER];  IDX += G;\
if (G<0) {\
K -= INC;\
if (xyz<lb) {\
if (Boundaries[MYSD][DIM][OH_LOWER]) { P->nid = -1;  return(-1); }\
XYZ += Grid[DIM].fcoord[OH_UPPER] - lb;\
}\
if (G<-OH_PGRID_EXT)  K = -OH_NEIGHBORS;\
} else if (G>=UB) {\
double ub = Grid[DIM].fcoord[OH_UPPER];\
K += INC;\
if (xyz>=ub) {\
if (Boundaries[MYSD][DIM][OH_UPPER]) { P->nid = -1;  return(-1); }\
XYZ -= ub - lb;\
}\
G-=UB;\
if (G>=OH_PGRID_EXT)  K = -OH_NEIGHBORS;\
}\
}


Adjust_Neighbor_Grid()  The macro Adjust Neighbor Grid(xd, m, d),  used  solely  in oh4p_map_particle_to_
neighbor() D-times, transform the d-th dimensional local coordinate xd < 0 of the lo-
cal node’s primary/secondary subdomain into that of its neighbor m. That is, if xd < 0 we
let xd ←xd + δd(m) = xd + (δud(m) −δld(m)) where δβd (m) = SubDomains[m][d][β].


#define Adjust_Neighbor_Grid(G, N, DIM)\
if (G<0) G += SubDomains[N][DIM][OH_UPPER]-SubDomains[N][DIM][OH_LOWER];


#### 4.10.49 oh4p_map_particle_to_neighbor()

oh4p_map_particle_to_neighbor_()  The API functions oh4p_map_particle_to_neighbor_()  for Fortran and oh4p_map_
oh4p_map_particle_to_neighbor()  particle_to_neighbor() for C find the subdomain m, which is primary (ps = p = 0)
or secondary (p = 1) subdomain of the local node n or its neighbor, should accommodate
the particle of species s = s in Particles[] and pointed by the argument pointer part = π,
and return m or −1 if particle went out-of-bounds. The function for C is also called from
oh4p_inject_particle() and oh4p_remap_particle_to_subdomain() to find the subdo-
main for a particle injected and mapped again respectively.
They have a number of differences from their level-3 couterparts oh3_map_particle_
to_neighbor[_]().  First, they receive the S_particle structure pointer π rather than
their position coordinate values.  Second, they need species s to refer to an element of
NOfPLocal[][][], mantaining it by the functions rather than the simulator body to bring
another difference, and NOfPGrid[][][] for which we add calculations of integer coordinate
values and the grid-position of the particle. Third, they accept particles causing anywhere
accommodation by calling oh4p_map_particle_to_subdomain() if the particles travel to
outside of the exterior of the primary/secondary subdomain.
The function oh4p_map_particle_to_neighbor_() simply calls its counterpart oh4p_
map_particle_to_neighbor(), but decrementing s by 1 to make it zero-origined.


<!-- Page 430 -->

int
oh4p_map_particle_to_neighbor_(struct S_particle *part, const int *ps,
const int *s) {
return(oh4p_map_particle_to_neighbor(part, *ps, *s-1));
}
int
oh4p_map_particle_to_neighbor(struct S_particle *part, const int ps,
const int s) {
const int ns = nOfSpecies,  inj = part>=Particles+totalParts;
int x, y, z, w, d, dw, mysd;
const int psnn = ps ? (s+nOfSpecies)*nOfNodes : s*nOfNodes;
int k = OH_NBR_SELF,  idx = 0;
int gz, gy, gx;
int sd;
Decl_Grid_Info();


First, we invoke Check_Particle_Location() to check the consistency of arguments,
before referring to elements in GridDesc[] and RegionId[]. The fifth argument of the macro
i is determined by whether π is beyond Particles+nOfInjections, i.e., π is for an injected
particle. Then, we invoke Map_Particle_To_Neighbor() D-times for d = D −1 down to
d = 0, with Do_Z() and Do_Y() to skip invocations if D < 3 or D < 2 respectively. To the
macro, we give the following arguments.

- the particle pointer π.

- d-th dimensional coordinate Xd of π’s position.

- the dimension d.

- local node’s subdomain identifier n′ = {n, parent(n)}[p] = RegionId[p].

- neighbor index k initialized to be ⌊3D/2⌋for the subdomain n′ itself.

- 3d to update k to let it have the neighbor index for the target subdomain m at last.

- d-th dimensional size of n′ being δd(n′) = GridDesc[p].{x, y, z}[d].

- the variable xd to have the local coordinate value.

- grid-position g intitalized to be 0 and multiplied by δmaxd−1 + 4eg = GridDesc[p].{w, d}
[d−1] after the invocation to have g = gidx(x0, . . .) for n’ at last.


Check_Particle_Location(part, ps, s, ns, inj);
x = GridDesc[ps].x;  y = GridDesc[ps].y;  z = GridDesc[ps].z;
w = GridDesc[ps].w;  d = GridDesc[ps].d;  dw = GridDesc[ps].dw;
mysd = RegionId[ps];
Do_Z(Map_Particle_To_Neighbor(part, part->z, OH_DIM_Z, mysd, k, 9, z, gz,
idx));
Do_Z(idx *= d);
Do_Y(Map_Particle_To_Neighbor(part, part->y, OH_DIM_Y, mysd, k, 3, y, gy,
idx));
Do_Y(idx *= w);
Map_Particle_To_Neighbor(part, part->x, OH_DIM_X, mysd, k, 1, x, gx, idx);


<!-- Page 431 -->

Now we have the neighbor index k, being most likely ⌊3D/2⌋to mean π stays in the sub-
odomain n′ it has resided. If so, g can be used both as the index of NOfPGrid[][][] and as the
grid-position part of π’s nid. Moreover, we know the subdomain is n′ for NOfPLocal[][][] and
for the return value without looking up AbsNeighbors[][]. Therefore, we quickly perform
necessary operations before returning to the caller with n′; incrementing NOfPGrid[p][s][g]
and NOfPLocal[p][s][n′]; letting nid be k · 2Γ + g by Combine_Subdom_Pos(). The other
operations we have to do before returning is to increment InjectedParticles[0][p][s] if the
particle is injected, because we need it maintained for non-position-aware particle transfer.
In addition we let the subdomain code of nid be k +N +3D if p = 1 by Secondarize_Id()
to tell its secondariness to the functions called from transbound4p().
On the other hand, one of Map_Particle_To_Neighbor() may let k = −3D to make
final k be negative to mean the particle warped outside the exterior of the local node’s
primary/secondary subdomain. If so, we consult oh4p_map_particle_to_subdomain() to
have the subdomain of the particle.

if (k==OH_NBR_SELF) {
NOfPGrid[ps][s][idx]++;
NOfPLocal[psnn+mysd]++;
part->nid = Combine_Subdom_Pos(k, idx);
if (inj) {
if (ps) {
InjectedParticles[ns+s]++;  Secondarize_Id(part);
} else {
InjectedParticles[s]++;
}
}
return(mysd);
} else if (k<0)
return(oh4p_map_particle_to_subdomain(part, ps, s));

Otherwise, i.e., if k ≥0 but k ̸= ⌊3D/2⌋, we have to consult AbsNeighbors[][] to have
m = AbsNeighbors[p][k]. Then  if m ≥N to indicate the neighbor does not exist, we
make π eliminated and return to the simulator body with −1. Otherwise we continue to
let xd ←xd + δd(m) for all d ∈[0, D) by Adjust_Neighbor_Grid() if xd < 0, to have
the local coordinates in m. We also maintain the per-subdomain population histogram by
incrementing NOfPLocal[p][s][m].

sd = AbsNeighbors[ps][k];
if (sd>=nOfNodes) {
part->nid = -1;  return(-1);
}
Adjust_Neighbor_Grid(gx, sd, OH_DIM_X);
Do_Y(Adjust_Neighbor_Grid(gy, sd, OH_DIM_Y));
Do_Z(Adjust_Neighbor_Grid(gz, sd, OH_DIM_Z));
NOfPLocal[psnn+sd]++;

Finally, with g being the grid-voxel index in the exterior of n′ and xd for d ∈[0, D) being
the grid-voxel coordinate in m, we increment NOfPGrid[p][s][g] and let π’s nid element be
k·2Γ+gidx(x0, . . .) by Coord_To_Index() and Combine_Subdom_Pos(). However, if m = n′
meaning n′ has itself as its neighbor and π travels to the neighbor subdomain, π must be
counted as a member in the grid-voxel at gidx(x0, . . .) rather than at g and π must have


<!-- Page 432 -->

⌊3D/2⌋in the subdomain code of its nid, to avoid complication due to g ̸= gidx(x0, . . .)
in n′. This also solves relatively minor problems by assuring that we will not make self-
communication for particles staying a subdomain eventually and that we will not have any
particles in the exterior of n′ if the corresponding neighbor is n′ itself. The other operation
we have to do for m = n′ case is to increment InjectedParticles[p][s] if π is an injected
particle as discussed before.
Then really finally, we return to the caller with m, after performing Secondarize_Id()
if the particle is injected and p = 1 as discussed before.

if (sd==mysd) {
idx = Coord_To_Index(gx, gy, gz, w, dw);
NOfPGrid[ps][s][idx]++;
part->nid = Combine_Subdom_Pos(OH_NBR_SELF, idx);
if (inj)  InjectedParticles[ps ? ns+s : s]++;
} else {
NOfPGrid[ps][s][idx]++;
part->nid = Combine_Subdom_Pos(k, Coord_To_Index(gx, gy, gz, w, dw));
}
if (inj && ps)  Secondarize_Id(part);
return(sd);
}


4.10.50  Macros Map_To_Grid, Map_Particle_To_Subdomain() and
Local_Coordinate()
Map_To_Grid()  The macro Map To Grid(π, X∗d, Xd, d, xd, x′d),  used  solely  in oh4p_map_particle_to_
subdomain() D-times, examines the d-th dimensional coordinate X∗d of the position of
particle π currently accommodated by the local node and copy it to a local variable Xd. It
calculates d-th dimensional global coordinate xd of the grid-voxel in which π resides, and
its raw value x′d without taking care of periodic system boundary if any.
First, we examine  if Xd < ∆ld  · γd =  Grid[d].fcoord[0] or Xd ≥∆ud  · γd =
Grid[d].fcoord[1] to mean π has crossed a system boundary.  If so, we confirm the d-
th dimensional system boundary condition is periodic, i.e., BoundaryCondition[d][β] = 0
for corresponding β ∈{0, 1}, or make π eliminated and force oh4p_map_particle_to_
subdomain() to return its caller with −1. Then we let Xd and X∗d be Xd ± (∆ud −∆ld)γd,
and let bd = ∓(∆ud −∆ld) where ∆βd = Grid[d].coord[β] to regain the raw value of xd by
x′d = xd + bd after we calculate xd.
Next, we calculate xd = (Xd −∆ld · γd)/γd + ∆ld, where 1/γd = Grid[d].rgsize, and
make the corection on it as discussed in §4.10.48 so that xdγd ≤Xd < (xd + 1)γd, where
γd = Grid[d].gsize. Finally, we let x′d = xd + bd where bd = ∓(∆ud −∆ld) if π has crossed
a system boundary as discussed above, or bd = 0 otherwise.


#define Map_To_Grid(P, PXYZ, XYZ, DIM, GG, LG) {\
const double gsize = Grid[DIM].gsize;\
const double lb = Grid[DIM].fcoord[OH_LOWER];\
const double ub = Grid[DIM].fcoord[OH_UPPER];\
double gf;\
XYZ = PXYZ;\
LG = 0;\
if (XYZ<lb) {\
if (BoundaryCondition[DIM][OH_LOWER]) { P->nid = -1;  return(-1); }\


<!-- Page 433 -->

XYZ += (ub - lb);  PXYZ = XYZ;\
LG = Grid[DIM].coord[OH_LOWER] - Grid[DIM].coord[OH_UPPER];\
}\
else if (XYZ>=ub) {\
if (BoundaryCondition[DIM][OH_UPPER]) { P->nid = -1;  return(-1); }\
XYZ -= (ub - lb);  PXYZ = XYZ;\
LG = Grid[DIM].coord[OH_UPPER] - Grid[DIM].coord[OH_LOWER];\
}\
GG = (XYZ-lb)*Grid[DIM].rgsize + Grid[DIM].coord[OH_LOWER];\
gf = GG * gsize;\
if (gf>XYZ) GG--;\
else if (gf+gsize<=XYZ) GG++;\
LG += GG;\
}


Map_Particle_To_Subdomain()  The macro Map Particle To Subdomain(xd, d, πd), used solely in oh4p_map_particle_to_
subdomain() D-times if we have regular process coordinate, calculates d-th dimensional
process (subdomain) coordinate value πd in which a particle at d-th dimensional integer
coordinate xd resides as;
{
⌊(xd −∆ld)/δmind  ⌋           x < ∆−d                                              πd ←   −
Π d + ⌊(xd −∆−d )/(δmind  + 1)⌋  x ≥∆−d

referring to;

Grid[d].coord[0] = ∆ld
Grid[d].light.{size,thresh,n} = {δmind    , ∆−d , Πd− }

in a similar way we discussed in §4.7.19 but with integer coordinate and parameters. Since
this integer arithmetic is definately accurate, we don’t need any corrections which level-3
counterparts does with Adjust_Subdomain().


#define Map_Particle_To_Subdomain(XYZ, DIM, SDOM) {\
double thresh = Grid[DIM].light.thresh;\
if (XYZ<thresh)\
SDOM = (XYZ - Grid[DIM].coord[OH_LOWER]) / Grid[DIM].light.size;\
else\
SDOM = (XYZ - thresh)/ (Grid[DIM].light.size + 1) + Grid[DIM].light.n;\
}

Local_Coordinate()  The macro Local Coordinate(m, n′, xd, x′d, d, k, 3d, a), used solely in oh4p_map_particle_
to_subdomain() D-times, converts d-th dimensional global coordinate value xd and its raw
counterpart x′d into the correspoinding local coordinate in m and in local node’s own n′
respectively.  It also updates the neighbor index k by ±3d  if xd  is in d-th dimensional
exteriorof n′. The conversion is basically done by xd ←xd −δld(m) and x′d ←x′d −δld(n′)
where δld(m) = SubDomains[m][d][0], but we have to take care of the case m = n′ and xd ̸=
x′d. This surprising combination can happen when a particle has crossed a periodic system
boundary and m = n′  is the sole subdomain between exiting and entering boundaries,
i.e., both of d-th dimensional boundary planes of n′ are also system boundary planes.
Therefore if m = n′, we let x′d = xd for referring NOfPGrid[][][] by gidx(. . . , x′d, . . .) because
of the reason we discussed in §4.10.49.  Otherwise, i.e.,  if m ̸= n′ and thus the particle


<!-- Page 434 -->

is not in the interior of n′, we confirm −eg ≤x′d < δd(n′) + eg = SubDomains[n′][d][1] −
SubDomains[n′][d][0] + eg and decrement/increment k by 3d  if x′d < 0 or x′d ≥δd(n′)
respectively, or we let a = 1 to mean we have anywhere accommodation.


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


#### 4.10.51 oh4p_map_particle_to_subdomain()

oh4p_map_particle_to_subdomain_()  The API functions oh4p_map_particle_to_subdomain_() for Fortran and oh4p_map_
oh4p_map_particle_to_subdomain()  particle_to_subdomain() for C find the subdomain m in which the local node’s pri-
mary (ps = p = 0) or secondary particle of species s = s pointed by part = π resides,
and return m or −1 if particle went out-of-bounds. The function for C is also called from
oh4p_map_particle_to_neighbor() and oh4p_remap_particle_to_subdomain() to find
the subdomain for a particle warping and being remapped respectively.
They have a number of difrences from their level-3 couterparts oh3_map_particle_
to_subdomain[_](). First, they receive the S_particle structure pointer π rather than
their position coordinate values.  Second, they need species s to refer to an element of
NOfPLocal[][][], mantaining it by the functions rather than the simulator body to bring
another difference, and NOfPGrid[][][] for which we need calculations of integer coordinate
values and the grid-position of the particle. Third, they take care of the system periodic
boundary if π has been crossing it.
The function oh4p_map_particle_to_subdomain_() simply calls its counterpart oh4p_
map_particle_to_subdomain(), but decrementing s by 1 to make it zero-origined.


int
oh4p_map_particle_to_subdomain_(struct S_particle *part, const int *ps,
const int *s) {
return(oh4p_map_particle_to_subdomain(part, *ps, *s-1));
}
int
oh4p_map_particle_to_subdomain(struct S_particle *part, const int ps,
const int s) {
const int ns = nOfSpecies,  inj = part>=Particles+totalParts;
const int nx  = Grid[OH_DIM_X].n;
const int nxy = If_Dim(OH_DIM_Y, nx*Grid[OH_DIM_Y].n, 0);
const int t = ps ? ns + s : s;
int w, dw, mysd;
int sd;


<!-- Page 435 -->

double x, y, z;
int px, py, pz;
int gx, gy, gz;
int lx, ly, lz;
int k = OH_NBR_SELF,  aacc = 0;
Decl_Grid_Info();


First, we invoke Check_Particle_Location() to check the consistency of arguments,
before referring to elements in GridDesc[] and RegionId[]. The fifth argument of the macro
i is determined by whether π is beyond Particles+nOfInjections, i.e., π is for an injected
particle. Then, we invoke Map_To_Grid() D-times to calculate the d-th dimensional global
integer coordinate xd and x′d of the grid-voxel in which π resides with/without taking care
of system periodic boundary crossing respectively, for all d ∈[0, D) supressing invocations
for d ≥D by Do_Y() and/or Do_Z().
Then if SubDomainDesc ̸= NULL to mean we have irregular process coordinate, we call
map_irregular_subdomain() giving it floating point coordinates of π to obtain the sub-
domain identifier m in which π resides, and confirm m ≥0 or make π elimintated due to
out-of-bounds letting its nid and the function’s return value be −1.
Otherwise,  i.e.,  if we have regular process coordinate, we invoke Map_Particle_To_
Subdomain() D-times to have d-th dimensional process coordinate πd, from which we cal-
culate m in which π resides by Coord_To_Index() to which we also give Π0 and Π0 · Π1
where Πd = Grid[d].n.

Check_Particle_Location(part, ps, s, ns, inj);
w = GridDesc[ps].w;  dw = GridDesc[ps].dw;  mysd = RegionId[ps];
Map_To_Grid(part, part->x, x, OH_DIM_X, gx, lx);
Do_Y(Map_To_Grid(part, part->y, y, OH_DIM_Y, gy, ly));
Do_Z(Map_To_Grid(part, part->z, z, OH_DIM_Z, gz, lz));
if (SubDomainDesc) {
sd = map_irregular_subdomain(x, If_Dim(OH_DIM_Y ,y, 0),
If_Dim(OH_DIM_Z, z, 0));
if (sd<0) { part->nid = -1;  return(-1); }
} else {
Map_Particle_To_Subdomain(gx, OH_DIM_X, px);
Do_Y(Map_Particle_To_Subdomain(gy, OH_DIM_Y, py));
Do_Z(Map_Particle_To_Subdomain(gz, OH_DIM_Z, pz));
sd = Coord_To_Index(px, py, pz, nx, nxy);
}

Now we can convert global coordinate values xd and x′d to their counterparts local to
m and n′ = {n, parent(n)}[p] = RegionId[p] for the local node n by invoking Local_
Coordinate() D-times. By these invocations, we also know π is at somewhere outside of
the exterior of n′ and thus we have anywhere accommodation.  If so, we set the bit for
accommodation mode in currMode by Mode_Set_Any() and let nid of π be (m+3D)·2Γ +
gidx(x0, . . .).
Otherwise, we increment NOfPGrid[p][s][gidx(x′0, . . .)] using Coord_To_Index() again
but this time giving it δmax0  + 4eg = GridDesc[p].w and the product of it and (δmax1  +
4eg) being GridDesc[p].dw. Then, we let nid of π be k · 2Γ + gidx(x0, . . .) by Coord_To_
Index() and Combine_Subdom_Pos() where k is the neighbor index calculated by Local_
Coordinate().
Then if the partilce π is injected one, we increment InjectedParticles[0][p][s] if m = n′
because we need it maintained for non-position-aware particle transfer, and let the subdo-
