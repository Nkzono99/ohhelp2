# 4.4 Header File ohhelp2.h

Source: `doc/original/ohhelp.pdf`, pages 226-233.

<!-- Page 226 -->

## 4.4 Header File ohhelp2.h

The header file of level-2 library, ohhelp2.h, #include’s a C header file oh part.h to declare
a structured data type S_particle to represent a particle, declares global variables used
in level-2 library codes to manipulate particles and buffers of them, and gives prototypes
of API functions and those called by higher level codes.


#### 4.4.1 Header File Inclusion

At very first, ohhelp2.h #include’s a header file named oh part.h which defines the struct
for a particle, S_particle. As discussed in §3.5.1, it has the following elements in default.

- x, y and z have the double-float three-dimensional coordinate at which the particle is
located.

- vx, vy and vz have the double-float velocity elements of the particle.

- pid is the unique 64-bit integer identifier given to the particle.

- nid is the identifier of the subdomain in which the particle is residing, or will be
resinding in the next simulation step.

- spec is the integer in [0, S−1] in C codes or in [1, S] in Fortran codes, to identify the
species of the particle.


#include "oh_part.h"



#### 4.4.2 Particle Buffers and Related Variables

Next we declare the following variables for particles and their transfer, using the trick of
EXTERN to have their home in ohhelp2.c.

nOfLocalPLimit     • The integer variable nOfLocalPLimit have the absolute maximum number of particles
which a local node can accommodate. Its value Plim should be defined by the simula-
tor body possibly by calling oh2_max_local_particles() and then should be given
to oh2_init() through its argument maxlocalp. The value defines the size of the par-
ticle buffer Particles[] if it is not allocated by the simulator body, and particle send
buffer SendBuf[]. The variable is referred to by oh2_inject_particle() to check if
Particles[] has room to inject a particle, and by oh2_remap_injected_particle()
and oh2_remove_injected_particle() to check if the argument particle is in the
region for injected particles.

Particles     • Each element of the array Particles[Plim] has the S_particle structure of a paritlce
accommodated by the local node. The array must be allocated by a Fortran-coded
simulator body and the pointer to it must be given to oh2_init() through its ar-
gument pbuf. On the other hand, a C-coded body may do so by giving the double
pointer to it through pbuf, or may give a pointer to NULL through pbuf to let oh2_
init() allocate the array body. The buffer is partitioned into two parts, one for
primary particles and the other for secondary ones. Then each part is further de-
composed for species to have 2S blocks pbuf (0, 0), . . . , pbuf (0, S−1), pbuf (1, 0), . . . ,
pbuf (1, S−1) in this order and the size of pbuf (p, s) is TotalP[p][s].

The array is directly referred to by the following functions;


<!-- Page 227 -->

move_to_sendbuf_secondary(), move_to_sendbuf_uw(),
move_to_sendbuf_dw(), move_injected_to_sendbuf(),
oh2_inject_particle(), oh2_remap_injected_particle() and
oh2_remove_injected_particle();

and indirectly through RecvBufBases[][] by the following functions;

try_primary2(), exchange_particles(), move_injected_from_sendbuf()
and receive_particles().

SendBuf     • The array SendBuf[Plim] of S_particle structures is used to send particles from
the local node to other nodes. The buffer is partitioned for species and then for
receiver nodes to have SN blocks sbuf (0, 0),  . . . , sbuf (0, N−1),  . . . , sbuf (S−1, 0),
. . . , sbuf (S−1, N−1) in this order and the displacement of the head of sbuf (s, n)
from the head of SendBuf[] is SendBufDisps[s][n].  Since a node may send out all
the particles in Particles[] and receives the same amount of particles from other
nodes, we need to have SendBuf[] of the size Plim same as Particles[]. However,
the number of sending particles is significantly smaller than Plim in usual cases, we
could limit the size of SendBuf to a small fraction, say 10 %, of Particles[] if we can
devise an in-place all-to-all communication required in, for example, initial particle
distribution.

The array is allocated by init2() or its counterpart in level-4p or higher library,
and referred to by try_primary2(), exchange_particles(), move_to_sendbuf_
uw(),  move_to_sendbuf_dw(),  move_injected_to_sendbuf(),  move_injected_
from_sendbuf(), receive_particles() and send_particles().

RecvBufBases     • The element [p][s] of the array RecvBufBases[2][S]37 is the S particle type pointer
to a block rbuf (p, s) in Particles[] to which the local node receives primary (p = 0)
or secondary (p = 1) particles of species s from other nodes. That is, the block is
the receive buffer for particle transfer. The array is allocated by init2() and then
its elements are initialized by move_to_sendbuf_uw() through move_to_sendbuf_
primary() and move_to_sendbuf_secondary(). Then the elements are referred to
by try_primary2(), exchange_particles(), and receive_particles() and are
updated by them so that the elements point the receive buffer for each sender node.
Similarly, move_injected_from_sendbuf() refers to and update the elements so that
they reflects particle injection into the primary subdomain of the local node.

secondaryBase     • The integer pointer secondaryBase and totalLocalParticles point the shadow
totalLocalParticles        variables of primaryParts and totalParts which are located in the argument array
of oh2_init() namely pbase[1] and pbase[2]. That is, the shadow variable pbase[1]
has the number of primary particles Qnn and thus the displacement of the base of
pbuf (1, 0), the first block of secondary particles, from the head of Particles[]. On
the other hand, the pbase[2] has the total number of particles Qn and thus the dis-
placement of the first unused entry of Particles[]. The poiters and pointed variables
are initialized by init2(), and *secondaryBase is updated by try_primary2() and
move_to_sendbuf_secondary() together with its substance primaryParts, while
*totalLocalParticles is updated by transbound2().

SendBufDisps     • The element [s][m] of the integer array SendBufDisps[S][N] has the displacement of
the block sbuf (s, m) in SendBuf[] from its head. That is, prior to sending the parti-
cles of species s to the node m is, they are at first moved from Particles[] into the

37It has one extra element [2][0] for sort received particles().


<!-- Page 228 -->

block starting from SendBuf[SendBufDisps[s][m]]. The array is allocated by init2()
and its elements are initialized by set_sendbuf_disps(). Then move_to_sendbuf_
uw(), move_to_sendbuf_dw() and move_injected_to_sendbuf() increment the el-
ements each time they move a particle from Particles[] to SendBuf[].  Finally,
after reinitialized by set_sendbuf_disps(), the elements are referred to by try_
primary2(), exchange_particles(), move_injected_from_sendbuf(), receive_
particles() and send_particles() for particle sending and injection.

RecvBufDisps     • The integer array RecvBufDisps[N] has displacements of receive buffer blocks in
Particles[] for particle transfer with anywhere accommodation. That is, the local
node receives primary (p = 0) or secondary (p = 1) particles of species s from the
node m into the block starting from RecvBufBases[p][s][RecvBufDisps[m]] by MPI_
Alltoallv(). The array is allocated by init2() and its elements are set and referred
to by try_primary2() and exchange_particles().

nOfInjections     • The integer varialbe nOfInjections = Qinjn has the number of particles injected by the
local node n using oh2_inject_particle(). That is, after zero-cleared by init2()
and transbound2(), it is incremented by oh2_inject_particle() to have the num-
ber of injected particles at the call of transbound2(). Then it is referred to by move_
to_sendbuf_primary(), move_to_sendbuf_secondary() and move_injected_to_
sendbuf() to send injected particles to other nodes or to keep primary ones in the
local node, and by oh2_remap_injected_particle() and oh2_remove_injected_
particle() to check if the argument particle is in the region for injected particles.

specBase     • The integer variable specBase has 0  if the library  is called from C-coded sim-
ulator body through oh2_init() or oh3_init(),  or has 1  if called from For-
tran counterpart through oh2_init_() or oh3_init_(), to represent the identifi-
cation number of the first species. Then it is referred to by move_injected_to_
sendbuf(), oh2_inject_particle(), oh2_remap_injected_particle() and oh2_
remove_injected_particle() to translate Fortran’s 1-base representation in spec
element of S_particle particle data into 0-base one used in the library.

T_Particle     • The MPI_Datatype variable T Particle has the MPI data-type of a S_particle
particle data  for  particle  transfer.   The  value  of  this  variable  is created by
MPI_Type_contiguous()  called  in init2(), and used  for MPI communication
in try_primary2(), exchange_particles(), receive_particles() and send_
particles().

Requests     • The array of MPI_Request data Requests[4NS] is to keep the requests of asyn-
Statuses       chronous communications MPI_Isend() and MPI_Irecv() performed in receive_
particles() and send_particles(). Then the results of these requests are stored in
the array Statuses[4NS] of MPI_Status type data by MPI_Waitall() in exchange_
particles(). These arrays are allocated by init2() to have 2 × 2 × S × N ele-
ments for sending/receiveing (2) primary and secondary particles (2) of S species to
N nodes.


EXTERN int nOfLocalPLimit;
EXTERN struct S_particle *Particles;    /* [nOfLocalPLimit] */
EXTERN struct S_particle *SendBuf;      /* [nOfLocalPLimit] */
EXTERN struct S_particle **RecvBufBases;/* [2][nOfSpecies] */
EXTERN int *secondaryBase, *totalLocalParticles;
EXTERN int *SendBufDisps;               /* [nOfSpecies][nOfNodes] */


<!-- Page 229 -->

EXTERN int *RecvBufDisps;               /* [nOfNodes] */
EXTERN int nOfInjections;
EXTERN int specBase;
EXTERN MPI_Datatype T_Particle;
EXTERN MPI_Request *Requests;           /* [nOfNodes*nOfSpecies*2*2] */
EXTERN MPI_Status *Statuses;            /* [nOfNodes*nOfSpecies*2*2] */



#### 4.4.3 Macro Particle_Spec()

Particle_Spec()  The macro Particle_Spec(),  used  in move_injected_to_sendbuf(),  oh2_inject_
particle(), oh2_remap_injected_particle() and oh2_remove_injected_particle()
and higher level library functions such as those in level-4p, is replaced with given argument
expression S if OH_HAS_SPEC is defined, or with 0 otherwise. That is, if S_particle struc-
ture has the element spec as asserted by the fact that OH HAS SPEC is defined, the expression
S having the reference to the element is used to have the species of the injected particle.
Otherwise, it is assured that S = 1 and thus 0 is given for the species unconditionally.

#ifdef OH_HAS_SPEC
#define Particle_Spec(S) (S)
#else
#define Particle_Spec(S) (0)
#endif



#### 4.4.4 Macros Decl_Grid_Info(), Subdomain_Id() and Primarize_id()

Fundamentally, a lower level library  is designed to work independently of the use of
higher level ones.  However a small fraction of level-2 functions have to change its be-
havior with/without position-aware particle management, i.e., depending on whether OH_
POS_AWARE is defined, because nid element of S particle structure has level-dependent
meaning.  Therefore, we define three macros Decl_Grid_Info(), Subdomain_Id() and
Primarize_Id() to cope with the level-dependency to let them act as follows if OH_POS_
AWARE.

Decl_Grid_Info()  The macro Decl_Grid_Info() is to declare an OH_nid_t variable nidelement and inte-
ger variables named subdomid, gridmask and loggrid. The variable gridmask is used
only in level-4p and/or higher level libraries, while other three are used to extract sub-
domain identifier from nid of S_particle structured data for particles referring to a
global variable AbsNeighbors[2][3D],  if the functions move_to_sendbuf_uw(), move_to_
sendbuf_dw() and move_injected_to_sendbuf() are used in a higher-level library for
position-aware particle management with OH_POS_AWARE defined. The variables gridmask
and loggrid are for caching gridMask and logGrid having the mask and bit-width of lower
bits for particle grid-position and thus loggrid has the right-shift count to eliminate it and
to extract subdomain code by the macro Subdomain_Id().

Subdomain_Id()  The macro Subdomain Id(i, p) acts on the subdomain code of the nid element i of a primary
(p = 0) or secondary (p = 1) particle, i.e., σ = ⌊i/2Γ⌋where Γ = loggrid, to give the
subdomain m in which the particle will reside, as its expansion result  if i ≥0.  The
subdomain code σ for a subdomain m is k ∈[0, 3D) if m is the k-th neighbor of the local
node’s primary (p = 0) or secondary (p = 1) subdomain definitely, or m + 3D otherwise38.

38σ can be m + 3D for a neighbor subdomain m.


<!-- Page 230 -->

On the other hand if i < 0, the macro is replaced with −1 so that functions using the macro
find eliminated particles by examining the result of the macro rather than the nid element,
as we can do so without position-aware partilce management.  Therefore, the macro is
replaced with m as;
{
−1                        i < 0
m =   AbsNeighbors[p][σ]   i ≥0 ∧σ < 3D
σ −3D                   i ≥0 ∧σ ≥3D

where AbsNeighbors[p][k] is the subdomain identifier of k-th neighbor of the local node’s
primary (p =  0) or secondary (p =  1) subdomain,  i.e.,  always-positive version  of
Neighbors[p][k] without aware of multiple occurrences of subdomains in a neighbor set.
Note that the local variables nidelement and subdomid declared by Decl_Grid_Info()
are used in this macro to have i and σ temporarily in it.

Primarize_Id()  The subdomain code of a particle can be (N + 3D) + σ  if the particle is injected into
a subdomain represented by σ as a secondary particle of the local node.  The macro
Primarize Id(π, m), used only in move_injected_to_sendbuf() in the level-2 library,
acts on the subdomain code σ′ = (N + 3D) + σ of the particle pointed by π to let it
σ = σ′ −(N + 3D). It also let m be the subdomain identifier encoded in σ by Subdomain_
Id() with p = 1 because the particle is a secondary one. Note that (N + 3D) is cast to
OH_nid_t prior to the subtraction to have σ because the nid can be long␣long␣int.

If OH_POS_AWARE is not defined, on the other hand, Decl_Grid_Info() just declares
a dummy variable unusedvariable which should not been referred to in the functions
using the macro39, while Subdomain_Id() is simply replaced with its first argument and
Primarize_Id() is not defined40.

#ifdef OH_POS_AWARE
EXTERN int gridMask, logGrid;
EXTERN int AbsNeighbors[2][OH_NEIGHBORS];
#define Decl_Grid_Info() \
OH_nid_t nidelement;  int subdomid;\
const int gridmask=gridMask, loggrid=logGrid
#define Subdomain_Id(ID, PS) \
((nidelement=(ID))<0 ? -1 :\
((subdomid=nidelement>>loggrid)<OH_NEIGHBORS ?\
AbsNeighbors[PS][subdomid] : subdomid-OH_NEIGHBORS))
#define Primarize_Id(P, SD) {\
const OH_nid_t nidelem =\
((P)->nid -= (OH_nid_t)(nOfNodes+OH_NEIGHBORS)<<loggrid);\
SD = Subdomain_Id(nidelem, 1);\
}
#else
#define Decl_Grid_Info() int unusedvariable
#define Subdomain_Id(ID, PS) (ID)
#endif



39If we make the expansion result of the macro empty, the macro cannot be followed by any variable
declarations with C89.
40Primarize Id() is not used if OH POS AWARE is undefined and thus we leave it undefined too.


<!-- Page 231 -->

#### 4.4.5 Function Prototypes

The next and last block is to declare function prototypes. First we declare the prototypes
of the API function pairs each of which consists of API for Fortran and C, as listed below.

- The function oh2_init[_]() initializes data strucutures of the level-2 library.

- The function oh2_transbound[_]() at first performs what its level-1 counterpart
oh1_transbound[_]() does to have particle transfer schedule, and then transfers
particles from/to the particle buffer Particles[].

- The function oh2_max_local_particles[_]() calculates Plim, the size of the particle
buffer Particles[].

- The function oh2_inject_particle[_]() injects a particle and place it at the bottom
of Particles[].

- The function oh2_remap_injected_particle[_]() maintains NOfPLocal[][][] and
InjectedParticles[][][] of an injected particle.

- The function oh2_remove_injected_particle[_]() removes an injected particle
maintaining NOfPLocal[][][] and InjectedParticles[][][].

- The function oh2_set_total_particles() is to initialize TotalP, primaryParts and
totalParts with NOfPLocal after having initial particle setting in Particles and
NOfPLocal but before injecting/removing particles into/from them prior to the first
call of oh2_transbound().

As done in §4.2.11, prior to showing the function prototypes, we show the second part
of the header files ohhelp c.h for C-coded simulators and ohhelp f.h for Fortran-coded ones,
which define the aliases of level-2 API functions41. First, they #define the aliases of level-2
API function which does not have higher level counterpart, in the #else part of #if␣OH_
LIB_LEVEL=1.

#else
#define oh_set_total_particles() oh2_set_total_particles()

Then ohhelp c.h gives the prototypes of the function above, which  is also given in
ohhelp2.h, after it #include’s the header file oh part.h to define S_particle.

#include "oh_part.h"
void oh2_set_total_particles();


Next, they #define the aliases of level-2 API functions which have level-4p counterparts;


#if OH_LIB_LEVEL!=4
#define oh_max_local_particles(A1,A2,A3) oh2_max_local_particles(A1,A2,A3)
#define oh_inject_particle(A1)           oh2_inject_particle(A1)
#define oh_remap_injected_particle(A1)   oh2_remap_injected_particle(A1)
#define oh_remove_injected_particle(A1)  oh2_remove_injected_particle(A1)

41Aliases of oh2 set total particles() in ohhelp c.h and ohhelp f.h are silightly different, i.e., the former
has ”()” in both of the macro and definition while latter does not have it.


<!-- Page 232 -->

and function prototypes are given to C headers42.  Note that the prototypes of oh2_
max_local_particles(), oh2_remap_injected_particle() and oh2_remove_injected_
particle() are not given if the level-4p extension is in effect because the functions are use-
less (and harmful), but that of oh2_inject_particle() is given regardless of the extension
because it is may be called for injections followed by remapping.

int  oh2_max_local_particles(long long int npmax, int maxfrac, int minmargin);
void oh2_remap_injected_particle(struct S_particle *part);
void oh2_remove_injected_particle(struct S_particle *part);
#endif
void oh2_inject_particle(struct S_particle *part);


Then both headers #define the aliases level-2 specific API functions if OH_LIB_LEVEL
is 2.

#if OH_LIB_LEVEL==2
#define oh_init(A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14) \
oh2_init(A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14)
#define oh_transbound(A1,A2) oh2_transbound(A1,A2)

Finally, the prototypes of these functions are given in ohhelp c.h and ohhelp1.h.

void oh2_init(int **sdid, int nspec, int maxfrac, int **nphgram,
int **totalp, struct S_particle **pbuf, int **pbase,
int maxlocalp, void *mycomm, int **nbor,
int *pcoord, int stats, int repiter, int verbose);
int  oh2_transbound(int currmode, int stats);


On the other hand, the prototypes of Fortran API functions are solely given in ohhelp2.h,
while their Fortran versions are given in oh mod2.F90 as shown in §3.5.

void oh2_set_total_particles_();
int  oh2_max_local_particles_(dint *npmax, int *maxfrac, int *minmargin);
void oh2_inject_particle_(struct S_particle *part);
void oh2_remap_injected_particle_(struct S_particle *part);
void oh2_remove_injected_particle_(struct S_particle *part);
void oh2_init_(int *sdid, int *nspec, int *maxfrac, int *nphgram,
int *totalp, struct S_particle *pbuf, int *pbase,
int *maxlocalp, struct S_mycommf *mycomm, int *nbor,
int *pcoord, int *stats, int *repiter, int *verbose);
int  oh2_transbound_(int *currmode, int *stats);


Next we declare the prototypes of the following functions used in the level-3 and/or
higher level library.

- The function init2() is the body of oh2_init().

- The function transbound2() is the body of oh2_transbound().

- The function exchange_primary_particles() is the core of the particle transfer in
primary mode.

42Prototypes of oh2 max local particles() in ohhelp c.h and ohhelp2.h are slightly different, i.e., the
type of its first argument is long long int in former, while in latter is dint.


<!-- Page 233 -->

- The functions move_to_sendbuf_primary() moves particles to be transferred from
Particles[] to SendBuf[] and packs those remaining in Particles[] in primary mode.

- The function set_sendbuf_disps() calculates each entry of SendBufDisps[][].

- The function exchange_particles() is the core of the particle transfer in secondary
mode.


/* Prototypes for the functions called from higher-level library code */
void init2(int **sdid, int nspec, int maxfrac, int **nphgram,
int **totalp, struct S_particle **pbuf, int **pbase, int maxlocalp,
struct S_mycommc *mycommc, struct S_mycommf *mycommf,
int **nbor, int *pcoord, int stats, int repiter, int verbose);
int  transbound2(int currmode, int stats, int level);
void exchange_primary_particles(int currmode, int stats);
void move_to_sendbuf_primary(int secondary, int stats);
void set_sendbuf_disps(int secondary, int parent);
void exchange_particles(struct S_commlist *secrlist, int secrlsize,
int oldparent, int neighboring, int currmode,
int stats);
