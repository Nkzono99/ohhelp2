# 3 OhHelp Library

Source: `doc/original/ohhelp-man.pdf`, pages 11-11.

<!-- Page 11 -->

## 3 OhHelp Library

### 3.1 Library Layers

The OhHelp library package has three fundamental layers which are referred to as level-1,
level-2 and level-3, and (so far) two extensional layers level-4p and level-4s. The functions
provieded by each layer are summarized as follows.

level-1: This level provieds a load-balancer function named oh1_transbound() which ex-
amines whether particles are distriuted among nodes in a well-balanced manner,
(re)builds helpand-helper configuration if necessary, and tells you how to move parti-
cles among nodes. That is, this function implements the OhHelp algorithm described
in §2. In addition, level-1 library has functions for collective communications in hel-
pand-helper families, and those for statistics and verbose messaging.  See §3.4 for
functions excluding those for statistics and verbose messaging which are explained in
§3.10 and §3.11 respectively.

level-2: In this level, the load-balancer function oh2_transbound() does what its level-
1 counterpart does, and transfers particles among nodes according to the schedule
determined by the level-1 function. See §3.5 for detailed explanation of level-2 API
functions.

level-3: Functions for particle manipulation added in this level are to determine the iden-
tifier of the subdomain where a given particle resides. The other useful functions are
for inter-node communications of arrays having vectors/scalars associated with grid
points in a subdomain, i.e., those for electromagnetic field, current density, and so
on. See §3.6 for detailed explanation of level-3 API functions.

level-4p: This extensional level is for position-aware particle management with which the
load balancing mechanism takes care of particle positions so that all particles in
a grid-voxel are accommodated by a particular node (almost) always.  Moreover,
primary/secondary particles of a specific species in a node are sorted according to
the coordinates of the grid-voxels in which they reside so that you easily find a set
of particles in a particular grid-voxel for, e.g., Monte Carlo collision.  See §3.7 for
detailed explanation of level-4p extension and its functinos.

level-4s: This extensional level is to provide yet another position-aware mechanism for,
e.g., SPH (Smoothed Particle Hydrodynamics) method. The differences between this
extension and level-4p one are as follows; each node is responsible of all particles in
a cuboid split from the subdomain for the node by slicing it by planes perpendicular
to z-axis; and each node accommodates not only the particles in the cuboid but also
those in the grid-voxels surrouding the cuboid as halo particles so that the compuation
on a particle in the cuboid may refer to particles nearby the particle. See §3.8 for
detailed explanation of level-4s extension and its functinos.

Functions in each fundamental layer are composed in a level-specific source file, namely
ohhelp1.c, ohhelp2.c and ohhelp3.c which require header files of same names, i.e., ohhelp1.h,
ohhelp2.h and ohhelp3.h. To have a library of level-2 or level-3, it is required to compile lower
level libraries as well, and thus you will have all functions in all layers if you are to use level-3
library. However, this does not mean that you have to use all functionalities provided by all
level libraries. In fact, except for the essential functionality given by oh1_transbound(),
you are almost free to pick functions you like to use. Therefore, API functions are named
with prefixes ‘oh1 ’, ‘oh2 ’ or ‘oh3 ’ to show which level they belong to.
