# 3.6 Level-3 Library Functions

Source: `doc/v1/original/ohhelp.pdf`, pages 48-71.

<!-- Page 48 -->

Fortran Interface

subroutine oh1_set_total_particles
end subroutine


C Interface

void oh2_set_total_particles();


The library internally maintains a copy of totalp to know the layout of pbuf at the call
of oh2_transbound() and update totalp and the copy upon its return. However, at the
first call of oh2_transbound() the library does not know the layout and thus consults
nphgram assuming it is consistent with the layout.  This assumption is usually correct
unless particles are injected/removed before the first call of oh2_transbound(). Therefore,
if by some reason your simulator code needs to inject particles by oh2_inject_particle()
and/or remove them by setting their nid to be −1 in initializing process, you have to set
nphgram so that it describes the contents of pbuf correctly, then call this function oh2_set_
total_particles() to let the library recognize the layout of pbuf, and then inject/remove
particles before the first call of oh2_transbound(). Calling this function in other occasions
are unnecessary but safe providing that nphgram correctly describes the layout of pbuf.

## 3.6 Level-3 Library Functions

Level-3 library provides the following functions.

oh3_init() performs initialization similar to what oh2_init() does and that of level-3’s
own for communications of field-arrays.

oh13_init() performs initialization similar to what oh3_init() does but excludes that for
the particle buffer. That is, roughly speeking, oh13_init() is equal to oh3_init()
minus oh2_init() plus oh1_init().

oh3_grid_size() specifies the grid size of each dimension.

oh3_transbound() performs load balancing almost equivalent to oh2_transbound() or
oh1_transbound() depending on the initializer you choose.

oh3_map_particle_to_neighbor() finds the subdomain which will be the residence of a
boudary crossing particle and is neighboring to the primary or secondary subdomain
of the local node, and then returns its identifier.

oh3_map_particle_to_subdomain() finds the subdomain which will be the residence of a
boudary crossing particle and may be anywhere in the whole space domain, and then
returns its identifier.

oh3_bcast_field() performs broadcast communication of a field-array in helpand-helper
families.

oh3_allreduce_field() performs all-reduce communication of a field-array in helpand-
helper families.

oh3_reduce_field() performs simple one-way reduce communication of a field-array in
helpand-helper families.


<!-- Page 49 -->

oh3_exchange_borders() performs neighboring communication to exchange subdomain
boundary data of a field-array.

The function API for Fortran programs is given by the module named ohhelp3 in the
file oh mod3.F90, while API for C is embedded in ohhelp c.h.


#### 3.6.1 oh3_init()

The function (subroutine) oh3 init() receives a number of fundamental parameters and
arrays through which oh3_transbound() and other subroutines/functions interacts with
your simulator body.  It also initializes internal data structures used in level-1, level-2
and level-3 libraries. Among its twenty-three (23!!)  arguments, other library functions
directly refer to only the bodies of the arguments nphgram and pbuf as their implicit
inputs. Therefore, after the call of oh3 init(), modifying the bodies of other arguments
has no effect to library functions.


Fortran Interface

subroutine oh3_init(sdid, nspec, maxfrac, nphgram, totalp, &
pbuf, pbase, maxlocalp, mycomm, nbor, pcoord, &
sdoms, scoord, nbound, bcond, bounds, ftypes, &
cfields, ctypes, fsizes, &
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
integer,intent(inout) :: sdoms(:,:,:)
integer,intent(in)    :: scoord(2,OH_DIMENSION)
integer,intent(in)    :: nbound
integer,intent(in)    :: bcond(2,OH_DIMENSION)
integer,intent(inout) :: bounds(:,:,:)
integer,intent(in)    :: ftypes(:,:)
integer,intent(in)    :: cfields(:)
integer,intent(in)    :: ctypes(:,:,:,:)
integer,intent(out)   :: fsizes(:,:,:)
integer,intent(in)    :: stats
integer,intent(in)    :: repiter
integer,intent(in)    :: verbose
end subroutine

sdid
nspec
maxfrac
nphgram


<!-- Page 50 -->

totalp
See §3.4.1 because the arguments above are perfectly equivalent to those of oh1_
init().
pbuf
pbase
maxlocalp
See §3.5.2 because the arguments above are perfectly equivalent to those of oh2_
init().

mycomm
nbor
pcoord
See §3.4.1 because the arguments above are perfectly equivalent to those of oh1_
init().

sdoms(2,D,N) should be an array whose element sdoms(β,d,m+1) should have the
d-th (d ∈[1, D]) dimensional integer coordinate of the lower (β = 1) or upper
(β = 2) boundary of the subdomain m ∈[0, N−1], namely δld(m) or δud(m) re-
spectively.  For example, for the 3-dimensional cuboid subdomain m whose grid
points at west-south-east and east-north-top corners are (δlx(m), δly(m), δlz(m)) and
(δux(m)−1, δuy (m)−1, δuz (m)−1), the subarray sdoms(1:2,1:3,m+1) should have the
followings (Figure 9).

sdoms(1:2,1:3,m+1) =
reshape((/δlx(m),δux(m),δly(m),δuy (m),δlz(m),δuz (m)/), (/2,3/))

Note that if the subdomain m is the d-th dimensional lower (upper) neighbor of n
sharing a (D−1)-dimensional plane perpendicular to d-th axis (e.g., a neighbor along
x-axis sharing a yz-plane), n’s lower (upper) boundary plane has to be m’s upper
(lower) boundary plane. For example, if D = 3 and m is n’s lower neighbor along
x-axis, the following must be satisfied.

∆lx = m∈[0,N−1]{δlmin    x(m)}   ∆u                                     x = m∈[0,N−1]{δumax    x(m)}
(δlx(n) = δux(m) ∨δlx(n) = δux(m) −(∆ux−∆lx) ∨δlx(n) = δux(m) + (∆ux−∆lx)) ∧
δly(n) = δly(m) ∧δuy (n) = δuy (m) ∧δlz(n) = δlz(m) ∧δuz (n) = δuz (m)

Alternatively, if the work to define sdoms is bothersome for you, you may delegate it
to oh3 init() by making sdoms(1,1,1) > sdoms(2,1,1), and giving the lower and
upper boundaries of the whole space domain [∆l1, ∆u1−1]×. . .×[∆lD, ∆uD−1] through
the argument array scoord(2,D) as follows.

scoord(:,:) = reshape((/∆l1,∆u1,. . .,∆lD,∆uD/), (/2,D/))

In this case, oh3 init() also refers to the argument array pcoord(D)=(/Π1,...,
ΠD/) and defines sdoms(β,d,m + 1) for m = rank(π1, . . . , πD) as follows.

ad = ⌊(∆ud −∆ld)/Πd⌋
kd = Πd −((∆ud −∆ld) mod Πd)                       {
∆ld + πd · ad             πd ≤kd              sdoms(1,d,m + 1) =
∆ld + πd · ad + (πd −kd)  πd > kd


<!-- Page 51 -->

sdoms(1,1,                 + #)= sdoms[ ][0][0]=      ∈ ) (    $  "            sdoms(2,1,  + )= sdoms[ ][0][1]=
= sdoms(1,1,3+ )= sdoms[  + 1][0][0]=
∈ 32   1    -  +                    sdoms(2,1,  + )= sdoms[ ][0][1]=
= sdoms(1,1,>+ )= sdoms[  + 1][0][0]=
∈ >               <     ;         676 5
sdoms(1,2,  + )= sdoms[ ][1][0]=
∈

= f      = _]_      = e       = c        = `
sdoms(2,2,  + )= sdoms[ ][1][1]=
= sdoms(1,2,  + ) = sdoms[  + 5][1][0]=
9   m = 5  m = 6   m = 7   m = 8    m = 9        ∈
sdoms(2,2,m + 1)= sdoms[m][1][1]=
= sdoms(1,2,m + 6)= sdoms[m + 5][1][0]= 6
( m ∈ [ 04, ])
=     =     =     =      =
sdoms(1,2,  + )= sdoms[ ][1][0]=
∈
27                                               sdoms(2,1,  + )= sdoms[ ][0][1]=
∈ ^            Z    V  T                                                                      []\
sdoms(2,1,  + ) = sdoms[ ][0][1]=
= sdoms(1,1,R+ )= sdoms[  + 1][0][0]=
∈ R          P    L  J                            sdoms(2,1,  + )= sdoms[ ][0][1]=
= sdoms(1,1,H+ )= sdoms[  + 1][0][0]=
∈ H              G        F    B  @

Figure 9: sdoms and its default setting for space domain of 27 × 19 given by scoord and
node coordinate system of 5 × 3 given by pcoord.

m+d = rank(π1, . . . , πd + 1, . . . , πD)                       {
sdoms(1,d,m+d + 1)  πd < Πd −1              sdoms(2,d,m + 1) =
∆ud                  πd = Πd −1


That is, if we have Πx subdomains along x-axis and the lower and upper boundaries
of the whole domain along x-axis are ∆lx and ∆ux, eastmost [(∆ux −∆lx) mod Πx]
subdomains have x-edges of ⌈(∆ux −∆lx)/Πx⌉while remaining western ones have x-
edges of ⌊(∆ux −∆lx)/Πx⌋. Note that the delegation of setting sdoms(:,:,:) also
means that for the argument array bounds(:,:,:).

scoord(2,D) should be an array whose element scoord(β,d) has the d-th (d ∈[1, D])
dimensional integer coordinate of the lower (β = 1) or upper (β = 2) boundary of
the whole space domain,  if you delegate the setting of the array sdoms(2,D,N)
to oh3 init(). That is, scoord(1:2,1:D) should have the following for the space
domain of [∆l1, ∆u1−1] × . . . × [∆lD, ∆uD−1].

scoord(:,:) = reshape((/∆l1,∆u1,. . .,∆lD,∆uD/), (/2,D/))


<!-- Page 52 -->

Figure 10: Complicated subdomains and their boundaries with walls and holes.


Otherwise, i.e., if you completely specify sdoms(:,:,:) by yourself, the array can
have any values.

nbound should be a positive integer representing the number of boundary condition types B
of the space domain. That is, you can specify a type of boundary condition b ∈[1, B]
for each boundary of the whole space domain through the argument bcond(2,D)
or of each subdomain through the argument bounds(2,D,N). Then also you can
specify how the communication through a boundary of a specific type is performed
through the argument ctypes(3,2,B,C). Remember that the boundary condition
type 1 is special and reserved for periodic boundaries.

bcond(2,D) should be an array whose element bcond(β,d) has the type of boundary
condition b ∈[1, B] for the lower (β = 1) or upper (β = 2) boundary plane of the
whole space domain perpendicular to the d-th axis, if you delegate the setting of the
array sdoms(2,D,N) and bounds(2,D,N) to oh3 init().  Otherwise, the array
can have any values.

bounds(2,D,N) should be an array whose element bounds(β,D,m + 1) has the type of
boundary condition b ∈[1, B] for the lower (β = 1) or upper (β = 2) boundary plane
of the subdomain m perpendicular to the d-th axis, if you specify sdoms(:,:,:) by
yourself. Remember that, for a pair of adjacent subdomains, the boundary condition
of the boundary plane shared by them must have type 1, unless the plane is a special
wall. Also remember that a subdomain boundary, which is also a boundary of the
whole space domain with periodic boundary condition, should have type 1 too. See
Figure 10 for an example of complicated subdomain boundaries with walls and holes.

Otherwise, i.e., you delegate the setting of the array sdoms(2,D,N) to oh3 init(),
it is assumed that you also delegate the setting of bounds(:,:,:).  In this case,
oh3 init() gives the type 1 to internal boundaries, while external boundaries of the
whole space domain will have corresponding types specified by bcond(:,:) as shown
in Figure 11.

ftypes(7,F+1) should be an array whose elements ftypes(1:7,f) should have the fol-
lowings to specify the field-array associated to grid points in a subdomain and iden-
tified by the integer f ∈[1, F], while ftypes(1,F+1) should be 0 (or less) to tell
oh3 init() that you have F types of arrays.

ftypes(1,f) is the number of elements associated to a grid point of a type f field-
array. For example, if f is for electromagnetic field array namely eb(6,:,:,:,2)
whose first dimension is for three electric and three magnetic field vector com-
ponents, ftypes(1,f) should be 6.


<!-- Page 53 -->

bcond(1,1) bcond[0][0]
bcond(2,1) bcond[0][1]
bcond(1,2) bcond[1][0]
bcond(2,2) bcond[1][1]





Figure 11: Default setting of subdomain boundaries.


ftypes(2:3,f) defines lower (2) and upper (3) extensions el(f) and eu(f) required
for the type f field-array, besides extensions for communication. That is, for a
subdomain of [0, σ1−1] × · · · × [0, σD−1], the array for f is at least as large as;

(el(f):σ1+eu(f)−1,  . . . ,el(f):σD+eu(f)−1)

Note that if the field-arrays of type f do not need such non-communicational
extensions, you should let el(f) = eu(f) = 0.
ftypes(4:5,f) defines lower (4) and upper (5) extensions ebl(f) and ebu(f) for the
broadcast of the type f field-array. For example, for your electromagnetic filed
eb(6,:,:,:,2) of type f for a subdomain of [0, σx−1] × [0, σy−1] × [0, σz−1],
oh3_bcast_field() sends elements in the range8;
from eb(1,ebl(f),ebl(f),ebl(f),1)
to eb(6,σx+ebu(f)−1,σy+ebu(f)−1,σz+ebu(f)−1,1)
to the helpers of the local node(Figure 12). Note that if the field-arrays of type
f are never broadcasted, you should let ebl(f) = ebu(f) = 0.
ftypes(6:7,f) defines lower (6) and upper (7) extensions erl (f) and eru(f) for the
reduction of the type f field-array. For example, for your current density array
of type f namely cd(3,:,:,:,2) for a subdomain of [0, σx−1] × [0, σy−1] ×
[0, σz−1], oh3_allreduce_field() or oh3_reduce_field() performs the re-
duction of the elements in the range9;
from cd(1,erl (f),erl (f),erl (f),1)
to cd(3,σx+eru(f)−1,σy+eru(f)−1,σz+eru(f)−1,1)
to have the sum in the primary family of the local node. Note that if you will
never perform reductions on the field-arrays of type f, you should let erl (f) =
eru(f) = 0.

cfields(C+1) should be an array whose element cfields(c) has f ∈[1, F] to identify a
field-array type for which a type of boundary communication identified by the integer
c ∈[1, C] is defined, while ctypes(C+1) should be 0 (or less) to tell oh3 init() that
you have C types of boundary communications.

This array implies that a field-array may have two or more boundary communication
types according to the timing of the communication, or no boundary communication
may be taken for the field-array.

8Not the subarray eb(:,ebl (f):σx+ebu(f)−1, ebl (f):σy+ebu(f)−1, ebl (f):σz+ebu(f)−1,1).
9Not the subarray cd(:,erl (f):σx+eru(f)−1, erl (f):σy+eru(f)−1, erl (f):σz+eru(f)−1,1).


<!-- Page 54 -->

σ − σ −


( 0, 0 )




Figure 12: Type f field-array of (σx +5)×(σy +5) for a subdomain of [0, σx−1]×[0, σy−1]
and its elements (painted) broadcasted by oh3_bcast_field() with setting of ebl(f) = −1
and ebu(f) = 2.





primary subdomain of local node                      received from x-neighbors

required by local node                                sent to x-neighbors

required by west/east neighbors                       received from y-neighbors

required by south/north neighbors                     sent to y-neighbors
(a)                                                                                                        (b  )

Figure 13:  Field-array with downward communication (ef, et, s) = (0, 0, 2) and upward
communication (ef, et, s) = (−1, −1, 1).


<!-- Page 55 -->

sent to x-neighbors

received from x-neighbors

sent to y-neighbors

received from y-neighbors


Figure 14: Field-array with downward communication (ef, et, s) = (−1, 2, 3) and upward
communication (ef, et, s) = (−1, −4, 3).


ctypes(3,2,B,C) should be an array whose element ctypes(1:3,w,b,c)=(/ef,et,s/)
defines downward (w = 1) or upward (w = 2) boundary communication through the
boundary of type b ∈[1, B] for a field-array f = cfields(c) of the subdomain of
[0, σ1−1] × . . . [0, σD−1] as follows (Figure 13).

- Downward (w = 1) communication along d-th dimensional axis is the pair of
sending s planes perpendicular to the axis to the lower neighbor and receiving the
planes from the upper neighbor. The first plane to be sent has d-th dimensional
coordinate ef, while that to be received is at σd + et.
- Upward (w = 2) communication along d-th dimensional axis is the pair of send-
ing s planes perpendicular to the axis to the upper neighbor and receiving the
planes from the lower neighbor. The first plane to be sent has d-th dimensional
coordinate σd + ef, while that to be received is at et.

Therefore, when you just need sl and su planes at the lower and upper boundaries
surrouding a subdomain, ef = et = 0 and s = su for downward communication, while
ef = et = −sl and s = sl for upward communication as shown in Figure 13(b). On
the other hand, if you need these planes keeping those calculated by the local node
for, e.g., the addition of current densities at boundaries, et = ef + su and s = su for
downward communication, while ef = et + sl for upward communication, as shown
in Figure 14.

Note that if no data is transferred by downward and/or upward type c communication
through a boundary of type b, the element ctypes(3,w,b,c), i.e., s, should be set to
0.

fsizes(2,D,F) should be an array whose element fsizes(β,d,f) will have ϕld(f) (β = 1)
or ϕud(f) −1 (β = 2) for the field-arrays of type f to notify you that the field-arrays


<!-- Page 56 -->

must have the shape (ε, ϕl1(f):ϕu1(f)−1,  . . . ,ϕlD(f):ϕuD(f)−1) for the leading
D+1 dimensions, where ε = ftypes(1,f). That is, if D = 3 and your field-array for
electromagnetic field vectors eb(6,:,:,:,2) has type feb, you have to allocate the
array by the following.

allocate(eb(6,fsizes(1,1,feb):fsizes(2,1,feb),
fsizes(1,2,feb):fsizes(2,2,feb),
fsizes(1,3,feb):fsizes(2,3,feb),2))

Note that the allocation above makes the origin of subdomains eb(:,0,0,0,:).
Therefore,  if you like to define some other coordinates to the origin, for example
eb(:,1,2,3,:), you have to do the following keeping the number of elements in each
dimension.

allocate(eb(6,fsizes(1,1,feb)+1:fsizes(2,1,feb)+1,
fsizes(1,2,feb)+2:fsizes(2,2,feb)+2,
fsizes(1,3,feb)+3:fsizes(2,3,feb)+3,2))

The value of ϕld(f) and ϕud(f) are calculated by the followings to obtain the maxi-
mum extensions at lower and upper boundaries from ftypes(:,:), cfields(:) and
ctypes(:,:,:,:), and the maximum size of each subdmain edge from sdoms(:,:,:).

Γ(f) = {c | cfields(c) = f}
{
e  s ̸= 0
λ(e, s) =
0  s = 0
s↓(b, c) = ctypes(3,1,b,c)
s↑(b, c) = ctypes(3,2,b,c)
e↓f(b, c) = λ(ctypes(1,1,b,c), s↓(b, c))
e↓t (b, c) = λ(ctypes(2,1,b,c), s↓(b, c))
e↑f(b, c) = λ(ctypes(1,2,b,c), s↑(b, c))
e↑t (b, c) = λ(ctypes(2,2,b,c), s↑(b, c))
t (b, c)})            eγl (f) = b∈[1,B],c∈Γ(f)({e↓min        f(b, c)} ∪{e↑
t (b, c) + s↓(b, c)} ∪{e↑f(b, c) + s↑(b, c)})            eγu(f) = b∈[1,B],c∈Γ(f)({e↓max
ϕld(f) = min(eγl (f), el(f), ebl(f), erl (f))
emaxu  (f) = max(eγu(f), eu(f), ebu(f), eru(f))
ϕmaxd  = m∈[0,N−1]{δumax    d(m) −δld(m)}
ϕud(f) = ϕmaxd  + emaxu  (f)

For example, suppose D = 2, the subdomain decomposition is done as shown in
Figure 9 with fully periodic boundaries, and you specify the followings for your elec-
tromagnetic field array eb(6,:,:,2) with field-array type identifier feb and boundary
communication type identifier ceb.

ftypes(:,feb)=(/6, 0,0, 0,1, 0,0/)
cfields(ceb)=feb
ctypes(:,:,1,ceb)=reshape((/0,0,2, -1,-1,1/), (/3,2)/)


<!-- Page 57 -->

Then you will have the followings in fsizes(:,:,feb) to allocate the array by
allocate(eb(6,-1:7,-1:8,2)).

fsizes(1,1,feb) = min(min(0, −1), 0, 0, 0) = −1
fsizes(2,1,feb) = 6 + max(max(2, 0), 0, 1, 0) = 6 + 2 −1 = 7
fsizes(1,2,feb) = min(min(0, −1), 0, 0, 0) = −1
fsizes(2,2,feb) = 7 + max(max(2, 0), 0, 1, 0) = 7 + 2 −1 = 8


stats
repiter
verbose
See §3.4.1 because the arguments above are perfectly equivalent to those of oh1_
init().


C Interface

void oh3_init(int **sdid, int nspec, int maxfrac, int **nphgram,
int **totalp, struct S_particle **pbuf, int **pbase,
int maxlocalp, void *mycomm, int **nbor, int *pcoord,
int **sdoms, int *scoord, int nbound, int *bcond, int **bounds,
int *ftypes, int *cfields, int *ctypes, int **fsizes,
int stats, int repiter, int verbose);


sdid
nspec
maxfrac
nphgram
See §3.4.1 because the arguments above are perfectly equivalent to those of oh1_
init().

totalp
pbuf
pbase
maxlocalp
See §3.5.2 because the arguments above are perfectly equivalent to those of oh2_
init().

mycomm
nbor
pcoord
See §3.4.1 because the arguments above are perfectly equivalent to those of oh1_
init().

**sdoms should be a double pointer to an array  of N × D × 2 elements to form
sdoms[N][D][2] conceptually, or a pointer to NULL (not NULL itself) if you want the li-
brary to allocate and initialize the array and return the pointer to it through the
argument.   If you prepare the array,  its element sdoms[m][d][β] should have the
d-th (d ∈[0, D−1]) dimensional integer coordinate of the lower (β = 0) or up-
per (β = 1) boundary of the subdomain m ∈[0, N−1], namely δld(m) or δud(m)
respectively.  For example, for the 3-dimensional cuboid subdomain m whose grid


<!-- Page 58 -->

points at west-south-east and east-north-top corners are (δlx(m), δly(m), δlz(m)) and
(δux(m)−1, δuy (m)−1, δuz (m)−1), the subarray sdoms[m][ ][ ] should have the followings
(Figure 9).

sdoms[m][0][0]=δlx(m); sdoms[m][0][1]=δux(m);
sdoms[m][1][0]=δly(m); sdoms[m][1][1]=δuy (m);
sdoms[m][2][0]=δlz(m); sdoms[m][2][1]=δuz (m);

Note that if the subdomain m is the d-th dimensional lower (upper) neighbor of n
sharing a (D−1)-dimensional plane perpendicular to d-th axis (e.g., a neighbor along
x-axis sharing a yz-plane), n’s lower (upper) boundary plane has to be m’s upper
(lower) boundary plane. For example, if D = 3 and m is n’s lower neighbor along
x-axis, the following must be satisfied.

∆lx = m∈[0,N−1]{δlmin    x(m)}   ∆u                                     x = m∈[0,N−1]{δumax    x(m)}
(δlx(n) = δux(m) ∨δlx(n) = δux(m) −(∆ux−∆lx) ∨δlx(n) = δux(m) + (∆ux−∆lx)) ∧
δly(n) = δly(m) ∧δuy (n) = δuy (m) ∧δlz(n) = δlz(m) ∧δuz (n) = δuz (m)

Alternatively, if the work to define sdoms is bothersome for you, you may delegate it to
oh3 init() by passing a pointer to NULL or by making sdoms[0][0][0] > sdoms[0][0][1]
and gives the lower and upper boundaries of the whole space domain [∆l0, ∆u0−1] ×
. . . × [∆lD−1, ∆uD−1−1] through the argument array scoord[D][2] as follows.

int scoord[D][2] = {{∆l0,∆u0},. . .,{∆lD−1,∆uD−1}};

In this case, oh3 init() also refers to the argument array pcoord[D] = {Π0, . . . ,
ΠD−1} and defines sdoms[m][d][β] for m = rank(π0, . . . , πD−1) as follows.

ad = ⌊(∆ud −∆ld)/Πd⌋
kd = Πd −((∆ud −∆ld) mod Πd)                      {
∆ld + πd · ad             πd ≤kd                     sdoms[m][d][0] =
∆ld + πd · ad + (πd −kd)  πd > kd
m+d = rank(π0, . . . , πd + 1, . . . , πD−1)                      {
sdoms[m+d ][d][0]  πd < Πd −1                     sdoms[m][d][1] =
∆ud             πd = Πd −1


That is, if we have Πx subdomains along x-axis and the lower and upper boundaries
of the whole domain along x-axis are ∆lx and ∆ux, eastmost [(∆ux −∆lx) mod Πx]
subdomains have x-edges of ⌈(∆ux −∆lx)/Πx⌉while remaining western ones have x-
edges of ⌊(∆ux −∆lx)/Πx⌋. Note that the delegation of setting sdoms also means that
for the argument array bounds.

*scoord should be a pointer to an array of D × 2 to form scoord[D][2] conceptually, if
you delegate the setting of the array sdoms[N][D][2] to oh3 init(). If so, its element
scoord[d][β] should have the d-th (d ∈[0, D−1]) dimensional integer coordinate of
the lower (β = 0) or upper (β = 1) boundary of the whole space domain. That is,
scoord[D][2] should have the following for the space domain of [∆l0, ∆u0−1] × . . . ×
[∆lD−1, ∆uD−1−1].


<!-- Page 59 -->

int scoord[D][2] = {{∆l0,∆u0},. . .,{∆lD−1,∆uD−1}};

Otherwise, i.e., if you completely specify sdoms by yourself, scoord can be NULL or
the array can have any values.

nbound should be a positive integer representing the number of boundary condition types
B of the space domain.  That  is, you can specify a type of boundary condition
b ∈[0, B−1] for each boundary of the whole space domain through the argument
bcond[D][2] or of each subdomain through the argument bounds[N][D][2]. Then also
you can specify how the communication through a boundary of a specific type is
performed through the argument ctypes[C][B][2][3]. Remember that the boundary
condition type 0 is special and reserved for periodic boundaries.

*bcond should be a pointer to an array of D × 2 to form bcond[D][2] conecptually, if you
delegate the setting of the array sdoms[N][D][2] and bounds[N][D][2] to oh3 init().
If so, its element bcond[d][β] should have the type of boundary condition b ∈[0, B−1]
for the lower (β = 0) or upper (β = 1) boundary plane of the whole space domain
perpendicular to the d-th axis. Otherwise, bcond can be NULL or the array can have
any values.

**bounds should be a double pointer to an array of N × D × 2 to form bounds[N][D][2]
conceptually,  if you specify sdoms by yourself.   If so, its element bounds[m][d][β]
should have the type of boundary condition b ∈[0, B−1] for the lower (β = 0) or
upper (β = 1) boundary plane of the subdomain m perpendicular to the d-th axis.
Remember that, for a pair of adjacent subdomains, the boundary condition of the
boundary plane shared by them must have type 0, unless the plane is a special wall.
Also remember that a subdomain boundary, which is also a boundary of the whole
space domain with periodic boundary condition, should have type 0 too. See Figure 10
an example of complicated subdomain boundaries with walls and holes.

Otherwise, i.e., you delegate the setting of the array sdoms[ ][ ][ ] to oh3 init(), it
is assumed that you also delegate the setting of bounds.  In this case, oh3 init()
allocate the array of N × D × 2 and set the pointer to it to *bounds if it was NULL,
and then initialize bounds so that internal boundaries have the type 0, while external
boundaries of the whole space domain have corresponding types specified by bcond
as shown in Figure 11.

*ftypes should be a pointer to an array of (F+1) × 7 to form ftypes[F+1][7] concep-
tually.  Its element ftypes[f][ ] should have the followings to specify the field-array
associated to grid points in a subdomain and identified by the integer f ∈[0, F−1],
while ftypes[F][0] should be 0 (or less) to tell oh3 init() that you have F types of
arrays.

ftypes[f][0] is the number of elements associated to a grid point of a type f field-
array. For example, if f is for electromagnetic field array namely eb[2][][][]
of six double elements struct for three electric and three magnetic field vector
components, ftypes[f][0] should be 6.
ftypes[f][1] and ftypes[f][2] defines lower (1) and upper (2) extensions el(f)
and eu(f) required for the type f field-arrays, besides extensions for commu-
nication. That is, for a subdomain of [0, σ0−1] × · · · × [0, σD−1−1], the array
for f  is at least as large as to have grid points of [el(f), σ0+eu(f)−1] × · · · ×
[el(f), σD−1+eu(f)−1]. Note that if the field-arrays of type f do not need such
non-communicational extensions, you should let el(f) = eu(f) = 0.


<!-- Page 60 -->

ftypes[f][3] and ftypes[f][4] defines lower (3) and upper (4) extensions ebl(f)
and ebu(f) for the broadcast of the type f field-arrays. For example, for your
electromagnetic filed eb[2][][][] of type f for a subdomain of [0, σx−1] ×
[0, σy−1] × [0, σz−1], oh3_bcast_field() sends structured elements in the
range10;
from eb[0][ebl(f)][ebl(f)][ebl(f)]
to eb[0][σz+ebu(f)−1][σy+ebu(f)−1][σx+ebu(f)−1]
to the helpers of the local node (Figure 12). Note that if the field-arrays of type
f are never broadcasted, you should let ebl(f) = ebu(f) = 0.
ftypes[f][5] and ftypes[f][6] defines lower (5) and upper (6) extensions erl (f)
and eru(f) for the reduction of the type f field-array.  For example, for your
current density array of type f namely cd[2][][][] having structured elements
of three vector components for a subdomain of [0, σx−1] × [0, σy−1] × [0, σz−1],
oh3_allreduce_field() or oh3_reduce_field() performs the reduction of the
elements in the range11;
from cd[0][erl (f)][erl (f)][erl (f)]
to cd[0][σz+eru(f)−1][σy+eru(f)−1][σx+eru(f)−1]
to have the sum in the primary family of the local node. Note that if you will
never perform reductions on the field-arrays of type f, you should let erl (f) =
eru(f) = 0.

*cfields should be a pointer to an array of C+1 elements and its element cfields[c]
should have f ∈[0, F−1] to identify a field-array type for which a type of bound-
ary communication identified by the integer c ∈[0, C−1] is defined, while ctypes[C]
should be −1 (or less) to tell oh3 init() that you have C types of boundary com-
munications.

This array implies that a field-array may have two or more boundary communication
types according to the timing of the communication, or no boundary communication
may be taken for the field-array.

*ctypes should be a pointer to an array of C × B × 2 × 3 to form ctypes[C][B][2][3]
conceptually. Its elements ctypes[c][b][w][ ] = (ef, et, s) defines downward (w = 0) or
upward (w = 1) boundary communication through the boundary of type b ∈[0, B−1]
for a field-array f = cfields[c] of the subdomain of [0, σ0−1] × . . . [0, σD−1−1] as
follows (Figure 13).

- Downward (w = 0) communication along d-th dimensional axis is the pair of
sending s planes perpendicular to the axis to the lower neighbor and receiving the
planes from the upper neighbor. The first plane to be sent has d-th dimensional
coordinate ef, while that to be received is at σd + et.
- Upward (w = 1) communication along d-th dimensional axis is the pair of send-
ing s planes perpendicular to the axis to the upper neighbor and receiving the
planes from the lower neighbor. The first plane to be sent has d-th dimensional
coordinate σd + ef, while that to be received is at et.

10Not the set of structured elements
{eb[0][z][y][x] | x ∈[ebl (f), σx+ebu(f)−1], y ∈[ebl (f), σy+ebu(f)−1], z ∈[ebl (f), σz+ebu(f)−1]}
11Not the set of structured elements
{cd[0][z][y][x] | x ∈[erl (f), σx+eru(f)−1], y ∈[erl (f), σy+eru(f)−1], z ∈[erl (f), σz+eru(f)−1]}


<!-- Page 61 -->

Therefore, when you just need sl and su planes at the lower and upper boundaries
surrouding a subdomain, ef = et = 0 and s = su for downward communication, while
ef = et = −sl and s = sl for upward communication as shown in Figure 13(b). On
the other hand, if you need these planes keeping those calculated by the local node
for, e.g., the addition of current densities at boundaries, et = ef + su and s = su for
downward communication, while ef = et + sl for upward communication, as shown
in Figure 14.

Note that if no data is transferred by downward and/or upward type c communication
through a boundary of type b, the element ctypes[c][b][w][2], i.e., s, should be set to
0.

**fsizes should be a double pointer to an array of F × D × 2 to form fsizes[F][D][2]
conceptually, or a pointer to NULL (not NULL itself) if you want the library to allocate
the array and return the pointer to it through the argument. In both cases, its element
fsizes[f][d][β] will have ϕld(f) (β = 0) or ϕud(f) (β = 1) for the field-arrays of type
f to notify you that the field-arrays must have the size of (ϕuD−1(f) −ϕlD−1(f)) ×
· · · × (ϕu0(f) −ϕuD(f)) × ε for each of primary and secondary subdomains, where
ε = ftypes[f][0]. That is,  if D = 3 and your field-array for electromagnetic field
vectors eb[2][][][] of struct named ebfield has type feb, you have to allocate
the array by the following.

int (*fs)[3][2]=(int(*)[3][2])(*fsizes);
int s[3]={fs[feb][0][1]-fs[feb][0][0],
fs[feb][1][1]-fs[feb][1][0],
fs[feb][2][1]-fs[feb][2][0]};
int lext=fs[feb][0][0]+s[0]*(fs[feb][1][0]+s[1]*fs[feb][2][0]);
eb[0] = (struct ebfield*)
malloc(sizeof(struct ebfield)*s[0]*s[1]*s[2]*2) - lext;
eb[1] = eb[0] + s[0]*s[1]*s[2];

Note that the allocation above makes eb[0] and eb[1] points the origin of the subdo-
main at (0, 0, 0) in its local integer coordinate system. Therefore, if you like to make
eb[] point some other grid point, for example (1, 2, 3), you have to modify lext above
as follows.

int lext=(fs[feb][0][0]-1)+
s[0]*((fs[feb][1][0]-2)+s[1]*(fs[feb][2][0]-3));

The value of ϕld(f) and ϕud(f) are calculated by the followings to obtain the max-
imum extensions at lower and upper boundaries from ftypes[][], cfields[] and
ctypes[][][][], and the maximum size of each subdmain edge from sdoms[][][].

Γ(f) = {c | cfields[c] = f}
{
e  s ̸= 0
λ(e, s) =
0  s = 0
s↓(b, c) = ctypes[c][b][0][2]
s↑(b, c) = ctypes[c][b][1][2]
e↓f(b, c) = λ(ctypes[c][b][0][0], s↓(b, c))
e↓t (b, c) = λ(ctypes[c][b][0][1], s↓(b, c))
e↑f(b, c) = λ(ctypes[c][b][1][0], s↑(b, c))


<!-- Page 62 -->

e↑t (b, c) = λ(ctypes[c][b][1][1], s↑(b, c))
t (b, c)})                eγl (f) = b∈[0,B−1],c∈Γ(f)({e↓min         f(b, c)} ∪{e↑
t (b, c) + s↓(b, c)} ∪{e↑f(b, c) + s↑(b, c)})                eγu(f) = b∈[0,B−1],c∈Γ(f)({e↓max
ϕld(f) = min(eγl (f), el(f), ebl(f), erl (f))
emaxu  (f) = max(eγu(f), eu(f), ebu(f), eru(f))
ϕmaxd  = m∈[0,N−1]{δumax    d(m) −δld(m)}
ϕud(f) = ϕmaxd  + emaxu  (f)

For example, suppose D = 2, the subdomain decomposition is done as shown in
Figure 9 with fully periodic boundaries, and you specify the followings for your elec-
tromagnetic field array eb[2][][] with field-array type identifier feb and boundary
communication type identifier ceb.

ftypes[feb][0]=6;
ftypes[feb][1]=0; ftypes[feb][2]=0;
ftypes[feb][3]=0; ftypes[feb][4]=1;
ftypes[feb][5]=0; ftypes[feb][6]=0;
cfields[ceb]=feb;
ctypes[ceb][0][0][0]=ctypes[ceb][0][0][1]=0;
ctypes[ceb][0][0][2]=2;
ctypes[ceb][0][1][0]=ctypes[ceb][0][1][1]=-1;
ctypes[ceb][0][1][2]=1;

Then you will have the followings in fsizes[feb][][] to allocate the array of six-
element structures of (1 + 8) × (1 + 9) × 2.

fsizes[feb][0][0] = min(min(0, −1), 0, 0, 0) = −1
fsizes[feb][0][1] = 6 + max(max(2, 0), 0, 1, 0) = 6 + 2 = 8
fsizes[feb][1][0] = min(min(0, −1), 0, 0, 0) = −1
fsizes[feb][1][1] = 7 + max(max(2, 0), 0, 1, 0) = 7 + 2 = 9


stats
repiter
verbose
See §3.4.1 because the arguments above are perfectly equivalent to those of oh1_
init().

#### 3.6.2 oh13_init()

The function (subroutine) oh13 init() performs what oh3_init() does excluding the
initialization of oh2_init() but including that of oh1_init(). More specifically, let I1,
I2 and I3 be the set of initializing operations performed in oh1_init(), oh2_init() and
oh3_init() respectively, and thus I1 ⊂I2 ⊂I3.  The funtion oh13 init() performs
I3 −(I2 −I1) for those who want to have functions provided by level-3 library but to
transfer and manage particles by themselves.  Therefore oh13 init() does not allocate
the large buffer for particle transfer.  It also inhibits perticle transfer operations in oh3_


<!-- Page 63 -->

transbound() to make it almost equivalent to oh1_transbound() besides a few necessary
operations for field-arrays.
The definition I3 −(I2 −I1) of the initialization by oh13 init() is similarly applicable
to its arguments. That is, its set of arguments is A3 −(A2 −A1) ∪A1 where Ak is the
set of arguments of ohk init(). Note that two arguments rcounts and scounts of oh1_
init(), which is excluded from oh2_init() and thus also from oh3_init(), is in the set
of oh13 init().


Fortran Interface

subroutine oh13_init(sdid, nspec, maxfrac, nphgram, totalp, &
rcounts, scounts, mycomm, nbor, pcoord, &
sdoms, scoord, nbound, bcond, bounds, ftypes, &
cfields, ctypes, fsizes, &
stats, repiter, verbose)
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
integer,intent(inout) :: sdoms(:,:,:)
integer,intent(in)    :: scoord(2,OH_DIMENSION)
integer,intent(in)    :: nbound
integer,intent(in)    :: bcond(2,OH_DIMENSION)
integer,intent(inout) :: bounds(:,:,:)
integer,intent(in)    :: ftypes(:,:)
integer,intent(in)    :: cfields(:)
integer,intent(in)    :: ctypes(:,:,:,:)
integer,intent(out)   :: fsizes(:,:,:)
integer,intent(in)    :: stats
integer,intent(in)    :: repiter
integer,intent(in)    :: verbose
end subroutine


C Interface

void oh13_init(int **sdid, int nspec, int maxfrac, int **nphgram,
int **totalp, int **rcounts, int **scounts,
void *mycomm, int **nbor, int *pcoord,
int **sdoms, int *scoord, int nbound, int *bcond,
int **bounds, int *ftypes, int *cfields, int *ctypes,
int **fsizes,
int stats, int repiter, int verbose);


sdid
nspec


<!-- Page 64 -->

maxfrac
nphgram
totalp
rcounts
scounts
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
fsizes
See §3.6.1 because the arguments above are perfectly equivalent to those of oh3_
init().

stats
repiter
verbose
See §3.4.1 because the arguments above are perfectly equivalent to those of oh1_
init().

#### 3.6.3 oh3_grid_size()

The function (subroutine) oh3 grid size() is to specify the grid size of each dimension
if the real coordinate for particle locations is different from the integer coordinate for sub-
domains and field-arrays of them.  Specifically, the d-th element (d ∈[1, D] for Fortran
and d ∈[0, D−1] for C) of its sole argument size being 1-dimensional array of D elemnets
should have the scale factor γd to map integer coordinate (xi1, · · · , xiD) to (xi1·γ1, · · · , xiD·γD).

Fortran Interface

subroutine oh3_grid_size(size)
implicit none
real*8,intent(in)     :: size(OH_DIMENSION)
end subroutine


C Interface

void oh3_grid_size(double size[OH_DIMENSION]);


The grid size γd will only affect the result of oh3_map_particle_to_neighbor() or oh3_
map_particle_to_subdomain() whose return value will be m iffxd ∈[δld(m)·γd, δud(m)·γd)
for all d ∈[1, D], where xd is the argument x, y or z of the functions. Note that this func-
tion should be called just once, if necessary, after oh3_init() (or oh13_init()) is called


<!-- Page 65 -->

and before the first call of oh3_map_particle_to_neighbor() or oh3_map_particle_to_
subdomain().

#### 3.6.4 oh3_transbound()

If you initialize the library by oh3_init(), the function oh3 transbound() at first performs
the same operetaions as oh2_transbound() does; that is, examination of the balancing and
(re)building of helpand-helper configuration if necessary, followed by particle transfer. Oth-
erwise, i.e., if you have called oh13_init(), oh3 transbound() acts as oh1_transbound()
to make particle transfer schedule.  Finally, in both cases, oh3 transbound() maintains
library’s internal data structures for field-arrays of the secondary subdomain, if helpand-
helper configuration has been (re)built. For this maintenance, the function refers to the
information given to oh3_init() but not the argument arrays themselves.
Since the arguments of oh3 transbound() and its return value are perfectly equivalent
to those of oh1_transbound() (and oh2_transbound()), see §3.4.4 for their definitions.


Fortran Interface

integer function oh3_transbound(currmode, stats)
implicit none
integer,intent(in) :: currmode
integer,intent(in) :: stats
end function


C Interface

int oh3_transbound(int currmode, int stats);



#### 3.6.5 oh3_map_particle_to_neighbor()

The function oh3 map particle to neighbor() returns the identifier of the subdomain
in which the particle at given position will reside and to which the primary or secondary
subdomain of the local node adjoins. Therefore, if the particle may be in a non-neighboring
subdomain due to, for example, initial particle distribution, particle injection or particle
warp, the relative function oh3_map_particle_to_subdomain() should be used.
Although the function is faster than oh3_map_particle_to_subdomain(), it is not good
idea to use it to examine whether the particle is in the primary/secondary subdomain of the
local node, because the calling cost is not negligible. That is, it is strongly recommended
to do the examination by yourself and then call this function if you find the particle has
gone.
This function has three instances with two, three and four arguments according to the
dimension of the simulated space domain defined by D = OH_DIMENSION.


Fortran Interface

integer function oh3_map_particle_to_neighbor(x, ps)
implicit none
real*8,intent(inout) :: x
integer,intent(in)   :: ps
end function


<!-- Page 66 -->

integer function oh3_map_particle_to_neighbor(x, y, ps)
implicit none
real*8,intent(inout) :: x
real*8,intent(inout) :: y
integer,intent(in)   :: ps
end function
integer function oh3_map_particle_to_neighbor(x, y, z, ps)
implicit none
real*8,intent(inout) :: x
real*8,intent(inout) :: y
real*8,intent(inout) :: z
integer,intent(in)   :: ps
end function


C Interface

int  oh3_map_particle_to_neighbor(double *x, int ps);
int  oh3_map_particle_to_neighbor(double *x, double *y, int ps);
int  oh3_map_particle_to_neighbor(double *x, double *y, double *z, int ps);


x,y,z (for Fortran)
*x,*y,*z (for C)
These three (if D = 3) arguments should be the coordinates at which a particle is
located in Fortran, or the pointers to the variables having the coordinates in C. In
both cases, the actual argument variables may be updated as discussed later.

ps should be 0 for a primary particle, or 1 for a secondary particle.

return value is the identifier of the subdomain in which the particle will reside, or −1 if
such a subdomain is not found as discussed later.

The function at first examines whether the particle is in the primary (ps = 0) or
secondary (ps = 1) subdomain of the local node and returns its identifier if the particle is
in it, referring to the subdomain boundaries given by or set to the argument sdoms of oh3_
init(). Otherwise, it assumes that the particle has moved into a subdomain adjoining to
the primary/secondary subdomain and returns the identifier of the subdomain into which
the particle has moved, referring to the neighboring infomation given by or set to the
argument nbor of oh3_init(), or that in the helpand.
In the latter case of the boundary crossing, the periodic boundary condition of the whole
space domain is taken care of by the function.  Therefore, the coordinates given by x, y
and z should be raw ones without wraparound. Moreover, the actual argument variables
are updated by the function if the particle has crossed a periodic boundary. For example,
if the particle has crossed the periodic boundary plane perpendicular to x-axis, the actual
argument variable x is updated as follows.
{
x + (∆ux −∆lx)  x < ∆lx                      x ←
x −(∆ux −∆lx)  x ≥∆ux

On the other hand,  if the particle has crossed a non-periodic boundary of the whole
space domain, the function returns −1 to indicate that the particle is out of bounds12. To

12The values in the actual argument variables are kept unless the particle has crossed two or more con-
tacting space domain boundaries including periodic ones at once. More specifically, the function examines
boundary crossing in the order of yz, xz and then xy planes if D = 3, and updates actual argument variables
x, y and z in this order if the corresponding boundary planes are periodic.


<!-- Page 67 -->

examine the boundary condition, the function refers to the conditions given through the
argument bcond or bounds of oh3_init(). The function also returns −1 if the particle has
moved into a non-exsistent neighbor, which may be defined by nbor.

#### 3.6.6 oh3_map_particle_to_subdomain()

The function oh3 map particle to subdomain() returns the identifier of the subdomain
in which the particle at given position will reside. Unlike the relative function oh3_map_
particle_to_neighbor(), this function can find the identifier of any subdomain and thus
should be used for, e.g., initial particle disribution, particle injection, particle warp, and
so on. Of course you may use this function always but have to remember that it is slower
than oh3_map_particle_to_neighbor() especially if you specify sdoms argument of oh3_
init() by yourself.
This function has three instances with one, two and three arguments according to the
dimension of the simulated space domain defined by D = OH_DIMENSION.


Fortran Interface

integer function oh3_map_particle_to_subdomain(x)
implicit none
real*8,intent(in)  :: x
end function
integer function oh3_map_particle_to_subdomain(x, y)
implicit none
real*8,intent(in)  :: x
real*8,intent(in)  :: y
end function
integer function oh3_map_particle_to_subdomain(x, y, z)
implicit none
real*8,intent(in)  :: x
real*8,intent(in)  :: y
real*8,intent(in)  :: z
end function


C Interface

int  oh3_map_particle_to_subdomain(double x);
int  oh3_map_particle_to_subdomain(double x, double y;
int  oh3_map_particle_to_subdomain(double x, double y, double z);


x,y and z should be the coordinates at which a particle is located.

return value is the identifier of the subdomain in which the particle will reside, or −1 if
such a subdomain is not found as discussed later.

If you delegated the setting of sdoms array of oh3_init(), the function finds the sub-
domain by a simple calculation taking O(1) time which should be, however, longer than
that taken by oh3_map_particle_to_neighbor() due to an integer division. Therefore, it
is not good idea to call this function to examine whether the particle is in the primary/
secondary subdomain of the local node. That is, you should examine it by yourself and
then, if the particle has gone outside, call this function. Also note that the calculation does
not take care of the periodic boundary condition of the whole space domain, and thus you


<!-- Page 68 -->

have to perform wraparound calculation before calling this function if necessary, or you will
get the return value −1 to indicate that particle is out of bounds.
On the other hand, if you specify the array sdoms by yourself, this function searches
the target subdomain. If your space domain is a cuboid (or a rectangler or a line segment)
without any holes, the cost of search is O(log N). Otherwise, for a complicatedly shaped
domain, the cost could be O(N) although the function does its best to reduce it to O(log N).
The search may fail if there is no subdomain including the given particle coordinates due
to, for example, going outside the whole space domain, dropping into a hole, and so on, to
make the function return −1.


#### 3.6.7 oh3_bcast_field()

The function (subroutine) oh3 bcast field() performs red-black broadcast communica-
tions of a field-array whose type is specified by its argument ftype in the families the local
node belongs to. The argument pfld specifies the field-array to be broadcasted in the
primary family, while sfld is for the data to be broadcasted in the secondary family. You
may be unaware that the local node really has its primary or secondary family, because
the function will skip the primary broadcast if it is a leaf and the secondary one if it is the
root. It is neither necessary to specify the data count because it is calculated by the library,
nor to give MPI data-type to the function because MPI_DOUBLE_PRECISION for Fortran or
MPI_DOUBLE for C is assumed13.


Fortran Interface

subroutine oh3_bcast_field(pfld, sfld, ftype)
implicit none
real*8,intent(in)  :: pfld
real*8,intent(out) :: sfld
integer,intent(in) :: ftype
end subroutine


C Interface

void oh3_bcast_field(void *pfld, void *sfld, int ftype);


pfld should be (the pointer to) the first field-array element at the origin of the primary
subodmain. The contents of the field-array are broadcasted from the local node to
its helpers in its primary family.

sfld should be (the pointer to) the first field-array element at the origin of the secondary
subodmain. The broadcasted data in the secondary family is received to the field-
array.

ftype should be the identifier to specify the type of the field-array.

For example, to broadcast your electromagnetic field-array eb(6,:,:,:,2) of type
feb, you can simply do the following in your Fortran code providing the origins are
eb(:,0,0,0,:).

call oh3_bcast_field(eb(1,0,0,0,1),eb(1,0,0,0,2),feb)

13Therefore, your field-arrays should have elements only of double presision floating point data or struc-
tures only of them.


<!-- Page 69 -->

As for C field-array of struct whose origins are pointed by eb[0] and eb[1], what you
have to do is simply the following.

oh3_bcast_field(eb[0],eb[1],feb);

In order to make the interfaces simple as shown above, the function refers to ebl(f) and
ebu(f) for f = ftype given in the argument ftypes of oh3_init(), and the size of primary/
secondary subdomain given in sdoms and that of the field-array itself set to fsizes. Note
that the elements to be broadcasted are not only in the subarray defined by ebl(f) and
ebu(f) but also some of outside the subarray as shown in Figure 12 in §3.6.1 for the sake of
efficiency. This overrun should not be harmful to the logical correctness of the simulation.


#### 3.6.8 oh3_allreduce_field()

The function (subroutine) oh3 allreduce field() performs red-black all-reduce summa-
tion of a field-array whose type is specified by its argument ftype in the families the local
node belongs to. The argument pfld specifies the field-array to be reduced in the primary
family, while sfld is for the data to be reduced in the secondary family. You may be un-
aware that the local node really has its primary or secondary family, because the function
will skip the primary reduction if it is a leaf and the secondary one if it is the root. It is
neither necessary to specify the data count because it is calculated by the library, to give
MPI data-type to the function because MPI_DOUBLE_PRECISION for Fortran or MPI_DOUBLE
is assumed, nor to tells it how the reduction is done because MPI_SUM is assumed14.


Fortran Interface

subroutine oh3_allreduce_field(pfld, sfld, ftype)
implicit none
real*8,intent(inout) :: pfld
real*8,intent(inout) :: sfld
integer,intent(in)   :: ftype
end subroutine


C Interface

void oh3_allreduce_field(void *pfld, void *sfld, int ftype);


pfld should be (the pointer to) the first field-array element at the origin of the primary
subodmain. The contents of the field-array are replaced with the sum in the primary
family.

sfld should be (the pointer to) the first field-array element at the origin of the secondary
subodmain. The contents of the field-array are replaced with the sum in the secondary
family.

ftype should be the identifier to specify the type of the field-array.

For example, to have the sum of your current density field-array cd(3,:,:,:,2) of
type fcd, you can simply do the following in your Fortran code providing the origins are
cd(:,0,0,0,:).

14Therefore, the function cannot be used for any other reductions than summing up.


<!-- Page 70 -->

call oh3_allreduce_field(cd(1,0,0,0,1),cd(1,0,0,0,2),fcd)

As for C field-array of struct whose origins are pointed by cd[0] and cd[1], what you
have to do is simply the following.

oh3_allreduce_field(cd[0],cd[1],fcd);
In order to make the interfaces simple as shown above, the function refers to erl (f) and
eru(f) for f = ftype given in the argument ftypes of oh3_init(), and the size of primary/
secondary subdomain given in sdoms and that of the field-array itself set to fsizes. Note
that the elements to be reduced are not only in the subarray defined by erl (f) and eru(f) but
also some of outside the subarray as shown in Figure 12 in §3.6.1 for the sake of efficiency.
This overrun should not be harmful to the logical correctness of the simulation.

#### 3.6.9 oh3_reduce_field()

The function (subroutine) oh3 reduce field() performs red-black one-way counterpart of
the function oh3_allreduce_field().

Fortran Interface
subroutine oh3_reduce_field(pfld, sfld, ftype)
implicit none
real*8,intent(inout) :: pfld
real*8,intent(in)    :: sfld
integer,intent(in)   :: ftype
end subroutine


C Interface

void oh3_reduce_field(void *pfld, void *sfld, int ftype);


pfld should be (the pointer to) the first field-array element at the origin of the primary
subodmain. The contents of the field-array are replaced with the sum in the primary
family.

sfld should be (the pointer to) the first field-array element at the origin of the secondary
subodmain. The contents of the field-array remain unchanged.

ftype should be the identifier to specify the type of the field-array.

#### 3.6.10 oh3_exchange_borders()

The function (subroutine) oh3 exchange borders() exchanges boundary planes of a field-
array between adjacent primary subdomains. Then, if specified to do, the boundary planes
are broadcasted from the local node to its helpers.


Fortran Interface
subroutine oh3_exchange_borders(pfld, sfld, ctype, bcast)
implicit none
real*8,intent(inout) :: pfld
real*8,intent(out)   :: sfld
integer,intent(in)   :: ctype
integer,intent(in)   :: bcast
end subroutine


<!-- Page 71 -->

C Interface

void oh3_reduce_field(void *pfld, void *sfld, int ctype, int bcast);


pfld should be (the pointer to) the first field-array element at the origin of the primary
subodmain.  The boundary planes (or line segments) of the field-array are sent/
received to/from the nodes which are responsible for the subdomains adjoining to the
primary subdomain of the local node as their primary ones.

sfld should be (the pointer to) the first field-array element at the origin of the secondary
subodmain. The boundary planes of the field-array are replaced with that in the
helpand of the local node, if bcast is non-zero and we are in secondary mode.

ctype should be the identifier to specify the type of the field-array comunication, which is
an index of ctypes of oh3_init().

bcast should be non-zero to broadcast obtained boundary planes to the helpers. If it is 0,
only the boundary exchange of the primary subdomain is performed. Note that if we
are in primary mode, the broadcast is not performed even if bcast ̸= 0.

For example, you can simply do the following in your Fortran code to exchange bound-
ary data of your electromagnetic field-array eb(6,:,:,:,2) of communication type ceb,
providing the origins are eb(:,0,0,0,:) and you do not want to broadcast the received
boundary planes.

call oh3_exchange_borders(eb(1,0,0,0,1),eb(1,0,0,0,2),ceb,0)

As for C field-array of struct whose origins are pointed by eb[0] and eb[1], what you
have to do is simply the following.

oh3_exchange_borders(eb[0],eb[1],ceb,0);

By these simple statements, you can achieve fairly complicated communications as shown
in Figure 13 of Sectin 3.6.1 because oh3 exchange borders() takes care of various matters.
First, it of course follows the specifications of the number of planes and their sources and
destinations in the field-array given through the argument ctypes of oh3_init(). The
specifiations are also used to determine the size of a plane depending on the axis along
which a communication is taken place. That is, the function enlarges the planes to be
exchanged as it proceeds the communication from along x-axis then y and to z-axis, so
that the local node obtains boundary data not only from the subdomains contacted with
planes but also with edges and vertices as shown in Figure 13. Finally, to have the shape
of the set of planes to be transferred and to represent them with a derivative data type of
MPI, the function consults the size of primary/secondary subdomain given in sdoms and
that of the field-array itself set to fsizes.
The finely designed boundary communication above is especially helpful for more com-
plicated communications required to have the sum of current densities of a grid point around
a vertices connecting subdomains. As shown in Figure 14 of §3.6.1, you can have 3D partial
sums calculated by 3D families by a simple definition in ctypes and the following simple
call in Fortran, providing your current density field-array is cd(3,:,:,:,2) and its type is
ccd.

call oh3_exchange_borders(cd(1,0,0,0,1),cd(1,0,0,0,2),ccd,1)
