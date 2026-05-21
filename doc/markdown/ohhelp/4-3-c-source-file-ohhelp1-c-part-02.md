# 4.3 C Source File ohhelp1.c - Part 2

Source: `doc/original/ohhelp.pdf`, pages 189-220.

<!-- Page 189 -->

static int
compare_int(const void* x, const void* y) {
int xx=*((int*)x), yy=*((int*)y);

if (xx<yy)  return(-1);
if (xx>yy)  return(1);
return(0);
}


#### 4.3.17 schedule_particle_exchange()

schedule_particle_exchange()  The function schedule particle exchange(), called from try_stable1() or rebalance1
() with one argument reb, makes the schedule of the inter-node transfer of particles which
reside in the primary subdomain of the local node in the next step. The argument reb has
one of the followings.

- −1 means we were in secondary mode with anywhere accommodation in the last step
and try_stable1() found the helpand-helper configuration is sustainable. Floating
particles for a subdomain may be found in any nodes due to, for example, particle
injections.

- 0 means we were in secondary mode with normal accommodation in the last step
and try_stable1() found the helpand-helper configuration is sustainable. Floating
particles for a subdomain must be found in nodes responsible for the subdomain or
its neighbors.

- 1 means we were in secondary mode with normal accommodataion in the last step
but try_stable1() found the helpand-helper configuration must be reformed by
rebalance1().  Floating particles for a subdomain must be found in nodes which
were responsible for the subdomain or its neighbors.

- 2 means we were in primary mode with normal accommodataion in the last step but
try_primary1() found we cannot continue primary mode execution and a new hel-
pand-helper configuration must be established by rebalance1(). Floating particles
for a subdomain must be found in nodes which were responsible for the subdomain
or its neighbors as primary ones.

- 3 means a new helpand-helper configuration was established by rebalance1() and
floating particles for a subdomain may be found in any nodes due to, for example,
initial particle distribution or particle injections, to mean we have anywhere accom-
modation.

Before this function is called, the callers have determined the numbers of primary and
secondary particles which should be gotten in or put out from/to each node n and have set
them in Rgetn = NN [n].get.prime and Qgetparent(n) = NN [n].get.sec, where NN  is Nodes
if reb ≤0 or NodesNext otherwise. That is, if a new family tree is build by rebalance1(),
NodesNext[] has the new configuration while Nodes[] keeps the old configuration.


static void
schedule_particle_exchange(int reb) {
int me=myRank, nn=nOfNodes, ns=nOfSpecies, nnns=nn*ns;


<!-- Page 190 -->

struct S_node *mynode, *ch;
int i, slidx;
struct S_commsched_context context;


First we build the sequence of S_commlist records in CommList[] for sending/receiving
particles in the local primary subdomain n. The sequnce is built by a loop to scan the
the family members rooted by the local node n in the next step, that is NN [n], calling
sched_comm() for each member with the following arguments.

- toget is Rgetn = NN [n].get.prime for the local node n or Qgetn = NN [m].get.sec for
its helper m. If it is positive, sched_comm() adds records into CommList[]. Otherwise,
i.e., the node receives no particles in the subdomain n but put some of them out,
sched_comm() will do nothing.

- rid is the receiving node ID n or m.

- tag is 0 for rid = n meaning primary particles, and S for others meaning secondary
particles. An MPI communication tag will be given by the sum of this argument and
the species of the particles transferred, so that the receiver recognizes whether the
particles are primary or secondary as well as their species. Moreover, the tag value of
pS+s where p ∈{0, 1} can be used for the one-dimensional index of a two-dimensional
array of [2][S] to access its element [p][s].

- reb is simply equal to the argument reb of this function.  It notifies sched_comm()
where it can find particle senders, only in current neighbor families (0), both in old
and new neighbor families (1), only in current neighbors (2), or in any nodes (−1
or 3).  It also shows that sched_comm() should refer to Nodes[]  if reb ≤0 or to
NodesNext[] if reb > 0 for the helpand-helper configuration. Note that if reb = −1
it is set to 3 after the call of sched_comm() to mean anywhere accommodation.

- context is a S_commsched_context structure having the following elements to hold
the execution context of sched_comm(), whose initial value is shown in parens.

  - neighbor is the index of DstNeighbors[] currently processed (0).

  - sender is the sender node ID currently processed (0 if reb = −1 or reb = 3, or
DstNeighbors[0] otherwise).

  - comidx is the index of CommList[] at which the next S_commlist record will be
stored (0).

  - spec is the species of the sender node currently processed (0).

  - dones is the number of already processed particles in spec (0).

  - donen is the number of already processed primary/secondary particles accom-
modated by the sender which is a family member (0).

Note that sched_comm() also consults TempArray[m] to check double visiting of the sender
node m, and thus we clear TempArray[] and then turns on the entry of the first sender. Also
note that sched_comm() let RLIndex[k] be the starting index of CommList[] for senders in
the family rooted by DstNeighbors[k] for all k > 0, and thus we let RLIndex[0] = 0.
Another  remark  is  that we  have  to  adjust  Qgetm  = NN [m].get.sec  for  all
m ∈F(n),  if reb >  0 meaning the function  is  called from rebalance1() which
set Qgetm  assuming m does not have any secondary  particles.   Therefore, we have


<!-- Page 191 -->

∑S            ∑S
to  calculate    s=0 q(m)[1][s][n] =    s=0 NOfPrimaries[1][s][m]  for m ∈ H(n) and∑S                ∑S
s=0 q(n)[1][s][parent(n)] =    s=0 NOfPLocal[1][s][parent(n)]  for n by count_real_
stay(), to set it in NN [{m, n}].stay.sec for further references33, and to subtract it from
Qget{m,n} so that they reflects the number of secondary particles accommodated by those
nodes because their secondary subdomain is unchanged or they have secondary particles in
new secondary subdomain accidentally.
After we finish the loop to call sched_comm() and let SLHeadTail[0] = context.comidx
to record the end of the primary receiving block, we return to the caller try_stable1()
or rebalance1()  if reb = 3 (including the case updated from −1) because we cannot
notify senders of the schedule by neighboring communication and broadcast in neighboring
families.

RLIndex[0] = 0;
context.neighbor = 0;
context.sender = (reb<0 ||reb==3) ? 0 : DstNeighbors[0];
context.comidx = 0;
context.spec = 0;  context.dones = 0;  context.donen = 0;
for (i=0; i<nn; i++) TempArray[i] = 0;
TempArray[context.sender] = 1;
if (reb>0) {
mynode = NodesNext + me;
for (ch=mynode->child; ch; ch=ch->sibling)
ch->get.sec -= (ch->stay.sec=count_real_stay(NOfPrimaries+nnns+ch->id));
/* NOfPrimaries[1][0][cid] */
sched_comm(mynode->get.prime, me, 0, reb, &context);
for (ch=mynode->child; ch; ch=ch->sibling)
sched_comm(ch->get.sec, ch->id, ns, reb, &context);
if (mynode->parent)
mynode->get.sec -=
(mynode->stay.sec=count_real_stay(NOfPLocal+nnns+mynode->parentid));
/* NOfPLocal[1][0][pid] */
} else {
mynode = Nodes + me;
sched_comm(mynode->get.prime, me, 0, reb, &context);
for (ch=mynode->child; ch; ch=ch->sibling)
sched_comm(ch->get.sec, ch->id, ns, reb, &context);
if (reb<0)  reb = 3;
}
SLHeadTail[0] = slidx = context.comidx;
if (reb==3) return;


Now we have the transfer schedule for the local primary subdomain in CommList[i]
where i ∈[0, σ) and σ = context.comidx = slidx which has sub-blocks starting from
RLIndex[k] for senders in each neighboring families rooted by DstNeighbors[k]. Since the
largest k namely kmax = context.neighbor could be less than OH_NEIGHBORS = 3D and
we let RLIndex[k] be σ for all k ∈[kmax, 3D], which could be left unassigned (very unlikely,
but  . . . ). The value of σ was also stored into SLHeadTail[0] from which the local node
receives sending schedules from its neighbors to form primary sending block.
Then we perform MPI_Sendrecv(), MPI_Send() or MPI_Recv() to send T_Commlist
type data in CommList[i] for  i ∈[RLIndex[k], RLIndex[k+1]−1] to the k-th neighbor

33NN[m].stay.sec is referred to by level-4p/4s function make recv list(), while NN[n]’s is not referred
to so far but we set the value to it for consistency.


<!-- Page 192 -->

DstNeighbors[k] from the local node, and/or to receive the schedule of (3D−1−k)-
th neighbor subdomain SrcNeighbors[k] which  is stored in the block starting from
CommList[SLHeadTail[0]]. The size of each received schedule for sending, which must be less
than N + NS, is obtained by MPI_Get_count() and its total is added to SLHeadTail[0]
and is stored in SLHeadTail[1]. Note that since we omit sending/receiving for negative
DstNeighbors[k] and SrcNeighbors[k] respectively, the schedule for a process is trans-
ferred only once.
Now the local node has the particle sending schedule, i.e., primary sending block, for
particles accommodated by it and its helpers.  Then  if reb = 2, since no nodes have
helpers because we are in primary mode, we finish this function and return to its caller
rebalance1().

for (i=context.neighbor+1; i<=OH_NEIGHBORS; i++)  RLIndex[i] = slidx;
for (i=0; i<OH_NEIGHBORS; i++) {
int dst=DstNeighbors[i];
int src=SrcNeighbors[i];
int rc;
MPI_Status st;
if (dst==me) continue;
if (src>=0) {
if (dst>=0)
MPI_Sendrecv(CommList+RLIndex[i], RLIndex[i+1]-RLIndex[i], T_Commlist,
dst, 0,
CommList+slidx, nn+nnns, T_Commlist, src, 0, MCW, &st);
else
MPI_Recv(CommList+slidx, nn+nnns, T_Commlist, src, 0, MCW, &st);
MPI_Get_count(&st, T_Commlist, &rc);
slidx += rc;
} else if (dst>=0)
MPI_Send(CommList+RLIndex[i], RLIndex[i+1]-RLIndex[i], T_Commlist,
dst, 0, MCW);
}
SLHeadTail[1] = slidx;
if (reb==2) return;


Now the local node broadcasts the transfer schedules, that created by itself and those
received from neighbors, to its helpers by oh1_broadcast(). First the local node broadcasts
its SLHeadTail[0, 1] to show its helpers the size of the primary receiving and primary
sending blocks, and stores that received from its helpand into SecSLHeadTail[0, 1], which
are initialized to be 0 for the family tree root because it does not receive anything. Then
both blocks of T_Commlist type are broadcasted but, if rebalanced in secondary mode, the
primary receiving block will be ignored by the helpers whose helpand remains unchanged
by the rebalance (i.e., the local node is their old and new helpand) because they only refer
to the duplicated block broadcasted in the newly established family afterward.

SecSLHeadTail[0] = SecSLHeadTail[1] = 0;
oh1_broadcast(SLHeadTail, SecSLHeadTail, 2, 2, MPI_INT, MPI_INT);
oh1_broadcast(CommList, CommList+slidx, slidx, SecSLHeadTail[1],
T_Commlist, T_Commlist);
}


<!-- Page 193 -->

#### 4.3.18 count_real_stay()

count_real_stay()  The  function count_real_stay(),  called from schedule_particle_exchange() and
∑S
rebalance1(), calculatates   s=0 q(k)[p][s][l] for (k, l, p) being (n, n, 0), (n, parent(n), 1),
or (m, n, 1) where m ∈H(n), and return the sum, being the number of primary/
secondary particles really accommodated by the local node n or its helper m, to the
caller. Since the targets of the summation are NOfPLocal[0][s][n] or NOfPrimaries[0][s][n],
NOfPLocal[1][s][parent(n)] and NOfPrimaries[1][s][m] respectively, the callers specify the
pointer to the element of s = 0 through the sole argument np of this function.


static int
count_real_stay(int *np) {
const int ns=nOfSpecies, nn=nOfNodes;
int stay, s;

for (s=0,stay=0; s<ns; s++,np+=nn)  stay += *np;
return(stay);
}


#### 4.3.19 sched_comm()

sched_comm()  The function sched comm(), called only from schedule_particle_exchange() with the
arguments discussed in §4.3.17, adds S_commlist records to CommList[] from its index
context->comidx, for the transfer of pget = toget (possibly non-positive) particles in the
primary subdomain n of the local node to the node nr = rid, which is the local node itself
or one of its helpers, from the node ns = context->sender and its successors which are
explained later. Each element of S_commlist record is set to the following.

- rid is the argument rid = nr always.

- sid is context->sender = ns or its successor.

- region is myRank = n always.

- tag is t + s for sepcies s where t is the argument tag which is 0 for if nr = n, i.e., nr
is the helpand of the subdomain n, or S otherwise, i.e., nr is a helper.

- count is the number of particles of the species s in the subdomain n transferred from
the node ns to node nr.

Note that it is possible that nr = ns to make a on-node communication for the particle
transfer from the primary to the secondary subdomain of the node nr = ns and vice versa.
The transfer starts from the particles of species s set to the argument context->spec and
accommodated by the node ns, but some of them whose amount qs is set in the argument
context->dones have already been processed.


static void
sched_comm(int toget, int rid, int tag, int reb,
struct S_commsched_context *context) {
int neighbor = context->neighbor, sid = context->sender;
int comidx = context->comidx;
int s=context->spec, havedones=context->dones, havedonen=context->donen;


<!-- Page 194 -->

int me=myRank;
int nn=nOfNodes, ns=nOfSpecies, nnns=nn*ns;
int i=nn*s+sid;                       /* [0][s][sid] */
struct S_node *nodesnext = reb>0 ? NodesNext : Nodes;


The heart of this funciton is to determine the count element of the S_commlist records.
To do that, we scan NOfPrimaries[0][s][ns] and NOfPrimaries[1][s][ns] from initial setting
of ns being context->sender and s being context->spec, incrementing s and, when s
goes back to 0 cyclicly, advancing ns to successors, while pget is positive. The count for
the species s and the sender node ns, namely pput = toput, is basically

pput = NOfPrimaries[0][s][ns] + NOfPrimaries[1][s][ns] −qs

representing the number of unprocessed particles which was accommodated by the node ns
but moved into the subdomain n. However, we have the following two exceptions where
NN  is NodesNext if the argument reb > 0 indicating the family is newly established by
rebalancing, or Nodes otherwise.

- If n = ns, i.e., the node ns is local node and thus the helpand of the family for n,
we have to replace NOfPrimaries[0][s][ns] with 0 if NN [ns].get.prime = Rgetns > 0
meaning no primary particles are put out from ns, or with −Rgetns otherwise indicating
pushing-down some particles to its helpers. Let g = max(0, −Rgetns ).

- If n = parent(ns), i.e., the node ns is a helper of the family for n, we have to replace
NOfPrimaries[1][s][ns] with 0 if NN [n].get.sec = Qgetns > 0 meaning no secondary
particles are put out from ns, or with −Qgetns otherwise indicating secondary overflow.
Let g = max(0, −Qgetns ).

In both cases above, g has the number of total particles, if any, to be put out regardless of
species. Thus we maintain the already-processed number of particles qn whose initial value
is given by the argument context->donen. Thus if qn < g we still have g−qn particles to be
processed. Therefore, number of particles of s to be put is min(NOfPrimaries[p][s][ns], g −
qn) where p = 0 if ns = n or p = 1 otherwise, and qn should be incremented by this amount
after all the particles of s are processed.
Then if pput > pget, i.e., the node ns still has particles in question more than those to
be sent to the node nr, pput is set to pget, qs is incremented by pget to indicate this amount
have been processed, and pget is cleared to be 0 (by subtracting pput = pget from it) to finish
the scanning loop. Otherwise, pget is decremented by pput (and possibly becomes 0) and we
completed the process for the species s. That is, qs is cleared to be 0, qn is incremented as
discussed above, and, after adding the records to CommList[], s is incremented to process
new species. Then, if s becomes S and goes back to 0, ns is advanced to process new sender
node.

while (toget>0) {
struct S_node *snoden=nodesnext+sid;
int npp = NOfPrimaries[i], nps = NOfPrimaries[i+nnns];
/* [0/1][s][sid] */
int toput, hdninc = 0;
int next=1;
if (sid<0)  local_errstop("PARTICLE TRANSFER SCHEDULING ERROR");
if (sid==me) {
int nput = snoden->get.prime + havedonen;


<!-- Page 195 -->

nput = nput<0 ? -nput : 0;
if (nput<npp) npp = nput;
hdninc = npp;
}
else if (snoden->parentid==me) {
int nput = snoden->get.sec + havedonen;
nput = nput<0 ? -nput : 0;
if (nput<nps) nps = nput;
hdninc = nps;
}
toput = npp + nps - havedones;
if (toput>0) {
struct S_commlist *cptr = CommList+(comidx++);
if (toput>toget) {
havedones += toget;
toput = toget;
next = 0;
}
cptr->rid = rid;  cptr->sid = sid;  cptr->region = me;
cptr->count = toput;  cptr->tag = tag + s;
toget -= toput;
}
if (next) {
havedones = 0;  havedonen += hdninc;
s++;  i += nn;

The advancement of ns is performed in the families rooted by neighboring subdomains
of n and the family of n itself. That is,  if ns is the root of a family of currently pro-
cessed k-th neighboring subdomain whose initial value is given by context->neighbor,
i.e., ns = DstNeighbors[k], the argument reb ≤1 to indicate that we are in secondary
mode currently, and ns has some helpers, we advance to ns’s first helper. On the other
hand,  if ns is a helper of a family of the k-th neighboring subdomain, reb ≤1, and ns
has a sibling, we advance to ns’s sibling. Otherwise, i.e., ns is the last family member of
the k-th neighboring subdomain (helpand without helper, possiblily due to that we are in
primary mode, or the last helper), we advance k to its first succeeding neighboring subdo-
main which has not been visited (i.e., DstNeighbors[k] ≥0) or k becomes 3D, each time
setting RLIndex[k] to the next index of CommList[]. In the former case, we simply set k to
the neighbor which has not been visited yet (as the root). In the latter case, we have to
visit the newly established family for n if the argument reb is 1 (in secondary mode) or 2
(in primary mode), since the neighboring subdomain families are those before rebalancing.
(The case of 3 will be explained later.) In this case we start the scan from its first helper
because there should be DstNeighbors[k] = n and thus we must have already visited n it-
self. This extra family scan with k = 3D will stop after the last helper is processed, or does
not eventually start if n’s family does not have helpers or reb = 0, resulting in ns = −1.
The advancement of ns should take care of the possibility that a sender may be scanned
twice, as the root of a neighboring family and as a helper of another neighboring family.
To detect the second visit, TempArray[ns] is cleared to be zero by the caller schedule_
particle_exchange() and then turned to 1 when ns is visited. Thus if we encounter a
node ns with TempArray[ns] = 1, we skip the node and advance ns further.
The family scan above is not performed when reb ∈{−1, 3} to indicate that we have
anywhere accommodation. In this case, we simply scans all nodes from 0 to N −1.


<!-- Page 196 -->

if (s==ns) {
havedonen = 0;
s = 0;
if (reb>=0 && reb!=3) {
struct S_node *nodes = (neighbor<OH_NEIGHBORS ? Nodes : NodesNext);
struct S_node *snode = nodes + sid;
while (sid>=0) {
if (neighbor==OH_NEIGHBORS) {
snode = snode->sibling;  sid = snode ? snode - nodes : -1;
}
else if (sid==DstNeighbors[neighbor] && reb<2 && snode->child) {
snode = snode->child;  sid = snode - Nodes;
}
else if (sid!=DstNeighbors[neighbor] && reb<2 && snode->sibling) {
snode = snode->sibling;  sid = snode - Nodes;
}
else {
RLIndex[++neighbor] = comidx;
while(neighbor<OH_NEIGHBORS && (sid=DstNeighbors[neighbor])<0)
RLIndex[++neighbor] = comidx;
if (neighbor==OH_NEIGHBORS) {
nodes = NodesNext;
snode = nodes[me].child;
sid = (snode && reb) ? snode - nodes : -1;
} else {
snode = Nodes + sid;
}
}
if (sid>=0 && TempArray[sid]==0) {
TempArray[sid] = 1;  break;
}
}
} else {
sid++;
}
i = sid;
}
}
}

Finally, after we complete the process for all particles the node nr receives, we store the
currently visiting node ns to context->sender toghether with the neighboring subdomain
index k of the family which ns belongs to into context->neighbor. We also store s, qs
and qn into context->{spec, dones, donen} respectively, together with the next index of
CommList[] into context->comidx. By returning these values, we can continue the scan
specified by them in the next call of this function.

context->neighbor = neighbor;  context->sender = sid;
context->comidx = comidx;
context->spec = s;  context->dones = havedones;  context->donen = havedonen;
}


<!-- Page 197 -->

#### 4.3.20 make_comm_count()

make_comm_count()  The function make comm count, called from try_stable1() or rebalance1(), sets (the
base of) TotalPNext[][] unconditionally and NOfRecv[][][] and NOfSend[][][]  if necessary.
Besides the arguments of callers themselves, i.e., currmode, level and stats, it has two
additional arguments as follows.

- reb = 0 if called from try_stable1(), or reb = 1 otherwise, i.e., from rebalance1().

- oldparent has the node ID of the local node’s helpand, in the configuration before
rebalancing if done. More specifically, this aregument for the local node n has the
following value where parent(n) and parentold(n) are n’s new (or current) and old
helpand before rebalancing.

 parent(n)    reb = 0
oldparent =   parentold(n)  reb = 1 ∧currmode = 1                           
−1          reb = 1 ∧currmode ̸= 1



static void
make_comm_count(int currmode, int level, int reb, int oldparent, int stats) {
int nn=nOfNodes, ns=nOfSpecies, nnns=nn*ns, nnns2=nnns*2, me=myRank;
struct S_node *mynode=Nodes+me;
int newparent=mynode->parentid;
int ps, s, i;


The first job of this function is to broadcast the primary receiving block CommList
[SLHeadTail[0]] to the (new) family members to have the block in SecRList[SecRLSize].
This broadcast is necessary for the following cases.

- The case reb = 1 and currmode = MODE_NORM_PRI (primary mode) in which
schedule_particle_exchange() did not broadcast the block because we do not have
old families and have not yet build new family communicators. SecRList is placed
following the primary sending block and thus starts from CommList[SLHeadTail[1]].

- The case reb = 1 and currmode = MODE_NORM_SEC (secondary mode) in which the
secondary receiving block was given from old helpand. SecRList is placed follow-
ing the secondary sending block and thus starts from CommList[SLHeadTail[1] +
SecSLHeadTail[1]].

- The case reb = 1 and Mode_Is_Any() for currmode  is true, or currmode =
MODE_ANY_SEC, in which schedule_particle_exchange() did not broadcast the
block because any node can be a sender to any subdomain and thus old family con-
figuration is useless if rebalanced. SecRList is placed following the primary receiving
block and thus starts from CommList[SLHeadTail[0]].

As done in schedule_particle_exchange(), the broadcast is done by two successive calls
of oh1_broadcast(), the former sending the size SLHeadTail[0] and the latter sending
CommList[SLHeadTail[0]] of T_Commlist data type.
On the other hand, if the broadcasting is not necessary (reb = 0 and currmode = 1),
SecRList and SecRLSize are set to represent the secondary receiving block which has
already obtained.


<!-- Page 198 -->

if (reb || currmode==MODE_ANY_SEC) {
SecRLSize = 0;
oh1_broadcast(SLHeadTail, &SecRLSize, 1, 1, MPI_INT, MPI_INT);
if (currmode==MODE_NORM_PRI)
SecRList = CommList + SLHeadTail[1];
else if (currmode==MODE_NORM_SEC)
SecRList = CommList + SLHeadTail[1] + SecSLHeadTail[1];
else
SecRList = CommList + SLHeadTail[0];
oh1_broadcast(CommList, SecRList, SLHeadTail[0], SecRLSize,
T_Commlist, T_Commlist);
} else {
SecRList = CommList + SLHeadTail[1];
SecRLSize = SecSLHeadTail[0];
}

Next, if level = 1, Mode_Is_Any() for currmode is true, or stats ̸= 0 meaning that we
need to return the shadow of NOfRecv[][][] and NOfSend[][][], namely RecvCounts[][][] and
SendCounts[][][], as level-1 API, or to have them for all-to-all particle transfers or statistics,
we count receiving and sending particles to set their amounts into the arrays by scanning
CommList[].  First we scan the primary receiving block by make_recv_count() and then
the secondary receiving block if the local node is not the root. Note that these scans not
only for NOfRecv[][][] but also for TotalPNext[][] and NOfSend[][][]. The former is to give the
base number of the particles the local node has in the next step by counting the receiving
particles. The latter is necessary because receivings block may have paritlce ejections to
family members.
Another remark is that we have to scan the secondary receiving block given by the
old helpand before rebalancing which is different from the new (current) helpand. That
is, in this case the local node ejects all the old secondary particles to the members in the
new family rooted by the old helpand, but this ejection may not be recoreded in sending
blocks.  This operation must be performed only when we were in secondary mode and
rebalancing were taken place switching the helpand of the local node. As discussed at the
beginning of this section, this condition for the local node n is confirmed by oldparent ̸=
parent(n) ∧oldparent ≥0.
Then if Mode_Is_Any() for currmode is true requiring all-to-all particle transfers, we
globally exchange NOfRecv[][][] by MPI_Alltoall() with T_Histogram data type to get
NOfSend[p][s][m] of the local node n from NOfRecv[p][s][n] of the node m34.
Otherwise, we can set elements of NOfSend[][][] by scanning sending blocks by calling
make_send_count() (usually) twice, once giving primary sending block, and then with
secondary sending block if the local node is not the root of the old family tree. Note that
in this scan we may encounter a S_commlist record which has already been found in the
scan of receiving blocks, but it is not hazardous because it simply overwrites an element of
NOfSend[][][] with the same value which has been stored.
If the operations above are not performed because level > 1, currmode < 2 and
stats = 0, the case in which we need to have neither NOfRecv[][][]| nor NOfSend[][][], we
only count the base value of TotalPNext[][] by calling count_next_particles() (usually)
twice, once giving primary receiving block, and then with secondary receiving block if the
local node is not the root of the family tree.

34Since Intel MPI has a bug in MPI Alltoall() with 12 ≤N ≤16 and the data type T Histogram that
NOfSend[p][s][n] of the local node n is not updated for p > 0 or s > 0, we have to copy NOfRecv[p][s][n] to
it explicitly until the bug is fixed.


<!-- Page 199 -->

for (s=0; s<ns*2; s++) TotalPNext[s] = 0;             /* TotalPNext[p][s] */
if (level==1 || Mode_Is_Any(currmode) || stats) {
for (i=0; i<nnns2; i++)  NOfRecv[i] = NOfSend[i] = 0;
make_recv_count(CommList, SLHeadTail[0]);
if (newparent>=0)
make_recv_count(SecRList, SecRLSize);
if (oldparent!=newparent && oldparent>=0)
make_recv_count(CommList+SLHeadTail[1], SecSLHeadTail[0]);
if (Mode_Is_Any(currmode)) {
MPI_Alltoall(NOfRecv, 1, T_Histogram, NOfSend, 1, T_Histogram, MCW);
#ifndef INTEL_MPI_BUG_FIXED
for (ps=0,i=me; ps<2; ps++)  for (s=0; s<ns; s++,i+=nn)
NOfSend[i] = NOfRecv[i];
#endif
} else {
make_send_count(CommList+SLHeadTail[0], SLHeadTail[1]-SLHeadTail[0]);
if (oldparent>=0)
make_send_count(CommList+SLHeadTail[1]+SecSLHeadTail[0],
SecSLHeadTail[1]-SecSLHeadTail[0]);
}
} else {
count_next_particles(CommList, SLHeadTail[0]);
if (newparent>=0)
count_next_particles(SecRList, SecRLSize);
}
if (stats) stats_secondary_comm(currmode, reb);

The last operation performed if level = 1 is to complete the setting of TotalPNext[][]
by adding the following q(p, s) to its element [p][s] for the local node n, while this operation
is left to level-2 function move_to_sendbuf_secondary() if level > 1.
{
min(0, −Rgetn  )  p = 0               qput(p) =
min(0, −Qgetn  )  p = 1
{
NOfPLocal[0][s][n]         p = 0
qstay(p, s) =
NOfPLocal[1][s][parent(n)]  p = 1
(                   (    ∑s−1        ))
q(p, s) = max  0, qstay(p, s) −max  0, qput(p) −     qstay(p, t)
t=0

That is, if the local node puts some primary/secondary particles out to its family members,
this ejection is done in smaller-first manner of species identifiers to reduce their particle
amounts or to make them empty. Otherwise, the number of particles currently accommo-
dated by the local node is simply added to the base value.

if (level==1) {
for (ps=0,i=0; ps<(newparent<0?1:2); ps++) {
int putme = ps==0 ? -mynode->get.prime : -mynode->get.sec;
int *mynps = ps==0 ? NOfPLocal+me : NOfPLocal+nnns+newparent;
if (putme<0) putme = 0;
for (s=0; s<ns; s++,i++,mynps+=nn) {
int stay=*mynps;
int tpni=TotalPNext[i];
if (putme<stay) {


<!-- Page 200 -->

TotalPNext[i] = tpni + stay - putme;  putme = 0;
}
else putme -= stay;
}
}
}
}


#### 4.3.21 make_recv_count()

make_recv_count()  The function make recv count(), called only from make_comm_count(), scans a part of
CommList whose head and size are specified by the arguments rlist and rlsize. The scan
is to set the primary (p = 0) or secondary (p = 1) particle count c of species s which the
local node n will receive from the node m into NOfRecv[p][s][m]. Therefore, when we find
a S_commlist record whose rid is n and which has tag = t = pS + s and sid = m, we set
its count = c into NOfRecv[p][s][m]. Note that since the one-dimensional index of [p][s][m]
for an array [2][S][N] is (pS + s)N + m, that index can be calculated by tN + m.
The receiving count c is also added to the element of the array TotalPNext[p][s] whose
one-dimensional index is pS +s or simply t, to have the total number of primary/secondary
receiving particles of species s.
In addition, the function also sets the number of particles c of species s which the local
node n will send to its family member node m as its primary (p = 0) or secondary (p = 1)
particles into NOfSend[p][s][m]. That is, when we find a record whose sid is n and which
has tag = t = pS + s and rid = m, we set its count = c into NOfSend[p][s][m] whose
one-dimensional index is tN + m = (pS + s)N + m.


static void
make_recv_count(struct S_commlist* rlist, int rlsize) {
int me=myRank, nn=nOfNodes;
int i;

for (i=0; i<rlsize; i++) {
int rid=rlist[i].rid, sid=rlist[i].sid;
int tag=rlist[i].tag, count=rlist[i].count;
if (rid==me) {
NOfRecv[tag*nn+sid] = count;
TotalPNext[tag] += count;
}
if (sid==me)
NOfSend[tag*nn+rid] = count;
}
}


#### 4.3.22 make_send_count()

make_send_count()  The function make send count(), called only from make_comm_count(), scans a part of
CommList whose head and size are specified by the arguments slist and slsize. The scan
is to set the primary (p = 0) or secondary (p = 1) particle count c of species s which the
local node n will send to the node m into NOfSend[p][s][m].  Therefore, when we find a
S_commlist record whose sid is n and which has tag = t = pS + s and rid = m, we set


<!-- Page 201 -->

its count = c into NOfSend[p][s][m]. Note that since the one-dimensional index of [p][s][m]
for an array [2][S][N] is (pS + s)N + m, that index can be calculated by tN + m.


static void
make_send_count(struct S_commlist* slist, int slsize) {
int me=myRank, nn=nOfNodes;
int i;

for (i=0; i<slsize; i++) {
if (slist[i].sid==me)
NOfSend[slist[i].tag*nn+slist[i].rid] = slist[i].count;
}
}


#### 4.3.23 count_next_particles()

count_next_particles()  The function count next particles(), called only from make_comm_count(), scans a part
of CommList whose head and size are specified by the arguments rlist and rlsize. The
scan is to count the number of primary (p = 0) or secondary (p = 1) particles of species s
which the local node n will receive from nodes and to set the count into TotalPNext[p][s].
Therefore, when we find a S_commlist record whose rid is n and which has tag = t =
pS + s, we add its count to TotalPNext[p][s]. Note that since the one-dimensional index
of [p][s] for an array [2][S] is pS + s, that index is simply t.


static void
count_next_particles(struct S_commlist* rlist, int rlsize) {
int me=myRank, i;

for (i=0; i<rlsize; i++) {
if (rlist[i].rid==me)
TotalPNext[rlist[i].tag] += rlist[i].count;
}
}


#### 4.3.24 oh1_broadcast()

oh1_broadcast_()  The API functions oh1 broadcast () for Fortran and oh1 broadcast() for C provide a
oh1_broadcast()  simulator body calling them with a safe broadcast communications in a family. The function
oh1 broadcast() is also used in library functions schedule_particle_exchange() and
make_comm_count() to broadcast blocks in CommList[], and build_new_comm() to do it for
Neighbors[0] and NeighborsShadow[0]. The functions have the following arguments.

- The input argument pbuf is the pointer to the buffer of data which the local node
broadcast to its helpers.

- The output argument sbuf is the pointer to the buffer of data which the local node
receives from its helpand by broadcast.

- The input argument pcount is the size (number of data elements) of the data to
broadcast to the helpers.


<!-- Page 202 -->

- The input argument scount is the size (number of data elements) of the broadcasted
data to be received from the helpand. The value of this argument must be equal to
the pcount argument of corresponding call of the function in the helpand.

- The input argument ptype is the MPI data-type of the data to broadcast to the
helpers.

- The input argument stype is the MPI data-type of the broadcasted data to be received
from the helpand. The value of this argument must be equal to the ptype argument
of corresponding call of the function in the helpand.

The Fortan API oh1 broadcast () simply calls its C counterpart oh1 broadcast() which
does what we have to do, translating its ptype and stype arguments into C representation
by MPI_Type_f2c().


void
oh1_broadcast_(void* pbuf, void* sbuf, int *pcount, int *scount,
int *ptype, int *stype) {
oh1_broadcast(pbuf, sbuf, *pcount, *scount,
MPI_Type_f2c(*ptype), MPI_Type_f2c(*stype));
}
void
oh1_broadcast(void* pbuf, void* sbuf, int pcount, int scount,
MPI_Datatype ptype, MPI_Datatype stype) {


The broadcast in a family consists of a pair of MPI_Bcast(), one as the helpand
sending the data spceified by pbuf, pcount and ptype to helpers in the communicator
MyComm->prime with its rank MyComm->rank, and the other as a helper receiving the data
specified by sbuf, scount and stype from the helpand whose rank is MyComm->root in the
communicator MyComm->sec. Thus, if we performed these two boradcasting without care
about their order, e.g., first as the helpand then as a helper, they should be unnecessar-
ily serialized waiting the completions of those for bottommost families, then the second
bottommost, and so on, in this example.
Therefore we perform broadcasts in two phases with red-black ordering. That is, each
node is assigned a color red (MyComm->black = 0) or black (MyComm->black = 1) so that
a black (red) helpand has red (black) helpers. Then first we perform the broadcasts from
black helpands as roots safely to their red helpers which dedicate only receiving in this
phase. Then in the second phase, red helpands also safely broadcast to their black helpers.
Note that if a helpand does not have nothing to broadcast (pcount = 0), corresponding
broadcast is not performed because its helpers know the fact (scount = 0). Also note that
the root of the family tree does not have a helpand and thus its MyComm->sec is MPI_COMM_
NULL, while leaves does not have helpers with their MyComm->prime being MPI_COMM_NULL.

if (MyComm->black) {
if (MyComm->prime!=MPI_COMM_NULL && pcount)
MPI_Bcast(pbuf, pcount, ptype, MyComm->rank, MyComm->prime);
if (MyComm->sec!=MPI_COMM_NULL && scount)
MPI_Bcast(sbuf, scount, stype, MyComm->root, MyComm->sec);
} else {
if (MyComm->sec!=MPI_COMM_NULL && scount)
MPI_Bcast(sbuf, scount, stype, MyComm->root, MyComm->sec);
if (MyComm->prime!=MPI_COMM_NULL && pcount)


<!-- Page 203 -->

MPI_Bcast(pbuf, pcount, ptype, MyComm->rank, MyComm->prime);
}
}


#### 4.3.25 rebalance1()

rebalance1()  The function rebalance1(), called from transbound1() and rebalance2() being the level-
2 counterpart of this function, builds the new family tree to rebalance the load among nodes.
It also makes an all-to-all type particle transfer schedule to make particles in a subdomain
accommodated by the family members responsible for the subdomain, and set NOfRecv[][][]
and NOfSend[][][] according to the schedule. The function has three arguments currmode,
level and stats whose meanings are as same as those of try_primary1().


void
rebalance1(int currmode, int level, int stats) {
int nn=nOfNodes;
dint nofp=nOfParticles;
dint npavefloor=nofp/nn;
dint npfracin=nofp-npavefloor*nn, npfracout=npfracin;
dint npavein=npavefloor+(npfracin==0 ? 0 : 1), npaveout=npavein;
int ns=nOfSpecies;
int i, j, k, s, bot, pm=Mode_PS(currmode)-1, me=myRank;
struct S_node *node, *mynode=NodesNext+me, *root;


The first job of the function, besides starting time measurement and verbose messaging,
is to initialize LessHeap and GreaterHeap by emptying them and clear GreaterHeap.
index[] to state that any subdomains are not in GreaterHeap. Note that LessHeap.index[]
is never referred to.

if (stats) oh1_stats_time(STATS_REBALANCE, 0);
Verbose(2,vprint("rebalance"));

LessHeap.n = GreaterHeap.n = 0;
for (i=0; i<nn; i++) GreaterHeap.index[i] = 0;


Next we split subdomains according to their particle populations and, by push_heap(),
push subdomain n such that TotalPGlobal[n] = Pn is less than average to LessHeap and
thus make it a leaf of the family tree, and push others to GreaterHeap. Note that the
average P/N can be a non-integer and thus we examine with the ceiling of the average
⌈P/N⌉until we push (P mod N) subdomians to LessHeap, while remainders are examined
with the floor ⌊P/N⌋. We also push NodesNext[n] such that n goes to LessHeap into
NodeQueue[] so that its first members are leaves. In addtion, after copying Nodes[n] into
NodesNext[n], we initialize NodesNext[n].child to be NULL indicating no children, and, if
we are in primary mode, NodesNext[n].parentid to be −1 to indicate that n does not have
a parent currently.

for (i=0,bot=0,node=NodesNext; i<nn; i++,node++) {
dint npg=TotalPGlobal[i];
if (npg<npavein) {
if (--npfracin==0) npavein--;
push_heap(i, &LessHeap, 0);


<!-- Page 204 -->

NodeQueue[bot++] = node;
} else {
push_heap(i, &GreaterHeap, 1);
}
*node = Nodes[i];
node->child = NULL;
if (pm) node->parentid = -1;
}

Now we repeatedly pick subdomains from LessHeap by pop_heap() and thus in lightest-
first manner untill LessHeap becomes empty. The node responsible for each popped sub-
domain becomes a helper and is assigned secondary particles so that it becomes accommo-
dating the average number of particles in total. Again the average can be a non-integer
and thus first (P mod N) subdomains will be loaded with ceiling of the average ⌈P/N⌉
while remainders with floor ⌊P/N⌋. The helpand of a node n remains unchanged if we
are in secondary mode and the subdomain of n’s helpand is in GreaterHeap from which
the helpand’s subdomain is removed by remove_heap(). Otherwise, the helpand is that
having the heaviest load popped from GreaterHeap by pop_heap().  In both cases, the
node n is assigned a −Pn secondary particles, where a = ⌈P/N⌉for first (P mod N) nodes
or a = ⌊P/N⌋for remainders, and the diffence is set into NodesNext[n].get.sec = Qgetn  .
This value is not the actual numbers of particles to be received by n but it will be adjusted
according to the number of particles currently accommodated by n as its secondary particle
(maybe incidentally) by schedule_particle_exchange() afterward to possibly result in
negative one meaning to put.
Then, after linking the node n and its helpand m by making elements of NodesNext[n]
and NodesNext[m]  for  the  helpand-helper  linkage have  appropriate  values, Pm =
TotalPGlobal[m] is decremented by the number of secondary particles assigned to the
n, namely a −Pn. If the resulting amount is less than the average, which is the average for
pushing to LessHeap rather than for popping from it, the node m is pushed to LessHeap
by push_heap() and to NodeQueue[].  Otherwise, it is returned to GreaterHeap also by
push_heap().

while (LessHeap.n) {
struct S_node *parent;
dint npg;
int get, pid, h;
j = pop_heap(&LessHeap, 0);
node = NodesNext + j;
get = npaveout - TotalPGlobal[j];
if (--npfracout==0) npaveout--;
if ((k=node->parentid)>=0 && (h=GreaterHeap.index[k]))
remove_heap(&GreaterHeap, 1, h);
else
k = pop_heap(&GreaterHeap, 1);
node->get.sec = get;
parent = NodesNext + k;
node->parentid = k;  node->parent = parent;
node->sibling = parent->child;
parent->child = node;
npg = (TotalPGlobal[k] -= get);
if (npg<npavein) {
if (--npfracin==0) npavein--;
push_heap(k, &LessHeap, 0);  NodeQueue[bot++] = parent;


<!-- Page 205 -->

} else {
push_heap(k, &GreaterHeap, 1);
}
}

When we complete the helper assignment, we have at least one node remaining in
GreaterHeap. We pick the first element GreaterHeap.node[1] to make it the root of the
family tree.  If GreaterHeap has two or more elements whose particle amounts are inci-
dentally tie the root’s, we make them root’s helpers without secondary particle assignment
(i.e., Qgetn = get.sec = 0) pushing them to NodeQueue[].  Finally, the root is pushed to
NodeQueue[] as its last element.
Note that in this final root-family member addition, a leaf node may be pushed to some
midst entry (i.e., not in the topmost sequence) of NodeQueue[]. This can be happen if Pn
for a node n is accidentaly equal to ⌈P/N⌉or ⌊P/N⌋and thus is pushed into GreaterHeap
by definition. Therefore, we cannot stop a top-down traversal of the tree, i.e., tail-to-head
scan of NodeQueue[] when we find a leaf node.

root = NodesNext + GreaterHeap.node[1];
root->parentid = -1;  root->parent = root->sibling = NULL;
root->get.sec = 0;
k = root->id;
for (i=2; i<=GreaterHeap.n; i++) {
j = GreaterHeap.node[i];
node = NodesNext + j;
node->get.sec = 0;
node->parentid = k;  node->parent = root;
node->sibling = root->child;  root->child = node;
NodeQueue[bot++] = node;
}
NodeQueue[bot] = root;

Now we have the family tree whose every node n has correct settings of Qgetn   in
NodesNext[n].get.sec with respect to the number of secondary particles the node should
accommodate. From now we temporalily switches to local operations. For the local node
n, We let

∑S       ∑S
NodesNext[n].get.prime = Rgetn = Pn −     q(n)[0][s][n] = Pn −    NOfPLocal[0][s][n]
s=0                    s=0

to  represent  the number  of  primary  particles  to  be  received  using count_real_            ∑S
stay() to calculate   s=0 q(n)[0][s][n]35.  Note that we set the calculation result into
NodesNext[n].stay.prime so that  it  is referred to by a level-4p function make_recv_
list().
Then, with the setting of Rgetn  for the local node n and Qgetm  for all nodes m ∈[0, N−1]
(especially m ∈H(n)), we call schedule_particle_exchange() with the argument reb as
follows, unless Special_Pexc_Sched() is true with the argument level to mean that the
caller of rebalance1() will do its own particle exchange scheduling.

- 1 means we are in secondary mode with normal accommodataion (currmode =
MODE_NORM_SEC) and thus the old family tree kept in Nodes[]  is correct.  Thus

35We may rely on stay.prime if we were in secondary mode but we always call count real stay() for
the sake of simplicity.


<!-- Page 206 -->

schedule_particle_exchange() and sched_comm() consult the tree to find old fam-
ily menbers for neighboring regions.

- 2 means we are  in primary mode with normal accommodation (currmode =
MODE_NORM_PRI) and thus the old family tree is obsolete but particles moving to
a subdomain n should be found only in the nodes whose primary subdomain is a
neighbor of the subdomain n. Thus schedule_particle_exchange() and sched_
comm() scan these nodes as senders.

- 3 means we are in primary or secondary mode with anywhere accommodation and
thus any nodes may have particles in any subdomain (Mode_Is_Any() for currmode
is true) because of  initial particle distribution or particle injections and so on.
Thus schedule_particle_exchange() and sched_comm() scan all nodes as potential
senders.

Now we have the transfer schedule for the local node in CommList[] a part of which is
obtained by broadcast in the old family, and thus we can now create communicators for
newly created families by build_new_comm().

mynode->get.prime = TotalPGlobal[me] -
(mynode->stay.prime=count_real_stay(NOfPLocal+me));
if (Special_Pexc_Sched(level)) return;
schedule_particle_exchange(currmode==MODE_NORM_SEC ?
1 : (currmode==MODE_NORM_PRI ? 2 : 3));
build_new_comm(currmode, level, 1, stats);
}


#### 4.3.26 build_new_comm()

build_new_comm()  The function build_new_comm(), called from rebalance1() and level-4 (or higher) library
functions with their own particle exchange scheduling mechanisms, creates MPI commu-
nicators for new family tree created by rebalance1(). The arguments of this function,
except for nbridx, are exactly same as those of rebalance1().
First of all, since the old family tree is no more useful36, we exchange Nodes[] and
NodesNext[].


void
build_new_comm(int currmode, int level, int nbridx, int stats) {
int bot=nOfNodes-1, me=myRank;
struct S_node *node, *ch;
struct S_node *mynode=NodesNext+me, *root=NodeQueue[bot];
int oldparent=Mode_PS(currmode) ? Nodes[me].parentid : -1;
int i, j;
MPI_Group grpw=GroupWorld, grp;

node = Nodes;  Nodes = NodesNext;  NodesNext = node;


36If without position-aware paritcle management. If with it, the old family tree will be referred to after
the call of build new comm but anyway the tree is kept in NodesNext for the referencce.


<!-- Page 207 -->

Then we creates communicators as follows.  Each family should have its own MPI
communicator for the broadcast subdomain field data and/or that of its borders and particle
transfer schedule from its helpand to helpers, and the (all-)reduce of the current and/or
charge density of the subdomain. Since a node may belong two families, one as the helpand
and the other as a helper,  if collective communications of both families were performed
in a careless order we could have unnecessarily heavy serialization.  For example,  if all
nodes perform a collective communication as the helpand first and then do as a helper, the
communication is serialized in the bottom-up manner. Reversing the order does not help
us simply causing top-down serialization. Thus we assign one of two colors, black and red,
to families so that the color of a family is differnt from that of its direct ancenstral and
direct descendant families. By this coloring, first we perform the collective communications
of black families which have no dependencies each other, and then those of red families
without serialization, as discussed in §4.3.24.
To build the communicators and to assign colors, we traverse the helper tree in top-
down manner starting from the bottom of NodeQueue[] as done in try_stable1(), after
freeing communicators for the old tree. To create the communicator for the family rooted
by the node n visited in the i-th (i ≥0) iteration for non-leaf nodes, we gather the MPI
ranks of the family members into TempArray[] from which the MPI group of the family
is created by MPI_Group_incl() with GroupWorld, and then the MPI communicator is
created from the group by MPI_Comm_create(). Then the created communicator stored
in Comms.body[i], and Nodes[n].comm.prime and Nodes[c].comm.sec are set to i for all n’s
helper nodes c. The rank of the node n in the family communicator is obtained by MPI_
Group_translate_ranks() from n in GroupWorld, and is stored in Nodes[n].comm.rank to
use it as the root node rank for collective communications. For the family tree root node r,
Nodes[r].comm.sec is set to −1, while leaf nodes l have Nodes[l].comm.prime = −1, both
of them meaning they don’t belong to communicators as a helper or the helpand.
The color for the topmost familiy is red and thus Nodes[r].comm.black for the root node
r is 0 while Nodes[c].comm.black is 1 for all r’s helper nodes c. Then the colors are set
into Nodes[n].comm.black so that it is reversed from the Nodes[parent(n)].comm.black.

if (stats) oh1_stats_time(STATS_REB_COMM, 0);
for (i=0; i<Comms.n; i++) {
if (Comms.body[i] != MPI_COMM_NULL)
MPI_Comm_free(Comms.body+i);
}
root->comm.black = 0;  root->comm.sec = -1;
for (i=0; bot>=0; bot--) {
int black, rid;
node = NodeQueue[bot];
if (!(ch=node->child))  continue;   /* a leaf may reside below some non-
leaves in NodeQueue when its number
of primaries is equal to the
average */
node->comm.prime = i;
rid = TempArray[0] = node->id;
black = 1 - node->comm.black;
for (j=1; ch; ch=ch->sibling, j++) {
TempArray[j] = ch->id;
ch->comm.prime = -1;  ch->comm.sec = i;  ch->comm.black = black;
}
MPI_Group_incl(grpw, j, TempArray, &grp);


<!-- Page 208 -->

MPI_Group_translate_ranks(grpw, 1, &rid, grp, &(node->comm.rank));
MPI_Comm_create(MCW, grp, Comms.body+i);
MPI_Group_free(&grp);
i++;
}

Then, after letting Comms.n be the number of families (non-leaf nodes) in the tree for
the freeing operation in the next occasion, we let FamIndex[] and FamMembers[] represent
F(m) for all m ∈[0, N), which we have just built,  if oh1_families() has been called
beforehand to make these arrays non-NULL. That is, for all m ∈[0, N) we let;


m−1∑
im = FamIndex[m] =     |F(j)|
j=0
FamMembers[im] = m
{FamMembers[im+k] | k ∈{1, H(m)}} = H(m)

visiting Nodes[m] and the chain of H(m) starting from Nodes[m].child. We also let
FamMembers[2N−1] = r to show the family tree root.

Comms.n = i;

if (FamIndex) {
int *fidx = FamIndex,  *fmem = FamMembers;
int nn = nOfNodes, j;
for (i=0,j=0; i<nn; i++) {
fidx[i] = j;
fmem[j++] = i;
for (ch=Nodes[i].child; ch; ch=ch->sibling, j++)  fmem[j] = ch->id;
}
fidx[nn] = j;  fmem[j] = root->id;
}

Next, we  set MyComm elements  for the  local node n  referring to the element  of
Nodes[n].comm and Comms.body[] having its communicators. Then if the library is called
from a C-coded simulator body which needs MyComm through its shadow() MyCommC of non-
NULL, we copy MyComm into MyCommC. Similary, if the library is called from Fortran and thus
we have MyCommF of non-NULL, the elements in MyComm are copied into those in MyCommF
translating communicators into Fortran forms by MPI_Comm_c2f().

MyComm->prime =
mynode->comm.prime<0 ? MPI_COMM_NULL : Comms.body[mynode->comm.prime];
MyComm->sec =
mynode->comm.sec<0 ? MPI_COMM_NULL : Comms.body[mynode->comm.sec];
MyComm->rank = mynode->comm.prime<0 ? -1 : mynode->comm.rank;
MyComm->black = mynode->comm.black;
if ((node=mynode->parent))  MyComm->root = node->comm.rank;
else MyComm->root = -1;
if (MyCommC) *MyCommC = *MyComm;
if (MyCommF) {
MyCommF->prime = MPI_Comm_c2f(MyComm->prime);
MyCommF->sec   = MPI_Comm_c2f(MyComm->sec);
MyCommF->rank  = MyComm->rank;


<!-- Page 209 -->

MyCommF->root  = MyComm->root;
MyCommF->black = MyComm->black;
}

Next, we broadcast Neighbors[0] = DstNeighbors to the helpers of the local node
which receive  it in Neighbors[i], where  i = nbridx = 1 when the function  is called
from rebalance1(), while  it can be  i = 2 when called from a higher level function,
e.g., make_recv_list() for position-aware particle management to keep the neighbors of
the old parent.  Similarly, we broadcast NeighborsShadow[0][] to the helpers to let them
have the array elements in their NeighborsShadow[1][] after pushing their old values to
NeighborsShadow[2][], if the array is non-NULL to mean oh1_neighbors() has been called
beforehand to show the neighbor information to a simulator body.
We also let RegionId[1] = Nodes[n].parentid to notify the simulator body of the MPI
rank of the helpand through its shadow SubdomainId[1].

oh1_broadcast(Neighbors[0], Neighbors[nbridx], OH_NEIGHBORS, OH_NEIGHBORS,
MPI_INT, MPI_INT);
if (NeighborsShadow) {
int (*nb)[OH_NEIGHBORS] = NeighborsShadow;
for (i=0; i<OH_NEIGHBORS; i++)  nb[2][i] = nb[1][i];
oh1_broadcast(nb[0], nb[1], OH_NEIGHBORS, OH_NEIGHBORS, MPI_INT, MPI_INT);
}
SubdomainId[1] = RegionId[1] = mynode->parentid;


Finally, if Special_Pexc_Sched() is true for the argument level meaning the caller of
build_new_comm() has its own particle exchange scheduling mechanism, we simply finish
the function body. Otherwise, we call make_comm_count() giving its argument reb = 1
to indicate the family tree is rebuilt and passing parent(n) of the local node n in the old
family tree, if it was valid and useful, through its argument oldparent, to have NOfRecv[][][]
and NOfSend[][][] as the output of oh1_transbound() if level = 1, or for non-neighboring
particle transfers if Mode_Is_Norm() for currmode is false.

if (!Special_Pexc_Sched(level))
make_comm_count(currmode, level, 1,
(Mode_Is_Norm(currmode) ? oldparent : -1), stats);
}


#### 4.3.27 push_heap()

push_heap()  The function push heap, called only from rebalance1(), adds the element for the subdo-
main n to the heap h = heap, which is GreaterHeap if g = greater = 1 or LessHeap
otherwise, according to the number of particles in n,  i.e., Pn = TotalPGlobal[n]. The
addtion to a heap whose number of elements becomes k is done by traversing the heap tree
from its k-th node (leaf) to its root inserting the values to be added shifting those recorded
on the path downward to keep the invariant of the tree, a parent is greater/less than its
children.
The upward path from the tree node stored in h.node[k] is depicted by the binary repre-
sentaion of k, namely k(l−1) . . . k(0) where k(l−1) = 1. That is, the binary representation
of the index of its parent of k is k(l −1) . . . k(1), that of the grand parent is k(l −1) . . . k(2)
and so on to the root h.node[1] whose index is represented by k(l −1) = 1. Thus we
go up the tree from k until we find minimum i such that, for all j ∈[1, i], Pn > Pn(j)  if


<!-- Page 210 -->

g = 1 or Pn ≤Pn(j) otherwise where n(j) = h.node[k/2j], shifting h.node[k/2j] down to
h.node[k/2j+1]. (If such i is not found because h.node[k/2] does not hold the inequation
above, we let i = 0.) Then we store n into h.node[k/2i] and set h.index[n] = k/2i.


static void
push_heap(int r, struct S_heap* heap, int greater) {
int n=heap->n, *hnode=heap->node, *index=heap->index;
dint np=TotalPGlobal[r];
int m, q, g;

heap->n = ++n;
for (; n>1; n=m) {
m = n>>1;  q = hnode[m];
g = (np>TotalPGlobal[q]) ? 1 : 0;
if (g!=greater) break;
hnode[n] = q;  index[q] = n;
}
hnode[n] = r;  index[r] = n;
}


#### 4.3.28 pop_heap()

pop_heap()  The function pop heap(), called only from rebalance1(), removes the root node of heap,
which is GreaterHeap if the argument greater is 1 or LessHeap otherwise, by remove_
heap() and returns the subdomain ID which had been stored in the root node.


static int
pop_heap(struct S_heap* heap, int greater) {
int pop=heap->node[1];

remove_heap(heap, greater, 1);
return(pop);
}


#### 4.3.29 remove_heap()

remove_heap()  The function remove heap(), called from rebalance1() directly or through pop_heap(),
removes the element r = rem of the heap h = heap which is GreaterHeap if g = greater = 1
or LessHeap otherwise. The removal of h.node[r] of the heap whose number of element
becomes k −1 is done by temporalily moving h.node[k] to h.node[r] and rearranging the
subtree rooted by r so that it keeps the invariant that a parent is greater/less than its
children.
Since a node h.node[i] has two children in h.node[2i] and h.node[2i+1] (if k > 2i+1),
the rearrangement of the subtree rooted by i is peformed as follows if g = 1 (or g = 0).

(1) If 2i + 1 > k and P2i+1 = TotalPGlobal[2i+1] is the maximum (minimum) in three
members, we exchange h.node[2i+1] and h.node[i] and rearrange the subtree rooted
by 2i + 1 recursively.

(2) Otherwise, if 2i > k and P2i > Pi (P2i ≤Pi), we exchange h.node[2i] and h.node[i]
and rearrange the subtree rooted by 2i recursively.


<!-- Page 211 -->

(3) Otherwise, we complete the procedure.


static void
remove_heap(struct S_heap* heap, int greater, int rem) {
int n=heap->n, *hnode=heap->node, *index=heap->index;
int id=hnode[n];
dint np=TotalPGlobal[id];
int i;

heap->n = --n;  index[hnode[rem]] = 0;
if (rem>n) return;
for (i=rem; ; ) {
int left=(i<<1), right=left+1;
if (right<=n) {
int lid=hnode[left], rid=hnode[right];
dint lnp=TotalPGlobal[lid], rnp=TotalPGlobal[rid];
int cgl=(np>lnp)?1:0, cgr=(np>rnp)?1:0, lgr=(lnp>rnp)?1:0;
if (cgl==greater) {
if (cgr==greater) {
hnode[i] = id;  index[id] = i;  return;
} else {
hnode[i] = rid;  index[rid] = i;  i = right;
}
} else if (lgr==greater) {
hnode[i] = lid;  index[lid] = i;  i = left;
} else {
hnode[i] = rid;  index[rid] = i;  i = right;
}
} else {
if (left<=n) {
int lid=hnode[left];
int cgl=(np>TotalPGlobal[lid])?1:0;
if (cgl==greater) {
hnode[i] = id;  index[id] = i;
} else {                /* we know left node has no children. */
hnode[i] = lid;  index[lid] = i;
hnode[left] = id;  index[id] = left;
}
} else {
hnode[i] = id;  index[id] = i;
}
return;
}
}
}


#### 4.3.30 oh1_accom_mode()

oh1_accom_mode_()  The API functions oh1_accom_mode_() for Fortran and oh1_accom_mode() for C simply
oh1_accom_mode()  retuns the value of accMode to a simulator body calling them to let it know the accommo-
dation mode is normal or anywhere.


<!-- Page 212 -->

int
oh1_accom_mode_() {
return(accMode);
}
int
oh1_accom_mode() {
return(accMode);
}


#### 4.3.31 oh1_all_reduce()

oh1_all_reduce_()  The API functions oh1 all reduce () for Fortran and oh1 all reduce() for C provide
oh1_all_reduce()  a simulator body calling them with a safe all-reduce communications in a family. The
functions have the following arguments.

- The input arguments pbuf and sbuf are the pointers to the buffers of data to be
all-reduced in the primary and secondary families which the local node belongs to
respectively.

- The input arguments pcount and scount are the sizes (number of data elements) of
the data to be all-reduced in the primary and secondary families which the local node
belongs to respectively. The pcount for a helpand must be equal to scount for its
helpers.

- The input arguments ptype and stype are the MPI data-types of the data to be
all-reduced in the primary and secondary families which the local node belongs to
respectively. The ptype for a helpand must be equal to stype for its helpers.

- The input arguments pop and sop are the MPI’s reduction operators of the data to
be all-reduced in the primary and secondary families which the local node belongs to
respectively. The pop for a helpand must be equal to sop for its helpers.

The Fortan API oh1 all reduce () simply calls its C counterpart oh1 all reduce() which
does what we have to do, translating its ptype and stype arguments into C representation
by MPI_Type_f2c(), and pop and sop arguments by MPI_Op_f2c().
The function calls MPI_Allreduce() twice with MPI_IN_PLACE option, as the parent
and a child, in the way similar to oh1_broadcast() to avoid serialization.


void
oh1_all_reduce_(void *pbuf, void *sbuf, int *pcount, int *scount,
int *ptype, int *stype, int *pop, int *sop) {
oh1_all_reduce(pbuf, sbuf, *pcount, *scount,
MPI_Type_f2c(*ptype), MPI_Type_f2c(*stype),
MPI_Op_f2c(*pop), MPI_Op_f2c(*sop));
}
void
oh1_all_reduce(void *pbuf, void *sbuf, int pcount, int scount,
MPI_Datatype ptype, MPI_Datatype stype,
MPI_Op pop, MPI_Op sop) {

if (MyComm->black) {
if (MyComm->prime!=MPI_COMM_NULL)
MPI_Allreduce(MPI_IN_PLACE, pbuf, pcount, ptype, pop, MyComm->prime);


<!-- Page 213 -->

if (MyComm->sec!=MPI_COMM_NULL)
MPI_Allreduce(MPI_IN_PLACE, sbuf, scount, stype, sop, MyComm->sec);
} else {
if (MyComm->sec!=MPI_COMM_NULL)
MPI_Allreduce(MPI_IN_PLACE, sbuf, scount, stype, sop, MyComm->sec);
if (MyComm->prime!=MPI_COMM_NULL)
MPI_Allreduce(MPI_IN_PLACE, pbuf, pcount, ptype, pop, MyComm->prime);
}
}


#### 4.3.32 oh1_reduce()

oh1_reduce_()  The API functions oh1 reduce () for Fortran and oh1 reduce() for C provide a simulator
oh1_reduce()  body calling them with a safe one-way reduction communications in a family. The functions
have the following arguments.

- The input arguments pbuf and sbuf are the pointers to the buffers of data to be
reduced in the primary and secondary families which the local node belongs to re-
spectively.

- The input arguments pcount and scount are the sizes (number of data elements) of
the data to be reduced in the primary and secondary families which the local node
belongs to respectively. The pcount for a helpand must be equal to scount for its
helpers.

- The input arguments ptype and stype are the MPI data-types of the data to be
reduced in the primary and secondary families which the local node belongs to re-
spectively. The ptype for a helpand must be equal to stype for its helpers.

- The input arguments pop and sop are the MPI’s reduction operators of the data to
be reduced in the primary and secondary families which the local node belongs to
respectively. The pop for a helpand must be equal to sop for its helpers.

The Fortan API oh1 reduce () simply calls its C counterpart oh1 reduce() which does
what we have to do, translating its ptype and stype arguments into C representation by
MPI_Type_f2c(), and pop and sop arguments by MPI_Op_f2c().
The function calls MPI_Reduce() twice, as the helpand with MPI_IN_PLACE option and
as a helper, in the way similar to oh1_broadcast() to avoid serialization.


void
oh1_reduce_(void *pbuf, void *sbuf, int *pcount, int *scount,
int *ptype, int *stype, int *pop, int *sop) {
oh1_reduce(pbuf, sbuf, *pcount, *scount,
MPI_Type_f2c(*ptype), MPI_Type_f2c(*stype),
MPI_Op_f2c(*pop), MPI_Op_f2c(*sop));
}
void
oh1_reduce(void *pbuf, void *sbuf, int pcount, int scount,
MPI_Datatype ptype, MPI_Datatype stype, MPI_Op pop, MPI_Op sop) {

if (MyComm->black) {
if (MyComm->prime!=MPI_COMM_NULL)
MPI_Reduce(MPI_IN_PLACE, pbuf, pcount, ptype, pop, MyComm->rank,


<!-- Page 214 -->

MyComm->prime);
if (MyComm->sec!=MPI_COMM_NULL)
MPI_Reduce(sbuf, NULL, scount, stype, sop, MyComm->root, MyComm->sec);
} else {
if (MyComm->sec!=MPI_COMM_NULL)
MPI_Reduce(sbuf, NULL, scount, stype, sop, MyComm->root, MyComm->sec);
if (MyComm->prime!=MPI_COMM_NULL)
MPI_Reduce(MPI_IN_PLACE, pbuf, pcount, ptype, pop, MyComm->rank,
MyComm->prime);
}
}


#### 4.3.33 oh1_init_stats()

oh1_init_stats_()  The API functions oh1 init stats () for Fortran and oh1 init stats() for C initialize
oh1_init_stats()  the data structure Stats for statistics and start the first time measurement specified by
the arguments key and ps, if statsMode ̸= 0. The initialization is done by calling clear_
stats() twice for subotal and total substructures in Stats. The time measurement is
started by setting 2k + p into Stats.curr.time.key where k = key and p = ps indicating
whether the first interval is for primary (p = 0) or secondary (p = 1) execution, and
setting the current wall-clock time obtained by MPI_Wtime() into Stats.curr.time.value.
We also clear all the elements in Stats.curr.time.ev[] to indicate no time measurement
intervals have completed.
Another initialization is to create the MPI data-type T_StatsTime for the reduction on
timing statistics, as a MPI_BYTE sequence of sizeof(struct␣S_statstime) by MPI_Type_
contiguous() followed by MPI_Type_commit(). The pairwise reduction is peformed by
the function stats_reduce_time() which MPI_Op_create() bind to Op_StatsTime to be
given to MPI_Reduce(). The other reducer function is stats_reduce_part() for particle
transfer statistics, which is bound to Op_StatsPart.


void
oh1_init_stats_(int *key, int *ps) {
if (statsMode) oh1_init_stats(*key, *ps);
}
void
oh1_init_stats(int key, int ps) {
int i;

if (!statsMode)  return;
clear_stats(&Stats.subtotal);
clear_stats(&Stats.total);
Stats.curr.time.key = (key<<1) + ps;
Stats.curr.time.value = MPI_Wtime();
for (i=0; i<(STATS_TIMINGS<<1)+1; i++) Stats.curr.time.ev[i] = 0;
MPI_Type_contiguous(sizeof(struct S_statstime), MPI_BYTE, &T_StatsTime);
MPI_Type_commit(&T_StatsTime);
MPI_Op_create(stats_reduce_time, 1, &Op_StatsTime);
MPI_Op_create(stats_reduce_part, 1, &Op_StatsPart);
}


<!-- Page 215 -->

#### 4.3.34 clear_stats()

clear_stats()  This function, called from oh1_init_stats() and oh1_show_stats(), clears statistics
recorded in a S_statstotal structure specified by its argument stotal, which should
be Stats.total or Stats.subtotal. Clearing its element arrays time[2Kt] and part[Kp],
where Kt = STATS_TIMINGS and Kp = STATS_PARTS, is commonly done by letting leaf
elements max and total be zero and min be the absolute maximum value, DBL_MAX for
time[] and INT_MAX for part[]. For times[k], an additional zero-clearing is also done for
the leaf element ev to indicate that the interval corresponging to the key k is not executed.
As for part[], a special treatment is taken for keys STATS_PART_PRIMARY and STATS_PART_
SECONDARY so that their elements min is set to 0 rather than INT_MAX because they act as
counters of execution mode transition.


static void
clear_stats(struct S_statstotal *stotal) {
int i;
struct S_statstime *st = stotal->time;
struct S_statspart *sp = stotal->part;

for (i=0; i<STATS_TIMINGS<<1; i++) {
st[i].ev = 0;
st[i].min = DBL_MAX;
st[i].max = 0.0;
st[i].total = 0.0;
}
for (i=0; i<STATS_PARTS; i++) {
sp[i].min = INT_MAX;
sp[i].max = 0;
sp[i].total = 0;
}
sp[STATS_PART_PRIMARY].min = sp[STATS_PART_SECONDARY].min = 0;
}


#### 4.3.35 oh1_stats_time()

oh1_stats_time_()  The API functions oh1 stats time () for Fortran and oh1 stats time() for C finishe the
oh1_stats_time()  time measurement of an interval and start that of the next interval, if statsMode ̸= 0. The
function oh1 stats time() is also called from level-1 library functions transbound1(),
try_stable1() and rebalance1(), and level-2 functions try_primary2(), exchange_
particles(), move_to_sendbuf_primary() and move_to_sendbuf_secondary() to mea-
sure time consumed in the libraray. The other function that calls oh1 stats time() is
oh1_show_stats() but it is not for starting measurement but only for finishing that of the
last interval.
The key kf for the interval whose time measurement is to be finished is recorded in
Stats.curr.time.key and thus we calculate the time consumed in the interval as the
difference of the starting time recorded in the leaf element value of Stats.curr.time and
the current wall-time obtained by MPI_Wtime() which is then recorded into value. The
calculated time is recorded into the leaf element val[kf] while another leaf element ev[kf]
is turned to 1 to indicate the interval is executed in the current simulation step.
Finally, to start the measurement of the new interval specified by the arguments key =
ks and p = ps, we store the key for the interval 2ks + p where p indicate whether the new
interval is for primary (p = 0) or secondary (p = 1) execution.


<!-- Page 216 -->

void
oh1_stats_time_(int *key, int *ps) {
if (statsMode)  oh1_stats_time(*key, *ps);
}
void
oh1_stats_time(int key, int ps) {
double t;
int k=Stats.curr.time.key;

if (!statsMode)  return;
t = MPI_Wtime();
Stats.curr.time.val[k] = t - Stats.curr.time.value;
Stats.curr.time.ev[k] = 1;
Stats.curr.time.value = t;
Stats.curr.time.key = (key<<1) + ps;
}


#### 4.3.36 stats_primary_comm()

stats_primary_comm()  The function stats primary comm(), called only from try_primary1() when it finds the
next simulation step is executed in primary mode, calculates the local statistics of particle
transfer to update Stats.curr.part[] by scanning NOfPrimaries[][][] and NOfPLocal[][][]
by stats_comm().  Then the element Stats.curr.part[STATS_PART_PRIMARY] is set to
either 1, 2 or 3, depending on we were in primary (1) or secondary (2) mode with normal
accommodation, or anywhere accommodataion (3).


static void
stats_primary_comm(int currmode) {
stats_comm(NOfPrimaries, NOfPLocal, Stats.curr.part, nOfSpecies*2);
Stats.curr.part[STATS_PART_PRIMARY] =
(currmode==MODE_ANY_PRI) ? 3 : Mode_PS(currmode)+1;
}


#### 4.3.37 stats_secondary_comm()

stats_secondary_comm()  The function stats secondary comm(),  called only from make_comm_count(),  calcu-
lates the  local  statistics  of  particle  transfer to update Stats.curr.part[] by scan-
ning at first NOfRecv[0][][] and NOfSend[0][][] by stats_comm() for primary particles,
and then by scanning NOfRecv[1][][] and NOfSend[1][][] for secondary particles.  Then
Stats.curr.part[STATS_PART_SECONDARY] is set to either 1, 2 or 3, depending on we were
in primary (1) mode, secondary mode without rebalancing (2), or with that (3).


static void
stats_secondary_comm(int currmode, int reb) {
int ns=nOfSpecies, nnns=nOfNodes*ns;

stats_comm(NOfRecv, NOfSend, Stats.curr.part, ns);
stats_comm(NOfRecv+nnns, NOfSend+nnns,
Stats.curr.part+STATS_PART_MOVE_SEC_MIN, ns);
Stats.curr.part[STATS_PART_SECONDARY] =


<!-- Page 217 -->

Mode_PS(currmode) ? (reb ? 3 : 2) : 1;
}


#### 4.3.38 stats_comm()

stats_comm()  The function stats comm(), called from stats_primary_comm() or stats_secondary_
comm(), calculates the local statistics of particle transfer to update scp[] = σ[] which
points Stats.curr.part[0] for the call from stats primary part() or the first call from
stats secondary comm(), or Stats.curr.part[STATS_PART_MOVE_SEC_MIN] for the the
second call from stats secondary comm(). The calculation is done by scanning argument
arrays nrecv[S′][N] and nsend[S′][N], where S′ = ns, which are set by callers as follows.
- When called from stats_primary_comm(), nrecv[S′][N] = NOfPrimaries[2][S][N]
and nsend[S′][N] = NOfPLocal[2][S][N] because S′ = 2S.

- When  called from stats_secondary_comm()  as  its  first  call,  nrecv[S′][N] =
(NOfRecv[0])[S][N] and nsend[S′][N] = (NOfSend[0])[S][N] because S′ = S.

- When called from stats_secondary_comm() as  its second  call, nrecv[S′][N] =
(NOfRecv[1])[S][N] and nsend[S′][N] = (NOfSend[1])[S][N] because S′ = S.

Then we define recv(m) and send(m) for the local node n as the numbers of particles
receiving from and sending to the node m.

S′−1∑                           S′−1∑
recv(m) =     nsend[s][m]    send(m) =     nrecv[s][m]

s=0                           s=0

With the definitions above, we update the statistics σ[k] as follows.

- The elements σ[k] for k = STATS PART MOVE PRI x (x ∈{MIN, MAX, AVE}) are set to
the followings.

σ[STATS_PART_MOVE_PRI_MIN] = min{recv(m) | m ̸= n, recv(m) > 0}
σ[STATS_PART_MOVE_PRI_MAX] = max{recv(m) | m ̸= n, recv(m) > 0}
σ[STATS_PART_MOVE_PRI_AVE] = |{recv(m) | m ̸= n, r(m) > 0}|

That is, they are the minimum and maximum numbers of particles the local node
receives from other nodes, and the number of nodes from which the local node receives
some (non-zero) particles.

- The elements σ[k] for k = STATS PART GET PRI x (x ∈{MIN, MAX}) and k =
STATS_PART_PG_PRI_AVE are commonly set to the following get, the total number
of particles the local node n receives from other nodes.

N−1∑
get =    recv(m) −recv(n)

m=0


- The elements σ[k] for k = STATS PART PUT PRI x (x ∈{MIN, MAX}) are commonly set
to the following put, the total number of particles the local node n sends to other
nodes.
N−1∑
put =    send(m) −send(n)

m=0


<!-- Page 218 -->

Note that the element σ[STATS PART y PRI x] corresponds to Stats.curr.part[STATS
PART y SEC x] when this function is called from stats_secondary_comm() as its second
call, because σ[] points Stats.curr.part + STATS_PART_MOVE_SEC_MIN.  Also note that
the local staticstics above will be gathered (reduced) from all nodes by update_stats().


static void
stats_comm(int* nrecv, int* nsend, dint* scp, int ns) {
int i, s, nn=nOfNodes, me=myRank;
int get=0, put=0, minmove=INT_MAX, maxmove=0, nmove=0;

for (i=0; i<nn; i++) {
int g=0, p=0, *npr=nrecv, *nps=nsend;
for (s=0; s<ns; s++,npr+=nn,nps+=nn) {
g += nrecv[i];                            /* nrecv[s][i] */
p += nsend[i];                            /* nsend[s][i] */
}
if (i!=me) {
get += g;  put += p;
if (g>0) {
if (g<minmove) minmove = g;
if (g>maxmove) maxmove = g;
nmove++;
}
}
if (minmove>maxmove) minmove = 0;
scp[STATS_PART_MOVE_PRI_MIN] = minmove;
scp[STATS_PART_MOVE_PRI_MAX] = maxmove;
scp[STATS_PART_MOVE_PRI_AVE] = nmove;
scp[STATS_PART_GET_PRI_MIN] = scp[STATS_PART_GET_PRI_MAX]
= scp[STATS_PART_PG_PRI_AVE] = get;
scp[STATS_PART_PUT_PRI_MIN] = scp[STATS_PART_PUT_PRI_MAX] = put;
}
}


#### 4.3.39 oh1_show_stats()

oh1_show_stats_()  The API functions oh1 show stats () for Fortran and oh1 show stats() for C at first
oh1_show_stats()  finishes the last interval of execution time measurement by oh1_stats_time() giving it
a dummy interval key STATS_TIMINGS and then performs a barrier synchronization by
MPI_Barrier(), if statsMode ̸= 0. Then, if statsMode = 2 ordering periodical statistics
printing, update the statistics recorded in Stats.subtotal by update_stats() giving it
arguments including currmode of this function’s own. Then, if we reach the end of period
specified by r = reportIteration, i.e., (step mod r) = 0 with the argument step having
the current simulation step number, we print statistics by print_stats() before clear the
statistics by clear_stats().  Finally, regardless of the value of statsMode, we update
total staticstics data recorded in Stats.total by update_stats() and do MPI_Barrier()
again.


void
oh1_show_stats_(int *step, int *currmode) {
if (statsMode)  oh1_show_stats(*step, *currmode);


<!-- Page 219 -->

}
void
oh1_show_stats(int step, int currmode) {

if (!statsMode)  return;
oh1_stats_time(STATS_TIMINGS,0);
MPI_Barrier(MCW);
if (statsMode==2) {
update_stats(&Stats.subtotal, step, currmode);
if (step%reportIteration == 0) {
print_stats(&Stats.subtotal, step, reportIteration);
clear_stats(&Stats.subtotal);
}
}
update_stats(&Stats.total, step, currmode);
MPI_Barrier(MCW);
}


#### 4.3.40 Macro Round()

Round()  The macro Round(), used in update_stats() and print_stats(), divides the numerator
N = NUM by the denominator D = DEN and round the quotient to have
⌊N + ⌊D/2⌋ ⌋
round(N/D) = ⌊N/D + 0.5⌋=
D

whose correctness is proved as follows. Let Q and R be the followings.
⌊N + ⌊D/2⌋ ⌋
Q =          R = N + ⌊D/2⌋−QD
D

That is R is the remainder of the integer division. If R ≥⌊D/2⌋, let r be R −⌊D/2⌋≥0
to have N = QD + r, Q = ⌊N/D⌋and r + ⌊D/2⌋= R < D, and thus Q = ⌊N/D⌋=
round(N/D).  Otherwise, let r be R −⌊D/2⌋+ D, being r ≥0 but r < D, to have
N = (Q −1)D + r, Q −1 = ⌊N/D⌋and r + ⌊D/2⌋= R + D ≥D, and thus Q =
⌊N/D⌋+ 1 = round(N/D). Note that the macro gives 0 if D = 0 without division.


#define Round(NUM,DEN) (DEN ? (NUM+(DEN>>1))/DEN : 0)



#### 4.3.41 update_stats()

update_stats()  The function update stats(), called only from oh1_show_stats(), updates the statis-
tics data in Stats.total or Stats.subtotal specified by the argument stotal with the
measured values of the current iteration recorded in Stats.curr.  The update of the
array element time[k] for an event of key k  is done only when the flag for the event
Stats.curr.time.ev[k] = 1 indicating the event has occurred in the current step. The
flag is then cleared if stotal = Stats.total meaning that the update for the event has
completed for both Stats.subtotal and Stats.total.
On the other hand, the particle transfer statistics in part[] is not updated if step ≤0
meaning, e.g., oh1_show_stats() is called outside simulation loop.  Otherwise, first we


<!-- Page 220 -->

reduce the local statistics in all nodes by MPI_Reduce() which calls stats_reduce_part()
through Op_StatsPart, if this function is invoked as the first call in oh1_show_stats(),
i.e., statsMode = 1 meaning this function is called only once, or stotal ̸= Stats.total
meaning the first call with Stats.subtotal. Then, if the local node rank is 0, we calculate
the average number of particle transfer among send/receive pairs and among nodes for
keys updating Stats.curr.part[k] = σp[k] for k = kp = STATS PART MOVE x AVE and
k = kt = STATS PART PG x AVE where x ∈{PRI, SEC}, referring to themselves and using
Round().  That  is, since the reduction results in that the total number of transferred
particles in σp[kt] and the number of send/receive pairs in σp[kp], we update them as
σp[kp] ←σp[kt]/σp[kp] and σp[kt] ←σp[kt]/N. Then we update the first half of part[] for
primary particles which is always significant, while do the second half for secondaries when
we are now in secondary mode, i.e. (currmode mod 2) = 1. Note that σp[STATS PART y]
(y ∈{PRIMARY, SECONDARY}) have the code of the previous mode rather than the number
of transferred particles. Thus we simply increment min, max or total element of their
counterparts in part[] according to the code being 1, 2 or 3 to sum up the number of each
occasion.


static void
update_stats(struct S_statstotal *stotal, int step, int currmode) {
int i, j, k, ev, nn=nOfNodes;
int evclr = stotal==&Stats.total, reduce = statsMode==1 || !evclr;
struct S_statstime *st = stotal->time;
struct S_statspart *sp = stotal->part;
int pm=Mode_PS(currmode)-1;
int transkey=pm?STATS_PART_PRIMARY:STATS_PART_SECONDARY;
dint trans=Stats.curr.part[transkey];

for (i=0; i<STATS_TIMINGS<<1; i++) {
if ((ev=Stats.curr.time.ev[i])) {
double t = Stats.curr.time.val[i];
st[i].ev++;
if (t<st[i].min) st[i].min = t;
if (t>st[i].max) st[i].max = t;
st[i].total += t;
if (evclr) Stats.curr.time.ev[i] = 0;
}
}
if (step<=0) return;
if (myRank!=0) {
if (reduce)
MPI_Reduce(st, NULL, STATS_PART_PRIMARY, MPI_LONG_LONG_INT, Op_StatsPart,
0, MCW);
return;
}
if (reduce)
MPI_Reduce(MPI_IN_PLACE, st, STATS_PART_PRIMARY, MPI_LONG_LONG_INT,
Op_StatsPart, 0, MCW);
for (i=0,j=0; i<(pm?1:2); i++,j+=STATS_PART_MOVE_SEC_MIN) {
dint *scp=Stats.curr.part+j;
struct S_statspart *spps=sp+j;
if (reduce) {
scp[STATS_PART_MOVE_PRI_AVE] = Round(scp[STATS_PART_PG_PRI_AVE],
scp[STATS_PART_MOVE_PRI_AVE]);
