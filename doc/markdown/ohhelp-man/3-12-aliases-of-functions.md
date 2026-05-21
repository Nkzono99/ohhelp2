# 3.12 Aliases of Functions

Source: `doc/original/ohhelp-man.pdf`, pages 100-121.

<!-- Page 100 -->

Note that your message is assumed fundamental and thus will be printed if verbose is 1
or larger. Also note that oh1 verbose() has MPI_Barrier() in it and thus it should be
called from all nodes to avoid deadlock. For example;

if (sdid(2).ge.0) then
oh1_verbose(’secondary particle push\0’)
call particle_push(...)
end if

will cause deadlock because the root node of helpand-helper tree will not call oh1 verbose()
while others do. Therefore, the code above should be modified as follows.

if (currmode.ne.0)
oh1_verbatim(’secondary particle push\0’)
if (sdid(2).ge.0) call particle_push(...)
end if


## 3.12 Aliases of Functions

As shown in previous sections, all library functions have one of prefixes ‘oh1 ’, ‘oh2 ’, ‘oh3 ’,
‘oh4p’ or ‘oh4s ’ to show the library layer they belong to. Although this naming makes it
clear that in order to use a function, say oh2_inject_particle(), you have to incorporate
level-2 or level-3 library, it will be tiresome to remember the layer number which a function
belongs to especially when you use (almost) everything provided by the layer you chose and
by lower ones.
Therefore, the library has special header files ohhelp f.h for Fortran and ohhelp c.h for
C to give API function aliases which just have a common prefix ‘oh ’. To use these files,
you have to #define a constant OH_LIB_LEVEL as the number of the layer you choose, i.e.,
1, 2, 3 or 4 in your source files, or have to edit the lines defining that in oh config.h. Then
you have the aliases shown in Table 1 according to the layer number you chose.
Note that both header files #include’s the header file oh config.h, and ohhelp c.h does
the followings in addition to aliasing.

- #include the standard MPI header file mpi.h.

- Declares prototypes of library functions in use according to the layer you chose.

- Define struct named S mycommc.

- #include the header file oh part.h to define struct named S particle if you choose
level-2 or higher.

Also note that the function oh13_init() does not have any aliases.

### 3.13 Sample Code

This section gives examples of application of the level-3 OhHelp library to tiny 3-dimensional
PIC simulators coded in Fortran and C. The main loop of these codes consists of calls of
the following subroutines/functions, besides libraray fucntions.

particle push() does what its name implies. The acceleration vector of each particle
is calculated by a subroutine/function named lorentz() whose code is outside the
scope of this document.


<!-- Page 101 -->

Table 1: Aliases of Library Functions

layer         alias                             autonym
any        oh_neighbors()                     oh1_neighbors()
oh_families()                      oh1_families()
oh_acc_mode()                      oh1_acc_mode()
oh_broadcast()                     oh1_broadcast()
oh_all_reduce()                    oh1_all_reduce()
oh_reduce()                        oh1_reduce()
oh_init_stats()                    oh1_init_stats()
oh_stats_time()                    oh1_stats_time()
oh_show_stats()                    oh1_show_stats()
oh_print_stats()                   oh1_print_stats()
oh_verbose()                       oh1_verbose()
1          oh_init()                          oh1_init()
oh_transbound()                    oh1_transbound()
2/3        oh_max_local_particles()           oh2_max_local_particles()
oh_inject_particle()               oh2_inject_particle()
oh_remap_injected_particle()       oh2_remap_injected_particle()
oh_remove_injected_particle()      oh2_remove_injected_particle()
2/3/4p/4s   oh_set_total_particles()           oh2_set_total_particles()
2          oh_init()                          oh2_init()
oh_transbound()                    oh2_transbound()
3/4p/4s    oh_grid_size()                     oh3_grid_size()
oh_bcast_field()                   oh3_bcast_field()
oh_reduce_field()                  oh3_reduce_field()
oh_allreduce_field()               oh3_allreduce_field()
oh_exchange_borders()              oh3_exchange_borders()
3          oh_init()                          oh3_init()
oh_transbound()                    oh3_transbound()
oh_map_particle_to_neighbor()      oh3_map_particle_to_neighbor()
oh_map_particle_to_subdomain()     oh3_map_particle_to_subdomain()
4p         oh_init()                          oh4p_init()
oh_max_local_particles()           oh4p_max_local_particles()
oh_per_grid_histogram()            oh4p_per_grid_histogram()
oh_transbound()                    oh4p_transbound()
oh_map_particle_to_neighbor()      oh4p_map_particle_to_neighbor()
oh_map_particle_to_subdomain()     oh4p_map_particle_to_subdomain()
oh_inject_particle()               oh4p_inject_particle()
oh_remove_mapped_particle()        oh4p_remove_mapped_particle()
oh_remap_particle_to_neighbor()    oh4p_remap_particle_to_neighbor()
oh_remap_particle_to_subdomain()   oh4p_remap_particle_to_subdomain()
4s          oh_init()                          oh4s_init()
oh_particles_buffer()              oh4s_particles_buffer()
oh_per_grid_histogram()            oh4s_per_grid_histogram()
oh_transbound()                    oh4s_transbound()
oh_exchange_border_data()          oh4s_exchange_border_data()
oh_map_particle_to_neighbor()      oh4s_map_particle_to_neighbor()
oh_map_particle_to_subdomain()     oh4s_map_particle_to_subdomain()
oh_inject_particle()               oh4s_inject_particle()
oh_remove_mapped_particle()        oh4s_remove_mapped_particle()
oh_remap_particle_to_neighbor()    oh4s_remap_particle_to_neighbor()
oh_remap_particle_to_subdomain()   oh4s_remap_particle_to_subdomain()


<!-- Page 102 -->

current scatter() also does what its name indicates. The contribution of each particle
to the current densities at grid points surrouding it is calculated by an out-of-scope
subroutine/function named scatter().

add boundary current() culculates current density vectors of the grid points in boundary
planes of a subdomain adding those obtained from neighboring subdomains to those
calcluated by the family members of the local onde. This calls add_boundary_curr()
for each boundary.

field solve e() is the first half of a leapfrog field solver to update electric field vectors.
The rotation of magnetic field ∇× B for the electirc field vector of each grid point is
calculated by an out-of-scope subroutine/fucntion named rotate_b().

field solve b() is the second half of a leapfrog field solver to update magnetic field vec-
tors.  Similar to its electric counterpart, ∇× E  is calculated by an out-of-scope
subroutine/fucntion named rotate_e().

In addition to them, it is assumed that we have two out-of-scope subroutines/functions
for initialization, namely initialize_eb() for electromagnetic field and initialize_
particles() for particles.


#### 3.13.1 Fortran Sample Code

The Fortran sample code given in the file sample.F90 is composed in a Fortran module
named sample. It starts with the following lines to #include the header file ohhelp f.h for
level-3 function aliasing and to use the Fortran module ohhelp3 defined in oh mod3.F90
for the inteface’s of level-3 and lower level libray functions.

#define OH_LIB_LEVEL 3
#include "ohhelp_f.h"
module sample
use ohhelp3



Declaration
At first, we declare a few parameter’s, MAXFRAC = 20 for maxfrac argument of oh3_
init(), field-array identifiers for electromagnetic field-array eb(:,:,:,:,:) (FEB = 1) and
current density cd(:,:,:,:,:) (FCD = 2), and element numbers of these arrays, EX, BX,
JX and so on.

implicit none
integer,parameter     :: MAXFRAC=20
integer,parameter     :: FEB=1,FCD=2
integer,parameter     :: EX=1,EY=2,EZ=3,BX=4,BY=5,BZ=6
integer,parameter     :: JX=1,JY=2,JZ=3


Then the variables to pass oh3_init() are declared with the same names as defined
in §3.6.1. We also declare two field-arrays, eb(:,:,:,:,:) for electromagnetic field and
cd(:,:,:,:,:) for current density.

integer               :: sdid(2)
integer,allocatable   :: nphgram(:,:,:)
integer,allocatable   :: totalp(:,:)


<!-- Page 103 -->

type(oh_particle),allocatable&
:: pbuf(:)
integer               :: pbase(3)
type(oh_mycomm)       :: mycomm
integer               :: nbor(3,3,3)
integer,allocatable   :: sdoms(:,:,:)
integer               :: bcond(2,OH_DIMENSION)
integer,allocatable   :: bounds(:,:,:)
integer               :: ftypes(7,3)
integer               :: cfields(3)
integer               :: ctypes(3,2,1,2)
integer               :: fsizes(2,OH_DIMENSION,2)
real*8,allocatable    :: eb(:,:,:,:,:)
real*8,allocatable    :: cd(:,:,:,:,:)


The last declarative work is to give the prototypes of out-of-scope subroutines.

interface
subroutine initialize_eb(eb, sdom)
implicit none
real*8            :: eb(:,:,:,:)
integer           :: sdom(:,:)
end subroutine
subroutine initialize_particles(pbuf, nspec, nphgram)
use oh_type
implicit none
type(oh_particle) :: pbuf(:)
integer           :: nspec
integer           :: nphgram(:,:)
end subroutine
subroutine lorentz(eb, x, y, z, s, acc)
implicit none
real*8            :: eb(:,:,:,:)
real*8            :: x, y, z
integer           :: s
real*8            :: acc(OH_DIMENSION)
end subroutine
subroutine scatter(p, s, c)
use oh_type
implicit none
type(oh_particle) :: p
integer           :: s
real*8            :: c(3,2,2,2)
end subroutine
subroutine rotate_b(eb, x, y, z, rot)
implicit none
real*8            :: eb(:,:,:,:)
integer           :: x, y, z
real*8            :: rot(OH_DIMENSION)
end subroutine
subroutine rotate_e(eb, x, y, z, rot)
implicit none
real*8            :: eb(:,:,:,:)
integer           :: x, y, z


<!-- Page 104 -->

real*8            :: rot(OH_DIMENSION)
end subroutine
end interface



Subroutine pic()
The first subroutine pic() is the core of the simulator and is called with a few simulation
parameters to be given to the arguments of oh3_init(), which are nspec, pcoord(3) and
scoord(2,3).  It also has arguments npmax for the absolute maximum number of the
particle in the whloe simulation and nstep to determine the number of simulation steps.

contains
subroutine pic(nspec, pcoord, scoord, npmax, nstep)
implicit none
integer             :: nspec
integer             :: pcoord(OH_DIMENSION)
integer             :: scoord(2,OH_DIMENSION)
integer*8           :: npmax
integer             :: nstep

integer             :: n, t, maxlocalp, currmode


The first job is to allocate the array totalp(nspec,2) and a few other arrays having
N as the size of a dimension,  i.e., nphgram, sdoms and bounds. The number of nodes
N = Πx × Πy × Πz  is calculated from pcoord. We also allocate the particle array
pbuf whose size maxlocalp is determined by oh2_max_local_particles() from npmax
and MAXFRAC without additional minimum margin.

allocate(totalp(nspec,2))
n = pcoord(1) * pcoord(2) * pcoord(3)
allocate(nphgram(n, nspec, 2))
allocate(sdoms(2, OH_DIMENSION, n))
allocate(bounds(2, OH_DIMENSION, n))

maxlocalp = oh_max_local_particles(npmax, MAXFRAC, 0)
allocate(pbuf(maxlocalp))


We continue initial setting of variables for oh3_init(); nbor and sdoms have the special
values to delegate their initializations to oh3_init(); bcond indicates fully periodic bound-
ary conditions by having 1s in all of its elements; the first element of ftypes for eb shows
that the range for its broadcast is from eb(1,-1,-1,-1,:) to eb(6,σx,σy,σz,:), while
the second element for cd gives that for the reduction being from cd(1,-1,-1,-1,:) to
cd(3,σx+1,σy+1,σz+1,:); cfields has just two elemnets for eb and cd and thus their
communication type identifiers are same as their field identifiers; the first and sencond
elements of ctypes for eb and cd are set as shown in Figure 13 and 14 respectively.
Now we can call oh3_init() and do it to have the sizes of field-arrays through ftypes
by which we allocate the arrays eb and cd.

nbor(1,1,1) = -1
sdoms(1,1,1) = 0;  sdoms(2,1,1) = -1
bcond(:,:) = reshape((/1,1, 1,1, 1,1/), (/2,OH_DIMENSION/))


<!-- Page 105 -->

ftypes(:,FEB) = (/6, 0,0, -1,1,  0,0/)                      ! for eb()
ftypes(:,FCD) = (/3, 0,0,  0,0, -1,2/)                      ! for cd()
ftypes(1,FCD+1) = -1                                        ! terminator
cfields(:) = (/FEB,FCD,0/)
ctypes(:,:,1,FEB) = reshape((/ 0,0,2,  -1,-1,1/), (/3,2/))  ! for eb()
ctypes(:,:,1,FCD) = reshape((/-1,2,3,  -1,-4,3/), (/3,2/))  ! for cd()

call oh_init(sdid(:), nspec, MAXFRAC, nphgram(:,:,:), totalp(:,:), &
pbuf(:), pbase(:), maxlocalp, mycomm, nbor(:,:,:), &
pcoord(:), sdoms(:,:,:), scoord(:,:), 1, bcond(:,:), &
bounds(:,:,:), ftypes(:,:), cfields(:), ctypes(:,:,:,:), &
fsizes(:,:,:), 0, 0, 0)

allocate(eb(6, fsizes(1,1,FEB):fsizes(2,1,FEB), &
fsizes(1,2,FEB):fsizes(2,2,FEB), &
fsizes(1,3,FEB):fsizes(2,3,FEB), 2))
allocate(cd(3, fsizes(1,1,FCD):fsizes(2,1,FCD), &
fsizes(1,2,FCD):fsizes(2,2,FCD), &
fsizes(1,3,FCD):fsizes(2,3,FCD), 2))


We still have a few initializations to have initial setting of eb for primary subdomain,
whose size and location in the space domain is given in sdoms(:,:,sdid(1)), by the
out-of-scope subroutine initialize_eb(), and that of primary particles in pbuf and the
count for each of nspec species and each of subdomain in nphgram(:,:,1) by the out-of-
scope subroutine initialize_particles()23. Then we call oh3_transbound() to examine
whether the initial particle positioning is balanced and, if not, broadcast eb to the helpers
of the local node by oh3_bcast_field()24. Finally, the boundary values of initial setting
of eb are exchanged between adjacent nodes by oh3_exchange_borders().

call initialize_eb(eb(:,:,:,:,1), sdoms(:,:,sdid(1)))
call initialize_particles(pbuf(:), nspec, nphgram(:,:,1))

currmode = oh_transbound(0, 0)
if (currmode.lt.0) then
call oh_bcast_field(eb(1,0,0,0,1), eb(1,0,0,0,2), FEB)
currmode = 1
end if
call oh_exchange_borders(eb(1,0,0,0,1), eb(1,0,0,0,2), FEB, currmode)


Now we start the main loop of simulation.  First, we call particle_push() giving it
primary particles and the electromagnetic field-array eb of primary subdomain. Then, if the
local node has secondary particles and subdomain, i.e., sdid(2) for its secondary subdomain
identifier is not negative, we call the subroutine again giving it secondary particles and the
field-array of secondary subdomain. Then we call oh3_transbound() to transfer particles
among nodes and, if it (re)built the helpand-helper configuration, oh3_bcast_field() to
broadcast eb to helpers.


23It might need other parameters to initialize pbuf, e.g., the number of initial particles of each species as
a whole, but such parameters are also out-of-scope.
24Broadcasting from the local subdomain coordinates (−1, −1, −1) to (σx, σy, σz) is a little bit larger
than what we really need because oh3_exchange_borders() just follows, but it is safe and the additional
communication cost is negligible.


<!-- Page 106 -->

do t=1, nstep
call particle_push(pbuf(pbase(1):), nspec, totalp(:,1), &
eb(:,:,:,:,1), sdoms(:,:,sdid(1)), sdid(1), 0, &
nphgram(:,:,1))
if (sdid(2).ge.0) &
call particle_push(pbuf(pbase(2):), nspec, totalp(:,2), &
eb(:,:,:,:,2), sdoms(:,:,sdid(2)), sdid(2), 1, &
nphgram(:,:,2))
currmode = oh_transbound(currmode, 0)
if (currmode.lt.0) then
call oh_bcast_field(eb(1,0,0,0,1), eb(1,0,0,0,2), FEB)
currmode = 1
end if


Next we call current_scatter() once or twice giving it primary and secondary parti-
cles and the field-array cd of subdomains, to have current density vectors in the primary
subdomain, or a partial results of them in primary and secondary subdomains if we are
in secondary mode.  In the latter case, we call oh3_allreduce_field() to have almost
complete sums of the vectors in both primary and secondary subdomains. Then, to obtain
the contribution of the particles near by the subdomain boundaries and residing (or having
resided) in adjacent subdomains, we call oh3_exchange_borders() to have the boundary
values of cd, and add_boundary_current() to add them to those calculated by the local
node.  If the local node has the secondary subdomain, add_boundary_current() is called
twice, one for the primary subdomain and the other for the secondary.

call current_scatter(pbuf(pbase(1):), nspec, totalp(:,1), &
cd(:,:,:,:,1), sdoms(:,:,sdid(1)), &
ctypes(:,:,1,FCD))
if (sdid(2).ge.0) &
call current_scatter(pbuf(pbase(2):), nspec, totalp(:,2), &
cd(:,:,:,:,2), sdoms(:,:,sdid(2)), &
ctypes(:,:,1,FCD))
if (currmode.ne.0) &
call oh_allreduce_field(cd(1,0,0,0,1), cd(1,0,0,0,2), FCD)
call oh_exchange_borders(cd(1,0,0,0,1), cd(1,0,0,0,2), FCD, currmode)
call add_boundary_current(cd(:,:,:,:,1), sdoms(:,:,sdid(1)), &
ctypes(:,:,1,FCD))
if (sdid(2).ge.0) &
call add_boundary_current(cd(:,:,:,:,2), sdoms(:,:,sdid(2)), &
ctypes(:,:,1,FCD))


Next, we update field vectors E and B in the primary subdomain by calling field_
solve_e() and field_solve_b() respectively, giving them the field-arrays of the primary
subdomain. Then, if the local node has the secondary subdomain, we call these two sub-
routines again giving them field-arrays of the secondary subdomain. Finally, the boudary
values of eb are exchanged between adjacent subdomains by oh3_exchange_borders() to
have what we need in the next simulation step.

call field_solve_e(eb(:,:,:,:,1), cd(:,:,:,:,1), sdoms(:,:,sdid(1)))
call field_solve_b(eb(:,:,:,:,1), sdoms(:,:,sdid(1)))
if (sdid(2).ge.0) then
call field_solve_e(eb(:,:,:,:,2), cd(:,:,:,:,2), sdoms(:,:,sdid(2)))


<!-- Page 107 -->

call field_solve_b(eb(:,:,:,:,2), sdoms(:,:,sdid(2)))
end if
call oh_exchange_borders(eb(1,0,0,0,1), eb(1,0,0,0,2), FEB, currmode)
end do
end subroutine



Subroutine particle_push()
The second subroutine particle push() is given eight arguments to specify primary or
secondary particles, primary or secondary subdomain and its field-array; pbuf for particle
buffer; nspec for the number of species; totalp for the number of particles in each species;
eb for the electromagnetic field-array; sdom for the size and the location of the subdomain;
n for the subdomain identifier; ps for primary or secondary mode; and nphgram for the
particle population histogram.

subroutine particle_push(pbuf, nspec, totalp, eb, sdom, n, ps, nphgram)
implicit none
type(oh_particle) :: pbuf(:)
integer           :: nspec
integer           :: totalp(:)
real*8            :: eb(:,:,:,:)
integer           :: sdom(:,:)
integer           :: n
integer           :: ps
integer           :: nphgram(:,:)

integer           :: xl, yl, zl, xu, yu, zu
integer           :: s, p, q, m
real*8            :: acc(OH_DIMENSION)


Before we enter the double loop for species and particles in each of them, we get lower
and upper subdomain boundaries from sdom to set them into xl, xu and so on, for the sake
of conciseness (and efficiency if your compiler is not smart enough).

xl=sdom(1,1);  yl=sdom(1,2);  zl=sdom(1,3)
xu=sdom(2,1);  yu=sdom(2,2);  zu=sdom(2,3)


Now we start the double loop letting nphgram(n+1,s) have totalp(s) as its initial value
at the beginning of the iteration for each species s, to mean that we will have totalp(s)
particles in the subdomain n if all the particles of the species s stay in the subdomain. Then
we call lorentz() to have the acceleration vector of each particle in the array acc(3), whose
elements are added to the velocity vector components of the particle. After this acceleration
(or deceleration), the particle is moved by adding the velocity vector to the position vector.

p = 0
do s=1, nspec
nphgram(n+1,s) = totalp(s)
do q=1, totalp(s)
p = p + 1
call lorentz(eb, pbuf(p)%x-xl, pbuf(p)%y-yl, pbuf(p)%z-zl, s, acc)
pbuf(p)%vx = pbuf(p)%vx + acc(1)
pbuf(p)%vy = pbuf(p)%vy + acc(2)


<!-- Page 108 -->

pbuf(p)%vz = pbuf(p)%vz + acc(3)
pbuf(p)%x = pbuf(p)%x + pbuf(p)%vx
pbuf(p)%y = pbuf(p)%y + pbuf(p)%vy
pbuf(p)%z = pbuf(p)%z + pbuf(p)%vz

Now we finish the job for a particle if it is still staying in the subdomain. Otherwise, we
call oh3_map_particle_to_neighbor() to obtain the identifier m of the subdomain in which
the particle now resides. Then nphgram(n+1,s) is decreased by one to indicate that the
particle has gone, while nphgram(m+1,s) is increased by one to represent its immigration.
We also update nid element of the particle to show it now resides in the subdomain m.

if (pbuf(p)%x.lt.xl .or. pbuf(p)%x.ge.xu .or. &
pbuf(p)%y.lt.yl .or. pbuf(p)%y.ge.yu .or. &
pbuf(p)%z.lt.zl .or. pbuf(p)%z.ge.zu) then
m = oh_map_particle_to_neighbor(pbuf(p)%x, pbuf(p)%y, pbuf(p)%z, ps)
nphgram(n+1,s) = nphgram(n+1,s) - 1
nphgram(m+1,s) = nphgram(m+1,s) + 1
pbuf(p)%nid = m
end if
end do
end do
end subroutine



Subroutine current_scatter()
The third subroutine current scatter() is given six arguments to specify primary or
secondary particles, primary or secondary subdomain and its field-array; pbuf for particle
buffer; nspec for the number of species; totalp for the number of particles in each species;
cd for the field-array of current density vectors; sdom for the size and the location of the
subdomain; and ctype to know the range in cd which the particles will contribute to.

subroutine current_scatter(pbuf, nspec, totalp, cd, sdom, ctype)
implicit none
type(oh_particle) :: pbuf(:)
integer           :: nspec
integer           :: totalp(:)
real*8            :: cd(:,:,:,:)
integer           :: sdom(:,:)
integer           :: ctype(3,2)

integer           :: xl, yl, zl, xu, yu, zu
integer           :: s, p, q
integer           :: i, j, k
real*8            :: x, y, z
real*8            :: c(3,2,2,2)


Before we enter the double loop for species and particles in each of them, we get lower
subdomain boundaries from sdom to set them into xl and so on, and upper bondaries to set
those in the local subdomain coordinates into xu and so on, for the sake of conciseness. Then
we zero-clear cd including the boundary planes we will send to adjacent nodes referring to
ctype.


<!-- Page 109 -->

xl = sdom(1,1);  yl = sdom(1,2);  zl = sdom(1,3)
xu = sdom(2,1)-xl;  yu = sdom(2,2)-yl;  zu = sdom(2,3)-zl
do k=ctype(1,1), zu+ctype(1,2)+ctype(1,3)-1
do j=ctype(1,1), yu+ctype(1,2)+ctype(1,3)-1
do i=ctype(1,1), xu+ctype(1,2)+ctype(1,3)-1
cd(JX, i, j, k) = 0.0d0
cd(JY, i, j, k) = 0.0d0
cd(JZ, i, j, k) = 0.0d0
end do;  end do;  end do


Now we start the double loop. In each iteration for a particle, we call scatter() to
have its contribution to the current density vectors of the grid points surrounding it in the
array c(3,2,2,2), whose elements are added to the corresponding elements of cd.

p = 0
do s=1, nspec
do q=1, totalp(s)
p = p + 1
call scatter(pbuf(p), s, c)
x = pbuf(p)%x - xl;  y = pbuf(p)%y - yl;  z = pbuf(p)%z - zl
do k=0,1;  do j=0,1;  do i=0,1
cd(JX, x+i, y+j, z+k) = cd(JX, x+i, y+j, z+k) + c(JX, i, j, k)
cd(JY, x+i, y+j, z+k) = cd(JY, x+i, y+j, z+k) + c(JY, i, j, k)
cd(JZ, x+i, y+j, z+k) = cd(JZ, x+i, y+j, z+k) + c(JZ, i, j, k)
end do;  end do;  end do;
end do
end do
end subroutine



Subroutine add_boundary_current()
The fourth subroutine add boundary current() is given three arguments to specify
the primary or secondary subdomain and its field-array; cd for the field-array of current
density vectors; sdom for the size and the location of the subdomain; and ctype to know
the boundary planes in cd.

subroutine add_boundary_current(cd, sdom, ctype)
implicit none
real*8            :: cd(:,:,:,:)
integer           :: sdom(2,OH_DIMENSION)
integer           :: ctype(3,2)

integer           :: xu, yu, zu
integer           :: sl, dl, nl, su, du, nu


First, we calculate the upper boundaries σx,y,z of the subdomain in its local coordinates
referring to sdom and set them into xu and so on.  Then, to calculate the base (lowest
corrdinate) of the boundary planes, slx,y,z and sux,y,z for the planes obtained from neighbors
and dlx,y,z and dux,y,z for those to add to, and the number of lower and upper boundary
planes nl and nu, we refer to ctype elements to have the followings.

slx,y,z = ctype(2,2)     nl = ctype(3,2)     dlx,y,z = slx,y,z + nl
sux,y,z = σx,y,z + ctype(2,1)    nu = ctype(3,1)    dux,y,z = sux,y,z −nu


<!-- Page 110 -->

That is, we suppose the planes to add to are at just inside of the planes obtained from
neighbors.

xu = sdom(2,1) - sdom(1,1)
yu = sdom(2,2) - sdom(1,2)
zu = sdom(2,3) - sdom(1,3)
sl = ctype(2,2);  nl = ctype(3,2);  dl = sl + nl
su = ctype(2,1);  nu = ctype(3,1);  du = su - nu


Then we call add_boundary_curr() six times for lower and upper boundary planes
perpendicular to z, y and x axes in this order to do the followings conceptually.

[slx, sux+nu)×[sly, suy+nu)×[dlz, dlz+nl) ←
[slx, sux+nu)×[sly, suy+nu)×[dlz, dlz+nl) + [slx, sux+nu)×[sly, suy+nu)×[slz, slz+nl)
[slx, sux+nu)×[sly, suy+nu)×[duz, duz+nu) ←
[slx, sux+nu)×[sly, suy+nu)×[duz, duz+nu) + [slx, sux+nu)×[sly, suy+nu)×[suz, suz+nu)
[slx, sux+nu)×[dly, dly+nl)×[dlz, duz+nu) ←
[slx, sux+nu)×[dly, dly+nl)×[dlz, duz+nu) + [slx, sux+nu)×[sly, sly+nl)×[dlz, duz+nu)
[slx, sux+nu)×[duy, duy+nu)×[dlz, duz+nu) ←
[slx, sux+nu)×[duy, duy+nu)×[dlz, duz+nu) + [slx, sux+nu)×[suy, suy+nu)×[dlz, duz+nu)
[dlx, dlx+nl)×[dly, duy+nu)×[dlz, duz+nu) ←
[dlx, dlx+nl)×[dly, duy+nu)×[dlz, duz+nu) + [slx, slx+nl)×[dly, duy+nu)×[dlz, duz+nu)
[dux, dux+nu)×[dly, duy+nu)×[dlz, duz+nu) ←
[dux, dux+nu)×[dly, duy+nu)×[dlz, duz+nu) + [sux, slx+nu)×[dly, duy+nu)×[dlz, duz+nu)

The operations above for a two-dimensional subdomain are illustarted in Figure 15.

call add_boundary_curr(sl, sl, xu+(su+nu-sl), &
sl, sl, yu+(su+nu-sl), &
sl, dl, nl, cd)
call add_boundary_curr(sl, sl, xu+(su+nu-sl), &
sl, sl, yu+(su+nu-sl), &
zu+su, zu+du, nu, cd)
call add_boundary_curr(sl, sl, xu+(su+nu-sl), &
sl, dl, nl, &
dl, dl, zu+(du-dl), cd)
call add_boundary_curr(sl, sl, xu+(su+nu-sl), &
yu+su, yu+du, nu, &
dl, dl, zu+(du-dl), cd)
call add_boundary_curr(sl, dl, nl, &
dl, dl, yu+(du-dl), &
dl, dl, zu+(du-dl), cd)
call add_boundary_curr(xu+su, xu+du, nu, &
dl, dl, yu+(du-dl), &
dl, dl, zu+(du-dl), cd)
end subroutine


<!-- Page 111 -->

+     +
+





+
calculatedcalculated byby locallocal nodenode         calculatedcalculated byby locallocal nodenode

receivedreceived fromfrom x-neighborsx-neighbors       receivedreceived fromfrom y-neighborsy-neighbors


Figure 15: Adding boundary planes of current density vectors.


Subroutine add_boundary_curr()
The fifth subroutine add boundary curr() does the followings conceptually for each
current density vector component in cd for the boundary plane addition in add_boundary_
current().

[xd, xd+nx)×[yd, yd+ny)×[zd, zd+nz) ←
[xd, xd+nx)×[yd, yd+ny)×[zd, zd+nz) + [xs, xs+nx)×[ys, ys+ny)×[zs, zs+nz)


subroutine add_boundary_curr(xs, xd, nx, ys, yd, ny, zs, zd, nz, cd)
implicit none
integer           :: xs, xd, nx, ys, yd, ny, zs, zd, nz
integer           :: i, j, k
real*8            :: cd(:,:,:,:)

do k=0, nz-1;  do j=0, ny-1;  do i=0, nx-1
cd(JX, xd+i, yd+j, zd+k) = &
cd(JX, xd+i, yd+j, zd+k) + cd(JX, xs+i, ys+j, zs+k)
cd(JY, xd+i, yd+j, zd+k) = &
cd(JY, xd+i, yd+j, zd+k) + cd(JY, xs+i, ys+j, zs+k)
cd(JZ, xd+i, yd+j, zd+k) = &
cd(JZ, xd+i, yd+j, zd+k) + cd(JZ, xs+i, ys+j, zs+k)
end do;  end do;  end do
end subroutine



Subroutine field_solve_e()
The sixth subroutine field solve e() is given three arguments to specify the primary
or secondary subdomain and its field-arrays; eb for the electromagnetic field-array; cd for
the field-array of current density vectors; and sdom for the size and the location of the
subdomain.

subroutine field_solve_e(eb, cd, sdom)
implicit none
real*8            :: eb(:,:,:,:)
real*8            :: cd(:,:,:,:)
integer           :: sdom(2,OH_DIMENSION)


<!-- Page 112 -->

integer           :: xu, yu, zu, x, y, z
real*8            :: rot(OH_DIMENSION)


First, we calculate the upper boundaries σx,y,z of the subdomain in its local coordinates
referring to sdom and set them into xu and so on. Then, in the loop for [0, σx]×[0, σy]×[0, σz],
we update each electric field vector following the Maxwell’s (or Amp`er’s circuital) law using
∇× B calculated by the out-of-scope subroutine rotate_b() and set into rot(3), and the
current density vectors cd.  Note that the constants EPSILON for ε0 and MU for µ0 are
assumed to have been defined somewhere in the simulation code.

xu = sdom(2,1) - sdom(1,1)
yu = sdom(2,2) - sdom(1,2)
zu = sdom(2,3) - sdom(1,3)
do z=0, zu;  do y=0, yu;  do x=0, xu
call rotate_b(eb(:,:,:,:), x, y, z, rot)
eb(EX, x, y, z) = eb(EX, x, y, z) + &
(1/EPSILON)*((1/MU)*rot(1) + cd(JX, x, y, z))
eb(EY, x, y, z) = eb(EY, x, y, z) + &
(1/EPSILON)*((1/MU)*rot(2) + cd(JY, x, y, z))
eb(EZ, x, y, z) = eb(EZ, x, y, z) + &
(1/EPSILON)*((1/MU)*rot(3) + cd(JZ, x, y, z))
end do;  end do;  end do
end subroutine



Subroutine field_solve_b()
The seventh and last subroutine field solve b() is given two arguments to specify the
primary or secondary subdomain and its field-array; eb for the electromagnetic field-array;
and sdom for the size and the location of the subdomain.

subroutine field_solve_b(eb, sdom)
implicit none
real*8            :: eb(:,:,:,:)
integer           :: sdom(2,OH_DIMENSION)

integer           :: xu, yu, zu, x, y, z
real*8            :: rot(OH_DIMENSION)


First, we calculate the upper boundaries σx,y,z of the subdomain in its local coordinates
referring to sdom and set them into xu and so on.  Then, in the loop for [0, σx−1] ×
[0, σy−1] × [0, σz−1], we update each magnetic field vector following the Maxwell’s (or
Faraday’s induction) law using ∇× E calculated by the out-of-scope subroutine rotate_
e() and set into rot(3).

xu = sdom(2,1) - sdom(1,1)
yu = sdom(2,2) - sdom(1,2)
zu = sdom(2,3) - sdom(1,3)
do z=0, zu-1;  do y=0, yu-1;  do x=0, xu-1
call rotate_e(eb(:,:,:,:), x, y, z, rot)
eb(BX, x, y, z) = eb(BX, x, y, z) + rot(1)
eb(BY, x, y, z) = eb(BY, x, y, z) + rot(2)
eb(BZ, x, y, z) = eb(BZ, x, y, z) + rot(3)


<!-- Page 113 -->

end do;  end do;  end do
end subroutine
end module


#### 3.13.2 C Sample Code

The C sample code is given in the file sample.c. It starts with the following lines to #include
the header file ohhelp c.h for level-3 function aliasing and prototypes of level-3 and lower
level libray functions. It also #include’s the standard header file stdlib.h for malloc().

#include <stdlib.h>
#define OH_LIB_LEVEL 3
#include "ohhelp_c.h"



Declaration
At first, we #define a few constants, MAXFRAC = 20 for maxfrac argument of oh3_
init(), field-array identifiers for electromagnetic field-array eb[] (FEB = 0) and current
density cd[] (FCD = 1).

#define MAXFRAC 20
#define FEB     0
#define FCD     1


Then the variables to pass oh3_init() are declared with the same names as defined
in §3.6.1 and a part of them are initialized as follows; pointers pbuf, nbor, sdoms and
bounds have NULL to make oh3_init() allocate them and initialize the last three in the
default manner; bcond indicates fully periodic boundary conditions by having 0s in all of its
elements; the first element of ftypes for eb shows that the range for its broadcast is from
the local subdomain coordinates (−1, −1, −1) to (σx, σy, σz) while the second element for
cd gives that for the reduction being from (−1, −1, −1) to (σx+1, σy+1, σz+1); cfields
has just two elemnets for eb and cd and thus their communication type identifiers are same
as their field identifiers; the first and sencond elements of ctypes for eb and cd are set as
shown in Figure 13 and 14 respectively. We also declare two pointer arrays to field-arrays,
eb[2] for electromagnetic field and cd[2] for current density, toghether with thier struct
namely ebfield and current.

int sdid[2];
int **nphgram[2];
int *totalp[2];
struct S_particle *pbuf=NULL;
int pbase[3];
int *nbor=NULL;
int (*sdoms)[OH_DIMENSION][2]=NULL;
int bcond[OH_DIMENSION][2]={{0,0},{0,0},{0,0}}; /* fully periodic */
int *bounds=NULL;
int ftypes[3][7]={{6, 0,0, -1,1,  0,0},         /* for eb[] */
{3, 0,0,  0,0, -1,2},         /* for cd[] */
{-1,0,0,  0,0,  0,0},         /* terminator */
};
int cfields[3]={0,1,-1};                        /* for eb[] and cd[] */


<!-- Page 114 -->

int ctypes[2][1][2][3]={
{{{ 0,0,2}, {-1,-1,1}}},                      /* for eb[] */
{{{-1,2,3}, {-1,-4,3}}},                      /* for cd[] */
};
int fsizes[2][OH_DIMENSION][2];
struct ebfield {
double ex, ey, ez, bx, by, bz;
} *eb[2];
struct current {
double jx, jy, jz;
} *cd[2];


Another declarative work is to give the prototypes of functions defined in this source
file and of out-of-scope ones.

/* prototypes of funcions defined in sample.c */
void  pic(int nspec, int pcoord[OH_DIMENSION], int scoord[OH_DIMENSION][2],
long long int npmax, int nstep);
void  particle_push(struct S_particle *pbuf, int nspec, int *totalp,
struct ebfield *eb, int sdom[OH_DIMENSION][2],
int fsize[OH_DIMENSION][2], int n, int ps, int **nphgram);
void  current_scatter(struct S_particle *pbuf, int nspec, int *totalp,
struct current *cd, int sdom[OH_DIMENSION][2],
int ctype[2][3], int fsize[OH_DIMENSION][2]);
void  add_boundary_current(struct current *cd, int sdom[OH_DIMENSION][2],
int ctype[2][3], int fsize[OH_DIMENSION][2]);
void  add_boundary_curr(int xs, int xd, int nx, int ys, int yd, int ny,
int zs, int zd, int nz, struct current *cd,
int fsize[3][2]);
void  field_solve_e(struct ebfield *eb, struct current *cd,
int sdom[OH_DIMENSION][2], int fsizee[OH_DIMENSION][2],
int fsizec[OH_DIMENSION][2]);
void  field_solve_b(struct ebfield *eb, int sdom[OH_DIMENSION][2],
int fsize[OH_DIMENSION][2]);

/* prototypes of funcions not defined in sample.c */
void  initialize_eb(struct ebfield *eb, int sdom[OH_DIMENSION][2],
int fsize[OH_DIMENSION][2]);
void  initialize_particles(struct S_particle* pbuf, int nspec, int **nphgram);
void  lorentz(struct ebfield *eb, double x, double y, double z, int s,
int fsize[OH_DIMENSION][2], double acc[OH_DIMENSION]);
void  scatter(struct S_particle p, int s, struct current c[2][2][2]);
void  rotate_b(struct ebfield *eb, double x, double y, double z,
int fsize[OH_DIMENSION][2], double rot[OH_DIMENSION]);
void  rotate_e(struct ebfield *eb, double x, double y, double z,
int fsize[OH_DIMENSION][2], double rot[OH_DIMENSION]);


The last declarative work is to define two functional macros field array size(FS) and
malloc field array(S,FS). The former is to calculate the number of elements in an array
of conceptually three dimensional but one-dimensional in reality from FS being a subarray
of fsizes[][OH_DIMENSION][2] reported from oh3_init(). The latter is to malloc()
a field-array whose element is a struct named S and whose size is given by FS being a


<!-- Page 115 -->

subarray of fsizes. These macros are for a concise implementation of what we described
in §3.6.1.

#define field_array_size(FS) \
((FS[0][1]-FS[0][0])*(FS[1][1]-FS[1][0])*(FS[2][1]-FS[2][0]))
#define malloc_field_array(S,FS) \
((struct S*)malloc(sizeof(struct S)*field_array_size(FS)*2)- \
FS[0][0]+(FS[0][1]-FS[0][0])*(FS[1][0]+(FS[1][1]-FS[1][0])*FS[2][0]))



Function pic()
The first function pic() is the core of the simulator and is called with a few simulation
parameters to be given to the arguments of oh3_init(), which are nspec, pcoord[3] and
scoord[3][2]. The function also has arguments npmax for the absolute maximum number
of the particle in the whloe simulation and nstep to determine the number of simulation
steps.

void pic(int nspec, int pcoord[OH_DIMENSION], int scoord[OH_DIMENSION][2],
long long int npmax, int nstep) {
int n, i, j, t;
int currmode;


The first job is the allocation of the bodies of totalp and nphgram, which we could
depute oh3_init() to do but in this example we dare to do for the sake of clearity. The
allocation for the former is fairly simple becase we just need an one-dimensional array of
S ×2 and make totalp[0] and totalp[1] point its element [0] and [S]. The allocatoin for
the later is a little bit more complicated as exemplified in §3.2.4. Its size N for the number
of nodes N = Πx × Πy × Πz is calculated from pcoord.

totalp[0] = (int*)malloc(sizeof(int)*nspec*2);
totalp[1] = totalp[0] + nspec;
n = pcoord[0] * pcoord[1] * pcoord[2];
nphgram[0] = (int**)malloc(sizeof(int*)*nspec*2);
nphgram[1] = nphgram[0] + nspec;
nphgram[0][0] = (int*)malloc(sizeof(int)*n*nspec*2);
nphgram[1][0] = nphgram[0][0] + n*nspec;
for (i=0; i<2; i++)  for (j=1; j<nspec; j++)
nphgram[i][j] = nphgram[i][j-1] + n;


Now we can call oh3_init() and do it giving the size of pbuf calculated by oh2_
max_local_particles() to its argument maxlocalp, and NULL to mycomm because it is
unnecessary. Then, with the sizes of field-arrays given through ftypes, we allocate the
arrays so that they are pointed by eb and cd using the macros malloc_field_array() and
field_array_size().

oh_init((int**)(&sdid), nspec, MAXFRAC, nphgram[0], totalp, &pbuf,
(int**)(&pbase), oh_max_local_particles(npmax, MAXFRAC, 0), NULL,
&nbor, pcoord, (int**)(&sdoms), &scoord[0][0], 1, &bcond[0][0],
&bounds, ftypes[0], cfields, ctypes[0][0][0], (int**)(&fsizes),
0, 0, 0);

eb[0] = malloc_field_array(ebfield, fsizes[FEB]);


<!-- Page 116 -->

eb[1] = eb[0] + field_array_size(fsizes[FEB]);
cd[0] = malloc_field_array(current, fsizes[FCD]);
cd[1] = cd[0] + field_array_size(fsizes[FCD]);


We still have a few initializations to have initial setting of eb for primary subdomain,
whose size and location in the space domain is given in sdoms[sdid[1]][][], by the
out-of-scope function initialize_eb(), and that of primary particles in pbuf and the
count for each of nspec species and each of subdomain in nphgram[0][][] by the out-
of-scope function initialize_particles()25. Note that initialize_eb() is also given
fsizes[FEB][][] as its argument to calculate one-dimensional indices of eb. Then we call
oh3_transbound() to examine whether the initial particle positioning is balanced and, if
not, broadcast eb to the helpers of the local node by oh3_bcast_field()26. Finally, the
boundary values of initial setting of eb are exchanged between adjacent nodes by oh3_
exchange_borders().

initialize_eb(eb[0], sdoms[sdid[0]], fsizes[FEB]);
initialize_particles(pbuf, nspec, nphgram[0]);

currmode = oh_transbound(0, 0);
if (currmode<0) {
oh_bcast_field(eb[0], eb[1], FEB);  currmode = 1;
}
oh_exchange_borders(eb[0], eb[1], FEB, currmode);


Now we start the main loop of simulation.  First, we call particle_push() giving it
primary particles and the electromagnetic field-array eb of primary subdomain.  Then,
if the local node has secondary particles and subdomain, i.e., sdid[1] for its secondary
subdomain identifier is not negative, we call the function again giving it secondary particles
and the field-array of secondary subdomain. Then we call oh3_transbound() to transfer
particles among nodes and,  if it (re)built the helpand-helper configuration, oh3_bcast_
field() to broadcast eb to helpers.

for (t=0; t<nstep; t++) {
particle_push(pbuf+pbase[0], nspec, totalp[0], eb[0], sdoms[sdid[0]],
fsizes[FEB], sdid[0], 0, nphgram[0]);
if (sdid[1]>=0)
particle_push(pbuf+pbase[1], nspec, totalp[1], eb[1], sdoms[sdid[1]],
fsizes[FEB], sdid[1], 1, nphgram[1]);
currmode = oh_transbound(0, 0);
if (currmode<0) {
oh_bcast_field(eb[0], eb[1], 0);  currmode = 1;
}


Next we call current_scatter() once or twice giving it primary and secondary parti-
cles and the field-array cd of subdomains, to have current density vectors in the primary
subdomain, or a partial results of them in primary and secondary subdomains if we are

25It might need other parameters to initialize pbuf, e.g., the number of initial particles of each species as
a whole, but such parameters are also out-of-scope.
26Broadcasting from the local subdomain coordinates (−1, −1, −1) to (σx, σy, σz) is a little bit larger
than what we really need because oh3_exchange_borders() just follows, but it is safe and the additional
communication cost is negligible.


<!-- Page 117 -->

in secondary mode.  In the latter case, we call oh3_allreduce_field() to have almost
complete sums of the vectors in both primary and secondary subdomains. Then, to obtain
the contribution of the particles near by the subdomain boundaries and residing (or having
resided) in adjacent subdomains, we call oh3_exchange_borders() to have the boundary
values of cd, and add_boundary_current() to add them to those calculated by the local
node.  If the local node has the secondary subdomain, add_boundary_current() is called
twice, one for the primary subdomain and the other for the secondary.

current_scatter(pbuf+pbase[0], nspec, totalp[0], cd[0], sdoms[sdid[0]],
ctypes[FCD][0], fsizes[FCD]);
if (sdid[1]>=0)
current_scatter(pbuf+pbase[1], nspec, totalp[1], cd[1], sdoms[sdid[1]],
ctypes[FCD][0], fsizes[FCD]);
if (currmode)  oh_allreduce_field(cd[0], cd[1], FCD);
oh_exchange_borders(cd[0], cd[1], FCD, currmode);
add_boundary_current(cd[0], sdoms[sdid[0]], ctypes[FCD][0], fsizes[FCD]);
if (sdid[1]>=0)
add_boundary_current(cd[1], sdoms[sdid[1]], ctypes[FCD][0], fsizes[FCD]);


Next, we update field vectors E and B in the primary subdomain by calling field_
solve_e() and field_solve_b() respectively, giving them the field-arrays of the primary
subdomain. Then, if the local node has the secondary subdomain, we call these two func-
tions again giving them field-arrays of the secondary subdomain.  Finally, the boudary
values of eb are exchanged between adjacent subdomains by oh3_exchange_borders() to
have what we need in the next simulation step.

field_solve_e(eb[0], cd[0], sdoms[sdid[0]], fsizes[FEB], fsizes[FCD]);
field_solve_b(eb[0], sdoms[sdid[0]], fsizes[FEB]);
if (sdid[1]>=0) {
field_solve_e(eb[1], cd[1], sdoms[sdid[1]], fsizes[FEB], fsizes[FCD]);
field_solve_b(eb[1], sdoms[sdid[1]], fsizes[FEB]);
}
oh_exchange_borders(eb[0], eb[1], FEB, currmode);
}
}


Function particle_push()
The second function particle push() is given nine arguments to specify primary or
secondary particles, primary or secondary subdomain and its field-array; pbuf for particle
buffer; nspec for the number of species; totalp for the number of particles in each species;
eb for the electromagnetic field-array; sdom for the size and the location of the subdomain;
fsize for the size of eb; n for the subdomain identifier; ps for primary or secondary mode;
and nphgram for the particle population histogram.
Then, in the local variable declaration, we get lower and upper subdomain boundaries
from sdom to set them into xl, xu and so on, for the sake of conciseness (and efficiency if
your compiler is not smart enough).

void particle_push(struct S_particle *pbuf, int nspec, int *totalp,
struct ebfield *eb, int sdom[OH_DIMENSION][2],
int fsize[OH_DIMENSION][2], int n, int ps, int **nphgram) {
int xl=sdom[0][0], yl=sdom[1][0], zl=sdom[2][0];
int xu=sdom[0][1], yu=sdom[1][1], zu=sdom[2][1];


<!-- Page 118 -->

int s, p, q, m;
double acc[OH_DIMENSION];


Now we start the double loop for species and particles in each of them.  We let
nphgram[s][n] have totalp[s] as its initial value at the beginning of the iteration for
each species s, to mean that we will have totalp[s] particles in the subdomain n if all
the particles of the species s stay in the subdomain. Then we call lorentz() to have the
acceleration vector of each particle in the array acc[3], whose elements are added to the
velocity vector components of the particle.  After this acceleration (or deceleration), the
particle is moved by adding the velocity vector to the position vector.

for (s=0,p=0; s<nspec; s++) {
nphgram[s][n] = totalp[s];
for (q=0; q<totalp[s]; p++,q++) {
lorentz(eb, pbuf[p].x-xl, pbuf[p].y-yl, pbuf[p].z-zl, s, fsize, acc);
pbuf[p].vx += acc[0];
pbuf[p].vy += acc[1];
pbuf[p].vx += acc[2];
pbuf[p].x += pbuf[p].vx;
pbuf[p].y += pbuf[p].vy;
pbuf[p].x += pbuf[p].vz;

Now we finish the job for a particle if it is still staying in the subdomain. Otherwise,
we call oh3_map_particle_to_neighbor() to obtain the identifier m of the subdomain in
which the particle now resides. Then nphgram[s][n] is decreased by one to indicate that
the particle has gone, while nphgram[s][m] is increased by one to represent its immigration.
We also update nid element of the particle to show it now resides in the subdomain m.

if (pbuf[p].x<xl || pbuf[p].x>=xu ||
pbuf[p].y<yl || pbuf[p].y>=yu ||
pbuf[p].z<zl || pbuf[p].z>=zu) {
m = oh_map_particle_to_neighbor(&pbuf[p].x, &pbuf[p].y, &pbuf[p].z,
ps);
nphgram[s][n]--;  nphgram[s][m]++;
pbuf[p].nid = m;
}
}
}
}


Function current_scatter()
The third function current scatter() is given seven arguments to specify primary or
secondary particles, primary or secondary subdomain and its field-array; pbuf for particle
buffer; nspec for the number of species; totalp for the number of particles in each species;
cd for the field-array of current density vectors; sdom for the size and the location of the
subdomain; ctype to know the range in cd which the particles will contribute to; and fsize
for the size of cd.
Then, in the local variable declaration, we get lower subdomain boundaries from sdom
to set them into xl and so on, and upper bondaries to set those in the local subdomain
coordinates into xu and so on, for the sake of conciseness. We also have local variables w
for width of the field array cd and wd for width times depth of it to calculate the index of
cd correspoinding to the local subdomain coordinates (x, y, z) by x + w · y + wd · z.


<!-- Page 119 -->

void current_scatter(struct S_particle *pbuf, int nspec, int *totalp,
struct current *cd, int sdom[OH_DIMENSION][2],
int ctype[2][3], int fsize[OH_DIMENSION][2]) {
int xl=sdom[0][0], yl=sdom[1][0], zl=sdom[2][0];
int xu=sdom[0][1]-xl, yu=sdom[1][1]-yl, zu=sdom[2][1]-zl;
int w=fsize[0][1]-fsize[0][0], wd=w*(fsize[1][1]-fsize[1][0]);
int s, p, q;
int i, j, k;
struct current c[2][2][2];


First we zero-clear cd including the boundary planes we will send to adjacent nodes
referring to ctype.

for (k=ctype[0][0]; k<zu+ctype[1][0]+ctype[1][2]; k++)
for (j=ctype[0][0]; j<yu+ctype[1][0]+ctype[1][2]; j++)
for (i=ctype[0][0]; i<xu+ctype[1][0]+ctype[1][2]; i++)
cd[i+w*j+wd*k].jx = cd[i+w*j+wd*k].jy = cd[i+w*j+wd*k].jz = 0.0;


Now we start the double loop. In each iteration for a particle, we call scatter() to
have its contribution to the current density vectors of the grid points surrounding it in the
array c[2][2][2], whose elements are added to the corresponding elements of cd.

for (s=0,p=0; s<nspec; s++) {
for (q=0; q<totalp[s]; p++,q++) {
int x=pbuf[p].x-xl, y=pbuf[p].y-yl, z=pbuf[p].z-zl;
scatter(pbuf[p], s, c);
for (k=0; k<2; k++)  for (j=0; j<2; j++)  for (i=0; i<2; i++) {
cd[(x+i)+w*(y+j)+wd*(z+k)].jx += c[k][j][i].jx;
cd[(x+i)+w*(y+j)+wd*(z+k)].jy += c[k][j][i].jy;
cd[(x+i)+w*(y+j)+wd*(z+k)].jz += c[k][j][i].jz;
}
}
}
}


Function add_boundary_current()
The fourth function add boundary current() is given four arguments to specify the
primary or secondary subdomain and its field-array; cd for the field-array of current density
vectors; sdom for the size and the location of the subdomain; ctype to know the boundary
planes in cd; and fsize for the size of cd.
In the local variable declaration, we calculate the upper boundaries σx,y,z of the sub-
domain in its local coordinates referring to sdom and set them into xu and so on. Then,
to calculate the base (lowest corrdinate) of the boundary planes, slx,y,z and sux,y,z for the
planes obtained from neighbors and dlx,y,z and dux,y,z for those to add to, and the number
of lower and upper boundary planes nl and nu, we refer to ctype elements to have the
followings.

slx,y,z = ctype[1][1]     nl = ctype[1][2]     dlx,y,z = slx,y,z + nl
sux,y,z = σx,y,z + ctype[0][1]    nu = ctype[0][2]    dux,y,z = sux,y,z −nu


<!-- Page 120 -->

That is, we suppose the planes to add to are at just inside of the planes obtained from
neighbors.

void add_boundary_current(struct current *cd, int sdom[OH_DIMENSION][2],
int ctype[2][3], int fsize[OH_DIMENSION][2]) {
int xu=sdom[0][1]-sdom[0][0], yu=sdom[1][1]-sdom[1][0],
zu=sdom[2][1]-sdom[2][0];
int sl=ctype[1][1], nl=ctype[1][2], dl=sl+nl;
int su=ctype[0][1], nu=ctype[0][2], du=su-nu;


Then we call add_boundary_curr() six times for lower and upper boundary planes
perpendicular to z, y and x axes in this order to do the followings conceptually.

[slx, sux+nu)×[sly, suy+nu)×[dlz, dlz+nl) ←
[slx, sux+nu)×[sly, suy+nu)×[dlz, dlz+nl) + [slx, sux+nu)×[sly, suy+nu)×[slz, slz+nl)
[slx, sux+nu)×[sly, suy+nu)×[duz, duz+nu) ←
[slx, sux+nu)×[sly, suy+nu)×[duz, duz+nu) + [slx, sux+nu)×[sly, suy+nu)×[suz, suz+nu)
[slx, sux+nu)×[dly, dly+nl)×[dlz, duz+nu) ←
[slx, sux+nu)×[dly, dly+nl)×[dlz, duz+nu) + [slx, sux+nu)×[sly, sly+nl)×[dlz, duz+nu)
[slx, sux+nu)×[duy, duy+nu)×[dlz, duz+nu) ←
[slx, sux+nu)×[duy, duy+nu)×[dlz, duz+nu) + [slx, sux+nu)×[suy, suy+nu)×[dlz, duz+nu)
[dlx, dlx+nl)×[dly, duy+nu)×[dlz, duz+nu) ←
[dlx, dlx+nl)×[dly, duy+nu)×[dlz, duz+nu) + [slx, slx+nl)×[dly, duy+nu)×[dlz, duz+nu)
[dux, dux+nu)×[dly, duy+nu)×[dlz, duz+nu) ←
[dux, dux+nu)×[dly, duy+nu)×[dlz, duz+nu) + [sux, slx+nu)×[dly, duy+nu)×[dlz, duz+nu)

The operations above for a two-dimensional subdomain are illustarted in Figure 15.

add_boundary_curr(sl, sl, xu+(su+nu-sl),
sl, sl, yu+(su+nu-sl),
sl, dl, nl, cd, fsize);
add_boundary_curr(sl, sl, xu+(su+nu-sl),
sl, sl, yu+(su+nu-sl),
zu+su, zu+du, nu, cd, fsize);
add_boundary_curr(sl, sl, xu+(su+nu-sl),
sl, dl, nl,
dl, dl, zu+(du-dl), cd, fsize);
add_boundary_curr(sl, sl, xu+(su+nu-sl),
yu+su, yu+du, nu,
dl, dl, zu+(du-dl), cd, fsize);
add_boundary_curr(sl, dl, nl,
dl, dl, yu+(du-dl),
dl, dl, zu+(du-dl), cd, fsize);
add_boundary_curr(xu+su, xu+du, nu,
dl, dl, yu+(du-dl),
dl, dl, zu+(du-dl), cd, fsize);
}


<!-- Page 121 -->

Function add_boundary_curr()
The fifth function add boundary curr() does the followings conceptually for each cur-
rent density vector component in cd for the boundary plane addition in add_boundary_
current().

[xd, xd+nx)×[yd, yd+ny)×[zd, zd+nz) ←
[xd, xd+nx)×[yd, yd+ny)×[zd, zd+nz) + [xs, xs+nx)×[ys, ys+ny)×[zs, zs+nz)



void  add_boundary_curr(int xs, int xd, int nx, int ys, int yd, int ny,
int zs, int zd, int nz, struct current *cd,
int fsize[3][2]) {
int w=fsize[0][1]-fsize[0][0], wd=w*(fsize[1][1]-fsize[1][0]);
int i, j, k;

for (k=0; k<nz; k++)  for (j=0; j<ny; j++)  for (i=0; i<nz; i++) {
cd[(xd+i)+w*(yd+j)+wd*(zd+k)].jx += cd[(xs+i)+w*(ys+j)+wd*(zs+k)].jx;
cd[(xd+i)+w*(yd+j)+wd*(zd+k)].jy += cd[(xs+i)+w*(ys+j)+wd*(zs+k)].jy;
cd[(xd+i)+w*(yd+j)+wd*(zd+k)].jz += cd[(xs+i)+w*(ys+j)+wd*(zs+k)].jz;
}
}


Function field_solve_e()
The sixth function field solve e() is given five arguments to specify the primary or
secondary subdomain and its field-arrays; eb for the electromagnetic field-array; cd for the
field-array of current density vectors; sdom for the size and the location of the subdomain;
and fsizee and fsizec for the sizes of eb and cd.
In the local variable declaration, we calculate the upper boundaries σx,y,z of the sub-
domain in its local coordinates referring to sdom and set them into xu and so on. We also
calculate the width and width times depth of eb and cd to set them into we, wde, wc and
wdc.

void field_solve_e(struct ebfield *eb, struct current *cd,
int sdom[OH_DIMENSION][2],
int fsizee[OH_DIMENSION][2], int fsizec[OH_DIMENSION][2]) {
int xu=sdom[0][1]-sdom[0][0], yu=sdom[1][1]-sdom[1][0],
zu=sdom[2][1]-sdom[2][0];
int we=fsizee[0][1]-fsizee[0][0], wde=we*(fsizee[1][1]-fsizee[1][0]);
int wc=fsizec[0][1]-fsizec[0][0], wdc=wc*(fsizec[1][1]-fsizec[1][0]);
int x, y, z;
double rot[OH_DIMENSION];


Then, in the loop for [0, σx]×[0, σy]×[0, σz], we update each electric field vector follow-
ing the Maxwell’s (or Amp`er’s circuital) law using ∇× B calculated by the out-of-scope
function rotate_b() and set into rot[3], and the current density vectors cd. Note that
the constants EPSILON for ε0 and MU for µ0 are assumed to have been defined somewhere
in the simulation code.

for (z=0; z<=zu; z++)  for (y=0; y<=yu; y++)  for (x=0; x<=xu; x++) {
rotate_b(eb, x, y, z, fsizee, rot);
eb[x+y*we+z*wde].ex += (1/EPSILON)*((1/MU)*rot[0] + cd[x+y*wc+z*wdc].jx);
