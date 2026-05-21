# 4.11 Level-4s Library Overview

Source: `doc/original/ohhelp.pdf`, pages 440-441.

<!-- Page 440 -->

## 4.11 Level-4s Library Overview

The level-4s library is an extension of OhHelp for yet another position-aware particle man-
agement for SPH (Smoothed Particle Hydrodynamics) method. In the SPH method, the
computations on a particle require attributes of other particles surrounding it. This means
that the computations on particles in a voxel may refer to particles in voxels surrounding
it, and thus a node responsible of a set of voxels must have particles in voxels surrounding
the voxel set.
This requirement makes it tough to implement the level-4s library as an extension of
the level-4p counterpart due to the followings.  First, we cannot split the particle set in
a hot-spot because the whole set must be accommodated by the node for the voxel and
possibly by other nodes for the voxels surrounding it. Second, in the level-4p the shape
of the voxel set for a node is not always a cuboid to bring a severe complication into the
communication to let the node have particles in voxels surrounding the set.
For the level-4s library, these difficulties are eased by introducing the consept of the
maximum density being the maximum number D of particles in a voxel. Since SPH simula-
tions of incompressible flow obviously has a certain upper bound of the particle density and
we can expect some upper bound even in those of compressible one, we exploit the maxi-
mum density to make us free from the hot-spot problem. Moreover, the maximum density
allows us to make the unit of subdomain splitting larger than a voxel because we have a
certain upper bound number of particles in a unit set of voxels. More specifically, the unit
in level-4s library is a xy-plane of a subdomain so that the set of voxels for a node is always
a cuboid, which we refer to as (primary/secondary) subcuboid. Though this causes that
the excess of the particle population for a node from what the OhHelp balancer suggests is
significantly large, up to D·δx(n)·δy(n) for a node having a subdomain n, the inter-process
communication of halo particles in the voxels at interior/exterior surface of the subcuboid,
or in other words in halo planes consisting interior halo planes and exterior halo planes
having particles to be sent and received respectively, can be implemented relatively easily.
Despite the level-4s’s own features shown above, its implementation shares many aspects
with the level-4p counterparts. Its outline is shown below emphasizing its similarity to and
difference from the level-4p library.

1. The  functions  to map  particles  to subdomains  such  as oh4s_map_particle_
to_neighbor() and  particle injection/removal functions such as oh4s_inject_
particle() are perfectly equivalent their level-4p counterparts, oh4p_map_particle_
to_neighbor(), oh4p_inject_particle() and so forth.

2. The  function transbound4s() and  its  direct  callee  functions try_stable4s()
and rebalance4s()  are very  similar  to  their  level-4s  counterparts,  but try_
primary4s()  is  completely  different  from  try_primary4p()  because  it  calls
exchange_particles4s() for primary particle transfer and sorting instead of having
its own mechanism. This difference comes from the functionality to exchange halo
particles between nodes for subdomains neighboring to the local node’s primary (and
secondary in general) subdomains.

3. When the complete per-grid histogram PT (p, s, g) = NOfPGridTotal[p][s][g] is built in
exchange_population(), the function also builds the per-plane histogram PZ(z) =  ∑ ∑
s   x,y PT (0, s, gidx(x, y, z)) for each xy-plane at z in the primary subdomain of
the local node. This histogram is scanned by make_recv_list() when we will be
in secondary mode in the next step so that each member node of the local node
n’s primary family is assigned particles in a subcuboid and the population in it is


<!-- Page 441 -->

approximately equal to that the OhHelp load balancing mechanism requires. That
is, make_recv_list() determines ζβp (m) being the local z-coordinate of the lower
(β = 0) or upper (β = 1) surface of the primary (p = 0) or secondary (p = 1)
subcuboid for all m ∈F(n) where p = 0 for m = n and p = 1 for others, and record
them in primary receiving block.

4. Based on the assignment above and that received from the helpand, or the obvious
assignment for primary mode execution, make_send_sched() decides the destination
of each particle in the local node in a manner similar to (but simpler than because of
no hot-spots) that of its level-4p counterpart. In addition, the function also performs
the level-4s’s own procedure to build the sending/receiving schedule of particles in
horizontal halo planes for nodes whose subcuboids are below/above the local node’s
subcuboids.

5. After exchanging the sending/receiving particle amount by exchange_xfer_amount()
as done in level-4p, non-halo particles are transferred and sorted by move_and_sort(),
xfer_particles() and sort_received_particles() if SendBuf[] can accommodate
particles both to be sent and to reside in the next step and helpand-helper reconfigu-
ration does not take place. Otherwise, move_to_sendbuf_4s(), xfer_particles()
and sort_particles() perform those operations. A differences from level-4p is that
move_and_sort() and move_to_sendbuf_4s() are commonly used for both primary
and secondary mode cases because their caller exchange_particles4s() is commonly
used too. The other and more important difference is that a level-4s’s own function
make_bxfer_sched() is called just before move_and_sort() or sort_particles()
to build the sending/receiving schedule of particles in vertical halo planes for neighbor
nodes having contacting subcuboids which share a vertical surface parallel to z-axis
with the local node’s primary/secondary subcuboid. The sending schedule is referred
to by move_and_sort(), sort_particles() and sort_received_particles() to
move particles in vertical interior halo planes parallel to z-axis, to the newly intro-
duced sending buffer BoundarySendBuf[] being the sequence of hbuf sv(d, p, β, m) for
primary(p = 0) or secondary (p = 1) particles to be sent to the node m responsible of
the d-th dimensional (d ∈{0, 1}) lower (β = 0, west or south) or upper (β = 1, east
or north) neighbor subdomain of the local node’s primary/secondary one.

6. After the particle transfer communication taken by xfer_particles() to have all
non-halo particles to be accommodated by the local node in SendBuf[] as done in level-
4p, xfer_boundary_particles_v() sends particles in vertical interior halo planes to
neighbors via hbuf sv(d, p, β, m) in BoundarySendBuf[] and then receives halo particles
into vertical exterior halo planes parallel to z-axis and just outside of thier interior
counterparts via hbuf rv(d, p, β, m) in Particles[]. The function works twice with d =
0 for west/east-bound communication and then with d = 1 for south/north-bound, the
latter of which carries halo particles in neighbor subdomains which contact with the
local one by edges to make the direct communications with nodes for the subdomains
unnecessary. Then xfer_boundary_particles_h() performs halo particle transfer
dierectly from horizontal interior halo planes in SendBuf[] namely hbuf sh(p, β, s) being
the bottommost (β = 0) and topmost (β = 1) xy-planes of the local node’s subcuboid,
and directly to horizontal exterior halo planes in SendBuf[] namely hbuf rh(p, β, s)
just below (β = 0) and above (β = 1) the interior counterparts. Since this carries
halo particles in neighbor subdomains contacting with the local one by vertices, it is
unnecessary to communicate the nodes for those subdomains directly too.
