# 3.2 Applying OhHelp to PIC Simulators

Source: `doc/v1/original/ohhelp.pdf`, pages 18-24.

<!-- Page 18 -->

curren  t          ield
particle push                      scatter              so    lv e
secondary
er
ans
r         +  all-reduce          broadcast                                                                                balance&
cle      ri ary        i                                              load
ar

d v     m             = q ( E + v × B )                  ∂ρ+ ∇ ⋅ J = 0  ∇ ⋅ E = ρ            dt                                ∂t           ε
d x                                         B = 0           = v                               ∇ ⋅                                              B          dt                                          ∇ × E = − ∂                                                                     ∂t
∂E                                          ∇ × B = µ J + µε                                                                              ∂t

Figure 5: Typical 3D PIC simulator with OhHelp.


On the other hand, the level-4p and level-4s extensions implemented by ohhelp4p.c,
ohhelp4p.h, ohhelp4s.c, ohhelp4s.h are only for users who need position-aware particle man-
agement given by API functions having prefix ‘oh4p ’ or ‘oh4s ’ respectively. Therefore, if
your simulation is not position-aware, it is safe to exclude these files for the extension from
your make file. Otherwise, you are required not only to compile and link them but also to
activate the level-4p/4s extension by editing the header file oh config.h as discussed in §3.3.
Note that level-4p and level-4s extensions are mutually exclusive.
This naming rule shown above could be too rigid for you to use all functions provided by
your preferred layer and lower, because it will be tiresome to remember the layer number
which a function belongs to. Therefore, the library has special header files ohhelp f.h for
Fortran programmers and ohhelp c.h for those who love C, in order to give API functions
aliases which just have a common prefix ‘oh ’ as discussed in §3.12.

## 3.2 Applying OhHelp to PIC Simulators

Figure 5 shows a typical configuration of OhHelp’ed PIC simulators.  In the figure, it is
assumed that the baseline simulator to apply OhHelp is domain-decomposed and its main
loop consists of four phases, particle pushing, particle transferring, current scattering, and
field solving as follows.

particle pushing: Each node accelerates particles residing in the subdomain assigned to
the node by electric and the Lorentz force law referring to electromagnetic field data
E and B associated to the grid points in its subdomain. Then the node moves par-
ticles according to their updated velocities. Particle movements crossing subdomain
boundaries will be taken care of by the next phase.

particle transferring: Each node transfers paritcles, which has crossed its subdomain
boundaries, to the nodes responsible for adjacent subdomains.


<!-- Page 19 -->

current scattering: Each node calculates the contributions of the movement of its parti-
cles to the current density J at the grid points in its subdomain. Then the boundary
values of J are exchanged between adjacent subdomains.

field solving: Each node locally updates the values of E and B at the grid points in its
subdomain using, for example, leapfrog method to solve Maxwell’s equations. Then
the boundary values of E and B are exchanged between adjacent subdomains.

Applying OhHelp to the baseline simulator outlined above is fairly easy. In fact, required
modifications to the main simulation loop of the baseline simulator are just as follows.

duplication of data structures: Data structures for the subdomain and particles in it
should be duplicated so that a node has primary and secondary subdomains and
particles.

duplication of computation: The phases except for particle transferring of the main
loop should be duplicated to locally update particle and field data.

addition of collective communications: Current densities for a secondary subdomain
is calculated locally and thus should be summed up to have the complete data for
the subdomain. The boundary or whole values of electromagnetic field should be
broadcasted from each helpand to its helpers.

attachment of load balancer: To transfer particles among nodes, the library function
for load balancing should be called to have the transfer schedule or to do the transfer
itself.

In the following subsections, the modifications above are explained more detailedly.


#### 3.2.1 Duplication of Data Structures

Since each node may have primary and secondary subdomains and particles, you have
to duplicate data structure for electromagnetic field and current density to have those
for primary subdomain and for secondary subdomain. For example, suppose the baseline
simulator is coded in Fortran and the electromagnetic field for a subdomain is declared and
allocated as;

real*8,allocatable :: eb(:,:,:,:)
allocate(eb(6, ϕlx:ϕux−1, ϕly:ϕuy−1, ϕlz:ϕuz−1))

where the first dimension is for three components of electric field vector and those of mag-
netic field vector, and ϕlx and ϕux and their counterparts of y and z axes are lower and
upper boundaries of the subdomain including a few planes for the overlap of adjacent sub-
domains. An OhHelp’ed version of this four-dimensional array has one additional dimension
for primary and secondary ones and is declared and allocated as;

real*8,allocatable :: eb(:,:,:,:,:)
allocate(eb(6, ϕlx:ϕux−1, ϕly:ϕuy−1, ϕlz:ϕuz−1, 2))

to have primary field data in the subarray eb(:,:,:,:,1) while secondary data are stored
in the other subarray eb(:,:,:,:,2).
The other example for a C-coded simulator is given with the following declaration and
allocation.


<!-- Page 20 -->

struct ebfield {double ex,ey,ez,bx,by,bz} *eb;
eb = (struct ebfield*)
malloc(sizeof(struct ebfield)*(ϕux−ϕlx)(ϕuy−ϕly)(ϕuz−ϕlz));

A reasonable way to apply OhHelp to the example above is;

struct ebfield {double ex,ey,ez,bx,by,bz} *eb[2];
eb[0] = (struct ebfield*)
malloc(sizeof(srtuct ebfield)*(ϕux−ϕlx)(ϕuy−ϕly)(ϕuz−ϕlz)*2);
eb[1] = eb[0] + (ϕux−ϕlx)(ϕuy−ϕly)(ϕuz−ϕlz);

Note that, for both examples above, ϕux, ϕuy and ϕuz in OhHelp’ed version could have to be
larger than those in the original version because they must be for the largest subdomain
in the system rather than for the primary subdomain for the node if the subdomain size
is not uniform in the system. That is, the node should be able to be responsible for any
subdomain in the system. Also note that it is not necessary to represent the electromagnetic
field by one array, but you may have two arrays for electric and magnetic fields, or even
six arrays for each component of electric and magnetic field vectors. However, you have
to remember that splitting arrays should cost in the communication of them for boudnary
data exchange and broadcast and/or reduction in helpand-helper families.
On the other hand, adding a dimension to the array for particles to accommodate
primary and secondary ones is not a good idea, because the number of particles in each
category is not fixed. Therefore, the array must have Pmax elements4 defined in the inequal-
ity (1) in §2.1. Then, in the node n, the first part of the array should accommodate Qnn
primary particles while the second part, which directly follows the first part, should have
Qpn particles for p = parent(n). The values of Qnn and Qpn are given by the library function
for load balancing as discussed later.
The other remark on the array of particles is that if the array is partitioned into portions
for S species, the library should know it. For example, suppose the baseline simulator has
two particle species, one for (super)ions and the other for (super)electrons, and the particle
array is partitioned into two regions to store ions in the first region and electrons in the
second region. This partitioning is done, for example, to save memory space eliminating
species identifier and/or physical quantities of species such as the charge and mass of a
particle from the array element representing a particle, and/or to save operations for the
references to these quantities and for the calculations on them.  Since the layout of two
types of particles should be kept after the particle transfer, the library function for load
balancing have to aware that S = 2 to make transfer schedule and, if you desire to do, to
transfer particles. The function is also capable to report you the number of particles for
each species and each of primary/secondary categories.
Note that particle transferring for a simulation step should consist of S transfers for
each species, a large S, say 10 or more, may cause a too large communication overhead to
benefit from the array partitioning.  Therefore, if your simulation has a large number of
species, it is recommended to attach the species identifier and/or the physical quantities to
each particle and tell the library that S = 1.
Also note that if you apply level-2 library or above5, a particle should be represented
by a structured data which should include particle position coordinates, velocity vector
components, and other necessary information as discussed in §3.5.1. Otherwise, i.e., if you

4If the total number of particles in the system fluctuates due to, for example, particle injection and/or
removal, P for Pmax calculation in the inequality (1) should be the maximum number of total particles in
the simulation.
5Unless you choose partial application of level-3 disabling level-2 functions, which is discussed in §3.6.2.


<!-- Page 21 -->

use level-1 only and transfer particles among nodes by yourself, the set of particles accom-
modated in a node can be represeted in two or more arrays paying some communication
overhead.


#### 3.2.2 Duplication of Computation

Since a particle is accommodated by only one node, the node is of course fully responsible for
the particle. Therefore, each node should perform particle pushing and current scattering
for its primary and secondary particles. A reasonable way to implement this duplicated
computation for particles is to call functions corresponding to the operations twice.
For example, if your simulator has a Fortran subroutine named particle_push() with
three arguments for the particle array, its size and electromagnetic field, fundamental oper-
ation to duplicate particle pushing is easy as follows, providing the array pbuf has particles
each of which is represented by a structured data.

call particle_push(pbuf(1), Qnn, eb(:,:,:,:,1))
call particle_push(pbuf(Qnn+1), Qpn, eb(:,:,:,:,2))

However, this is not sufficient because two instances of particle_push() should have
different base coordinates by which the particle position in the coordinate system of whole
domain is mapped onto local coordinate system for a subdomain. That is, suppose the base
simulator calculates the particle velocity in particle_push() by;

call lorentz(eb, pbuf(i)%x-xl, pbuf(i)%y-yl, pbuf(i)%z-zl, acc(1:3))
pbuf(i)%vx = pbuf(i)%vx + acc(1)
pbuf(i)%vy = pbuf(i)%vy + acc(2)
pbuf(i)%vz = pbuf(i)%vz + acc(3)

where the structure elements x, y, z, vx, vy and vz are for x/y/z-components of the position
and the velocity of the i-th particle, lorentz() is the subroutine to calculate accelaration
vector acc(1:3) referring to electromagnetic field vectors on the grid points surrounding the
particle, and xl, yl and zl are the base coordinates of the subdomain, i.e., the coordinates
of the west-south-bottom corner of the subdomain.
The code above should be modified to refer to subdomain dependent base coordinates.
A reasonable way is to have a map of subdomain boundaries, say sdoms(2,3,N) whose
element sdoms(β,d,n) has lower (β = 1) or upper (β = 2) boundary of d-th dimension
of the subdomain n. With this array, the modified version of particle_push() has an
additional array argument, say sdom(2,3) for the subdomain in problem and is called as
follows where p = parent(n).

call particle_push(pbuf(1), Qnn, eb(:,:,:,:,1), sdoms(:,:,n))
call particle_push(pbuf(Qnn+1), Qpn, eb(:,:,:,:,2), sdoms(:,:,p))

Then at the beginning of the body of particle_push(), the following assignment is added
for the base coordinates where sdom is the fourth argument array passed to the subroutine.

xl = sdom(1,1)
yl = sdom(1,2)
zl = sdom(1,3)

Note that the upper boundaries sdom(2,:) will also be used in the function to detect the
particles crossing the subdomain boundaries. Remember that you are responsible for count-
ing number of particles in each subdomain, each species and each primary/secondary cate-
gory and for reporting it to the library. Also note that the array equivalent to sdoms(:,:,:)


<!-- Page 22 -->

can be given by the initialization function oh3_init() of level-3 library as discussed in
§3.6.1. A C-code version of the example above looks as follows.

particle_push(pbuf, Qnn, eb[0], sdoms[n]);
particle_push(pbuf+Qnn, Qpn, eb[1], sdoms[p]);
...
void particle_push(struct S_particle *pbuf, int nofparticles,
struct ebfield *eb, int sdom[2][3]) {
int xl=sdom[0][0], yl=sdom[0][1], zl=sdom[0][2];  double acc[3];
...
lorentz(eb, pbuf[i].x-xl, pbuf[i].y-yl, pbuf[i].z-zl, acc);

pbuf[i].vx += acc[0];
pbuf[i].vy += acc[1];
pbuf[i].vz += acc[2];
...
}

The modification of current scattering can be implemented similarly, but it needs col-
lective communications to sum local results of the scattering calculated by nodes in the
family. The sum is obtained by a simple reduce operation or by an all-reduce operation
to share the sum in family members, depending on the implementation of field solving as
discussed below. Also, the (all-)reduce communication is discussed in §3.2.3.
As for field solving, there are two reasonable ways to modify its baseline implementation.
The first candidate is to simply broadcast the solution of primary subdomain from each
helpand to its helpers.  That is, each node updates electromagnetic field vectors in its
primary subdomain, exchanges boundary data between adjacent nodes, and broadcasts the
whole field vectors in its primary subdomain and a few boundary planes to its helpers by
the method discussed in §3.2.3. In this broadcast-type implementation, since the current
density on each grid point in a subdomain is referred to only by the node responsible for the
subdomain as primary, summing current densities will be performed by a simple one-way
reduction followed by boundary exchange.
The other candidate is to duplicate the calculation of field solving. That is, each node
updates electromagnetic field vectors in both primary and secondary subdomains. A rea-
sonable way to obtain boudary values is to exchange boundary planes of adjacent primary
subdomains and then to broadcast planes to the helpers. In this duplicate-type implemen-
tation, since the current density on each grid point in a subdomain is referred to by all the
nodes responsible for the subdomain as primary or secondary, summing current densities
will be performed by an all-reduce communication followed by boundary exchange between
primary subdomains and boradcast boundary planes from the helpand to its helpers.
The choice from these two candidates should be determined by trading offthe compu-
tation cost of field solving and the communication cost of broadcasting. In practice, if your
simulator performs one leapfrog solving per one simulation step, the duplicate-type should
be chosen because a leapfrog update of a subdomain is faster than broadcast. On the other
hand, if your simulator adopts sub-stepping method to iterate leapfrog multiple times in a
simulation step with, for example, particle-fluid hybrid method, the broadcast-type can be
better.


#### 3.2.3 Addition of Collective Communications

As discussed in §3.2.2, you need to add at least the following collective communications.


<!-- Page 23 -->

- A simple one-way reduction or an all-reduce communication to sum the current density
among family members. In the latter case, the current density vectors of grid points
in boundary planes should be broadcasted from the helpand to its helpers.

- Broadcast of electromagnetic field vectors of the whole of or the boundaries of the
subdomain from the helpand to its helpers.

- Broadcast of electromagnetic field vectors when the helpand-helper tree is reconfigured
due to an unacceptable imbalance and each node has new helpand.

Fundamentally, the collective operations above are performed by MPI functions, MPI_
Reduce() or MPI_Allreduce() and MPI_Bcast() with argument comm being the commu-
nicator for the family which each node belongs to. Simply calling these functions, however,
should cause a severe performance problem because a node may belong to two families,
one as the helpand and the other as a helper. That is,  if we carelessly perform collec-
tive communications by doing them, for example, as the helpand and then as a helper, it
may cause unnecessary serialization because the root family must wait the completion of
the communications in the second generation families which must wait those in the third
ones and so on. Reversing the helpand/helper order cannot solve the problem because the
bottom families must wait the completion in the second-bottom ones and so on.
This problem is solved by a simple red-black technique which paints families of odd-
number generations by red and even ones by black and performs communications of red
families first and then of black families. Since families of same color are mutually exclusive,
the communications among them are performed in parallel.
The library provides various means for the red-black collective communications as fol-
lows.

- Level-1 library manages the family communicators and report you the communicators
for the families which the local node belongs to, together with their colors and the
ranks of the roots in the communicators. These information is sufficient to implement
your own version of collective communications besides those provided by the library
shown below.

- Level-1 library also provides you with functions for one-way reduction, all-reduce and
broadcast with given data buffers, data counts and data types. All of these functions
take care of the red-black ordering and special treatment for the tree root and leaves,
each of which belongs to only one family.

- Level-3 library provides functions for you to perform one-way reduction, all-reduce and
broadcast of the current vectors, electromagnetic field, and other arrays, for example
that having charge densities, if necessary. The usage of the functions is much simpler
than the level-1 counterparts, because you simply need to register each of field arrays,
which are arrays for current density vectors, electromagnetic field and so on having
elements associated to the grid points in a subdomain, and call these functions with
primary and secondary arrays and the identifier of the array.

- Level-3 library also provides a function to exchange boundaries of field-arrays option-
ally followed by broadcast of boundary data from the helpand to its helpers.


#### 3.2.4 Attachment of Load Balancer

Attaching OhHelp load balancer to your simulator is of course essential. What you need
to do is simply calling ohl transbound(), where l is level identifier in {1, 2, 3, 4p, 4s} with


<!-- Page 24 -->

a few explicit arguments. In addition, if you use a fundamental level library (i.e., 1 to 3),
you have to (implicitly) give it a histogram of particles accommodated by the local node.
That is, if your code is written in Fortran, you have to have an array, say nphgram(N,S,2)
whose element nphgram(m+1,s,c) has the number of particles residing in the subdomain
m ∈[0, N), categorized in the species s ∈[1, S] and accommodated by the local node as its
primary (c = 1) or secondary (c = 2) ones.
Similarly, C-coded simulator should have a conceptually three-dimensional array
nphgram[N × S × 2] whose element nphgram[m + N(s + Sc)] has the number of particles
residing in the subdomain m, categorized in the species s ∈[0, S −1] and accommodated
by the local node as its primary (c = 0) or secondary (c = 1) ones. If you like to access the
array elelment by nphgram[c][s][m] in your ANSI-C code, you have to do the followings.

int **nphgram[2];
nphgram[0] = (int**)malloc(sizeof(int*)*S*2);
nphgram[1] = nphgram[0] + S;
nphgram[0][0] = (int*)malloc(sizeof(int)*N*S*2);
nphgram[1][0] = nphgram[0][0] + N*S;
for (i=0; i<2; i++)  for (j=1; j<S; j++)
nphgram[i][j] = nphgram[i][j-1] + N;

Alternatively, you may choose C99 to simplify the code snip above to have the following.

int (*nphgram)[S][N]=(int(*)[S][N])malloc(sizeof(int)*N*S*2);

The main output you will obtain from the level-1 oh1_transbound() is a pair of (concep-
tually) three-dimensional arrays, say rcounts(N,S,2) and scounts(N,S,2) for Fortran
or rcounts[N × S × 2] and scounts[N × S × 2] for C, which tell you incoming and out-
going particle transfer schedules. That is, rcounts(m+1,s,c) and scounts(m+1,s,c)
notify you how many particles of species s should be received/sent from/to the node m as
receiver’s primary (c = 1) or secondary (c = 2) ones. Similary, rcounts[m + N(s + Sc)]
and scounts[m + N(s + Sc)] tell you the receiving/sending counts of primary (c = 0) or
secondary(c = 1) particles for the node m and species s.
On the other hand, level-2 function oh2_transbound() and its level-3 counterpart oh3_
transbound() perform particle transfer on behalf of you. To make the functions do the job
easily, you have to give an additional tip to show which subdomain each particle has moved
into. That is, each element of the array of particles, say pbuf, should be a structured
data having an element nid to have the identifier of the subdomain where the particle
is residing after particle pushing. Therefore, your subroutine/function for particle pushing
should modify this element for each particle which has just crossed the subdomain boundary.
Remember that level-3 library has functions to calculate the subdomain identifier from the
particle position.
An important notice is that the transfer schedule given by oh1_transbound() and that
used in oh2_transbound() and oh3_transbound() are unaware of particle positions. That
is, in secondary mode, a pair of closely located particles may be parted from each other to
be accommodated by two different nodes in the family for the subdomain where the pair
resides.  Therefore,  if your simulator takes care of proximal particle-particle interactions
using, for example, Monte Carlo Collision method, you have to use position-aware level-4p
or level-4s library and their function oh4p_transbound() or oh4s_transbound(). Unlike
its lower level counterparts, they do not need the per-subdomain particle histogram nphgram
because the histogram is maintained inside of the library. For other important functionality
of the level-4p/4s libraries such as per-grid histogram and sorted layout of particle buffer,
see §3.7 and/or §3.8.
