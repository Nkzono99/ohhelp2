# 3.8 Level-4s Extension and Its Functions

Source: `doc/original/ohhelp.pdf`, pages 84-95.

<!-- Page 84 -->

Fortran Interface

integer function oh4p_remap_particle_to_subdomain(part, ps, s)
use oh_type
implicit none
type(oh_particle),intent(inout) :: part
integer,intent(in)   :: ps
integer,intent(in)   :: s
end function


C Interface

int  oh4p_remap_particle_to_subdomain(struct S_particle *part, const int ps,
const int s);


part (for Fortran)
*part (for C)
The argument part should be a oh particle structure in Fortran, or a pointer to
S particle structure in C, to be remapped.

ps should be 0 for a primary particle, or 1 for a secondary particle.

s should be the species identifier of the particle in [1, S] in Fortran while in [0, S) in C.

## 3.8 Level-4s Extension and Its Functions

#### 3.8.1 Position-Aware Particle Management in Level-4s

The level-4s extension is similar to the level-4p counterpart to provide you of position-aware
particle management, but the load balancing particle transfer mechanism given by oh4s_
transbound() has the following features different from the level-4p counterpart.

- A node n responsible of a subdomain np (p ∈{0, 1}) as its primary (n0 = n) or
secondary (n1 = parent(n)) subdomain accommodates all particles in the subcuboid;

[δlx(np), δux(np)) × [δly(np), δuy (np)) × [δlz(np)+ζlp(n), δlz(np)+ζup (n))

where 0 ≤ζlp(n) ≤ζup (n) ≤δuz (m) −δlz(m).  That  is, the subcuboid consists of
grid-voxels in the subdomain np whose local z-coordinates are in [ζlp(n), ζup (n)). The
function (subroutine) oh4s_transbound() determine ζβp (n) (β ∈{l, u}) and returns
them through oh4s_init()’s argument array zbound. Unlike the level-4p counter-
part, it is assured that all particles in a subcuboid is accommodated by a particular
node, but this requires that the particle population in a grid-voxel, or the density,
should have a certain upper bound. Therefore, you have to determine this maximum
density D and show it to the library through maxdensity argument of oh4s_init().

- In addition to the particles in the subdomain np’s subcuboid responsible of, the node
n above also accommodates halo particles residing in grid-voxels just outside the
surface of the subcuboid.  That  is, halo particles are those residing in the set of
grid-voxels whose coordinates local to the subdomain np are in the following where
δd(m) = δud(m) −δld(m).
[−1, δx(np)+1) × [−1, δy(np)+1) × [ζlp(n)−1, ζup (n)+1) −
[0, δx(np)) × [0, δy(np)) × [ζlp(n), ζup (n))


<!-- Page 85 -->

These halo particles assure that, for every particle residing at the position (x, y, z)
in the subcuboid of the node n, all particles in the sphere with center (x, y, z) and
radius min(γx, γy, γz) are accommodated by the node n.

- In addition to the per-grid histogram whose element pghgram(c, s, x, y, z) for Fortran
or pghgram[z][y][x][s][c] for C having the number of primary (c = 1 in Fortran while
c = 0 in C) or secondary (c = 2 in Fortran while c = 1 in C) particles of species s in
the grid-voxel (x, y, z), oh4s_transbound() gives you the index of the first particle
in it through the second argument per-grid index array, say pgindex(c, s, x, y, z) or
pgindex[z][y][x][s][c] of oh4s_per_grid_histogram(). With this index array, parti-
cles in all grid-voxels the local node is responsible of after (2t+k)-th (k ∈{0, 1}) can
be visited by the following Fortran code snip.

if (has_secondary_subdomain()) then;  cc=2;  else;  cc=1;  end if
do c=1, cc
do z=zbound(1,c), zbound(2,c)-1
do y=0, sdoms(2,2,sdid(c))-sdoms(1,2,sdid(c))-1
do x=0, sdoms(2,1,sdid(c))-sdoms(1,1,sdid(c))-1
do s=1, nspec
do i=0, pghgram(x,y,z,s,c)-1
call do_something(pbuf(pgindex(x,y,z,s,c)+i,k+1))
end do
end do;  end do;  end do;  end do;  end do;


In C, the code snip corresponding to above is as follows.

for (c=0; c<has_secondary_subodmain() ? 2 : 1; c++) {
for (z=zbound[c][0]; z<zbound[c][1]; z++) {
for (y=0; y<sdoms[sdid[c]][1][1]-sdoms[sdid[c]][1][0]; y++) {
for (x=0; x<sdoms[sdid[c]][0][1]-sdoms[sdid[c]][0][0]; x++) {
for (s=0; s<nspec; s++) {
for (i=0; i<pghgram[c][s][z][y][x]; i++)
do_something(pbuf[k][pgindex[c][s][z][y][x]+i]);
} } } } }


Moreover, for a particle p in the grid-voxel (x, y, z), all particles whose distance from
p can be less than min(γx, γy, γx) can be found by the following Fortran code snip.

do dz=-1,1;  do dy=-1,1;  do dx=-1,1
do i=0, pghgram(x+dx,y+dy,z+dz,s,c)-1
call do_something(pbuf(pgindex(x+dx,y+dy,z+dz,s,c)+i,k+1))
end do
end do;  end do;  end do;


The C version of the code above is as follows.

for (dz=-1;dz<2;dz++)  for (dy=-1;dy<2;dy++)  for (dx=-1;dx<2;dx++) {
for (i=0; i<pghgram[c][s][z+dz][y+dy][x+dx]; i++)
do_something(pbuf[k][pgindex[c][s][z+dz][y+dy][x+dx]+i]);
}


<!-- Page 86 -->

Note that pghgram and pgindex are meaningful for halo region with x = −1, x =
δx(np), etc, so that you may access halo particles in the code snip shown above.

- Besides the particle transfer mechanism provided by oh4s_transbound(), the level-
4s library provides you of inter-node transfer of the halo part of any one-dimensional
particle-associated array whose layout is similar to the particle buffer.  For exam-
ple, suppose your simulation code has a vector v in each node and its i-th element
corresponds to the i-th particle in the particle buffer of the node. The function (sub-
routine) oh4s_exchange_border_data() takes the vector v (and send/receive buffers
and data-type as discussed in §3.8.7) to send v’s elements in grid-voxels whose local
coordinate (xs, ys, zs) of local subdomain m satisfies;

xs = 0 ∨xs = δx(m) −1 ∨ys = 0 ∨ys = δy(m) −1 ∨zs = 0 ∨zs = δz(m) −1

to the nodes responsible of m’s neighbors, and to receives elements for m’s local
coordinate (xr, yr, zr) satisfying;

xs = −1 ∨xs = δx(m) ∨ys = −1 ∨ys = δy(m) ∨zs = −1 ∨zs = δz(m)

from the neighbor nodes.  This function will be convenient to implement, e.g., an
iterative linear solver of unknowns corresponding to particles.


#### 3.8.2 Level-4s Functions

Level-4s extension provides the following functions.

oh4s_init() performs initialization similar to what oh4p_init() does with a few modifi-
cations for level-4s’s own features.

oh4s_particle_buffer() tells other library functions where the particle buffer is located
in your code.

oh4s_per_grid_histogram() tells other library functions where the per-grid histogram
and index arrays are located in your code.

oh4s_transbound() performs position-aware load balancing and particle transfer.

oh4s_exchange_border_data() transfers one-dimensional array elements corresponding
to halo particles.

oh4s_map_particle_to_neighbor() finds the subdomain and grid-voxel which will be the
residence of a particle that stays in the original subdomain or travel to its neighbor.

oh4s_map_particle_to_subdomain() finds the subdomain and grid-voxel which will be
the residence of a particle that may go to any subdomains.

oh4s_inject_particle() injects a particle to the bottom of the particle buffer.

oh4s_remove_mapped_particle() removes a particle which you have mapped by oh4s_
map_particle_to_neighbor() or oh4s_map_particle_to_subdomain(), or injected
by oh4s_inject_particle() after the last call of oh4s_transbound().

oh4s_map_particle_to_neighbor() does what oh4s_remove_mapped_particle() and
oh4s_map_particle_to_neighbor() do.


<!-- Page 87 -->

oh4s_map_particle_to_subdomain() does what oh4s_remove_mapped_particle() and
oh4s_map_particle_to_subdomain() do.

The function API for Fortran programs is given by the module named ohhelp4s in the
file oh mod4s.F90, while API for C is embedded in ohhelp c.h.


#### 3.8.3 oh4s_init()

The function (subroutine) oh4s init() receives a number of fundamental parameters and
arrays through which oh4s_transbound() and other library functions interacts with your
simulator body. It also initializes internal data structures used in level-4s and lower level
libraries.  Though some of 26 arguments are modified by oh4s_transbound(),  it and
other library functions will not directly refer to any of them.  Therefore, after the call
of oh4s init(), modifying the bodies of arguments has no effect to library functions.


Fortran Interface

subroutine oh4s_init(sdid, nspec, maxfrac, npmax, minmargin, maxdensity, &
totalp, pbase, maxlocalp, cbufsize, mycomm, nbor, &
pcoord, sdoms, scoord, nbound, bcond, bounds, &
ftypes, cfields, ctypes, fsizes, zbound, &
stats, repiter, verbose)
use oh_type
implicit none
integer,intent(out)   :: sdid(2)
integer,intent(in)    :: nspec
integer,intent(in)    :: maxfrac
integer*8,intent(in)  :: npmax
integer,intent(in)    :: minmargin
integer,intent(in)    :: maxdensity
integer,intent(out)   :: totalp(:,:)
integer,intent(out)   :: pbase(3)
integer,intent(out)   :: maxlocalp
integer,intent(out)   :: cbfsize
type(oh_mycomm),intent(out) :: mycomm
integer,intent(inout) :: nbor(3,3,3)
integer,intent(in)    :: pcoord(OH_DIMENSION)
integer,intent(inout) :: sdoms(:,:,:)
integer,intent(in)    :: scoord(2,OH_DIMENSION)
integer,intent(in)    :: nbound
integer,intent(in)    :: bcond(2,OH_DIMENSION)
integer,intent(inout) :: bounds(:,:,:)
integer,intent(in)    :: ftypes(:,:)
integer,intent(in)    :: cfields(:)
integer,intent(in)    :: ctypes(:,:,:,:)
integer,intent(out)   :: fsizes(:,:,:)
integer,intent(out)   :: zbound(2,2)
integer,intent(in)    :: stats
integer,intent(in)    :: repiter
integer,intent(in)    :: verbose
end subroutine


C Interface


<!-- Page 88 -->

void oh4s_init(int **sdid, const int nspec, const int maxfrac,
const long long int npmax, const int minmargin,
const int maxdensity, int **totalp, int **pbase,
int *maxlocalp, int *cbufsize, void *mycomm, int **nbor,
int *pcoord, int **sdoms, int *scoord, const int nbound,
int *bcond, int **bounds, int *ftypes, int *cfields,
int *ctypes, int **fsizes, int **zbound,
const int stats, const int repiter, const int verbose);


sdid
nspec
See §3.4.1 because two arguments above are perfectly equivalent to those of oh1_
init().

maxfrac is perfectly equivalent to that of oh1_init() and thus should have the tolerance
factor percentage of load imbalance α greater than 0 and less than 100, as discussed
in §3.4.1. This argument is used to calculate the base value of the particle buffer size
P lim′ = maxlocalp.

npmax should be the absolute maximum number of particles which your simulator is capable
of as a whole. Unlike the level-2/3/4p libraries, this argument is given to oh4s_init()
for the calcuation of Plim′ = maxlocalp rather than to oh2_max_local_particles()
or oh4p_max_local_particles().

minmargin should be the minimum margin by which Plim′  = maxlocalp has to clear over
the per node average of npmax. Unlike the level-2/3/4p libraries, this argument is
given to oh4s_init() for the calcuation of Plim′  = maxlocalp rather than to oh2_
max_local_particles() or oh4p_max_local_particles().

maxdensity should be the maximum density D being the maximum particle population in
a grid-voxel to be used for the calculation of Plim′ = maxlocalp.

totalp is perfectly equivalent to that of oh1_init() shown in §3.4.1. Note that nphgram
which the level-1 to level-3 counterparts have is not a member of the arguments of
oh4s_init() because maintaining the per-subdomain histogram is perfectly up to
the level-4s library functions, as in level-4p.

pbase is perfectly equivalent to that of oh2_init() shown in §3.5.2.  Note that pbuf
which the level-2/3/4p counterparts have is not a member of the arguments of oh4s_
init() because the particle buffer should be allocated referring to P lim′ = maxlocalp
calculated by this function and then be given to the level-4s library through oh4s_
particle_buffer().

maxlocalp (for Fortran)
*maxlocalp (for C)
The (variable pointed by this) argument will have the base value of the absolute limit
of the particle buffer, P lim,′   given by the following.

P = ⌈npmax/N⌉
δmaxd  = max {δd(m)}
0≤m<N
Phalo = D((δmaxx  + 2)(δmaxy  + 2)(δmaxz  + 2) −δmaxx  δmaxy  δmaxz   )


<!-- Page 89 -->

Pmgn = Dδmaxx  δmaxy
⌈               ⌉
Plim′ = max( P(100 + α)/100  , P + minmargin) + 2(Phalo + Pmgn)

Note that Phalo represents the maximum number of halo particle in each of primary
and secondary subcuboids, while Pmgn means we have to allow the excess of this
amount from the particle population which OhHelp load balancer suggests for each
of primary and secondary because particles in a xy-plane of subdomain is the unit of
balancing. Also note that this argument maxlocalp is output one for oh4s_init()
rather than input in level-2/3/4p counterparts.

cbufsize (for Fortran)
*cbufsize (for C)
The (variable pointed by this) argument will have the size Pcomm of send and re-
ceive buffers required by the halo-part communication of a particle-associated one-
dimensional array by oh4s_exchange_border_data(), given by the following.

Pcomm = 2Dδmaxz   max(δmaxx  + 2, δmaxy   )

Note that Pcomm < 2Phalo because of the followings two reasons. First, elements in
the bottom/top surfaces grid-voxels of a subcuboid are directly sent from the particle-
associated array and those in just below/above the bottom/top surfaces are directly
received into the array, without buffering.  Second, the communication for vertical
surfaces takes place in two phases, at first yz-surfaces (or west/east ones) and then
xz-surfaces (or south/north ones) including their intersections of yz-surfaces, so that
the buffers are not necessary to keep elements of both at the same time but are
sufficient to accommodate larger one of them.

mycomm
nbor
pcoord
See §3.4.1 because the arguments above are perfectly equivalent to those of oh1_
init().

sdoms
scoord
nbound
bcond
bounds
ftypes
cfields
ctypes
See §3.6.1 because the arguments above are perfectly equivalent to those of oh3_
init().

fsizes is perfectly equivalent to that  of oh4p_init() shown in  §3.7.3.   Therefore,
fsizes(β, d, F+1) or fsizes[F][d][β] is for the per-grid histogram (and per-grid in-
dex) you must (or may) allocate.

zbound(2,2) (for Fortran)
**zbound (for C)
The argument zbound should be an two-dimensional integer array of (2, 2) in For-
tran, while in C it should be a double pointer to an integer array of [2 × 2] or a


<!-- Page 90 -->

pointer to NULL (not NULL itself) to make oh4s init() allocate the array for you and
return the pointer to it through the argument. After a call of oh4s_transbound(),
zbound(β+1, p+1) or zbound[p][b] (p, β ∈{0, 1}) will have the local z-coordinate of
the lower (β = 0) or upper (β = 1) surface of the primary (p = 0) or secondary
(p = 1) subcuboid of the local node n, i.e., ζβp (n).

stats
repiter
verbose
See §3.4.1 because the arguments above are perfectly equivalent to those of oh1_
init().

#### 3.8.4 oh4s_particle_buffer()

The function (subroutine) oh4s particle buffer() is to let level-4s library functions know
where the particle buffer is located in your simulator body, or to allocate the buffer for you.
Unlike level-2/3/4p libraries, the particle buffer is not given to (or by) oh4s_init() because
its mininum size P lim′    is calculated by oh4s_init() and is reported through its argument
maxlocalp. Therefore, if your simulator is coded in Fortran, you must allocate the buffer
for 2Plim′   or more particles and give the buffer to this function through pbuf argument,
together with the real buffer size Plim through the argument maxlocalp of this function.
As for C coded simulators, you may allocate the buffer and give the double pointer to it,
or let this function allocate the buffer of 2Plim = 2 × maxlocalp elements.
As in the level-4p library, the buffer pbuf is conceptually split into two portions of
equal size Plim. At the first call of oh4s_transbound(), the first half should have the
particles which the node accommmodates at initial, and the second half will have the
primary/secondary particles for the node in the next (usually first) simulation step. Then
you will update velocities and positions of the particles in the second half and call oh4s_
transbound() again to have the particles for the next step in the first half. This buffer
switching continues alternating the role of first and second halves each time you call oh4s_
transbound().

Fortran Interface

integer subroutine oh4s_particle_buffer(maxlocalp, pbuf)
use oh_type
implicit none
integer,intent(in)   :: maxlocalp
type(oh_particle),intent(inout) :: pbuf(:)
end subroutine


C Interface

void oh4s_particle_buffer(const int maxlocalp, struct S_particle **pbuf);


maxlocalp should have the absolute limit of each portion of the particle buffer pbuf and
thus defines Plim. That is, the particle buffer pbuf should have (or will have) 2Plim
elements. The value of Plim must not be less than P lim′   calculated by oh4s_init()
and reported through its argument of the same name, or this function aborts the
execution. On the other hand, you may (or must) specify Plim > P lim′   to ensure that
each portion of the buffer can accommodate Plim −P lim′   particles to be injected, for
example.


<!-- Page 91 -->

pbuf(Plim) (for Fortran)
**pbuf (for C) The argument pbuf should be an one-dimensional array of oh particle
type structure and have 2Plim elements in Fortran. As for C coded simulators, it
should be a double pointer to an array of S particle structure having 2Plim elements,
or a pointer to NULL (not NULL itself) to make oh4s_particle_buffer() allocate the
buffer for you and return the pointer to it through the argument.


#### 3.8.5 oh4s_per_grid_histogram()

The function (subroutine) oh4s_per_grid_histogram() is similar to its level-4p counter-
part oh4p_per_grid_histogram(), and thus is to let level-4s library functions know where
the array of per-grid histogram is located in your simulator body, or to allocate the array
for you. However, this function has an additional argument pgindex for per-grid index
whose location is also given to the library or which is allocated by this function.


Fortran Interface

subroutine oh4s_per_grid_histogram(pghgram, pgindex)
implicit none
integer,intent(inout) :: pghgram
integer,intent(inout) :: pgindex
end subroutine


C Interface

void oh4s_per_grid_histogram(int **pghgram, int **pgindex);


pghgram (for Fortran)
**pghgram (for C)
The argument pghgram for Fortran should be the origin of (D + 2)-dimensional array
for the per-grid histogram, say h(0,0,0,1,1) for the particles of the first species
in the grid-voxel at (0, 0, 0) of the primary subdomain. In C, it should be a double
pointer to such an array element, say &&h[0][0][0][0][0], or a pointer to NULL (not
NULL itself) if you want the library to allocate the array and return the pointer to its
origin element through the argument. Note that if you give the origin element to the
function, the array must have the shape ϕx × ϕy × ϕz × S × 2 where ϕd = ϕud −ϕld
and ϕβd = fsizes(β, d, F+1) or ϕβd = fsizes[F][d][β] obtained through the fsizes
argument of oh4s_init().

pgindex (for Fortran)
**pgindex (for C)
The argument pgindex for Fortran should be the origin of (D + 2)-dimensional array
for the per-grid index, say i(0,0,0,1,1) for the particles of the first species in the
grid-voxel at (0, 0, 0) of the primary subdomain. In C, it should be a double pointer
to such an array element, say &&i[0][0][0][0][0], or a pointer to NULL (not NULL
itself) if you want the library to allocate the array and return the pointer to its origin
element through the argument. The shape of the array must be same as that specified
for pghgram if the origin element is given to this function.

Note that oh4s_transbound() for Fortran will let each element of the per-grid index array
i(x, y, z, s, c) have the one-origin index of the first primary (c = 1) or secondary (c = 2)


<!-- Page 92 -->

particle of species s (∈[1, S]) in the grid-voxel at (x, y, z) in a half portion of the particle
buffer, if grid-voxel has one or more particles, i.e. h(x, y, z, s, c) > 0. For C coded simulators,
oh4s_transbound() will let i[c][s][z][y][x] have the zero-origin index of the first primary
(c = 0) or secondary (c = 1) particle of species s (∈[0, S)) in the grid-voxel at (x, y, z) in a
half portion of the particle buffer, if h[c][s][z][y][x] > 0. On the other hand, if a grid-voxel
has no particles, the corresponding element of the per-grid index array will have the index
of the first particle in the next non-empty grid-voxel, or the index next to the last particle
if there are no non-empty grid-voxels following the corresponging grid-voxel.  Therefore,
i(-1,-1,-1,1,1) = 1 for Fortran and i[0][0][-1][-1][-1] = 0 for C, always.

#### 3.8.6 oh4s_transbound()

The function oh4s transbound() at first performs operetaions for load balancing as same
as that oh1_transbound() does; examination of the per-subdomain particle population
histogram to check the balancing and (re)building of helpand-helper configuration if nec-
essary. Then for each grid-voxel set sharing a z-coordinate value, it determines the node
to accommodate particles in the set to assign primary/secondary subcuboids to each node.
After that particles in the first/second half of the particle buffer, pbuf argument of oh4s_
particle_buffer(), are transferred to satisfy load balancing, position-awareness and the
accommodation of halo particles.  Finally particles in each node are sorted according to
the coordinates of grid-voxels in which they reside and the per-grid histogram and per-
grid index in the node presented to oh4s_per_grid_histogram() are updated to show the
number of particles in each grid-voxel and the pbuf’s index of the first particle in it. The
sorted result is stored in the second/first half of pbuf.
Since the arguments of oh4s transbound() and its return value are perfectly equivalent
to those of oh1_transbound() (and its level-2/3/4p counterparts), see §3.4.4 for their
definitions.


Fortran Interface

integer function oh4s_transbound(currmode, stats)
implicit none
integer,intent(in) :: currmode
integer,intent(in) :: stats
end function


C Interface

int oh4s_transbound(const int currmode, const int stats);



#### 3.8.7 oh4s_exchange_border_data()

The function (subroutine) oh4s_exchange_border_data() performs the inter-node com-
munication for a particle-associated one-dimensional array so that its part corresponding
to halo particles in each node has the value computed by other nodes responsible of the
particles. In addition to the array buf of Plim (or more) elements, the function needs to
be given a send buffer sbuf and a receive buffer rbuf whose sizes are commonly Pcomm (or
more) reported through the argument cbufsize of oh4s_init().


<!-- Page 93 -->

Fortran Interface

subroutine oh4s_exchange_border_data(buf, sbuf, rbuf, type)
implicit none
real*8,intent(inout) :: buf
real*8,intent(out)   :: sbuf
real*8,intent(out)   :: rbuf
integer,intent(in)   :: type
end subroutine


C Interface

void oh4s_exchange_border_data(void *buf, void *sbuf, void *rbuf,
MPI_Datatype type);


buf17 should be (the pointer to) the first element of the particle-associated array of Plim
(or more) elements whose halo part will have values computed by other nodes.

sbuf should be (the pointer to) the first element of an one-dimensional array of Pcomm (or
more) elements to be used as send buffer in the function.

rbuf should be (the pointer to) the first element of an one-dimensional array of Pcomm (or
more) elements to be used as receive buffer in the function.

type should have the MPI data-type of elements of the particle-associated array.

#### 3.8.8 oh4s_map_particle_to_neighbor()

The function oh4s map particle to neighbor()  is perfectly equivalent to oh4p_map_
particle_to_neighbor() discussed in §3.7.7.  It is also as same as the level-4p counter-
part that you have to call oh4s_map_particle_to_neighbor() or oh4s_map_particle_
to_subdomain() for all particles which the local node is responsible of, i.e., those residing
in the primary/secondary subcuboids, for histogram maintenance by the library. This “all
particles resoponsible of”, however, does not means “all particles in the particle buffer”
because the buffer has halo particles which other nodes are responsible of. In fact, the nid
elements of halo particles are set to be negative by oh4s_transbound() when it received
other nodes because they should be eliminated in the next call of oh4s_transbound()18,
and thus applying the mapping function on a halo particle should make erroneous duplica-
tion of it, i.e., one on the local node and the other on the node responsible of it.


Fortran Interface

integer function oh4s_map_particle_to_neighbor(part, ps, s)
use oh_type
implicit none
type(oh_particle),intent(inout) :: part
integer,intent(in)   :: ps
integer,intent(in)   :: s
end function

17In the Fortran module file oh mod4s.F90, the arguments buf, sbuf and rbuf of are declared as real*8
type hoping it matches the type of the elements in your array.  If this is incorrect, feel free to modify the
declaration or to remove it, so that your compiler accept your calls of the library subroutines.
18Though a halo particle at a simulation step can be (or is likely) accommodated by the node as a halo
or ordinary particle, it cannot stay in the node but has to travel from the node responsible of it.


<!-- Page 94 -->

C Interface

int  oh4s_map_particle_to_neighbor(struct S_particle *part, const int ps,
const int s);



#### 3.8.9 oh4s_map_particle_to_subdomain()

The function oh4s map particle to subdomain() is perfectly equivalent to oh4p_map_
particle_to_subdomain() discussed in §3.7.8, and the caution about halo particles given
in §3.8.8 is also applicable to this function.


Fortran Interface

integer function oh4s_map_particle_to_subdomain(part, ps, s)
use oh_type
implicit none
type(oh_particle),intent(inout) :: part
integer,intent(in)   :: ps
integer,intent(in)   :: s
end function


C Interface

int  oh4s_map_particle_to_subdomain(struct S_particle *part, const int ps,
const int s);



#### 3.8.10 oh4s_inject_particle()

The function oh4s inject particle() is perfectly equivalent to oh4p_inject_particle()
discussed in §3.7.9.


Fortran Interface

integer function oh4s_inject_particle(part, ps)
use oh_type
implicit none
type(oh_particle),intent(inout) :: part
integer,intent(in)   :: ps
end function


C Interface

int  oh4s_inject_particle(const struct S_particle *part, const int ps);



#### 3.8.11 oh4s_remove_mapped_particle()

The function (subroutine) oh4s remove mapped particle()  is perfectly equivalent to
oh4p_remove_mapped_particle() discussed in §3.7.10.


<!-- Page 95 -->

Fortran Interface
subroutine oh4s_remove_mapped_particle(part, ps, s)
use oh_type
implicit none
type(oh_particle),intent(inout) :: part
integer,intent(in)   :: ps
integer,intent(in)   :: s
end subroutine


C Interface

void oh4s_remove_mapped_particle(struct S_particle *part, const int ps,
const int s);


#### 3.8.12 oh4s_remap_particle_to_neighbor()

The function oh4s remap particle to neighbor() is perfectly equivalent to oh4p_remap_
particle_to_neighbor() discussed in §3.7.11.

Fortran Interface
integer function oh4s_remap_particle_to_neighbor(part, ps, s)
use oh_type
implicit none
type(oh_particle),intent(inout) :: part
integer,intent(in)   :: ps
integer,intent(in)   :: s
end function


C Interface

int  oh4s_remap_particle_to_neighbor(struct S_particle *part, const int ps,
const int s);


#### 3.8.13 oh4s_remap_particle_to_subdomain()

The function oh4s remap particle to subdomain()  is  perfectly equivalent to oh4p_
remap_particle_to_subdomain() discussed in §3.7.12.

Fortran Interface
integer function oh4s_remap_particle_to_subdomain(part, ps, s)
use oh_type
implicit none
type(oh_particle),intent(inout) :: part
integer,intent(in)   :: ps
integer,intent(in)   :: s
end function


C Interface

int  oh4p_remap_particle_to_subdomain(struct S_particle *part, const int ps,
const int s);
