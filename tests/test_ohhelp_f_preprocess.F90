program test_ohhelp_f_preprocess
#include "ohhelp_f.h"
#ifndef oh_init
#error "oh_init macro missing"
#endif
#ifndef oh_transbound
#error "oh_transbound macro missing"
#endif
#ifndef oh_set_region_weights
#error "oh_set_region_weights macro missing"
#endif
#if OH_LIB_LEVEL >= 3
#ifndef oh_grid_size
#error "oh_grid_size macro missing for level 3+"
#endif
#endif
#ifdef OH_LIB_LEVEL_4P
#ifndef oh_per_grid_histogram
#error "oh_per_grid_histogram macro missing for level 4p"
#endif
#endif
#ifdef OH_LIB_LEVEL_4S
#ifndef oh_particle_buffer
#error "oh_particle_buffer macro missing for level 4s"
#endif
#endif
  implicit none

  integer, external :: oh_transbound
  external :: oh_init
  external :: oh_set_region_weights
#if OH_LIB_LEVEL >= 3
  external :: oh_grid_size
#endif
#ifdef OH_LIB_LEVEL_4P
  external :: oh_per_grid_histogram
#endif
#ifdef OH_LIB_LEVEL_4S
  external :: oh_particle_buffer
#endif
  integer :: mode

  mode = oh_transbound(0, 0)
end program test_ohhelp_f_preprocess
