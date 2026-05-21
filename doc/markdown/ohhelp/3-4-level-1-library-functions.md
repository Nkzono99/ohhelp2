# 3.4 Level-1 Library Functions

Source: `doc/original/ohhelp.pdf`, pages 26-40.

<!-- Page 26 -->

remove this comment surrounding the line below.
#define OH_BIG_SPACE
*/
/* If you want to let level-4p/4s’s particle mapping functions run without
checking the consistency of their arguments, remove this comment
surrounding the line below.
#define OH_NO_CHECK
*/
#else
#ifndef OH_LIB_LEVEL
#define OH_LIB_LEVEL 3
#endif
#endif

## 3.4 Level-1 Library Functions

Level-1 library provides the following functions.

oh1 init() receives fundamental parameters and arrays by which the library interacts with
your simulator body, and initializes internal data structures.

oh1 neighbors() receives an array through which the neighbors of the local node is re-
ported each time the helpand-helper tree is reconfigured.

oh1 families() receives two arrays through which the configuration of all families is re-
ported each time the helpand-helper tree is reconfigured.

oh1 transbound() implements the core algorithm of OhHelp and reports the particle trans-
fer schedule.

oh1 accom mode() shows whether particle accommodation by nodes are normal or any-
where, i.e., particles accommodated by a node are in the subdomains assigned to the
node and in the neighbors of them, or not.

oh1 broadcast() performs broadcast communication in helpand-helper families.

oh1 all reduce() performs all-reduce communication in helpand-helper families.

oh1 reduce() performs simple one-way reduce communication in helpand-helper families.

oh1 init stats()
oh1 stats time()
oh1 show stats()
oh1 print stats()
See §3.10 for the functions for statistics above.

oh1 verbose() is explained in §3.11.

The function API for Fortran programs is given by the module named ohhelp1 in the
file oh mod1.F90, while API for C is embedded in ohhelp c.h.


<!-- Page 27 -->

#### 3.4.1 oh1_init()

The function (subroutine) oh1 init() receives a few fundamental parameters and arrays
through which oh1_transbound() interacts with your simulator body.  It also initializes
internal data structures used in level-1 library. Among its thirteen arguments, other library
functions directly refer to only the contents of the argument array nphgram as their implicit
inputs. Therefore, after the call of oh1 init(), modifying the bodies of other arguments
has no effect to library functions.


Fortran Interface

subroutine oh1_init(sdid, nspec, maxfrac, nphgram, totalp, rcounts, &
scounts, mycomm, nbor, pcoord, stats, repiter, verbose)
use oh_type
implicit none
integer,intent(out)   :: sdid(2)
integer,intent(in)    :: nspec
integer,intent(in)    :: maxfrac
integer,intent(inout) :: nphgram(:,:,:)
integer,intent(out)   :: totalp(:,:)
integer,intent(out)   :: rcounts(:,:,:)
integer,intent(out)   :: scounts(:,:,:)
type(oh_mycomm),intent(out) :: mycomm
integer,intent(inout) :: nbor(3,3,3)        ! for 3D codes.
integer,intent(in)    :: pcoord(OH_DIMENSION)
integer,intent(in)    :: stats
integer,intent(in)    :: repiter
integer,intent(in)    :: verbose
end subroutine

sdid(2) will have the identifiers of primary and secondary subdomain of the local node in
sdid(1) and sdid(2) respectively. Therefore, sdid(1) is always equivalent to the
MPI rank number of the calling process. On the other hand, sdid(2) intially has −1
to mean we are in primary mode initially, but will be set to a non-negative number in
[0, N−1] to identify the secondary subdomain by oh1_transbound() if it turns the
mode to secondary. Note that, even in secondary mode, sdid(2) may have −1 if the
local node is the root of the helpand-helper tree.

nspec should have the number of species S.

maxfrac should have the tolerance factor percentage of load imbalance α greater than 0
and less than 100.

nphgram(N,S,2) should be an array whose element nphgram(m+1,s,c) should have the
number of particles residing in the subdomain m ∈[0, N−1] categorized in the species
s ∈[1, S] and accommodated by the local node as its primary (c = 1) or secondary
(c = 2) ones. The contents of the array can be undefined at the call of oh1 init() but
must be completely defined at the call of oh1_transbound(). Upon returning from
oh1 init() and oh1_transbound(), the contents of the array will be zero-cleared, so
that you can (re)start counting.

totalp(S,2) should be an array whose element totalp(s,c) will have the number of
primary (c = 1) or secondary (c = 2) particles of species s to be accommodated by


<!-- Page 28 -->

the local node as the result of load balancing performed by oh1_transbound(). Note
that oh1 init() does not give any values to the array.

rcounts(N,S,2) should be an array whose element rcounts(m+1,s,c) will have the
number of particles of species s which the local node should receive from the node
m as primary (c = 1) or secondary (c = 2) ones of the local node, after each call of
oh1_transbound(). Remember that rcounts(n+1,s,c) for the local node n itself
can be non-zero when it has particles residing in its primary (secondary) subdomain
moving to its secondary (primary) subdomain.

scounts(N,S,2) should be an array whose element scounts(m+1,s,c) will have the
number of particles of species s which the local node should send to the node m as
primary (c = 1) or secondary (c = 2) ones of the node m (not of the local node), after
each call of oh1_transbound(). Remember that scounts(n+1,s,c) for the local
node n itself can be non-zero when it has particles residing in its primary (secondary)
subdomain moving to its secondary (primary) subdomain.

mycomm should be a structured data of oh mycomm type whose definition  is given in
oh type.F90 as a part of the module named oh type and will have the following inte-
gers when oh1_transbound() (re)builds a new helpand-helper configuration.

prime is the MPI communicator for the family which the local node belongs to as
the helpand, or MPI_COMM_NULL if it is a leaf of the helpand-helper tree.

sec is the MPI communicator for the family which the local node belongs to as a
helper, or MPI_COMM_NULL if it is the root of the helpand-helper tree.

rank is the rank of the local node in the prime communicator, or −1 if it is a leaf.

root is the rank of the helpand node in the sec communicator, or −1 if the local
node is the root.

black is 0 if the prime communicator is colored red, or 1 if colored black.

That is, oh mycomm is defined as follows.

type oh_mycomm
sequence
integer :: prime, sec, rank, root, black
end type oh_mycomm

nbor(3,. . .,3) should be a D-dimensional array of three elements for each dimension and
its element nbor(ν1,...,νD) (νd ∈[1, 2, 3]) must have the identifier of the subdo-
main adjacent to the primary subdomain of the local node. More specifically, let
(π1, . . . , πD) be the coordinates for the local node in a conceptual D-dimensional in-
teger coordinate system in which computational nodes (or equivalently their primary
subdomains) are laid out, and rank(π′1, . . . , π′D) be the function to map the grid point
(π′1, . . . , π′D) to the identifier (MPI rank) of the node located at the point. With these
definitions, an element of the array nbor should have the following (Figure 6).

nbor(ν1,...,νD) = rank(π1 + ν1 −2,  . . . , πD + νD −2)

If D = 3, for example, nbor(1,1,1) should have the idenetifier of the neighbor node
whose primary subdomain contacts with that of the local node only at its west-south-
bottom corner, nbor(1,2,3) should be for the node which shares west-top edge of


<!-- Page 29 -->

=       ( 0, Π − 1 )                                                                            = rank (Π − ,1 Π − 1 )    = 0rank+ Π (Π − 1 )                                                                             = (Π − 1 ) J+ Π (Π − 1 )    = Π (ΠL− 1 )                                                                                = Π Π − 1 J  L                                  J





nbor(1,3)           nbor(2,3)         nbor(3,3)
=(*nbor)[2][0]       =(*nbor)[2][1]    =(*nbor)[2][2]
=   π − π +       =   π π +       =   π + π +
= π −  + Π π +      = π + Π π +       = π +  + Π π +
=  − + Π             =  + Π             =  + + Π





nbor(1,2)          nbor(2,2)          nbor(3,2)
=(*nbor)[1][0]      =(*nbor)[1]         =(*nbor)[1][2]
=   π − π        =   π π          =   π + π                             = π −  + Π π        = π# $+ Π[1]&π           = π +  + Π π                             =  −                 =   &            =  +




nbor(1,1)           nbor(2,1)         nbor(3,1)
=(*nbor)[0]          =(*nbor)[0][1]    =(*nbor)[0][2]
=   π − π −       =   π π −       =   π + π −                            = -/.0,π − [0]*+ Π π −      = π + Π π −       = π +  + Π π −       3                            =  − − Π *         =  − Π             =  + − Π





=                                                                                          =  Π −      =  ;/< +>Π  ⋅                                                                                   = FΠH −  + Π  ⋅      = 6                                                                                   = Π D− @



Figure 6: nbor and its default setting in Πx × Πy node coordinate system given by pcoord.


<!-- Page 30 -->

the local node, nbor(3,2,2) should be the east neighbor of the local node, and
nbor(2,2,2) is the local node itself.

Note that the neighboring relationship may or may not be periodic along each axis.
That is, if the node coordinate system is [0, Πx−1] × [0, Πy−1] × [0, Πz−1] and the
local node is located at (0, 0, 0), it may have west neighbor (Πx−1, 0, 0) while its south
neighbor can be nonexistent. In the latter case for nonexistent neighbors, nbor can
have elements being −2 (or less) to indicate that the corresponding neighboring grid
points have no nodes. Also note that nonexistent neighbors can be found not only
outside the node coordinate system but also in its inside for, e.g., holes.

Alternatively, if the work to define nbor is tiresome for you, you may delegate it to
oh1 init() by making nbor(1, . . . ,1) = −1, and giving the size of node coordinate
system Π1×· · ·×ΠD = N through the argument array pcoord(D)=(/Π1,...,ΠD/).
In this case, oh1 init() initializes nbor assuming fully periodic coordinate system of
[0, Π1−1] × . . . [0, ΠD−1] and r = rank(π1, . . . , πD) is given as follows.

rD = πD     rd = rd+1Πd + πd     r = r1

pcoord(D) should be an array whose element pcoord(d) has the size of the d-th dimension
Πd of the conceptual integer coordnate system of [0, Π1−1]×· · ·×[0, ΠD−1] in which
N = Π1 × · · · × ΠD computational nodes are layed out, if you delegate the setting of
the array nbor(3,. . .,3) to oh1 init(). Otherwise, the array can have any values.

stats defines how statistics data is collected. See §3.10 for more details.

repiter defines how frequently statistics data is reported when stats = 2. See §3.10 for
more details.

verbose defines how verbosely the execution progress is reported.  See §3.11 for more
details.


C Interface

void oh1_init(int **sdid, int nspec, int maxfrac, int **nphgram,
int **totalp, int **rcounts, int **scounts, void *mycomm,
int **nbor, int *pcoord, int stats, int repiter, int verbose);


**sdid should be a double pointer to an array of two elements, or a pointer to NULL (not
NULL itself) to order oh1 init() to allocate the array and return the pointer to it
through the argument. The array will have the identifiers of primary and secondary
subdomains of the local node in (*sdid)[0] and (*sdid)[1] respectively. Therefore,
(*sdid)[0] is always equivalent to the MPI rank number of the calling process.
On the other hand, (*sdid)[1] intially has −1 to mean we are in primary mode
initially, but will be set to a non-negative number in [0, N−1] to identify the secondary
subdomain by oh1_transbound() if it turns the mode to secondary. Note that, even
in secondary mode, (*sdid)[1] may have −1  if the local node is the root of the
helpand-helper tree.

nspec should have the number of species S.

maxfrac should have the tolerance factor percentage of load imbalance α greater than 0
and less than 100.


<!-- Page 31 -->

**nphgram should be a double pointer to an array of 2 × S × N elements to form
nphgram[2][S][N] conceptually,  or a pointer to NULL (not NULL  itself) to order
oh1 init() to allocate the array and return the pointer to it through the argument.
Its element nphgram[c][s][m]6 has the number of particles residing in the subdomain
m ∈[0, N−1], categorized in the species s ∈[0, S−1] and accommodated by the local
node as its primary (c = 0) or secondary (c = 1) ones. The contents of the array can
be undefined at the call of oh1 init() but must be completely defined at the call of
oh1_transbound(). Upon returning from oh1 init() and oh1_transbound(), the
contents of the array will be zero-cleared, so that you can (re)start counting.

**totalp should be a double pointer to an an array of 2×S elements to form totalp[2][S]
conceptually, or a pointer to NULL (not NULL itself) to order oh1 init() to allocate
the array and return the pointer to it through the argument. Its element totalp[c][s]
will have the number of primary (c = 0) or secondary (c = 1) particles of species s
to be accommodated by the local node as the result of load balancing performed by
oh1_transbound(). Note that oh1 init() does not give any values to the array.

**rcounts should be a double pointer to an array of 2 × S × N elements to form
rcounts[2][S][N] conceptually,  or a pointer to NULL (not NULL  itself) to order
oh1 init() to allocate the array and return the pointer to it through the argu-
ment.  Its element rcounts[c][s][m] will have the number of particles of species s
which the local node should receive from the node m as primary (c = 0) or secondary
(c = 1) ones of the local node, after each call of oh1_transbound(). Remember
that rcounts[c][s][n] for the local node n itself can be non-zero when it has parti-
cles residing in its primary (secondary) subdomain moving to its secondary (primary)
subdomain.

**scounts should be a double pointer to an array of 2 × S × N elements to form
scounts[2][S][N] conceptually,  or a pointer to NULL (not NULL  itself) to order
oh1 init() to allocate the array and return the pointer to it through the argument.
Its element scounts[c][s][m] will have the number of particles of species s which the
local node should sent to the node m as primary (c = 0) or secondary (c = 1) ones of
the node m (not of the local node), after each call of oh1_transbound(). Remember
that scounts[c][s][n] for the local node n itself can be non-zero when it has parti-
cles residing in its primary (secondary) subdomain moving to its secondary (primary)
subdomain.

*mycomm should be a pointer to a structured data named S mycommc whose definition is
given in ohhelp c.h.  Alternatively,  it can be NULL (itself)  if you do not want to
bother to play with family communicators but use only library functions for collective
communications among family members.   If you give the pointer to a S mycommc
structure, you will have the followings when oh1_transbound() (re)builds a new
helpand-helper configuration.

MPI_comm prime is the MPI communicator for the family which the local node be-
longs to as the helpand, or MPI_COMM_NULL if it is a leaf of the helpand-helper
tree.

6For the sake of conciseness, an element of conceptual n-dimensional array a of m0 ×· · · mn−1 elements,
which is one-dimensional in reality with ANSI-C, is denoted by a[i0] . . . [in−1] which should be a[j] in reality
where j is defined by j0 = i0, jk = ik + jk−1mk, j = jn−1. Therefore nphgram[c][s][m] is (*nphgram)[m +
N(s + Sc)] in reality with ANSI-C, while the three-dimensional notation can be used with C99.


<!-- Page 32 -->

MPI_comm sec is the MPI communicator for the family which the local node belongs
to as a helper, or MPI_COMM_NULL if it is the root of the helpand-helper tree.

rank is the rank of the local node in the prime communicator, or −1 if it is a leaf.

root is the rank of the helpand node in the sec communicator, or −1 if the local
node is the root.

int black is 0 if the prime communicator is colored red, or 1 if colored black.

That is, S mycommc is defined as follows.

struct S_mycommc {
MPI_Comm prime, sec;
int rank, root, black;
};

**nbor should be a double pointer to an array of 3D elements to form nbor[3] . . . [3] con-
ceptually, or a pointer to NULL (not NULL itself) if you want the library to allocate
and initialize the array and return the pointer to it through the argument.  If you
prepare the array, its element nbor[νD−1] . . . [ν0] (νd ∈[0, 1, 2]) must have the iden-
tifier of the subdomain adjacent to the primary subdomain of the local node. More
specifically, let (π0, . . . , πD−1) be the coordinates for the local node in a conceptual
D-dimensional integer coordinate system in which computational nodes (or equiva-
lently their primary subdomains) are laid out, and rank(π′0, . . . , π′D−1) be the function
to map the grid point (π′0, . . . , π′D−1) to the identifier (MPI rank) of the node located
at the point. With these definitions, an element of the array nbor should have the
following (Figure 6).

nbor[νD−1] . . . [ν0] = rank(π0 + ν0 −1,  . . . , πD−1 + νD−1 −1)

If D = 3, for example, nbor[0][0][0] should have the idenetifier of the neighbor
node whose primary subdomain contacts with that of the local node only at its west-
south-bottom corner, nbor[2][1][0] should be for the node which shares west-top
edge of the local node, nbor[1][1][2] should be the east neighbor of the local node,
and nbor[1][1][1] is the local node itself.

Note that the neighboring relationship may or may not be periodic along each axis.
That is, if the node coordinate system is [0, Πx−1] × [0, Πy−1] × [0, Πz−1] and the
local node is located at (0, 0, 0), it may have west neighbor (Πx−1, 0, 0) while its south
neighbor can be nonexistent. In the latter case for nonexistent neighbors, nbor can
have elements being −2 (or less) to indicate that the corresponding neighboring grid
points have no nodes. Also note that nonexistent neighbors can be found not only
outside the node coordinate system but also in its inside for, e.g., holes.

Alternatively,  if the work to define nbor is tiresome for you, you may delegate it
to oh1 init() by passing a pointer to NULL or by making **nbor = −1, and
giving the size of node coordinate system Π0 × · · · ΠD−1 = N through the ar-
gument array pcoord[D] = {Π0, . . . , ΠD−1}.  In this case, oh1 init() initializes
(*nbor) assuming fully periodic coordinate system of [0, Π1−1] × . . . [0, ΠD−1] and
r = rank(π0, . . . , πD−1) is given as follows.

rD−1 = πD−1     rd = rd+1Πd + πd     r = r0


<!-- Page 33 -->

*pcoord should be a pointer to an array of D elements and each element pcoord[d] should
have the size of the d-th dimension Πd of the conceptual integer coordnate system of
[0, Π0−1] × · · · × [0, ΠD−1−1] in which N = Π0 × · · · × ΠD−1 computational nodes
are layed out,  if you delegate the setting of the array (*nbor)[3D] to oh1 init().
Otherwise, pcoord can be NULL or the array can have any values.

stats defines how statistics data is collected. See §3.10 for more details.

repiter defines how frequently statistics data is reported when stats = 2. See §3.10 for
more details.

verbose defines how verbosely the execution progress is reported.  See §3.11 for more
details.


#### 3.4.2 oh1_neighbors()

The function (subroutine) oh1_neighbors() receives an array nbor through which oh1_
transbound() will report the neighbors of the local nodes to your simulator body.


Fortran Interface

subroutine oh1_neighbors(nbor)
implicit none
integer,intent(inout) :: nbor(3,3,3,3)      ! for 3D codes.
end subroutine


C Interface

void oh1_neighbors(int **nbor);


nbor should be a (D+1)-dimensional array nbor(3,. . . ,3,3) in Fortran or a double pointer
to an array of 3 · 3D elements to form nbor[3]. . . [3][3] conceptually in C. When
D = 3 for example, nbor(:,:,:,1) or nbor[0][][][] will always have what ν(:,:,:)
or ν[][][] has where ν is the array which you gave to oh1_init() (or its higher-level
counterpart) through its argument nbor. On the other hand, nbor(:,:,:,2) or
nbor[1][][][] will have ν(:,:,:) or ν[][][] in the helpand of the local node to show
you the neighbors of its secondary subdomain, when we are in secondary mode. In
addition, nbor(:,:,:,3) or nbor[2][][][] will have what nbor(:,:,:,2) or nbor[1][][][]
had just before you call oh1_transbound() (or its higher-level counterpart) which
returns −1 to mean helpand-helper configuration is (re)built. That is, nbor(:,:,:,3)
or nbor[2][][][] has neighbors of the old secondary subdomain which the local node was
responsible for before the helpand-helper reconfiguration.

The function helps you to find a subdomain adjacent to the local node’s primary and,
particularly, secondary subdomains. For example, if you find a set of secondary particles
crossing the west-top edge of the secondary subdomain, you will know the destination
subdomain looking up nbor(1,2,3,2) or nbor[1][2][1][0] if the last oh1_transbound()
returns 1, while nbor(1,2,3,3) or nbor[2][2][1][0] will show you the destination if the
return value is −1 because the node is still responsible for sending the particles crossing a
boundary of the old secondary subdomain.
As described above, the argument array nbor has a tight relationship with the array ν
being nbor of oh1_init(). More specifically, the relationship is maintained as follows.


<!-- Page 34 -->

- The simplest way is to give the same array to oh1_init() and oh1_neighbors().
For  example,   if  your  Fortran  array  is myneighbor(3,3,3,3),  you may  give
myneighbor(:,:,:,1) to oh1_init() and then myneighbor(:,:,:,:)  to oh1_
neighbors(). If your simulator is written in C and your array is myneighbor[3][3]
[3][3] on the  other hand,  both  functions  will work  perfectly  well  receiving
(int**)(&&myneighbor[0][0][0][0]) commonly.  Alternatively, a simulator in C
may give a double pointer to myneighbor pointing NULL to oh1_init() to allocate
an array of 3 · 33 integers, and then give the same pointer to oh1_neighbors() to
have the access to the array through *to myneighbor.

- If you have some reason to have two arrays, say nbor a and nbor b, for oh1_init()
and oh1_neighbors() respectively, the contents of nbor a(:,:,:) or *nbor a[][][] are
copied into nbor b(:,:,:,1) or *nbor b[0][][][] by oh1_neighbors() automatically.
In this case, your C simulator may give a double pointer nbor b such that *nbor b =
NULL to let oh1_neighbors() allocate the array and to let *nbor b point the head of
the array.

- Though it is recommended to call oh1_neighbors() after the call of oh1_init(),
you may call the function before the call of oh1_init(). If you do it, the array given
to oh1_neighbors() is initialized by oh1_init() consistently.

Note that nbor(:,:,:,2) or nbor[1][][][] is meaningless when the local node does not
have a secondary subdomain, except for the timing the mode is switched from secondary
to primary by the last oh1_transbound(). In this critical timing, the subarray remembers
the neighbors of the old secondary subdomain to be released from the local node. Similarly,
nbor(:,:,:,3) or nbor[2][][][] is meaningless when the last oh1_transbound() did not
returns −1, or the local node did not have a secondary subdomain before the call even if
the return value was −1.

#### 3.4.3 oh1_families()

The function (subroutine) oh1_families() receives arrays famindex and members through
which oh1_transbound() will report the configuration of all families to your simulator body
each time the helpand-helper tree is reconfigured.


Fortran Interface

subroutine oh1_families(famindex, members)
implicit none
integer,intent(inout) :: famindex(:)
integer,intent(inout) :: members(:)
end subroutine


C Interface

void oh1_families(int **famindex, int **members);


famindex should be a one-dimensional array of N + 1 elements (or larger) in Fortran, or a
double pointer to the array in C, to have indices of the array members below.

members should be a one-dimensional array of 2N elements (or larger) in Fortran, or a
double pointer to the array in C, to have ranks of the members of all families as
described below.


<!-- Page 35 -->

Note that a simulator body in C may give double pointers to NULL for the arguments above
to let the library allocate the arrays.
As discussed in §2.3, in secondary mode, each subdomain m has a family of nodes
F(m) = m ∪H(m) where H(m) is the set of helpers for m and satisfies;

N−1∩            N−1∪
H(m) = ∅      H(m) = [0, N) −{r}

m=0            m=0

with a root node r. The arrays famindex and members represent F(m) for all m ∈[0, N)
as follows.

m−1∑
famindex(m+1) = famindex[m] = im =     |F(j)|
j=0
members(im+1) = members[im] = m
{members(k) | im+1 < k ≤im+1} = {members[k] | im+1 ≤k < im+1} = H(m)

Note that famindex(N + 1) = famindex[N] = 2N −1 to make im+1 −im = |F(m)|
∑N−1
for all m ∈[0, N) because   m=0 |F(m)| = 2N −1 always.  Moreover, the last element
of members namely members(2N) = members[2N−1] has r being the rank of root node.
Therefore, for any subdomain m you can identify all members in F(m) by scanning elements
members(im+1), . . . , members(im+1) or members[im], . . . , members[im+1−1]. Moreover, you
can traverse the helpand-helper tree from the root r.
The two arrays represent F(m) even when we are in primary mode in which F(m) =
{m} for all m resulting in famindex(m+1) = members(m+1) = m or famindex[m] =
members[m] = m.
When you try to perform inter-node particle transfer by yourself, you will consult the two
arrays and nbor given to oh1_neighbors() to find the family members of the subdomain
adjacent to the primary or secondary subdomain of the local node.  For example, the
following code snip is to send particles in a one-dimensional array sbuf. In this example, it
is supposed that primary (ps = 0) or secondary (ps = 1) particles of species s moving to (or
staying in) the neighbor subdomain identified by x, y and z are in the region in sbuf from
the index head(x,y,z,s,ps+1) or head[ps][s][z][y][x], and the number of particles
to be sent to a node m ∈[0, N) is given by scounts(m+1,s,ps+1) or scounts[ps][s][m]
being an argument array of oh1_init().

! Fortran
r=1
do ps=1,2
do s=1,nspec
do z=1,3;  do y=1,3;  do x=1,3
n=neighbor(x,y,z,ps)
from=famindex(n+1)+2;  to=famindex(n+2)
h=head(x,y,z,s,ps);  c=scounts(n+1,s,1)
call MPI_Isend(sbuf(h), c, ptype, n, tag, MPI_COMM_WORLD, req(r), e)
h=h+c;  r=r+1
do k=from,to
m=members(k);  c=scounts(m+1,s,2)
call MPI_Isend(sbuf(h), c, ptype, m, tag, MPI_COMM_WORLD, req(r), e)
h=h+c;  r=r+1
end do
end do;  end do;  end do;


<!-- Page 36 -->

end do
end do

//C
r=0;
for (ps=0;ps<2;ps++)
for (s=0;s<nspec;s++)
for (z=0;z<3;z++)  for (y=0;y<3;y++)  for (x=0;x<3;x++) {
n=neighbor[ps][z][y][x];
from=famindex[n]+1;  to=famindex[n+1];
h=head[ps][s][z][y][x];  c=scounts[0][s][n];
MPI_Isend(sbuf+h, c, ptype, n, tag, MPI_COMM_WORLD, req[r]);
h+=c;  r++;
for (k=from; k<to; k++) {
m=members[k];  c=scounts[1][s][m];
MPI_Isend(sbuf+h, c, ptype, m, tag, MPI_COMM_WORLD, req[r]);
h+=c;  r++;
}
}



#### 3.4.4 oh1_transbound()

The function oh1 transbound() performs global collective communications of nphgram
to examine whether the number of particles in nodes are well balanced.  If it finds load
imbalance is unacceptably large, it (re)builds helpand-helper configuration updating the
structure mycomm.  In addition,  if oh1_neighbors() and/or oh1_families() have been
called prior to this function, it updates the arrays given to these functions (subroutines) to
show your simulator body the new neighbors and families corresponding to the new helpand-
helper configuration. Finally, it makes particle transfer schedule to report it through the
arrays rcounts and scounts, and updates the array totalp so that the array has the
number of particles accommodated by the local node after the particle transfer.  It also
makes nphgram zero-cleared to give initial values of particle counting in the next simulation
step.  Note that the arrays nphgram, rcounts, scounts and totalp and the structure
mycomm were given to oh1_init() as its arguments.
Besides these global arrays and structure, oh1 transbound() takes two arguments and
returns an integer value to show you the mode in the next simulation step, as follows.


Fortran Interface

integer function oh1_transbound(currmode, stats)
implicit none
integer,intent(in) :: currmode
integer,intent(in) :: stats
end function


C Interface

int oh1_transbound(int currmode, int stats);


currmode should have an integer in [0, 1] to represent current execution mode as follows.

- 0 means we are in primary mode.


<!-- Page 37 -->

- 1 means we are in secondary mode.

stats inactivates statistics collection if 0, regardless the specification given by stats ar-
gument of oh1_init(). You may inactivate the statistics collection temporarily on,
for example, the first call of oh1 transbound() for initial load balancing as discussed
in §3.10.

return value is an integer in {−1, 0, 1} to represent the execution mode in the next sim-
ulation step as follows.

- −1 means helpand-helper configuration is (re)built and thus we will be in sec-
ondary mode.  This also means that you have to broadcast field-arrays from
helpands to their helpers if their replications are necessary for helpers.

- 0 means we will be in primary mode.

- 1 means we will be in secondary mode but helpand-helper configuration has been
kept.

Usually, telling the current execution mode and receiving that in the next simulation step
to/from oh1 transbound() is easily implmented by having your own currmode variable.
That is, the following should be necessary and sufficient.

- Give 0 to oh1 transbound() at the first call because you have not yet built helpand-
helper configuration even if the initial particle distribution causes an unacceptable
load imbalance.

- Let your own currmode be the return value. If it is negative, let it be 1 and broadcast
field-arrays if necessary. Then give it to oh1 transbound() on the second call and
repeat this for successive calls.


#### 3.4.5 oh1_accom_mode()

The function oh1 accom mode() shows its caller whether particle accommodation by nodes
are normal or anywhere through its return value. That is, if all nodes have its primary and
secondary particles in its corresponding primary and secondary subdomains or the neighbor
of the subdomains, the function returns 0 to indicate normal accommodation with which,
for example, your own particle transfer mechanism may exchange particles in a local node
only with its family members and those of neighbors. Otherwise, i.e., if a node has a particle
residing in a subdomain other than its primary or secondary subdomain or a neighbor of
the subdomain due to initial particle distribution, particle warp, particle injection into an
arbitrary position, and so on, the function returns 1 to indicate anywhere accommodation
which requires an all-to-all-type global communication for particle transfer.
Note that the accommodation mode is according to the last call of oh1_transbound()
and, if it made the helpand-helper (re)configuration, to the subdomain assignments before
the (re)configuration. Therefore in normal accmmodation, if we were in secondary mode and
the helpand-helper reconfiguration took place, a node may have secondary particles in its
old secondary subdomain and the neighbors of the subdomain to be sent to the members
of the new families for the subdomains.  Similarly,  if we were in secondary mode but in
primary mode now, a node may have secondary particles in its old secondary subdomain
and the neighbors of the subdomain to be sent to the nodes responsible for the subdomains
as their primary subdomains.


<!-- Page 38 -->

Fortran Interface

integer function oh1_accom_mode()
implicit none
end function


C Interface

int  oh1_accom_mode();


return value is an integer in {0, 1} to represent the accmmodation mode either of normal
(0) or anywhere (1).


#### 3.4.6 oh1_broadcast()

The function (subroutine) oh1 broadcast() performs red-black broadcast communications
in the families the local node belongs to. The arguments of the function pbuf, pcount
and ptype specify the data to be broadcasted in the primary family which the local node
belongs to as the helpand, while sbuf, scount and stype are for the data to be broadcasted
in the secondary family which the local node belongs to as a helper, as shown in Figure 7.
You may be unaware that the local node really has its primary or secondary family, because
the function will skip the primary broadcast if it is a leaf and the secondary one if it is the
root.


Fortran Interface

subroutine oh1_broadcast(pbuf, sbuf, pcount, scount, ptype, stype)
implicit none
real*8,intent(in)  :: pbuf
real*8,intent(out) :: sbuf
integer,intent(in) :: pcount
integer,intent(in) :: scount
integer,intent(in) :: ptype
integer,intent(in) :: stype
end subroutine


C Interface

void oh1_broadcast(void *pbuf, void *sbuf, int pcount, int scount,
MPI_Datatype ptype, MPI_Datatype stype);


pbuf7 should be (the pointer to) the first element of the data buffer which the local node
broadcasts to its helpers in its primary family.

sbuf should be (the pointer to) the first element of the data buffer to receive data broad-
casted in the secondary family.

pcount should have the number of ptype elements to be broadcasted in the primary family.
This value should match scount of the call in the helpers.

7In  the  Fortran module  file oh mod1.F90,  the arguments pbuf and sbuf  of  oh1_broadcast(),
oh1_all_reduce() and oh1_reduce() are declared as real*8 type hoping it matches the type of the el-
ements in your buffers. If this is incorrect, feel free to modify the declaration or to remove it, so that your
compiler accept your calls of the library subroutines.


<!-- Page 39 -->

sbuf stype × scount

pbuf ptype × pcount





Figure 7: Red-black collective communication by oh1_broadcast(), oh1_all_reduce()
and oh1_reduce().


scount should have the number of stype elements to be broadcasted in the seconary family.
This value should match pcount of the call in the helpand.

ptype should have the MPI data-type of elements to be broadcasted in the primary family.
This value should match stype of the call in the helpers.

stype should have the MPI data-type of elements to be broadcasted in the secondary
family. This value should match ptype of the call in the helpand.


#### 3.4.7 oh1_all_reduce()

The function (subroutine) oh1 all reduce() performs red-black all-reduce communications
in the families the local node belongs to. The arguments of the function pbuf, pcount,
ptype, pop specify the data to be reduced in the primary family, while sbuf, scount, stype
and sop are for the data to be reduced in the secondary family. You may be unaware that
the local node really has its primary or secondary family, because the function will skip the
primary reduction if it is a leaf and the secondary one if it is the root.


Fortran Interface

subroutine oh1_all_reduce(pbuf, sbuf, pcount, scount, ptype, stype, &
pop, sop)
implicit none
real*8,intent(inout) :: pbuf
real*8,intent(inout) :: sbuf
integer,intent(in)   :: pcount
integer,intent(in)   :: scount
integer,intent(in)   :: ptype
integer,intent(in)   :: stype
integer,intent(in)   :: pop


<!-- Page 40 -->

integer,intent(in)   :: sop
end subroutine


C Interface

void oh1_all_reduce(void *pbuf, void *sbuf, int pcount, int scount,
MPI_Datatype ptype, MPI_Datatype stype,
MPI_Op pop, MPI_Op sop);


pbuf should be (the pointer to) the first element of the data buffer to be reduced in the
primary family. The buffer is replaced with the reduction result.

sbuf should be (the pointer to) the first element of the data buffer to be reduced in the
secondary family. The buffer is replaced with the reduction result.

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
should match pop of the call in the helpers.

#### 3.4.8 oh1_reduce()

The function (subroutine) oh1 reduce() performs red-black simple one-way reduce com-
munications in the families the local node belongs to. The arguments of the function pbuf,
pcount, ptype, pop specify the data to be reduced in the primary family, while sbuf,
scount, stype and sop are for the data to be reduced in the secondary family. You may be
unaware that the local node really has its primary or secondary family, because the function
will skip the primary reduction if it is a leaf and the secondary one if it is the root.


Fortran Interface

subroutine oh1_reduce(pbuf, sbuf, pcount, scount, ptype, stype, pop, sop)
implicit none
real*8,intent(inout) :: pbuf
real*8,intent(in)    :: sbuf
integer,intent(in)   :: pcount
integer,intent(in)   :: scount
integer,intent(in)   :: ptype
integer,intent(in)   :: stype
integer,intent(in)   :: pop
integer,intent(in)   :: sop
end subroutine
