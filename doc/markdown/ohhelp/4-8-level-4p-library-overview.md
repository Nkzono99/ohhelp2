# 4.8 Level-4p Library Overview

Source: `doc/original/ohhelp.pdf`, pages 323-324.

<!-- Page 323 -->

## 4.8 Level-4p Library Overview

The level-4p library is an extension of OhHelp for position-aware particle management so
that particles in a particular grid-voxel are accommodated by a particular node responsible
of the primary/secondary subdomain including the grid-voxel, as long as it is not so con-
gested. Moreover, particles in each pbuf (p, s) are sorted by their grid-positions, being the
one-dimensional indices of their resident grid-voxels of (conceptual) D-dimensional array,
so that the simulator body captures particles in a particular grid-voxel with the help of
per-grid histogram of particle population provided by the level-4p library function oh4p_
transbound().
The fundamental mechanism of position-aware particle management is fairly simple as
summarized below.

1. Simulator body calls oh4p_map_particle_to_neighbor() or oh4p_map_particle_
to_subdomain() for each primary (p = 0) or secondary (p = 1) particle of species s at
its move to a grid-position g so that the library accumulates the population in the grid-
voxel at g in the local per-grid histogram entry namely PL(p, s, g) = NOfPGrid[p][s][g].

2. In oh4p_transbound(), the per-grid histograms in a helpand-helper family are ac-
cumulated and then its entries at boundary planes are exchanged between neigh-
bors by exchange_population() to have the complete per-grid histogram named
PT (p, s, g) = NOfPGridTotal[p][s][g] in the helpand of each subdomain.

3. Each helpand scans its per-grid histogram for its primary subdomain to assign a set
of consecutive grid-voxels and the particles resinding in them to its primary family
members by make_recv_list() so that the number of particles accommodated by
each member node is approximately equal to that the OhHelp load balancing mech-
anisim requires. Then the assignment is broadcasted to helpers, exchanged between
neighboring helpands and then broadcasted again to helpers of neighbors so that they
recognize the destination of each particle they accommodate by make_send_sched().
The sending and receiving amount of particles are then exchanged by exchange_
xfer_amount() among neighboring family menbers.

4. Prior to the particle transfer communication taken by xfer_particles(), each node
scans its primary and secondary particles sorting those which will stay in the node by
move_and_sort_primary() or move_and_sort_secondary() which moves particles
from Particles[] to SendBuf[]. Then after the particle transfer, received particles
are sorted by sort_received_particles() to have completely sorted particle buffer
in SendBuf[] which becomes Particles[] in the next simulation step exchanging their
roles.

The real implementaion is, however, a little bit more complicated due to various subjects
summarized as follows.


primary mode   If we will be in primary mode in the next simulation step, the particle
transfer is simpler than above because all particles in a subdomain will be sent to the node
responsible of the subdomain as primary one.


particle sorting  The sorting prior to the particle transfer explained above minimizes
the number of scans of Particles[], i.e., we need just one scan. However it requires that
SendBuf[] have the space large enough for both the particles accommodated in the next
simulation step and those to be sent to other nodes. Since it is not ensured that SendBuf[]


<!-- Page 324 -->

is large enough though likely, we need to reverse the order of the sorting and transfer if
insufficient. That is, we have to transfer particles in non-position-aware manner to have all
particles to be accommodated in Particles[] and then sort and move them to SendBuf[]
by sort_particles().


anywhere accommodation  Since we cannot have the complete per-grid histogram if
we have anywhere accommodation or we need a histogram as large as the whole simulation
space, the particle transfer in anywhere accommodation mode takes place in two phases;
at first particles are transferred in ordinary non-position-aware manner only aware of their
residing subdomains; then the second phase transfer takes place in each helpand-helper
family in position-aware manner. In order to minimize the size of per-grid histogram, we
have to define normal accmmodation more restrictedly so that it means all particles reside
in their orignial subdomain or in the adjacent boundary plane of one of its neighbors.


hot-spot We cannot be perfectly position-aware  if a grid-voxel is too congested, or a
node to which the voxel is assigned should have too many particles which can be all in
simulated system in the most extreme case. Therefore, we could have to split the set of
particles in a too congested hot-spot into subsets whose cardinalities are at least a threshold
Phot = gridOverflowLimit/2 given by the simulator body. The threshold plays two roles;
every split particle subset for a grid-voxel is large enough and not less than Phot to ensure
the satisfaction of, e.g., the law of large numbers for Monte Carlo collision in the grid-voxel;
and the particle population for a node is not too large and the excess over that expected
by OhHelp balancing mechanism is less than 2Phot.
When we have a hot-spot, the nodes which have and will have the particles in them
should know how many particles reside and will reside in each node involved. Since we can-
not gather/scatter all per-grid histograms of involved nodes or the total size of them can
be as large as the histogram for the whole simulation space, we have to gather/scatter only
the histogram entry of the hot-spot. Furthermore, we have to be careful to perform the
gather/scatter to avoid unnecessarily frequent or system-wide collective communications
and also to avoid deadlocks or unnecessary serializations on possibliy multiple inter-family
collective communications. Therefore, we need to design a sophisticated hot-spot manage-
ment scheme and a set of functions such as gather_hspot_recv(), gather_hspot_send(),
scatter_hspot_send() and scatter_hspot_recv().


rebalance  In order to fully exploit the restricted definition of normal accommodation
and to minimize the amount of inter-node communications for a subdomain as well as the
number of nodes involved in them, we have to strictly manage the set of nodes which can
send/receive particles residing in the subdomain. This management requires a special care
when the helpand-helper tree is reconfigured because, for example, a node n may have to
send particles to another node which becomes a helper of a neighbor of the old helpand of
n, or n may have to receive particles from another node which was a helper of a neighbor
of the new helpand of n. These inter-family communication with transitional state of the
family tree requires us to keep the neighbors of old helpand in Neighbors[2], to keep the
transtive neighboring configuration in RealDstNeighbors[1][] and RealSrcNeighbors[1][]
by update_real_neighbors(), to do additional work for the new helpand different from
the old one in make_send_sched(), gather_hspot_send() and scatter_hspot_recv(),
and so on.
