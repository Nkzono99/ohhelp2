program test_oh_v2_fortran_runtime
  use iso_c_binding
  use mpi
  use ohhelp_v2
  implicit none

  type, bind(C) :: pic_particle
    real(c_double) :: x
    real(c_double) :: y
    real(c_double) :: z
    integer(c_int) :: region
    integer(c_int) :: species
  end type

  type(oh_context_handle) :: context
  type(oh_particle_adapter_handle) :: adapter
  type(oh_particle_adapter_handle) :: callback_adapter
  type(pic_particle), target :: particle
  type(pic_particle), allocatable, target :: callback_particles(:)
  real(c_double), allocatable, target :: weights(:)
  integer(c_int), allocatable, target :: callback_nphgram(:)
  integer(c_int), target :: callback_sdid(2)
  integer(c_int), target :: callback_totalp(2)
  integer(c_int), target :: callback_pbase(3)
  type(c_ptr) :: callback_particles_ptr
  type(c_ptr) :: callback_sdid_ptr
  type(c_ptr) :: callback_nphgram_ptr
  type(c_ptr) :: callback_totalp_ptr
  type(c_ptr) :: callback_pbase_ptr
  integer(c_size_t) :: region_offset
  integer(c_size_t) :: species_offset
  integer(c_size_t) :: x_offset
  integer(c_size_t) :: y_offset
  integer(c_size_t) :: z_offset
  integer(c_int) :: mode
  integer(c_int) :: ierr
  integer :: mpierr
  integer :: rank
  integer :: nranks

  call MPI_Init(mpierr)
  call MPI_Comm_rank(MPI_COMM_WORLD, rank, mpierr)
  call MPI_Comm_size(MPI_COMM_WORLD, nranks, mpierr)

  call oh_context_create(context, int(MPI_COMM_WORLD, c_int), ierr)
  if (ierr /= 0_c_int) stop 1
  if (.not. oh_context_associated(context)) stop 2

  allocate(weights(nranks))
  weights = 1.0_c_double
  weights(1) = 2.0_c_double
  call oh_context_configure_particles(context, 1_c_int, 20_c_int)
  call oh_context_set_region_weights(context, weights)
  call oh_context_set_region_weights(context)

  call oh_particle_adapter_create_byte(adapter, c_sizeof(particle), ierr)
  if (ierr /= 0_c_int) stop 3
  if (.not. oh_particle_adapter_associated(adapter)) stop 4
  if (oh_particle_adapter_validate(adapter) == 0_c_int) stop 5

  region_offset = oh_particle_field_offset(c_loc(particle), &
                                           c_loc(particle%region))
  species_offset = oh_particle_field_offset(c_loc(particle), &
                                            c_loc(particle%species))
  x_offset = oh_particle_field_offset(c_loc(particle), c_loc(particle%x))
  y_offset = oh_particle_field_offset(c_loc(particle), c_loc(particle%y))
  z_offset = oh_particle_field_offset(c_loc(particle), c_loc(particle%z))

  call oh_particle_adapter_use_integer_fields( &
    adapter, region_offset, c_sizeof(particle%region), &
    species_offset, c_sizeof(particle%species))
  call oh_particle_adapter_set_species_base(adapter, 1_c_int)
  call oh_particle_adapter_use_level3_position_fields(adapter, x_offset, &
                                                      y_offset, z_offset)
  call oh_context_set_particle_adapter(context, adapter)
  call oh_context_set_particle_adapter(context)

  call oh_particle_adapter_create_byte(callback_adapter, c_sizeof(particle), &
                                       ierr)
  if (ierr /= 0_c_int) stop 6
  call oh_particle_adapter_set_callbacks(callback_adapter, &
    c_funloc(callback_get_region), c_funloc(callback_set_region), &
    c_funloc(callback_get_species), c_funloc(callback_map_region), &
    c_funloc(callback_map_region))
  call oh_particle_adapter_set_species_base(callback_adapter, 1_c_int)
  if (oh_particle_adapter_validate(callback_adapter) == 0_c_int) stop 7
  call oh_context_set_particle_adapter(context, callback_adapter)
  call oh_particle_adapter_destroy(callback_adapter)

  allocate(callback_particles(4))
  allocate(callback_nphgram(2 * nranks))
  callback_particles = pic_particle(0.5_c_double, 0.5_c_double, &
                                    0.5_c_double, int(rank, c_int), &
                                    1_c_int)
  callback_sdid = [int(rank, c_int), -1_c_int]
  callback_nphgram = 0_c_int
  if (nranks == 2) then
    callback_particles(1)%region = int(1 - rank, c_int)
    callback_nphgram(2 - rank) = 1_c_int
  else
    callback_nphgram(rank + 1) = 1_c_int
  end if
  callback_totalp = 0_c_int
  callback_pbase = 0_c_int
  callback_particles_ptr = c_loc(callback_particles(1))
  callback_sdid_ptr = c_loc(callback_sdid(1))
  callback_nphgram_ptr = c_loc(callback_nphgram(1))
  callback_totalp_ptr = c_loc(callback_totalp(1))
  callback_pbase_ptr = c_loc(callback_pbase(1))

  call oh_context_bind_region_ids(context, callback_sdid_ptr, &
                                  OH_PARTICLES_BORROWED)
  call oh_context_bind_particles(context, callback_particles_ptr, 4_c_int, &
                                 OH_PARTICLES_BORROWED)
  call oh_context_bind_particle_accounting(context, callback_nphgram_ptr, &
                                           callback_totalp_ptr, &
                                           callback_pbase_ptr, &
                                           OH_PARTICLES_BORROWED)
  call oh_context_set_total_particles(context)
  mode = oh_context_transbound2(context, OH_MODE_NORMAL_PRIMARY, 0_c_int)
  if (mode /= OH_MODE_NORMAL_PRIMARY) stop 8
  if (nranks == 2) then
    if (callback_pbase(3) /= 1_c_int) stop 9
    if (callback_particles(1)%region /= int(rank, c_int)) stop 10
    if (callback_particles(1)%species /= 1_c_int) stop 11
  end if
  call oh_context_unbind_particle_accounting(context)
  call oh_context_unbind_particles(context)
  call oh_context_unbind_region_ids(context)

  call oh_particle_adapter_destroy(adapter)
  call oh_context_destroy(context)
  deallocate(callback_nphgram)
  deallocate(callback_particles)
  deallocate(weights)

  call MPI_Finalize(mpierr)
contains
  function callback_get_region(adapter_ptr, particle_ptr, &
                               primary_or_secondary) result(region) bind(C)
    type(c_ptr), value :: adapter_ptr
    type(c_ptr), value :: particle_ptr
    integer(c_int), value :: primary_or_secondary
    integer(c_long_long) :: region
    type(pic_particle), pointer :: part

    call c_f_pointer(particle_ptr, part)
    region = int(part%region, c_long_long)
    if (primary_or_secondary /= 0_c_int .and. part%region < 0_c_int) &
      region = -1_c_long_long
    if (.not. c_associated(adapter_ptr)) region = region
  end function

  subroutine callback_set_region(adapter_ptr, particle_ptr, region, &
                                 primary_or_secondary) bind(C)
    type(c_ptr), value :: adapter_ptr
    type(c_ptr), value :: particle_ptr
    integer(c_long_long), value :: region
    integer(c_int), value :: primary_or_secondary
    type(pic_particle), pointer :: part

    call c_f_pointer(particle_ptr, part)
    part%region = int(region, c_int)
    if (primary_or_secondary < 0_c_int .and. c_associated(adapter_ptr)) &
      part%region = part%region
  end subroutine

  function callback_get_species(adapter_ptr, particle_ptr) result(species) &
      bind(C)
    type(c_ptr), value :: adapter_ptr
    type(c_ptr), value :: particle_ptr
    integer(c_int) :: species
    type(pic_particle), pointer :: part

    call c_f_pointer(particle_ptr, part)
    species = part%species
    if (.not. c_associated(adapter_ptr)) species = species
  end function

  function callback_map_region(adapter_ptr, particle_ptr, &
                               primary_or_secondary) result(region) bind(C)
    type(c_ptr), value :: adapter_ptr
    type(c_ptr), value :: particle_ptr
    integer(c_int), value :: primary_or_secondary
    integer(c_long_long) :: region

    region = callback_get_region(adapter_ptr, particle_ptr, &
                                 primary_or_secondary)
  end function
end program
