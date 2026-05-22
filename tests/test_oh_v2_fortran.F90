#include "oh_config.h"
program test_oh_v2_fortran
  use iso_c_binding
  use ohhelp_v2
  implicit none

  type, bind(C) :: pic_particle
    real(c_double) :: x
    real(c_double) :: y
    real(c_double) :: z
    real(c_double) :: vx
    real(c_double) :: vy
    real(c_double) :: vz
    integer(c_int) :: region
    integer(c_int) :: species
  end type

  type(oh_context_handle) :: context
  type(oh_particle_adapter_handle) :: adapter
  type(pic_particle), target :: particle
  type(pic_particle), target :: particle_buffer(4)
  type(oh_mycomm_v2), target :: mycomm
  real(c_double), target :: weights(1)
  real(c_double), target :: field(1)
  real(c_double), target :: x, y, z
  integer(c_int), target :: sdid(2), nphgram(1,1,2), totalp(1,2)
  integer(c_int), target :: pbase(3), nbor(3,3,3), pcoord(3)
  integer(c_int), target :: sdoms(2,3,1), scoord(2,3), bcond(2,3)
  integer(c_int), target :: bounds(2,3,1), ftypes(7,2), cfields(2)
  integer(c_int), target :: ctypes(3,2,1,2), fsizes(2,3,2)
  integer(c_size_t) :: region_offset, species_offset
  integer(c_size_t) :: x_offset, y_offset, z_offset
  integer(c_int) :: ierr
  integer(c_int) :: dst
  type(c_ptr) :: injected
  type(c_ptr) :: raw_particles

  context = oh_default_context()
  if (.not. oh_context_associated(context)) stop 1

  weights = 1.0_c_double
  call oh_context_set_region_weights(context, weights)
  call oh_context_set_region_weights(context)

  call oh_particle_adapter_create_byte(adapter, c_sizeof(particle), ierr)
  if (.not. oh_particle_adapter_associated(adapter)) stop 2

  region_offset = oh_particle_field_offset(c_loc(particle), &
                                           c_loc(particle%region))
  species_offset = oh_particle_field_offset(c_loc(particle), &
                                            c_loc(particle%species))
  x_offset = oh_particle_field_offset(c_loc(particle), c_loc(particle%x))
  y_offset = oh_particle_field_offset(c_loc(particle), c_loc(particle%y))
  z_offset = oh_particle_field_offset(c_loc(particle), c_loc(particle%z))

  call oh_particle_adapter_use_int_fields(adapter, region_offset, &
                                          species_offset)
  call oh_particle_adapter_use_integer_fields( &
    adapter, region_offset, c_sizeof(particle%region), &
    species_offset, c_sizeof(particle%species))
  call oh_particle_adapter_use_level3_position_fields(adapter, x_offset, &
                                                      y_offset, z_offset)
  call oh_context_set_particle_adapter(context, adapter)
  call oh_set_particle_adapter(adapter)

  raw_particles = c_loc(particle_buffer(1))
  call oh2_init_raw(c_loc(sdid(1)), 1_c_int, 20_c_int, &
                    c_loc(nphgram(1,1,1)), c_loc(totalp(1,1)), &
                    raw_particles, c_loc(pbase(1)), 4_c_int, &
                    c_loc(mycomm), c_loc(nbor(1,1,1)), c_loc(pcoord(1)), &
                    0_c_int, 0_c_int, 0_c_int)
  call oh3_init_raw(c_loc(sdid(1)), 1_c_int, 20_c_int, &
                    c_loc(nphgram(1,1,1)), c_loc(totalp(1,1)), &
                    raw_particles, c_loc(pbase(1)), 4_c_int, &
                    c_loc(mycomm), c_loc(nbor(1,1,1)), c_loc(pcoord(1)), &
                    c_loc(sdoms(1,1,1)), c_loc(scoord(1,1)), 0_c_int, &
                    c_loc(bcond(1,1)), c_loc(bounds(1,1,1)), &
                    c_loc(ftypes(1,1)), c_loc(cfields(1)), &
                    c_loc(ctypes(1,1,1,1)), c_loc(fsizes(1,1,1)), &
                    0_c_int, 0_c_int, 0_c_int)

  dst = oh_context_transbound1(context, 0_c_int, 0_c_int)
  dst = oh_context_transbound2(context, 0_c_int, 0_c_int)
  dst = oh_context_transbound3(context, 0_c_int, 0_c_int)
  call oh_context_grid_size(context, field)
  dst = oh_context_map_particle_to_neighbor(context, x, y, z, 0_c_int)
  dst = oh_context_map_particle_to_subdomain(context, x, y, z)

  call oh_context_bcast_field(context, c_loc(field(1)), c_loc(field(1)), &
                              0_c_int)
  call oh_context_reduce_field(context, c_loc(field(1)), c_loc(field(1)), &
                               0_c_int)
  call oh_context_allreduce_field(context, c_loc(field(1)), c_loc(field(1)), &
                                  0_c_int)
  call oh_context_exchange_borders(context, c_loc(field(1)), c_loc(field(1)), &
                                   0_c_int, 0_c_int)

  call oh_context_inject_particle(context, c_loc(particle))
  injected = oh_context_inject_particle_get(context, c_loc(particle))
  call oh_context_remap_injected_particle(context, injected)
  call oh_context_remove_injected_particle(context, injected)

  call oh_context_set_particle_adapter(context)
  call oh_particle_adapter_destroy(adapter)
end program
