# 2.2 Secondary Subdomain Assignment

Source: `doc/original/ohhelp-man.pdf`, pages 6-6.

<!-- Page 6 -->

as that in the primary mode. This is another justification for making a node with densely
populated primary subdomain help another node.
The examination whether the load is balanced well and the mode switching possibly with
load rebalancing are performed as follows every simulation time step in which particles can
move crossing subdomain boundaries1.

1. If the inequality (1) is satisfied for all subdomains, the mode stays in or turns to
primary. In the case of staying, only the particles crossing subdomain boundaries are
transferred between nodes by neighboring communications. Otherwise, in addition to
boundary crossing ones, particles that have been secondary are transferred to nodes
responsible for them as primary particles.

2. If the current mode is secondary and the inequality (1) is not satisfied but (2) is
satisfiable keeping the secondary subdomain assignment, the mode stays in secondary
without global rebalancing.  Particles may be transferred among the helpers and
their helpand 2 for the local load balancing in addition to the transfer of the particles
crossing boundaries.  The statisfiability check for (2) and the local balancing are
discussed in §2.3.

3. Otherwise, the secondary subdomain assignments are performed (or modified) so that
Qn  is equal to P/N for all n to accomplish perfect balancing3.  The subdomain
assignment algorithm is discussed in §2.2.

## 2.2 Secondary Subdomain Assignment

When it is detected that the inequality (1) or (2) is unsatisfiable in primary or secondary
mode respectively, secondary subdomains are assigned to nodes, by modifying the original
assignment if the mode has already been in secondary, to accomplish perfect balancing.
The assignment algorithm is quite simple as follows.

(b1) Split the set of nodes into two disjoint subsets L = {n | Pn < P/N} and G = {n | Pn ≥
P/N}. Let the tentative value of Qn be Pn for all n.

(b2) Repeat the following steps (b3) through (b5) until L becomes empty.

(b3) Remove an element l from L such that Ql = minn∈L{Qn} and remove an element g
from G as follows.

- If the mode is secondary and l has been helping a node n in G, let g be n.

- Otherwise, the node g is chosen such that Qg = maxn∈G{Qn}.

(b4) Assign the subdomain g to the node l as its secondary subdomain and also assign
Qgl = (P/N)−Ql particles in the subdomain g to the node l so that Ql ←Ql +Qgl =
P/N. Now Qg becomes Qg −Qgl .

(b5) If Qg < P/N, add g to L. Otherwise add g back to G.

1You may reduce the frequency of these operations by overlapping adjacent subdomains a little bit more
heavily and by exploiting the fact that the velocity of a particle is limited to some upper bound, e.g., light
speed.
2We know English does not has such a word but dare to neologize to mean “the node helped by other
nodes.”
3If P is a multiple of N. Otherwise, Qn is ⌊P/N⌋or ⌈P/N⌉, but we assume P is a multiple of N in this
section for the sake of explanation simplicity.
