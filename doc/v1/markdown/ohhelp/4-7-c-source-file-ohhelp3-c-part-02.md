# 4.7 C Source File ohhelp3.c - Part 2

Source: `doc/v1/original/ohhelp.pdf`, pages 314-322.

<!-- Page 314 -->

int
oh3_map_region_to_adjacent_node_(double *x, double *y, int *ps) {
return(oh3_map_particle_to_neighbor(x, y, *ps));
}
int
oh3_map_particle_to_neighbor_(double *x, double *y, int *ps) {
return(oh3_map_particle_to_neighbor(x, y, *ps));
}
int
oh3_map_particle_to_neighbor(double *x, double *y, int ps) {
int rid=RegionId[ps], n=OH_NEIGHBORS>>1;

Map_Particle_To_Neighbor(x, rid, OH_DIM_X, n, 1);
Map_Particle_To_Neighbor(y, rid, OH_DIM_Y, n, 3);
return(Neighbor_Id(Neighbors[ps][n]));
}
#else
int
oh3_map_region_to_adjacent_node_(double *x, double *y, double *z, int *ps) {
return(oh3_map_particle_to_neighbor(x, y, z, *ps));
}
int
oh3_map_particle_to_neighbor_(double *x, double *y, double *z, int *ps) {
return(oh3_map_particle_to_neighbor(x, y, z, *ps));
}
int
oh3_map_particle_to_neighbor(double *x, double *y, double *z, int ps) {
int rid=RegionId[ps], n=OH_NEIGHBORS>>1;

Map_Particle_To_Neighbor(x, rid, OH_DIM_X, n, 1);
Map_Particle_To_Neighbor(y, rid, OH_DIM_Y, n, 3);
Map_Particle_To_Neighbor(z, rid, OH_DIM_Z, n, 9);
return(Neighbor_Id(Neighbors[ps][n]));
}
#endif


#### 4.7.19 Macros Map_Particle_To_Subdomain() and Adjust_Subdomain()

Map_Particle_To_Subdomain()  The macro Map Particle To Subdomain(x, d, πd), used in three versions of oh3_map_
Adjust_Subdomain()  particle_to_subdomain(), does;
{
⌊(x −∆ld · γd)/(δmind    · γd)⌋           x < ∆−d  · γd                                         πd ←   −
Π d + ⌊(x −∆−d  · γd)/((δmind  + 1) · γd)⌋  x ≥∆−d  · γd

to translate the given particle coordinate x into the d-th dimensional process coordinate πd
in the regular process coordinate system where the particle resides if ∆ld · γd ≤x < ∆ud · γd,
referring to

Grid[d].fcoord[{0, 1}] = {∆ld · γd, ∆ud · γd}
Grid[d].light.{rfsize,rfsizeplus,fthresh,n} =
{1/(δmind    · γd), 1/((δmind  +1) · γd), ∆−d  · γd, Πd− }

Otherwise, the macro makes its user function return to its caller with −1 to indicate the
particle is out-of-bounds.


<!-- Page 315 -->

The translation above, however, can be inaccurate because neither the division by δmind    ·
γd and (δmind  +1)·γd nor multiplication by their reciprocals done in the implementation give
accurate result due to floating-point calculation error. Therefore oh3_map_particle_to_
subdomain() invokes Adjust Subdomain(x, d, m, Π ′) where m = rank(π0, · · · , πD−1) and
′  ∏d−1
Π =   i=0 Πi to correct the possible errors by the following.
{              ′                                    m −Π   x < δld(m)
m ← m + Π ′  x ≥δud(m)
m       otherwise



#define Map_Particle_To_Subdomain(XYZ,DIM,SDOM) {\
double thresh = Grid[DIM].light.fthresh;\
if (XYZ<Grid[DIM].fcoord[OH_LOWER] || XYZ>=Grid[DIM].fcoord[OH_UPPER])\
return(-1);\
if (XYZ<thresh)\
SDOM = (XYZ - Grid[DIM].fcoord[OH_LOWER]) * Grid[DIM].light.rfsize;\
else  SDOM = (int)((XYZ - thresh) * Grid[DIM].light.rfsizeplus) + \
Grid[DIM].light.n;\
}
#define Adjust_Subdomain(XYZ,DIM,SDOM,INC) {\
if (XYZ<SubDomainsFloat[SDOM][DIM][OH_LOWER])  SDOM-=INC;\
else if (XYZ>=SubDomainsFloat[SDOM][DIM][OH_UPPER])  SDOM+=INC;\
}


#### 4.7.20 oh3_map_particle_to_subdomain()

oh3_map_particle_to_subdomain_()  The API function oh3_map_particle_to_subdomain_()57  for Fortran and oh3_map_
oh3_map_particle_to_subdomain()  particle_to_subdomain() for C find the subdomain m, which should accommodate the
particle whose coordinate is given by the arguments x, y (if D ≥2) and z (if D = 3), and is
returned to the caller. If such a subdomain is not found due to that the particle is moving
out-of-bounds, the function returns −1 instead of the subdomain identifier.
Since the function takes D arguments for the particle coordintate, we have three versions
of each function. In all versions, the Fortran API oh3_map_particle_to_subdomain_()58
simply calls C counterpart oh3_map_particle_to_subdomain() and returns what the C
counterpart returns. Also in all versions, oh3_map_particle_to_subdomain() for the local
node n calls map_irregular_subdomain() with x, y and z (or 0 if D < 3) to have m (or
−1) and to return to the caller with it, if SubDomainDesc ̸= NULL meaning irregular process
coordinate. Otherwise, i.e., SubDomainDesc = NULL meaning regular process coordinate,
the function invokes the macro Map_Particle_To_Subdomain() D times giving arguments
xd = {x, y, z}[d], d and πd to have πd for all d ∈[0, D) from which the return value is
approximated by m = rank(π0, . . . , πD−1), which is, for example, π0 + Π0 · (π1 + Π1 · π2)
if D = 3. Then the macro Adjust_Subdomain() is invoked D times with arguments xd,
∏d−1
d, m and   i=0 Πd to let m be its neighbor including itself to correct the floating-point
calculation error in Map_Particle_To_Subdomain().


#if OH_DIMENSION==1
int

57And its aliase oh3 map region to node () for backward compatiblity.
58And oh3 map region to node () as well.


<!-- Page 316 -->

oh3_map_region_to_node_(double *x) {
return(oh3_map_particle_to_subdomain(*x));
}
int
oh3_map_particle_to_subdomain_(double *x) {
return(oh3_map_particle_to_subdomain(*x));
}
int
oh3_map_particle_to_subdomain(double x) {
int sdx;

if (SubDomainDesc)  return(map_irregular_subdomain(x, 0.0, 0.0));
Map_Particle_To_Subdomain(x, OH_DIM_X, sdx);
Adjust_Subdomain(x, OH_DIM_X, sdx, 1);
return(sdx);
}
#elif OH_DIMENSION==2
int
oh3_map_region_to_node_(double *x, double *y) {
return(oh3_map_particle_to_subdomain(*x, *y));
}
int
oh3_map_particle_to_subdomain_(double *x, double *y) {
return(oh3_map_particle_to_subdomain(*x, *y));
}
int
oh3_map_particle_to_subdomain(double x, double y) {
int sdx, sdy, sd, nx=Grid[OH_DIM_X].n;

if (SubDomainDesc)  return(map_irregular_subdomain(x, y, 0.0));
Map_Particle_To_Subdomain(x, OH_DIM_X, sdx);
Map_Particle_To_Subdomain(y, OH_DIM_Y, sdy);
sd = sdx + nx * sdy;
Adjust_Subdomain(x, OH_DIM_X, sd, 1);
Adjust_Subdomain(y, OH_DIM_Y, sd, nx);
return(sd);
}
#else
int
oh3_map_region_to_node_(double *x, double *y, double *z) {
return(oh3_map_particle_to_subdomain(*x, *y, *z));
}
int
oh3_map_particle_to_subdomain_(double *x, double *y, double *z) {
return(oh3_map_particle_to_subdomain(*x, *y, *z));
}
int
oh3_map_particle_to_subdomain(double x, double y, double z) {
int sdx, sdy, sdz, sd, nx=Grid[OH_DIM_X].n, nxy=nx*Grid[OH_DIM_Y].n;

if (SubDomainDesc)  return(map_irregular_subdomain(x, y, z));
Map_Particle_To_Subdomain(x, OH_DIM_X, sdx);
Map_Particle_To_Subdomain(y, OH_DIM_Y, sdy);
Map_Particle_To_Subdomain(z, OH_DIM_Z, sdz);


<!-- Page 317 -->

sd = sdx + nx * sdy + nxy * sdz;
Adjust_Subdomain(x, OH_DIM_X, sd, 1);
Adjust_Subdomain(y, OH_DIM_Y, sd, nx);
Adjust_Subdomain(z, OH_DIM_Z, sd, nxy);
return(sd);
}
#endif


#### 4.7.21 map_irregular_subdomain()

map_irregular_subdomain()  The  function map_irregular_subdomain(),  called from  three  versions  of oh3_map_
particle_to_subdomain(), simply calls map_irregular() passing its own arguments x, y
and z for a particle position, together with dim = 0, from = 0 and n = N to let it search the
subdomain containing the particle from the whole members of SubDomainDesc[N] starting
from the dimension 0.


int
map_irregular_subdomain(double x, double y, double z) {
return(map_irregular(x, y, z, OH_DIM_X, 0, nOfNodes));
}


#### 4.7.22 map_irregular()

map_irregular()  The recursive function map_irregular(), called from map_irregular_subdomain() and
map_irregular() itself, tries to find the subdomain containing the particles whose (d+k)-
th coordinate is given by the argument pk, where d = dim and k ∈[0, 2] (and pk = 0 for k
s.t. d + k ≥D), from SubDomainDesc[[i0, in)] where i0 = from and in = i0 + n.
SubDomainDesc[[i0, in)] is ascendingly ordered by δld(i(m)) + δud(i(m)) or equivalently
(δld(i(m)) + δud(i(m))) · γd, where {i(m), δld(i(m)) · γd, δud(i(m)) · γd} = SubDomainDesc[m].
{id, coord[d].fc[{0, 1}]}, corresponging to the midpoint plane of the lower/upper bound-
ary planes, and a subdomain m such that δld(i(m)) · γd ≤x = p0 < δud(i(m)) · γd should
satisfy the following because (δud(i(m)) −δld(i(m))) · γd ≤δmaxd     · γd = Grid[d].fsize.

δld(i(m)) + δud(i(m))                                                                                               δmaxd                                                    δld(i(m)) + δud(i(m)) −δmax                                                          +                                                                       d   ≤x/γd <
2             2                    2             2
⇐⇒2x −δmaxd     · γd < (δld(i(m)) + δud(i(m))) · γd ≤2x + δmaxd     · γd

Therefore,  j0 = min{m | (δld(i(m)) + δud(i(m)))  · γd >  2x −δmaxd      · γd} and jn =
min{m | (δld(i(m)) + δud(i(m))) · γd > 2x + δmaxd     · γd} can be found by a binary search
in SubDomainDesc[[i0, in)] and then SubDomainDesc[[j0, in)] by calling map_irregular_
range() twice giving it 2x −δmaxd     · γd and 2x + δmaxd     · γd to limit the targets to find
{m | δld(i(m)) ≤x/γd < δud(i(m))} ⊆[j0, jn).
Then we traverse SubDomainDesc[[j0, jn)] only for those SubDomainDesc[j].coord[d].h
= j skipping its successors such that δld(i(j′)) = δld(i(j)) and δud(i(j′)) = δud(i(j)) where
j′ ∈(j, j+l) and l = SubDomainDesc[j].coord[d].n. Note that the minimality of j0 assured
that SubDomainDesc[j0].coord[d].h = j0, i.e., j0 is the head subdomain in a wall or pillar.
Then for each j such that δld(i(j)) ≤x/γd < δud(i(j)) if d < D −1, we recursively call
map_irregular() itself giving it p1, p2 and 0 for the particle position, d + 1, and j and
l to specify the wall or pillar in which the search takes place. Then we simply return the


<!-- Page 318 -->

return value to the caller if it is non-negative and thus the target subdomain identifier, or
we continue the traversal otherwise.
On the other hand, if d = D −1 and we find δld(i(j)) ≤x/γd < δud(i(j)), i(j) is returned
to the caller as the target subdomain identifier.
Finally,  if we could not find the target in the traversal (including the case of empty
[j0, jn)), we return −1 to indicate the search failure.


static int
map_irregular(double p0, double p1, double p2, int dim, int from, int n) {
double size=Grid[dim].fsize;
int to=from+n, lo, up, i;
struct S_subdomdesc *sd = SubDomainDesc;

lo = map_irregular_range(p0*2.0-size, dim, from, to);
up = map_irregular_range(p0*2.0+size, dim, lo, to);
for (i=lo; i<up; ) {
int n = sd[i].coord[dim].n;
if (p0>=sd[i].coord[dim].fc[OH_LOWER] &&
p0< sd[i].coord[dim].fc[OH_UPPER]) {
if (dim<OH_DIMENSION-1) {
int ret = map_irregular(p1, p2, 0.0, dim+1, i, n);
if (ret>=0)  return(ret);
}
else
return(sd[i].id);
}
i += n;
}
return(-1);
}


#### 4.7.23 map_irregular_range()

map_irregular_range()  The function map_irregular_range(), called solely from map_irregular(), finds mmin =
min{m | i0 ≤m <  in,  (δld(i(m)) + δud(i(m))) · γd > x′} from SubDomainDesc[[i0, in)]
where x′ = p = 2x ± δmaxd      · γd  for the d-th dimensional coordinate x of a parti-
cle, d =  dim,  i0 =  from,  in =  to, and {i(m),  δld(i(m)) · γd,  δud(i(m)) · γd} =
SubDomainDesc[m].{id, coord[d].fc[{0, 1}]}.
At first, if i0 = in to mean [i0, in) = ∅or (δld(i(in−1)) + δud(i(in−1))) · γd ≤x′ to mean
that no subdomains satisfiy the criterion, we return in to indicate nothing is found. On
the other hand if (δld(i(i0)) + δud(i(i0))) · γd > x′ to mean that i0 satisfies the criterion, we
simply return i0 being the minimum one.
Otherwise, i.e., (δld(i(i0)) + δud(i(i0))) · γd ≤x′ < (δld(i(in−1)) + δud(i(in−1))) · γd, we
start a binary search to find mmin starting with j0 = i0, jn = in and j = ⌊(j0 + jn)/2⌋to
examine j satisfies the criterion. Then we let jn ←j if satisfied, while j0 ←j otherwise,
and let j = ⌊(j0 + jn)/2⌋again, and repeat this process while j0 < j. Since at initial j0
is unsatisfiable while jn −1 is satisfiable, it is assured that we should reach j = j0 and
j + 1 = jn = mmin to return jn as the result.


static int
map_irregular_range(double p, int dim, int from, int to) {


<!-- Page 319 -->

struct S_subdomdesc *sd = SubDomainDesc;
int i;

if (from==to) return(to);
if (p<sd[from].coord[dim].fc[OH_LOWER]+sd[from].coord[dim].fc[OH_UPPER])
return(from);
if (p>=sd[to-1].coord[dim].fc[OH_LOWER]+sd[to-1].coord[dim].fc[OH_UPPER])
return(to);
for (i=(from+to)>>1; from<i; i=(from+to)>>1) {
if (p<sd[i].coord[dim].fc[OH_LOWER]+sd[i].coord[dim].fc[OH_UPPER])
to = i;
else
from = i;
}
return(to);
}


#### 4.7.24 oh3_bcast_field()

oh3_bcast_field_()  The API functions oh3_bcast_field_() for Fortran and oh3_bcast_field() for C provide
oh3_bcast_field()  a simulator body calling them with a safe mechanism of broadcast communications in
primary/secondary families of the local node. The broadcasts are performed on a field-
array type f = ftype whose origins are pointed by pfld and sfld for the primary and
secondary subdomains respectively. Both functions simply call oh1_broadcast() giving it
the bases of the subarrays to be broadcasted which are offset by FieldDesc[f ′].bc.base
from the origins, their sizes FieldDesc[f ′].bc.size[p] for primary (p = 0) and secondary
subdomains, and the data type MPI_DOUBLE commonly for primary/secondary ones, where
f ′ = f −1 for Fortan API while f ′ = f for C API.


void
oh3_bcast_field_(void *pfld, void *sfld, int *ftype) {
int base=FieldDesc[*ftype-1].bc.base;
int *size=FieldDesc[*ftype-1].bc.size;

oh1_broadcast((double*)pfld+base, (double*)sfld+base, size[0], size[1],
MPI_DOUBLE, MPI_DOUBLE);
}
void
oh3_bcast_field(void *pfld, void *sfld, int ftype) {
int base=FieldDesc[ftype].bc.base;
int *size=FieldDesc[ftype].bc.size;

oh1_broadcast((double*)pfld+base, (double*)sfld+base, size[0], size[1],
MPI_DOUBLE, MPI_DOUBLE);
}


#### 4.7.25 oh3_reduce_field()

oh3_reduce_field_()  The API functions oh3_reduce_field_() for Fortran and oh3_reduce_field() for C pro-
oh3_reduce_field()  vide a simulator body calling them with a safe mechanism of summing-up reductions in
primary/secondary families of the local node. The reductions are performend on a field-
array type f = ftype whose origins are pointed by pfld and sfld for the primary and


<!-- Page 320 -->

secondary subdomains respectively. Both functions simply call oh1_reduce() giving it the
bases of the subarrays to be reduced which are offset by FieldDesc[f ′].red.base from
the origins, their sizes FieldDesc[f ′].red.size[p] for primary (p = 0) and secondary sub-
domains, and the data type MPI_DOUBLE and operator MPI_SUM commonly for primary/
secondary ones, where f ′ = f −1 for Fortan API while f ′ = f for C API.


void
oh3_reduce_field_(void *pfld, void *sfld, int *ftype) {
int base=FieldDesc[*ftype-1].red.base;
int *size=FieldDesc[*ftype-1].red.size;

oh1_reduce((double*)pfld+base, (double*)sfld+base, size[0], size[1],
MPI_DOUBLE, MPI_DOUBLE, MPI_SUM, MPI_SUM);
}
void
oh3_reduce_field(void *pfld, void *sfld, int ftype) {
int base=FieldDesc[ftype].red.base;
int *size=FieldDesc[ftype].red.size;

oh1_reduce((double*)pfld+base, (double*)sfld+base, size[0], size[1],
MPI_DOUBLE, MPI_DOUBLE, MPI_SUM, MPI_SUM);
}


#### 4.7.26 oh3_allreduce_field()

oh3_allreduce_field_()  The API functions oh3_allreduce_field_() for Fortran and oh3_allreduce_field() for
oh3_allreduce_field() C are simply all-reduce versions of oh3_reduce_field[_](). Therefore their implementa-
tions are different from the reduce-counterparts just in the point that they call oh1_all_
reduce() instead of oh1_reduce().


void
oh3_allreduce_field_(void *pfld, void *sfld, int *ftype) {
int base=FieldDesc[*ftype-1].red.base;
int *size=FieldDesc[*ftype-1].red.size;

oh1_all_reduce((double*)pfld+base, (double*)sfld+base, size[0], size[1],
MPI_DOUBLE, MPI_DOUBLE, MPI_SUM, MPI_SUM);
}
void
oh3_allreduce_field(void *pfld, void *sfld, int ftype) {
int base=FieldDesc[ftype].red.base;
int *size=FieldDesc[ftype].red.size;

oh1_all_reduce((double*)pfld+base, (double*)sfld+base, size[0], size[1],
MPI_DOUBLE, MPI_DOUBLE, MPI_SUM, MPI_SUM);
}


#### 4.7.27 oh3_exchange_borders()

oh3_exchange_borders_()  The API functions oh3_exchange_borders_() for Fortran and oh3_exchange_borders()
oh3_exchange_borders()  for C provide a simulator body calling them with a mechanism of boundary communications


<!-- Page 321 -->

type c = ctype of field-array pointed by pfld between adjacent primary subdomains.  It
also performs broadcast communications in primary/secondary families of the local node
to send receiving planes of the primary field-array to its children and to receive receiving
planes of the secondary field-array pointed by sfld from its parent, if bcast is true.


void
oh3_exchange_borders_(void *pfld, void *sfld, int *ctype, int *bcast) {
oh3_exchange_borders(pfld, sfld, *ctype-1, *bcast);
}
void
oh3_exchange_borders(void *pfld, void *sfld, int ctype, int bcast) {
MPI_Status st;
int d, lu;
double *pf=(double*)pfld, *sf=(double*)sfld;


First we perform downward (w = 0) and upward (w = 1) communications for d-th di-
mensional boundary planes for all d ∈[0, D) and in ascending order of d from d = 0. Each
communication is to send lower/upper sending planes to the node Adjacent[d][w] refer-
ring to the communication parameters in BorderExc[c][0][d][w].send and to receive them
into upper/lower receiving planes from the node Adjacent[d][1−w] with the parameters in
BorderExc[c][0][d][w].recv. The communication is performed by MPI_Sendrecv() if count
elements of both parameter sets are positive, by MPI_Send() if only send.count is positive,
or by MPI_Recv() if only recv.count is positive.

for (d=0; d<OH_DIMENSION; d++) {
for (lu=OH_LOWER; lu<=OH_UPPER; lu++) {
int dst=Adjacent[d][lu], src=Adjacent[d][1-lu];
struct S_borderexc *bx=&BorderExc[ctype][0][d][lu];
int scount=bx->send.count;
int rcount=bx->recv.count;
if (scount && rcount)
MPI_Sendrecv(pf+bx->send.buf, scount, bx->send.type, dst, 0,
pf+bx->recv.buf, rcount, bx->recv.type, src, 0,
MCW, &st);
else if (scount)
MPI_Send(pf+bx->send.buf, scount, bx->send.type, dst, 0, MCW);
else if (rcount)
MPI_Recv(pf+bx->recv.buf, rcount, bx->recv.type, src, 0, MCW, &st);
}
}

Next and finally, we perform the broadcast communications of receiving planes if we
are in secondary mode (currMode mod 2 ̸= 0) and broadcasting is required by bcast ̸= 0.
Before broadcasting, we call set_border_exchange() to set up the type c communication
parameters for the secondary subdomain if BorderExc[c][1][0][0].send.count < 0 to mean
that the local node just has been assigned a new secondary subdomain. Note that the type
argument of set_border_exchange() is always MPI_DOUBLE because the exceptional case
requiring MPI_LONG_LONG_INT for per-grid histogram with position-aware particle manage-
ment does not perform the broadcast of receiving planes. Then we perform the broadcast
communications for all d ∈[0, D) and β = {0, 1} by oh1_broadcast() giving it arguments
based on the parameters for the primary subdomain BorderExc[c][0][d][β] and those for the
secondary subdomain BorderExc[c][1][d][β].


<!-- Page 322 -->

if (Mode_PS(currMode) && bcast) {
if (RegionId[1]>=0 &&
BorderExc[ctype][1][OH_DIM_X][OH_LOWER].send.count<0)
set_border_exchange(ctype, 1, MPI_DOUBLE);
for (d=0; d<OH_DIMENSION; d++) {
for (lu=OH_LOWER; lu<=OH_UPPER; lu++) {
struct S_borderexc *bxp=&BorderExc[ctype][0][d][lu];
struct S_borderexc *bxs=&BorderExc[ctype][1][d][lu];
oh1_broadcast(pf+bxp->recv.buf, sf+bxs->recv.buf,
bxp->recv.count, bxs->recv.count,
bxp->recv.type, bxs->recv.type);
}
}
}
}
