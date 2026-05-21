# 4.5 C Source File ohhelp2.c - Part 2

Source: `doc/original/ohhelp.pdf`, pages 266-269.

<!-- Page 266 -->

NOfPLocal[(ns+s)*nn+n]++;
InjectedParticles[ns+s]++;
} else {
NOfPLocal[nn*s+n]++;
if (n==myRank)  InjectedParticles[s]++;
}
}


#### 4.5.20 oh2_remap_injected_particle()

oh2_remap_injected_particle_()  The API  function oh2_remap_injected_particle_()  for  Fortran and oh2_remap_
oh2_remap_injected_particle()  injected_particle() for C maintain NOfPLocal[p][s][m] and InjectedParticles[0][p][s]
of the particle π pointed by the sole argument part, which has been injected by oh2_
inject_particle() with negative nid element or has been removed by oh2_remove_
injected_particle(), where m = π.nid, s is the species of π obtained by Particle_
Spec(), and p ∈{0, 1} is 1 iffm = parent(n) for the local node n.
At first the function checks if

Particles + Qn ≤&π < Particles + Qn + Qinjn

i.e., π in at a location for injected particles, and aborts the execution by local_errstop()
if unsatisfied.  Then we do the following as done in oh2_inject_particle(); check
if OH_HAS_SPEC  is defined or S = 1 and abort the execution  if both are unsatisfied;
increment NOfPLocal[p][s][m]  if m ≥0; and increment InjectedParticles[0][p][s]  if
m ∈{n, parent(n)}.


void
oh2_remap_injected_particle_(struct S_particle *part) {
oh2_remap_injected_particle(part);
}
void
oh2_remap_injected_particle(struct S_particle *part) {
const int pidx = part - Particles, ns=nOfSpecies, nn=nOfNodes;
int s, n;

if (pidx<totalParts || pidx>=totalParts+nOfInjections)
local_errstop("’part’ argument pointing %c%d%c of the particle buffer is "\
"not for injected particles",
specBase?’(’:’[’, pidx+specBase, specBase?’)’:’]’);
#ifndef OH_HAS_SPEC
if (ns!=1)
local_errstop("particles cannot be injected when S_particle does not "
"have ’spec’ element and you have two or more species");
#endif
s = Particle_Spec(part->spec - specBase);
n = part->nid;
if (n<0)  return;
if (n==RegionId[1]) {
NOfPLocal[(ns+s)*nn+n]++;
InjectedParticles[ns+s]++;
} else {
NOfPLocal[nn*s+n]++;


<!-- Page 267 -->

if (n==myRank)  InjectedParticles[s]++;
}
}


#### 4.5.21 oh2_remove_injected_particle()

oh2_remove_injected_particle_()  The API function oh2_remove_injected_particle_()  for Fortran and oh2_remove_
oh2_remove_injected_particle()  injected_particle() for C remove the particle π pointed by the sole argument part
which has been injected by oh2_inject_particle(), maintaining NOfPLocal[p][s][m] and
InjectedParticles[0][p][s] of the particle pointed by part = π, where m = π.nid, s is the
species of π obtained by Particle_Spec(), and p ∈{0, 1} is 1 iffm = parent(n) for the
local node n.
The functions performs the following similarly to oh2_remap_injected_particle() but
opposite way in the maintaiance of NOfPLocal[][][] and InjectedParticles[0][][]; check if π
is at a location for injected particles, and aborts the execution by local_errstop() if un-
satisfied; check if OH_HAS_SPEC is defined or S = 1 and abort the execution if both are unsat-
isfied; decrement NOfPLocal[p][s][m] if m ≥0; and decrement InjectedParticles[0][p][s]
if m ∈{n, parent(n)}. Finally, π.nid is let be −1 for removal.


void
oh2_remove_injected_particle_(struct S_particle *part) {
oh2_remove_injected_particle(part);
}
void
oh2_remove_injected_particle(struct S_particle *part) {
const int pidx = part - Particles, ns=nOfSpecies, nn=nOfNodes;
int s, n;

if (pidx<totalParts || pidx>=totalParts+nOfInjections)
local_errstop("’part’ argument pointing %c%d%c of the particle buffer is "\
"not for injected particles",
specBase?’(’:’[’, pidx+specBase, specBase?’)’:’]’);
#ifndef OH_HAS_SPEC
if (ns!=1)
local_errstop("particles cannot be injected when S_particle does not "
"have ’spec’ element and you have two or more species");
#endif
s = Particle_Spec(part->spec - specBase);
n = part->nid;
if (n<0)  return;
if (n==RegionId[1]) {
NOfPLocal[(ns+s)*nn+n]--;
InjectedParticles[ns+s]--;
} else {
NOfPLocal[nn*s+n]--;
if (n==myRank)  InjectedParticles[s]--;
}
part->nid = -1;
}


<!-- Page 268 -->

#### 4.5.22 oh2_set_total_particles()

oh2_set_total_particles_()  The API  function oh2_set_total_particles_()  for  Fortran and oh2_set_total_
oh2_set_total_particles()  particles() for C tell the library that the simulator body has initialized Particles[] and
NOfPLocal[][][] but it will modify them for particle injection/removal before the first call
of oh2_transbound(). This means that the library should grasp the layout in Particles[]
through NOfPLocal[][][] to  initialize substance variables TotalP[][], primaryParts and
totalParts.  Since this initialization is usually done in the first call of transbound1()
by set_total_particles(), the API functions simply call it.


void
oh2_set_total_particles_() {
set_total_particles();
}
void
oh2_set_total_particles() {
set_total_particles();
}


#### 4.5.23 oh2_max_local_particles()

oh2_max_local_particles_()  The API  function oh2_max_local_particles_()  for  Fortran and oh2_max_local_
oh2_max_local_particles()  particles() for C calculates the maximum number of particles a local node can accom-
modate and returns it to a simulator body calling them. The function takes the following
arguments.

- npmax = P limG   is the absolute maximum number of particles which the simulator is
capable of as a whole.

- maxfrac = α is the tolerance factor percentage of load imbalance and should be same
as the argument maxfrac of oh2_init().

- minmargin = ∆is the minimum margin by which the return value Plim has to clear
over the per node average of npmax.

Prior to calculating Plim by;
⌈               ⌉
P = ⌈Plim/N⌉G          Plim = max( P(100 + α)/100  , P + ∆)

the function obtains N by MPI_Comm_size() and confirms P limG > 0 and 0 < α ≤100 are
satisfied or aborts the execution by errstop(). It also confirms Plim ≤INT_MAX or aborts
the execution by mem_alloc_error().


int
oh2_max_local_particles_(dint *npmax, int *maxfrac, int *minmargin) {
return(oh2_max_local_particles(*npmax, *maxfrac, *minmargin));
}
int
oh2_max_local_particles(dint npmax, int maxfrac, int minmargin) {
int nn, nplint;
dint npl, npmargin;

MPI_Comm_size(MCW, &nn);


<!-- Page 269 -->

if (npmax<=0) errstop("max # of particles should be greater than 0");
if (maxfrac<=0 || maxfrac>100)
errstop("load imbalance factor (%d) should be in the range [1..100]",
maxfrac);
npl = (npmax-1)/nn + 1; /* ceil(npmax/nn) */
npmargin = (npl*maxfrac-1)/100 + 1;
npl += (npmargin<minmargin) ? minmargin : npmargin;
if (npl>INT_MAX) mem_alloc_error("Particles", 0);
nplint = npl;
return(nplint);
}
