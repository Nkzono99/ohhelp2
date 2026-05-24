# 3.5 Level-2 Library Functions

Source: `doc/v1/original/ohhelp-man.pdf`, pages 35-41.

<!-- Page 35 -->

C Interface

void oh1_reduce(void *pbuf, void *sbuf, int pcount, int scount,
MPI_Datatype ptype, MPI_Datatype stype,
MPI_Op pop, MPI_Op sop);


pbuf should be (the pointer to) the first element of the data buffer to be reduced in the
primary family. The buffer is replaced with the reduction result.

sbuf should be (the pointer to) the first element of the data buffer to be reduced in the
secondary family. The buffer will remain unchanged.

pcount should have the number of ptype elements to be reduced in the primary family.
This value should match scount of the call in the helpers.

scount should have the number of stype elements to be reduced in the secondary family.
This value should match pcount of the call in the helpand.

ptype should have the MPI data-type of elements to be reduced in the primary family.
This value should match stype of the call in the helpers.

stype should have the MPI data-type of elements to be reduced in the secondary family.
This value should match ptype of the call in the helpand.

pop should have the MPI operator for the reduction in the primary family.  This value
should match sop of the call in the helpers.

sop should have the MPI operator for the reduction in the secondary family. This value
should match pop of the call in the helpand.

## 3.5 Level-2 Library Functions

Level-2 library provides the following functions.

oh2_init() performs initialization similar to what oh1_init() does and that of level-2’s
own for particle buffers.

oh2_max_local_particles() calculates the size of particle buffers.

oh2_transbound() performs load balancing similar to oh1_transbound() and transfers
particles according to the schedule.

oh2_inject_particle() injects a particle to the bottom of the particle buffer.

oh2_remap_injected_particle() maintains library’s internal state for an injected but
not mapped particle.

oh2_remove_injected_particle() removes an injected particle maintaining library’s in-
ternal state.

oh2_set_total_particles() tells the library you will inject/remove particles before your
first call oh2_transbound().

The function API for Fortran programs is given by the module named ohhelp2 in the
file oh mod2.F90, while API for C is embedded in ohhelp c.h.


<!-- Page 36 -->

#### 3.5.1 Particle Data Type

Since oh2_transbound() and its higher-level counterparts transfer particles among nodes,
they need to know how each particle is represented. The default configuration of the struct
to represent a particle for C-coded simulator body and the library, namely S particle is
defined in the C header file oh part.h, while its Fortran counterpart oh particle is given
in oh type.F90. Both definitions are of course consistent with the following elements.

x, y and z are for the x/y/z coordinates of the position at which a particle resides.

vx, vy and vz are for the x/y/z components of the velocity of a particle.

pid is the unique identifier of a particle by which, for example, you can trace the trajectory
of the particle.

nid is the identifier of the subdomain in which a particle resides.

spec is the identifier of the species which a particle belongs to.

In the elements listed above, nid is essential for the library and must have the identifier
of the subdomain in which the particle resides at the call of oh2_transbound(). In addition,
spec is also necessary if S > 1 and you inject particles by the library function oh2_inject_
particle() or its level-4p/4s counterparts oh4p_inject_particle() or oh4s_inject_
particle(), and must has a value in [1, S] if your simulator is coded in Fortran, or in
[0, S−1] for C-coded simulators.
On the other hand, you may freely modify the definitions in oh part.h and,  if your
simulator is coded in Fortran, oh type.F90, by adding, removing and/or renaming other
elements. However, if you use the level-4p/4s extension, S particle in oh part.h should
have elements x, y and z (or the first one or two if D < 3), their type should be double or
float, and oh type.F90 should be consistent with them if you work with Fortran. As for
spec, you may remove it toghether with #define of OH HAS SPEC if S = 1 or you use neither
oh2_inject_particle(), oh4p_inject_particle() nor oh4p_inject_particle() and
want to save four bytes for each particle.
Another caution to the user of level-4p/4s extension is that the nid element can be a 64-
bit integer rathan than 32-bit if you made OH_BIG_SPACE defined in oh config.h (see §3.3).
C programmers should also notice that the element has type OH nid t which is defined as
long␣long␣int or int when OH_BIG_SPACE is defined or not, respectively. We will revisit
this issue in some further details in §3.7.
The verbatim definitions of S particle and oh particle are as follows.

#include "oh_config.h"
#ifdef  OH_BIG_SPACE
typedef long long int OH_nid_t;
#else
typedef int OH_nid_t;
#endif

struct S_particle {
double x, y, z, vx, vy, vz;
long long int pid;
OH_nid_t nid;
int spec;
};
#define OH_HAS_SPEC


<!-- Page 37 -->

type oh_particle
sequence
real*8    :: x, y, z, vx, vy, vz
integer*8 :: pid
#ifdef OH_BIG_SPACE
integer*8 :: nid
#else
integer   :: nid
#endif
integer   :: spec
end type


#### 3.5.2 oh2_init()

The function (subroutine) oh2 init() receives a few fundamental parameters and arrays
through which oh2_transbound() interacts with your simulator body.  It also initializes
internal data structures used in level-1 and level-2 libraries. Among its fourteen arguments,
other library functions directly refer to only the bodies of the arguments nphgram and pbuf
as their implicit inputs. Therefore, after the call of oh2 init(), modifying the bodies of
other arguments has no effect to library functions.


Fortran Interface

subroutine oh2_init(sdid, nspec, maxfrac, nphgram, totalp, &
pbuf, pbase, maxlocalp, mycomm, nbor, pcoord, &
stats, repiter, verbose)
use oh_type
implicit none
integer,intent(out)   :: sdid(2)
integer,intent(in)    :: nspec
integer,intent(in)    :: maxfrac
integer,intent(inout) :: nphgram(:,:,:)
integer,intent(out)   :: totalp(:,:)
type(oh_particle),intent(inout) :: pbuf(:)
integer,intent(out)   :: pbase(3)
integer,intent(in)    :: maxlocalp
type(oh_mycomm),intent(out) :: mycomm
integer,intent(inout) :: nbor(3,3,3)        ! for 3D codes.
integer,intent(in)    :: pcoord(OH_DIMENSION)
integer,intent(in)    :: stats
integer,intent(in)    :: repiter
integer,intent(in)    :: verbose
end subroutine


C Interface

void oh2_init(int **sdid, int nspec, int maxfrac, int **nphgram,
int **totalp, struct S_particle **pbuf, int **pbase,
int maxlocalp, void *mycomm, int **nbor,
int *pcoord, int stats, int repiter, int verbose);


sdid
nspec


<!-- Page 38 -->

pbase(1)=pbase[0]=0                                 pbase(2)=pbase[1]              pbase(3)=pbase[2]    maxlocalp



totalp(1,1)     totalp(2,1)       totalp(3,1)      totalp(1,2)     totalp(2,2)      totalp(3,1)pbuf      =totalp[0][0]   =totalp[0][1]     =totalp[0][2]    =totalp[1][0]   =totalp[1][1]    =totalp[1][2]




Figure 8: Particle buffer and related variables.


maxfrac
nphgram
totalp
See §3.4.1 because the arguments above are perfectly equivalent to those of oh1_
init().

pbuf(Plim) (for Fortran)
**pbuf (for C)
The argument pbuf should be an one-dimensional array of oh particle type struc-
ture elements in Fortran, while it should be a double pointer to an array of S particle
structure in C. The array have to be large enough to accommodate Plim particles,
where Plim  is given through the argument maxlocalp and should not be less than
Pmax at any time (Figure 8). In C code, pbuf can be a pointer to NULL (not NULL
itself) to make oh2 init() allocate the buffer for you and return the pointer to it
through the argument.

pbase(3) (for Fortran)
**pbase (for C)
The argument pbase should be an one dimensional array of three elements in Fortran,
while it should be a double pointer to such an array in C.  After zero-cleared by
oh2 init(), each call of oh2_transbound() make the array for the local node n
have 0, Qnn and Qn in this order to represent the zero-origin displacement of the first
primary particle and the first secondary particle, and the head of unused region of
pbuf. That is, the first Qnn portion of pbuf is used for primary particles, while the
second Qparent(n)n    = Qn −Qnn particles are for secondary particles. In C code, pbase
can be a pointer to NULL (not NULL itself) to make oh2 init() allocate the array for
you and return the pointer to it through the argument.

maxlocalp should have the absolute limit of the particle buffer pbuf and thus defines Plim.
You may ask the library function oh2_max_local_particles() to calculate Plim from
the system-wide absolute limit. Note that oh2 init() allocates a buffer for particle
transfer and thus your machine should have memory large enough to have 2 × Plim
particles per computation node.

mycomm
nbor
pcoord
stats
repiter
verbose
See §3.4.1 because the arguments above are perfectly equivalent to those of oh1_
init().


<!-- Page 39 -->

Note that oh2 init() has neither arguments rcounts nor scounts which oh1_init() has,
because particle transfer in oh2_transbound() makes it unnecessary to report transfer
schedule.


#### 3.5.3 oh2_max_local_particles()

The function oh2 max local particles() calculates the absolute maximum number of
particles which a node can accommodate and returns it to its caller. The return value can
be directly passed to the argument maxlocalp of oh2_init().


Fortran Interface

integer function oh2_max_local_particles(npmax, maxfrac, minmargin)
implicit none
integer*8,intent(in) :: npmax
integer,intent(in)   :: maxfrac
integer,intent(in)   :: minmargin
end function


C Interface

int  oh2_max_local_particles(long long int npmax, int maxfrac,
int minmargin);


npmax should be the absolute maximum number of particles which your simulator is capable
of as a whole.

maxfrac should have the tolerance factor percentage of load imbalance α and should be
same as the argument maxfrac of oh2_init().

minmargin should be the minimum margin by which the return value Plim has to clear over
the per node average of npmax.

return value is the number of particles Plim given by the following.
⌈               ⌉
P = ⌈npmax/N⌉    Plim = max( P(100 + α)/100  , P + minmargin)

Note that minmargin is the margin over P to be kept besides the tolerance factor α
for, e.g., initial particle accommodation in each node. Therefore it does not assure
that a node has a room for minmargin particles in simulation. If you need such a room
for, e.g., particle injection, add the room to Plim to give it the argument maxlocalp
of oh2_init().

#### 3.5.4 oh2_transbound()

The function oh2 transbound() at first performs operetaions for load balancing as same
as that oh1_transbound() does; examination of nphgram to check the balancing and
(re)building of helpand-helper configuration updating mycomm if necessary. Then, instead
of reporting the particle transfer schedule, it sends particles in pbuf to other nodes and
receives them into pbuf, updates totalp and pbase according to the transfer result, and
clears nphgram with zeros. Note that the arrays nphgram, pbuf, totalp and pbase and the
structure mycomm were given to oh2_init() as its arguments.


<!-- Page 40 -->

The arguments of oh2 transbound() and its return value, besides these global arrays
and structures, are perfectly equivalent to those of oh1_transbound() and thus see §3.4.4
for them.


Fortran Interface

integer function oh2_transbound(currmode, stats)
implicit none
integer,intent(in) :: currmode
integer,intent(in) :: stats
end function


C Interface

int oh2_transbound(int currmode, int stats);



#### 3.5.5 oh2_inject_particle()

The function (subroutine) oh2 inject particle() injects a given particle at the bottom of
pbuf and increase an element of nphgram according to its residence subdomain and species.
Note that the number of particles injected in a simulation step should not be greater than
Plim −Qn.


Fortran Interface

subroutine oh2_inject_particle(part)
use oh_type
implicit none
type(oh_particle),intent(in) :: part
end subroutine


C Interface

void oh2_inject_particle(struct S_particle *part);


part (for Fortran)
*part (for C)
The argument part should be a oh particle structure in Fortran, or a pointer to
S particle structure in C, to be injected. Elements in the given particle structure
should be completely set with significant values in advance, especially for nid and, if
S ̸= 1, spec elements which are referred to by the function to update nphgram. See
§3.9 for further discussion on injection.


#### 3.5.6 oh2_remap_injected_particle()

The function (subroutine) oh2 remap injected particle() maintains library’s internal
state of a particle injected by oh2_inject_particle() with nid element −1.


<!-- Page 41 -->

Fortran Interface

subroutine oh2_remap_injected_particle(part)
use oh_type
implicit none
type(oh_particle),intent(in) :: part
end subroutine


C Interface

void oh2_remap_injected_particle(struct S_particle *part);


part (for Fortran)
*part (for C)
The argument part should be pbuf(Qn +k) being oh particle structure in Fortran,
or a pointer pbuf + Qn + k −1 to S particle structure in C, for the k-th injected
particle in a simulation step.  Elements in the given particle structure should be
completely set with significant values in advance, especially for nid and, if S ̸= 1,
spec elements which are referred to by the function to update nphgram. See §3.9 for
further discussion on injection.

#### 3.5.7 oh2_remove_injected_particle()

The function (subroutine) oh2 remove injected particle() removes a particle injected
by oh2_inject_particle().

Fortran Interface

subroutine oh2_remove_injected_particle(part)
use oh_type
implicit none
type(oh_particle),intent(inout) :: part
end subroutine


C Interface

void oh2_remove_injected_particle(struct S_particle *part);


part (for Fortran)
*part (for C)
The argument part should be pbuf(Qn +k) being oh particle structure in Fortran,
or a pointer pbuf + Qn + k −1 to S particle structure in C, for the k-th injected
particle in a simulation step.  Elements in the given particle structure should be
completely set with significant values in advance, especially for nid and, if S ̸= 1,
spec elements which are referred to by the function to update nphgram. See §3.9 for
further discussion on injection.

#### 3.5.8 oh2_set_total_particles()

The function (subroutine) oh2 set total particles() tells the library that you will inject
and/or remove particles before the first call of oh2_transbound(). This function consults
the array nphgram which must be consistent with the contents of particle buffer pbuf, and
updates (initializes) totalp according to nphgram.
