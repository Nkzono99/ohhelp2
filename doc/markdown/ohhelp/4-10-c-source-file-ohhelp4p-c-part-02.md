# 4.10 C Source File ohhelp4p.c - Part 2

Source: `doc/original/ohhelp.pdf`, pages 375-403.

<!-- Page 375 -->

MPI_LONG_LONG_INT, MPI_SUM, MyComm->rank, MyComm->prime);
}
if (MyComm->prime==MPI_COMM_NULL)
memcpy(NOfPGridTotal[0][0]+base, NOfPGrid[0][0]+base,
size[0]*sizeof(dint));
}


#### 4.10.19 make_recv_list()

make_recv_list()  The function make_recv_list(), called solely from exchange_particles4p(), scans per-
grid histogram to build primary receiving block, and then exchanges the block between
neighbors to have primary sending block and broadcast them for secondary receiving,
secondary sending and alternative secondary receiving blocks for helpers.  Its arguments
currmode, level, reb and stats are perfectly equivalent to those of the caller exchange_
particles4p(), while oldp and newp are parents in the last and next simulation step. The
function returns the pointer to the record next to the last block which it builds, or in other
words the head of hot-spot sending.


static struct S_commlist*
make_recv_list(const int currmode, const int level, const int reb,
const int oldp, const int newp, const int stats) {
const int me = myRank, ns=nOfSpecies, nn=nOfNodes, nnns=nn*ns;
const int nn2 = nn<<1;
struct S_node *nodes = reb ? NodesNext : Nodes;
struct S_node *mynode = nodes + me;
struct S_node *ch;
struct S_recvsched_context
context = {0, 0, 0, 0, 0, 0, 0, 0, CommList};
int rlsize, rlidx;
const int ft=nOfFields-1;
const int npgbase = FieldDesc[ft].bc.base;
const int *npgsize = FieldDesc[ft].bc.size;
const int lastg =
Coord_To_Index(GridDesc[0].x-1, GridDesc[0].y-1, GridDesc[0].z-1,
GridDesc[0].w, GridDesc[0].dw);
struct S_commlist *lastrl;
int i;


First, the function builds primary receiving block by calling sched_recv() for the local
node’s helpers and then the node itself to determine the grid-voxels to be accommodated
by them and to find hot-spots. The order of the node scannig, helpers first and then the
node itself, is to make the scanning orders of hot-spot senders and receivers coherent. That
is, as we will discuss in §4.10.26, since the function scatter_hspot_send() scans the hot-
spot senders in helper-first manner, the coherent order of receiver scanning will minimize
the amount of hot-spot particles transferred among the family members involved in the
hot-spot.
The arguments for the function, besides trivial currmode and reb, are as follows, where
NN is Nodes[] if reb is false, or NodesNext[] otherwise, for new helpands.
- get is Rgetn = NN [n].get.prime for the local node n, or Qgetm = NN [m].get.sec for
its helper m, to specify the baseline number of receiving (if positive) or sending (if
negative) primary/secondary particles of n or m.


<!-- Page 376 -->

- stay is Qnn = NN [n].stay.prime for the local node n, or Qnm = NN [m].stay.sec
for its helper m, to specify the number of primary/secondary particles currently ac-
commodated by n or m. The value is not useful for helpers if we have normal ac-
commodation and helpand-helper reconfiguration is not taking place, but get has the
base line number of particles to be accommodated by the helper in this case.

- nid is the node identifier of the local node n or its helper m.

- tag is 0 for the local node, or NS for its helpers, to distinguish helpand and helpers
and to be set into S_commlist record element tag.

- context is a S_recvsched_context structure discussed in §4.9.4, whose elements x,
y, z, g, hs, nptotal, nplimit, carryover are 0 at initial, while cptr is initialized to
point the head of CommList[].

Now we have ρ =  σ =  context.cptr −CommList  records  in primary  receiving
block, but  its last record does not necessary has the largest grid-position at gmax =
gidx(δx(n)−1, δy(n)−1, δz(n)−1) of the interior of the primary subdomain in its region
element, because the grid-voxel can be empty.  If so, we need to make the last record’s
region have gmax but we cannot simply do it by overwriting the record in two cases. One
extreme case is that the subdomain has no particles and thus the primary receiving block
is empty. The other is that the last record is for a hot-spot with which the last node cannot
have particles any more. In both cases, we add a record to assign all empty grid-voxels up
to gmax to the local node n for the former or the last node for the latter letting region
element be gmax.

for (ch=mynode->child; ch; ch=ch->sibling)
sched_recv(currmode, reb, ch->get.sec, ch->stay.sec, ch->id, nnns,
&context);
sched_recv(currmode, reb, mynode->get.prime, mynode->stay.prime, me, 0,
&context);
rlidx = rlsize = context.cptr - CommList;  lastrl = context.cptr - 1;
if (rlsize==0 || (lastrl->region<lastg && lastrl->count)) {
struct S_commlist *rl = lastrl + 1;
rl->rid = rlsize ? lastrl->rid : me;
rl->tag = 0;  rl->sid = 0;  rl->count = 0;  rl->region = lastg;
rlidx = ++rlsize;
} else
lastrl->region = lastg;

If we have anywhere accommodation, we have already transferred particles among (pos-
sibly) all nodes in the first non-position-aware phase. Therefore, the position-aware particle
transfer schedule we need is just for the transfer among the nodes in a helpand-helper fam-
ily. Thus the local node broadcasts the size of its primary receiving block ρ and then the
block itself by oh1_broadcast(), while these calls also let it receive its helpand’s primary
receiving block as its secondary receiving block to SecRList[] starting from CommList[ρ].
Finally we return to the caller with the pointer to SecRList[ρ′] where ρ′  is the size of
secondary receiving block given by the helpand by the first oh1_broadcast(), or ρ′ = 0 if
the local node does not have helpand, to let the next block be hot-spot sending block.

if (Mode_Acc(currmode)) {
SecRList = CommList + rlidx;  rlsize = 0;
oh1_broadcast(&rlidx, &rlsize, 1, 1, MPI_INT, MPI_INT);
oh1_broadcast(CommList, SecRList, rlidx, rlsize, T_Commlist, T_Commlist);


<!-- Page 377 -->

return(SecRList+rlsize);
}

Otherwise, i.e., we have normal accommodation, the local node n exchanges its primary
receiving block between its neighbors to have primary sending blocks. We scan all 3D
neighbors in DstNeighbors[k] and SrcNeighbors[k] to send/receive primary receiving/
primary sending block as follows.

- If DstNeighbors[k] = n, we skip the exchanging communication but let RLIndex[k]
be 0 to assume we received primary receiving block itself as the primary sending
block with which make_send_sched() builds sending schedule of the local node’s
own primary particles to helpers, and then have secondary receiving block as the
secondary sending block for sending secondary particles to the helpand and/or sibling
helpers. Note that we check neither SrcNeighbors[k] nor the non-first occurrence of
n in DstNeighbors[] or SrcNeighbors[] because it is assured that SrcNeighbors[k]
is n or −(n + 1) if DstNeighbors[k] is n or −(n + 1) respectively, by the symmetry
of neighboring. Also note that though we let RLIndex[k′] = RLIndex[k] = 0 for the
non-first occurrence of n at k′, the primary receiving block is processed by make_
send_sched() just once for k = ⌊3D/2⌋because it is assured that if a node itself is
its neighbor its corresponding exterior, i.e., sending plane/edge/vertex (set) does not
have any particles.

- If SrcNeighbors[k] = ms ≥0 to mean the first occurrence of ms, we receive some,
but at most 2N, records of ms’s primary receiving block into k-th primary sending
block from CommList[ρ] and let RLIndex[k] = ρ. We also send σ records in primary
receiving block to DstNeighbors[k] = md at the same time by MPI_Sendrecv() if
md ≥0, or only perform the reception by MPI_Recv(). The size of the received block
is obtained by MPI_Get_count() and ρ is incremented by the size.

- If SrcNeighbors[k] = ms < 0 to mean the second or subsequent occurrence of −(ms+
1), we let RLIndex[k] be that of k′ = FirstNeighbor[k] where SrcNeighbors[k′]
should have ms. By this operation, make_send_sched() can refer to the primary
sending block obtained from ms as the k-th neighbor, for which the exterior is different
from that for k′-th neighbor. In addition, if DstNeighbors[k] = md ≥0, we send σ
records in primary receiving block to md by MPI_Send().


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


<!-- Page 378 -->

if (dst>=0)
MPI_Send(CommList, rlsize, T_Commlist, dst, 0, MCW);
RLIndex[i] = (src<-nn) ? rlidx : RLIndex[FirstNeighbor[i]];
}
}

Now the local node has primary sending block with RLIndex[k] for all k ∈[0, 3D).
Then we let RLIndex[3D] = ρ so that it has the combined size of primary receiving and
primary sending blocks, or in other words the index of CommList[] of the head of next block,
secondary receiving or alternative secondary receiving block. We also let SecRList points
the next block head, as well as AltSecRList in case we don’t have secondary receiving
block.
Then if we were in secondary mode, the local node broadcast RLIndex[] and then primary
receiving and primary sending blocks to its helpers by oh1_broadcast(), which also let
the node have secondary receiving and secondary sending blocks, whose combined size is
SecRLIndex[3D], in SecRList[] and indices of them in SecRLIndex[]. We let ρ be the
combined size of all primary receiving, primary sending, secondary receiving and secondary
sending blocks and let AltSecRList points CommList[ρ]. Note that SecRLIndex[3D] remains
0 if the local node does not have helpand.
Then if helpand-helper reconfiguration is taking place, we call build_new_comm() to
build new communicators for the new primary/secondary families of the local node.  Its
second argument −level tells the function not to call make_comm_count() being un-
necessary for position-aware particle transfer, and the third argument nbridx = 2  is
to have the neighbors of the new helpand in Neighbors[2].  We also call the follow-
ings; update_descriptors() to update FieldDesc[] for the new secondary subdomain
which will be referred to by the oh1_broadcast() shortly; set_grid_descriptor() to
let GridDesc[2][] has the shape of per-grid histogram for the new secondary subdomain;
and update_real_neighbors() with the code URN_TRN to let RealDstNeighbors[0][][] and
RealSrcNeighbors[0][][] have neighoring information according to the new families while
RealDstNeighbors[1][][] and RealSrcNeighbors[1][][] have that according to transitional
state representing new/old helpers of old/new helpand’s neighbors. Then the local node
broadcasts the size of primary receiving block σ and then the block itself to its helpers by
oh1_broadcast(), which also let the node have alternative secondary receiving block and
its size (being 0 if it does not have helpand) to be added to ρ to know the next hot-spot
sending block starts from CommList[ρ].
Finally, the local node broadcasts per-grid histogram NOfPGridTotal[0][][] to (possibly)
new helpers referring to FieldDesc[F−1].bc.{base, size[]} by oh1_broadcast(), which
also lets the node have its helpand’s per-grid histogram in NOfPGridTotal[1][][], before
returning to the caller with the pointer to the head of hot-spot sending block namely
CommList[ρ].

RLIndex[OH_NEIGHBORS] = rlidx;  SecRLIndex[OH_NEIGHBORS] = 0;
AltSecRList = SecRList = CommList + rlidx;
if (Mode_PS(currmode)) {
oh1_broadcast(RLIndex, SecRLIndex, OH_NEIGHBORS+1, OH_NEIGHBORS+1,
MPI_INT, MPI_INT);
oh1_broadcast(CommList, SecRList, rlidx,
SecRLIndex[OH_NEIGHBORS], T_Commlist, T_Commlist);
AltSecRList = CommList + (rlidx += SecRLIndex[OH_NEIGHBORS]);
}
if (reb) {
int altrlsize = 0;


<!-- Page 379 -->

build_new_comm(currmode, -level, 2, stats);
update_descriptors(oldp, newp);
set_grid_descriptor(2, newp);
update_real_neighbors(URN_TRN, Mode_PS(currmode), oldp, newp);
oh1_broadcast(&rlsize, &altrlsize, 1, 1, MPI_INT, MPI_INT);
oh1_broadcast(CommList, AltSecRList, rlsize, altrlsize,
T_Commlist, T_Commlist);
rlidx += altrlsize;
}
oh1_broadcast(NOfPGridTotal[0][0]+npgbase, NOfPGridTotal[1][0]+npgbase,
npgsize[0], npgsize[1], MPI_LONG_LONG_INT, MPI_LONG_LONG_INT);
return(CommList+rlidx);
}


#### 4.10.20 sched_recv()

Prior to discuss the funcion sched_recv(), we show three macros, Sched_Recv_Check(),
Sched_Recv_Return() and For_All_Grid_From() used solely in the function.

Sched_Recv_Check()  The first macro Sched Recv Check(l, g) is the kernel of the iteration visiting the grid-voxel
at g in the loop scanning grid-voxels in sched_recv(), if l ̸= 0. Otherwise (l = 0), the
macro is used before the function enters the loop to cope with particles carried over from
the previous reciever node due to a hot-spot at g.
The macro refers to, besides its arguments, the following elements of S_recvsched_
context structure given to sched_recv() as an argument of the function, or their cached
version in local variables and with possible modification, for the call of sched_recv() with
the f-th (f ∈[0, |F(n)|) member mf of the local node n’s primary family where PT (g)
defined as follows.

S−1∑            S−1∑
PT (g) =   PT (0, s, g) =    NOfPGridTotal[0][s][g]
s=0             s=0
∑
- nptotal = PΣ(g) =   i≤g PT (i) is the number of particles in grid-voxels we have
already scanned including g itself.
∑f
- nplimit = PΛ(f) =   i=0 Qnmi is the sum of number of particles which the node mi
(i ∈[0, f]) is expected to accommodate.

- cptr is the pointer to a record of primary receiving block in CommList[] into which
the receiving schedule for mf is stored.

The macro examines if PΣ(g) < PΛ(f) to mean that mf may accommodate more particles
in grid-voxels beyond g.  If so, the macro does nothing to let the loop in sched_recv()
continue to visit the next grid-voxel. Otherwise, i.e., PΣ(g) ≥PΛ(f), it lets region element
of the S_commlist record pointed by ctpr be g, to mean that we assign grid-voxels up to g
to mf. Then it examines if PΣ(g)−PΛ(f) < 2Phot to mean the excess of PΣ(g) over PΛ(f)
is acceptable. If so, the macro lets the loop iterate once more but lets it stop before visiting
the next grid-voxel so that sched_recv() returns to its caller telling that the next call will
visit the next grid-voxel. In this case, we have assigned whole particles in the grid-voxel at
g to mf so that it will accommodate Qnf particles where

f−1∑
Qnf = PΣ(g) −   Qni = PΣ(g) −QΣ(f −1)
i=0


<!-- Page 380 -->

and thus
PΛ(f) ≤QΣ(f) = PΣ(g) < PΛ(f) + 2Phot

Otherwise, i.e., PΣ(g)−PΛ(f) ≥2Phot, we found a hot-spot. In this case, count element
of the S_commlist record is set to qhot(f) = ⌈(PΛ(f) −PΣ(g −1))/Phot⌉Phot to mean we
assign hot-spot particles of the smallest multiple of Phot to mf so that QΣ(f) ≥PΛ(f).
Note that since qhot(f) −Phot < PΛ(f) −PΣ(g −1) and PΣ(g) = PΣ(g −1) + PT (g), the
carryover amount qco(f + 1) to be set into carryover element of context

qco(f + 1) = PT (g) −qhot(f) > PΣ(g) −PΛ(f) −Phot ≥2Phot −Phot = Phot

is sufficiently large for the hot-spot spliting. Also note that since

qhot(f) < PΛ(f) −PΣ(g −1) + Phot

we have
QΣ(f) = PΣ(g −1) + qhot(f) < PΛ(f) + Phot

Then the macro invokes Sched_Recv_Return() to directly return from sched_recv() keep-
ing the coordinate and index of grid-voxel unchanged so that they corresponds to the hot-
spot we are now visiting.
Therefore, in both cases we have PΛ(f) ≤QΣ(f) < PΛ(f) + 2Phot and it is satisfied
that
PΛ(0) ≤QΣ(0) = Qn0 < PΛ(0) + 2Phot = Qnm0 + 2Phot

for f = 0, a simple induction leads us that

Qnf = QΣ(f) −QΣ(f −1) < PΛ(f) + 2Phot −PΛ(f −1) = Qnmf + 2Phot

to make it sure that the excess of the number of particles assigned to mf over that expected
is less than 2Phot.


#define Sched_Recv_Check(INLOOP, G) {\
if (nptotal>=nplimit) {\
cptr->region = G;\
if (nptotal-nplimit>ovflimit) {\
const int thresh = ovflimit>>1;\
const int count = (((nplimit-(nptotal-npt)-1)/thresh) + 1) * thresh;\
cptr->count = count;  carryover = npt - count;\
Sched_Recv_Return(INLOOP);\
} else\
ret = 1;\
}\
}


Sched_Recv_Return()  The macro Sched Recv Return(l) is to return from sched_recv(). The macro is invoked in
Sched_Recv_Check() used in the sched_recv()’s loop (l ̸= 0), or before the loop (l = 0).
The macro is also invoked directly by sched_recv() at its very end after the loop with
l = 1. What the macro does is write-back of cached local variables for S_recvsched_
context elements, nptotal, carryover and cptr. It also write the elements x, y, z and g
back to the structure if l ̸= 0 to mean those elements are modified by the loop.


<!-- Page 381 -->

#define Sched_Recv_Return(INLOOP) {\
if (INLOOP) {\
context->x = Grid_X();  context->y = Grid_Y();  context->z = Grid_Z();\
context->g = The_Grid();\
}\
context->nptotal = nptotal;  context->carryover = carryover;\
context->cptr = cptr + 1;\
return;\
}


For_All_Grid_From()  The macro For All Grid From(x0, y0, z0) is for a D-dimensional nested loop similar to
that constructed by For All Grid(0,0,0,0,0,0,0) for primary subdomain interior but
the starting grid-voxel is at (x0, y0, z0). The macro is expanded as shown below if D = 3
to iterate the loop body for (x0, y0, z0), (x0+1, y0, z0), . . . , (x1−1, y0, z0), (0, y0+1, z0), . . . ,
(x1−1, y0+1, z0), . . . , (x1−1, y1−1, z0), (0, 0, z0+1), . . . , (x1−1, y1−1, z1−1).

for(z = z0, x1 = δx(m) + x1,  y1 = δy(m) + y1, z1 = δz(m) + z1,
x′ = x0, y′ = y0, w = δmaxx  (m) + 4eg, d = δmaxy  (m) + 4eg,
gz = z0 · d · w, gy = gz + y0 · w, gx = gy + x0;
z < z1; z++, gz = gz + d · w, gy = gz, x′ = y′ = 0)
for(y = y′; y < y1; y++, gy = gy + w, gx = gy, x′ = 0)
for(x = x′; x < x1; x++, gx++)


#define For_All_Grid_From(X0, Y0, Z0)\
For_Z((fag_zidx=(Z0),\
fag_x1=GridDesc[0].x, fag_y1=GridDesc[0].y, fag_z1=GridDesc[0].z,\
fag_x0=(X0), fag_y0=(Y0),\
fag_w=GridDesc[0].w, fag_dw=GridDesc[0].dw,\
fag_gz=(Z0)*fag_dw, fag_gy=fag_gz+fag_y0*fag_w,\
fag_gx=fag_gy+fag_x0),\
(fag_zidx<fag_z1),\
(fag_zidx++, fag_gz+=fag_dw, fag_gx=fag_gy=fag_gz, fag_x0=fag_y0=0))\
For_Y((fag_yidx=fag_y0), (fag_yidx<fag_y1),\
(fag_yidx++, fag_gy+=fag_w, fag_gx=fag_gy, fag_x0=0))\
for (fag_xidx=fag_x0; fag_xidx<fag_x1; fag_xidx++,fag_gx++)



sched_recv()  The function sched_recv(), called solely from make_recv_list(), scans per-grid his-
togram to determine the set of grid-voxels to be hosted by a node nid = mf being the
f-th member of the local node’s primary family, whose expected number of accommodat-
ing primary (tag = 0) or secondary (tag = NS) particles is determined by the argumetns
currmode, reb, get and stay. The scanning and assignment context is kept in the S_
recvsched_context structure argument context whose elements and their definitions were
given in §4.10.19.


static void
sched_recv(const int currmode, const int reb, const int get, const int stay,
const int nid, const int tag, struct S_recvsched_context *context) {
const int x0=context->x, y0=context->y, z0=context->z, g=context->g;
const int ovflimit=gridOverflowLimit;


<!-- Page 382 -->

dint nptotal=context->nptotal;
dint nplimit=context->nplimit;
dint carryover=context->carryover;
dint **npg=NOfPGridTotal[0];
struct S_commlist *cptr=context->cptr;
const int ns=nOfSpecies;
int s, npt=carryover, ret=0;
Decl_For_All_Grid();
int fag_x0, fag_y0, fag_z0;

First, the function calculate PΛ(f) = PΛ(f −1) + Qnmf depending on the currmode,
reb and tag arguments which determine how Qnmf for the beginning of the next step is
calculated from the argument get, stay and NOfPrimaries[][][] as follows.

- If we have normal accommodation and helpand-helper reconfiguration is taking place,
get = Qgetmf has the expected number to be accommodated by the helper node mf,
i.e., Qnmf .
- Otherwise, get is calcluated based on stay which has Qnmf at the end of the last step.
Therefore, the next step’s Qnmf is the sum of get and stay. Note that stay is correctly
set by schedule_particle_exchange() in non-position-aware particle transfer if we
had anywhere accommodation, by rebalance1() for the local node n, or by count_
stay() if we had normal accommodation and helpand-helper configuration is stable.


if (!Mode_Acc(currmode) && reb && tag)
nplimit += get;
else
nplimit += get + stay;

Then after writing PΛ(f) back to context, we examine if PΣ(g′) −qco(f) ≥PΛ(f)
where g′ = g −1 if qco(f) = 0 or g′ = g otherwise to let the left-hand side of the inequality
mean the number of particles we have already assigned to nodes prceeding mf.  If this
ineqaulity holds to mean that we don’t have any particles to assign mf, we simply return
from this function without adding S_commlist record70.
Now we have some particles to assign to mf and thus set S_commlist record elements,
rid to mf, tag to that given as the argument, count to 0 to indicate the record is not for
hot-spot so far, and sid to the hot-spot ordinal kept in hs of context71. Then we invoke
Sched_Recv_Check() to assign particles carried over to mf  if any. That is, if qco(f) = 0,
we know PΣ(g −1) < PΛ(f) and thus the macro does nothing. Otherwise, some particles
are assigned to mf and it could make the direct return from the macro due to too heavy
population to assign particles carried over. If this consecutive carry-over occurs, the macro
assigns an amount of particles to mf knowing that we have already assigned PΣ(g)−qco(f)
to mi for i < f, because we initialize npt to have qco(f) in the declarative part.
Otherwise, i.e., all carry-over particles have assigned to mf, we let count element of
the S_commlist record be the number of them and its sid element be −1 to indicate the

70If the local node’s primary subdomain has no particles, this inequality holds at the first call of
sched recv() with g = 0 and f = 0, because PΣ(g −1) = qco(f) = PΛ(f) = 0.  Therefore, the caller
make recv list() of sched recv() will have no S commlist records in primary receiving block in this case
as discussed in §4.10.19.
71The element sid is meaningful only for hot-spot records but we let it have some specific value to avoid
to leave it undefined.


<!-- Page 383 -->

record is the terminal, and increment hs element in context for the next hot-spot. We also
have to take care that the possibility that assigning the carry-over particles to mf made
PΣ(g) ≥PΛ(f), i.e., it made mf unable to have more particles.  If not the case, we add
a new S_commlist record and set its rid, tag, count and sid as done above so that mf
has another (likely non-hot-spot) record, after letting the old record’s region be g because
Sched_Recv_Check() did not do that.

context->nplimit = nplimit;
if (nptotal-carryover>=nplimit)  return;
cptr->rid = nid;  cptr->tag = tag;  cptr->sid = context->hs;
cptr->count = 0;
Sched_Recv_Check(0, g);
if (carryover) {
cptr->count = carryover;  cptr->sid = -1;  context->hs++;
if (!ret)  {
cptr->region = g;  cptr++;
cptr->rid = nid;  cptr->tag = tag;  cptr->sid = context->hs;
cptr->count = 0;
}
}

Now we start scanning grid-voxels by For_All_Grid_From() for the subdomain interior
(i.e., without exterior).  In the loop, at first we check whether qco(f) > 0 and  if so we
skip one iteration so as to visit the grid-voxel next to the hot-spot without visiting it any
more. Then,  if Sched_Recv_Check() for the hot-spot or for the last iteration tells that
we have determined the number of particles to be assigned to mf, we return from the
∑S−1
function by Sched_Recv_Return(). Otherwise, we calculate PT (g) =   s=0 PT (0, s, g) =              ∑S−1
s=0 NOfPGridTotal[0][s][g], let PΣ(g) = PΣ(g −1) + PT (g), and then invoke Sched_
Recv_Check() to check the completion of the scanning.
Finally, we invoke Sched_Recv_Return() after we finish the last loop iteration for the
very last grid-voxel at g visiting which should have made PΣ(g) = PΛ(f).

For_All_Grid_From(x0, y0, z0) {
if (carryover) { carryover = 0;  continue; }
if (ret) Sched_Recv_Return(1);
for (s=0,npt=0; s<ns; s++)  npt += npg[s][The_Grid()];
nptotal += npt;
Sched_Recv_Check(1, The_Grid());
}
Sched_Recv_Return(1);
}


#### 4.10.21 make_send_sched()

make_send_sched()  The function make_send_sched(), called solely from exchange_particles4p(), scans pri-
mary receiving, primary sending, secondary receiving, secondary sending and alternative
secondary receiving blocks in CommList[] to determine the node to which the local node n
send the particles in each grid-voxel and processes all hot-spots after the scan. The func-
tion is given arguments currmode, helpand-helper reconfiguration indicator reb, the parent
status code pcode, the identifier of the old and (possibly) new helpand oldp and newp, the
pointer to the head of hot-spot sending block hslist, an array nacc[2] to accummulate the
number of primary ([0] = Qnn) or secondary ([1] = Qparent(n)n        ) particles to be accommodated


<!-- Page 384 -->

by the local node, and the pointer nsend to return the number of particles P nsend to be sent
from the local node.


static void
make_send_sched(const int currmode, const int reb, const int pcode,
const int oldp, const int newp, struct S_commlist *hslist,
int *nacc, int *nsend) {
const int psold = Parent_Old(pcode) ? 1 : 0;
const int ns2 = nOfSpecies<<1,  nn = nOfNodes;
int s, ps, n, h, maxhs=-1;
int nfrom, nto;
struct S_commlist *rlist[2] = {CommList,SecRList};
int *rlidx[2] = {RLIndex,SecRLIndex};


First, the function performs a few initializations to scan the CommList[] records; all ele-
ments of TotalPNext[2][S] are cleared, HotSpotTop is let point the head of HotSpotList[],
and it determines the set of neighbor indices whose sub-blocks in primary sending and sec-
ondary sending blocks are scanned, [0, 3D) if we have normal accommodation, or {⌊3D/2⌋}
otherwise because only in-family particle transfer will take place.

for (s=0; s<ns2; s++)  TotalPNext[s] = 0;
HotSpotTop = HotSpotList;
if (Mode_Acc(currmode)) {
nfrom = OH_NBR_SELF;  nto = nfrom + 1;
} else {
nfrom = 0;  nto = OH_NEIGHBORS;
}

Next, we scan primary sending block in CommList[] always and then secondary sending
block in SecRList[] if the local node has helpand in the last step, i.e., if Parent Old(pcode)
is true.  For each block, we scan the sub-block for k-th neighbor whose head index is
RLIndex[k′] for primary sending block or SecRLIndex[k′] for secondary sending block where
k′ = (3D −1) −k. Note that we have to reverse the neighboring index because RLIndex[k′]
and SecRLIndex[k′] are corresponds to SrcNeighbors[k′] and thus to Neighbors[p][(3D −
1) −k′] = Neighbors[p][k] for primary sending (p = 0) and secondary sending (p = 1)
blocks. Also note that we visit a neighbor twice or more if it occurred multiple times in
Neighbors[p][] to examine corresponding exterior, but just once the local node itself or its
old helpand with the index ⌊3D/2⌋for the interior of the primary/secondary subdomain
without its exterior because it is assured that corresponging exterior does not have any
particles.  Yet another remark is that we explicitly skip inexistent neighbors such that
Neighbors[p][k] < −(N + 1) because no records are in CommList[] for them while make_
send_sched_body() needs at least one proper record.
Before scanning each sub-block for neighbor k by make_send_sched_body(), we ini-
tialize HotSpot[p][k] so that it has a dummy S_hotspot record obtained from HotSpotTop
for the queue tail. Then, we call make_send_sched_body() to scan the sub-block for the
neighbor k and per-grid histogram with the following arguments.

- ps = p to indicate the block to be scanned is primary sending block for primary
subdomain’s neighbor (p = 0) or secondary sending block for secondary subdomain’s.

- n = k being the neighbor index.


<!-- Page 385 -->

- sdid = Neighbors[p][k] or −(Neighbors[p][k] + 1) being the subdomain identifier of
the sub-block.

- self is true iffk = ⌊3D/2⌋and either p = 0 or the local node has same helpand in
the last and next step, i.e., Parent New Same(pcode) is true. This means that make_
send_sched_body() scans primary receiving or secondary receiving block in which
the local node may appear as a receiver.

- sender is true to indicate that make_send_sched_body() scans primary sending or
secondary sending block instead of alternative secondary receiving block.

- rlist pointing the first record of the sub-block to be scanned.

- maxhs is the pointer to the local variable of the same name to keep the greatest ordinal
hmax of the hot-spot encountered in the scan.

- naccptr pointing to nacc[p], i.e., Qnn or Qparent(n)n           .

- nsendptr is nsend argument to point P nsend .


for (ps=0; ps<=psold; ps++) {
const int root = ps ? oldp : myRank;
for (n=nfrom; n<nto; n++) {
int nrev = OH_NEIGHBORS - 1 - n;
int sdid = Neighbors[ps][n];
const int self = n==OH_NBR_SELF && (ps==0 || Parent_New_Same(pcode));
struct S_hotspot *hs = HotSpotTop++;
hs->comm = NULL;  hs->next = NULL;  hs->g = 0;  hs->lev = INT_MAX;
HotSpot[ps][n].head = HotSpot[ps][n].tail = hs;
if (sdid<0)  sdid = -(sdid+1);
if (sdid<nn && (sdid!=root || n==OH_NBR_SELF))
make_send_sched_body(ps, n, sdid, self, 1, rlist[ps]+rlidx[ps][nrev],
&maxhs, nacc+ps, nsend);
}
}

If we have normal accommodation and helpand-helper reconfiguration is taking place
to change the local node’s helpand, i.e., Parent New Diff(pcode) is true, we perform one
more scan for alternative secondary receiving block by make_send_sched_body(), after
initializing HotSpot[2][⌊3D/2⌋]. The arguments for this call diffeent from the previous ones
are as follows.

- ps = 2 to indicate the block to be scanned is alternative secondary receiving block.

- n = ⌊3D/2⌋being the neighbor index for the secondary subdomain.

- sdid is the identifier of the newly assigned secondary subdomain.

- self is true to mean the local node may appear in alternative secondary receiving
block as a receiver.

- sender is false to indicate that make_send_sched_body() scans alternative secondary
receiving block for receiving particles rather than sending.

- rlist = AltSecRList pointing the head of alternative secondary receiving block.


<!-- Page 386 -->

- naccptr pointing to nacc[1] to count secondary particles Qparent(n)n           .


if (!Mode_Acc(currmode) && Parent_New_Diff(pcode)) {
struct S_hotspot *hs = HotSpotTop++;
hs->comm = NULL;  hs->next = NULL;  hs->g = 0;  hs->lev = INT_MAX;
HotSpot[2][OH_NBR_SELF].head = HotSpot[2][OH_NBR_SELF].tail = hs;
make_send_sched_body(2, OH_NBR_SELF, newp, 1, 0, AltSecRList, &maxhs,
nacc+1, nsend);
}

Finally, we process hot-spots we encountered in the scan, i.e., those having ordinals not
greater than hmax. For deadlock-free gather/scatter of hot-spot information, we process hot-
spots according to their ordinal from 0 to hmax with implicit synchronization. That is, we
call gather_hspot_recv() and scatter_hspot_send() to gather/scatter information for
the h-th hot-spot in the local node’s primary subdomain, and gather_hspot_send() and
scatter_hspot_recv() to do that for hot-spots having the ordinal h in other subdomains
if any, in a interleaving manner for each ordinal. Therefore, when the local node processes
the h-th hot-spot in its primary subdomain, it is assured that other nodes involved in the
hot-spot should respond the hot-spot processing in question.
For each ordinal h ∈[0, hmax], at first we call gather_hspot_recv() to initiate gather
operation and to obatain the number of MPI_Irecv() requests Rr made in the function,
if the head of the hot-spot queue for the primary subdomain HotSpot[0][⌊3D/2⌋] has the
ordinal h, passing it currmode and reb argument and the S_hotspot structure at the
queue head.  Then we call gather_hspot_send() to respond gather_hspot_recv() in
other nodes with h, the parent status code pcode, Rr to know the first available entry in
Requests[], the set of neighbor indices to scan, the pointer to hslist argument to let it
have the next available hot-spot sending block record, and the pointer to the variable to
have the number of MPI_Irecv() requests Rs made in the function. Then  if we called
gather_hspot_recv(), i.e., if the primary subdomain has the hot-spot of the ordinal h, we
call scatter_hspot_send() with Rr, nacc argument, and the pointer to hslist argument.
Finally, we call scatter_hspot_recv() with h, pcode, Rr, Rs, the neighbor index range,
and nacc and nsend arguments, to process the response from other nodes’ scatter_hspot_
send() if any.

for (h=0; h<=maxhs; h++) {
int rreq=0, sreq;
struct S_hotspot *hs = HotSpot[0][OH_NBR_SELF].head;
const int self = hs->lev==h;
if (self)  rreq = gather_hspot_recv(currmode, reb, hs);
gather_hspot_send(h, pcode, rreq, nfrom, nto, &hslist, &sreq);
if (self)  scatter_hspot_send(rreq, nacc, &hslist);
scatter_hspot_recv(h, pcode, rreq, sreq, nfrom, nto, nacc, nsend);
}
}


#### 4.10.22 make_send_sched_body()

Grid_Boundary()  Prior to discussing the funcion make_send_sched_body(), we show a macro Grid Boundary
(νd, δd(n′), m, d, xld, xud, ∆d) used solely by the function. The macro gives the d-th dimen-
sional lower (xld) and upper (xud) bound of an exterior or interior of the local node n’s
primary/secondary subdomain whose d-th dimensional size  is δd(n′) where n′ = n or


<!-- Page 387 -->

n′ = parent(n), for the neighbor m whose d-th dimensional process coordinate relative
to the subdomain is νd −1 ∈{−1, 0, 1}. Note that the upper bound xud is relative to the
subdomain’s upper bound δd(n′). The macro also gives the d-th dimensional offset ∆d from
a grid-voxel in the local node’s subdomain to that in m’s subdomain.
The lower and upper bounds xld and xlu + δd(n′) are given as follows.

 (−eg, 0)             νd −1 = −1
(xld, xud + δd(n′)) =   (0, δd(n′))           νd −1 = 0                               
(δd(n′), δd(n′) + eg)  νd −1 = 1

On the other hand, d-th dimensional coordinate value 0 for the subdomain n′ corresponds to
δd(m) for the lower d-th dimensional neighbor m, and δd(n′) for n′ to 0 for upper neighbor
m. Therefore, the offset (difference) ∆d from the grid-voxel at x(n′) for n′ to that x(m)
for m is defined as follows to give us x(m) = x(n′) + ∆d , where δd(m) = δud(m) −δld(m) =
SubDomains[m][d][1] −SubDomains[m][d][0].

 δd(m)    νd −1 = −1
∆d =   0        νd −1 = 0

−δd(n′)  νd −1 = 1


#define Grid_Boundary(N, GS, SD, DIM, PL, PU, OFF) {\
const int e = OH_PGRID_EXT;\
const int *b = SubDomains[SD][DIM];\
const int off = If_Dim(DIM, b[OH_UPPER]-b[OH_LOWER], 0);\
if (N==0)      { PL = -e;    PU = -(GS);  OFF = off; }\
else if (N==1) { PL = 0;     PU = 0;      OFF = 0; }\
else           { PL = (GS);  PU = e;      OFF = -(GS); }\
}


make_send_sched_body()  The function make_send_sched_body(), called solely from make_send_sched() but up
to 2 · 3D + 1 times, scans a sub-block in primary sending or secondary sending block of
CommList[] and per-grid histogram of local node n’s primary or secondary subdomain, to
find the node in which particles in each grid-voxel are accommodated and to enqueue the
grid-voxel into hot-spot queue. The arguements given to the function were discussed in
§4.10.21.
At first, we determine the exterior or interior of the local node’s subdomain to be
∑D−1
scanned, S = [xl, xu) × [yl, yu) × [zl, zu) for the neighbor whose index k =   d=0 νd3d is
given by the argument n invoking macro Grid_Boundary() for each dimension d ∈[0, D).
The macro also gives us the d-th dimensional offset ∆d from a grid-voxel g in the local
node’s subdomain to that in the neighbor subdomain g′, from which we calculate the offset
of grid-voxel index ∆= gidx(∆x, ∆y, ∆z) by Coord_To_Index() to have g′ = g + ∆.


static void
make_send_sched_body(const int psor2, const int n, const int sdid,
const int self, const int sender,
struct S_commlist *rlist, int *maxhs, int *naccptr,
int *nsendptr) {
const int me=myRank, nn=nOfNodes, ns=nOfSpecies;
const int ps = psor2==2 ? 1 : psor2;
const int nsor0 = ps ? ns : 0;


<!-- Page 388 -->

const int nx = n % 3, ny = n/3 % 3, nz = n/9;
int xl, xu, yl, yu, zl, zu, xoff, yoff, zoff, ngoff;
int rlg = rlist->region;
int nacc = *naccptr, nsend = *nsendptr;
Decl_For_All_Grid();

Grid_Boundary(nx, GridDesc[psor2].x, sdid, OH_DIM_X, xl, xu, xoff);
Grid_Boundary(ny, GridDesc[psor2].y, sdid, OH_DIM_Y, yl, yu, yoff);
Grid_Boundary(nz, GridDesc[psor2].z, sdid, OH_DIM_Z, zl, zu, zoff);
ngoff = Coord_To_Index(xoff, yoff, zoff, GridDesc[psor2].w,
GridDesc[psor2].dw);


Now we scan each grid-voxel at g in S by For_All_Grid(). At first we skip S_commlist
records until we find the record having g(r) in its region element where r is its rid element
such that g(r) ≥g′ = g + ∆to mean the particles in the grid-voxel will be accommodated
by the node r. Then if g(r) = g and the count element of the record qhot(r) > 0 to mean
the grid-voxel is a hot-spot and the local node is involved in it, we enqueue the S_hotspot
record for it at the tail of HotSpot[p][k] where p is the psor2 argument, obtaining a new
dummy record from HotSpotTop to copy the tail record into it. Then we let the elemets of
the old tail record be as follows.

- g = g to remember the grid-voxel.

- n be the number of the series of hot-spot records in the CommList[] for g′ and thus
the number of receivers of the hot-spot particles.

- lev be the hot-spot ordinal h recorded in sid in the S_commlist record. In addition,
if h > hmax where hmax is pointed by maxhs argument, we let hmax = h.

- self be true iffthe local node appears as a receiver in the series of hot-spot records
and the self argument is true to mean we are scanning the interior of primary or
secondary subdomain of the local node. Note that the local node can appear as a
receiver of a hot-spot in its exterior with transitional helpand-helper reconfiguration.
We have to distinguish the occurrences in interior and exterior because the former is
to receive particles from the latter. In addition, if we are scanning interior but the
local nodes does not appear in the hot-spot records as a receiver, we let PO(p′, s, g) =
NOfPGridOut[p′][s][g] = 0 where p′ = 1 if p = 2 or p′ = p otherwise, for all species
s ∈[0, S) to mean that the local node will not have any particles in the grid-voxel.

- next be the pointer to the newly acquired tail record.

- comm be the pointer to the S_commlist record for the hot-spot.  This element is
meaningful only for hot-spots in the interior of the local node’s primary subdomain
and is referred to by scatter_hspot_send(). Then afterward, this element will be
let point to a S_commlist record in hot-spot sending block by gather_hspot_send_
body() or scatter_hspot_send().

Note that we skip all hot-spot records and visit the record following the tail hot-spot record
having sid = −1 for the next grid-voxel.

For_All_Grid(psor2, xl, yl, zl, xu, yu, zu) {
const int g = The_Grid();
const int ng = g + ngoff;


<!-- Page 389 -->

while (rlg<ng)  rlg = (++rlist)->region;
if (rlg==ng && rlist->count) {
struct S_hotspot *hs = HotSpot[psor2][n].tail;
struct S_hotspot *hst = HotSpot[psor2][n].tail = HotSpotTop++;
struct S_commlist *rl = rlist;
int involved = rlist->rid==me, lev, s;
*hst = *hs;
hs->g = g;  hs->next = hst;  lev = hs->lev = rlist->sid;
hs->comm = rlist;
for (rlist++; rlist->sid>=0; rlist++)
involved = involved || rlist->rid==me;
involved = involved || rlist->rid==me;  rlist++;
/* involved = involved || (rlist++)->rid==me
doesn’t work if involved has been true */
hs->n = rlist - rl;  rlg = rlist->region;
hs->self = self && involved;
if (self && !involved)
for (s=0; s<ns; s++)  NOfPGridOut[ps][s][g] = 0;
if (lev>*maxhs)  *maxhs = lev;

Otherwise,  i.e., the S_commlist record is not for a hot-spot, we examine  if its rid
element r is equal to n.  If so and we are scanning the interior of n’s primary/secondary
subdomain, i.e., self argument is true, we let PO(p′, s, g) = PT (p′, s, g), or in other words
NOfPGridOut[p′][s][g] = NOfPGridTotal[p′][s][g], and add PT (p′, s, g) to Qnn or Qparent(n)n       72
and to TotalPNext[p′][s], for each species s ∈[0, S) because the local node will host the
grid-voxel as a whole. The reason why we must check self has been discussed above for the
hot-spot case. In addition, if we are scanning primary sending or secondary sending block,
i.e., sender argument is true, we let NOfPGrid[p′][s][g] = 0 changing its role to mean no
particles in the grid-voxel will be sent to other nodes. Note that if sender is false to mean
we are scanning alternative secondary receiving block, we cannot do it because we have
already visited the grid-voxel in the scan of secondary receiving block being a sub-block of
secondary sending one to send all particles in it to other nodes in the new family of the old
helpand of the local node.
If we are scanning interior but r ̸= n, on the other hand, we let NOfPGridOut[p′][s][g] = 0
because the grid-voxel will be empty. On the other hand further, if r ̸= n or we are scanning
exterior, and sender is true as discussed above, we add NOfPGrid[p′][s][g] to P nsend 73 and
NOfSend[p′′][s][r], where p′′ = 0 if the tag element t of the record is 0 to mean the particles
are primary for r or p′′ = 1 if t = NS to mean secondary, for each s ∈[0, S) because
all particles will be sent to r. Note that the one-dimensional index of NOfSend[p′′][s][r] is
obtained by t + sN + r, and thus we also let NOfPGrid[p′][s][g] = t + sN + r + 1 changing
its role so that we can revisit NOfSend[p′′][s][r] easily when we find a primary (p′ = 0) or
secondary (p′ = 1) particle of species s in the grid-voxel g in move_to_sendbuf_sec4p()
or move_and_sort_secondary().

} else {
const int rid = rlist->rid;
int s;
if (rid==me && self) {
for (s=0; s<ns; s++) {
int naccinc = NOfPGridOut[ps][s][g] = NOfPGridTotal[ps][s][g];

72Not directly to one of them but to a local variable nacc caching it pointed by naccptr argument.
73Not directly to it but to a local variable nsend caching it pointed by nsendptr argument.


<!-- Page 390 -->

nacc += naccinc;  TotalPNext[nsor0+s] += naccinc;
if (sender)  NOfPGrid[ps][s][g] = 0;
}
} else {
if (self)
for (s=0; s<ns; s++)  NOfPGridOut[ps][s][g] = 0;
if (sender) {
int nofsidx = rlist->tag + rid;               /* [ps][0][rid] */
for (s=0; s<ns; s++,nofsidx+=nn) {
int nsendinc = NOfPGrid[ps][s][g];
nsend += nsendinc;  NOfSend[nofsidx] += nsendinc;
NOfPGrid[ps][s][g] = nofsidx + 1;
}
}
}
}
}

Finally we return to the caller after writing the cached Qnn or Qparent(n)n      and P nsend back to
the originals in the grand-caller exchange_particles4p() through the pointer arguments
naccptr and nsendptr.

*naccptr = nacc; *nsendptr = nsend;
}


#### 4.10.23 gather_hspot_recv()

Is_Boundary()  Prior to discuss the funcion gather_hspot_recv(), we show a macro Is Boundary(xd,
δd(n)) to examine  if grid-voxel in the interior of the local node n’s primary subdomian
having d-th dimensinal coordinate xd is in a d-th dimensional boundary plane of eg thick.
That is, the macro is expanded to the following value bd.
{                                        −1  xd < eg
bd =  0    eg ≤xd < δd(n) −eg
1    δd(n) −eg ≤xd

Note that we don’t check if the grid-voxel in the interior because we know it is definitely
so.


#define Is_Boundary(P, B)  (P<OH_PGRID_EXT ? -1 :\
(P>=B-OH_PGRID_EXT ? 1 : 0))


gather_hspot_recv()  The function gather_hspot_recv(), called solely from make_send_sched() but as many
times as the number of hot-spots in the local node’s primary subdomain, initiates the
gather reception by MPI_Irecv() for a hot-spot h pointed by the argument hs at the head
of the queue HotSpot[0][⌊3D/2⌋] for the local node n’s primary subdomain. The function
posts MPI_Irecv() for all nodes which can have the hot-spot in their primary or secondary
subdomains. More specifically, the target nodes are n’s helpers and, if the hot-spot is in a
boundary plane of its primary subdomain, the neighbors sharing the plane as the exterior
of their primary subdomain and their helpers.


<!-- Page 391 -->

static int
gather_hspot_recv(const int currmode, const int reb,
const struct S_hotspot *hs) {
const int me=myRank, ns=nOfSpecies, nn=nOfNodes;
const int g = hs->g,  psold = Mode_PS(currmode) || Mode_Acc(currmode);
int rreq=0, nbx, nby, nbz, nx, ny, nz;
const struct S_node *nodes = reb ? NodesNext : Nodes;
MPI_Request *reqs=Requests;


At first we find neighbors involved in the hot-spot h at g. If we have normal accommo-
dation, we calculate (x, y, z) = gidx −1(g) by Index_To_Coord() and give each coordinate
value to Is_Boundary() to have βd ∈{−1, 0, 1} for each d ∈[0, D), which means a neighbor
∑D−1
having index k =   d=0 νd3d is involved iffνd −1 = βd or νd −1 = 0 for all d ∈[0, D).
Therefore, we have 23 = 8 neighbors if h is at a vertex, 22 = 4 if on an edge, 21 = 2 if in a
plane, or 20 = 1 if in other inside region, including n itself.
Otherwise, i.e., we have anywhere accommodation, the nodes involved in the hot-spot
are only the family member of the local node and thus we make βd = 0 for all d ∈[0, D).

if (Mode_Acc(currmode))
nbx = nby = nbz = 0;
else {
int x, y, z;
Index_To_Coord(g, x, y, z, GridDesc[0].w, GridDesc[0].dw);
nbx = Is_Boundary(x, GridDesc[0].x);
nby = If_Dim(OH_DIM_Y, Is_Boundary(y, GridDesc[0].y), 0);
nbz = If_Dim(OH_DIM_Z, Is_Boundary(z, GridDesc[0].z), 0);
}

Then we scan each neighbor m = DstNeighbors[k] or m = −(DstNeighbors[k] + 1)
whose index k matches the criteria above but excluding n itself to post MPI_Irecv() for
receiving the number of particles of all S species in h accommodated by the neighbor into
HSRecv[k][m][]. Note that we don’t exclude the second or subsequent occurence of a node m,
but their report should be recieved individually in HSRecv[][][]. Though it looks funny that
the node m has multiple hot-spots correspongind to a particular hot-spot in the local node’s
primary subdomain, it may occur because these hot-spots are at different grid-positions in
the exterior of m, e.g., in west and east sending planes if m is east and west neighbor of the
local node at the same time. Therefore, we gives k to MPI_Irecv() as its tag argument for
distinguishment.
On the other hand, if k = ⌊3D/2⌋, we copy NOfPGrid[0][s][g] into HSRecv[3D/2][n][s] for
each s ∈[0, S) to let n receive what it would receive if m were not n.
Then for each neighbor m above but including the local node n itself with k = ⌊3D/2⌋, we
scan its helpers in H(m) listed in NN [m].child to post MPI_Irecv() for each m′ ∈H(m)
to receive its report into HSRecv[k][m][] again, if in the last step we were in secondary mode
and thus nodes have old helpers or we had anywhere accommodation and thus nodes have
new helpers which are made possible to have hot-spot by the first-phase non-position-aware
particle transfer.
Note that we refer to NodesNext[] if helpand-helper reconfiguration is taking place in-
dicated by reb ̸= 0, or Nodes[] otherwise, for the scanning helpers because the hot-spot is
hosted by old helpers. Also note that we scan helpers of a neighbor m multiple times if
m appears multiple times, but helpers of the local node n itself is scanned only once with


<!-- Page 392 -->

the neighbor index k = ⌊3D/2⌋because they cannot have hot-spots in the exterior of their
secondary subdomains. In addition, we gives k to MPI_Irecv() as its tag argument again
to receive the particle amount from a helper of a neighbor which appear multiple times.
Since a node m may appear as a neighbor and also as a helper of other neighbor, giving a
tag k for a neighbor and its helpers looks to cause some confusion. However, the node m
cannot be the k-th neighbor and a helper of the k-th neighbor at the same time, using tag
k is sufficient to make the pair (k, m) and thus the receiving buffer HSRecv[k][m][] unique
in the calls of MPI_Irecv().
Finally, we return to the caller reporting the number of consumed entries in Requests[],
being 2D−1N at most, as the return value.

for (nz=-1; nz<2; nz++) {
if (nz && nz!=nbz)  continue;
for (ny=-1; ny<2; ny++) {
if (ny && ny!=nby)  continue;
for (nx=-1; nx<2; nx++) {
const int nbr = (nx+1)+3*((ny+1)+3*(nz+1));
int nid = DstNeighbors[nbr];
struct S_node *ch;
if (nx && nx!=nbx)  continue;
if (nid<0)  nid = -(nid+1);
if (nid>=nn)  continue;
if (nid!=me)
MPI_Irecv(HSRecv[nbr]+nid*ns, ns, MPI_INT, nid, nbr, MCW,
reqs+rreq++);
if (nbr==OH_NBR_SELF) {
int s,  *hsr = HSRecv[OH_NBR_SELF] + me*ns;
for (s=0; s<ns; s++)  hsr[s] = NOfPGrid[0][s][g];
}
if (psold && (nid!=me || nbr==OH_NBR_SELF)) {
for (ch=nodes[nid].child; ch; ch=ch->sibling) {
const int chid = ch->id;
MPI_Irecv(HSRecv[nbr]+chid*ns, ns, MPI_INT, chid, nbr, MCW,
reqs+rreq++);
}
}
}
}
}
return(rreq);
}


#### 4.10.24 gather_hspot_send()

gather_hspot_send()  The function gather_hspot_send(), called solely from make_send_sched() but as many
times as the maximum ordinal of hot-spots the local node is involved in, scans all hot-
spot queued in HotSpot[][] having the ordinal h given as the argument hxidx. The other
arguments it receives are pcode for the parent status, rreq = Rr being the number of
MPI_Irecv() posted in gather_hspot_recv() and meaning that Requests[Rr] is the first
available entry for the use in this function, nfrom and nto to specify the set of neighbor
indices [0, 3D) or {⌊3D/2⌋} to be scanned, hslist is the head of hot-spot sending block in
which hot-spot sending schedules are built, and sreqptr pointing Rs being the number of
MPI_Irecv() posted in this function.


<!-- Page 393 -->

In this function we call gather_hspot_send_body() for each neighbor Neighbors[p][k],
where index k is in the set specified, of the primary subdomain (p = 0) always and secondary
subdomain (p = 1) if exists, i.e., Parent_Old() of pcode is true, to send the number of
particles for all species in the hot-spot if any and to initiate the reception of the sending
schedule in return to it. The function may also initiate another reception to know the
number of particles which the local node will accommodate when the neighbor is its helpand.
We also call the function once more if helpand-helper reconfiguration is taking place to gives
the local node a new secondary subdomain different from the old one, i.e., Parent_New_
Diff() of pcode is true, only for the reception of the number of accommodating particles.
The arguments given to gather_hspot_send_body(), other than the arugments of this
functin itself, are as follows; ps = p ∈[0, 2] to refer to HotSpot[p][], n and dst being the
index and identifier of the neighbor, and sender is false for the new helpand and true for
others.


static void
gather_hspot_send(const int hsidx, const int pcode, const int rreq,
const int nfrom, const int nto, struct S_commlist **hslist,
int *sreqptr) {
const int psold=Parent_Old(pcode) ? 1 : 0;
MPI_Request *reqs = Requests + rreq;
int ps, n;

*sreqptr = 0;
for (ps=0; ps<=psold; ps++) {
for (n=nfrom; n<nto; n++)
gather_hspot_send_body(hsidx, ps, n, Neighbors[ps][n], 1, hslist, reqs,
sreqptr);
}
if (Parent_New_Diff(pcode))
gather_hspot_send_body(hsidx, 2, OH_NBR_SELF, RegionId[1], 0, hslist, reqs,
sreqptr);
}


#### 4.10.25 gather_hspot_send_body()

gather_hspot_send_body()  The function gather_hspot_send_body(), called solely from gather_hspot_send() but
up to 2 · 3D + 1 times, examines the head record H of the hot-spot queue HotSpot[p][k]
has the hot-spot whose ordinal is h, where p = psor2, k = n and h = hsidx given by the
arguments. Then if the ordinals match, it processes the hot-spot with other arguments; dst
is the subdomain identifier m or −(m+1) of the neighbor k; sender is true iffthe local node
may send the particles in the hot-spot to m or its helpers; hslist is the double pointer to
CommList[c] from which the sending schedule of the hot-spot is bulit; and sreqptr pointing
the variable Rs in which we accumulate the number of MPI_Irecv() in gather_hspot_
send().
At first we examine  if h is equal to H.lev, and return to the caller without doing
anything if not, because it is not the turn for H or the queue is empty and thus H has
INT_MAX ordinal. We also examine if the neighbor subdomain is the local node’s primary
one, i.e., p = 0 and k = ⌊3D/2⌋, and return to the caller again if so. Note that the primary
subdomain may appear as a neighbor subdomain but the queue for it should always be
empty. Also note that the queue should always be empty too for inexsistent neighbors and
thus we don’t check if m = −(N + 1).


<!-- Page 394 -->

Then if H.self is true to mean m is the local node n’s helpand and n is a receiver of
the hot-spot particles, we post MPI_Irecv() to receive the amounts of hot-spot particles
to accommodate into HSRecvFromParent[s] for all species s ∈[0, S) from the helpand m.
Note that we give a tag 2 · 3D to MPI_Irecv() to distinguish it from those in gather_
hspot_recv() less than 3D and from that in this function in [3D, 2·3D) to receive hot-spot
sending schedules from a neighbor or that of the helpand.
Then, after initializing H.comm to be NULL, we send the amount of particles in the
hot-spot to m  if sender is true.  That  is, we copy PL(p′, s, g) = NOfPGrid[p′][s][g] to
HSSend[s], where p′ = 1 if p = 2 or p′ = p otherwise and g = H.g being the grid-position
of the hot-spot, for each s ∈[0, S), and send the HSSend[] to m by MPI_Send() with a tag
k′ = 3D −1 −k so that m’s MPI_Irecv() can distinguish each of multiple occurrence of n
in m’s neighbors or their helpers. Note that the k-th neighbor m should have the neighbor
index (3D −1 −k) for n or its helpand.
Then  if the hot-spot has one or more particles, we post MPI_Irecv() to receive ρS
records of T_Commlist for hot-spot sending schedule into CommList[c] and its successors
from m, where ρ is the number of receivers of the hot-spot recorded in H.n. The tag for
this MPI_Irecv() is 3D + k′ to correspond to k′ for the MPI_Send() but also to distinguish
it from MPI_Irecv() from m posted by the local node in gather_hspot_recv(). Then the
H.comm is let to point CommList[c] and (conceptually) c is incremented by ρS for the next
available S_commlist record. Note that the number of receiving records can be smaller
than ρS but we simply waste those unused records.
Finally, we return to the caller, after letting hslist arguments have the double pointer
to CommList[c] with (possibly) updated c, and reporting the caller the value of Rs which
has been incremented by 0, 1 or 2, through sreqptr argument.


static void
gather_hspot_send_body(const int hsidx, const int psor2, const int n, int dst,
const int sender, struct S_commlist **hslist,
MPI_Request *reqs, int *sreqptr) {
struct S_hotspot *hs = HotSpot[psor2][n].head;
const int ns = nOfSpecies,  g = hs->g,  nrec = hs->n * ns;
const int ps = psor2==2 ? 1 : psor2;
const int nrev = OH_NEIGHBORS - 1 - n;
struct S_commlist *hsl = *hslist;
int sreq = *sreqptr;
int np, s;

if (hs->lev!=hsidx || (ps==0 && n==OH_NBR_SELF))  return;
if (dst<0)  dst = -(dst+1);
if (hs->self)
MPI_Irecv(HSRecvFromParent, ns, MPI_INT, dst, OH_NEIGHBORS<<1, MCW,
reqs+sreq++);
hs->comm = NULL;
if (sender) {
for (s=0,np=0; s<ns; s++)  np += (HSSend[s] = NOfPGrid[ps][s][g]);
MPI_Send(HSSend, ns, MPI_INT, dst, nrev, MCW);
if (np) {
MPI_Irecv(hsl, nrec, T_Commlist, dst, OH_NEIGHBORS+nrev, MCW,
reqs+sreq++);
hs->comm = hsl;  hsl += nrec;
}
}


<!-- Page 395 -->

*hslist = hsl;  *sreqptr = sreq;
}


#### 4.10.26 scatter_hspot_send()

scatter_hspot_send()  The function scatter_hspot_send(), called solely from make_send_sched() but as many
times as the number of hot-spots in the local node’s primary subdomain, completes asyn-
cronous receptions by MPI_Irecv() posted by gather_hspot_recv() for the hot-spot head
record H of the queue HotSpot[0][⌊3D/2⌋] for the local node n’s primary subdomain, and
then build the hot-spot receiving and sending schedules to send them to nodes involved
in the hot-spot. The function receives the number of posted receptions nreq = Rr, the
pointer nacc to Qnn, and the double pointer hslist to the first available S_commlist record
at CommList[c] = Csend(0) in hot-spot sending block as its arguments.


static void
scatter_hspot_send(const int rreq, int *nacc, struct S_commlist **hslist) {
struct S_hotspot *hs = HotSpot[0][OH_NBR_SELF].head;
const struct S_commlist *rl = hs->comm;
struct S_commlist *slhead = *hslist,  *sl;
const int ns=nOfSpecies, nn=nOfNodes, me=myRank, g=hs->g, nr=hs->n;
int r, ri, s, sinc, *hsr, *nofr;
dint hst;


At first we confirm the completion of  all Rr asynchronous receptions recorded in
Requests[] by MPI_Waitall() to obtain their statuses in Statuses[],  if Rr > 0. Then
we sum up NOfPGridTotal[0][s][g] = PT (0, s, g) = Qhot(s) for all s ∈[0, S) to have the
grand total population QΣhot in the hot-spot at g = H.g.
Next we scan hot-spot records Crecv(r) in primary receiving block from H.comm for each
r ∈[0, ρ) to build the receiving schedule qrecvhot (r, s) for each r ∈[0, ρ) and s ∈[0, S), where
ρ = H.n, based on Crecv(r).count = qhot(r), Qhot(s) and QΣhot. For r = 0, we calculate base
values of qrecvhot (r, s) namely qrhot(r, s) = ⌊qhot(r) · Qhot(s)/QΣhot⌋so that qrecvhot (r, s)/qhot(r) ≈
Qhot(s)/QΣhot for all r and for each s and thus each mr accommodates particles of species                                                             ∑S−1
s with approximately consistent density. However, since q′hot(r) =   s=0 qrhot(r, s) can be
less than qhot(r) at most by S −1, we need to make adjustment by letting qrecvhot (r, s) =                                                          ∑S−1
qrhot(r, s) + q∆rhot(r, s) for each s with q∆rhot(r, s) ∈[0, S) such that   s=0 q∆rhot(r, s) = qhot(r) −
q′hot(r) = q∆hot(r).
For the adjustment, first we let Qhot(s) ←Qhot(s) −qrhot(r, s) and q∆rhot(r, s) = 0 for all
s ∈[0, S). Then we scan s from 0 to find Qhot(s) > 0 and let q∆rhot(r, s) ←q∆rhot(r, s) + 1 and                                                       ∑S−1
Qhot(s) ←Qhot(s) −1 each time of finding, until we have   s=0 q∆rhot(r, s) = q∆hot(r). If we
don’t reach this goal when we have s = S −1, we go back to s = 0 and repeat the scan
cyclicly.
Now we have qrecvhot (r, s) for r = 0 and for all s ∈[0, S) in NOfRecv[r][s] and then
send them to mr by MPI_Send() giving it the tag 2 · 3D to distinguish it from those for
gathering [0, 3D) and those for sending schedule we will send later [3D, 2·3D), if mr ̸= n.
Otherwise, i.e., mr = n, we copy them into NOfPGridOut[0][s][g] = PO(0, s, g) and add it
to TotalPNext[s] as if n received it. In this case we also add qhot(r) to Qnn through the
pointer argument nacc.
Then we do above for r = 1 but this time we start the scan of Qhot(s) for the adjustment
from the next s of what we visited at last for r = 0. We repeat this for succeeding r to
have qrecvhot (r, s) for all r ∈[0, ρ).


<!-- Page 396 -->

if (rreq)  MPI_Waitall(rreq, Requests, Statuses);
for (s=0,hst=0; s<ns; s++)  hst += NOfPGridTotal[0][s][g];
for (ri=0,sinc=0,nofr=NOfRecv; ri<nr; ri++,nofr+=ns) {
const int count = rl[ri].count,  rid = rl[ri].rid;
int nget = 0;
for (s=0; s<ns; s++) {
const int ng = nofr[s] = (NOfPGridTotal[0][s][g]*count) / hst;
nget+= ng;  NOfPGridTotal[0][s][g] -= ng;
}
for (nget=count-nget; nget>0;) {
if (NOfPGridTotal[0][sinc][g]) {
nofr[sinc]++;  NOfPGridTotal[0][sinc][g]--;  nget--;
}
if (++sinc>=ns)  sinc = 0;
}
hst -= count;
if (rid==me) {
for (s=0; s<ns; s++) {
nget = NOfPGridOut[0][s][g] = nofr[s];
TotalPNext[s] += nget;
}
*nacc += count;
} else {
MPI_Send(nofr, ns, MPI_INT, rid, OH_NEIGHBORS<<1, MCW);
}
}

Now we have the receiving schedules qrecvhot (r, s) of each mr of r ∈[0, ρ) and s ∈[0, S),
and then based on them and HSRecv[ki][mi][s] = qsendhot (ki, mi, s) for each i ∈[0, Rr] we build
the sending schedule for mi as the ki-th neighbor of the local node n. Note that mi and ki
are obtained from MPI_SOURCE and MPI_TAG elements of Statuses[i] while kRr = ⌊3D/2⌋
and mRr = n for n itself which must have the hot-spot in question. Also note that for m
and m′ in the local node’s family and thus has some r and r′ such that m = mr, m′ = mr′
and r < r′, it is assured that i < i′ if we have qsendhot (ki, mi, s) and qsendhot (ki′, mi′, s) such that
ki = ki′ = ⌊3D/2⌋, mi = m and mi′ = m′ because of the scanning order in make_recv_
list() and gather_hspot_recv() so that the amount of particles transferred among the
family members is kept small.
At first we initialize HSReceiver[s] = r(s) to be 0 to mean we start scan from qrecvhot (0, s)
for the assignment to m0 for each s ∈[0, S). Then for each i ∈[0, Rr] we do the followings
for each s ∈[0, S) to determine the set of pairs S(i, s) = {(r0s, q0s), (r1s, q1s), . . .} to mean
that the node mi sends its qjs particles of species s to the hot-spot receiver node having
ordinal rjs, i.e., Crecv(rjs).rid. For i and s with qsendhot (ki, mi, s) = 0, S(i, s) = ∅. Otherwise,
we have c(i, s) pairs, i.e., |S(i, s)| = c(i, s) such that as follows.

r0s = min{r | r ≥r(s), qrecvhot (r, s) > 0}
rjs = min{r | r > rj−1s    , qrecvhot (r, s) > 0}
∑c−1
c(i, s) = min{c |     qrecvhot (rjs, s) ≥qsendhot (ki, mi, s)}
j=0


<!-- Page 397 -->

Then we let qjs, qrecvhot (rjs, s) and r(s) as follows.

qrecvhot (rjs, s)                            j < c(i, s) −1            
qjs =                         c(i,s)−2∑
 qsendhot (ki, mi, s) −         qrecvhot (rjs, s)  j = c(i, s) −1
j=0

 0                                      j < c(i, s) −1
c(i,s)−1∑             qrecvhot (rjs, s) ←                    qrecvhot (rjs, s) −qsendhot (ki, mi, s)  j = c(i, s) −1
j=0
= qrecvhot (rjs, s) −qjs
r(s) ←rc(i,s)−1s
∪S−1                            ∑S−1
Then all pairs   s=0 S(i, s) are listed from Csend(0) to have   s=0 c(i, s) records of S_                                             ∑s−1
commlist so that Csend(l) has the following where l = l(s, j) =   t=0 c(i, t) + j.

Csend(l).sid = c(i, s)
Csend(l).rid = Crecv(rjs).rid
Csend(l).region = Crecv(rjs).region = H.g
Csend(l).count = qjs
Csend(l).tag = Crecv(rjs).tag + sN = s′N   (s′ ∈[0, 2S))


Note that Csend(l).sid = c(i, s) is only for l = l(s, 0) and those for l(s, j) (j > 0) are
left undefined74. Also note that Csend(l).tag = sN  if the receiver m is the local node, or
(s + S)N  if a helper, and thus Csend(l).tag + Csend(l).rid gives one-dimensional index of
[p][s][m] of an array of [2][S][N].
Then if there are some S(i, s) ̸= ∅, i.e., there are some qsendhot (ki, mi, s) > 0, we send the
list to mi ̸= n for i < Rr by MPI_Send() with tag 3D + ki to distinguish it from those for
gathering [0, 3D) and that for the receiving schedule sent earlier in this function 2 · 3D. For
i = Rr and thus mi = n, on the other hand, we simply let H.comm point Csend(0) so that
the local node n can refer to the records in scatter_hspot_recv(), or let it be NULL if
S(i, s) = ∅for all s ∈[0, S). Then we report the caller the next available record in hot-spot
sending block being the pointer to Csend(∑S−1s=0 c(Rr, s)) through hslist argument.
Note that records sent to other nodes or linked from H.comm don’t have species and
thus it looks impossible to judge whether the records for S(i, s) for a species s exist or not
by scanning the records. However, since a receiver node or the local node mi knows whether
qsendhot (ki, mi, s) = 0 and thus S(i, s) = ∅, the records are properly scanned and processed in
the function scatter_hspot_recv_body().

for (s=0; s<ns; s++)  HSReceiver[s] = 0;
for (r=0; r<=rreq; r++) {
const int dst = r==rreq ? me          : Statuses[r].MPI_SOURCE;
const int nbr = r==rreq ? OH_NBR_SELF : Statuses[r].MPI_TAG;
struct S_commlist *slsave;
int tag;
hsr = HSRecv[nbr] + dst*ns;
for (s=0,sl=slsave=slhead,tag=0; s<ns; s++,tag+=nn) {

74They have Crecv(l).sid for the hot-spot ordinal but meaningless.


<!-- Page 398 -->

int nput = hsr[s],  nget = 0;
if (nput==0) continue;
for (ri=HSReceiver[s],nofr=NOfRecv+ri*ns; ; ri++,nofr+=ns) {
const int ng = nofr[s], ngetsave = nget;
if (ng) {
nget += ng;  *sl = rl[ri];  sl->tag += tag;
if (nput>nget)  {
nofr[s] = 0;  (sl++)->count = ng;
} else {
nofr[s] -= ((sl++)->count = nput-ngetsave);  HSReceiver[s] = ri;
break;
}
}
}
slsave->sid = sl - slsave;  slsave = sl;
}
if (r==rreq) {
hs->comm = sl>slhead ? slhead : NULL;  *hslist = sl;
} else if (sl>slhead) {
MPI_Send(slhead, sl-slhead, T_Commlist, dst, OH_NEIGHBORS+nbr, MCW);
}
}
}


#### 4.10.27 scatter_hspot_recv()

scatter_hspot_recv()  The function scatter_hspot_recv(), called solely from make_send_sched() but as many
times as the maximum ordinal of hot-spots the local node is involved in, scans all hot-
spot queued in HotSpot[][] having the ordinal h given as the argument hxidx. The other
arguments it receives are pcode for the parent status, rreq = Rr and sreq = Rs being the
number of MPI_Irecv() posted in gather_hspot_recv() and gather_hspot_send() to
mean the requests of latter are in Requests[i] where i ∈[Rr, Rs), nfrom and nto to specify
the set of neighbor indices [0, 3D) or {⌊3D/2⌋} to be scanned, nacc[2] = {Qnn, Qparent(n)n        },
and the pointer nsend to P nsend .
In this function, at first we confirm the completion of all Rs asynchronous receptions
recorded in Requests[i] by MPI_Waitall() to obtain their statuses in Statuses[i] for i ∈
[Rr, Rs). Then we call scatter_hspot_recv_body() for each neighbor Neighbors[p][k],
where index k is in the set specified, of the primary subdomain (p = 0) always and secondary
subdomain (p = 1) if exists, i.e., Parent_Old() of pcode is true, to examine the receiving
and sending schedules for a hot-spot sent from a neighbor. We also call the function once
more if helpand-helper reconfiguration is taking place to give the local node a new secondary
subdomain different from the old one, i.e., Parent_New_Diff() of pcode is true, only for
the receiving schedule.
The arguments given to gather_hspot_send_body(), other than the arugments of this
functin itself, are ps = p ∈[0, 2] to refer to HotSpot[p][], n being the neighbor index to visit,
and nacc+{0, 1} being +0 if p = 0 or +1 otherwise to specify Qnn or Qparent(n)n        respectively.


static int
scatter_hspot_recv(const int hsidx, const int pcode, const int rreq,
const int sreq, const int nfrom, const int nto, int *nacc,
int *nsend) {


<!-- Page 399 -->

const int psold = Parent_Old(pcode) ? 1 : 0;
int ps, n;
MPI_Status *st = Statuses + rreq;

if (sreq>0)  MPI_Waitall(sreq, Requests+rreq, st);
for (ps=0; ps<=psold; ps++) {
for (n=nfrom; n<nto; n++) {
scatter_hspot_recv_body(hsidx, ps, n, nacc+ps, nsend);
}
}
if (Parent_New_Diff(pcode)) {
scatter_hspot_recv_body(hsidx, 2, OH_NBR_SELF, nacc+1, nsend);
}
}


#### 4.10.28 scatter_hspot_recv_body()

scatter_hspot_recv_body()  The function scatter_hspot_recv_body(), called solely from scatter_hspot_recv() but
up to 2 · 3D + 1 times, examines if the head record H of the hot-spot queue HotSpot[p][k]
has the hot-spot whose ordinal is h, where p = psor2, k = n and h = hsidx given
by the arguments. Then  if the ordinals match, it processes the hot-spot receiving and
sending schedule sent from the k-th neighbor of the local node n’s primary (p = 0) or
secondary(p ̸= 0) subdomain including n or its helpand themselves, updating either Qnn or
Qparent(n)n       pointed by naccptr argument and/or Pnsend pointed by nsendptr argument.


static void
scatter_hspot_recv_body(const int hsidx, const int psor2, const int n,
int *naccptr, int *nsendptr) {
const int ns=nOfSpecies, me=myRank;
const int ps = psor2==2 ? 1 : psor2;
const struct S_hotspot *hs = HotSpot[psor2][n].head;
struct S_commlist *sl = hs->comm;
const int g = hs->g,  self = hs->self;
int nsend = *nsendptr;
int slidx, s, si;


At first we examine  if h is equal to H.lev, and return to the caller without doing
anything if not, because it is not the turn for H or the queue is empty and thus H has
INT_MAX ordinal. Otherwise we dequeue H to let HotSpot[p][k].head points the successor
of H.
Then  if H.self is true and p ̸= 0 to mean that the hot-spot is in the local node
n’s secondary subdomain and n is one of its hosts, we should have received the receiving
schedule in HSRecvFromParent[S]. Therefore, we copy its element HSRecvFromParent[s] =
qrecvhot (n, s) into NOfPGridOut[1][s][g] = PO(1, s, g) where g = H.g for each s ∈[0, S), to
fix each number of particles to accommodate for the hot-spot. We also add qrecvhot (n, s) to
∑S−1
TotalPNext[1][s], and   s=0 qrecvhot (n, s) to Qparent(n)n       through naccptr argument.

if (hs->lev!=hsidx)  return;
HotSpot[psor2][n].head = hs->next;
if (self && ps) {
int nacc=*naccptr;


<!-- Page 400 -->

for (s=0; s<ns; s++) {
const int nget = NOfPGridOut[1][s][g] = HSRecvFromParent[s];
nacc += nget;  TotalPNext[ns+s] += nget;
}
*naccptr = nacc;
}

Then we examine H.comm and return to the caller  if  it  is NULL to mean the lo-
cal node does not have any particles to send in the hot-spot.  Otherwise,  i.e.,  if  it
points the head of record sequence Csend(0) = CommList[b], Csend(1) = CommList[b + 1],
. . . , we have S_commlist records comprising of S(s) = {(r0s, q0s), (r1s, q1s), . . .} for s such
that NOfPGrid[p′][s][g] = PL(p′, s, g) > 0 and Csend(l).(rid, count) are (rjs, qjs) ∈S(s),                                             ∑s−1
where p′ = 1  if p = 2 or p′ = p otherwise,  l =  l(s, j) =    t=0 c(t) + j and c(s) =
Csend(l(s, 0)).sid = |S(s)|.
For each s, we do nothing if PL(p′, s, g) = 0 and thus there are no records for s, leaving
NOfPGrid[p′][s][g] = PL(p′, s, g) = 0 unchanged75. Otherwise, i.e., if PL(p′, s, g) > 0 we let
NOfPGrid[p′][s][g] be −(b+l(s, 0)+1) so that we can revisit Csend(l(s, 0)) being the head of
S(s) when we find a particle in the grid-voxel g in move_to_sendbuf_sec4p(), move_to_
sendbuf_uw4p(), move_to_sendbuf_dw4p() and/or move_and_sort_secondary(). Then
we scan all records in (rjs, qjs) in S(s) to add qjs = Csend(l(s, j)).count to NOfSend[p′′][s][rjs]
where the one-dimensional base index for [p′′][s][0] is given by Csend(l(s, j)).tag. We also
add qjs to P nsend because they will be sent.
We also examine if there exists rjs = n when H.self is true, and if so we exchange the
record Csend(l(s, j)) and Csend(l(s, c(s) −1)) so that the record for the lcoal node is at the
tail and thus the functions above let the particles for the local node n stay in n after letting
other particles be sent. In addition we let Csend(l(s, c(s) −1)).tag = −1 to indicate it is
at the tail and is for the number of particles to be accommodated by the local node rather
than that for sending.
Finally we update P nsend in the grand-grand-caller exchange_particles4p() through
the argument pointer nsendptr.

if (!sl)  return;
slidx = -(sl - CommList + 1);
for (s=0,si=0; s<ns; s++) {
int mysi = -1,  r;
const int nr = sl[si].sid;
if (NOfPGrid[ps][s][g]==0)  continue;
NOfPGrid[ps][s][g] = slidx - si;
for (r=0; r<nr; r++,si++) {
const int rid = sl[si].rid,  count = sl[si].count;
sl[si].sid = nr;
if (rid==me && self)  mysi = si;
else {
NOfSend[sl[si].tag+rid] += count;  nsend += count;
}
}
if (mysi>=0) {
struct S_commlist sltmp = sl[mysi];
sl[mysi] = sl[si-1];  sl[si-1] = sltmp;  sl[si-1].tag = -1;
}
}

75NOfPGrid[p′][s][g] will not be referred to until transbound4p() finishes.


<!-- Page 401 -->

*nsendptr = nsend;
}


#### 4.10.29 update_descriptors()

update_descriptors()  The function update_descriptors(), called from exchange_particles4p() when we
had  anywhere accommodation and  from make_recv_list()  otherwise,  reinitializes
BorderExc[][1][][].{send, recv} for the old secondary subdomain given through the ar-
gument oldp by clear_border_exchange(), and update FieldDesc[].{bc,red}.size[1]
for the new secondary subdomain given through the argument m =  newp  giving
FieldTypes[F][7] and SubDomains[m][D][2].  It also calls adjust_field_descriptor()
to modify FieldDesc[F−1].{bc,red}.size[1] for per-grid histograms giving it ps = 1.
Note that we do above if the old and new parents are different, and call clear_border_
exchange() if the old one exists, while other two functions are called if the new one exists.


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


#### 4.10.30 update_neighbors()

Neighbor_Grid_Offset()  Prior to discuss the funcion update_neighbors(), we show a macro Neighbor Grid Offset
(p, νd −1, m, d, c) which calculates

 δld(m) −δud(m) = −δd(m)  νd = 0
x0d(m, n′) = δld(m) −δld(n′) =    δld(n′) −δld(n′) = 0         νd = 1                                 
δud(n′) −δld(n′) = δd(n′)    νd = 2
where n′ = {n, parent(n)}[p] for the local node n, for GridOffset[p][∑D−1d=0 νd3d] as dis-
cussed in §4.9.5. Note that δβd (m) = SubDomains[m][d][β] and δd(n′) = GridDesc[p].c where
c = {x, y, z}[d].


#define Neighbor_Grid_Offset(PS, N, SD, D, XYZ)\
(N==0 ? 0 : (N<0 ? SubDomains[SD][D][OH_LOWER]-SubDomains[SD][D][OH_UPPER] :\
GridDesc[ps].XYZ))


update_neighbors()  The function update_neighbors() is called from init4p() with p = ps = 0, and from
rebalance4p() or exchange_particles4p() with p = 1 when we had normal or anywhere


<!-- Page 402 -->

accommodation respectively. The function initializes/updates AbsNeighbors[p][k] for all
k ∈[0, 3D) to let it have;
{
Neighbors[p][k]         Neighbors[p][k] ≥0
AbsNeighbors[p][k] = mk =
−(Neighbors[p][k] + 1)  Neighbors[p][k] < 0

and  lets GridOffset[p][k] =  gidx(x00(mk, n′), . . .) where  n′ =  {n, parent(n)}[p] and
x0d(mk, n′) is given by Neighbor_Grid_Offset(), as discussed in §4.9.5.


static void
update_neighbors(const int ps) {
int n = 0,  nx,  ny = 0,  nz = 0;
const int nn = nOfNodes;

Do_Z(for (nz=-1; nz<2; nz++)) {
Do_Y(for (ny=-1; ny<2; ny++)) {
for (nx=-1; nx<2; nx++,n++) {
int nbr = Neighbors[ps][n];
nbr = AbsNeighbors[ps][n] = nbr<0 ? -(nbr+1) : nbr;
if (nbr>=nn)  GridOffset[ps][n] = 0;
else
GridOffset[ps][n] =
Coord_To_Index(Neighbor_Grid_Offset(ps, nx, nbr, OH_DIM_X, x),
Neighbor_Grid_Offset(ps, ny, nbr, OH_DIM_Y, y),
Neighbor_Grid_Offset(ps, nz, nbr, OH_DIM_Z, z),
GridDesc[0].w, GridDesc[0].dw);
}
}
}
}


#### 4.10.31 set_grid_descriptor()

set_grid_descriptor()  The function set_grid_descriptor() is called from init4p() with idx = i = 0 and
nid = m = n arguments for the local node n for the initialization, from rebalance4p() or
exchange_particles4p() with i = 1 and m = parent(n) when we had normal or anywhere
accommodation respectively, and from make_recv_list() with i = 2 and m = parent(n),
when helpand-helper reconfiguration is taking place assigning m to the local node as its
secondary subdomain. The function lets GridDesc[i] have the shape information of the
per-grid histogram for the subdomain m. Note that GridDesc[2] is used to have the shape
information of new parent(n) due to helpand-helper reconfiguration while [1] keeps that of
old parent(n).
The function lets the elements w, d and h of GridDesc[i] be δmaxd  +4eg = Grid[d].size+
4 · OH_PGRID_EXT with d = 0, 1 and 2 respectively for the physical array size, and dw be
d × w, if D = 3. The elements d and/or h are, however, is let be 1 if D < 2 or D < 3
respectively.
Then the elements x, y and z are let be δd(m) = SubDomains[m][d][1] −SubDomains
[m][d][0] with d = 0, 1 and 2 respectively for the upper bound (or the size) of interior of
the subdomian m,  if D = 3 and m ≥0. The elements y and/or z are, however, is let
be 0  if D < 2 or D < 3 respectively. On the other hand,  if m < 0 to mean that the
local node does not have secondary subdomain, those elements are let be −4eg so that any


<!-- Page 403 -->

possible coordinate value less than 2eg relative to the upper bound falls out-of-bounds with
a coordinate less than −2eg being the absolute lower bound and thus For_All_Grid() with
them does nothing.


static void
set_grid_descriptor(const int idx, const int nid) {
const int exto2 = OH_PGRID_EXT<<2;
const int w = GridDesc[idx].w = Grid[OH_DIM_X].size+(exto2);
const int d = GridDesc[idx].d =
If_Dim(OH_DIM_Y, Grid[OH_DIM_Y].size+(exto2), 1);

GridDesc[idx].h = If_Dim(OH_DIM_Z, Grid[OH_DIM_Z].size+(exto2), 1);
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
GridDesc[idx].x = GridDesc[idx].y = GridDesc[idx].z = -exto2;
/* to ensure, e.g., x+2*(OH_PGRID_EXT)<=-2*(OH_PGRID_EXT) */
}
}


#### 4.10.32 adjust_field_descriptor()

adjust_field_descriptor()  The function adjust_field_descriptor() is called from init4p() with argument ps =
p = 0 for the initialization of the local node n’s primary subdomain, and from update_
descriptors() with p = 1 for n’s secondary subdomain newly assigned to it by helpand-
helper reconfiguration. The function modifies FieldDesc[F−1].{bc, red}.size[p] for per-
grid histogram so that the broadcast/reduction for it are performed on all subarrays for
species s ∈[0, S) rather than on a subarray for a certain s. That is, with their value σ set
∏D−1
by set_field_descriptors(), the function updates it as (S−1)  d=0 Φd(F−1)+σ where
Φd(f) is FieldDesc[f].size[d] and thus δmaxd  + 4eg for per-grid histogram with f = F−1.
This means that the origial value σ specifies that the collective communication is peformed
on the elements from [s][b] to [s][b+σ−1] while the updated one lets the communication be
done on the elements from [0][b] to [S−1][b+σ−1].


static void
adjust_field_descriptor(const int ps) {
const int f = nOfFields - 1,  ns = nOfSpecies;
int d, fs;

for (d=0,fs=1; d<OH_DIMENSION; d++)  fs *= FieldDesc[f].size[d];
fs *= ns-1;
FieldDesc[f].bc.size[ps] += fs;    FieldDesc[f].red.size[ps] += fs;
}
