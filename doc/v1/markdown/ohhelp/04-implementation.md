# 4 Implementation

Source: `doc/v1/original/ohhelp.pdf`, pages 130-131.

<!-- Page 130 -->

## 4 Implementation

This section describes the implementation details of the OhHelp library showing every line
of almost all source files, which are extracted from this section to make the source files
perfectly correspond to the explanation given in this section.
The library package consists of the following files.

Common headers oh config.h, oh part.h and oh stats.h for the library and simulator body
programs, which were discussed and shown in §3.3, §3.5.1 and §3.10.1.

Level-1 library sources ohhelp1.h and ohhelp1.c, whose source lines are explained and
shown in §4.2 and §4.3, to implement the fundamental part of OhHelp algorithm, basic
collective communications among family members, statistics collection and reporting,
and verbose messaging.

Level-2 library sources ohhelp2.h and ohhelp2.c, whose source lines are explained and
shown in §4.4 and §4.5, to implement particle transfer and injection.

Level-3 library sources ohhelp3.h and ohhelp3.c, whose source lines are explained and
shown in §4.6 and §4.7, to implement particle-to-subdomain mapping and communi-
cations of field-arrays.

Level-4p library sources ohhelp4p.h and ohhelp4p.c, whose source lines are explained
and shown in §4.9 and §4.10, to implement position-aware particle management.

Level-4s library sources ohhelp4s.h and ohhelp4s.c, whose source lines are explained and
shown in §4.12 and §4.13, to implement yet another position-aware particle manage-
ment for, e.g., SPH method.

Fortran module sources oh type.F90 shown in §3.4.1 and §3.5.1, oh mod1.F90 shown
§3.4, oh mod2.F90 shown in §3.5, oh mod3.F90 shown in §3.6, oh mod4p.F90 shown
in §3.7, and oh mod4s.F90 shown in §3.8, to provide Fortran structured data type
oh_mycomm and oh_particle and the prototypes of Fortran API functions.

Headers for function aliasing ohhelp f.h for Fortran and ohhelp c.h for C, which are
discussed and shown in §4.2.11, §4.4.5, §4.6.6, §4.9.7 and §4.12.7.

Sample simulator sources sample.F90 and sample.c which were shown in §3.13.1 and
§3.13.2.

Sample make files samplef.mk for Fortran and samplec.mk for C, which are given in §4.14.1
and §4.14.2.

### 4.1 Naming Convention

To name indentifiers, i.e., variables, functions and so on, we use the following conventions.

Macro Constants are named only with uppercase letters and underscores as usual. For
example, OH_DIMENSION is a macro constant.

Global Variables are named with a combination of uppercase and lowercase letters. The
first letter of an atomic varialbe is lowercase, while an array or a structured variable is
capitalized. For example, nOfNodes is an integer global variable, while NOfPrimaries
is a global and (concepturally) three-dimensional array. Their names usually do not


<!-- Page 131 -->

have underscores but there are two exceptions. One is for MPI data-types whose
names are prefixed by T  like T_Histogram. The other is for MPI operators whose
names start with Op  like Op_StatsTime.

Local Variables are named only with lowercase letters without underscores, such as i
and nn.

Structures have a prefix S  followed by lowercase letters. An element of a structure is
named only with lowercase letters without underscores.  For example, S_heap is a
struct having three elements n, node and index.

Functions are named with lowercase letters and usually with underscores. API functions
is prefixed by ohl  where l is the library level identifier (i.e., 1, 2, 3, 4p or 4s), and
also postfixed by an underscore for those called from Fortran.  For example, oh1_
transbound() is an API functions for C codes while its Fortran counterpart is named
as oh1_transbound_().

Functional Macros have capitalized name like Vprint(). If a name has underscores, the
letters following them are also capitalized like Stats_Reduce_Part_Min().
