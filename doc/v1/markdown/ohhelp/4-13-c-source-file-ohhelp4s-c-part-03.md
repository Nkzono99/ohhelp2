# 4.13 C Source File ohhelp4s.c - Part 3

Source: `doc/v1/original/ohhelp.pdf`, pages 526-553.

<!-- Page 526 -->

dst = npg[g];\
if (dst==0)  { ACT; }\
else if (!PIL) {\
if (TOSB)  sb[NOfSend[dst-1]++] = *P;\
}\
else if (dst>0)\
sb[NOfSend[Pillar_Lower(dst)-1]++] = *P;\
else {\
ACT;\
const dint bsbidx = -dst;\
if (!Is_Pillar_Voxel(bsbidx)) {\
BoundarySendBuf[bsbidx-1] = *P;  npg[g] = dst - 1;\
}\
else {\
BoundarySendBuf[Pillar_Lower(bsbidx)-1] =\
BoundarySendBuf[Pillar_Upper(bsbidx)-1] = *P;\
npg[g] = dst - (Add_Pillar_Voxel(1) + 1);\
}\
}\
}


#### 4.13.38 move_to_sendbuf_4s()

move_to_sendbuf_4s()  The function move_to_sendbuf_4s(), called solely from exchange_particles4s() when
it finds Qn + P nsend > Plim or we have helpand-helper reconfiguration to mean we have to
transfer particles among neighbors before sorting, is the level-4s counterpart of move_to_
sendbuf_sec4p() shown in §4.10.41, to move particles to be sent to SendBuf[] and pack
those to stay in the local nodes in Particles[]. The function is literally very similar to the
counterpart but has the following differences from it.

- Since exchange_particles4s() works not only in the case that we will be in sec-
ondary mode but also in primary mode, this function is called both cases as well.
Therefore, the function is given nextmode = p′n being 0 or 1 to indicate the mode we
will be in. However, the mode in the next step does not affect the mechanism of the
function almost at all, because data structures referred to in this function have values
consistent with primary mode in the next step as discussed in §4.13.14 and thus the
mechanism can depend almost only on the mode in the last step. Therefore, p′n is
used only as an argument of oh1_stats_time() and set_sendbuf_disps4s().

- We need another mode indicator argument for the next step, namely psnew = pn ∈
{0, 1}, being 1 iffthe local node will have secondary subdomain and thus secondary
particles. This argument is used when we scan NOfRecv[p][][] for p ∈{0, pn} to know
the size of rbuf (p, s) in pbuf i(p, s) for each s ∈[0, S) so that InteriorParts[p][s].size
has it as the initial value.

- Since we have no hot-spots but halo particles, what Move_Or_Do() used in the function
does is significantly different from its level-4p counterpart as discussed in §4.13.37,
though the literal difference is just that the macro has an additional and last argument
η being 0 in this function to mean that copying halo particles to BoundarySendBuf[]
does not take place in this function (but in sort_particles()).


<!-- Page 527 -->

- The funcitons set_sendbuf_disps4s(), move_to_sendbuf_uw4s() and move_to_
sendbuf_dw4s() called in this function have level-4s specific names and small dif-
ferences from their level-4p counterparts.

new }, we simply passes   • Since the argument nacc[2] has {Qnn, Qn} instead of {Qnn, Qnpn
nacc[1] to the second call of move_to_sendbuf_dw4s() without calculating Qn.

On the other hand, the arguments other than nextmode, psnew and nacc are equivalent
to those of the counterpart as follows; psold = pc = 1 if the local node had a secondary
subdomain or 0 otherwise; trans = t = 1 iffwe have transitional state of helpand-helper
configuration; oldp = npold being the local node n’s helpand in the last step; nsend = Pnsend ;
and stats ̸= 0 if we have to start new timing measurement. The reason why npold is given
instead of npnew is also as same as that shown in §4.10.41 for the counterpart.


static void
move_to_sendbuf_4s(const int nextmode, const int psold, const int psnew,
const int trans, const int oldp, const int *nacc,
const int nsend, const int stats) {
const int me=myRank, ns=nOfSpecies, nn=nOfNodes, sbase=specBase;
const int ninj=nOfInjections, nplim=nOfLocalPLimit;
int ps, s, t, i;
int *nofr;
int ninjp=0, ninjs=nplim;
struct S_particle *sb = SendBuf,  *p;
Decl_Grid_Info();


As done in the level-4p counterpart, after starting the new timing measurement with
the key STATS_TB_MOVE if required by stats ̸= 0, we call set_sendbuf_disps4s() to build
the index array of sbuf (p, s, m) in NOfSend[p][s][m] for m ∈RealDstNeighbors[t][p] with
p ∈{0, p′n} giving it p′n and t as its argument. The argument p′n (not pn) is level-4s’s
own to keep the function from working on NOfSend[1][][] referring to meaningless values in
RealDstNeighbors[0][1]98.
Next, as a very level-4s’s own operation, we let InteriorParts[p][s].size have the
following for all p ∈{0, 1} and s ∈[0, S).

NS(p) = RealSrcNeighbors[t][p].nbor[]
 ∑
        NOfRecv[p][s][m]  p ≤pn
InteriorParts[p][s].size =   m∈Ns(p)                     
0                      p > pn

That is InteriorParts[p][s].size is let have the size of rbuf (p, s), which is 0 for p = 1
if pn = 0. This value is the base of the size of pbuf i(p, s) being added to the number of
particles staying and thus being packed into pbuf i(p, s) by move_to_sendbuf_uw4s() and
move_to_sendbuf_dw4s().
Then, again as done in the counterpart, we scan all injected particles invoking Move_
Or_Do() for each to move those staying in the local node to the tail region of SendBuf[]
temporalily, and to do others to corresponding sbuf (p, s, m) in SendBuf[]. Since we tell the
macro that halo particles are not taken care of in this function by η = 0, the mechanism
of this part is almost equivalent to the level-4p counterpart for particles in non-hot-spot

98Though letting the function work on p = 1 is safe.


<!-- Page 528 -->

grid-voxels. The difference is, however, that we need to enlarge rbuf (p, s) by increasing
InteriorParts[p][s].size for each injected-and-staying particle because it will be moved
back into the buffer.  Note that we can be unaware the possiblity that we have some
secondary injected particles and turn the mode from secondary to primary, because, if so,
any NOfPGrid[][][] for them tell us that they should send to other node, old helpand in fact
and thus definitely we will have none of them at the tail of SendBuf[], as happening in the
case of helpand-helper reconfiguration changing helpand.

if (stats) oh1_stats_time(STATS_TB_MOVE, nextmode);
set_sendbuf_disps4s(nextmode, trans);

for (ps=0,t=0,nofr=NOfRecv; ps<2; ps++) {
const int nnbr = RealSrcNeighbors[trans][ps].n;
const int *rnbr = RealSrcNeighbors[trans][ps].nbor;
if (ps<=psnew) {
for (s=0; s<ns; s++,t++,nofr+=nn) {
int n, nrec;
for (n=0,nrec=0; n<nnbr; n++)  nrec += nofr[rnbr[n]];
InteriorParts[t].size = nrec;
}
} else
for (s=0; s<ns; s++,t++)  InteriorParts[t].size = 0;
}
for (i=0,p=Particles+totalParts; i<ninj; i++,p++) {
const int s = Particle_Spec(p->spec-sbase);
const OH_nid_t nid = p->nid;
const int ps = Secondary_Injected(nid) ? 1 : 0;
dint *npg = NOfPGrid[ps][s];
if (nid<0) continue;
if (ps) {
Primarize_Id_Only(p);
Move_Or_Do(p, ps, oldp, 1,
(sb[--ninjs]=*p, InteriorParts[ns+s].size++), 0);
} else
Move_Or_Do(p, ps, me, 1,
(sb[nsend+ninjp++]=*p, InteriorParts[s].size++), 0);
}

The next part is almost logically equivalent to that of the level-4p counterpart for the
case the local node does not have any hot-spots in its subdomains, though the code is a
little bit different as discussed at the beginning of this section. That is, we call move_
to_sendbuf_uw4s() and move_to_sendbuf_dw4s() once or twice for each according to pc
being 0 or 1 respectively, to pack particles to stay in the local node in Particles[], to move
others to SendBuf[], and, as a level-4s’s own operation, to let InteriorParts[p][s] has the
index and size of pbuf i(p, s). In addition, if pc = 0 we let RecvBufBases[1][s] point the
head of pbuf (1, s) and, as another level-4s’s own operation, let InteriorParts[1][s].head
have its index so that rbuf (1, s) = pbuf i(1, s) because its size element has |rbuf (1, s)|,
knowing that this is meaningless with pn = 0 but safe because they will not be referred to
(and have meaningful common vaules Qnn = Qn).

move_to_sendbuf_uw4s(0, me, 0, 0);
if (psold) {
move_to_sendbuf_uw4s(1, oldp, primaryParts, nacc[0]);
move_to_sendbuf_dw4s(1, oldp, totalParts, nacc[1]);


<!-- Page 529 -->

} else {
struct S_particle *rbb=Particles+nacc[0];
int s;
for (s=0; s<ns; s++) {
RecvBufBases[ns+s] = rbb;  InteriorParts[ns+s].head = rbb - Particles;
rbb += TotalPNext[ns+s];
}
}
move_to_sendbuf_dw4s(0, me, primaryParts, nacc[0]);


The last part, to move back injected and staying particles from SendBuf[] to Particles[]
and to let primaryParts and its shadow pointed by secondaryBase be Qnn in the next step,
is literally and logically equivalent to that of the level-4p counterpart.

for (i=0,p=SendBuf+nsend; i<ninjp; i++,p++)
*(RecvBufBases[Particle_Spec(p->spec-sbase)]++) = *p;
for (i=ninjs,p=SendBuf+ninjs; i<nplim; i++,p++)
*(RecvBufBases[Particle_Spec(p->spec-sbase)+ns]++) = *p;

primaryParts = *secondaryBase = nacc[0];
}


#### 4.13.39 move_to_sendbuf_uw4s()

move_to_sendbuf_uw4s()  The function move_to_sendbuf_uw4s(), called solely from move_to_sendbuf_4s() once
or twice, scans particles in the local node n’s primary (ps = p = 0) or secondary (p = 1)
subdomain mysd = m from Particles[b−0 ] where b−0 = cbase argument for pbuf (p, 0)
in the last step.  It moves scanned particles to be sent to other nodes to SendBuf[], and
packs those to stay in the local node upward, i.e., to the direction of smaller indices of
Particles[] and to the region beginning Particles[b+0 ] where b+0 = nbase argument for
the packed pbuf (p, 0).
This function is almost logically equivalent to no hot-spot case of its level-4p counterpart
move_to_sendbuf_uw4p() shown in §4.10.42 but has the following differences from it, where
b−s = b−s−1 + TotalP[p][s−1] and b+s = b+s−1 + TotalPNext[p][s−1].

- In the cases of b+s ≤b−s or b+s+1 ≤b−s+1, i.e. the cases in which old pbuf (p, s) is scanned
by this function, we let InteriorParts[p][s].head = b+s being the head index of new
pbuf (p, s) because at its top pbuf i(p, s) is placed. We also add the number of packed
staying particles to InteriorParts[p][s].size which had the size of rbuf (p, s) placed
at the bottom of pbuf i(p, s).
- The case of b+s > b−s ∧b+s+1 > b−s+1 is eliminated from this function because we can
do nothing for the case including the placement of rbuf (p, s) which is now placed at
the top of pbuf i(p, s) rather than that of pbuf (p, s).

- For all four invocations of the macro Move_Or_Do() in the loop body, we add the
last argument η = 0 to mean halo particles are not taken care of in this function.
Therefore, in the first invocation with b+s ≤b−s , and the third invocation scanning the
bottom half of pbuf (p, s) with b+s > b−s and b+s+1 ≤b−s+1, the macro works equivalently
to no hot-spot case of its level-4p counterpart.


<!-- Page 530 -->

- In the case of b+s > b−s and b+s+1 ≤b−s+1, we give µ = 0 to the macro Move_Or_Do() in
the first scan of the top half of pbuf (p, s), and then give µ = 1 in the second scan. This
means that the first scan just counts the number of staying particles without moving
any particles, while the second scan moves them to SendBuf[] or in Particles[] for
packing.  Since there are no hot-spots, the fate of all particles in a grid-voxel are
common and thus the operations are correct.


static void
move_to_sendbuf_uw4s(const int ps, const int mysd, const int cbase,
const int nbase) {
const int ns=nOfSpecies;
const int nsor0 = ps ? ns : 0;
const int *ctp = TotalP + nsor0,  *ntp = TotalPNext + nsor0;
struct S_interiorp *ip = InteriorParts + nsor0;
struct S_particle *p,  **rbb = RecvBufBases + nsor0,  *sb = SendBuf;
int s, c, d, cn, dn;
Decl_Grid_Info();

for (s=0,c=cbase,d=nbase; s<ns; s++,c=cn,d=dn) {
dint *npg = NOfPGrid[ps][s];
cn = c + ctp[s];  dn  = d + ntp[s];
ip[s].head = d;
if (d<=c) {
for (p=Particles+c; c<cn; c++,p++)
Move_Or_Do(p, ps, mysd, 1, (Particles[d++]=*p), 0);
rbb[s] = Particles + d;  ip[s].size += d - ip[s].head;
} else if (dn<=cn) {
const int cb = c;
int cm, dm;
for (p=Particles+c; c<d; c++,p++)
Move_Or_Do(p, ps, mysd, 0, (d++), 0);
cm = c - 1;  dm = d - 1;
for (p=Particles+c; c<cn; c++,p++)
Move_Or_Do(p, ps, mysd, 1, (Particles[d++]=*p), 0);
rbb[s] = Particles + d;  ip[s].size += d - ip[s].head;
for (c=dm,d=dm,p=Particles+c; c>=cb; c--,p--)
Move_Or_Do(p, ps, mysd, 1, (Particles[d--]=*p), 0);
}
}
}


#### 4.13.40 move_to_sendbuf_dw4s()

move_to_sendbuf_dw4s()  The function move_to_sendbuf_dw4s(), called solely from move_to_sendbuf_4s() once
or twice, scans particles in the local node n’s primary (ps = p = 0) or secondary (p = 1)
subdomain mysd = m from Particles[b−S −1] where b−S = ctail argument for the element
following pbuf (p, S−1) in the last step. It moves scanned particles to be sent to other nodes
to SendBuf[], and packs those to stay in the local node downward, i.e., to the direction of
greater indices of Particles[] and to the region ending Particles[b+S −1] where b+S = ntail
argument for the element following the packed pbuf (p, S−1).
This function is almost logically equivalent to no hot-spot case of its level-4p counterpart
move_to_sendbuf_dw4p() shown in §4.10.43 but has the following differences.


<!-- Page 531 -->

- We give η = 0 to the macro Move_Or_Do() in the particle scanning loop telling it that
halo particles are not taken care of. Therefore, the function is logically equivalent to
no hot-spot case of the counterpart.

- After  the  partcile  scanning  loop  for  pbuf (p, s) from Particles[b−s+1 −1]  to
Particles[b−s ] and  to move  those  staying  into  pbuf i(p, s) whose  tail  is  at
Partilce[b+s+1 −1], we let

InteriorParts[p][s].head = i −|rbuf (p, s)|
InteriorParts[p][s].size = b+s+1 −i + |rbuf (p, s)|
RecvBufBases[p][s] = Particles + i −|rbuf (p, s)|

where |rbuf (p, s)| is kept in InteriorParts[p][s].size until its update above, i is
the index of Particles[] for the last particle moved into pbuf i(p, s), b−s = b−s+1 −
TotalP[p][s+1] and b+s = b+s+1 −TotalPNext[p][s+1], so that pbuf i(p, s) is placed at
the bottom of pbuf (p, s) and rbuf (p, s) at the top of pbuf i(p, s) (rather than the top
of pbuf (p, s)).


static void
move_to_sendbuf_dw4s(const int ps, const int mysd, const int ctail,
const int ntail) {
const int ns=nOfSpecies;
const int nsor0 = ps ? ns : 0;
const int *ctp = TotalP + nsor0,  *ntp = TotalPNext + nsor0;
struct S_interiorp *ip = InteriorParts + nsor0;
struct S_particle *sb = SendBuf,  *p,  **rbb = RecvBufBases + nsor0;
int s, c, d, cn, dn;
Decl_Grid_Info();

cn = ctail;  dn = ntail;
for (s=ns-1,c=cn-1,d=dn-1; s>=0; s--,c=cn-1,d=dn-1) {
dint *npg = NOfPGrid[ps][s];
const int dd = d;
cn -= ctp[s];  dn -= ntp[s];
if (c>=d || cn>=dn)  continue;
for (p=Particles+c; c>=cn; c--,p--)
Move_Or_Do(p, ps, mysd, 1, (Particles[d--]=*p), 0);
ip[s].head = d - ip[s].size + 1;  ip[s].size += dd - d;
rbb[s] = Particles + ip[s].head;
}
}


#### 4.13.41 Macro Sort_Particle()

Sort_Particle()  The  level-4s’s own macro Sort Particle(π), used  in sort_particles() and sort_
received_particles(), moves  local node n’s primary (p =  0) or secondary (p =
1)  particle π  of  species  s  at  g =  Grid Position(π.nid)  in  n’s  subcuboid  to
SendBuf[NOfPGridTotal[p][s][g]] for sorting and increments NOfPGridTotal[p][s][g] for the
next particle in g.  Then  if v = NOfPGrid[p][s][g] < 0 to mean g  is in a interior halo
plane, π  is copied to BoundarySendBuf[i]  if v = −(i + 1) > −232 incrementing i by
decrementing v, or to BoundarySendBuf[i] and BoundarySendBuf[j] otherwise to mean


<!-- Page 532 -->

v = −(i + 1) · 232 −(j + 1) ≤−232 incrementing i and j by subtracting 232 + 1 from v,
as done in Move_Or_Do(). Note that NOfPGridTotal[p][s][] and NOfPGrid[p][s][] are given
through implicit arguments npgt and npg.


#define Sort_Particle(P) {\
const int g = Grid_Position(P->nid);\
const dint dst = npg[g];\
SendBuf[npgt[g]++] = *P;\
if (dst<0) {\
const dint bsbidx = -dst;\
if (!Is_Pillar_Voxel(bsbidx)) {\
BoundarySendBuf[bsbidx-1] = *P;  npg[g] = dst - 1;\
}\
else {\
BoundarySendBuf[Pillar_Lower(bsbidx)-1] =\
BoundarySendBuf[Pillar_Upper(bsbidx)-1] = *P;\
npg[g] = dst - (Add_Pillar_Voxel(1) + 1);\
}\
}\
}


#### 4.13.42 sort_particles()

sort_particles()  The function sort_particles(), called solely from exchange_particles4s(), moves par-
ticles in Particles[] to SendBuf[] with sorting after non-position-aware particle transfer
due to Qn + Pnsend > Plim or helpand-helper reconfiguration. The function is almost logi-
cally equivalent to its level-4p counterpart shown in §4.10.37, but literally different from it
significantly because of the followings.

- The caller is solely exchange_particles4s() regardless of the execution mode in the
next step, and the index array for sorting is always NOfPGridTotal[][][].

- The role changing of NOfPGridTotal[][][] from per-grid histogram to per-grid index
has been done in exchange_particles4s() and thus the code for it is eliminated
from this function.

- Since we cannot scan whole of pbuf (p, s) because it includes halo particles not yet re-
ceived, we scan its subset pbuf i(p, s) of non-halo ones referring to InteriorParts[p][s]
for its head index and size.

- Since we have to take care of particles in interior halo planes, we use Sort_Particle()
to move a particle.

The function is given nextmode being 0 or 1 according to the mode in the next step is
primary or secondary respectively for timing measurement, pn = psnew ∈{0, 1} being 1 iff
the local node n has secondary subdomain in the next step, and stats ̸= 0 iffwe have to
start new timing measurement.
After starting the new timing measurement with the key STATS_TB_SORT if required
by stats ̸= 0, we scan all particles in pbuf i(p, s) (not pbuf (p, s)), whose head index and
size are in InteriorParts[p][s], for all p ∈{0, pn} and s ∈[0, S). For each particle, we
invoke Sort_Particle() for each of them to move it to SendBuf[] with sorting. Note that
NOfPGrid[p][s][] and NOfPGridTotal[p][s][] are passed to the macro implicitly.


<!-- Page 533 -->

static void
sort_particles(const int nextmode, const int psnew, const int stats) {
const int ns=nOfSpecies;
struct S_particle *p;
int ps, s, t, i;
Decl_Grid_Info();

if (stats) oh1_stats_time(STATS_TB_SORT, nextmode);
for (ps=0,t=0; ps<=psnew; ps++) {
for (s=0; s<ns; s++,t++) {
dint *npg = NOfPGrid[ps][s], *npgt = NOfPGridTotal[ps][s];
const int ips = InteriorParts[t].size;
for (i=0,p=Particles+InteriorParts[t].head; i<ips; i++,p++)
Sort_Particle(p);
}
}
}


#### 4.13.43 move_and_sort()

move_and_sort()  The function move_and_sort(), called solely from exchange_particles4s() when it finds
Qn + Pnsend ≤Plim without helpand-helper reconfiguration to mean we can move particles
staying in and leaving from the local node together from Particles[] to SendBuf[] with
sorting. It is given the following arguments; nextmode = p′n being 0 or 1 according to the
mode in the next step being primary or secondary respectively; psold = pc = 1 iffthe
local node had secondary particles; psnew = pn = 1 iffit will have secondary particles;
oldp = npold being the local node n’s helpand in the last step; nacc[2] = {Qnn, Qn}; and
stats ̸= 0 iffwe have to start new timing measurement. The reason why this function
needs to have npold instead of npnew is same as what we discussed in §4.10.41.
The function is almost logically equivalent to its level-4p counterpart move_and_sort_
secondary() shown in §4.10.44, but literally different from it somewhat because of the
followings.

- The caller exchange_particles4s() works not only when we will be in secondary
mode in the next step but also in primary mode, this function works with both modes
as well.  Therefore, the function is given nextmode = p′n but it does not affect the
mechanism of the function almost at all, because data structures referred to in this
function have values consistent with primary mode in the next step as discussed in
§4.13.14 and thus the mechanism can depend almost only on the mode in the last step.
Therefore, pn is used only as an argument of oh1_stats_time() and set_sendbuf_
disps4s().

- Since this function is not called if we are in the transitional state of helpand-helper
reconfiguration, the argument trans to indicate that is eliminated.

- The role changing of NOfPGridTotal[][][] from per-grid histogram to per-grid index
has been done in exchange_particles4s() and thus the code for it is eliminated
from this function.

- Since we have to take care of particles in interior halo planes, we pass η = 1 to the
macro Move_Or_Do() as its last argument.


<!-- Page 534 -->

static void
move_and_sort(const int nextmode, const int psold, const int psnew,
const int oldp, const int *nacc, const int stats) {
const int me=myRank, ns=nOfSpecies, nn=nOfNodes, sbase=specBase;
const int mysubdom[2] = {me, oldp},  ninj = nOfInjections;
struct S_particle *p,  *rbb,  *sb = SendBuf + nacc[1];
int *nofr;
int ps, s, t, i;
Decl_For_All_Grid();
Decl_Grid_Info();


The first part for timing measurement, building the index array of sbuf (p, s, m) in
NOfSend[p][s][m], and building the pointer array of rbuf (p, s) in RecvBufBases[p][s] has a
few differences from that of the level-4p counterpart; p′n is passed to oh1_stats_time()
and set_sendbuf_disps4s(); 0 is passed to set_sendbuf_disps4s() through its second
argument trans and is used as the first dimensional index of RealSrcNeighbors[][] because
we cannot be in transitional state of helpand-helper reconfiguration; and the role changing
of NOfPGridTotal[][][] is eliiminated.

if (stats) oh1_stats_time(STATS_TB_MOVE, nextmode);
set_sendbuf_disps4s(nextmode, 0);
for (ps=0,t=0,nofr=NOfRecv,rbb=Particles; ps<=psnew; ps++) {
const int nnbr = RealSrcNeighbors[0][ps].n;
const int *rnbr = RealSrcNeighbors[0][ps].nbor;
for (s=0; s<ns; s++,t++,nofr+=nn) {
int n, nrec;
for (n=0,nrec=0; n<nnbr; n++)  nrec += nofr[rnbr[n]];
RecvBufBases[t] = rbb;  rbb += nrec;
}
}
RecvBufBases[t] = rbb;


The second part to scan all particles in pbuf (p, s) for all p ∈{0, pc} and s ∈[0, S),
and the third part to scan all injected primary (p = 0) or secondary (p = 1) particles of
species s are almost equivalent to those in the level-4p counterpart, except that we give
η = 1 to the macro Move_Or_Do() to copy each particle in a vertical interior halo plane to
BoundarySendBuf[]. As for the last part to let primaryParts and its shadow pointed by
secondaryBase be Qnn, it is equivalent to the counterpart.

for (ps=0,p=Particles,t=0; ps<=psold; ps++) {
const int mysd = mysubdom[ps];
for (s=0; s<ns; s++,t++) {
dint *npg = NOfPGrid[ps][s],  *npgt = NOfPGridTotal[ps][s];
const int itail = TotalP[t];
for (i=0; i<itail; i++,p++)
Move_Or_Do(p, ps, mysd, 1, (SendBuf[npgt[g]++]=*p), 1);
}
}
for (i=0; i<ninj; i++,p++) {
const int s = Particle_Spec(p->spec-sbase);
const OH_nid_t nid = p->nid;


<!-- Page 535 -->

const int ps = Secondary_Injected(nid) ? 1 : 0;
const int mysd = mysubdom[ps];
dint *npg = NOfPGrid[ps][s],  *npgt = NOfPGridTotal[ps][s];
if (nid<0) continue;
if (ps)  Primarize_Id_Only(p);
Move_Or_Do(p, ps, mysd, 1, (SendBuf[npgt[g]++]=*p), 1);
}
primaryParts = *secondaryBase = nacc[0];
}


#### 4.13.44 sort_received_particles()

sort_received_particles()  The function sort_received_particles() is solely called from exchange_particles4s(),
to sort particles received from other nodes when it finds Qn+Pnsend ≤Plim without helpand-
helper reconfiguration. The function is almost equivalent to its level-4p counterpart shown
in §4.10.39, but has a difference that we use Sort_Particle() to move a particle from
rbuf (p, s) to take care of the case that it is in a vertical interior halo plane. This difference
let us refer to NOfPGrid[p][s][] in addition to NOfPGridTotal[p][s][] so that they are passed
to the macro implicitly.


static void
sort_received_particles(const int nextmode, const int psnew, const int stats) {
const int ns=nOfSpecies;
int ps, s;
struct S_particle *p = Particles, **rbb = RecvBufBases+1;
Decl_Grid_Info();

if (stats) oh1_stats_time(STATS_TB_SORT, nextmode);
for (ps=0; ps<=psnew; ps++) {
for (s=0; s<ns; s++,rbb++) {
dint *npg = NOfPGrid[ps][s], *npgt = NOfPGridTotal[ps][s];
const struct S_particle *rbtail = *rbb;
for (; p<rbtail; p++)  Sort_Particle(p);
}
}
}


#### 4.13.45 set_sendbuf_disps4s()

set_sendbuf_disps4s()  The function set_sendbuf_disps4s(), called from move_to_sendbuf_4s() and move_
and_sort() prior to their particle scan, is almost equivalent to its level-4p counterpart set_
sendbuf_disps4p() shown in §4.10.45 to build the index array for SendBuf[] in NOfSend[][][]
based on the sending counts in itself. The only one difference is that this function is given an
additional argument nextmode = pn being 0 or 1 according to the mode in the next step be-
ing primary or secondary respectively, so that the function scans RealDstNeighbors[t][p],
where t is the argument trans, for p ∈{0, pn} instead of p ∈{0, 1} in order to keep it
from referring to meaningless values in RealDstNeighbors[0][1] when pn = 0 (with t = 0
definitely) because the callers work not only with pn = 1 but also with pn = 0.


static void


<!-- Page 536 -->

set_sendbuf_disps4s(const int nextmode, const int trans) {
const int nn=nOfNodes, ns=nOfSpecies;
int ps, s, i, np, *sbd;

for (ps=0,sbd=NOfSend,np=0; ps<=nextmode; ps++) {
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


#### 4.13.46 xfer_particles()

xfer_particles()  The function xfer_particles(), called solely from exchange_particles4s() regardless
of Qn + Pnsend ≤Plim or not, sends particles in the local node n to other nodes in
RealDstNeighbors[t][] and receives particles from other nodes in RealSrcNeighbors[t][],
where t = trans ∈{0, 1} argument being 1 iffwe have transitional state of helpand-helper
configuration. The other arguments are as follows; psnew = pn ∈{0, 1} being 1 iffn will
have secondary subdomain in the next step and thus may have some secondary particles to
receive; nextmode = p′n ∈{0, 1} being 1 iffwe will be in secondary mode in the next step
and thus may have some particles to send to other nodes as their secondary particles; and
sbuf is the pointer to SendBuf[0] or SendBuf[Qn] to specify the location of sbuf (0, 0, 0).
The function is very similar to its level-4p counterpart shown in §4.10.46, but have one
small difference that this function has the argument nextmode = p′n, because it and its caller
works regardless of p′n, to avoid referring to meaningless elements in RealDstNeighbors[0][1]
when p′n = 0.


static void
xfer_particles(const int trans, const int psnew, const int nextmode,
struct S_particle *sbuf) {
const int nn=nOfNodes, ns=nOfSpecies;
int ps, s, t, i, req, sdisp, *nofr, *nofs;


The first part to post MPI_Irecv() scanning NOfRecv[p][s][mi] for all p ∈{0, pn},
s ∈[0, S) and mi ∈RealSrcNeighbors[t][p].nbor[] is perfectly equivalent to the level-
4p counterpart. However, the second part to post MPI_Isend() scanning NOfSend[p][s][mi]
for all p ∈{0, p′n}, s ∈[0, S) and mi ∈RealDstNeighbors[t][p].nbor[] is a little bit different
from the counterpart because p ∈{0, p′n} instead of p ∈{0, 1} as discussed above. The last
part to confirm the completions of all MPI_Irecv() and MPI_Isend() by MPI_Waitall()
is perfectly equivalent to the counterpart again.

for (ps=0,t=0,nofr=NOfRecv,req=0; ps<=psnew; ps++) {
const int n = RealSrcNeighbors[trans][ps].n;
const int *nbor = RealSrcNeighbors[trans][ps].nbor;
for (s=0; s<ns; s++,t++,nofr+=nn) {


<!-- Page 537 -->

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
for (ps=0,t=0,sdisp=0,nofs=NOfSend; ps<=nextmode; ps++) {
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
}
}
}
MPI_Waitall(req, Requests, Statuses);
}


#### 4.13.47 xfer_boundary_particles_v()

xfer_boundary_particles_v()  The level-4s’s own function xfer_boundary_particles_v(), called solely from exchange_
particles4s() but twice, sends particles in vertical interior halo planes of the local node
n’s subcuboids to other nodes share the planes as their vertical exterior halo planes, and
receives particles in n’s vertical exterior halo planes from the nodes. The function is given
three arguments; psnew = pn ∈{0, 1} being 1 iffn has secondary subcuboid in the next
step and thus may have some secondary particles received; trans = t ∈{0, 1} being 1 iff
we have transitional state of helpand-helper configuration and thus the shape of secondary
subcuboid is determined by the subdomain of new parent; and d = d ∈{0, 1} for the
transfer along x (d = 0) or y (d = 1) axis.


static void
xfer_boundary_particles_v(const int psnew, const int trans, const int d) {
const int ns=nOfSpecies;
int vphi=d*2*2;
const int vphead=VPlaneHead[vphi], vptail=VPlaneHead[vphi+2*2];
int i, s, req=0, ps;
struct S_vplane *vp;
struct S_particle *p;
Decl_For_All_Grid();


<!-- Page 538 -->

At first we examine the number of entries for d-th dimensional transfers in VPlane[]
being V = Vt −Vh is 0 where {Vh, Vt} = VPlaneHead[{d, d+1}][0][0] and, if so, return to
the caller without doing nothing because n has neither subcuboids at all nor any neighbors
along d-th axis in this case.
Otherwise, we have V  pairs of hbuf sv(d, p, β, m) and hbuf rv(d, p, β, m) from/to which
primary (p = 0) or secondary (p = 1) halo particles are sent/received to/from the
node m = VPlane[v].nbor being in d-th dimensional lower (β = 0) or upper (β = 1)
neighbor family of the subdomain np = {n, parent(n)}[p], whose neighbor index is k =
32 + νy · 31 + νx · 30 where νx = {2β, 0}[d] and νy = {0, 2β}[d]. The serieses of both
type buffers are commonly formed as a conceptual 8-dimensional array as a whole having
elements [d][p][β][z][s][y][x][i] for d ∈{0, 1}, p ∈{0, pn}, β ∈{0, 1} z ∈[ζlp(n), ζup (n)),
s ∈[0, S), y ∈[ytl(p, k), ytu(p, k)), x ∈[xtl(p, k), xtu(p, k)) and i ∈[0, PO(p, s, gidx(x, y, z))),
where ytb(p, k) and xtb(p, k) (t ∈{s, r}, b ∈{l, u}) determine a vertical interior (t = s) or
exterior (t = r) halo plane in it shared with the neighbor of the subdomain np. The buffer
hbuf sv(d, p, β, m) has the portion [d][p][β][z][][][][] for z ∈[ζlp(n), ζup (n))∩ζlp′(m)ζup′(m) where
p′ = 0 if the k-th neighbor subdomain of np is primary one for m, or p′ = 1 otherwise, i.e.,
secondary one. In the buffer hbuf sv(d, p, β, m), all particles in the corresponding grid-voxels
have been copied from Particles[] by sort_particles(), move_and_sort() or sort_
received_particles(), and thus its correspondent hbuf sv(d, p′, 1−β, n) in the node m has
particles which hbuf rv(d, p, β, m) should have.
Therefore, we scan all entries VPlane[v] for all v ∈[Vh, Vt) to post MPI_Irecv() to
receive particles to hbuf rv(d, p, β, m). The location and the size of hbuf rv(d, p, β, m) is speci-
fied by the elements of VPlane[v]; its index in Particles[] is .rbuf and the size is .nrecv.
The tag of MPI_Irecv() is τ = p · 3D + k′ = VPlane[v].rtag where k′ = 3D −1 −k, to
make the combination (m, τ) unique in all receptions even when m occurs twice or more in
VPlane[].nbor due to its membership in two families and/or periodic system boundaries.

if (vphead==vptail)  return;

for (i=vphead,vp=VPlane+vphead; i<vptail; i++,vp++) {
const int nrecv = vp->nrecv;
if (nrecv)
MPI_Irecv(Particles+vp->rbuf, nrecv, T_Particle, vp->nbor, vp->rtag,
MCW, Requests+req++);
}

Next, we scan all entries VPlane[v] for all v ∈[Vh, Vt) again to post MPI_Isend() to
send particles from hbuf sv(d, p, β, m) to the node m as its (not n’s) primary (p′ = 0) or
secondary (p′ = 1) ones. The location and the size of hbuf sv(d, p, β, m) is specified by the
elements of VPlane[v]; its index in BoundarySendBuf[] is .sbuf and the size is .nsend. The
tag of MPI_Isend() is τ = p′ · 3D + k = VPlane[v].stag, to make the combination (n, τ)
unique in all receptions in m even when m occurs twice or more in VPlane[].nbor, and to
match the tag of MPI_Irecv() posted by m because n is in (3D−1−k)-th neighbor family
of the subdomain {m, parent(m)}[p′].

for (i=vphead,vp=VPlane+vphead; i<vptail; i++,vp++) {
const int nsend = vp->nsend;
if (nsend)
MPI_Isend(BoundarySendBuf+vp->sbuf, nsend, T_Particle, vp->nbor,
vp->stag, MCW, Requests+req++);
}


<!-- Page 539 -->

Then, we return to the caller if neither MPI_Irecv() nor MPI_Isend() have been posted
at all due to empty vertical halo planes. Othewise, we confirm the completion of all posted
MPI_Irecv() and MPI_Isend() by MPI_Waitall() giving it the number of posts, and
Requests[] and Statuses[] for each of posts.
Finally, we scan n’s vertical exterior halo planes of primary (p = 0) subcuboid always
and then secondary (p = 1) one if pn = 1. The series of hbuf rv(d, p, β, m), starting from
Particles[VPlane[Vh].rbuf],  is determined by make_bxfer_sched() and make_brecv_
sched() to form a conceptual 8-dimensional array to have elements [d][p][β][z][s][y][x][i] as
discuessed above where yrb(p, k) are determined by

Grid Exterior Boundary(νy, δy(np), yrl (p, k), yru(p, k)−δy(np))

and, if d = 0, xrb(p, k) are determined by

Grid Exterior Boundary(νx, δx(np), xrl (p, k), xru(p, k)−δx(np))

but xrb(p, k) = {−eg, δx(np)+eg} if d = 1 because south/north vertical exterior halo planes
should have pillars at their west/east ends.
Therefore, we scan PO(p, s, g) = NOfPGridOut[p][s][g] where g = gidx(x, y, z) and
the particles in the corresponding grid-voxel in row-major order of the conceptual ar-
ray to copy them to the region whose head is at SendBuf[NOfPGridIndex[p][s][g]], while
hbuf rv(d, p, β, m) are scanned sequentially from its top at Particles[VPlane[Vh].rbuf]. We
also let the nid element of the copied particles be −2 to indicate that they are not in n’s
subdomain.
One attention we have to pay in the scanning process is that, when d = 0, we will
encounter a grid-voxel at g in an exterior pillar with NOfPGrid[p][s][g] = (j+1)·232+σ ≥232
to mean the particles received for the grid-voxel should be relayed to a node m′ in the south
(β′ = 0) or north (β′ = 1) neighbor family of np and thus we have to copy them into
hbuf sv(1, p, β′, m′) starting from BoundarySendBuf[j].
The other remark is that hbuf rv(d, p, β, m) are definitely empty  if the corresponding
neighbor is inexistent. Since in this case it should be VPlaneHead[v] = VPlaneHead[v + 1]
where v = 4d+2p+β for [d][p][β], we can skip the unncecessary scan of the vertical exterior
halo plane when it holds.

if (req==0)  return;
MPI_Waitall(req, Requests, Statuses);

p = Particles + VPlane[vphead].rbuf;
for (ps=0; ps<=psnew; ps++) {
const int psor2 = ps ? trans + 1 : 0;
const int zl = ZBound[ps][OH_LOWER];
const int zu = ZBound[ps][OH_UPPER] - GridDesc[psor2].z;
int du;
for (du=OH_LOWER; du<=OH_UPPER; du++,vphi++) {
int ny;
int xl, yl, xu, yu;
if (VPlaneHead[vphi]==VPlaneHead[vphi+1])  continue;
if (d==OH_DIM_X) {
ny = 1;
Grid_Exterior_Boundary(du<<1, GridDesc[psor2].x, xl, xu);
} else {


<!-- Page 540 -->

ny = du<<1;
xl = -OH_PGRID_EXT;  xu = OH_PGRID_EXT;
}
Grid_Exterior_Boundary(ny, GridDesc[psor2].y, yl, yu);
For_All_Grid_Z(psor2, xl, yl, zl, xu, yu, zu) {
for (s=0; s<ns; s++) {
dint *npg=NOfPGrid[ps][s];
int *npgo=NOfPGridOut[ps][s], *npgi=NOfPGridIndex[ps][s];
For_All_Grid_XY(psor2, xl, yl, xu, yu) {
const int g = The_Grid(),  tail = npgi[g] + npgo[g];
const dint dst = npg[g];
int i;
if (Is_Pillar_Voxel(dst)) {
struct S_particle *q = p;
int j = Pillar_Upper(dst) - 1;
for (i=npgi[g]; i<tail; i++)  BoundarySendBuf[j++] = *q++;
}
for (i=npgi[g]; i<tail; i++) {
SendBuf[i] = *p++;  SendBuf[i].nid = -2;
}
}
}
}
}
}
}


#### 4.13.48 xfer_boundary_particles_h()

xfer_boundary_particles_h()  The level-4s’s own function xfer_boundary_particles_h(), called solely from exchange_
particles4s(), sends particles in horizontal interior halo planes of the local node n’s
subcuboids to other nodes sharing the planes as their horizontal exterior halo planes, and
receives particles in n’s horizontal exterior halo planes from the nodes. The function is
given an arguments psnew = pn ∈{0, 1} being 1 iffn has secondary subcuboid in the next
step and thus may have some secondary particles to be sent and received.


static void
xfer_boundary_particles_h(const int psnew) {
const int ns=nOfSpecies;
int ps, ud, s, req=0;

We have pairs of hbuf sh(p, β, s) and hbuf rh(p, β, s) in SendBuf[] from/to which primary
(p = 0) or secondary (p = 1) halo particles of species s are sent/received to/from the
node m = HPlane[p][β].nbor having a subcuboid just below (β = 0) or above (β = 1) n’s
subcuboid in its subdomain np = {n, parentn}[p]. The locations of the buffers are given as
the indices of SendBuf[] in HPlane[p][s].{sbuf, rbuf}[s] respectively, while their sizes are
in HPlane[p][s].{nsend, nrecv}[s] resepctively. Note that each buffer is a part of pbuf (p, s)
in the next step and is for grid-voxels whose indices g = gidx(x, y, zβt ) are given as follows
where zβs and zβr are for hbuf sh(p, β, s) and hbuf rh(p, β, s) respectively.

x ∈[−eg, δx(np) + eg),  y ∈[−eg, δy(np) + eg)
z0s = ζlp(n),   z1s = ζup (n) −1,   z0r = ζlp(n) −1,   z1s = ζup (n)


<!-- Page 541 -->

Therefore, what we basically have to do is simply sending the particles in hbuf sh(p, β, s) to
m and receiving those in m into hbuf rh(p, β, s) by posting MPI_Irecv() at first and then
MPI_Isend(). Note that halo particles in the intersection of horizontal exterior halo planes
and vertical exterior halo planes are obtained by this simple communication, because m has
already performed xfer_boundary_particles_v() to have those particles in its horizontal
interior halo planes.
One caution is that the tag τr for MPI_Irecv() is HPlane[p][β].rtag = (p·3D +k′)S +s
while τs for MPI_Isend() is HPlane[p][β].stag = (p′ · 3D + k)S + s, where p′ ∈{0, 1} is 1
iffthe subcuboid of m is secondary one, k = 2β · 32 + 31 + 30 and k′ = 3D −1 −k, so that
τr is unique for n’s reception even when m occurs multiple times in HPlane[][].nbor, and
τs is so for m’s reception as well and matches to n’s sending. The other caution is that m
can be MPI_PROC_NULL when n does not have any subcuboid, or its bottom/top surface is
in a non-periodic system boundary.

for (ps=0; ps<=psnew; ps++) {
for (ud=OH_LOWER; ud<=OH_UPPER; ud++) {
struct S_hplane *hp = HPlane[ps] + ud;
int *nrecv = hp->nrecv,  *rbuf = hp->rbuf;
const int nbor = hp->nbor,  tag = hp->rtag;
if (nbor!=MPI_PROC_NULL) {
for (s=0; s<ns; s++) {
if (nrecv[s])
MPI_Irecv(SendBuf+rbuf[s], nrecv[s], T_Particle, nbor, tag+s, MCW,
Requests+req++);
}
}
}
}
for (ps=0; ps<=psnew; ps++) {
for (ud=OH_LOWER; ud<=OH_UPPER; ud++) {
struct S_hplane *hp = HPlane[ps] + ud;
int *nsend = hp->nsend,  *sbuf = hp->sbuf;
const int nbor = hp->nbor,  tag = hp->stag;
if (nbor!=MPI_PROC_NULL) {
for (s=0; s<ns; s++) {
if (nsend[s])
MPI_Isend(SendBuf+sbuf[s], nsend[s], T_Particle, nbor, tag+s, MCW,
Requests+req++);
}
}
}
}

Then, we return to the caller if neither MPI_Irecv() nor MPI_Isend() have been posted
at all due to empty horizontal halo planes.  Othewise, we confirm the completion of all
posted MPI_Irecv() and MPI_Isend() by MPI_Waitall() giving it the number of posts,
and Requests[] and Statuses[] for each of posts.
Finally, we scan hbuf rh(p, s, β) for all p ∈{0, pn}, s ∈[0, S) and β ∈{0, 1} to let the nid
elements of received particles be −2 to indicate that they are not in n’s subdomain.

if (req==0)  return;
MPI_Waitall(req, Requests, Statuses);

for (ps=0; ps<=psnew; ps++) {


<!-- Page 542 -->

for (ud=OH_LOWER; ud<=OH_UPPER; ud++) {
struct S_hplane *hp = HPlane[ps] + ud;
int *nrecv = hp->nrecv,  *rbuf = hp->rbuf;
if (hp->nbor!=MPI_PROC_NULL) {
for (s=0; s<ns; s++) {
const int tail = rbuf[s] + nrecv[s];
int i;
for (i=rbuf[s]; i<tail; i++)  SendBuf[i].nid = -2;
}
}
}
}
}


#### 4.13.49 oh4s_exchange_border_data()

oh4s_exchange_border_data_()  The API functions oh4s_exchange_border_data_() for Fortran and oh4s_exchange_
oh4s_exchange_border_data()  border_data() for C perform inter-node communication for a particle-associated one-
dimensional array buf, whose element type is given by type of MPI_Datatype, using send/
receive buffers sbuf and rbuf so that its halo portions have the values computed by other
nodes which responsible of particles with which the array elements are associated.
The function oh4s_exchange_border_data_() simply  calls  its counterpart oh4s_
exchange_border_data() but type argument is converted into C’s value by MPI_Type_
f2c(). The function oh4s_exchange_border_data() is also simple because it just calls
exchange_border_data_v() twice for west/east bound communications and then south/
north ones, and then exchange_border_data_h() for horizontal halo plane, after obtaining
the byte-size of each element by MPI_Type_get_extent().


void
oh4s_exchange_border_data_(void *buf, void *sbuf, void *rbuf, int *type) {
oh4s_exchange_border_data(buf, sbuf, rbuf, MPI_Type_f2c(*type));
}
void
oh4s_exchange_border_data(void *buf, void *sbuf, void *rbuf,
MPI_Datatype type) {
MPI_Aint esize, lb;

MPI_Type_get_extent(type, &lb, &esize);
exchange_border_data_v(buf, sbuf, rbuf, type, esize, 0);
exchange_border_data_v(buf, sbuf, rbuf, type, esize, 1);
exchange_border_data_h(buf, type, esize);
}


#### 4.13.50 exchange_border_data_v()

exchange_border_data_v()  The function exchange_border_data_v(), called solely from oh4s_exchange_border_
data() but twice, sends particle-associated data in d-th (d = d ∈{0, 1}) dimensional
vertical interior halo planes of the one-dimensional array buf to other nodes after copying
them to sbuf, and recieves those from the nodes into rbuf and then moves them into ver-
tical exterior halo planes of buf. The data type and byte-size of each element is given by
other arguments t = type and e = esize.


<!-- Page 543 -->

static void
exchange_border_data_v(void *buf, void *sbuf, void *rbuf, MPI_Datatype type,
const MPI_Aint esize, const int d) {
char *b = (char*)buf, *sb = (char*)sbuf,  *rb = (char*)rbuf;
const int ns=nOfSpecies, pscurr=RegionId[1]<0 ? 0 : 1, vphi=d*2*2;
const int vphead=VPlaneHead[vphi], vptail=VPlaneHead[vphi+2*2];
struct S_vplane *vp;
int ps, s, i, req=0;
Decl_For_All_Grid();


This function has an execution flow similar to the particle transfer function xfer_
boundary_particles_v(), but has an additional phase to copy particle-associated data
elements to the series of hbuf sv(d, p, β, m) prior to the MPI communications for transfer
of particle-associated data.  That  is, after confirming that Vh  ̸= Vt where {Vh, Vt} =
VPlaneHead[{d, d+1}][0][0] to mean the local node n has some subcuboids, we scan particle-
associated data elements in grid-voxels of n’s vertical interior halo planes, whose pop-
ulation and head index in buf[] are given by PO(p, s, g) = NOfPGridOut[p][s][g] and
NOfPGridIndex[p][s][g], in the nested loops for all p ∈{0, pc}, β ∈{0, 1}, z ∈[ζlp(n), ζup (n)),
s ∈[0, S), y ∈[yl, yu) and x ∈[xl, xu) in this order, where pc ∈{0, 1} and pc = 1 iff
RegionId[1] = parent(n) ≥0, g = gidx(x, y, z), and yl, yu, xl, xu are given as follows with
Grid_Interior_Boundary() and np = {n, parent(n)}[p].

 (0, δy(np))          d = 0
(yl, yu) =   (0, eg)              d = 1, β = 0

(δy(np) −eg, δy(np))  d = 1, β = 1

 (−eg, δx(np) + eg)    d = 1
(xl, xu) =   (0, eg)              d = 0, β = 0

(δx(np) −eg, δx(np))  d = 0, β = 1

Since we copy all particle-associated data for each grid-voxel we visit to sbuf[] from its
head by memcpy(), giving it byte-addresses of the appropriate portions in buf[] and sbuf[]
and total byte-count of the elements to be copied knowing the element size is e-byte, the
buffer is formed as a conceptual 7-dimensional arrays of [p][β][z][s][y][x][i] for p,  . . . , x
shown above and i ∈[0, PO(p, s, gidx(x, y, z))).  Therefore, sbuf[] should be formed as a
series of buffers each of which has elements as many as hbuf sv(d, p, β, m). However, we have
to pay an attention that hbuf sv(d, p, β, m0) must not exist if the corresponding neighbor
does not exist, or particles copied from the corresponding vertical interior halo plane will
push down the buffers following hbuf sv(d, p, β, m0) to send incorrect particles to the nodes
in the family of the folloiwng neighbors.  Therefore we examine inexistence by checking
VPlaneHead[j] = VPlaneHead[j + 1] where j = 4d + 2p + β for [d][p][β].

if (vphead == vptail)  return;

for (ps=0,i=vphi; ps<=pscurr; ps++) {
int du;
const int zl = ZBound[ps][OH_LOWER];
const int zu = ZBound[ps][OH_UPPER] - GridDesc[ps].z;
for (du=OH_LOWER; du<=OH_UPPER; du++,i++) {
int ny;
int xl, yl, xu, yu;


<!-- Page 544 -->

if (VPlaneHead[i]==VPlaneHead[i+1])  continue;
if (d==OH_DIM_X) {
ny = 1;
Grid_Interior_Boundary(du<<1, GridDesc[ps].x, xl, xu);
} else {
ny = du<<1;
xl = -OH_PGRID_EXT;  xu = OH_PGRID_EXT;
}
Grid_Interior_Boundary(ny, GridDesc[ps].y, yl, yu);
For_All_Grid_Z(ps, xl, yl, zl, xu, yu, zu) {
for (s=0; s<ns; s++) {
int *npgo=NOfPGridOut[ps][s], *npgi=NOfPGridIndex[ps][s];
For_All_Grid_XY(ps, xl, yl, xu, yu) {
const int g = The_Grid(),  nbyte = npgo[g]*esize;
memcpy(sb, b+npgi[g]*esize, nbyte);
sb += nbyte;
}
}
}
}
}

Then, as done in xfer_boundary_particles_v(), we scan all entries VPlane[v] for all
v ∈[Vh, Vh) twice, at first to post MPI_Irecv() to receive particle-associated data into
rbuf[] and then to post MPI_Isend() to send data from sbuf[]. One attention we have to
pay is that VPlane[].{sbuf, rbuf} have indices of hbuf sv(d, p, β, m) and hbuf rv(d, p, β, m) for
all d ∈{0, 1}, while sbuf[] and rbuf[] are for particular d ∈{0, 1}. In addition, we have to
remember that the element size is e-byte and we assume sbuf[] and rbuf[] are char-type
buffer so that their elements have any basic type or a series of types. Therefore, the head
index of a buffer in sbuf[] or rbuf[] corresponding to hbuf sv(d, p, β, m) or hbuf rv(d, p, β, m)
specified by VPlane[v] is (VPlane[v].b −VPlane[Vh].b) · e where b ∈{sbuf, rbuf} corre-
spondingly.

rb -= VPlane[vphead].rbuf * esize;
for (i=vphead,vp=VPlane+vphead; i<vptail; i++,vp++) {
const int nrecv = vp->nrecv;
if (nrecv)
MPI_Irecv(rb+vp->rbuf*esize, nrecv, type, vp->nbor, vp->rtag, MCW,
Requests+req++);
}
sb = (char*)sbuf - VPlane[vphead].sbuf * esize;
for (i=vphead,vp=VPlane+vphead; i<vptail; i++,vp++) {
const int nsend = vp->nsend;
if (nsend)
MPI_Isend(sb+vp->sbuf*esize, nsend, type, vp->nbor, vp->stag, MCW,
Requests+req++);
}

Finally, still as done in xfer_boundary_particles_v(), we scan n’s vertical exterior
halo planes to copy the contents in rbuf[] to buf[] referring to elements of NOfPGridOut[][][]
and NOfPGridIndex[][][] corresponging to grid-voxels in the planes, after checking  if we
have posted some MPI_Irecv() or MPI_Isend() for early return and confirming their
completion by MPI_Waitall() with Requests[] and Statuses[], and also after check-
ing  if the neighbor corresponding to each plane determined by d, p and β exists,  i.e.,


<!-- Page 545 -->

VPlaneHead[j] ̸= VPlaneHead[j + 1] where j = 4d + 2p + β for [d][p][β]. A difference from
xfer_boundary_particles_v() is, besides the source and destination buffers, that the
copy of an element is done by memcpy() again with byte-addresses and byte-count. The
other difference is that we don’t take care of received particle-associated data in exterior
pillars because they are copied to corresponding region in buf[] by the first call with d = 0
and then copied to that in sbuf[] by the second call with d = 1 for relaying those data from
west/east neighbors to south/north ones.

if (req==0)  return;
MPI_Waitall(req, Requests, Statuses);

rb = (char*)rbuf;
for (ps=0,i=vphi; ps<=pscurr; ps++) {
const int zl = ZBound[ps][OH_LOWER];
const int zu = ZBound[ps][OH_UPPER] - GridDesc[ps].z;
int du;
for (du=OH_LOWER; du<=OH_UPPER; du++,i++) {
int ny;
int xl, yl, xu, yu;
if (VPlaneHead[i]==VPlaneHead[i+1])  continue;
if (d==OH_DIM_X) {
ny = 1;
Grid_Exterior_Boundary(du<<1, GridDesc[ps].x, xl, xu);
} else {
ny = du<<1;
xl = -OH_PGRID_EXT;  xu = OH_PGRID_EXT;
}
Grid_Exterior_Boundary(ny, GridDesc[ps].y, yl, yu);
For_All_Grid_Z(ps, xl, yl, zl, xu, yu, zu) {
for (s=0; s<ns; s++) {
int *npgo=NOfPGridOut[ps][s], *npgi=NOfPGridIndex[ps][s];
For_All_Grid_XY(ps, xl, yl, xu, yu) {
const int g = The_Grid(),  nbyte = npgo[g]*esize;
memcpy(b+npgi[g]*esize, rb, nbyte);
rb += nbyte;
}
}
}
}
}
}


#### 4.13.51 exchange_border_data_h()

exchange_border_data_h()  The function exchange_border_data_h(), called solely from oh4s_exchange_border_
data(), sends particle-associated data in horizontal  interior halo planes  of the one-
dimensional array buf to other nodes, and recieves those from nodes into horizontal exterior
halo planes also in buf. The data type and byte-size of each element is given by other ar-
guments t = type and e = esize.


static void
exchange_border_data_h(void *buf, MPI_Datatype type, const MPI_Aint esize) {


<!-- Page 546 -->

char *b=(char*)buf;
const int ns=nOfSpecies, pscurr=RegionId[1]<0 ? 0 : 1;
int ps, ud, s, req=0;
Decl_For_All_Grid();


This function is very similar to the halo particle transfer function xfer_boundary_
particles_h() except that the followings; scanning range p for HPlane[p][β] is given by
RegionId[1]; send/receive buffer is buf[], which we assume a char-type buffer with elements
of MPI_Datatype t having e-bytes for each, rather than SendBuf[]; and we do nothing after
the confirmation of communication completion while xfer_boundary_particles_h() have
to scan received particles to modify their nid elements.

for (ps=0; ps<=pscurr; ps++) {
for (ud=OH_LOWER; ud<=OH_UPPER; ud++) {
struct S_hplane *hp = HPlane[ps] + ud;
int *nrecv = hp->nrecv,  *rbuf = hp->rbuf;
const int nbor = hp->nbor,  tag = hp->rtag;
if (nbor!=MPI_PROC_NULL) {
for (s=0; s<ns; s++) {
if (nrecv[s])
MPI_Irecv(b+rbuf[s]*esize, nrecv[s], type, nbor, tag+s, MCW,
Requests+req++);
}
}
}
}
for (ps=0; ps<=pscurr; ps++) {
for (ud=OH_LOWER; ud<=OH_UPPER; ud++) {
struct S_hplane *hp = HPlane[ps] + ud;
int *nsend = hp->nsend,  *sbuf = hp->sbuf;
const int nbor = hp->nbor,  tag = hp->stag;
if (nbor!=MPI_PROC_NULL) {
for (s=0; s<ns; s++) {
if (nsend[s])
MPI_Isend(b+sbuf[s]*esize, nsend[s], type, nbor, tag+s, MCW,
Requests+req++);
}
}
}
}
if (req)  MPI_Waitall(req, Requests, Statuses);
}


#### 4.13.52 Macro Check_Particle_Location()

Check_Particle_Location()  The macro Check Particle Location(π, p, s, S, i) is perfectly equivalent to its level-4p
counterpart shown in §4.10.47.


#ifndef OH_NO_CHECK
#define Check_Particle_Location(P, PS, S, NS, INJ) {\
const int t = (PS) ? (S)+(NS) : (S);\
const int pidx = (P) - Particles;\


<!-- Page 547 -->

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


4.13.53  Macros Map_Particle_To_Neighbor() and Adjust_Neighbor_Grid()

Map_Particle_To_Neighbor()  The  macros  Map Particle To Neighbor(π, Xd, d, m, k, 3d, δd(m), xd, g)  and  Adjust
Adjust_Neighbor_Grid()  Neighbor Grid(xd, m, d) are perfectly equivalent to their level-4p counterparts shown in
§4.10.48.


#define Map_Particle_To_Neighbor(P, XYZ, DIM, MYSD, K, INC, UB, G, IDX) {\
const double xyz = XYZ;\
const double gsize = Grid[DIM].gsize;\
const double lb = Grid[DIM].fcoord[OH_LOWER];\
const double gf =\
(G = (xyz-lb)*Grid[DIM].rgsize + Grid[DIM].coord[OH_LOWER]) * gsize;\
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
#define Adjust_Neighbor_Grid(G, N, DIM)\
if (G<0) G += SubDomains[N][DIM][OH_UPPER]-SubDomains[N][DIM][OH_LOWER];


<!-- Page 548 -->

#### 4.13.54 oh4s_map_particle_to_neighbor()

oh4s_map_particle_to_neighbor_()  The API functions oh4s_map_particle_to_neighbor_()  for Fortran and oh4s_map_
oh4s_map_particle_to_neighbor()  particle_to_neighbor() for C are perfectly equivalent to their level-4p counterparts
oh4p_map_particle_to_neighbor[_]() shown in §4.10.49 except for their and callees’
names.


int
oh4s_map_particle_to_neighbor_(struct S_particle *part, const int *ps,
const int *s) {
return(oh4s_map_particle_to_neighbor(part, *ps, *s-1));
}
int
oh4s_map_particle_to_neighbor(struct S_particle *part, const int ps,
const int s) {
const int ns = nOfSpecies,  inj = part>=Particles+totalParts;
int x, y, z, w, d, dw, mysd;
const int psnn = ps ? (s+nOfSpecies)*nOfNodes : s*nOfNodes;
int k = OH_NBR_SELF,  idx = 0;
int gz, gy, gx;
int sd;
Decl_Grid_Info();

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
return(oh4s_map_particle_to_subdomain(part, ps, s));
sd = AbsNeighbors[ps][k];
if (sd>=nOfNodes) {
part->nid = -1;  return(-1);
}
Adjust_Neighbor_Grid(gx, sd, OH_DIM_X);


<!-- Page 549 -->

Do_Y(Adjust_Neighbor_Grid(gy, sd, OH_DIM_Y));
Do_Z(Adjust_Neighbor_Grid(gz, sd, OH_DIM_Z));
NOfPLocal[psnn+sd]++;

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


4.13.55  Macros Map_To_Grid, Map_Particle_To_Subdomain() and
Local_Coordinate()
Map_To_Grid()  The macros Map Particle To Subdomain(xd, d, πd), Map To Grid(π, X∗d, Xd, d, xd, x′d) and
Map_Particle_To_Subdomain()  Local Coordinate(m, n′, xd, x′d, d, k, 3d, a) are perfectly equivalent to their counterparts
Local_Coordinate()  shown in §4.10.50.


#define Map_To_Grid(P, PXYZ, XYZ, DIM, GG, LG) {\
const double gsize = Grid[DIM].gsize;\
const double lb = Grid[DIM].fcoord[OH_LOWER];\
const double ub = Grid[DIM].fcoord[OH_UPPER];\
double gf;\
XYZ = PXYZ;\
LG = 0;\
if (XYZ<lb) {\
if (BoundaryCondition[DIM][OH_LOWER]) { P->nid = -1;  return(-1); }\
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
#define Map_Particle_To_Subdomain(XYZ, DIM, SDOM) {\
double thresh = Grid[DIM].light.thresh;\
if (XYZ<thresh)\
SDOM = (XYZ - Grid[DIM].coord[OH_LOWER]) / Grid[DIM].light.size;\
else\
SDOM = (XYZ - thresh)/ (Grid[DIM].light.size + 1) + Grid[DIM].light.n;\


<!-- Page 550 -->

}
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


#### 4.13.56 oh4s_map_particle_to_subdomain()

oh4s_map_particle_to_subdomain_()  The API functions oh4s_map_particle_to_subdomain_() for Fortran and oh4s_map_
oh4s_map_particle_to_subdomain()  particle_to_subdomain() for C are perfectly equivalent to their level-4p counterparts
oh4p_map_particle_to_subdomain[_]() shown in §4.10.51 except for their names.


int
oh4s_map_particle_to_subdomain_(struct S_particle *part, const int *ps,
const int *s) {
return(oh4s_map_particle_to_subdomain(part, *ps, *s-1));
}
int
oh4s_map_particle_to_subdomain(struct S_particle *part, const int ps,
const int s) {
const int ns = nOfSpecies,  inj = part>=Particles+totalParts;
const int nx  = Grid[OH_DIM_X].n;
const int nxy = If_Dim(OH_DIM_Y, nx*Grid[OH_DIM_Y].n, 0);
const int t = ps ? ns + s : s;
int w, dw, mysd;
int sd;
double x, y, z;
int px, py, pz;
int gx, gy, gz;
int lx, ly, lz;
int k = OH_NBR_SELF,  aacc = 0;
Decl_Grid_Info();

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


<!-- Page 551 -->

Map_Particle_To_Subdomain(gx, OH_DIM_X, px);
Do_Y(Map_Particle_To_Subdomain(gy, OH_DIM_Y, py));
Do_Z(Map_Particle_To_Subdomain(gz, OH_DIM_Z, pz));
sd = Coord_To_Index(px, py, pz, nx, nxy);
}
Local_Coordinate(sd, mysd, gx, lx, OH_DIM_X, k, 1, aacc);
Do_Y(Local_Coordinate(sd, mysd, gy, ly, OH_DIM_Y, k, 3, aacc));
Do_Z(Local_Coordinate(sd, mysd, gz, lz, OH_DIM_Z, k, 9, aacc));
NOfPLocal[t*nOfNodes+sd]++;
if (aacc) {
currMode = Mode_Set_Any(currMode);
part->nid = Combine_Subdom_Pos(sd+OH_NEIGHBORS,
Coord_To_Index(gx, gy, gz, w, dw));
} else {
NOfPGrid[ps][s][Coord_To_Index(lx, ly, lz, w, dw)]++;
part->nid = Combine_Subdom_Pos(k, Coord_To_Index(gx, gy, gz, w, dw));
}
if (inj) {
if (sd==mysd)  InjectedParticles[t]++;
if (ps)  Secondarize_Id(part);
}
return(sd);
}


#### 4.13.57 oh4s_inject_particle()

oh4s_inject_particle_()  The API functions oh4s_inject_particle_() for Fortran and oh4s_inject_particle()
oh4s_inject_particle()  for C are perfectly equivalent to their level-4p counterparts oh4p_inject_particle[_]()
shown in §4.10.52 except for their and callees’ names.


int
oh4s_inject_particle_(const struct S_particle *part, const int *ps) {
return(oh4s_inject_particle(part, *ps));
}
int
oh4s_inject_particle(const struct S_particle *part, const int ps) {
const int ns = nOfSpecies;
int inj = totalParts + nOfInjections++;
struct S_particle *p = Particles + inj;
int s = Particle_Spec(part->spec - specBase);
int sd;

#ifndef OH_HAS_SPEC
if (ns!=1)
local_errstop("particles cannot be injected when S_particle does not "
"have ’spec’ element and you have two or more species");
#endif
if (inj>=nOfLocalPLimit)
local_errstop("injection causes local particle buffer overflow");
*p = *part;
sd = oh4s_map_particle_to_neighbor(p, ps, s);
if (sd<0)  nOfInjections--;
return(sd);


<!-- Page 552 -->

}


#### 4.13.58 oh4s_remove_mapped_particle()

oh4s_remove_mapped_particle_()  The API functions oh4s_remove_mapped_particle_() for Fortran and oh4s_remove_
oh4s_remove_mapped_particle()  mapped_particle() for C are perfectly equivalent to their level-4p counterparts oh4p_
remove_mapped_particle[_]() shown in §4.10.53 except for their names.


void
oh4s_remove_mapped_particle_(struct S_particle *part, const int *ps,
const int *s) {
oh4s_remove_mapped_particle(part, *ps, *s-1);
}
void
oh4s_remove_mapped_particle(struct S_particle *part, const int ps,
const int s) {
const int nn = nOfNodes, ns = nOfSpecies, inj = part>=Particles+totalParts;
OH_nid_t nid = part->nid;
int sd, g, psreal=ps, mysd, t;
Decl_Grid_Info();

Check_Particle_Location(part, psreal, s, ns, inj);
if (nid<0)  return;
sd = Subdomain_Id(nid, psreal);
g = Grid_Position(nid);
if (sd>=nn) {
psreal = 1;  Primarize_Id(part, sd);  nid = part->nid;
}
mysd = RegionId[psreal];
part->nid = -1;
t = psreal ? ns+s : s;
NOfPLocal[t*nn+sd]--;
if (inj && sd==mysd)  InjectedParticles[t]--;
if (Mode_Acc(currMode))  return;
if (sd!=mysd)  g = Local_Grid_Position(g, nid, psreal);
NOfPGrid[psreal][s][g]--;
}


#### 4.13.59 oh4s_remap_particle_to_neighbor()

oh4s_remap_particle_to_neighbor_()  The API functions oh4s_remap_particle_to_neighbor_() for Fortran and oh4s_remap_
oh4s_remap_particle_to_neighbor()  particle_to_neighbor() for C are perfectly equivalent to their level-4p counterparts
oh4p_remap_particle_to_neighbor[_]() shown in §4.10.54 except for their and callees’
names.


int
oh4s_remap_particle_to_neighbor_(struct S_particle *part, const int *ps,
const int *s) {
return(oh4s_remap_particle_to_neighbor(part, *ps, *s-1));
}
int
oh4s_remap_particle_to_neighbor(struct S_particle *part, const int ps,


<!-- Page 553 -->

const int s) {
oh4s_remove_mapped_particle(part, ps, s);
return(oh4s_map_particle_to_neighbor(part, ps, s));
}


#### 4.13.60 oh4s_remap_particle_to_subdomain()

oh4s_remap_particle_to_subdomain_()  The API functions oh4s_remap_particle_to_subdomain_() for Fortran and oh4s_remap_
oh4s_remap_particle_to_subdomain()  particle_to_subdomain() for C are perfectly equivalent to their level-4p counterparts
oh4p_remap_particle_to_subdomain[_]() shown in §4.10.55 except for their and callees’
names.


int
oh4s_remap_particle_to_subdomain_(struct S_particle *part, const int *ps,
const int *s) {
return(oh4s_remap_particle_to_subdomain(part, *ps, *s-1));
}
int
oh4s_remap_particle_to_subdomain(struct S_particle *part, const int ps,
const int s) {
oh4s_remove_mapped_particle(part, ps, s);
return(oh4s_map_particle_to_subdomain(part, ps, s));
}
