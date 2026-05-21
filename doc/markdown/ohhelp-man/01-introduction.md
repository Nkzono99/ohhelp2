# 1 Introduction

Source: `doc/original/ohhelp-man.pdf`, pages 4-4.

<!-- Page 4 -->

## 1 Introduction

Particle-in-Cell (PIC) simulations have played an indispensable role in theoretical and prac-
tical research of high-energy physics, space plasma physics, cloud modeling, combustion
engineering, and so on, since early 1980’s.  In typical PIC simulations, a huge number
of charged particles interact with electromagnetic field mapped onto a large number of
grid points, governed by Maxwell’s equations and the Lorentz force law. These hugeness
and largeness of the simulation essentially require to parallelize the computation not only
for efficient execution but also for feasible implementation on distributed memory systems
which are the majority of modern supercomputers. That is, the simulation has to be de-
composed almost equally so that good load balancing is achieved and, more importantly,
each decomposed subproblem is accommodated by a local memory of limited capacity. This
almost-equal decomposition is a necessary condition to make the simulation scalable so that
we fully utilize larger scale systems with nearly stable efficiency by enlarging the problem
size proportionally to the system size.
However, this necessary condition is satisfied neither by simple particle-decomposed sim-
ulations, by also simple static domain-decomposed ones, nor even by sophisticated dynamic
domain-decomposed simulations, because a process in these conventional methods would
have too large (sub)domain or too many particles.  Therefore, we have proposed a new
domain-decomposed PIC simulation method named OhHelp[1] which is scalable in terms
of the number of particles as well as the domain size. Its problem decomposition and load
balancing mechanisms are outlined as follows.

1. The space domain is equally partitioned to assign each subdomain to each node as
its primary subdomain.

2. If one or more subdomains have too many particles, i.e., more than average plus a
certain tolerance, every but one node is responsible for another subdomain which has
particles more than average as its secondary subdomain.

3. A part of particles in the secondary subdomain of a node are assigned to the node so
that no nodes have too many particles.

Since a node has to have at most two subdomains, OhHelp is scalable with respect to
the domain size. As for the number of particles, OhHelp keeps its excess over the per-
node average less than the tolerance by dynamically rearranging the secondary subdomain
assignment and thus also achieves good scalability.
In the rest of this document, we describe OhHelp and its library as follows. In the next
§2, OhHelp algorithm is explained more detailedly. Then §3, the heart of this document,
describes API of the OhHelp library so that you incoroparte OhHelp into your own PIC
simulator.

References

[1] H. Nakashima, Y. Miyake, H. Usui and Y. Omura.  OhHelp: A Scalable Domain-
Decomposing Dynamic Load Balancing for Particle-in-Cell Simulations. In Proc. Intl.
Conf. Supercomputing, pp. 90–99, June 2009.
