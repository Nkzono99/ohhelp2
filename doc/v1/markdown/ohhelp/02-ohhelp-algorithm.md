# 2 OhHelp Algorithm

Source: `doc/v1/original/ohhelp.pdf`, pages 11-11.

<!-- Page 11 -->

## 2 OhHelp Algorithm

### 2.1 Overview and Definitions

As shown in Figure 1, OhHelp simply partitions the simulated D-dimensional space do-
main (D ≤3) into (almost) equal-size N subdomains and assings each subdomain n
(n ∈[0, N−1]) to each of N (MPI) processeses, or computation node, whose MPI rank,
or identifier, is also n, as its primary subdomain. In the figure, non-italic black numbers
are the identifiers of nodes and also those of primary subdomains assigned to them. Each
node n is responsible for its primary subdomain n, and also all the particles in it if the
numbers of those primary particles in subdomains are balanced well, or more specifically,
if the number of particles Pn in a subdomain n satisfies the following inequality for all n,

Pn ≤(P/N)(100 + α)/100 ≡Pmax                          (1)

where P  is the total number of particles and α is the tolerance factor percentage greater
than 0 and less than 100. We refer to the simulation phases in this fortunate situation as
those in primary mode.
Otherwise, i.e., if the inequality (1) is not satisfied for some subdomain n as shown in
Figure 1, the simulation is performed in secondary mode. In this mode, every node, except
for one node (12 in the figure), is responsible for a secondary subdomain having particles
more than the average, in addition to its primary one. For example, the subdomain 22 has
helper nodes 02, 30 and 33 shown in italic and blue letters in Figure 1. The particles in
a densely populated subdomain are also distributed to its helper nodes as their secondary
particles so that each node n has Qn particles in total, which are the union of Qnn pri-
mary particles in the primary subdomain n and Qmn secondary particles in the secondary
subdomain m, satisfying the following inequality for balancing similar to (1) for all n.

Qn = Qnn + Qmn ≤(P/N)(100 + α)/100 = Pmax                   (2)

Note that since all but one nodes have secondary subdomains, a node whose primary
subdomain is densely populated, e.g., node 22, is not only helped by other nodes but also
helps another node 20, as the balancing algorithm discussed in §2.2 orders.
Also note that the load in secondary mode is balanced not only in the number of particles
but also in the size of responsible subdomains, although the latter load is twice as heavy


03   13   23   33


02 03 12 11 22 02 32
00 20   30
10 31   33
01   11 01 21 13 31 21
23   32

00   10   20 22 30




Figure 1: Space domain partitioning.
