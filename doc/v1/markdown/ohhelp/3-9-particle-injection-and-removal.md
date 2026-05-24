# 3.9 Particle Injection and Removal

Source: `doc/v1/original/ohhelp.pdf`, pages 96-97.

<!-- Page 96 -->

## 3.9 Particle Injection and Removal

As discussed in §3.5.5, level-2 library provides you with a function (subroutine) oh2_
inject_particle() to inject a particle dynamically. The level-4p extended library also
has its own version of the injection function oh4p_inject_particle() as shown in §3.7.9.
This section revisits this issue and also discusses its counterpart, particle removal.


#### 3.9.1 Level-1 Injection and Removal

If you use level-1 library only, what you need to do on injecting and/or removing particles
is to maintain nphgram correctly as far as the library concerns. Since the function oh1_
transbound() will not be surprised at a sudden apparition of a particle into any subdo-
main and any node, you may freely increase an element of nphgram to notify the library of
the particle injection19. This unusual increase of nphgram elements, however, may cost if
particles are injected into a node which is not responsible for the subdomain to which the
particles have appeared or for that adjoining the subdomain. That is, oh1_transbound()
needs some global communications to make the particle transfer schedule, which are un-
necessary on usual boundary crossing transfers. On the other hand, decreasing elements
of nphgram to remove particles20 is no problem in terms of both logical correctness and
performance of oh1_transbound().
An important caution on the play with nphgram is that oh1_transbound() is only aware
of the load balancing of particles whose populations in subdmains are reported in nphgram,
of course. This means that if you have a stock of inactive particles in your particle buffer
from which you pick particles to be injected and into which you fling removed particles, your
buffer could overflow because oh1_transbound() does not know anything about the stock.
Therefore, the stock should be sufficiently small, say up to some hundred thousands. Note
that particle recycling without stock, i.e., injecting a particle only when another particle is
removed by overwriting particle data, should cause no problem.
A way to avoid the overflow of the stock, especially when the stock is significantly large,
is to include the number of particles in the stock into nphgram making them pretend to
reside in a subdomain. This works well with respect to the balancing of required memory
space but might cause severe imbalance of computation, because oh1_transbound() does
not know that particles in the stock are inactive. Moreover, since oh1_transbound() may
decide to throw particles in the stock away to other nodes, the node could find it has no
particles to recycle in the stock on injection.


#### 3.9.2 Level-2 (and 3) Injection and Removal

On the other hand, an injection by oh2_inject_particle() is not only as easy as just
increasing nphgram but also consistent with other library functions especially with oh2_
transbound() (and thus oh3_transbound() usually), which recognizes the particle, the
subdomain into which it is injected, and the memory location at which it is stored. That
is, oh2_transbound() automatically picks injected particles from the bottom of pbuf and
places them into appropriate position in pbuf or transfers them to appropriate nodes which
are responsible for the subdomains they reside. What you need to take care of is that
you have to reserve some space (not a stock) in pbuf large enough to inject particles in a
simulation time step. If the space is too large for a node due to a significantly large number
of potential injections, you can limit the space to a reasonable size and let the node having

19Unless the total of nphgram reaches or exceeds 231 −1.
20Or skipping the increment of nphgram element for the particle to be removed.


<!-- Page 97 -->

too many particles to be injected push overflown ones to other nodes. A simple solution
to do it is to repeat oh2_transbound() and an all-reduce communication to confirm the
completion of all particle injections, because it is assured that the space for injection is
emptied each time oh2_transbound() is executed.
Particle removal can be implemented more easily with level-2 or level-3 library. What
you need to do is to set nid element of the particle in problem to be −1, excluding it
from counting particles for nphgram. Then oh2_transbound() will remove the particles
reclaiming the space for them. However, if you want to remove an injected particle before
the call of oh2_transbound() following the injection by your own special reason, you
have to call oh2_remove_injected_particle() passing the particle in the reserved space
into which the injected particle is stored by oh2_inject_particle(), not decrementing
nphgram by yourself but delegating it to the function. This caution is based on that the
library internally maintains information about injected particles so that oh2_transbound()
properly handle them and thus you have to tell the library that the particle once injected
is removed.
Similarly, if you want to move a particle after its injection and before the call of oh2_
transbound(), you have to remove it by oh2_remove_injected_particle() and then
call oh2_remap_injected_particle() after setting the structure elements of the particle
especially nid and spec. If you are (almost) sure that injceted particles will move afterward,
however, you can omit the call of oh2_remove_injected_particle() by giving the particle
having negative nid when you call oh2_inject_particle(). Note that the maintenance of
nphgram should be delegated to oh2_remap_injected_particle() as in the case of oh2_
inject_particle() and oh2_remove_injected_particle().
Another caution of the injection by oh2_inject_particle() and the removal by setting
nid to −1 is that these operations are expected to be performed after the first call of
oh2_transbound() or oh3_transbound() which takes care of initial particle distribution.
Therefore, if by some reason your simulator code needs to inject/remove particles into/from
initial setting of particles before the first call of oh2_transbound() or oh3_transbound(),
you need to call oh2_set_total_particles() before the injection/removal setting nphgram
correctly, as discussed in §3.5.8.


#### 3.9.3 Level-4p and 4s Injection and Removal

The discussion above for level-2 (and level-3) injection/removal also holds for level-4p ex-
tension with its own injection function oh4p_inject_particle(), as far as you are fully
aware of particle histograms maintained by this function and mapping functions oh4p_
map_particle_to_neighbor() and oh4p_map_particle_to_subdomain(). That is, if the
injected particle stays at the position, where you specified when you call oh4p_inject_
particle(), until you call oh4p_transbound(), per-subdomain and per-grid histograms
are properly passed to oh4p_transbound(). Similarly, if you set the nid element of a par-
tilce to −1 without calling oh4p_map_particle_to_neighbor() nor oh4p_map_particle_
to_subdomain(), the particle will be safely eliminated by oh4p_transbound().
However, the injection/removal logic of your simulation code could violate the rule
above. For example, you might wish to move a particle after injecting it and before the
call of oh4p_transbound() to cause a trouble because oh4p_transbound() cannot rec-
ognize the motion.  Calling a mapping function at moving cannot solve the problem be-
cause it simply causes double counting for its original and new positions. A simple so-
lution is to call oh4p_remove_mapped_particle() to eliminate the particle in problem
temporarily and then oh4p_map_particle_to_neighbor() or oh4p_map_particle_to_
subdomain(), or the combined function oh4p_remap_particle_to_neighbor() or oh4p_
