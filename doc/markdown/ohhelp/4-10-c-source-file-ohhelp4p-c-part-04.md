# 4.10 C Source File ohhelp4p.c - Part 4

Source: `doc/original/ohhelp.pdf`, pages 436-439.

<!-- Page 436 -->

main code of nid be {k, m+3D} + N + 3D  if p = 1 by Secondarize_Id() to tell its
secondariness to the functions called from transbound4p().
Finally we return to the the caller giving m as the return value.

Local_Coordinate(sd, mysd, gx, lx, OH_DIM_X, k, 1, aacc);
Do_Y(Local_Coordinate(sd, mysd, gy, ly, OH_DIM_Y, k, 3, aacc));
Do_Z(Local_Coordinate(sd, mysd, gz, lz, OH_DIM_Z, k, 9, aacc));
NOfPLocal[t*nOfNodes+sd]++;
if (aacc) {
currMode = Mode_Set_Any(currMode);
part->nid = Combine_Subdom_Pos(sd+OH_NEIGHBORS,
Coord_To_Index(gx, gy, gz, w, dw));
} else {
NOfPGrid[ps][s][Coord_To_Index(lx, ly, lz, w, dw)]++;
part->nid = Combine_Subdom_Pos(k, Coord_To_Index(gx, gy, gz, w, dw));
}
if (inj) {
if (sd==mysd)  InjectedParticles[t]++;
if (ps)  Secondarize_Id(part);
}
return(sd);
}


#### 4.10.52 oh4p_inject_particle()

oh4p_inject_particle_()  The API functions oh4p_inject_particle_() for Fortran and oh4p_inject_particle()
oh4p_inject_particle()  for C inject a particle pointed by part = π as a primary (ps = p = 0) or secondary (p = 1)
one for the local node expecting (or knowing) it resides in the local node n’s primary/
secondary subdomain.
The differences of them from their counterparts oh2_inject_particle[_]() are as
follows. First they have ps = p argument to specify the subdomain in which the particle
likely resides, while the level-2 counterparts guess that from its nid element. Second, they
determine the subdomain m in which the particle resides by themselves calling oh4p_map_
particle_to_neighbor() which also maintains NOfPLocal[][][] and NOfPGrid[][][] for the
particle, rather than expecting nid has the subdomain identifier. Finally, they have a return
value m so that the caller is aware that the particle is out-of-bounds if so.
The function oh4p_inject_particle_() simply calls its counterpart oh4p_inject_
particle() which does everything.


int
oh4p_inject_particle_(const struct S_particle *part, const int *ps) {
return(oh4p_inject_particle(part, *ps));
}
int
oh4p_inject_particle(const struct S_particle *part, const int ps) {
const int ns = nOfSpecies;
int inj = totalParts + nOfInjections++;
struct S_particle *p = Particles + inj;
int s = Particle_Spec(part->spec - specBase);
int sd;


<!-- Page 437 -->

In the declaration part, we determine the location of the particle  is stored,  i.e.,
Particles[Qn + Qinjn  ] where Qn = totalParts and Qinjn = nOfInjections, increment
Qinjn  to keep track the total number of injected particles, and have the species s of the
particle by Particle_Spec() taking its origin specBase into account.
Then we confirm S_particle structure has spec element, i.e., OH_HAS_SPEC is true, or
S = 1, or abort the execution by local_errstop(). The abortion also takes place if the
total number of the accommodating particles including that just now injected exceeds the
absolute limit Plim = nOfLocalPLimit.
Then we store the particle into the location above and call oh4p_map_particle_to_
neighbor() to obtain the subdomain identifier m, which could be less than 0 to cause can-
celing the injection. Note that oh4p_map_particle_to_neighbor() recognizes the particle
is injected because we give it the location beyond Particles[totalParts] and thus main-
tains InjectedParticles[][][] and secondarize the particle if necessary.
Finally we return to the the caller giving m as the return value.

#ifndef OH_HAS_SPEC
if (ns!=1)
local_errstop("particles cannot be injected when S_particle does not "
"have ’spec’ element and you have two or more species");
#endif
if (inj>=nOfLocalPLimit)
local_errstop("injection causes local particle buffer overflow");
*p = *part;
sd = oh4p_map_particle_to_neighbor(p, ps, s);
if (sd<0)  nOfInjections--;
return(sd);
}


#### 4.10.53 oh4p_remove_mapped_particle()

oh4p_remove_mapped_particle_()  The API functions oh4p_remove_mapped_particle_() for Fortran and oh4p_remove_
oh4p_remove_mapped_particle()  mapped_particle() for C eliminate a primary (ps = p = 0) or secondary (p = 1) particle
pointed by part = π of species s = s, which has already been mapped on to a subdomain
by oh4p_map_particle_to_neighbor(), oh4p_map_particle_to_subdomain() or oh4p_
inject_particle(). This explicit elmination is required to maintain NOfPLocal[][][] and
NOfPGrid[][][] in order to reflect the elimination to them.
First, we invoke Check_Particle_Location() to check the consistency of arguments
giving the fifth argument i determined by whether π is beyond Particles+nOfInjections,
i.e., π is for an injected particle. Then we examine if the π’s nid is negative and return
to the caller doing nothing  if so.  Next we obtain the subdomain identifier m of π by
Subdomain_Id() and examine if m ≥N. If so to mean the particle is an injected secondary
one, we get real m by Primarize_Id() and force p = 1.
Then we mark π eliminated by letting its nid be −1 and decrement NOfPLocal[p][s][m].
Then if the particle is injected into the local node n’s primary/secondary subdomain, i.e.,
m = n′ = {n, parent(n)}[p] = RegionId[p], we decrement InjectedParticles[0][p][s] to
cancel the increment on the injection.  Finally, if we have normal accommodation so far,
i.e., Mode_Acc() of currMode is false, we also decrement NOfPGrid[p][s][g′], where g′ = g if
m = n′ or otherwise g′ is what Local_Grid_Position() gives us with g, nid of (primarized)
π and p, to cancel the increment done when π was mapped.


<!-- Page 438 -->

void
oh4p_remove_mapped_particle_(struct S_particle *part, const int *ps,
const int *s) {
oh4p_remove_mapped_particle(part, *ps, *s-1);
}
void
oh4p_remove_mapped_particle(struct S_particle *part, const int ps,
const int s) {
const int nn = nOfNodes, ns = nOfSpecies, inj = part>=Particles+totalParts;
OH_nid_t nid = part->nid;
int sd, g, psreal=ps, mysd, t;
Decl_Grid_Info();

Check_Particle_Location(part, psreal, s, ns, inj);
if (nid<0)  return;
sd = Subdomain_Id(nid, psreal);
g = Grid_Position(nid);
if (sd>=nn) {
psreal = 1;  Primarize_Id(part, sd);  nid = part->nid;
}
mysd = RegionId[psreal];
part->nid = -1;
t = psreal ? ns+s : s;
NOfPLocal[t*nn+sd]--;
if (inj && sd==mysd)  InjectedParticles[t]--;
if (Mode_Acc(currMode))  return;
if (sd!=mysd)  g = Local_Grid_Position(g, nid, psreal);
NOfPGrid[psreal][s][g]--;
}


#### 4.10.54 oh4p_remap_particle_to_neighbor()

oh4p_remap_particle_to_neighbor_()  The API functions oh4p_remap_particle_to_neighbor_() for Fortran and oh4p_remap_
oh4p_remap_particle_to_neighbor()  particle_to_neighbor() do what the functions oh4p_remove_mapped_particle() and
oh4p_map_particle_to_neighbor() do in series, giving arguments part, ps and s to both
of them.


int
oh4p_remap_particle_to_neighbor_(struct S_particle *part, const int *ps,
const int *s) {
return(oh4p_remap_particle_to_neighbor(part, *ps, *s-1));
}
int
oh4p_remap_particle_to_neighbor(struct S_particle *part, const int ps,
const int s) {
oh4p_remove_mapped_particle(part, ps, s);
return(oh4p_map_particle_to_neighbor(part, ps, s));
}


<!-- Page 439 -->

#### 4.10.55 oh4p_remap_particle_to_subdomain()

oh4p_remap_particle_to_subdomain_()  The API functions oh4p_remap_particle_to_subdomain_() for Fortran and oh4p_remap_
oh4p_remap_particle_to_subdomain()  particle_to_subdomain() do what the functions oh4p_remove_mapped_particle() and
oh4p_map_particle_to_subdomain() do in series, giving arguments part, ps and s to
both of them.


int
oh4p_remap_particle_to_subdomain_(struct S_particle *part, const int *ps,
const int *s) {
return(oh4p_remap_particle_to_subdomain(part, *ps, *s-1));
}
int
oh4p_remap_particle_to_subdomain(struct S_particle *part, const int ps,
const int s) {
oh4p_remove_mapped_particle(part, ps, s);
return(oh4p_map_particle_to_subdomain(part, ps, s));
}
