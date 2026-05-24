program test_oh_context_lifecycle_fortran
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

  type(oh_context_handle) :: context_x
  type(oh_context_handle) :: context_y
  type(oh_particle_adapter_handle) :: adapter
  type(c_ptr) :: particles_x
  type(c_ptr) :: particles_y
  type(c_ptr) :: nphgram_x
  type(c_ptr) :: nphgram_y
  type(c_ptr) :: totalp_x
  type(c_ptr) :: totalp_y
  type(c_ptr) :: pbase_x
  type(c_ptr) :: pbase_y
  type(c_ptr) :: sdid_x_ptr
  type(c_ptr) :: sdid_y_ptr
  real(c_double), target :: field(8)
  real(c_double), target :: coord_x(3)
  real(c_double), target :: coord_y(3)
  type(pic_particle), target :: particle
  type(pic_particle), target :: particle_buffer_x(16)
  integer(c_int), target :: sdid_x(2)
  integer(c_int), target :: sdid_y(2)
  integer(c_int), target :: copied_sdid(2)
  integer(c_int), target :: nphgram_x_store(4)
  integer(c_int), target :: totalp_x_store(2)
  integer(c_int), target :: pbase_x_store(3)
  integer(c_int), pointer :: pbase_x_values(:)
  integer(c_size_t) :: region_offset
  integer(c_size_t) :: species_offset
  integer(c_size_t) :: x_offset
  integer(c_size_t) :: y_offset
  integer(c_size_t) :: z_offset
  integer :: mpierr
  integer :: rank
  integer :: nranks
  integer(c_int) :: ierr
  integer(c_int) :: mode

  call MPI_Init(mpierr)
  call MPI_Comm_rank(MPI_COMM_WORLD, rank, mpierr)
  call MPI_Comm_size(MPI_COMM_WORLD, nranks, mpierr)

  call oh_context_create(context_x, int(MPI_COMM_WORLD, c_int), ierr)
  if (ierr /= 0_c_int) stop 1
  call oh_context_create(context_y, int(MPI_COMM_WORLD, c_int), ierr)
  if (ierr /= 0_c_int) stop 2
  if (oh_context_max_local_particles_for_capacity( &
        context_x, 1000_c_long_long, 250_c_int, 8_c_int) /= &
      ((1000_c_int - 1_c_int) / int(nranks, c_int) + 1_c_int) + &
      ((((1000_c_int - 1_c_int) / int(nranks, c_int) + 1_c_int) * &
        250_c_int - 1_c_int) / 100_c_int + 1_c_int)) stop 27

  call configure_context(context_x, nranks, 1, .false.)
  call configure_context(context_y, nranks, 2, .true.)

  call oh_particle_adapter_create_byte(adapter, c_sizeof(particle), ierr)
  if (ierr /= 0_c_int) stop 28
  region_offset = oh_particle_field_offset(c_loc(particle), &
                                           c_loc(particle%region))
  species_offset = oh_particle_field_offset(c_loc(particle), &
                                            c_loc(particle%species))
  x_offset = oh_particle_field_offset(c_loc(particle), c_loc(particle%x))
  y_offset = oh_particle_field_offset(c_loc(particle), c_loc(particle%y))
  z_offset = oh_particle_field_offset(c_loc(particle), c_loc(particle%z))
  call oh_particle_adapter_use_int_fields(adapter, region_offset, &
                                          species_offset)
  call oh_particle_adapter_use_level3_position_fields(adapter, x_offset, &
                                                      y_offset, z_offset)
  call oh_context_set_particle_adapter(context_x, adapter)

  field = 0.0_c_double
  coord_x = 0.5_c_double
  coord_y = 0.5_c_double
  coord_x(1) = (real(rank, c_double) + 0.5_c_double) / &
               real(nranks, c_double)
  coord_y(2) = (real(rank, c_double) + 0.5_c_double) / &
               real(nranks, c_double)

  if (oh_context_map_particle_to_subdomain( &
        context_x, coord_x(1), coord_x(2), coord_x(3)) /= int(rank, c_int)) &
    stop 9
  if (oh_context_map_particle_to_subdomain( &
        context_y, coord_y(1), coord_y(2), coord_y(3)) /= int(rank, c_int)) &
    stop 10
  if (oh_context_map_particle_to_neighbor( &
        context_x, coord_x(1), coord_x(2), coord_x(3), 0_c_int) &
      /= int(rank, c_int)) stop 11
  if (oh_context_map_particle_to_neighbor( &
        context_y, coord_y(1), coord_y(2), coord_y(3), 0_c_int) &
      /= int(rank, c_int)) stop 12

  call oh_context_bcast_field(context_x, c_loc(field(1)), c_loc(field(1)), &
                              0_c_int)
  call oh_context_reduce_field(context_x, c_loc(field(1)), c_loc(field(1)), &
                               0_c_int)
  call oh_context_allreduce_field(context_x, c_loc(field(1)), c_loc(field(1)), &
                                  0_c_int)
  call oh_context_bcast_field(context_y, c_loc(field(1)), c_loc(field(1)), &
                              0_c_int)
  call oh_context_reduce_field(context_y, c_loc(field(1)), c_loc(field(1)), &
                               0_c_int)
  call oh_context_allreduce_field(context_y, c_loc(field(1)), c_loc(field(1)), &
                                  0_c_int)

  sdid_x = [int(rank, c_int), -1_c_int]
  sdid_y = [int(rank, c_int), -1_c_int]
  sdid_x_ptr = c_loc(sdid_x(1))
  sdid_y_ptr = c_loc(sdid_y(1))
  call oh_context_bind_region_ids(context_x, sdid_x_ptr, &
                                  OH_PARTICLES_BORROWED)
  call oh_context_bind_region_ids(context_y, sdid_y_ptr, &
                                  OH_PARTICLES_BORROWED)

  particle_buffer_x = pic_particle(0.0_c_double, 0.0_c_double, 0.0_c_double, &
                                   0_c_int, 1_c_int)
  particles_x = c_loc(particle_buffer_x(1))
  particles_y = c_null_ptr
  call oh_context_bind_particles(context_x, particles_x, 16_c_int, &
                                 OH_PARTICLES_BORROWED)
  call oh_context_bind_particles(context_y, particles_y, 4_c_int, &
                                 OH_PARTICLES_OWNED)
  if (.not. c_associated(particles_x)) stop 3
  if (.not. c_associated(particles_y)) stop 4
  if (c_associated(particles_x, particles_y)) stop 5

  nphgram_x_store = 0_c_int
  totalp_x_store = 0_c_int
  pbase_x_store = 0_c_int
  nphgram_x = c_loc(nphgram_x_store(1))
  totalp_x = c_loc(totalp_x_store(1))
  pbase_x = c_loc(pbase_x_store(1))
  nphgram_y = c_null_ptr
  totalp_y = c_null_ptr
  pbase_y = c_null_ptr
  call oh_context_bind_particle_accounting(context_x, nphgram_x, totalp_x, &
                                           pbase_x, OH_PARTICLES_BORROWED)
  call oh_context_bind_particle_accounting(context_y, nphgram_y, totalp_y, &
                                           pbase_y, OH_PARTICLES_OWNED)
  if (.not. c_associated(nphgram_x)) stop 6
  if (.not. c_associated(totalp_x)) stop 7
  if (.not. c_associated(pbase_x)) stop 8
  if (.not. c_associated(nphgram_y)) stop 13
  if (.not. c_associated(totalp_y)) stop 14
  if (.not. c_associated(pbase_y)) stop 15
  if (c_associated(nphgram_x, nphgram_y)) stop 16
  if (c_associated(totalp_x, totalp_y)) stop 17
  if (c_associated(pbase_x, pbase_y)) stop 18

  mode = oh_context_transbound1(context_x, OH_MODE_NORMAL_PRIMARY, 0_c_int)
  if (mode /= OH_MODE_NORMAL_PRIMARY) stop 19
  mode = oh_context_transbound2(context_x, OH_MODE_NORMAL_PRIMARY, 0_c_int)
  if (mode /= OH_MODE_NORMAL_PRIMARY) stop 20
  mode = oh_context_transbound3(context_x, OH_MODE_NORMAL_PRIMARY, 0_c_int)
  if (mode /= OH_MODE_NORMAL_PRIMARY) stop 21
  mode = oh_context_transbound1(context_y, OH_MODE_NORMAL_PRIMARY, 0_c_int)
  if (mode /= OH_MODE_NORMAL_PRIMARY) stop 22
  mode = oh_context_transbound2(context_y, OH_MODE_NORMAL_PRIMARY, 0_c_int)
  if (mode /= OH_MODE_NORMAL_PRIMARY) stop 23
  mode = oh_context_transbound3(context_y, OH_MODE_NORMAL_PRIMARY, 0_c_int)
  if (mode /= OH_MODE_NORMAL_PRIMARY) stop 24
  call oh_context_get_region_ids(context_x, c_loc(copied_sdid(1)))
  if (any(copied_sdid /= sdid_x)) stop 25
  call oh_context_get_region_ids(context_y, c_loc(copied_sdid(1)))
  if (any(copied_sdid /= sdid_y)) stop 26

  if (nranks == 2) then
    particle%x = 0.25_c_double
    particle%y = 0.5_c_double
    particle%z = 0.5_c_double
    particle%region = 0_c_int
    particle%species = 1_c_int
    if (rank == 0) then
      do mode = 1, 8
        call oh_context_inject_particle(context_x, c_loc(particle))
      end do
    end if
    call oh_context_set_total_particles(context_x)
    mode = oh_context_transbound3(context_x, OH_MODE_NORMAL_PRIMARY, 0_c_int)
    if (mode /= OH_MODE_REBALANCE_SECONDARY) stop 29
    call oh_context_get_region_ids(context_x, c_loc(copied_sdid(1)))
    call c_f_pointer(pbase_x, pbase_x_values, [3])
    if (rank == 0) then
      if (copied_sdid(2) /= -1_c_int) stop 30
    else
      if (copied_sdid(2) /= 0_c_int) stop 31
      if (pbase_x_values(3) <= 0_c_int) stop 32
    end if
  end if

  call oh_context_unbind_region_ids(context_x)
  call oh_context_unbind_particle_accounting(context_x)
  call oh_context_unbind_particles(context_x)
  call oh_context_unbind_region_ids(context_y)
  call oh_context_unbind_particle_accounting(context_y)
  call oh_context_unbind_particles(context_y)
  call oh_context_destroy(context_x)
  call oh_context_destroy(context_y)
  call oh_particle_adapter_destroy(adapter)

  call MPI_Finalize(mpierr)

contains
  subroutine configure_context(context, nranks_value, axis, legacy_level3)
    type(oh_context_handle), intent(in) :: context
    integer, intent(in) :: nranks_value
    integer, intent(in) :: axis
    logical, intent(in) :: legacy_level3
    integer(c_int), target :: pcoord(3)
    integer(c_int), target :: scoord(2,3)
    integer(c_int), target :: sdoms(2,3,1)
    integer(c_int), target :: bcond(2,3)
    integer(c_int), target :: ftypes(7,2)
    integer(c_int), target :: cfields(1)
    integer(c_int), target :: ctypes(3,2,1,1)
    integer(c_int), target :: fsizes(2,3,1)
    real(c_double), target :: gsize(3)
    integer :: i

    call oh_context_configure_particles(context, 1_c_int, 20_c_int)
    pcoord = 1_c_int
    pcoord(axis) = int(nranks_value, c_int)
    scoord = 0_c_int
    bcond = 0_c_int
    sdoms = 0_c_int
    sdoms(1, 1, 1) = 0_c_int
    sdoms(2, 1, 1) = -1_c_int
    do i = 1, 3
      scoord(2, i) = pcoord(i)
      gsize(i) = 1.0_c_double / real(pcoord(i), c_double)
    end do
    if (legacy_level3) bcond = 1_c_int
    ftypes = 0_c_int
    ftypes(1, 1) = 1_c_int
    cfields(1) = -1_c_int
    ctypes = 0_c_int
    fsizes = 0_c_int
    if (legacy_level3) then
      call oh_context_configure_level3_legacy( &
        context, c_loc(pcoord(1)), c_loc(sdoms(1,1,1)), &
        c_loc(scoord(1,1)), 1_c_int, c_loc(bcond(1,1)), c_null_ptr, &
        c_loc(ftypes(1,1)), c_loc(cfields(1)), &
        c_loc(ctypes(1,1,1,1)), c_loc(fsizes(1,1,1)))
    else
      call oh_context_configure_level3(context, c_loc(pcoord(1)), c_null_ptr, &
                                       c_loc(scoord(1,1)), 1_c_int, &
                                       c_loc(bcond(1,1)), c_null_ptr, &
                                       c_loc(ftypes(1,1)), c_loc(cfields(1)), &
                                       c_loc(ctypes(1,1,1,1)), &
                                       c_loc(fsizes(1,1,1)))
    end if
    call oh_context_grid_size(context, gsize)
  end subroutine
end program
