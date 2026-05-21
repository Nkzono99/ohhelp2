# 3.7 Level-4p Extension and Its Functions

Source: `doc/original/ohhelp.pdf`, pages 72-83.

<!-- Page 72 -->

Note that the boundary planes obtained by the communication between adjoined primary
subdomains are broadcasted to the helpers of the local node if necessary in the example
above. The C counterpart of the example is also simple as follows.

oh3_exchange_borders(cd[0],cd[1],ccd,1);

## 3.7 Level-4p Extension and Its Functions

#### 3.7.1 Position-Aware Particle Management

The level-4p extension is for position-aware particle management for which the load balanc-
ing particle transfer mechanism provided by oh4p_transbound() takes care that (almost)
all particles in a grid-voxel are accommodated by a particular node. In addition, the func-
tion gives you a per-grid histogram in an array, say

pghgram(ϕlx:ϕux−1, ϕlx:ϕux−1, ϕlx:ϕux−1, S, 2)

for Fortran where ϕld and ϕud (d ∈{x, y, z}) are given by an API function oh4p_init()
based on the shape of the largest subdomain. By referring to pghgram(x, y, z, s, c) you can
know the number of primary (c = 1) or secondary(c = 2) particles of species s resinding
in a grid-voxel whose integer coordinates local to its residing subdomain are (x, y, z) where
(0, 0, 0) is at the bottom-south-west corner of the primary/secondary subdomain. For C,
the array is
pghgram[ϕx × ϕy × ϕz × S × 2]
where ϕd =  ϕud −ϕld, and  the  particle population  in a  grid-voxel  at  (x, y, z)  is
pghgram[c][s][z][y][x] conceputually, where c ∈{0, 1} and s ∈[0, S).
Moreover, the primary/secondary particles of a species accommodated by a node is
sorted in its particle buffer, say pbuf, according to the coordinates of their resident grid-
voxels as follows. Unlike the lower level couterpart, pbuf should accommodate 2Plim parti-
cles where Plim is given to the library as the argument maxlocalp of oh4p_init(). Then
on the t-th (t ≥1) call of oh4p_transbound(), the first half pbuf(:,1) or pbuf[0][]
should have the input particles to the function which outputs the result of particle transfer
and sorting to the second half pbuf(:,2) or pbuf[1][] if t is odd, while the roles of first
and second half are switched if t is even.
Let base(c, s) be the index of the first primary (c = 0) or secondary (c = 1) particle of
species s, i.e.,

∑c−1 ∑S      ∑s−1
base(c, s) =        totalp(s′, i+1) +    totalp(s′, c+1) + 1
i=0 s′=1                    s′=1

for Fortran, while

∑c−1 S−1∑     ∑s−1
base(c, s) =         totalp[i][s′] +     totalp[c][s′]
i=0 s′=0                s′=0

for C. Then the particles after (2t+k)-th (k ∈{0, 1}) call of oh4p_transbound() and in
(0, 0, 0) are in

pbuf(base(c, s):base(c, s)+pghram(x, y, z, s, c+1)−1, k+1)    (Fortran)
pbuf[k][base(c, s)],  . . . , pbuf[k][base(c, s)+pghgram[c][s][z][y][x]−1]    (C)


<!-- Page 73 -->

followed by those in (0, 0, 1), then (0, 0, 2) and so on. For example, a Fortran code snip to
visit all particles in each grid-voxel is as follows.

if (has_secondary_subdomain()) then;  cc=2;  else;  cc=1;  end if
do c=1, cc
b = pbase(c)
do s=1, nspec
base(s) = b;  b = b + totalp(s,c)
end do
do z=0, sdoms(2,3,sdid(c))-sdoms(1,3,sdid(c))-1
do y=0, sdoms(2,2,sdid(c))-sdoms(1,2,sdid(c))-1
do x=0, sdoms(2,1,sdid(c))-sdoms(1,1,sdid(c))-1
do s=1, nspec
do i=1, pghgram(x,y,z,s,c)
call do_something(pbuf(base(s)+i,k+1))
end do
base(s) = base(s) + pghgram(x,y,z,s,c)
end do;  end do;  end do;  end do;  end do;


The C’s counterpart of the code above will be as follows.

for (c=0; c<has_secondary_subodmain() ? 2 : 1; c++) {
b = pbase[c];
for (s=0; s<nspec; s++) {
base[s] = b;  b += totalp[c][s];
}
for (z=0; z<sdoms[sdid[c]][2][1]-sdoms[sdid[c]][2][0]; z++) {
for (y=0; y<sdoms[sdid[c]][1][1]-sdoms[sdid[c]][1][0]; y++) {
for (x=0; x<sdoms[sdid[c]][0][1]-sdoms[sdid[c]][0][0]; x++) {
for (s=0; s<nspec; s++) {
for (i=0; i<pghgram[c][s][z][y][x]; i++)
do_something(pbuf[k][base[s]+i]);
base[s] += pghgram[c][s][z][y][x];
} } } } }


An important notice is that the maintenance of per-grid histogram is up to library
as well as the per-subdomain counterpart which is referred to as nphgram in lower levels.
Therefore, you have to call oh4p_map_particle_to_neighbor() or oh4p_map_particle_
to_subdomain() once for each and every particle15, before each call of oh4p_transbound(),
in order to let the library know the particle position. Since these functions have to exam-
ine the position of a particle, the structure of oh particle for Fortran or S particle
structure for C must have double-precision floating-point elements x, y and z  if your
simulator is three-dimensional.
You also have to remember that nid element is (almost) meaningless for you because
the mapping functions encode the information to identify the subdomain and grid-voxel in
which the particle resides in the element. Moreover, if both of the number of nodes and the
size of each subdomain are large, i.e., your whole space domain is large having grid-voxels
more than about 109, you have to #define the macro OH_BIG_SPACE in oh config.h to let
nid element be long␣long␣int. More specifically, you have to do #define the macro if

15Except for those eliminated by setting their sid elements to −1 as discussed in §3.9.


<!-- Page 74 -->

the following holds.
⌈D−1  ⌉           ∏
G =     ϕd     (N + 3D)2G ≥231

d=0

Due to the encoding of nid element and the delegation of the histogram management
to the library, it might become tough for you to find and fix problems caused by some im-
proper usage of API functions, especially those for particle mapping, injection and removal.
Therefore, the following functions check the consistency of their arguments you give, unless
you #define the macro OH_NO_CHECK in oh config.h to mean your code is well debugged
and thus the consistency check should be omitted to eliminate a few percent overhead.

oh4p_map_particle_to_neighbor()  oh4p_map_particle_to_subdomain()
oh4p_inject_particle()  oh4p_remove_mapped_particle()
oh4p_remap_particle_to_neighbor()  oh4p_remap_particle_to_subdomain()

Another important notice is that oh4p_transbound() does its best to make all particles
in a grid-voxel accommodated by a node but cannot do it if the grid-voxel has too many
particles. That is, we could have an extreme case in which all particles in the simulated
system are concentrated in a grid-voxel and thus we cannot let a node accommodate all
of them. To cope with such concentration, you have to define a threshold Phot to allow
oh4p_transbound() to split the set of particles in a grid-voxel into subsets each of which
has a cardinality not less than Phot. In other words, oh4p_transbound() may split a set
of particles in a hot-spot grid-voxel having cardinality of 2Phot or greater if otherwise a
node should have primary/secondary particles more than ordered by the load balancing
algorithm by 2Phot or more. Therefore, a node may have Pmax + 4Phot particles and thus
the particle buffer should be large enough to accommodate them.
The specific value of Phot should be determined trading offtwo factors; greater value
will satisfy the law of large numbers better when you pick a set of particles from those
in a grid-voxel (e.g., a pair of colliding particles) while load imbalance will be severer and
required particle buffer size will be larger. A compromization will be found at around 10-
times of the average number of particles in a grid-voxel, but of course the decision is up to
you. The value of Phot should be passed to oh4p_max_local_particles() which will tell
you the (minimum) size of the particle buffer taking 4Phot margin into account.


#### 3.7.2 Level-4p Functions

Level-4p extension provides the following functions.

oh4p_init() performs initialization similar to what oh3_init() and lower level counter-
parts do and that of level-4p’s own for position-aware particle management.

oh4p_max_local_particles() calculates the size of particle buffers taking the hot-spot
threshold Phot into account.

oh4p_per_grid_histogram() tells other library functions where the per-grid histogram is
located in your code.

oh4p_transbound() performs position-aware load balancing and particle transfer.

oh4p_map_particle_to_neighbor() finds the subdomain and grid-voxel which will be the
residence of a particle that stays in the original subdomain or travel to its neighbor.


<!-- Page 75 -->

oh4p_map_particle_to_subdomain() finds the subdomain and grid-voxel which will be
the residence of a particle that may go to any subdomains.

oh4p_inject_particle() injects a particle to the bottom of the particle buffer.

oh4p_remove_mapped_particle() removes a particle which you have mapped by oh4p_
map_particle_to_neighbor() or oh4p_map_particle_to_subdomain(), or injected
by oh4p_inject_particle() after the last call of oh4p_transbound().

oh4p_remap_particle_to_neighbor() does what oh4p_remove_mapped_particle() and
oh4p_map_particle_to_neighbor() do.

oh4p_remap_particle_to_subdomain() does  what  oh4p_remove_mapped_particle()
and oh4p_map_particle_to_subdomain() do.

The function API for Fortran programs is given by the module named ohhelp4p in the
file oh mod4p.F90, while API for C is embedded in ohhelp c.h.


#### 3.7.3 oh4p_init()

The function (subroutine) oh4p init() receives a number of fundamental parameters and
arrays through which oh4p_transbound() interacts with your simulator body.  It also
initializes internal data structures used in level-4p and lower level libraries. Among its
twenty-two arguments, other library functions directly refer to only the bodies of the ar-
gument pbuf as their implicit inputs. Therefore, after the call of oh4p init(), modifying
the bodies of other arguments has no effect to library functions.


Fortran Interface

subroutine oh4p_init(sdid, nspec, maxfrac, totalp, pbuf, pbase, &
maxlocalp, mycomm, nbor, pcoord, sdoms, scoord, &
nbound, bcond, bounds, ftypes, cfields, ctypes, &
fsizes, &
stats, repiter, verbose)
use oh_type
implicit none
integer,intent(out)   :: sdid(2)
integer,intent(in)    :: nspec
integer,intent(in)    :: maxfrac
integer,intent(out)   :: totalp(:,:)
type(oh_particle),intent(inout) :: pbuf(:)
integer,intent(out)   :: pbase(3)
integer,intent(in)    :: maxlocalp
type(oh_mycomm),intent(out) :: mycomm
integer,intent(inout) :: nbor(3,3,3)        ! for 3D codes.
integer,intent(in)    :: pcoord(OH_DIMENSION)
integer,intent(inout) :: sdoms(:,:,:)
integer,intent(in)    :: scoord(2,OH_DIMENSION)
integer,intent(in)    :: nbound
integer,intent(in)    :: bcond(2,OH_DIMENSION)
integer,intent(inout) :: bounds(:,:,:)
integer,intent(in)    :: ftypes(:,:)
integer,intent(in)    :: cfields(:)
integer,intent(in)    :: ctypes(:,:,:,:)


<!-- Page 76 -->

integer,intent(out)   :: fsizes(:,:,:)
integer,intent(in)    :: stats
integer,intent(in)    :: repiter
integer,intent(in)    :: verbose
end subroutine


C Interface

void oh4p_init(int **sdid, const int nspec, const int maxfrac, int **totalp,
struct S_particle **pbuf, int **pbase, const int maxlocalp,
void *mycomm, int **nbor, int *pcoord, int **sdoms,
int *scoord, const int nbound, int *bcond, int **bounds,
int *ftypes, int *cfields, int *ctypes, int **fsizes,
const int stats, const int repiter, const int verbose);


sdid
nspec
maxfrac
totalp
See §3.4.1 because the arguments above are perfectly equivalent to those of oh1_
init(). Note that nphgram which the level-1 to level-3 counterparts have is not a
member of the arguments of oh4p_init() because maintaining the per-subdomain
histogram is perfectly up to the level-4p library functions.

pbuf(Plim) (for Fortran)
**pbuf (for C)
The argument pbuf should be an one-dimensional array of oh particle type struc-
ture elements in Fortran, while it should be a double pointer to an array of S particle
structure in C. Unlike the level-2 (and level-3) counterpart, the array should be large
enough to accommodate 2Plim particles, where Plim is given through the argument
maxlocalp and should not be less than Pmax at any time. The buffer is conceptually
split into two portions of equal size, i.e., Plim. At the first call of oh4p_transbound(),
the first half should have the particles which the node accommmodates at initial, and
the second half will have the primary/secondary particles for the node in the next
(usually first) simulation step. Then you will update velocities and positions of the
particles in the second half and call oh4p_transbound() again to have the particles
for the next step in the first half. This buffer switching continues alternating the role
of first and second halves each time you call oh4p_transbound().

Note that this double buffering does not increase the required memory size for particles
from simulations with lower level libraries. That is, when you use the level-2 or level-
3 libraries the second half is hidden from you but the library functions keep it for
particle transfer.  Also note that C coded simulator body can pass pbuf having a
pointer to NULL (not NULL itself) to make oh4p_init() allocate the buffer for you
and return the pointer to it through the argument.

pbase
maxlocalp
See §3.5.2 because the arguments above are perfectly equivalent to those of oh2_
init().

mycomm


<!-- Page 77 -->

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

fsizes(2,D,F+1) (for Fortran)
**fsizes() (for C)
The argument fsizes should be a three-diemsional array of integers in Fortran where
F is the number of field-arrays defined by ftypes. In C, it should be a double pointer
to such an array of (F + 1) × D × 2 to form fsizes[F+1][D][2] conceptually, or
a pointer to NULL (not NULL itself)  if you want the library to allocate the array
and return the pointer to it through the argument. In any cases, the array element
fsizes(β, d, f) (f ∈[1, F]) or fsizes[f][d][β] (f ∈[0, F)) will have ϕld(f) (β = 0)
or ϕud(f) (β = 1) for the field-arrays of type f to notify you that the required size
of field-arrays as the counterpart of oh3_init() does. The difference is that oh4p_
init()’s has one additional element set of fsizes(β, d, F+1) or fsizes[F][d][β] for
the per-grid histogram you must (or may) allocate.

stats
repiter
verbose
See §3.4.1 because the arguments above are perfectly equivalent to those of oh1_
init().

#### 3.7.4 oh4p_max_local_particles()

The function oh4p max local particles() calculates the absolute maximum number of
particles which a node can accommodate and returns it to its caller, as the level-2 coun-
terpart oh2_max_local_particles() shown in §3.5.3 does. The difference is that this
function has one additional argument hsthresh for the hot-spot threshold Phot and takes
it into account for the calculation. The return value can be directly passed to the argument
maxlocalp of oh4p_init().


Fortran Interface

integer function oh4p_max_local_particles(npmax, maxfrac, minmargin, &
hsthresh)
implicit none
integer*8,intent(in) :: npmax
integer,intent(in)   :: maxfrac
integer,intent(in)   :: minmargin


<!-- Page 78 -->

integer,intent(in)   :: hsthresh
end function


C Interface

int  oh4p_max_local_particles(const dint npmax, const int maxfrac,
const int minmargin, const int hsthresh);


npmax should be the absolute maximum number of particles which your simulator is capable
of as a whole.

maxfrac should have the tolerance factor percentage of load imbalance α and should be
same as the argument maxfrac of oh4p_init().

minmargin should be the minimum margin by which the return value Plim has to clear over
the per node average of npmax.

hsthresh should be the hot-spot threshold Phot to define the minimum cardinality of a
subset split from the set of a concentrated grid-voxel when the particles in it are
assigned to two or more nodes.

return value is the number of particles Plim given by the following.
⌈               ⌉
P = ⌈npmax/N⌉    Plim = max( P(100 + α)/100  , P + minmargin) + 4Phot

Note that minmargin is the margin over P to be kept besides the tolerance factor α
for, e.g., initial particle accommodation in each node. Therefore it does not assure
that a node has a room for minmargin particles in simulation. If you need such a room
for, e.g., particle injection, add the room to Plim to give it the argument maxlocalp
of oh4p_init(). Also note that oh4p_init() confirms that this function has been
called prior to the call of it and its maxlocalp argument is not less than the return
value of this function, or abort the execution if both or either of them don’t hold.


#### 3.7.5 oh4p_per_grid_histogram()

The function (subroutine) oh4p per grid histogram() is to let level-4p library functions
know where the array of per-grid histogram is located in your simulator body, or to allocate
the array for you.


Fortran Interface

subroutine oh4p_per_grid_histogram(pghgram)
implicit none
integer,intent(inout) :: pghgram
end subroutine


C Interface

void oh4p_per_grid_histogram(int **pghgram);


pghgram (for Fortran)


<!-- Page 79 -->

**pghgram (for C)
The argument pghgram for Fortran should be the origin of (D + 2)-dimensional array
for the per-grid histogram, say h(0,0,0,1,1) for the particles of the first species in the
grid-voxel at (0, 0, 0) (if three-dimensional simulation) of the primary subdomain. In
C, it should be a double pointer to such an array element, say &&h[0][0][0][0][0],
or a pointer to NULL (not NULL itself) if you want the library to allocate the array
and return the pointer to its origin element through the argument.  Note that  if
you give the origin element to the function, the array must have the shape, if three-
dimensional, ϕx × ϕy × ϕz × S × 2 where ϕd = ϕud −ϕld and ϕβd = fsizes(β, d, F+1)
or ϕβd = fsizes[F][d][β] obtained through the fsizes argument of oh4p_init().

#### 3.7.6 oh4p_transbound()

The function oh4p transbound() at first performs operetaions for load balancing as same
as that oh1_transbound() does; examination of the per-subdomain particle population
histogram to check the balancing and (re)building of helpand-helper configuration if nec-
essary. Then for each grid-voxel, it determines the node to accommodate particles in the
grid-voxel or a set of nodes to do that if the grid-voxel is a hot-spot. After that particles in
the first/second half of the particle buffer, pbuf argument of oh4p_init(), are transferred
to satisfy load balancing and position-awareness. Finally particles in each node are sorted
according to the coordinates of grid-voxels in which they reside and the per-grid histogram
in the node presented to oh4p_per_grid_histogram() is updated to show the number of
particles in each grid-voxel that the node accommodates. The sorted result is stored in the
second/first half of pbuf.
Since the arguments of oh4p transbound() and its return value are perfectly equivalent
to those of oh1_transbound() (and oh2_transbound() and oh3_transbound()), see §3.4.4
for their definitions.


Fortran Interface

integer function oh4p_transbound(currmode, stats)
implicit none
integer,intent(in) :: currmode
integer,intent(in) :: stats
end function


C Interface

int oh4p_transbound(const int currmode, const int stats);



#### 3.7.7 oh4p_map_particle_to_neighbor()

The function oh4p map particle to neighbor() returns the identifier of the subdomain in
which the primary (ps = 0) or secondary (ps = 1) particle part of spec s will reside and to
which the primary or secondary subdomain of the local node likely adjoins. Alghough the
function, unlike the level-3 couterpart oh3_map_particle_to_neighbor(), accepts parti-
cles traveling a non-neighboring subdomain due to, for example, initial particle distribution
or particle warp, using the relative function oh4p_map_particle_to_subdomain() is rec-
ommended because it is faster for such particles.
Also unlike the level-3 counterpart oh3_map_particle_to_neighbor(), you have to call
this function or oh4p_map_particle_to_subdomain() for all particles which the local node


<!-- Page 80 -->

accommodates so that the library maintains the per-subdomain and per-grid histograms.
Another differences from the level-3 function are that the particle itself is passed through
the first argument rather than its position, and its species s has to be given as the third
argument.


Fortran Interface

integer function oh4p_map_particle_to_neighbor(part, ps, s)
use oh_type
implicit none
type(oh_particle),intent(inout) :: part
integer,intent(in)   :: ps
integer,intent(in)   :: s
end function


C Interface

int  oh4p_map_particle_to_neighbor(struct S_particle *part, const int ps,
const int s);


part (for Fortran)
*part (for C)
The first argument part should be a oh particle type structured data in Fortran,
while it should be a pointer to S particle structure in C. In both cases, the actual
argument structure may be updated as discussed later.

ps should be 0 for a primary particle, or 1 for a secondary particle.

s should be the species identifier of the particle in [1, S] in Fortran while in [0, S) in C.
Note that if the particle structure has the spec element, s must be equal to the value
of the element of part.

return value is the identifier of the subdomain in which the particle will reside, or −1 if
such a subdomain is not found as discussed later.

The function at first examines whether the particle is in the primary (ps = 0) or
secondary (ps = 1) subdomain of the local node and returns its identifier if the particle
is in it, referring to the subdomain boundaries given by or set to the argument sdoms
of oh4p_init().  Otherwise, it assumes that the particle has moved into a subdomain
adjoining to the primary/secondary subdomain and returns the identifier of the subdomain
into which the particle has moved, referring to the neighboring infomation given by or set
to the argument nbor of oh4p_init(), or that in the helpand.
In the latter case of the boundary crossing, the periodic boundary condition of the whole
space domain is taken care of by the function. Therefore, the coordinates given by x, y and
z elements of the argument part should be raw ones without wraparound. Moreover, the
elements in the actual argument are updated by the function if the particle has crossed a
periodic boundary. For example, if the particle has crossed the periodic boundary plane
perpendicular to x-axis, the actual argument variable x is updated as follows.
{
x + (∆ux −∆lx)γx  x < ∆lx · γx                    x ←
x −(∆ux −∆lx)γx  x ≥∆ux · γx


<!-- Page 81 -->

On the other hand,  if the particle has crossed a non-periodic boundary of the whole
space domain, the function returns −1 to indicate that the particle is out of bounds16. To
examine the boundary condition, the function refers to the conditions given through the
argument bcond or bounds of oh4p_init(). The function also returns −1 if the particle
has moved into a non-exsistent neighbor, which may be defined by nbor.

#### 3.7.8 oh4p_map_particle_to_subdomain()

The function oh4p map particle to subdomain() returns the identifier of the subdomain
in which the primary (ps = 0) or secondary (ps = 1) particle part of spec s will reside. The
difference between this and the relative function oh4p_map_particle_to_neighbor() is
that this function more quickly find the identifier of the non-neighboring resident subdomain
for the particle and thus is desgined to be used for, e.g., initial particle disribution, particle
warp, and so on. Of course you may use this function always but have to remember that
it is much slower than oh4p_map_particle_to_neighbor() for particles staying in the
primary/secondary subdomain or just crossing a subdomain boundary.
Unlike the level-3 counterpart oh3_map_particle_to_subdomain(), you have to call
this function or oh4p_map_particle_to_neighbor() for all particles which the local node
accommodates so that the library maintains the per-subdomain and per-grid histograms.
The other differences from the level-3 function are that the particle itself is passed through
the first argument rather than its position, its primariness/secondariness has to be given
as the second argument ps, and its species s has to be given as the third argument. In
addition, this function takes care of crossing periodic boundaries of the whole system.
Since the arguments of oh4p_map_particle_to_subdomain() and its return value are
perfectly equivalent to those of oh4p_map_particle_to_neighbor(), though this function
is much slower and thus you are discouraged to use it in usual cases, see §3.7.7 for their
definitions.


Fortran Interface

integer function oh4p_map_particle_to_subdomain(part, ps, s)
use oh_type
implicit none
type(oh_particle),intent(inout) :: part
integer,intent(in)   :: ps
integer,intent(in)   :: s
end function


C Interface

int  oh4p_map_particle_to_subdomain(struct S_particle *part, const int ps,
const int s);



#### 3.7.9 oh4p_inject_particle()

The function oh4p inject particle() injects a given particle at the bottom of pbuf and
maintains per-subdomain and per-grid histograms according to its residence subdomain,

16The values in elements of part are kept unless the particle has crossed two or more contacting space
domain boundaries including periodic ones at once. More specifically, the function examines boundary
crossing in the order of yz, xz and then xy planes if D = 3, and updates part’s elements x, y and z in this
order if the corresponding boundary planes are periodic.


<!-- Page 82 -->

grid-voxel, primariness and species. Note that the number of particles injected in a simu-
lation step should not be greater than Plim −Qn.


Fortran Interface

integer function oh4p_inject_particle(part, ps)
use oh_type
implicit none
type(oh_particle),intent(inout) :: part
integer,intent(in)   :: ps
end function


C Interface

int  oh4p_inject_particle(const struct S_particle *part, const int ps);


part (for Fortran)
*part (for C)
The argument part should be a oh particle structure in Fortran, or a pointer to
S particle structure in C, to be injected. Elements except for nid in the given par-
ticle structure should be completely set with significant values in advance, especially
if S ̸= 1, spec elements which are referred to by the function to update histograms.

ps should be 0 for a primary particle, or 1 for a secondary particle. Sepcifying primariness/
secondariness is important for good performace  if the particle is injected into (or
around) primary/secondary subdomain of the local node.

return value is the identifier of the subdomain in which the particle will reside, or −1 if
such a subdomain is not found.


#### 3.7.10 oh4p_remove_mapped_particle()

The function (subroutine) oh4p remove mapped particle() removes a particle which
you have mapped by oh4p_map_particle_to_neighbor() or oh4p_map_particle_to_
subdomain(),  or  injected by oh4p_inject_particle()  after the  last  call  of oh4p_
transbound(). Since the mapping or injection incremented counter elements in the per-
subdomain and per-grid histograms, you have to call this function to cancel the increment
when you discard the particle, instead of setting its nid element to −1.


Fortran Interface

subroutine oh4p_remove_mapped_particle(part, ps, s)
use oh_type
implicit none
type(oh_particle),intent(inout) :: part
integer,intent(in)   :: ps
integer,intent(in)   :: s
end subroutine


<!-- Page 83 -->

C Interface

void oh4p_remove_mapped_particle(struct S_particle *part, const int ps,
const int s);


part (for Fortran)
*part (for C)
The argument part should be a oh particle structure in Fortran, or a pointer to
S particle structure in C, to be removed.

ps should be 0 for a primary particle, or 1 for a secondary particle.

s should be the species identifier of the particle in [1, S] in Fortran while in [0, S) in C.

Note that the nid element of the particle part is set to −1 by the function.

#### 3.7.11 oh4p_remap_particle_to_neighbor()

The function oh4p remap particle to neighbor() cancels the mapping of the primary
(ps = 0) or secondary (ps = 1) particle part of spec s done by functions such as oh4p_map_
particle_to_neighbor() and then find the subdomain in which the particle will reside
to return its identifer. That is, this function does in series what oh4p_remove_mapped_
particle() and oh4p_map_particle_to_neighbor() do.

Fortran Interface
integer function oh4p_remap_particle_to_neighbor(part, ps, s)
use oh_type
implicit none
type(oh_particle),intent(inout) :: part
integer,intent(in)   :: ps
integer,intent(in)   :: s
end function


C Interface

int  oh4p_remap_particle_to_neighbor(struct S_particle *part, const int ps,
const int s);


part (for Fortran)
*part (for C)
The argument part should be a oh particle structure in Fortran, or a pointer to
S particle structure in C, to be remapped.

ps should be 0 for a primary particle, or 1 for a secondary particle.

s should be the species identifier of the particle in [1, S] in Fortran while in [0, S) in C.

#### 3.7.12 oh4p_remap_particle_to_subdomain()

The function oh4p remap particle to subdomain() cancels the mapping of the primary
(ps = 0) or secondary (ps = 1) particle part of spec s done by functions such as oh4p_map_
particle_to_neighbor() and then find the subdomain in which the particle will reside
to return its identifer. That is, this function does in series what oh4p_remove_mapped_
particle() and oh4p_map_particle_to_subdomain() do.
