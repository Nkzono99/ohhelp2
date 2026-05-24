# Index - Part 2

Source: `doc/v1/original/ohhelp.pdf`, pages 590-593.

<!-- Page 590 -->

Gen: Modify the description about nphgram exculded from level-4p API.    . . . . . . . . . . .  23
Gen: Modify the description of position-awareness.   . . . . . . . . . . . . . . . . . . . . . . . . . . .  24
oh part.h: Add description of oh4p inject particle().   . . . . . . . . . . . . . . . . . . . . . . . .  42
oh part.h: Add description of the necessity of x, y, z elements in S particle.   . . . . . . . .  42
Gen: Add the section to describe level-4p extension.   . . . . . . . . . . . . . . . . . . . . . . . . . .  72
Gen: Add introduction of oh4p inject particle().    . . . . . . . . . . . . . . . . . . . . . . . . . .  96
Gen: Split S3.9 into three sub-sub-sections.  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  96
Gen: Add description of level-4p injection and removal.   . . . . . . . . . . . . . . . . . . . . . . . .  97
Gen: Add aliases oh per grid histogram() and oh remove mapped particle(), and those of
oh4p init(), oh4p max local particles(), oh4p transbound(),
oh4p map particle to neighbor(), oh4p map particle to subdomain() and
oh4p inject particle().   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  107
Gen: Add level-4p library files   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 129
Gen: Add level-4p library files.  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 129
Gen: Add description of level-4s.    . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 130
Gen: Add the section for the overview of level-4p extension.  . . . . . . . . . . . . . . . . . . . . . 323
ohhelp4p.h: Add this header file with the section for it.    . . . . . . . . . . . . . . . . . . . . . . . . 325
ohhelp4p.c: Add this source file with the section for it.  . . . . . . . . . . . . . . . . . . . . . . . . . 346
samplef.mk: Add level-4p library files.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 554
samplec.mk: Add level-4p library files.  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 555
v0.9.9-09
Gen: The followings are for the further mending of the bug fix in v0.9.8 of the secondary
mode stability check not only with particle injection but also with particle removal and
position-aware particle management.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 1
try_stable1(): Remove the explanation added in v0.9.8 because count stay() does not
exclude injected particles now.  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  177
try_stable1(): Fix the bug caused by ignoring the possibility that Pmax < Qnn + Qparent(n)n
and modify/add explanation on this issue.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  177
try_stable1(): Remove the explanation added in v0.9.8 because injected particles are
included in stay.prime, stay.sec and NOfPToStay[].   . . . . . . . . . . . . . . . . . . . . . .  180
try_stable1(): Rename the variable npmove as floating to show its meaning more
appropriately.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  183
count_stay(): Remove the subtraction of the number of injected particles and the
explanation on it.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  185
count_stay(): Modify the coding of the addition of NOfPToStay[].   . . . . . . . . . . . . . . . . 185
schedule_particle_exchange(): Remove the adjustment of get.prime and get.sec when
reb ≤0 together with the explanation on it, because they now include injected particles
and thus are not necessary to be adjusted.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  191
v0.9.9-10
Gen: The followings are for the introduction of oh2 remap injected particle() and
oh2 remap injected particle(), and a few coding style modifications in
oh2 inject particle().   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 1
Gen: Add description of oh2 remap injected particle().  . . . . . . . . . . . . . . . . . . . . . .  41
Gen: Add description of oh2 remove injected particle().  . . . . . . . . . . . . . . . . . . . . .  41
Gen: Add description of oh2 remap injected particle().  . . . . . . . . . . . . . . . . . . . . . .  46
Gen: Add description of oh2 remove injected particle().  . . . . . . . . . . . . . . . . . . . . .  47
Gen: Add description of oh2 remove injected particle().  . . . . . . . . . . . . . . . . . . . . .  97
Gen: Add description of oh2 remap injected particle().  . . . . . . . . . . . . . . . . . . . . . .  97
Gen: Add aliases of oh2 remap injected particle() and
oh2 remap injected particle().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  107
nOfLocalPLimit: Add references from oh2 remap injected particle() and
oh2 remove injected particle().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  226
Particles: Add references from oh2 remap injected particle() and
oh2 remove injected particle().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  226


<!-- Page 591 -->

nOfInjections: Add references from oh2 remap injected particle() and
oh2 remove injected particle().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  228
specBase: Add references from oh2 remap injected particle() and
oh2 remove injected particle().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  228
Particle_Spec(): Add uses in oh2 remap injected particle() and
oh2 remove injected particle().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  229
ohhelp2.h: Add prototype of oh2 remap injected particle().  . . . . . . . . . . . . . . . . . . . 231
ohhelp2.h: Add prototype of oh2 remove injected particle().   . . . . . . . . . . . . . . . . . . 231
ohhelp2.h: Add aliases of oh2 remap injected particle() and
oh2 remove injected particle().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  231
ohhelp2.h: Add prototypes of oh2 remap injected particle() and
oh2 remove injected particle().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  232
ohhelp2.h: Add prototypes of oh2 remap injected particle () and
oh2 remove injected particle ().   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  232
oh2_inject_particle(): Modify coding style to use cached local variables for nOfNodes and
nOfSpecies.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  265
oh2_remap_injected_particle(): Introduced to maintain InjectedParticles[0][][] when a
particle is moved after injection.    . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  266
oh2_remove_injected_particle(): Introduced to maintain InjectedParticles[0][][] when a
particle is removed after injection.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  267
v1.0.0
Gen: Introduction of ohhelp4s.h and ohhelp4s.c. (2012/05/11)    . . . . . . . . . . . . . . . . . . . .  1
v1.0.0-01
Gen: The followings are for newly designed ohhelp4s.h and ohhelp4s.c.    . . . . . . . . . . . . . .  1
Gen: Modify the sentence for the level-4p extension to add the level-4s extension.  . . . . . .  1
Gen: Add introductory description of level-4s extension.    . . . . . . . . . . . . . . . . . . . . . . .  17
Gen: Add description about level-4s API.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  23
OH_LIB_LEVEL_4S: Add commented-out definition of the macro to oh config.h for the
activation of level-4s extention.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  25
OH_LIB_LEVEL_4PS: Introduce this macro being defined iffeither OH LIB LEVEL 4P or
OH LIB LEVEL 4P is defined.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  25
oh part.h: Add description of oh4s inject particle().   . . . . . . . . . . . . . . . . . . . . . . . .  42
Gen: Add the section to describe level-4s extension.  . . . . . . . . . . . . . . . . . . . . . . . . . . .  84
Gen: Add description of level-4s injection and removal.   . . . . . . . . . . . . . . . . . . . . . . . .  98
Gen: Add aliases of level-4s’s API functions.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 107
Gen: Add level-4s library files  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 129
Gen: Add level-4s library files.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 129
Gen: Add description of level-4s.    . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 130
Gen: Add the section for the overview of level-4s extension.   . . . . . . . . . . . . . . . . . . . . . 440
ohhelp4s.h: Add this header file with the section for it.  . . . . . . . . . . . . . . . . . . . . . . . . . 442
ohhelp4s.c: Add this source file with the section for it.   . . . . . . . . . . . . . . . . . . . . . . . . . 462
v1.0.0-02
Gen: The followings are for changes in libraries other than level-4s to add level-4s extension. 1
Gen: Change the configuration of oh config.h for the introduction of the level-4s extension. 25
oh config.h: Change the switch for level-4 definitions from OH LIB LEVEL 4P to
OH LIB LEVEL 4PS   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  25
STATS_TB_SORT: Change the switch to control the definition from OH LIB LEVEL 4P to
OH LIB LEVEL 4PS.  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  99
StatsTimeStrings: Change the switch to control the element string definition from
OH LIB LEVEL 4P to OH LIB LEVEL 4PS.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  100
OH_POS_AWARE: Change the switch to control the definition from OH LIB LEVEL 4P to
OH LIB LEVEL 4PS.  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  132
ohhelp c.h: Surround aliase definitions and prototype delarations for level-4p by #ifdef
OH LIB LEVEL 4P construct.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  343


<!-- Page 592 -->

ohhelp f.h: Surround aliase definitions for level-4p by #ifdef OH LIB LEVEL 4P construct. 343
v1.0.0-03
Gen: The followings are bug fix in libraries other than level-4s.    . . . . . . . . . . . . . . . . . . .  1
init1(): Fix the bug by which TempArray[n] is mistakingly updated when n < 0.   . . . . . 167
transbound1(): Insert a workaround code to cope with Intel MPI’s bug in
MPI Alltoall().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  173
make_comm_count(): Insert a workaround code to cope with Intel MPI’s bug in
MPI Alltoall().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  198
Vprint_Norank(): Introduce this macro for vprint() without printing rank identifier so as to
keep compilers checking the consistency of format string and argments from complaining
the inconsistency of RANKFORMAT without rank argument.    . . . . . . . . . . . . . . . . . . .  224
vprint(): Use Vprint Norank() instead of Vprint() when verboseMode < 3 and thus the
rank identifier is not printed.  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  224
init_subdomain_actively(): Message.xyz[i] is replaced with Message.xyz[d].   . . . . . 290
upd_real_nbr(): Add int after const for the declaration of nid which Fujitsu’s CC somehow
accepted without int.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  408
v1.1.0
Gen: Introduction of oh1 neighbors(), oh1 families() and oh1 accom mode().
(2015/07/31)    . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 1
v1.1.0-01
Gen: The followings are for introduction of oh1 neighbors().   . . . . . . . . . . . . . . . . . . . .  1
Gen: Add a description of oh1 neighbors().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  26
Gen: Add a description of oh1 neighbors().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  33
Gen: Add a description of update of neighboring information.   . . . . . . . . . . . . . . . . . . .  36
Gen: Add alias of oh1 neighbors().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 107
ohhelp1.h: Add a brief description of NeighborsShadow and NeighborsTemp.   . . . . . . . . . 147
ohhelp1.h: Add a brief explanation of oh1 neighbors().  . . . . . . . . . . . . . . . . . . . . . . . . 152
ohhelp c.h: Add alias of oh1 neighbors().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 153
ohhelp f.h: Add alias of oh1 neighbors().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 153
ohhelp1.h: Add prototype of oh1 neighbors().   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 153
ohhelp1.h: Add prototype of oh1 neighbors ().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 154
NeighborsShadow: Introduce this global pointer.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 161
NeighborsTemp: Introduce this global pointer.  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 161
init1(): Allocate an array of [3][3D] for *nbor if oh1 neighbors() has not been called
yet.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  165
init1(): Let NeighborsTemp have *nbor, and initialize NeighborsShadow[0][] with *nbor if
oh1 neighbors() has been called beforehand.  . . . . . . . . . . . . . . . . . . . . . . . . . . . .  166
oh1_neighbors_(): Introduce the function.  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 168
oh1_neighbors(): Introduce the function.  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 168
build_new_comm(): Add update of NeighborsShadow.   . . . . . . . . . . . . . . . . . . . . . . . . . 209
v1.1.0-02
Gen: The followings are for introduction of oh1 families().   . . . . . . . . . . . . . . . . . . . . .  1
Gen: Add a description of oh1 families().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  26
Gen: Add a description of oh1 families().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  34
Gen: Add a description of update of family information.   . . . . . . . . . . . . . . . . . . . . . . .  36
Gen: Add alias of oh1 families().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 107
ohhelp1.h: Add a brief description of FamIndex and FamMembers.    . . . . . . . . . . . . . . . . . 146
ohhelp1.h: Add a brief explanation of oh1 families().    . . . . . . . . . . . . . . . . . . . . . . . . 152
ohhelp c.h: Add alias of oh1 families().    . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 153
ohhelp f.h: Add alias of oh1 families().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 153
ohhelp1.h: Add prototype of oh1 families().   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 153
ohhelp1.h: Add prototype of oh1 families ().    . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 154
FamIndex: Introduce this global pointer.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 169
FamMembers: Introduce this global pointer.    . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 169


<!-- Page 593 -->

oh1_families_(): Introduce the function.  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 169
oh1_families(): Introduce the function.    . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 169
try_primary1(): Add reinitialization of FamIndex and FamMembers.   . . . . . . . . . . . . . . . 175
build_new_comm(): Add update of FamIndex and FamMembers.  . . . . . . . . . . . . . . . . . . . 208
v1.1.0-03
Gen: The followings are for introduction of oh1 accom mode().  . . . . . . . . . . . . . . . . . . . .  1
Gen: Add a description of oh1 accom mode().   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  26
Gen: Add a description of oh1 accom mode().   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  37
Gen: Add alias of oh1 acc mode().   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 107
accMode: Introduce this global accommodation mode indicator.   . . . . . . . . . . . . . . . . . . 135
ohhelp1.h: Add a brief explanation of oh1 accom mode().   . . . . . . . . . . . . . . . . . . . . . . . 152
ohhelp c.h: Add alias of oh1 accom mode().   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 153
ohhelp f.h: Add alias of oh1 accom mode().   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 153
ohhelp1.h: Add prototype of oh1 accom mode().  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 153
ohhelp1.h: Add prototype of oh1 accom mode ().   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 154
init1(): Add initialization of accMode.    . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 162
transbound1(): Add setting of accMode.  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 174
oh1_accom_mode_(): Introduce the function.   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 211
oh1_accom_mode(): Introduce the function.  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 211
v1.1.1
Gen: Bugs of level-1 specific functionalities in ohhelp1.c are fixed. (2015/10/23)   . . . . . . .  1
try_primary1(): Corrections of NOfSend and NOfRecv were done only on [me], i.e., [0][0][n],
outside the loop for s. They are now included in the inner-loop for m (j) so that the
correctoins are performed if m = n, i.e., on [0][s][n] for all s.   . . . . . . . . . . . . . . . . .  176
were left   make_comm_count():                     putme
variable does not                       hold max(0,                                       qput(p)                                   unchanged−∑s−1t=0whenqstay(p,it becomest)) whenlessthethansecondstay,argumenti.e., theof
max(·, ·) becomes negative. It is now let have 0 when it is less than stay.    . . . . . . .  199
