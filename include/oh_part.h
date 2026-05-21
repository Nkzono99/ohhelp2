/* File: oh_part.h
   Version 1.1.1 (2015/10/23)
   Copyright (C) 2009-2015  Hiroshi Nakashima <h.nakashima@media.kyoto-u.ac.jp>
                            (ACCMS, Kyoto University)
   This program can be freely used, redistributed and modified for non-
   commercial purpose providing that the copyright notice above remains
   unchanged.
*/
#ifndef OH_PART_H
#define OH_PART_H

#include "oh_config.h"
#ifdef  OH_BIG_SPACE
typedef long long int OH_nid_t;
#else
typedef int OH_nid_t;
#endif

#ifdef OH_PARTICLE_HEADER
#include OH_PARTICLE_HEADER
#else
struct S_particle {
  double x, y, z, vx, vy, vz;
  int pid, preside;
  long long int trace_id;  /* Unique particle tracking ID */
  OH_nid_t nid;
  int spec;
};
#define OH_HAS_SPEC
#endif

#endif
