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
  real(c_double), target :: weights(1)
  real(c_double), target :: field(1)
  real(c_double), target :: x, y, z
  integer(c_size_t) :: region_offset, species_offset
  integer(c_size_t) :: x_offset, y_offset, z_offset
  integer(c_int) :: ierr
  integer(c_int) :: dst
  type(c_ptr) :: injected

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
