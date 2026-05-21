# 4.7 C Source File ohhelp3.c - Part 1

Source: `doc/original/ohhelp.pdf`, pages 282-313.

<!-- Page 282 -->

## 4.7 C Source File ohhelp3.c

#### 4.7.1 Header File Inclusion

The first job done in ohhelp3.c is the inclusion of the header files ohhelp1.h, ohhelp2.h and
ohhelp3.h. Before the inclusion of ohhelp1.h and ohhelp2.h, we #define the macro EXTERN
as extern so as to make variables declared in the files external, but after that we make it
#undef’iend and then #define it as empty so as to provide variables declared in ohhelp3.h
with their homes, as discussed in §4.2.3.

#define EXTERN extern
#include "ohhelp1.h"
#include "ohhelp2.h"
#undef  EXTERN
#define EXTERN
#include "ohhelp3.h"



#### 4.7.2 Function Prototypes

The next and last job to do prior to function definitions is to declare the prototypes of the
following functions private for the level-3 library.

- The function init_subdomain_actively() initializes domain/subdomain and their
boudnary descriptors SubDomains[][][], Grid[] and Boundaries[][][] when regular pro-
cess coordinate is specified by sdoms argument of oh3_init().

- The function init_subdomain_passively() initializes domain/subdomain and their
boudnary descriptors Grid[], SubDomainDesc[] and Boundaries[][][] according to
SubDomains[][][] = sdoms argument of oh3_init() which specifies irregular process
coordinate.

- The function comp_xyz()  is used to sort SubDomainDesc[] in init_subdomain_
passively() through qsort().

- The function init_fields() initializes field-array descriptors FieldTypes[][] and
FieldDesc[].  It also initializes boundary communication descriptors BoundaryComm
Fields[], BoundaryCommTypes[][][][] and BorderExc[][][][].

- The function set_border_exchange() sets up the elements of BorderExc[c][p][d][w]
for given c and p and for all d ∈[0, D) and w ∈{0, 1}.

- The function set_border_comm()  sets up BorderExc[c][p][d][w].{send, recv}  for
given c ∈[0, C), p ∈{0, 1} and w ∈{0, 1}, and for all d ∈[0, D).

- The function transbound3() is the body of oh3_transbound().

- The function map_irregular() is the body of map_irregular_subdomain().

- The function map_irregular_range() is to find a set of candidate subdomains from
which the subdomain is searched by map_irregular().


<!-- Page 283 -->

static void init_subdomain_actively(int (*sd)[OH_DIMENSION][2],
int sc[OH_DIMENSION][2],
int *pcoord, int bc[OH_DIMENSION][2],
int (*bd)[OH_DIMENSION][2], int nb,
int bbase);
static void init_subdomain_passively(int (*sd)[OH_DIMENSION][2],
int (*bd)[OH_DIMENSION][2], int nb,
int bbase);
static int  comp_xyz(const void* aa, const void* bb);
static void init_fields(int (*ft)[OH_FTYPE_N], int *cf, int cfid,
int (*ct)[2][OH_CTYPE_N], int nb,
int sd[OH_DIMENSION][2], int **fsizes);
static void set_border_exchange(int e, int ps, MPI_Datatype type);
static void set_border_comm(int esize, int f, int *xyz, int *wdh,
int (*exti)[2], int (*exto)[2],
int (*off)[2], int (*size)[2],
int lu, int sr, MPI_Datatype basetype,
struct S_borderexc bx[OH_DIMENSION][2]);
static int  transbound3(int currmode, int stats, int level);
static int  map_irregular(double p0, double p1, double p2, int dim, int from,
int n);
static int  map_irregular_range(double p, int dim, int from, int to);



#### 4.7.3 oh3_init() and oh13_init()

oh3_init_()  The API functions oh3_init_() for Fortran and oh3_init() for C receive a set of ar-
oh3_init()  ray/structure variables through which level-1 to level-3 library functions communicate with
the simulator body, and a few integer parameters to specify the behavior of the library.
The functions have the following arguments.

- sdid
nspec
maxfrac
nphgram
totalp
The five arguments above are perfectly equivalent to those of the level-1 counterparts
oh1_init[_]().

- pbuf
pbase
maxlocalp
The three arguments above are perfectly equivalent to those of the level-2 counterparts
oh2_init[_]().

- mycomm
nbor
pcoord
The three arguments above are perfectly equivalent to those of the level-1 counterparts
oh1_init[_]().


<!-- Page 284 -->

- The argument sdoms should be the (double) pointer to an integer arrray of [N][D][2]
being the shadow of SubDomains[][][].   If sdoms[0][0][0] > sdoms[0][0][1], or sdoms
points NULL to make init3() allocate the array, all the elements of the array [m][d][β]
are filled by init_subdomain_actively() to have boundary coordinates of subdo-
main m, δld(m) (β = 0) and δud(m) (β = 1), for regular process coordinate. Otherwise,
the array should have δ{l,u}d   (m) to be referred to by init_subdomain_passively()
to create SubDomainDesc[] for irregular process coordinate.

- The argument scoord should be the pointer to an integer array of [D][2] to specify
boundary coordinates of the system domain, ∆ld (β = 0) and ∆ud (β = 1), in its
element [d][β], if regular process coordinate is specified. Otherwise, it can be NULL or
can point anything.

- The integer argument nbound shoud have the number of boundary conditions B =
nOfBoundaries.

- The argument bcond should be the pointer to an integer array of [D][2] to specify the
boundary condition type b ∈[0, B) of the d-th dimensional lower (β = 0) or upper
(β = 1) boundary plane of the system domain in its element [d][β], if regular process
coordinate is specified. Otherwise, it can be NULL or can point anything.

- The argument bounds should be the (double) pointer to an integer array of [N][D][2]
being the shadow of Boundaries[][][] or to NULL. If regular process coordinate is spec-
ified by sdoms, all the elements of the array [m][d][β] are filled by init_subdomain_
actively() to have the boundary conditions of d-th dimensional lower (β = 0) and
upper (β = 1) boundary planes of subdomain m. Otherwise, the array should have
the boundary conditions for each boundary plane of each subdomain to be referred
to by init_subdomain_passively().

- The argument ftypes should be the pointer to an integer array of [F+1][7] being
the shadow of FieldTypes[] to have ε(f), el(f), eu(f), ebl(f), ebu(f), erl (f) and eru(f)
in [f][0:6] for all f ∈[0, F) being field-array identifiers, while [F][0] ≤0 as the
terminator.

- The argument cfields should be the pointer to an integer arryay of [C + 1] being
the shadow of BoundaryCommFields[] to have an index f ∈[0, F) of FieldTypes[] in
its element [c] to specify that ctypes[c][B][2][3] = BoundaryCommTypes[c][B][2][3] is
for field-array f, while [C] < 1 for oh3_init_() or [C] < 0 for oh3_init() as the
terminator.

- The argument ctypes should be the pointer to an integer array of [C][B][2][3] to
specify three parameters ef, et and s of sending/receiving boundary planes for the
type c downward (w = 0) and upward (w = 1) boundary communication through a
boundary plane of the boundary condition b in its element [c][b][w][0:2].

- The argument fsizes should be the (double) pointer to an integer array of [F][D][2]
or to NULL.  Its element [f][d][β] is filled with ϕld(f) (β = 0) or ϕud(f) (β = 1) to
specify the lower or upper terminal index of the field-array f.

- stats
repiter


<!-- Page 285 -->

verbose
The three arguments above are perfectly equivalent to those of the level-1 countger-
parts oh1_init[_]().

oh13_init_() We also have two additional API functions for initialization, namely oh13_init_() for
oh13_init()  Fortran and oh13_init() for C to perform what oh3_init[_]() does but excluding what
oh2_init[_]() does. Therefore, they have the following arguments equivalent to those of
oh1_init[_]() and/or oh3_init[_]().

- The following eleven are equivalent to those of oh1_init[_]() and oh3_init[_]().

sdid, nspec, maxfrac, nphgram, totalp, mycomm, nbor, pcoord, stats,
repiter, verbose

- The following two are equivalent to those of oh1_init[_]() but oh3_init[_]() does
not have them.

rcounts, scounts

- The following nine are equivalent to those of oh3_init[_]()’s own.

sdoms, scoord, nbound, bcond, bounds, ftypes, cfields, ctypes, fsizes

These four API functions almost simply call the initializer function init3() passing all
given arguments to it except for the followings.

- oh3_init_() and oh13_init_() pass the pointers to sdid, nphgram, totalp, nbor,
sdoms, bounds and fsizes rather than themselves. oh3_init_() also does so for
pbuf and pbase, while oh13_init_() does so for rcounts and scounts.

- oh3_init_() and oh13_init_() pass mycomm to mycommf of init3() while NULL is
passed through mycommc of init3() to keep it from allocation of MyCommC.

- oh3_init() and oh13_init() pass mycomm to mycommc of init3() while NULL is
passed through mycommf of init3() telling it that the body of MyCommF is not required.
It also casts the argument as S_mycommc pointer type, because mycomm is declared as a
void pointer to allow the simulator body to be completely unaware of the structure.

- Prior to calling init3(), oh3 init () lets specBase be 1 to indicate the species in
S_particle structures is represented by one-origin manner, while oh3 init() lets it
be 0 to indicate zero-origin numbering.

- oh3_init_() and oh13_init_() pass −1 to cfid argument while oh3_init() and
oh13_init() pass 0 to it, in order to obtain the boundary condition and field-array
identifiers b and f from those specified in bounds and ftypes, b′ and f ′, by b =
b′ + cfid and f = f ′ + cfid.

- oh3_init[_]() passes NULL to rcounts and scounts of init3() because they are
not required.  Similarly, oh13_init[_]() passes NULL to pbuf and pbase and 0 to
maxlocalp which are used only in the level-2 library.

- oh3_init[_]() passes 0 to the skip2 argument of init3() while oh13_init[_]()
passes 1 to it, to let init3() skip the initialization for the level-2 library iffthe
argument is 1.


<!-- Page 286 -->

void
oh3_init_(int *sdid, int *nspec, int *maxfrac, int *nphgram,
int *totalp, struct S_particle *pbuf, int *pbase, int *maxlocalp,
struct S_mycommf *mycomm, int *nbor, int *pcoord,
int *sdoms, int *scoord, int *nbound, int *bcond, int *bounds,
int *ftypes, int *cfields, int *ctypes, int *fsizes,
int *stats, int *repiter, int *verbose) {
specBase = 1;
init3(&sdid, *nspec, *maxfrac, &nphgram, &totalp, NULL, NULL, &pbuf, &pbase,
*maxlocalp, NULL, mycomm, &nbor, pcoord, &sdoms, scoord, *nbound,
bcond, &bounds, ftypes, cfields, -1, ctypes, &fsizes,
*stats, *repiter, *verbose, 0);
}
void
oh3_init(int **sdid, int nspec, int maxfrac, int **nphgram,
int **totalp, struct S_particle **pbuf, int **pbase, int maxlocalp,
void *mycomm, int **nbor, int *pcoord,
int **sdoms, int *scoord, int nbound, int *bcond, int **bounds,
int *ftypes, int *cfields, int *ctypes, int **fsizes,
int stats, int repiter, int verbose) {
specBase = 0;
init3(sdid, nspec, maxfrac, nphgram, totalp, NULL, NULL, pbuf, pbase,
maxlocalp, (struct S_mycommc*)mycomm, NULL, nbor, pcoord, sdoms,
scoord, nbound, bcond, bounds, ftypes, cfields, 0, ctypes, fsizes,
stats, repiter, verbose, 0);
}
void
oh13_init_(int *sdid, int *nspec, int *maxfrac, int *nphgram,
int *totalp, int *rcounts, int *scounts,
struct S_mycommf *mycomm, int *nbor, int *pcoord,
int *sdoms, int *scoord, int *nbound, int *bcond, int *bounds,
int *ftypes, int *cfields, int *ctypes, int *fsizes,
int *stats, int *repiter, int *verbose) {
init3(&sdid, *nspec, *maxfrac, &nphgram, &totalp, &rcounts, &scounts,
NULL, NULL, 0, NULL, mycomm, &nbor, pcoord, &sdoms, scoord, *nbound,
bcond, &bounds, ftypes, cfields, -1, ctypes, &fsizes,
*stats, *repiter, *verbose, 1);
}
void
oh13_init(int **sdid, int nspec, int maxfrac, int **nphgram,
int **totalp, int **rcounts, int **scounts,
void *mycomm, int **nbor, int *pcoord,
int **sdoms, int *scoord, int nbound, int *bcond, int **bounds,
int *ftypes, int *cfields, int *ctypes, int **fsizes,
int stats, int repiter, int verbose) {
init3(sdid, nspec, maxfrac, nphgram, totalp, rcounts, scounts, NULL, NULL,
0, (struct S_mycommc*)mycomm, NULL, nbor, pcoord, sdoms, scoord,
nbound, bcond, bounds, ftypes, cfields, 0, ctypes, fsizes,
stats, repiter, verbose, 1);
}


<!-- Page 287 -->

#### 4.7.4 init3()

init3()  The function init3(), called from oh3_init[_]() and oh13_init[_](), implements the
initialization for those API functions. The arguments of the function are almost same as
the union of those of oh3_init() and oh13_init(), but their mycomm is split into two
arguments mycommc and mycommf and there are two additions cfid and skip2 as discussed
in §4.7.3.


void
init3(int **sdid, int nspec, int maxfrac, int **nphgram,
int **totalp, int **rcounts, int **scounts,
struct S_particle **pbuf, int **pbase, int maxlocalp,
struct S_mycommc *mycommc, struct S_mycommf *mycommf,
int **nbor, int *pcoord, int **sdoms, int *scoord,
int nbound, int *bcond, int **bounds, int *ftypes,
int *cfields, int cfid, int *ctypes, int **fsizes,
int stats, int repiter, int verbose, int skip2) {
int nn;
int (*sd)[OH_DIMENSION][2]=(int(*)[OH_DIMENSION][2])*sdoms;
double (*sdf)[OH_DIMENSION][2];
int (*sc)[2]=(int(*)[2])scoord;
int (*bc)[2]=(int(*)[2])bcond;
int (*bd)[OH_DIMENSION][2]=(int(*)[OH_DIMENSION][2])*bounds;
int (*ft)[OH_FTYPE_N]=(int(*)[OH_FTYPE_N])ftypes;
int (*ct)[2][OH_CTYPE_N]=(int(*)[2][OH_CTYPE_N])ctypes;
int d, n, m;


First, the function calls  its level-1 or level-2 counterpart init1() or init2() ac-
cording to the specification given by skip2 and then set it into excludeLevel2 so that
transbound3() refers to it to determine which of transbound1() or transbound2() should
be called.

if (skip2)
init1(sdid, nspec, maxfrac, nphgram, totalp, rcounts, scounts,
mycommc, mycommf, nbor, pcoord, stats, repiter, verbose);
else
init2(sdid, nspec, maxfrac, nphgram, totalp, pbuf, pbase, maxlocalp,
mycommc, mycommf, nbor, pcoord, stats, repiter, verbose);
excludeLevel2 = skip2;
nn = nOfNodes;


Next, we allocate shadow arrays of SubDomains[N][D][2] and/or Boundaries[N][D][2]
by mem_alloc() if the arguments sdoms and/or bounds point NULL. In addition, [0][0][0]
and [0][0][1] of the shadow of SubDomains[][][] are set to 0 and −1 to specify regular process
coordinate.

if (!sd) {
sd = (int(*)[OH_DIMENSION][2])
(*sdoms = (int*)mem_alloc(sizeof(int), nn*OH_DIMENSION*2,
"SubDomains"));
sd[0][OH_DIM_X][OH_LOWER] = 0;  sd[0][OH_DIM_X][OH_UPPER] = -1;
}
if (!bd)


<!-- Page 288 -->

bd = (int(*)[OH_DIMENSION][2])
(*bounds = (int*)mem_alloc(sizeof(int), nn*OH_DIMENSION*2,
"Boundaries"));


Next we  initialize Adjacent[d][β] for  all d ∈[0, D) and β ∈{0, 1} referring to
∑D−1
DstNeighbors[] whose element  [k] where k =    i=0  νi3i has rk = rank(π0+ν0−1,
. . . , πD−1+νD−1−1) or −(rk+1) where (π0, . . . , πD−1)  is the coordinate of the local
node in the D-dimensional process coordinate space.  Since Adjacent[d][β] should have
rank(π′0, . . . π′D−1) where π′d = πd +2β −1 and π′i = πi for all i ̸= d, i.e., νd = 2β and νi = 1
for all i ̸= d, it should be set to the following.

∑           D−1∑
k(d, β) =     3i + 2β · 3d =     3i −3d + 2β · 3d

0≤i<D              i=0
i̸=d
{
−3d  β = 0               = (3D −1)/2 + (2β −1)3d = ⌊3D/2⌋+
3d   β = 1
{
r′k            r′k ≥0                            r′k = DstNeighbors[k]     rk =                                                     −(r′k + 1)   r′k < 0
Adjacent[d][β] = r′k(d,β)

Note that if the local node has a non-existent neighbor in DstNeighbors[k], it was set to
−(N + 1) by init1() and thus its correspondent in Adjacent[][] is set to N.

for (d=0,n=1,m=OH_NEIGHBORS>>1; d<OH_DIMENSION; d++,n*=3) {
int nl=DstNeighbors[m-n], nu=DstNeighbors[m+n];
Adjacent[d][OH_LOWER] = nl<0 ? -(nl+1) : nl;
Adjacent[d][OH_UPPER] = nu<0 ? -(nu+1) : nu;
}

Next  if *sdoms[0][0][0] >  *sdoms[0][0][1] meaning regular process coordinate, we
call init_subdomain_actively() to  initialize  *sdoms[][][] and  *bounds[][][].   Other-
wise,  i.e., irregular process coordinate, we call init_subdomain_passively() to create
SubDomainDesc[]. Both functions also initialize Grid[3].

if (sd[0][OH_DIM_X][OH_LOWER]>sd[0][OH_DIM_X][OH_UPPER])
init_subdomain_actively(sd, sc, pcoord, bc, bd, nbound, -cfid);
else
init_subdomain_passively(sd, bd, nbound, -cfid);


Then we allocate SubDomains[N][D][2] and Boundaries[N][D][2] by malloc() and then
copy their shadows *sdoms[][][] and *bounds[][][] to them by memcpy(). We also allocate
SubDomainsFloat[N][D][2] being grid-size-aware counterpart of SubDomains[][][] and copy
*sdoms[][][] into it with integer/floating-point conversion to make it has the default coordi-
nate values with grid size γd = 1 for all d ∈[0, D). After that, if cfid = −1 meaning that
*bounds[][][] ∈[1, B], all elements of Boundaries[][][] are decremented so that they have
values in [0, B).

SubDomains = (int(*)[OH_DIMENSION][2])
mem_alloc(sizeof(int), nn*OH_DIMENSION*2, "SubDomains");
sdf = SubDomainsFloat =


<!-- Page 289 -->

(double(*)[OH_DIMENSION][2])
mem_alloc(sizeof(double), nn*OH_DIMENSION*2, "SubDomainsFloat");
Boundaries = (int(*)[OH_DIMENSION][2])
mem_alloc(sizeof(int), nn*OH_DIMENSION*2, "Boundaries");
memcpy(SubDomains, sd, sizeof(int)*nn*OH_DIMENSION*2);
for (n=0; n<nn; n++)  for (d=0; d<OH_DIMENSION; d++) {
sdf[n][d][OH_LOWER] = sd[n][d][OH_LOWER];
sdf[n][d][OH_UPPER] = sd[n][d][OH_UPPER];
}
memcpy(Boundaries, bd, sizeof(int)*nn*OH_DIMENSION*2);
bd = Boundaries;
if (cfid) {
for (n=0; n<nn; n++)  for (d=0; d<OH_DIMENSION; d++) {
bd[n][d][OH_LOWER]--;  bd[n][d][OH_UPPER]--;
}
}

Finally we call init_fields() to initialize field-array and boundary communication
descriptors.

init_fields(ft, cfields, cfid, ct, nbound, sd[myRank], fsizes);
}


#### 4.7.5 init_subdomain_actively()

init_subdomain_actively()  The  function  init_subdomain_actively(),  called  solely  from  init3(),  initializes
*sdoms[N][D][2] = sd[N][D][2] = {δ{l,u}d   (m)} and Grid[3] referring to scoord[D][2] =
sc[D][2] = {∆{l,u}d   } and pcoord[D] = {Πd}.   It also initializes *bounds[N][D][2] =
bd[N][D][2] to have a value in [b, B + b), where B  is given through nbound = nb and
b ∈{0, 1} is given through bbase, referring to bcond[D][2] = bc[D][2], for regular process
coordintate.


static void
init_subdomain_actively(int (*sd)[OH_DIMENSION][2], int sc[OH_DIMENSION][2],
int *pcoord, int bc[OH_DIMENSION][2],
int (*bd)[OH_DIMENSION][2], int nb, int bbase) {
int nn=nOfNodes, pqr=1;
int d, lu, i, j, k, x, y, z, n;


At first we set SubDomainDesc to NULL to indicate regular process coordinate, and then
initialize Grid[d] for all d ∈[0, D) as follows with γd = 1 for all d at initial.

- Define Grid[d].coord[β] = Grid[d].fcoord[β] = scoord[d][β] = ∆βd and Grid[d].n =
pcoord[n] = Πd simply.

- Let ∆d = (∆ud −∆ld), then define Grid[d].light.size = δmind  = ⌊∆d/Πd⌋also sim-
ply. Also define Grid[d].light.rfsize = 1/δmind   and Grid[d].light.rfsizeplus =
1/(δmind  + 1).
- Define Grid[d].light.thresh = Grid[d].light.fthresh = ∆−d = ∆ld + Π d− · δmind    .


<!-- Page 290 -->

- Let Π−d  =  Grid[d].light.n =  Πd −(∆d mod Πd),  then  Grid[d].size =
Grid[d].fsize = δmaxd  = ⌈∆d/Πd⌉is defined as;
{
δmind     Πd− = Πd                              δmaxd  =           −
δmind  + 1 Πd  ̸= Πd

- Finally, initialize Grid[d].gsize = γd = 1 and Grid[d].rgsize = 1/γd = 1 as default.

In addition, we check if Πd > 0 and ∆d > 0, or abort the execution by errstop(). Note that
if D < 3, we set Πd = Πd− = 1, γd = 1/γd = 1, ∆ld = ∆ud = ∆−d = 0 and δmaxd  = δmind  = 0
with their reciprocals for all d ≥D.

SubDomainDesc = NULL;
for (d=0; d<OH_DIMENSION; d++) {
int lo = Grid[d].coord[OH_LOWER] = sc[d][OH_LOWER];
int up = Grid[d].coord[OH_UPPER] = sc[d][OH_UPPER];
int size = up - lo;
int ave, nl;
Grid[d].fcoord[OH_LOWER] = lo;  Grid[d].fcoord[OH_UPPER] = up;
n = Grid[d].n = pcoord[d];
if (n<=0)
errstop("# of %c-nodes (%d) should be positive", Message.xyz[d], n);
if (size<=0)
errstop("upper edge of %c-coordinate (%d) should be greater than "
"lower edge (%d)", Message.xyz[d], up, lo);
ave = Grid[d].light.size = size/n;
Grid[d].light.rfsize = 1.0/(double)ave;
Grid[d].light.rfsizeplus = 1.0/(double)(ave+1);
nl =  Grid[d].light.n = n - size%n;
Grid[d].light.fthresh = (Grid[d].light.thresh = lo + nl * ave);
Grid[d].fsize = (Grid[d].size = n==nl ? ave : ave+1);
Grid[d].gsize = Grid[d].rgsize = 1.0;
pqr *= n;
}
for (; d<3; d++) {
Grid[d].n = Grid[d].light.n = 1;
Grid[d].coord[OH_LOWER] = Grid[d].coord[OH_UPPER] = 0;
Grid[d].fcoord[OH_LOWER] = Grid[d].fcoord[OH_UPPER] = 0.0;
Grid[d].size = Grid[d].light.size = Grid[d].light.thresh = 0;
Grid[d].fsize = Grid[d].light.rfsize
= Grid[d].light.rfsizeplus = Grid[d].light.fthresh = 0.0;
Grid[d].gsize = Grid[d].rgsize = 1.0;
}
∏D−1
We also check if N =   d=0 Πd, or abort the execution by errstop() with a message
appropriate to D. The other check is to confirm that bcond[d][β] = bc[d][β] ∈[b, B+b) for
all d ∈[0, D) and β ∈{0, 1} where b = bbase argument of the function which has 0 or
1 for C or Fortran simulator body respectively.  If the condition is not satisfied we abort
the execution by errstop() giving Message.xyz[d] and Message.loup[β] to produce an
appropriate error message.

if (pqr!=nn) {
if (OH_DIMENSION==1)
errstop("<# of x-nodes>(%d) should be eqal to <# of nodes>(%d)",


<!-- Page 291 -->

pcoord[0], nn);
else if (OH_DIMENSION==2)
errstop("<# of x-nodes>(%d) * <# of y-nodes>(%d) "
"should be eqal to <# of nodes>(%d)",
pcoord[0], pcoord[1], nn);
else
errstop("<# of x-nodes>(%d) * <# of y-nodes>(%d) * <# of z-nodes>(%d) "
"should be eqal to <# of nodes>(%d)",
pcoord[0], pcoord[1], pcoord[2], nn);
}
for (d=0; d<OH_DIMENSION; d++) {
for (lu=OH_LOWER; lu<=OH_UPPER; lu++) {
if (bc[d][lu]<bbase || bc[d][lu]>=nb+bbase)
errstop("system’s %s boundary condition for %c-coordinate %d is "
"invalid",
Message.loup[lu], Message.xyz[d], bc[d][lu]);
}
}

The  last operation  is to  fill *sdoms[N][D][β] =  sd[N][D][β] =  {δ{l,u}d   (m))} and
*bounds[N][D][β] = bd[N][D][β] as follows.

m = rank(π0(m), . . . , πD−1(m))
m−d = rank(π0(m), . . . , πd(m)−1, . . . , πD−1(m))                  {
∆ld      πd(m) = 0                           δld(m) =                               δud(m−d )  πd(m) > 0
{
δmind       πd(m) < Πd−                     δud(m) = δld(m) +                 −
δmind  + 1  πd(m) ≥Πd
{
bc[d][0]  πd(m) = 0
bd[m][d][0] =
b       πd(m) > 0
{
bc[d][1]  πd(m) = Πd −1
bd[m][d][1] =
b       πd(m) < Πd −1

The definitions of δld(m) and δud(m) above are corresponding to the implementation but are
different from those shown in §3.6.1. Their equivalence is, however, proved as follows.
{
δmind       π < Πd−     δd(π) =              −
δmind  + 1  π ≥Πd
{
∆ld      πd(m) = 0    δud(m) = δd(πd(m)) + δld(m) = δd(πd(m)) +                                              δud(m−d )  πd(m) > 0
{                   πd(m)∑               πd(m)∑                                               0              πd(m) < Π d−      = ∆ld +      δd(π) = ∆ld +      δmind  +            −          −
πd(m) + 1 −Π d   πd(m) ≥Π d                  π=0              π=0
= ∆ld + (πd(m) + 1)δmind  + max(0, πd(m) + 1 −Π d− )
δld(m) = ∆ld + πd(m)δmind  + max(0, πd(m) −Πd− )           {
πd(m)δmind                  πd(m) ≤Πd−      = ∆ld +                   −          −
πd(m)δmind  + (πd(m) −Π d )  πd(m) > Πd
m+d = rank(π0(m), . . . , πd(m)+1, . . . , πD−1(m))
δud(m) = δld(m+d )    (πd(m) < Πd −1)


<!-- Page 292 -->

δud(m) = ∆ld + Πdδmind  + max(0, Πd −Π d− )
= ∆ld + Πd⌊(∆ud −∆ld)/Πd⌋+ Πd −(Πd −((∆ud −∆ld) mod Πd))
= ∆ld + (∆ud −∆ld) −((∆ud −∆ld) mod Πd) + ((∆ud −∆ld) mod Πd)
= ∆ud    (πd(m) = Πd −1)       {
δld(m+d )  πd(m) < Πd −1  δud(m) =
∆ud      πd(m) = Πd −1


for (i=0,z=Grid[OH_DIM_Z].coord[OH_LOWER],n=0; i<Grid[OH_DIM_Z].n; i++) {
int bot=z, top=z+Grid[OH_DIM_Z].light.size;
int bzlo = i==0 && OH_DIMENSION>OH_DIM_Z ?
bc[OH_DIM_Z][OH_LOWER] : bbase;
int bzup = i==Grid[OH_DIM_Z].n-1  && OH_DIMENSION>OH_DIM_Z ?
bc[OH_DIM_Z][OH_UPPER] : bbase;
if (i>=Grid[OH_DIM_Z].light.n)  top++;
z = top;
for (j=0,y=Grid[OH_DIM_Y].coord[OH_LOWER]; j<Grid[OH_DIM_Y].n; j++) {
int south=y, north=y+Grid[OH_DIM_Y].light.size;
int bylo = j==0 && OH_DIMENSION>OH_DIM_Y ?
bc[OH_DIM_Y][OH_LOWER] : bbase;
int byup = j==Grid[OH_DIM_Y].n-1  && OH_DIMENSION>OH_DIM_Y ?
bc[OH_DIM_Y][OH_UPPER] : bbase;
if (j>=Grid[OH_DIM_Y].light.n)  north++;
y = north;
for (k=0,x=Grid[OH_DIM_X].coord[OH_LOWER]; k<Grid[OH_DIM_X].n; k++,n++) {
int west=x, east=x+Grid[OH_DIM_X].light.size;
if (k>=Grid[OH_DIM_X].light.n)  east++;
x = east;
sd[n][OH_DIM_X][OH_LOWER] = west;
sd[n][OH_DIM_X][OH_UPPER] = east;
bd[n][OH_DIM_X][OH_LOWER] = bd[n][OH_DIM_X][OH_UPPER] = bbase;
if (OH_DIMENSION>OH_DIM_Y) {
sd[n][OH_DIM_Y][OH_LOWER] = south;
sd[n][OH_DIM_Y][OH_UPPER] = north;
bd[n][OH_DIM_Y][OH_LOWER] = bylo;
bd[n][OH_DIM_Y][OH_UPPER] = byup;
}
if (OH_DIMENSION>OH_DIM_Z) {
sd[n][OH_DIM_Z][OH_LOWER] = bot;
sd[n][OH_DIM_Z][OH_UPPER] = top;
bd[n][OH_DIM_Z][OH_LOWER] = bzlo;
bd[n][OH_DIM_Z][OH_UPPER] = bzup;
}
}
bd[n-Grid[OH_DIM_X].n][OH_DIM_X][OH_LOWER] = bc[OH_DIM_X][OH_LOWER];
bd[n-1][OH_DIM_X][OH_UPPER] = bc[OH_DIM_X][OH_UPPER];
}
}
}


<!-- Page 293 -->

#### 4.7.6 init_subdomain_passively()

init_subdomain_passively()  The function init_subdomain_passively(), called solely from init3(), initialize Grid[3]
and SubDomainDesc[N] after allocating it by mem_alloc(), referring to *sdoms[N][D][2] =
sd[N][D][2] = {δ{l,u}d   (m)}.  It also checks the consistency of sd[][][], *bounds[N][D][2] =
bd[N][D][2], nb = B and bbase = b ∈{0, 1}.


static void
init_subdomain_passively(int (*sd)[OH_DIMENSION][2],
int (*bd)[OH_DIMENSION][2], int nb, int bbase) {
int nn=nOfNodes;
struct S_subdomdesc *sdd = SubDomainDesc =
(struct S_subdomdesc*)mem_alloc(sizeof(struct S_subdomdesc), nn,
"SubDomainDesc");
int min[OH_DIMENSION], max[OH_DIMENSION];
int smin[OH_DIMENSION], smax[OH_DIMENSION];
int me=myRank;
int i, d, dd, lu, l;
int lo[OH_DIMENSION-1], up[OH_DIMENSION-1], h[OH_DIMENSION-1];

First we  copy  sd[m][d][β] =  δβd (m)  to  SubDomainDesc[m].coord[d].c[β]  and
SubDomainDesc[m].coord[d].fc[β], and then  calculate ∆ld =  minm{δld(m)}, ∆ud =
maxm{δud(m)}, δmind  = minm{δud(m) −δld(m)} and δmaxd  = maxm{δud(m) −δld(m)}. We
also initialize SubDomainDesc[m].coord[d].n = 0 for all m ∈[0, N) and d ∈[0, D) to make
them have a some specific value (but not being referred to) because the set-up opreation
discussed later will leave them unchanged for m ̸= SubDomainDesc[m].h[d]. The other but
necessary initialization is SubDomainDesc[m].id = m for all m ∈[0, N) to keep the subdo-
main identity after the sorting also discussed later. In addition we check if δld(m) < δud(m),
and if bd[m][d][β] ∈[b, B+b) where b = 0 for C or b = 1 for Fortran simulator body, or
abort the execution by errstop() giving Message.xyz[d] and Message.loup[β] to produce
appropriate error messages.

for (d=0; d<OH_DIMENSION; d++) {
min[d] = sd[0][d][OH_LOWER];  max[d] = sd[0][d][OH_UPPER];
smin[d] = smax[d] = max[d] - min[d];
}
for (i=0; i<nn; i++) {
for (d=0; d<OH_DIMENSION; d++) {
int lo=sd[i][d][OH_LOWER], up=sd[i][d][OH_UPPER], n=up-lo;
sdd[i].coord[d].fc[OH_LOWER] = (sdd[i].coord[d].c[OH_LOWER] = lo);
sdd[i].coord[d].fc[OH_UPPER] = (sdd[i].coord[d].c[OH_UPPER] = up);
sdd[i].coord[d].n = 0;
if (n<smin[d])  smin[d] = n;
if (n>smax[d])  smax[d] = n;
if (lo<min[d])  min[d] = lo;
if (up>max[d])  max[d] = up;
if (n<=0)
errstop("subdomain %d has %c-coordinate lower boundary %d "
"not less than upper boundary %d", i, Message.xyz[d], lo, up);
for (lu=OH_LOWER; lu<=OH_UPPER; lu++) {
if (bd[i][d][lu]<bbase || bd[i][d][lu]>=nb+bbase)
errstop("rank-%d’s %s boundary condition for %c-coordinate %d is "


<!-- Page 294 -->

"invalid",
i, Message.loup[lu], Message.xyz[d], bd[i][d][lu]);
}
}
sdd[i].id = i;
}

Next we set elements of Grid[d]  for  all d ∈[0, D) as size = fsize = δmaxd    ,
light.size = δmind    , coord[0] = fcoord[0] = ∆ld, coord[1] = fcoord[1] = ∆ud, gsize =
γd = 1 and rgsize = 1/γd =  1.  The other elements n = Πd, light.n = Π d− ,
light.thresh = light.fthresh = ∆−d , light.rfsize = 1/δmind   and light.rfsize =
1/(δmind  + 1) are set to 0 but they are never referred to.  Note that  if D < 3, we set
Πd = Π d− = 1, ∆ld = ∆ud = ∆−d = 0, γd = 1/γd = 1, and δmaxd  = δmind  = 0 with their
reciprocals for all d ≥D, after the loop for d ∈[0, D).
Then we check if the boundary coordinates of local node are consistent with those of
its neighbors.  Let mβd be Adjacent[d][β] the d-th dimensional lower (β = 0) or upper
(β = 1) neighbor of the local node n. Unless mβd = N meaning the neighbor does not exist
or bd[n][d][β] ̸= b meaning the boundary between the local node and it is special, δβe (n),
δβe (mβd) and δ1−βe  (mβd) should satisfy the following where δβd (m) = δ{l,u}[β]d         .

δ1−βd  (mβd) = δβd (n) ∨δ1−βd  (mβd) = δβd (n) + (∆ud −∆ld)
∨δ1−βd  (mβd) = δβd (n) −(∆ud −∆ld)
δγe (mβd) = δγe (n)       (e ∈[0, D) −{d}, γ ∈{0, 1})

If a condition above is not satisfied, we abort the execution by local_errstop() giving it
elements of Message to produce appropriated error messages.

for (d=0; d<OH_DIMENSION; d++) {
Grid[d].fsize = (Grid[d].size = smax[i]);
Grid[d].light.size = smin[d];
Grid[d].light.rfsize = Grid[d].light.rfsizeplus = 0.0;
Grid[d].fcoord[OH_LOWER] = (Grid[d].coord[OH_LOWER] = min[d]);
Grid[d].fcoord[OH_UPPER] = (Grid[d].coord[OH_UPPER] = max[d]);
Grid[d].n = Grid[d].light.n = 0;    /* never referred but ... */
Grid[d].light.thresh = 0;  Grid[d].light.fthresh = 0.0;
Grid[d].gsize = Grid[d].rgsize = 1.0;
for (lu=OH_LOWER; lu<=OH_UPPER; lu++) {
int n=Adjacent[d][lu];
if (n==nn || bd[me][d][lu]!=bbase) continue;
for (dd=0; dd<OH_DIMENSION; dd++) {
if (d==dd) {
int diff = sd[n][dd][OH_UPPER-lu] - sd[me][dd][lu];
int dsize = max[dd] - min[dd];
if (diff!=0 && diff!=dsize && diff!=-dsize)
local_errstop("rank-%d and its %c-%s neighbor rank-%d have "
"incompatible %s/%s boundaries of %c-coordinate "
"%d and %d",
me, Message.xyz[d], Message.loup[lu], n,
Message.loup[lu], Message.loup[OH_UPPER-lu],
Message.xyz[dd],
sd[me][dd][lu], sd[n][dd][OH_UPPER-lu]);
} else {


<!-- Page 295 -->

for (l=OH_LOWER; l<=OH_UPPER; l++) {
if (sd[n][dd][l]!=sd[me][dd][l])
local_errstop("rank-%d and its %c-%s neighbor rank-%d have "
"incompatible %s boundary of %c-coordinate "
"%d and %d",
me, Message.xyz[d], Message.loup[lu], n,
Message.loup[l], Message.xyz[dd],
sd[me][dd][l], sd[n][dd][l]);
}
}
}
}
}
for (; d<3; d++) {
Grid[d].n = Grid[d].light.n = 1;
Grid[d].coord[OH_LOWER] = Grid[d].coord[OH_UPPER] = 0;
Grid[d].fcoord[OH_LOWER] = Grid[d].fcoord[OH_UPPER] = 0.0;
Grid[d].size = Grid[d].light.size = Grid[d].light.thresh = 0;
Grid[d].fsize = Grid[d].light.rfsize
= Grid[d].light.rfsizeplus = Grid[d].light.fthresh = 0.0;
Grid[d].gsize = Grid[d].rgsize = 1.0;
}

Finally, we sort SubDomainDesc[N] and set its elements so that oh3_map_particle_to_
subdomain() find the subdomain in which a particle resides. The sorting is performed by
qsort() which compares two elements by comp_xyz() which defines the total ordering of
nodes with the irreflective relation m1 ≺m2 for m1 ̸= m2 as follws.

δmd (m) = δld(m) + δud(m)
m1 d= m2 ⇔δld(m1) = δld(m2) ∧δud(m1) = δud(m2)
m1 d≺m2 ⇔δmd (m1) < δmd (m2) ∨(δmd (m1) = δmd (m2) ∧δld(m1) < δld(m2)) ∨
(δmd (m1) = δmd (m2) ∧δld(m1) = δld(m2) ∧δud(m1) < δud(m2))    (d < D)
m1 D≺m2 ⇔m1 < m2
d−1∧
m1 ≺m2 ⇔∃d ∈[0, D] :   (m1 e= m2) ∧m1 d≺m2
e=0

The definition above assures that, if D = 3, for a wall (or a set of them) of subdomains
which share δl0(m) and δu0 (m), the members in it constitutes a sequence in the sorted
SubDomainDesc[].  It is also assured that for a pillar (or a set of them) of subdomains in
a wall (set) which also share δl1(m) and δu1 (m), the members in it constitues a sequence.
Therefore, we let SubDomainDesc[m].coord[d].h have the head of the wall (d = 0) and pillar
(d = 1) which are the first members of the wall/pillar to which the m-th subdomain belongs
to. We also let SubDomainDesc[h].coord[d].n have the number of members in a wall/pillar
whose head is h. More specifically, we let h and n of SubDomainDesc[m].coord[d] for all
m ∈[0, N) and d ∈[0, D−1) be the followings.

i(m) = SubDomainDesc[m].id
Md(m) = {k | ∀e ≤d  : δle(i(k)) = δle(i(m)), δue (i(k)) = δue (i(m))}
SubDomainDesc[m].coord[d].h = min(Md(m))


<!-- Page 296 -->

{
|Md(m)| m = min(Md(m))
SubDomainDesc[m].coord[d].n =
0     m ̸= min(Md(m))

For the setting above, we scan SubDomainDesc[] keeping track hd = min(Md(m)) in h[d]
and δ{l,u}d   (i(hd)) in {lo, up}[d] with hd = 0 at initial. Then each time we find m such
that δld(i(m)) ̸= lo[d] or δud(i(m)) ̸= up[d] and thus the head of a new wall/pillar, for all
e such that e ∈[d, D−1) we let n of the current d-th dimensional head he be |Md(he)| =
m −he, and then he ←m, lo[e] ←δle(i(m)) and up[e] ←δue (i(m)). On the other hand, if
δle(i(m)) = lo[e] and δue (i(m)) = up[e] hold for all e ∈[0, d], we let h of m be he.
On the other hand, we let h = m and n = 1 for SubDomainDesc[m].coord[D−1] for all
m ∈[0, N) because a subdomain in a pillar will not (and should not) share its (D−1)-th
boundary coordinates with other subdomains in the pillar.

qsort(sdd, nn, sizeof(struct S_subdomdesc), comp_xyz);
for (d=0; d<OH_DIMENSION-1; d++) {
sdd[0].coord[d].h = h[d] = 0;
lo[d] = sdd[0].coord[d].c[OH_LOWER];
up[d] = sdd[0].coord[d].c[OH_UPPER];
}
for (i=1; i<nn; i++) {
for (d=0; d<OH_DIMENSION-1; d++) {
if (lo[d]!=sdd[i].coord[d].c[OH_LOWER] ||
up[d]!=sdd[i].coord[d].c[OH_UPPER]) {
for (dd=d; dd<OH_DIMENSION-1; dd++) {
sdd[h[dd]].coord[dd].n = i - h[dd];
sdd[i].coord[dd].h = h[dd] = i;
lo[dd] = sdd[i].coord[dd].c[OH_LOWER];
up[dd] = sdd[i].coord[dd].c[OH_UPPER];
}
break;
} else {
sdd[i].coord[d].h = h[d];
}
}
sdd[i].coord[OH_DIMENSION-1].n = 1;  sdd[i].coord[OH_DIMENSION-1].h = i;
}
for (d=0; d<OH_DIMENSION-1; d++)  sdd[h[d]].coord[d].n = nn - h[d];
}


#### 4.7.7 comp_xyz()

comp_xyz()  The  function comp_xyz(),  called  solely from qsort()  called  in init_subdomain_
passively(), compares two elements of SubDomainDesc[N] pointed by  its arguments
aa and bb to return −1  if ma ≺mb or 1 otherwise where *aa = SubDomainDesc[ma],
*bb = SubDomainDesc[mb] and ≺is the irreflective relation defined in §4.7.6.


static int
comp_xyz(const void* aa, const void* bb) {
struct S_subdomdesc *a=(struct S_subdomdesc*)aa, *b=(struct S_subdomdesc*)bb;
int d;

for (d=0; d<OH_DIMENSION; d++) {


<!-- Page 297 -->

if (a->coord[d].c[OH_LOWER]+a->coord[d].c[OH_UPPER]<
b->coord[d].c[OH_LOWER]+b->coord[d].c[OH_UPPER])  return(-1);
if (a->coord[d].c[OH_LOWER]+a->coord[d].c[OH_UPPER]>
b->coord[d].c[OH_LOWER]+b->coord[d].c[OH_UPPER])  return(1);
if (a->coord[d].c[OH_LOWER]<b->coord[d].c[OH_LOWER])  return(-1);
if (a->coord[d].c[OH_LOWER]>b->coord[d].c[OH_LOWER])  return(1);
if (a->coord[d].c[OH_UPPER]<b->coord[d].c[OH_UPPER])  return(-1);
if (a->coord[d].c[OH_UPPER]>b->coord[d].c[OH_UPPER])  return(1);
}
return(a->id<b->id ? -1 : 1);
}


#### 4.7.8 Macro Field_Disp()

Field_Disp()  The macro Field Disp(f, i0, i1, i2), used in init_fields(), set_field_descriptors()
and set_border_comm(), is replaced with the one-dimensional index a = fdisp(f, i0, i1, i2)
of [i2][i1][i0][0] in an array of [Φ2(f)][Φ1(f)][Φ0(f)][ε(f)] where ε(f) = FieldDesc[f].esize
and Φd(f) = FieldDesc[f].size[d], providing id = 0 and Φd(f) = 1 for all d ≥D, as
follows.
aD−1 = iD−1    ad = ad+1 · Φd(f) + id    a = a0 · ε(f)




#if OH_DIMENSION==1
#define Field_Disp(F,X,Y,Z) (FieldDesc[F].esize * (X))
#elif OH_DIMENSION==2
#define Field_Disp(F,X,Y,Z)\
(FieldDesc[F].esize *\
((X) + FieldDesc[F].size[OH_DIM_X] * (Y)))
#else
#define Field_Disp(F,X,Y,Z)\
(FieldDesc[F].esize *\
((X) + FieldDesc[F].size[OH_DIM_X] *\
((Y) + FieldDesc[F].size[OH_DIM_Y] * (Z))))
#endif


#### 4.7.9 init_fields()

init_fields()  The function init_fields(),  called  solely from init3(), makes the substances  of
FieldTypes[F][7], BoundaryCommFields[C] and BoundaryCommFields[C][B][2][3], by copy-
ing the contents of their shadows given through its and oh3_init()’s arguments ft[][] =
ftypes[][], cf[] = cfields[] and ct[][][] = ctypes[][][] to them, referring to its and init3()’s
argument cfid being 0 for C or −1 for Fortan simulator body. Then the funcion cre-
ates and initializes FieldDesc[F], BorderExc[C][2][D][2] and the array pointed by *fsizes
argument of the function and oh3_init(), referring to its and oh3_init()’s argument
nb = nbound = B.


static void
init_fields(int (*ft)[OH_FTYPE_N], int *cf, int cfid, int (*ct)[2][OH_CTYPE_N],
int nb, int sd[OH_DIMENSION][2], int **fsizes) {
struct S_flddesc *fd;
struct S_borderexc (*bx)[2][OH_DIMENSION][2];


<!-- Page 298 -->

int (*fs)[OH_DIMENSION][2]=(int(*)[OH_DIMENSION][2])*fsizes;
int nf, ne;
int f, e, b, d, lu, i, *tmp;


First, we set the array size variable nOfBoundaries = B, as well as nOfFields = F
being the number of leading elements of ft[][] having positive ε(f), and nOfExc = C being
non-negative (cfid = 0) or positive (cfid = −1) leading elements of cf[]. Note that the
terminator of cf[] is negative (−1) regardless of cfid if OH_POS_AWARE is defined to mean
cf[] is BoundaryCommFields[] allocated and initialized by level-4p initializer init4p() and
thus have zero-origin indices of ft[][] = FieldTypes[].

nOfBoundaries = nb;
for (nf=0; ft[nf][OH_FTYPE_ES]>0; nf++);
nOfFields = nf;
#ifdef OH_POS_AWARE
for (ne=0; cf[ne]>=0; ne++);
#else
for (ne=0; cf[ne]+cfid>=0; ne++);
#endif
nOfExc = ne;


Next, we allocate FieldDesc[F] by mem_alloc().  We also allocate substances of
FieldTypes[F][7], BoundaryCommTypes[C][B][2][3] and BoundaryCommFields[C], whose el-
ements are then copied from their shadows ft[][], ct[][][][] and cf[] by memcpy() for first two
and by an explict for-loop for the last to make its elements zero-origined, unless OH_POS_
AWARE is defined to mean they are allocated and initialized by level-4p initializer init4p()
with one additional element for each. The other allocation takes place for *fsizes[F][D][2]
to have ϕ{l,u}d   (f) if the double pointer points NULL.

FieldDesc = fd = (struct S_flddesc*)mem_alloc(sizeof(struct S_flddesc), nf,
"FieldDesc");
#ifndef OH_POS_AWARE
FieldTypes = (int(*)[OH_FTYPE_N])
mem_alloc(sizeof(int), nf*OH_FTYPE_N, "FieldTypes");
BoundaryCommTypes  = (int(*)[2][OH_CTYPE_N])
mem_alloc(sizeof(int), ne*nb*2*OH_CTYPE_N,
"BoundaryCommTypes");
memcpy(FieldTypes, ft, sizeof(int)*nf*OH_FTYPE_N);
memcpy(BoundaryCommTypes, ct, sizeof(int)*ne*nb*2*OH_CTYPE_N);
ft = FieldTypes;  ct = BoundaryCommTypes;

tmp = (int*)mem_alloc(sizeof(int), ne, "BoundaryCommFields");
for (e=0; e<nOfExc; e++)  tmp[e] = cf[e] + cfid;
BoundaryCommFields = cf = tmp;
#endif

if (!fs)
fs = (int(*)[OH_DIMENSION][2])
(*fsizes = (int*)mem_alloc(sizeof(int), nf*OH_DIMENSION*2,
"FieldSizes"));


<!-- Page 299 -->

Next we scan FieldTypes[F][7] to set FieldDesc[f].esize = ε(f), and temporalily
FieldDesc[f].ext[0] = eminl  (f) = min(el(f), ebl(f), erl (f)) and FieldDesc[f].ext[1] =
emaxu  (f) = max(eu(f), ebu(f), eru(f)).

for (f=0; f<nf; f++) {
int lo=ft[f][OH_FTYPE_LO], up=ft[f][OH_FTYPE_UP];
fd[f].esize = ft[f][OH_FTYPE_ES];
for (lu=OH_FTYPE_BL; lu<OH_FTYPE_RU; lu+=2) {
int lot=ft[f][lu], upt=ft[f][lu+1];
if (lot<lo)  lo = lot;
if (upt>up)  up = upt;
}
fd[f].ext[OH_LOWER] = lo;  fd[f].ext[OH_UPPER] = up;
}
Next we scan BoundaryCommTypes[C][B][2][3] to calculate eγ{l,u}(f) as follows and then
let eminl  (f) ←min(eminl   (f), eγl (f)) and emaxu  (f) ←max(emaxu   (f), eγu(f)).

Γ(f) = {c | BoundaryCommFields[c] = f}
{
e  s ̸= 0
λ(e, s) =
0  s = 0
s↓(b, c) = BoundaryCommTypes[c][b][0][2]
s↑(b, c) = BoundaryCommTypes[c][b][1][2]
e↓f(b, c) = λ(BoundaryCommTypes[c][b][0][0], s↓(b, c))
e↓t (b, c) = λ(BoundaryCommTypes[c][b][0][1], s↓(b, c))
e↑f(b, c) = λ(BoundaryCommTypes[c][b][1][0], s↑(b, c))
e↑t (b, c) = λ(BoundaryCommTypes[c][b][1][1], s↑(b, c))
t (b, c)})               eγl (f) = b∈[0,B),c∈Γ(f)({e↓min        f(b, c)} ∪{e↑
t (b, c) + s↓(b, c)} ∪{e↑f(b, c) + s↑(b, c)})               eγu(f) = b∈0B,c∈Γ(f)({e↓max

That is, eγl (f) is the minimum local coordinate among the bottom of non-empty sending
boundary plane sets for downward communication and that of receiving counterparts for
upward, for all boundary conditions and for all communication types for the field-array
f.  Similarly, eγu(f) −1 is the maximum local coordinate among the top of non-empty
sending boundary plane sets for upward communication and that of receiving counterparts
for downward, for all boundary condtions and for all communication types for the field-array
f.
In the scan of BoundaryCommTypes[c] for all c ∈[0, C), we check if BoundaryCommFields
[c] < F or abort the execution by errstop().

for (e=0,i=0; e<ne; e++) {
int f=cf[e];
int lo, up;
if (f>=nf)
errstop("boundary communication #%d cannot be defined for "
"undefined field #%d", e-cfid, f-cfid);
lo = fd[f].ext[OH_LOWER];  up = fd[f].ext[OH_UPPER];
for (b=0; b<nb; b++,i++) {


<!-- Page 300 -->

int sl=ct[i][OH_LOWER][OH_CTYPE_SIZE];
int su=ct[i][OH_UPPER][OH_CTYPE_SIZE];
int lo1=ct[i][OH_LOWER][OH_CTYPE_FROM];
int lo2=ct[i][OH_UPPER][OH_CTYPE_TO];
int up1=ct[i][OH_LOWER][OH_CTYPE_TO]   + sl;
int up2=ct[i][OH_UPPER][OH_CTYPE_FROM] + su;
if (sl && lo1<lo)  lo = lo1;
if (su && lo2<lo)  lo = lo2;
if (sl && up1>up)  up = up1;
if (su && up2>up)  up = up2;
}
fd[f].ext[OH_LOWER] = lo;  fd[f].ext[OH_UPPER] = up;
}

Next, we do the followings to define the required size of field-array f ∈[0, F) for all its
dimensions d ∈[0, D).

*fsizes[f][d][0] = ϕld(f) = eminl  (f)
*fsizes[f][d][1] = ϕud(f) = δmaxd  + emaxu  (f) + cfid
FieldDesc[F].size[d] = Φd(f) = δmaxd  + (emaxu  (f) −eminl   (f))

Note that we add cfid ∈{0, −1} to give the upper limit of the field-array to a Fortan
simulator body while the positive extent is given to a C simulator body.

for (f=0; f<nf; f++) {
int lo=fd[f].ext[OH_LOWER], up=fd[f].ext[OH_UPPER];
for (d=0; d<OH_DIMENSION; d++) {
fs[f][d][OH_LOWER] = lo;
fs[f][d][OH_UPPER] = (Grid[d].size+cfid) + up;
fd[f].size[d] = Grid[d].size + (up - lo);
}
}

Next, we let FieldDesc[f].{bc, red}.base be fdisp(f, exl (f), exl (f), exl (f)) where x ∈
{b, r} for all f ∈[0, F). We also set FieldDesc[f].{bc, red}.size[0] for the primary domain
by set_field_descriptors() giving it the argument of this function ft[][] = *ftypes[][]
and sd[][] = SubDomains[n][][] for the local node n, letting its argument ps = 0 to indicate
that the target subdomain is primary.

for (f=0; f<nf; f++) {
int bl = ft[f][OH_FTYPE_BL];
int rl = ft[f][OH_FTYPE_RL];
fd[f].bc.base  = Field_Disp(f, bl, bl, bl);
fd[f].red.base = Field_Disp(f, rl, rl, rl);
}
set_field_descriptors(ft, sd, 0);


Finally, we allocate BorderExc[C][2][D][2] and set its elements [c][0][d][β] of the primary
subdomain for all c ∈[0, C), d ∈[0, D) and β ∈{0, 1} by set_border_exchange() giving
it c and letting its argument ps = 0 to indicate that the target subdomain is primary.
Note that the last argument type is usually MPI_DOUBLE but can be MPI_LONG_LONG_INT
if OH_POS_AWARE is defined to mean position-aware particle management is in effect and
c = C −1 for the per-grid histogramrather than user-defined field-array. We also initialize


<!-- Page 301 -->

BorderExc[c][1][d][β] for the secondary subdomain by clear_border_exchange() setting
{send, recv}.deriv = 0 before calling it to keep it from freeing the undefined data-type in
{send, recv}.type.

BorderExc = bx =
(struct S_borderexc(*)[2][OH_DIMENSION][2])
mem_alloc(sizeof(struct S_borderexc), ne*2*OH_DIMENSION*2, "BorderExc");

for (e=0; e<ne; e++) {
for (d=0; d<OH_DIMENSION; d++) {
for (lu=0; lu<2; lu++)
bx[e][1][d][lu].send.deriv = bx[e][1][d][lu].recv.deriv = 0;
}
#ifdef OH_POS_AWARE
set_border_exchange(e, 0, e<ne-1 ? MPI_DOUBLE : MPI_LONG_LONG_INT);
#else
set_border_exchange(e, 0, MPI_DOUBLE);
#endif
}
clear_border_exchange();
}


#### 4.7.10 set_field_descriptors()

set_field_descriptors()  The function set_field_descriptors(), called from init_fields() and transbound3(),
sets FieldDesc[f].{bc, red}.size[p] = σ(f, {b, r}, m) for all f ∈[0, F) where p = ps
argument of the function to indicate primary subdomain m = n if p = 0, or secondary
subdomain m = parent(n) otherwise, for the local node n. The function refers ε(f), ebu(f)
and eru(f) through its argument ft[F][7] = FieldTypes[F][7], and δld(m) and δld(m) through
the argument sd[D][2] = SubDomains[m][D][2]. The value of σ(f, {b, r}, m) is calculated by
Field_Disp() as follows.
{
δud(m) −δld(m) + e{b,r}u   (f) −1  d < D                              υd(m) =
0                          d ≥D
σ(f, {b, r}, m) = (fdisp(f, υ0(m), υ1(m), υ2(m)) + ε(f) −FieldDesc[f].{bc, red}.base

Note that we use size[3] for υd(m) above.


void
set_field_descriptors(int (*ft)[OH_FTYPE_N], int sd[OH_DIMENSION][2], int ps) {

int nf=nOfFields;
struct S_flddesc *fd = FieldDesc;
int size[3] = {0,0,0};
int d, f;

for (d=0; d<OH_DIMENSION; d++)  size[d] = sd[d][OH_UPPER] - sd[d][OH_LOWER];
for (f=0; f<nf; f++) {
int bu = ft[f][OH_FTYPE_BU] - 1;
int ru = ft[f][OH_FTYPE_RU] - 1;
int es = ft[f][OH_FTYPE_ES];
fd[f].bc.size[ps] =


<!-- Page 302 -->

Field_Disp(f, size[OH_DIM_X]+bu, size[OH_DIM_Y]+bu, size[OH_DIM_Z]+bu) -
fd[f].bc.base + es;
fd[f].red.size[ps] =
Field_Disp(f, size[OH_DIM_X]+ru, size[OH_DIM_Y]+ru, size[OH_DIM_Z]+ru) -
fd[f].red.base + es;
}
}


#### 4.7.11 set_border_exchange()

set_border_exchange()  The function set_border_exchange(), called from init_fields() and oh3_exchange_
borders(),  fill the elements in BorderExc[c][p][2], where c and p are given through
its arguments e and ps, for a communication type c ∈[0, C) of the field-array f =
BoundaryCommFields[c] of the primary (p = 0) or secondary subdomain m of the local
node, i.e., m = RegionId[p]. The MPI data types to be recorded in BorderExc[c][p][][] is
given by type argument or that derived from it, which is usually MPI_DOUBLE but can be
MPI_LONG_LONG_INT for per-grid histogram used in level-4p library.


static void
set_border_exchange(int e, int ps, MPI_Datatype type) {
struct S_borderexc (*bx)[2] = BorderExc[e][ps];
int f = BoundaryCommFields[e];
int nb = nOfBoundaries;
int (*bt)[2][OH_CTYPE_N] = &BoundaryCommTypes[e*nb];
int (*bd)[2] = Boundaries[RegionId[ps]];
int (*sd)[2] = SubDomains[RegionId[ps]];
struct S_flddesc *fd = &FieldDesc[f];
int esize = fd->esize;
int fext = fd->ext[OH_UPPER] - fd->ext[OH_LOWER];
int xyz[3] = {
sd[OH_DIM_X][OH_UPPER]-sd[OH_DIM_X][OH_LOWER],
OH_DIMENSION>OH_DIM_Y ? sd[OH_DIM_Y][OH_UPPER]-sd[OH_DIM_Y][OH_LOWER] : 0,
OH_DIMENSION>OH_DIM_Z ? sd[OH_DIM_Z][OH_UPPER]-sd[OH_DIM_Z][OH_LOWER] : 0
};
int *wdh = fd->size;
int exti[OH_DIMENSION][2], exto[OH_DIMENSION][2];
int soff[OH_DIMENSION][2], roff[OH_DIMENSION][2];
int ssize[OH_DIMENSION][2], rsize[OH_DIMENSION][2];
int d, lu;


First, it fills the following argument arrays for set_border_comm(), for all d ∈[0, D),
β ∈{0, 1} and w ∈{0, 1}.

- exti[d][β] is the d-th dimensional inner extension of the field-array f being the bottom
(β = 0) or top-plus-one (β = 1) coordinate of the sending planes relative to the bottom
or top boundary plane.

- exto[d][β] is the d-th dimensional outer extension of the field-array f being the bottom
(β = 0) or top-plus-one (β = 1) coordinate of the receiving planes relative to the
bottom or top boundary plane.


<!-- Page 303 -->

- soff[d][w] and roff[d][w] are the bottoms of the d-th dimensional sending/receiving
planes of the field-array f to be sent/received in downward (w = 0) or upward (w = 1)
communication.

- ssize[d][w] and rsize[d][w] are the number of the d-th dimensional sending/receiving
planes of the field-array f to be sent/received in downward (w = 0) or upward (w = 1)
communication.

Therefore, they are defined as follows, where {ef, et, s}(bd, β) = BoundaryCommTypes[c][bd]
[w][0:2] and bd ∈{bld, bud} = Boundaries[m][d][0:1].

exti[d][0] = ef(bld, 0)        exti[d][1] = ef(bud, 1) + s(bud, 1)
exto[d][0] = et(bld, 1)        exto[d][1] = et(bud, 0) + s(bud, 0)
soff[d][w] = ef(bwd , w)      roff[d][w] = et(b1−wd     , w)
ssize[d][w] = s(bwd , w)      rsize[d][w] = s(b1−wd     , w)

Note that a downward (w = 0) or upward (w = 1) communication is a sending one through
lower (β = 0) or upper (β = 1) boundary plane respectively (i.e., β = w), while it is a
receiving one through upper (β = 1) or (β = 0) lower boundary plane respectively (i.e.,
β = 1 −w).

for (d=0; d<OH_DIMENSION; d++) {
int blo=bd[d][OH_LOWER], bup=bd[d][OH_UPPER];
exti[d][OH_LOWER] = bt[blo][OH_LOWER][OH_CTYPE_FROM];
exti[d][OH_UPPER] =
bt[bup][OH_UPPER][OH_CTYPE_FROM] + bt[bup][OH_UPPER][OH_CTYPE_SIZE];
exto[d][OH_LOWER] = bt[blo][OH_UPPER][OH_CTYPE_TO];
exto[d][OH_UPPER] =
bt[bup][OH_LOWER][OH_CTYPE_TO] + bt[bup][OH_LOWER][OH_CTYPE_SIZE];
for (lu=OH_LOWER; lu<=OH_UPPER; lu++) {
int sb=bd[d][lu], rb=bd[d][1-lu];
soff[d][lu] = bt[sb][lu][OH_CTYPE_FROM];
roff[d][lu] = bt[rb][lu][OH_CTYPE_TO];
ssize[d][lu] = bt[sb][lu][OH_CTYPE_SIZE];
rsize[d][lu] = bt[rb][lu][OH_CTYPE_SIZE];
}
}

Then, we call set_border_comm() four times for all combination of its argument lu =
{0, 1} for downward/upward communication and sr = {0, 1} for sending/receiving, with
the following other arguments.

- esize = FieldDesc[f].esize = ε(f) and f = f.

- xyz[0:D−1] = {SubDomains[m][d][1] −SubDomains[m][d][0]} = {δud(m) −δld(m)} fol-
lowed by 0’s.

- wdh[D] = FieldDesc[f].size[D] = {Φd(f)}

- exti[D][2] and exto[D][2].

- off[D][2] ∈{soff[D][2], roff[D][2]} and size[D][2] ∈{ssize[D][2], rsize[D][2]}.

- type is the argument of this function itself and is MPI_DOUBLE usually but can be
MPI_LONG_LONG_INT.


<!-- Page 304 -->

- bx[D][2] = BorderExc[c][p][D][2].


for (lu=OH_LOWER; lu<=OH_UPPER; lu++) {
set_border_comm(esize, f, xyz, wdh, exti, exto, soff, ssize, lu, 0, type,
bx);
set_border_comm(esize, f, xyz, wdh, exti, exto, roff, rsize, lu, 1, type,
bx);
}
}


#### 4.7.12 set_border_comm()

set_border_comm()  The function set_border_comm(), called solely from set_border_exchange(), fills ele-
ments buf, count, deriv and type of BorderExc[c][p][d][w]{send, recv} for the downward
(w = 0) or upward (w = 1) boundary communication of type c through d-dimensional
boundary plane of a field-array f  of primary (p = 0) or secondary (p = 1) subdo-
main m of the local node for  all d ∈[0, D), where BorderExc[c][p]  is given through
its argument bx, w is through the argument lu, and the argument sr determines send
(sr = 0) or recv (sr = 1).  The other arguments, esize = ε(f), f = f, xyz[D] =
{δud(m) −δld(m)}, wdh[D] = Φ[0,D)(f), exti[D][2] =  e{l,u}i    ([0, D)) =  e{l,u}i    ({x, y, z}),
exto[D][2] =  e{l,u}o    ([0, D)) =  e{l,u}o   ({x, y, z}),  off[D][2],  size[D][2] =  σ{0,1}[0,D) and
basetype ∈{MPI DOUBLE, MPI LONG LONG INT} have been discussed in §4.7.11.
Since;

- the boundary communications of the field-array f take place through d-th dimensional
boundary plane in the ascending order of d;

- the MPI data-type for each communication depends on D; and

- we optimize the MPI data-type if the boundaries of sending/receiving planes are also
those of field-array;

the code has many if-then-else’s.


static void
set_border_comm(int esize, int f, int *xyz, int *wdh,
int exti[OH_DIMENSION][2], int exto[OH_DIMENSION][2],
int off[OH_DIMENSION][2], int size[OH_DIMENSION][2],
int lu, int sr, MPI_Datatype basetype,
struct S_borderexc bx[OH_DIMENSION][2]) {
int bl[2]={1,1};
MPI_Datatype tmptype[2]={MPI_DATATYPE_NULL,MPI_UB};
int w=wdh[OH_DIM_X], wd=w*esize;
int dp=OH_DIMENSION==1 ? 1 : wdh[OH_DIM_Y];
MPI_Aint dispz[2]={0, wd*dp*sizeof(double)};
struct S_bcomm *bcx, *bcy, *bcz;
int xexto, yexti, yexto, zexti;
int s;
int lower = sr ? lu==OH_UPPER : lu==OH_LOWER;


<!-- Page 305 -->

First, we set BorderExc[c][p][d][w].t.deriv = 0, where t is send (sr = 0) or recv
(sr = 1), as their default values indicating type has a basic MPI data-type, for all d ∈[0, D).
Then we calculate following inner and outer extents of the field-array f as;

χid = (δud + eui (d)) −(δld + eli(d))
χod = (δud + euo(d)) −(δld + euo(d))

to have χo0×· · · χod−1×χid+1×· · · χiD−1 as the shape of a d-th dimensional sending/receiving
plane53. By this shape definition and the ascending order of communications through
each of d-th dimensional boundary planes from d = 0 to d = D−1, the d-th dimensional
sending plane should have a part of (d−1)-th and lower dimensional receiving planes to
relay boundary data of a subdomain to its neighbor contacted with a edge (if D = 3) or
vertex (if D ≥2).
The base (lowest) local coordinate of the d-th dimensional sending/receiving plane
(λ0, . . . , λD−1) is defined as follows.
{
off[d][w]                          t = w
bd =
(δud(m) −δld(m)) + off[d][w]   t ̸= w

 elo(k)  k < d
λk =   bd     k = d

eli(k)  k > d

Note that t = w above means the downward sending or upward receiving planes and thus
lower planes, while t ̸= w means the upward sending or downward receiving ones being
upper planes.

bcx = (sr==0) ? &bx[OH_DIM_X][lu].send : &bx[OH_DIM_X][lu].recv;
bcx->deriv = 0;
xexto = xyz[OH_DIM_X] + exto[OH_DIM_X][OH_UPPER] - exto[OH_DIM_X][OH_LOWER];
if (OH_DIMENSION>OH_DIM_Y) {
bcy = (sr==0) ? &bx[OH_DIM_Y][lu].send : &bx[OH_DIM_Y][lu].recv;
bcy->deriv = 0;
yexti = xyz[OH_DIM_Y] +
exti[OH_DIM_Y][OH_UPPER] - exti[OH_DIM_Y][OH_LOWER];
yexto = xyz[OH_DIM_Y] +
exto[OH_DIM_Y][OH_UPPER] - exto[OH_DIM_Y][OH_LOWER];
}
if (OH_DIMENSION>OH_DIM_Z) {
bcz = (sr==0) ? &bx[OH_DIM_Z][lu].send : &bx[OH_DIM_Z][lu].recv;
bcz->deriv = 0;
zexti = xyz[OH_DIM_Z] +
exti[OH_DIM_Z][OH_UPPER] - exti[OH_DIM_Z][OH_LOWER];
}

Next, we fill BorderExc[c][p][d][w].t.{buf, count, deriv, type} according to D and the
shape of sending/receiving planes.  In general, we let buf = count = 0 and type =
MPI_DATATYPE_NULL and keep deriv = 0 for d if σwd = 0 to mean no downward (w = 0) or
upward (w = 1) communications take place through d-th dimensional boundary plane.

53Since 1 ≤D ≤3, at most we have two χ{i,o}d    in the shape definition but it is conceptually as shown.


<!-- Page 306 -->

If D = 1 and σwx > 0, we simply fill the elements only for d = 0 to transfer contiguous
size = σwx ·ε(f) elements of basetype from buf = fdisp(f, bx, 0, 0) given by Field_Disp()
for the element [bx][0] of the field-array f.

if (OH_DIMENSION==1) {
if ((s=size[OH_DIM_X][lu])==0) {
bcx->buf = bcx->count = 0;  bcx->type = MPI_DATATYPE_NULL;
} else {
bcx->type = basetype;
bcx->count = s * esize;
bcx->buf =
Field_Disp(f,
lower ? off[OH_DIM_X][lu] : xyz[OH_DIM_X]+off[OH_DIM_X][lu],
0, 0);
}

If D = 2, the elements for d = 0 are to transfer a set of σwx  line segments of χiy grid
points perpendicular to x-axis unless σwx = 0. Therefore, we create a MPI data-type by
MPI_Type_vector() with count = χiy being the inner extent along y-axis, blocklength =
σwx  · ε(f) and stride = Φx(f) · ε(f) to store it in type and then commit it by MPI_
Type_commit().  The element count = 1 because we transfer this single stride-vector
and deriv = 1 because the data-type is derivative. The base of the vector buf is the
one-dimensional index fdisp(f, bx, eli(y), 0) of the element [eli(y)][bx][0] of the field-array f,
which is given by Field_Disp().
The elements for d = 1 are to transfer a set of σwy  line segments of χox grid points
perpendicular to y-axis unless σwy = 0. If χox being the outer extent along x-axis is equal to
Φx(f) being the x-dimensional size of the field-array f, transferred data set has contiguous
count = σwy ·χox·ε(f) elements of the data type given by basetype argument. Otherwise, we
have to create a MPI data-type by MPI_Type_vector() with count = σwy , blocklength =
χox · ε(f) and stride = Φx(f) · ε(f) to store it in type and then commit it by MPI_Type_
commit().  In this case, the element count = 1 because we transfer this stride-vector
and deriv = 1 because the data-type is derivative.  In both cases, the base buf is the
one-dimensional index fdisp(f, elo(x), by, 0) of the element [by][elo(x)][0] of the field-array f,
which is given by Field_Disp().

} else if (OH_DIMENSION==2) {
if ((s=size[OH_DIM_X][lu])==0) {
bcx->buf = bcx->count = 0;  bcx->type = MPI_DATATYPE_NULL;
} else {
MPI_Type_vector(yexti, s*esize, wd, basetype, &(bcx->type));
MPI_Type_commit(&(bcx->type));  bcx->deriv = 1;
bcx->count = 1;
bcx->buf =
Field_Disp(f,
lower ? off[OH_DIM_X][lu] : xyz[OH_DIM_X]+off[OH_DIM_X][lu],
exti[OH_DIM_Y][OH_LOWER], 0);
}
if ((s=size[OH_DIM_Y][lu])==0) {
bcy->buf = bcy->count = 0;  bcy->type = MPI_DATATYPE_NULL;
} else {
if (xexto==w) {
bcy->type = basetype;
bcy->count = s * wd;


<!-- Page 307 -->

} else {
MPI_Type_vector(s, xexto*esize, wd, basetype, &(bcy->type));
MPI_Type_commit(&(bcy->type));  bcy->deriv = 1;
bcy->count = 1;
}
bcy->buf =
Field_Disp(f, exto[OH_DIM_X][OH_LOWER],
lower ? off[OH_DIM_Y][lu] : xyz[OH_DIM_Y]+off[OH_DIM_Y][lu],
0);
}

Finally if D = 3, the elements for d = 0 are to transfer a set of σwx yz-subplanes of
χiz × χiy grid points unless σwx = 0. Therefore, we at first create a stride-vector for χiy × σwx
strip by MPI_Type_vector() as done for D = 2 and d = 0, then create a structured type
by MPI_Type_struct() to stack the strips so that first elements of adjacent vectors are
sxy = Φy(f) · Φx(f) · ε(f) · sizeof(double) bytes54apart from each other, and finally
commit it by MPI_Type_commit(). The element count = χiz being the z-dimensional inner
extent and deriv = 1 because the data-type is derivative. The base of the planes buf is
the one-dimensional index fdisp(f, bx, eli(y), eli(z)) of the element [eli(z)][eli(y)][bx][0] of the
field-array f, which is given by Field_Disp().
The elements for d = 1 are to transfer a set of σwy xz-subplanes of χiz × χox grid points
unless σwy = 0.  If χox = Φx(f), the set can be represented by a set of χiz xy-strips of
σwy  · χox · ε(f) contiguous elements of basetype stacked along z-axis with a stride of Φy(f) ·
Φx(f)·ε(f), and thus by one stride-vector created and set into type element by MPI_Type_
vector(). Otherwise, the strip is a stride-vector created by MPI_Type_vector() with σwy
line segments of χox · ε(f) elements with a stride of Φx(f) · ε(f). Then χiz strips are stacked
so that first elements of adjacent strips are sxy-byte apart from each other by MPI_Type_
struct() to set the strip type into type element and by setting count element to χiz. In
both cases, the MPI data-type in type element is commited by MPI_Type_commit(), deriv
is set to 1 because of derivative, and the base of the planes buf is the one-dimensional index
fdisp(f, elo(x), by, eli(z)) of the element [eli(z)][by][elo(x)][0] of the field-array f, which is given
by Field_Disp().
The elements for d = 2 are to transfer a set of σwz xy-subplanes of χoy × χox grid points
unless σwz = 0. The type and count elements are dependent on if χox = Φx(f) and/or
χoy = Φy(f) and thus we have the following four cases.

- If χox = Φx(f) and χoy = Φy(f), the set has contiguous σwz  · χoy · χox · ε(f) elements of
basetype

- If χox = Φx(f) but χoy ̸= Φy(f), a xy-subplane has contiguous χoy · χox · ε(f) elements
of basetype and then σwz subplanes are stacked with a stride of Φy(f) · Φx(f) · ε(f).
Therefore, the type element is created by MPI_Type_vector() and count = 1 to
transfer one single stride-vector.

- If χox ̸= Φx(f) but χoy = Φy(f), the set of xy-subplanes are considered as the set of
σwz  · χoy line segments of χox · ε(f) basetype elements with a stride of Φx(f) · ε(f).
Therefore, the type element is created by MPI_Type_vector() and count = 1 to
transfer one single stride-vector.

54If basetype  is MPI LONG LONG INT, the gap should be  calculated with sizeof(dint) but using
sizeof(double) is safe because they are equivalent.


<!-- Page 308 -->

- If χox ̸= Φx(f) and χoy  ̸= Φy(f), a xy-subplane is the set of χoy line segments of
χox · ε(f) basetype elements with a stride of Φx(f) · ε(f), which is created by MPI_
Type_vector(). Then σwz  subplanes are stacked so that first elements of adjacent
subplanes are sxy-byte apart from each other by MPI_Type_struct() to set the strip
type into type element and by setting count element to σwz .

For the last three cases, we commit the MPI data-type in type element by MPI_Type_
commit() and set deriv = 1. Then for all of four cases, the base of xy-subplanes buf is
the one-dimensional index fdisp(f, elo(x), elo(z), bz) of the element [bz][elo(y)][elo(x)][0] of the
field-array f, which is given by Field_Disp().

} else {
if ((s=size[OH_DIM_X][lu])==0) {
bcx->buf = bcx->count = 0;  bcx->type = MPI_DATATYPE_NULL;
} else {
MPI_Type_vector(yexti, s*esize, wd, basetype, tmptype);
MPI_Type_struct(2, bl, dispz, tmptype, &(bcx->type));
MPI_Type_commit(&(bcx->type));  bcx->deriv = 1;
bcx->count = zexti;
bcx->buf =
Field_Disp(f,
lower ? off[OH_DIM_X][lu] : xyz[OH_DIM_X]+off[OH_DIM_X][lu],
exti[OH_DIM_Y][OH_LOWER], exti[OH_DIM_Z][OH_LOWER]);
}
if ((s=size[OH_DIM_Y][lu])==0) {
bcy->buf = bcy->count = 0;  bcy->type = MPI_DATATYPE_NULL;
} else {
if (xexto==w) {
MPI_Type_vector(zexti, s*wd, wd*dp, basetype, &(bcy->type));
bcy->count = 1;
} else {
MPI_Type_vector(s, xexto*esize, wd, basetype, tmptype);
MPI_Type_struct(2, bl, dispz, tmptype, &(bcy->type));
bcy->count = zexti;
}
MPI_Type_commit(&(bcy->type));  bcy->deriv = 1;
bcy->buf =
Field_Disp(f, exto[OH_DIM_X][OH_LOWER],
lower ? off[OH_DIM_Y][lu] : xyz[OH_DIM_Y]+off[OH_DIM_Y][lu],
exti[OH_DIM_Z][OH_LOWER]);
}
if ((s=size[OH_DIM_Z][lu])==0) {
bcz->buf = bcz->count = 0;  bcz->type = MPI_DATATYPE_NULL;
} else {
if (xexto==w && yexto==dp) {
bcz->type = basetype;
bcz->count = s * wd * dp;
} else {
if (xexto==w) {
MPI_Type_vector(s, wd*yexto, wd*dp, basetype, &(bcz->type));
bcz->count = 1;
} else if (yexto==dp) {
MPI_Type_vector(s*yexto, xexto*esize, wd, basetype, &(bcz->type));
bcz->count = 1;


<!-- Page 309 -->

} else {
MPI_Type_vector(yexto, xexto*esize, wd, basetype, tmptype);
MPI_Type_struct(2, bl, dispz, tmptype, &(bcz->type));
bcz->count = s;
}
MPI_Type_commit(&(bcz->type));  bcz->deriv = 1;
}
bcz->buf =
Field_Disp(f, exto[OH_DIM_X][OH_LOWER], exto[OH_DIM_Y][OH_LOWER],
lower ? off[OH_DIM_Z][lu] :
xyz[OH_DIM_Z]+off[OH_DIM_Z][lu]);
}
}
}

Note that the data types for each D can be created by MPI_Type_create_subarray()
giving it the following arguments, for example, if D = 3 and d = 2.

ndims = 4
array of sizes = {Φz(f), Φy(f), Φx(f), ε(f)}
array of subsizes = {σwz , χoy, χox, ε(f)}
array of starts = {bz, elo(y), elo(x), 0}
order = MPI_ORDER_C
oldtype = basetype

However, it is not sure that any MPI implementations take care of special cases such as
χox = Φx(f) and χoy = Φy(f) to create σz · χoy · χox · ε(f) contiguos basetype elements as
MPI_Type_contiguous() does.  Therefore, we check such condisions and use MPI_Type_
vector() and MPI_Type_struct() only if necessary.

#### 4.7.13 clear_border_exchange()

clear_border_exchange()  The function clear_border_exchange(), called from init_fields() and transbound3(),
reinitalizes elements of BorderExc[c][1][d][w].{send, recv} for the boundary communication
of the secondary subdomain of the local node for all c ∈[0, C), d ∈[0, D) and w ∈{0, 1}.
The essential part of the reinitialization is to free derivative data types in type elements
by MPI_Type_free()  if deriv = 1.  The other essentially required operation is to let
count element be −1 to indicate that the communication parameters for the secondary
subdomain are not set and thus we need to set them by set_border_exchange() when the
result of type c communication is to broadcasted by oh3_exchange_borders(). The other
operations to let buf and deriv be 0 and to let type be MPI_DATATYPE_NULL are reasonable
but not necessary.


void
clear_border_exchange() {
int ne=nOfExc, e, d, lu;
struct S_borderexc (*bx)[2][OH_DIMENSION][2] = BorderExc;

for (e=0; e<ne; e++) {
for (d=0; d<OH_DIMENSION; d++) {
for (lu=OH_LOWER; lu<=OH_UPPER; lu++) {


<!-- Page 310 -->

if (bx[e][1][d][lu].send.deriv)
MPI_Type_free(&bx[e][1][d][lu].send.type);
if (bx[e][1][d][lu].recv.deriv)
MPI_Type_free(&bx[e][1][d][lu].recv.type);
bx[e][1][d][lu].send.buf =   bx[e][1][d][lu].recv.buf = 0;
bx[e][1][d][lu].send.count = bx[e][1][d][lu].recv.count = -1;
bx[e][1][d][lu].send.deriv = bx[e][1][d][lu].recv.deriv = 0;
bx[e][1][d][lu].send.type =  bx[e][1][d][lu].recv.type =
MPI_DATATYPE_NULL;
}
}
}
}


#### 4.7.14 oh3_grid_size()

oh3_grid_size_()  The API function oh3_grid_size_() for Fortran and oh3_grid_size() for C provide a
oh3_grid_size()  simulator body calling them with the means to specify the grid size of each dimension if the
real coordinate for particle is different from integer coordinate for subdomains. Its size[D]
argument array has the scale factor γd in its element [d].
For each d ∈[0, D), the function lets Grid[d].gsize = γd and Grid[d].rgsize = 1/γd,
and updates SubDomainsFloat[m][d][β] = δβd (m) for all m ∈[0, N) and Grid[d]’s ele-
ments, namely fcoord[β] = ∆βd, fsize = δmaxd   and light.fthresh = ∆−d , by multi-
plying them by γd.  It also updates and Grid[d]’s elements light.rfsize = 1/δmind   and
light.rfsizeplus = 1/(δmind  +1) by dividing them by γd. If irregular process coordinate,
in addition, SubDomainDesc[m].coord[d].fc[β] = δβd (m) for all m ∈[0, N) are also updated
by multiplying them by γd.


void
oh3_grid_size_(double size[OH_DIMENSION]) {
oh3_grid_size(size);
}
void
oh3_grid_size(double size[OH_DIMENSION]) {
int d, n, nn=nOfNodes;
for (d=0; d<OH_DIMENSION; d++) {
double s = (Grid[d].gsize = size[d]);
Grid[d].rgsize = 1 / s;
for (n=0; n<nn; n++) {
SubDomainsFloat[n][d][OH_LOWER] *= s;
SubDomainsFloat[n][d][OH_UPPER] *= s;
}
Grid[d].fcoord[OH_LOWER] *= s;
Grid[d].fcoord[OH_UPPER] *= s;
Grid[d].fsize *= s;
Grid[d].light.rfsize /= s;
Grid[d].light.rfsizeplus /= s;
Grid[d].light.fthresh *= s;
if (SubDomainDesc) {
for (n=0; n<nn; n++) {
SubDomainDesc[n].coord[d].fc[OH_LOWER] *= s;
SubDomainDesc[n].coord[d].fc[OH_UPPER] *= s;


<!-- Page 311 -->

}
}
}
}


#### 4.7.15 oh3_transbound() and transbound3()

oh3_transbound_()  The API function oh3_transbound_() for Fortran and oh3_transbound() for C provide a
oh3_transbound()  simulator body calling them with the load-balanced particle transfer mechanism of level-2
or level-1 library together with subdomain-related functions of level-3’s own. The meanings
of their two arguments, currmode and stats, and return value in {−1, 0, 1} are perfectly
equivalent to those of the level-1 and level-2 counterparts oh1_transbound[_]() and oh2_
transbound[_](). Also similarly to the counterparts, their bodies only have a simple call
of transbound3() but the third argument level is 3 to indicate the function is called from
level-3 API functions.


int
oh3_transbound_(int *currmode, int *stats) {
return(transbound3(*currmode, *stats, 3));
}
int
oh3_transbound(int currmode, int stats) {
return(transbound3(currmode, stats, 3));
}


transbound3()  The function transbound3(), called from oh3_transbound_() or oh3_transbound(), at
first calls its level-2 counterpart transbound2() usually, i.e., excludeLevel2 is false, or the
level-1 counterpart transbound1() if the level-3 library was initialized by oh13_init() and
thus excludeLevel2 is true.
Then  if the transbound1() or transbound2() assigned a parent to the local node
different from before the call, i.e., RegionId[1] before and after the call are different, we call
clear_border_exchange() to reinitialize BorderExc[c][1][d][w] for all c ∈[0, C), d ∈[0, D)
and w ∈{0, 1} for the old secondary subdomain, and then call set_field_descriptors()
to set FieldDesc[f].{bc,red}.size[1] for all f ∈[0, F) for the new secondary subdomain
giving FieldTypes[F][7] and SubDomains[m][D]][2] to the function where m is the new
secondary subdomain identifier. Note that these function are not called if old/new value
of RegionId[1] are negative meaning that we were or will be in primary mode or the local
node was or will be the root of the family tree.
Finally the function returns the value from transbound1() or transbound2() to the
caller.


static int
transbound3(int currmode, int stats, int level) {
int oldp=RegionId[1], newp;

currmode = excludeLevel2 ? transbound1(currmode, stats, 1) :
transbound2(currmode, stats, level);
newp = RegionId[1];
if (oldp!=newp) {
if (oldp>=0)  clear_border_exchange();


<!-- Page 312 -->

if (newp>=0)  set_field_descriptors(FieldTypes, SubDomains[newp], 1);
}
return(currmode);
}


#### 4.7.16 Macro Map_Particle_To_Neighbor()

Map_Particle_To_Neighbor()  The macro Map Particle To Neighbor(&x, m, d, k, 3d), used in three versions of oh3_map_
particle_to_neighbor() does;

 k −3d  x < δld(m) · γd
k ←   k         δld(m) · γd ≤x < δud(m) · γd                             
k + 3d  δud(m) · γd < x
referring to SubDomainsFloat[m][d][β] = δβd (m) · γd, so that k finally has the index of
Neighbors[p][3D] for the subdomain which the particle whose d-th dimensional integer
coordinate is xI. Thus the subdomain is expected to be neighboring to the subdomain m
being the primary subdomain n (p = 0) or the secondary subdomain parent(n) (p = 1) of
the local node n.
It also checks if x < ∆ld · γd = Grid[d].fcoord[0] or x ≥∆ud · γd = Grid[d].fcoord[1].
If so and the lower/upper boundary condition of the system domain is not periodic, i.e.,
Boundaries[m][d][{0, 1}] ̸= 0, the macro makes its user function return to its caller with
−1 to indicate particle is out-of-bound.  If periodic, on the other hand, the floating point
coordinate x of the particle is incremented/decremented by (∆ud −∆ld) · γd so that the
particle moves to the opposite end along the d-th dimensional axis of the system domain.


#define Map_Particle_To_Neighbor(XYZ,RID,DIM,N,INC) {\
double xyz=*XYZ;\
if (xyz<SubDomainsFloat[RID][DIM][OH_LOWER]) {\
N -= INC;\
if (xyz<Grid[DIM].fcoord[OH_LOWER]) {\
if (Boundaries[RID][DIM][OH_LOWER])  return(-1);\
*XYZ += Grid[DIM].fcoord[OH_UPPER] - Grid[DIM].fcoord[OH_LOWER];\
}\
} else if (xyz>=SubDomainsFloat[RID][DIM][OH_UPPER]) {\
N += INC;\
if (xyz>=Grid[DIM].fcoord[OH_UPPER]) {\
if (Boundaries[RID][DIM][OH_UPPER])  return(-1);\
*XYZ -= Grid[DIM].fcoord[OH_UPPER] - Grid[DIM].fcoord[OH_LOWER];\
}\
}\
}


#### 4.7.17 Macro Neighbor_Id()

Neighbor_Id()  The macro Neighbor_Id(), used in three versions of oh3_map_particle_to_neighbor(),
translates its argument m′ in Neighbors[][] into m = m′  if m′ ≥0, m = −(m′ + 1)  if
−N ≤m′ < 0, or m = −1 if m′ < −N, to have the real neighboring subdomain identifier
m, or −1 to indicate out-of-bounds.


#define Neighbor_Id(N) ((n=(N))<0 ? ((n=-n-1)<nOfNodes ? n : -1) : n)


<!-- Page 313 -->

#### 4.7.18 oh3_map_particle_to_neighbor()

oh3_map_particle_to_neighbor_()  The API functions oh3_map_particle_to_neighbor_()55  for Fortran and oh3_map_
oh3_map_particle_to_neighbor()  particle_to_neighbor() for C find the subdomain m, which is neighboring to the pri-
mary (ps = p = 0) or secondary (ps = p = 1) subdomain, should accommodate the particle
whose coordinate values are pointed by the arguments x, y (if D ≥2) and z (if D = 3),
and is returned to the caller.  If such a subdomain is not found due to that the particle is
moving out-of-bounds, the function returns −1 instead of the subdomain identifier. The
function also modifies the coordinate pointed by the arguments if the particle has moved
crossing periodic boundary planes of the system domain, so that it jumps to the coordinate
point corresponding to the opposite boundary planes.
Since the function takes D arguments for the particle coordinate, we have three versions
of each function. In all versions, the Fortran API oh3_map_particle_to_neighbor_()56
simply calls C counterpart oh3_map_particle_to_neighbor() and returns what the C
counterpart returns. Also in all versions, oh3_map_particle_to_neighbor() for the local
node n invokes the macro Map_Particle_To_Neighbor() D times giving arguments &x =
{x, y, z}[d], m = RegionId[p] ∈{n, parent(n)}, d, k and 3d to its (d+1)-th invocation where
∑D−1
k = (3D −1)/2 =   d=0 3d at initial. Since the macro modifies k as;

 k −3d  x < δld(m) · γd
k ←   k         δld(m) · γd ≤x < δud(m) · γd                              
k + 3d  δud(m) · γd < x
∑D−1
we have k =   d=0 νd3d (νd ∈[0, 2]) corresponding to the process coordinate (π0+ν0−1,
. . . , πD−1 + νD−1 −1) where (π0, . . . , πD−1) is for that of the subdomain m. Therefore, the
target subdomain l or the out-of-bound indicator −1 is obtained from l′ = Neighbors[p][k]
by the following calculation with macro Neighbor Id(k).
{
−1             l′ < −N
l =   −(l′ + 1) −N ≤l′ < 0
l′         0 ≤l′



#if OH_DIMENSION==1
int
oh3_map_region_to_adjacent_node_(double *x, int *ps) {
return(oh3_map_particle_to_neighbor(x, *ps));
}
int
oh3_map_particle_to_neighbor_(double *x, int *ps) {
return(oh3_map_particle_to_neighbor(x, *ps));
}
int
oh3_map_particle_to_neighbor(double *x, int ps) {
int rid=RegionId[ps], n=OH_NEIGHBORS>>1;

Map_Particle_To_Neighbor(x, rid, OH_DIM_X, n, 1);
return(Neighbor_Id(Neighbors[ps][n]));
}
#elif OH_DIMENSION==2

55And its aliase oh3 map region to adjacent node () for backward compatiblity.
56And oh3 map region to adjacent node () as well.
