# 2.3 Checking and Keeping Local Balancing

Source: `doc/v1/original/ohhelp.pdf`, pages 13-16.

<!-- Page 13 -->

av. # of particles





33 00 32 01 30 10 13 03 23 20 31 02 11 21 12 22

(a) before balancing
av.# ofparticles 22 12 21 11 22 12 21 02 11 12 12 22 12 31     20





33 00 32 01 30 10 13 03 23 20 31 02 11 21 12 22

(b) after balancing

Figure 2: Subdomain assignment with perfect balancing of number of particles.


(b6) If G has two or more elements, pick an arbitrary element r from G and assign the
subdomain r to other nodes in G without particle assignment. Otherwise, i.e., G has
only one element, let r be this node.

It is obvious the algorithm stops making every node n except for r have a secondary
subdomain and Qn = P/N for all n. As mentioned in §2.1, the key for perfect balancing is
the step (b5) where we add g with Pg ≥P/N but Qg < P/N to L so that it helps other
node when it has deputed so many particles to its helpers that Qg becomes less than P/N
tentatively. Figure 2 shows an example balancing result for the particle distribution shown
in Figure 1 providing we suddenly faces the imbalance due to, for example, initial particle
positioning. The number of particles in each subdomain (a) and that assigned to each node
(b) are illustrated by the bar whose color and numbers above and below it represent the
subdomain and the node.

## 2.3 Checking and Keeping Local Balancing

In the secondary mode, the particle movements crossing subdomain boundaries could break
the satisfiability of the inequality (2) if we stuck to the secondary subdomain assignment.
To examine the satisfiability and to keep the local balancing among a helpand-helper family,
we form a tree T whose vertices are the computation nodes and edges represent helpand-
helper relationship. That is, the root of the tree is the node r defined in the step (b6) of the
previous section, and the parent of a non-root node is its helpand. The tree corresponding
to the balancing result in Figure 2(b) is show in Figure 3.
The examination of the satisfiability of (2) is performed by traversing the tree T in a
bottom-up (leaf-to-root) manner as follows.

(e1) Let a set of nodes S be that of leaves of the tree T. Let Pnmin be Pn for all n ∈S. If


<!-- Page 14 -->

1212


1111         3131            2020                   1010 0000


0101  2323     21            22


1313  3232         0202     3030  3333


0303


Figure 3: Helpand-helper tree for balancing result in Figure 2(b).


there is an element n ∈S such that Pn = P nmin > Pmax, the examination fails.

(e2) Repeat the following steps (e3) and (e4) until S becomes {r}.

(e3) Find a node n such that the set of its helpers H(n) is a subset of S, and remove H(n)
from S.

(e4) Add n to S and let Pnmin be as follows.
∑
P nmin = max(0, Pn −   (Pmax −P mmin ))
m∈H(n)

If Pnmin > Pmax, the examination fails.

Since a leaf node does not have helpers, the failure in the step (e1) obviously means that                            ∑
the inequality (2) cannot be satisfied. As for the failure in (e4), since  m∈H(n)(Pmax−P mmin )
means the maximum particle amount which n’s helpers accommodate as their secondary
particles and thus P nmin is the minimum number of particles in n which the node n has to be
responsible for, P nmin > Pmax leads us that the inequality (2) is unsatisfiable. Therefore, the
algorithm is complete. On the other hand, when the algorithm stops at (e2) with P nmin ≤
Pmax for all n, it is assured that, for all n, Pn particles can be distributed among n and its
helpers keeping Qm ≤Pmax for all m ∈F(n) where F(n) is defined as {n} ∪H(n). That
is, even if n has to accommodate Pmax −P nmin particles for its helpand,∑   Pn −P nmin particles
can be accommodated by its helpers because they are at most   m∈H(n)(Pmax −P mmin ).
Therefore, the algorithm is sound.
If the examination passes, a part of particles in a subdomain n are redistributed to the
members of the family F(n), i.e., the node n and its helpers in H(n). The target of the
redistribution is the following, where Qnk is the number of particles in the subdomain n and
currently accommodated by the node k.

- Particles currently in a node m /∈F(n), which has just crossed a boundary and moved
into the subdomain n from other subdomain.


<!-- Page 15 -->

P
B

Φ   QQ    R    QQ  (> PP     )
R'R'         PP    QQ −PP

(a) without pushing down                    (b) with pushing down

Figure 4: Particle redistribution in a family.


- Particles overflown from a node m ∈F(n). More specifically, particles are overflown
from m in either of the following cases.

  - m ̸= n and Qnm+P mmin > Pmax and thus Qnm+Pmmin −Pmax particles are overflown
to satisfy the minimum requirement defined by P mmin .
  - m = n and Qnn + Rn > Pmax where Rn is the number of particles assigned to
n as the result of the redistribution for the family rooted by p = parent(n) to
which n belongs as a helper. That is, Rn = Qpn at the beginning of the next
simulation step. The number of overflown particles is Qnn + Rn −Pmax.

Note that the criteria above are to minimize the amount of particle transfer rather than to
minimize the load deviation among the nodes. Let Rfltn be the total number of redistributed
particles defined above or, more specifically, be as follows.
∑  ∑
Rfltn =   Qnk +    max(0, Qnm+P mmin −Pmax) + max(0, Qnn+Rn−Pmax)
k/∈F (n)  m∈H(n)

The local balancing in a helpand-helper family is partly achieved by the following algo-
rithm traversing the tree T in a top-down manner.

(d1) Let a set of node S = {r}, and Rr = 0.

(d2) Repeat the following steps (d3) to (d6) until S becomes empty.

(d3) Remove a node n from S.  If n is the leaf node, let Qn be Pn + Rn and skip the
following steps (d4) to (d6). Otherwise, add the helpers of n, i.e., H(n), to S.

(d4) If the following inequality is satisfied;
∑
Pn + Rn +   max(P mmin , Qmm) ≤Pmax · |F(n)|
m∈H(n)

we need not to push down primary particles of any node m to its own helpers.  If
this holds, let Bm = min(Pmax, Qnm+ max(Pmmin , Qmm)) for all m∈H(n) to represent
the baseline number of particles above which we place particles to be redistributed
as shown in Figure 4(a). Otherwise, let the baseline Bm be min(Pmax, Qnm+Pmmin ) to
allow us to push down Qmm −P mmin particles as shown in Figure 4(b). In both cases,
let Bn, the baseline of n, be min(Pmax, Qnn+Rn).


<!-- Page 16 -->

(d5) Find the minimum subset Fl(n) of F(n) such that the followings are satisfied.

∀m′ ∈Fl(n), ∀m ∈F(n) −Fl(n) : Bm′ ≤Bm
∑
∀m ∈F(n) −Fl(n) : Rfltn +   Bm′ ≤Bm · |Fl(n)|
m′∈Fl(n)

(d6) Let Rm for all m ∈H(n) and Qn be the followings.
  ∑
 (Rfltn +    Bm′)/|Fl(n)| −Bm m ∈Fl(n)
R′m =        m′∈Fl(n)              
0               m /∈Fl(n)
Rm = R′m   Qn = Bn + R′n

The step (d5) is to find the leftmost three bars (nodes) in Figure 4(a) and (b) for the
local load balancing among these lightly loaded nodes by distributing R′m given in the step
(d6).
