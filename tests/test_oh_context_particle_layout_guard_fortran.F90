program test_oh_context_particle_layout_guard_fortran
  use iso_c_binding
  use ieee_arithmetic
  use mpi
  use ohhelp_v2
  implicit none
#include "oh_config.h"

  type, bind(C) :: guard_particle
    integer(c_int) :: region
    integer(c_int) :: species
  end type

  type(oh_context_handle) :: context
  type(oh_particle_adapter_handle) :: adapter
  type(guard_particle), target :: guard_particles(1)
  type(guard_particle), target :: injected_particle
  type(c_ptr) :: particles
  type(c_ptr) :: copy_ptr
  type(c_ptr) :: sdid_ptr
  type(c_ptr) :: nphgram_ptr
  type(c_ptr) :: totalp_ptr
  type(c_ptr) :: pbase_ptr
  character(len=32) :: mode_arg
  integer(c_int), target :: particle_storage
  integer(c_int), target :: sdid(2)
  integer(c_int), target :: nphgram(2)
  integer(c_int), target :: totalp(2)
  integer(c_int), target :: pbase(3)
  real(c_double), target :: x
  real(c_double), target :: y
  real(c_double), target :: field(1)
  real(c_double), target :: weights(1)
  real(c_double), target :: long_weights(2)
  real(c_double), target :: grid_size(OH_DIMENSION)
  real(c_double), allocatable, target :: short_grid_size(:)
  real(c_double), allocatable, target :: empty_weights(:)
  real(c_double), allocatable, target :: empty_grid_size(:)
  integer(c_size_t) :: region_offset
  integer(c_size_t) :: species_offset
  integer(c_int) :: ierr
  integer :: mpierr

  call MPI_Init(mpierr)
  call oh_context_create(context, int(MPI_COMM_WORLD, c_int), ierr)
  if (ierr /= 0_c_int) stop 1
  allocate(empty_weights(0))
  allocate(empty_grid_size(0))
  allocate(short_grid_size(OH_DIMENSION - 1))

  call get_command_argument(1, mode_arg)
  if (trim(mode_arg) == "null-context") then
    call oh_context_destroy(context)
    call oh_context_configure_particles(context, 1_c_int, 0_c_int)
  end if
  if (trim(mode_arg) == "null-bind-particles") then
    call oh_context_destroy(context)
    particles = c_null_ptr
    call oh_context_bind_particles(context, particles, 0_c_int, &
                                   OH_PARTICLES_BORROWED)
  end if
  if (trim(mode_arg) == "null-bind-region-ids") then
    call oh_context_destroy(context)
    sdid_ptr = c_null_ptr
    call oh_context_bind_region_ids(context, sdid_ptr, OH_PARTICLES_BORROWED)
  end if
  if (trim(mode_arg) == "null-get-region-ids") then
    call oh_context_destroy(context)
    call oh_context_get_region_ids(context, c_null_ptr)
  end if
  if (trim(mode_arg) == "null-bind-accounting") then
    call oh_context_destroy(context)
    nphgram_ptr = c_null_ptr
    totalp_ptr = c_null_ptr
    pbase_ptr = c_null_ptr
    call oh_context_bind_particle_accounting(context, nphgram_ptr, &
                                             totalp_ptr, pbase_ptr, &
                                             OH_PARTICLES_BORROWED)
  end if
  if (trim(mode_arg) == "null-transbound3") then
    call oh_context_destroy(context)
    ierr = oh_context_transbound3(context, OH_MODE_NORMAL_PRIMARY, 0_c_int)
  end if
  if (trim(mode_arg) == "null-grid-size") then
    call oh_context_destroy(context)
    grid_size = 0.0_c_double
    call oh_context_grid_size(context, grid_size)
  end if
  if (trim(mode_arg) == "null-inject") then
    call oh_context_destroy(context)
    call oh_context_inject_particle(context, c_null_ptr)
  end if
  if (trim(mode_arg) == "null-region-weights") then
    weights = 1.0_c_double
    call oh_context_destroy(context)
    call oh_context_set_region_weights(context, weights)
  end if
  if (trim(mode_arg) == "default-weights-before-init") then
    weights = 1.0_c_double
    context = oh_default_context()
    call oh_context_set_region_weights(context, weights)
  end if
  if (trim(mode_arg) == "short-weights") then
    call oh_context_set_region_weights(context, empty_weights)
  end if
  if (trim(mode_arg) == "long-weights") then
    long_weights = 1.0_c_double
    call oh_context_set_region_weights(context, long_weights)
  end if
  if (trim(mode_arg) == "short-weights-n2") then
    weights = 1.0_c_double
    call oh_context_set_region_weights(context, weights)
  end if
  if (trim(mode_arg) == "short-grid-size") then
    call oh_context_grid_size(context, empty_grid_size)
  end if
  if (trim(mode_arg) == "short-grid-size-nonempty") then
    short_grid_size = 0.0_c_double
    call oh_context_grid_size(context, short_grid_size)
  end if
  if (trim(mode_arg) == "missing-y") then
    x = 0.0_c_double
    ierr = oh_context_map_particle_to_neighbor(context, x, ps=0_c_int)
  end if
  if (trim(mode_arg) == "missing-z") then
    x = 0.0_c_double
    y = 0.0_c_double
    ierr = oh_context_map_particle_to_neighbor(context, x, y, ps=0_c_int)
  end if
  if (trim(mode_arg) == "missing-subdomain-y") then
    x = 0.0_c_double
    ierr = oh_context_map_particle_to_subdomain(context, x)
  end if
  if (trim(mode_arg) == "missing-subdomain-z") then
    x = 0.0_c_double
    y = 0.0_c_double
    ierr = oh_context_map_particle_to_subdomain(context, x, y)
  end if
  if (trim(mode_arg) == "unconfigured") then
    particles = c_null_ptr
    call oh_context_bind_particles(context, particles, 0_c_int, &
                                   OH_PARTICLES_BORROWED)
  end if
  if (trim(mode_arg) == "configure-zero-species") then
    call oh_context_configure_particles(context, 0_c_int, 0_c_int)
  end if
  if (trim(mode_arg) == "configure-negative-maxfrac") then
    call oh_context_configure_particles(context, 1_c_int, -1_c_int)
  end if

  call oh_context_configure_particles(context, 1_c_int, 0_c_int)
  if (trim(mode_arg) == "short-weights-configured") then
    call oh_context_set_region_weights(context, empty_weights)
  end if
  if (trim(mode_arg) == "long-weights-configured") then
    long_weights = 1.0_c_double
    call oh_context_set_region_weights(context, long_weights)
  end if
  if (trim(mode_arg) == "short-weights-n2-configured") then
    weights = 1.0_c_double
    call oh_context_set_region_weights(context, weights)
  end if
  if (trim(mode_arg) == "negative-region-weight") then
    weights = -1.0_c_double
    call oh_context_set_region_weights(context, weights)
  end if
  if (trim(mode_arg) == "zero-region-weight") then
    weights = 0.0_c_double
    call oh_context_set_region_weights(context, weights)
  end if
  if (trim(mode_arg) == "nan-region-weight") then
    weights = ieee_value(0.0_c_double, ieee_quiet_nan)
    call oh_context_set_region_weights(context, weights)
  end if
  if (trim(mode_arg) == "inf-region-weight") then
    weights = ieee_value(0.0_c_double, ieee_positive_inf)
    call oh_context_set_region_weights(context, weights)
  end if
  sdid = [0_c_int, -1_c_int]
  sdid_ptr = c_loc(sdid(1))
  if (trim(mode_arg) == "owned-region") then
    call oh_context_bind_region_ids(context, sdid_ptr, OH_PARTICLES_OWNED)
  end if
  if (trim(mode_arg) == "borrowed-region-null") then
    sdid_ptr = c_null_ptr
    call oh_context_bind_region_ids(context, sdid_ptr, OH_PARTICLES_BORROWED)
  end if
  if (trim(mode_arg) == "invalid-region-ownership") then
    call oh_context_bind_region_ids(context, sdid_ptr, 99_c_int)
  end if
  if (trim(mode_arg) == "get-region-null") then
    call oh_context_get_region_ids(context, c_null_ptr)
  end if
  if (trim(mode_arg) == "owned-particle") then
    particle_storage = 0_c_int
    particles = c_loc(particle_storage)
    call oh_context_bind_particles(context, particles, 1_c_int, &
                                   OH_PARTICLES_OWNED)
  end if
  if (trim(mode_arg) == "invalid-particle-ownership") then
    particle_storage = 0_c_int
    particles = c_loc(particle_storage)
    call oh_context_bind_particles(context, particles, 1_c_int, 99_c_int)
  end if
  if (trim(mode_arg) == "borrowed-particle-null") then
    particles = c_null_ptr
    call oh_context_bind_particles(context, particles, 1_c_int, &
                                   OH_PARTICLES_BORROWED)
  end if
  if (trim(mode_arg) == "owned-accounting") then
    nphgram = 0_c_int
    totalp = 0_c_int
    pbase = 0_c_int
    nphgram_ptr = c_loc(nphgram(1))
    totalp_ptr = c_loc(totalp(1))
    pbase_ptr = c_loc(pbase(1))
    call oh_context_bind_particle_accounting(context, nphgram_ptr, &
                                             totalp_ptr, pbase_ptr, &
                                             OH_PARTICLES_OWNED)
  end if
  if (trim(mode_arg) == "invalid-accounting-ownership") then
    nphgram = 0_c_int
    totalp = 0_c_int
    pbase = 0_c_int
    nphgram_ptr = c_loc(nphgram(1))
    totalp_ptr = c_loc(totalp(1))
    pbase_ptr = c_loc(pbase(1))
    call oh_context_bind_particle_accounting(context, nphgram_ptr, &
                                             totalp_ptr, pbase_ptr, &
                                             99_c_int)
  end if
  if (trim(mode_arg) == "borrowed-accounting-null") then
    nphgram_ptr = c_null_ptr
    totalp_ptr = c_null_ptr
    pbase_ptr = c_null_ptr
    call oh_context_bind_particle_accounting(context, nphgram_ptr, &
                                             totalp_ptr, pbase_ptr, &
                                             OH_PARTICLES_BORROWED)
  end if
  if (trim(mode_arg) == "borrowed-accounting-null-pbase") then
    nphgram = 0_c_int
    totalp = 0_c_int
    nphgram_ptr = c_loc(nphgram(1))
    totalp_ptr = c_loc(totalp(1))
    pbase_ptr = c_null_ptr
    call oh_context_bind_particle_accounting(context, nphgram_ptr, &
                                             totalp_ptr, pbase_ptr, &
                                             OH_PARTICLES_BORROWED)
  end if
  if (trim(mode_arg) == "owned-accounting-nonnull-pbase") then
    nphgram_ptr = c_null_ptr
    totalp_ptr = c_null_ptr
    pbase = 0_c_int
    pbase_ptr = c_loc(pbase(1))
    call oh_context_bind_particle_accounting(context, nphgram_ptr, &
                                             totalp_ptr, pbase_ptr, &
                                             OH_PARTICLES_OWNED)
  end if
  if (trim(mode_arg) == "level3-field-unconfigured") then
    field = 0.0_c_double
    call oh_context_configure_level3(context, c_null_ptr, c_null_ptr, &
                                     c_null_ptr, 0_c_int, c_null_ptr, &
                                     c_null_ptr, c_null_ptr, c_null_ptr, &
                                     c_null_ptr, c_null_ptr)
    call oh_context_bcast_field(context, c_loc(field(1)), c_loc(field(1)), &
                                0_c_int)
  end if
  if (trim(mode_arg) == "level3-exchange-unconfigured") then
    field = 0.0_c_double
    call oh_context_configure_level3(context, c_null_ptr, c_null_ptr, &
                                     c_null_ptr, 0_c_int, c_null_ptr, &
                                     c_null_ptr, c_null_ptr, c_null_ptr, &
                                     c_null_ptr, c_null_ptr)
    call oh_context_exchange_borders(context, c_loc(field(1)), &
                                     c_loc(field(1)), 0_c_int, 0_c_int)
  end if
  if (trim(mode_arg) == "remap-finalized-injected-copy" .or. &
      trim(mode_arg) == "remove-finalized-injected-copy" .or. &
      trim(mode_arg) == "remap-original-injected-source" .or. &
      trim(mode_arg) == "remove-original-injected-source") then
    guard_particles = guard_particle(-1_c_int, 1_c_int)
    injected_particle = guard_particle(0_c_int, 1_c_int)
    nphgram = 0_c_int
    totalp = 0_c_int
    pbase = 0_c_int
    call oh_particle_adapter_create_byte(adapter, &
                                         c_sizeof(injected_particle), ierr)
    if (ierr /= 0_c_int) stop 2
    region_offset = oh_particle_field_offset(c_loc(injected_particle), &
                                             c_loc(injected_particle%region))
    species_offset = oh_particle_field_offset(c_loc(injected_particle), &
                                              c_loc(injected_particle%species))
    call oh_particle_adapter_use_int_fields(adapter, region_offset, &
                                            species_offset)
    call oh_context_set_particle_adapter(context, adapter)
    particles = c_loc(guard_particles(1))
    nphgram_ptr = c_loc(nphgram(1))
    totalp_ptr = c_loc(totalp(1))
    pbase_ptr = c_loc(pbase(1))
    call oh_context_bind_region_ids(context, sdid_ptr, OH_PARTICLES_BORROWED)
    call oh_context_bind_particles(context, particles, 1_c_int, &
                                   OH_PARTICLES_BORROWED)
    call oh_context_bind_particle_accounting(context, nphgram_ptr, &
                                             totalp_ptr, pbase_ptr, &
                                             OH_PARTICLES_BORROWED)
    call oh_context_set_total_particles(context)
    copy_ptr = oh_context_inject_particle_get(context, &
                                              c_loc(injected_particle))
    if (trim(mode_arg) == "remap-original-injected-source") then
      call oh_context_remap_injected_particle(context, &
                                              c_loc(injected_particle))
    end if
    if (trim(mode_arg) == "remove-original-injected-source") then
      call oh_context_remove_injected_particle(context, &
                                               c_loc(injected_particle))
    end if
    call oh_context_set_total_particles(context)
    if (trim(mode_arg) == "remove-finalized-injected-copy") then
      call oh_context_remove_injected_particle(context, copy_ptr)
    else
      call oh_context_remap_injected_particle(context, copy_ptr)
    end if
  end if

  particles = c_null_ptr
  call oh_context_bind_particles(context, particles, 0_c_int, &
                                 OH_PARTICLES_BORROWED)

  if (trim(mode_arg) == "reconfigure") then
    call oh_context_configure_particles(context, 1_c_int, 0_c_int)
  end if

  if (trim(mode_arg) == "adapter") then
    call oh_context_set_particle_adapter(context)
  else if (trim(mode_arg) == "type") then
    call oh_context_set_particle_mpi_type(context, &
                                          int(MPI_DATATYPE_NULL, c_int))
  else if (trim(mode_arg) == "type-extent") then
    call oh_context_unbind_particles(context)
    call oh_context_set_particle_mpi_type(context, int(MPI_BYTE, c_int))
  end if

  call oh_context_unbind_particles(context)
  call oh_context_set_particle_adapter(context)
  call oh_context_set_particle_mpi_type(context, &
                                        int(MPI_DATATYPE_NULL, c_int))
  call oh_context_destroy(context)
  call MPI_Finalize(mpierr)
end program
