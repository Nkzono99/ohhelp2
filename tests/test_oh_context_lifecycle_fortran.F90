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
  real(c_double), target :: raw_pbuf(1)
  real(c_double), target :: raw_sbuf(1)
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
  if (oh_context_region_count(context_x) /= int(nranks, c_int)) stop 52
  if (oh_context_is_level3_configured(context_x)) stop 53
  if (oh_context_max_local_particles_for_capacity( &
        context_x, 1000_c_long_long, 250_c_int, 8_c_int) /= &
      ((1000_c_int - 1_c_int) / int(nranks, c_int) + 1_c_int) + &
      ((((1000_c_int - 1_c_int) / int(nranks, c_int) + 1_c_int) * &
        250_c_int - 1_c_int) / 100_c_int + 1_c_int)) stop 27

  call run_context_owned_comm_test(rank, nranks)

  call configure_context(context_x, nranks, 1, .false.)
  call configure_context(context_y, nranks, 2, .true.)
  if (.not. oh_context_is_level3_configured(context_x)) stop 54

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
  call oh_particle_adapter_use_level3_neighbor_position_fields( &
    adapter, x_offset, y_offset, z_offset)
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

  raw_pbuf = real(rank + 1, c_double)
  raw_sbuf = -1.0_c_double
  call oh_context_broadcast(context_x, c_loc(raw_pbuf(1)), &
                            c_loc(raw_sbuf(1)), 1_c_int, 1_c_int, &
                            int(MPI_DOUBLE, c_int), int(MPI_DOUBLE, c_int))
  raw_pbuf = real(rank + 1, c_double)
  raw_sbuf = real(rank + 1, c_double)
  call oh_context_all_reduce(context_x, c_loc(raw_pbuf(1)), &
                             c_loc(raw_sbuf(1)), 1_c_int, 1_c_int, &
                             int(MPI_DOUBLE, c_int), &
                             int(MPI_DOUBLE, c_int), int(MPI_SUM, c_int), &
                             int(MPI_SUM, c_int))
  raw_pbuf = real(rank + 1, c_double)
  raw_sbuf = real(rank + 1, c_double)
  call oh_context_reduce(context_x, c_loc(raw_pbuf(1)), c_loc(raw_sbuf(1)), &
                         1_c_int, 1_c_int, int(MPI_DOUBLE, c_int), &
                         int(MPI_DOUBLE, c_int), int(MPI_SUM, c_int), &
                         int(MPI_SUM, c_int))

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
    call run_context_collective_value_test(context_x, rank)
    call oh_context_get_region_ids(context_x, c_loc(copied_sdid(1)))
    call c_f_pointer(pbase_x, pbase_x_values, [3])
    if (rank == 0) then
      if (copied_sdid(2) /= -1_c_int) stop 30
    else
      if (copied_sdid(2) /= 0_c_int) stop 31
      if (pbase_x_values(3) <= 0_c_int) stop 32
    end if
  end if
  call run_weighted_load_rebalance_test(adapter, rank, nranks)
  call run_region_weight_reset_behavior_test(adapter, rank, nranks)
  call run_legacy_passive_bounds_test(rank, nranks)
  call run_injected_accounting_contract_test(adapter, rank, nranks)

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
  subroutine run_context_owned_comm_test(rank_value, nranks_value)
    integer, intent(in) :: rank_value
    integer, intent(in) :: nranks_value
    type(oh_context_handle) :: context
    integer :: app_comm
    integer :: mpierr_local
    integer(c_int), target :: sdid(2)
    integer(c_int), target :: totalp(2)
    integer(c_int), target :: pbase(3)
    integer(c_int), allocatable, target :: nphgram(:)
    type(c_ptr) :: sdid_ptr
    type(c_ptr) :: nphgram_ptr
    type(c_ptr) :: totalp_ptr
    type(c_ptr) :: pbase_ptr
    integer(c_int) :: ierr_local
    integer(c_int) :: mode_value

    allocate(nphgram(2 * nranks_value))
    nphgram = 0_c_int
    sdid = [int(rank_value, c_int), -1_c_int]
    totalp = 0_c_int
    pbase = 0_c_int

    call MPI_Comm_dup(MPI_COMM_WORLD, app_comm, mpierr_local)
    if (mpierr_local /= MPI_SUCCESS) stop 44
    call oh_context_create(context, int(app_comm, c_int), ierr_local)
    if (ierr_local /= 0_c_int) stop 45
    call MPI_Comm_free(app_comm, mpierr_local)
    if (mpierr_local /= MPI_SUCCESS) stop 46

    sdid_ptr = c_loc(sdid(1))
    nphgram_ptr = c_loc(nphgram(1))
    totalp_ptr = c_loc(totalp(1))
    pbase_ptr = c_loc(pbase(1))
    call oh_context_configure_particles(context, 1_c_int, 0_c_int)
    call oh_context_bind_region_ids(context, sdid_ptr, OH_PARTICLES_BORROWED)
    call oh_context_bind_particle_accounting(context, nphgram_ptr, &
                                             totalp_ptr, pbase_ptr, &
                                             OH_PARTICLES_BORROWED)
    mode_value = oh_context_transbound1(context, OH_MODE_NORMAL_PRIMARY, &
                                        0_c_int)
    if (mode_value /= OH_MODE_NORMAL_PRIMARY) stop 47
    call oh_context_unbind_particle_accounting(context)
    call oh_context_unbind_region_ids(context)
    call oh_context_destroy(context)
    deallocate(nphgram)
  end subroutine

  subroutine run_context_collective_value_test(context, rank_value)
    type(oh_context_handle), intent(in) :: context
    integer, intent(in) :: rank_value
    real(c_double), target :: raw_pbuf(1)
    real(c_double), target :: raw_sbuf(1)

    raw_pbuf(1) = merge(11.0_c_double, -1.0_c_double, rank_value == 0)
    raw_sbuf(1) = merge(-2.0_c_double, -3.0_c_double, rank_value == 0)
    call oh_context_broadcast(context, c_loc(raw_pbuf(1)), &
                              c_loc(raw_sbuf(1)), 1_c_int, 1_c_int, &
                              int(MPI_DOUBLE, c_int), int(MPI_DOUBLE, c_int))
    if (rank_value == 0) then
      if (raw_pbuf(1) /= 11.0_c_double) stop 80
    else
      if (raw_sbuf(1) /= 11.0_c_double) stop 81
    end if

    raw_pbuf(1) = merge(2.0_c_double, -20.0_c_double, rank_value == 0)
    raw_sbuf(1) = merge(-30.0_c_double, 5.0_c_double, rank_value == 0)
    call oh_context_all_reduce(context, c_loc(raw_pbuf(1)), &
                               c_loc(raw_sbuf(1)), 1_c_int, 1_c_int, &
                               int(MPI_DOUBLE, c_int), &
                               int(MPI_DOUBLE, c_int), int(MPI_SUM, c_int), &
                               int(MPI_SUM, c_int))
    if (rank_value == 0) then
      if (raw_pbuf(1) /= 7.0_c_double) stop 82
    else
      if (raw_sbuf(1) /= 7.0_c_double) stop 83
    end if

    raw_pbuf(1) = merge(3.0_c_double, -20.0_c_double, rank_value == 0)
    raw_sbuf(1) = merge(-30.0_c_double, 4.0_c_double, rank_value == 0)
    call oh_context_reduce(context, c_loc(raw_pbuf(1)), c_loc(raw_sbuf(1)), &
                           1_c_int, 1_c_int, int(MPI_DOUBLE, c_int), &
                           int(MPI_DOUBLE, c_int), int(MPI_SUM, c_int), &
                           int(MPI_SUM, c_int))
    if (rank_value == 0) then
      if (raw_pbuf(1) /= 7.0_c_double) stop 84
    end if
  end subroutine

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

  subroutine run_legacy_passive_bounds_test(rank_value, nranks_value)
    integer, intent(in) :: rank_value
    integer, intent(in) :: nranks_value
    type(oh_context_handle) :: context
    integer(c_int), target :: pcoord(3)
    integer(c_int), target :: scoord(2,3)
    integer(c_int), allocatable, target :: sdoms(:,:,:)
    integer(c_int), target :: bcond(2,3)
    integer(c_int), allocatable, target :: bounds(:,:,:)
    integer(c_int), target :: ftypes(7,2)
    integer(c_int), target :: cfields(1)
    integer(c_int), target :: ctypes(3,2,2,1)
    integer(c_int), target :: fsizes(2,3,1)
    real(c_double), target :: gsize(3)
    real(c_double), target :: coord(3)
    real(c_double), target :: field(8)
    integer(c_int) :: mapped
    integer(c_int) :: expected
    integer(c_int) :: ierr
    integer :: i

    call oh_context_create(context, int(MPI_COMM_WORLD, c_int), ierr)
    if (ierr /= 0_c_int) stop 40
    call oh_context_configure_particles(context, 1_c_int, 20_c_int)

    allocate(sdoms(2,3,nranks_value))
    allocate(bounds(2,3,nranks_value))
    pcoord = [int(nranks_value, c_int), 1_c_int, 1_c_int]
    scoord = 0_c_int
    do i = 1, 3
      scoord(2, i) = pcoord(i)
      gsize(i) = 1.0_c_double / real(pcoord(i), c_double)
    end do
    sdoms = 0_c_int
    bounds = 1_c_int
    do i = 1, nranks_value
      sdoms(1, 1, i) = int(i - 1, c_int)
      sdoms(2, 1, i) = int(i, c_int)
      sdoms(1, 2, i) = 0_c_int
      sdoms(2, 2, i) = 1_c_int
      sdoms(1, 3, i) = 0_c_int
      sdoms(2, 3, i) = 1_c_int
    end do
    bcond = 1_c_int
    ftypes = 0_c_int
    ftypes(1, 1) = 1_c_int
    cfields(1) = -1_c_int
    ctypes = 0_c_int
    fsizes = 0_c_int

    call oh_context_configure_level3_legacy( &
      context, c_loc(pcoord(1)), c_loc(sdoms(1,1,1)), &
      c_loc(scoord(1,1)), 2_c_int, c_loc(bcond(1,1)), &
      c_loc(bounds(1,1,1)), c_loc(ftypes(1,1)), c_loc(cfields(1)), &
      c_loc(ctypes(1,1,1,1)), c_loc(fsizes(1,1,1)))
    call oh_context_grid_size(context, gsize)

    coord = 0.5_c_double
    coord(1) = (real(rank_value, c_double) + 0.5_c_double) / &
               real(nranks_value, c_double)
    if (oh_context_map_particle_to_subdomain( &
          context, coord(1), coord(2), coord(3)) /= int(rank_value, c_int)) &
      stop 41
    coord = 0.5_c_double
    if (rank_value == 0) then
      coord(1) = -0.25_c_double / real(nranks_value, c_double)
      expected = int(nranks_value - 1, c_int)
    else
      coord(1) = 1.0_c_double + &
                 0.25_c_double / real(nranks_value, c_double)
      expected = 0_c_int
    end if
    mapped = oh_context_map_particle_to_neighbor( &
      context, coord(1), coord(2), coord(3), 0_c_int)
    if (mapped /= expected) stop 42
    if (coord(1) < 0.0_c_double .or. coord(1) >= 1.0_c_double) stop 43
    field = 0.0_c_double
    call oh_context_bcast_field(context, c_loc(field(1)), c_loc(field(1)), &
                                0_c_int)
    call oh_context_destroy(context)
    deallocate(bounds)
    deallocate(sdoms)
  end subroutine

  subroutine run_injected_accounting_contract_test(adapter, rank_value, &
                                                   nranks_value)
    type(oh_particle_adapter_handle), intent(in) :: adapter
    integer, intent(in) :: rank_value
    integer, intent(in) :: nranks_value
    type(oh_context_handle) :: context
    type(pic_particle), target :: particles(8)
    type(pic_particle), target :: injected_particle
    type(pic_particle), pointer :: injected_copy
    integer(c_int), target :: sdid(2)
    integer(c_int), allocatable, target :: nphgram(:)
    integer(c_int), target :: totalp(2)
    integer(c_int), target :: pbase(3)
    integer(c_int), pointer :: pbase_values(:)
    type(c_ptr) :: particles_ptr
    type(c_ptr) :: copy_ptr
    type(c_ptr) :: sdid_ptr
    type(c_ptr) :: nphgram_ptr
    type(c_ptr) :: totalp_ptr
    type(c_ptr) :: pbase_ptr
    integer(c_int) :: ierr

    allocate(nphgram(2 * nranks_value))
    nphgram = 0_c_int
    totalp = 0_c_int
    pbase = 0_c_int
    particles = pic_particle(0.0_c_double, 0.0_c_double, 0.0_c_double, &
                             -1_c_int, 1_c_int)
    sdid = [int(rank_value, c_int), -1_c_int]

    call oh_context_create(context, int(MPI_COMM_WORLD, c_int), ierr)
    if (ierr /= 0_c_int) stop 48
    call configure_context(context, nranks_value, 1, .false.)
    call oh_context_set_particle_adapter(context, adapter)

    particles_ptr = c_loc(particles(1))
    sdid_ptr = c_loc(sdid(1))
    nphgram_ptr = c_loc(nphgram(1))
    totalp_ptr = c_loc(totalp(1))
    pbase_ptr = c_loc(pbase(1))
    call oh_context_bind_region_ids(context, sdid_ptr, OH_PARTICLES_BORROWED)
    call oh_context_bind_particles(context, particles_ptr, 8_c_int, &
                                   OH_PARTICLES_BORROWED)
    call oh_context_bind_particle_accounting(context, nphgram_ptr, &
                                             totalp_ptr, pbase_ptr, &
                                             OH_PARTICLES_BORROWED)
    call oh_context_set_total_particles(context)
    if (nphgram(rank_value + 1) /= 0_c_int) stop 49

    injected_particle%x = (real(rank_value, c_double) + 0.5_c_double) / &
                          real(nranks_value, c_double)
    injected_particle%y = 0.5_c_double
    injected_particle%z = 0.5_c_double
    injected_particle%region = int(rank_value, c_int)
    injected_particle%species = 1_c_int
    copy_ptr = oh_context_inject_particle_get(context, &
                                              c_loc(injected_particle))
    if (.not. c_associated(copy_ptr)) stop 50
    if (c_associated(copy_ptr, c_loc(injected_particle))) stop 51
    call c_f_pointer(copy_ptr, injected_copy)
    if (injected_copy%region /= int(rank_value, c_int)) stop 52
    if (nphgram(rank_value + 1) /= 1_c_int) stop 53

    call oh_context_remove_injected_particle(context, copy_ptr)
    if (nphgram(rank_value + 1) /= 0_c_int) stop 54
    if (injected_copy%region >= 0_c_int) stop 55

    injected_copy%region = int(rank_value, c_int)
    call oh_context_remap_injected_particle(context, copy_ptr)
    if (nphgram(rank_value + 1) /= 1_c_int) stop 56

    call oh_context_set_total_particles(context)
    call c_f_pointer(pbase_ptr, pbase_values, [3])
    if (nphgram(rank_value + 1) /= 1_c_int) stop 57
    if (pbase_values(1) /= 0_c_int) stop 58
    if (pbase_values(2) /= 1_c_int) stop 59
    if (pbase_values(3) /= 1_c_int) stop 60

    call oh_context_unbind_particle_accounting(context)
    call oh_context_unbind_particles(context)
    call oh_context_unbind_region_ids(context)
    call oh_context_destroy(context)
    deallocate(nphgram)
  end subroutine

  subroutine run_weighted_load_rebalance_test(adapter, rank_value, nranks_value)
    type(oh_particle_adapter_handle), intent(in) :: adapter
    integer, intent(in) :: rank_value
    integer, intent(in) :: nranks_value
    type(oh_context_handle) :: context
    type(pic_particle), target :: particles(16)
    integer(c_int), target :: sdid(2)
    integer(c_int), target :: copied_sdid(2)
    integer(c_int), target :: nphgram_store(4)
    integer(c_int), target :: totalp_store(2)
    integer(c_int), target :: pbase_store(3)
    integer(c_int), pointer :: pbase_values(:)
    real(c_double), target :: weights(2)
    type(c_ptr) :: particles_ptr
    type(c_ptr) :: sdid_ptr
    type(c_ptr) :: nphgram_ptr
    type(c_ptr) :: totalp_ptr
    type(c_ptr) :: pbase_ptr
    integer(c_int) :: ierr
    integer(c_int) :: mode_value

    if (nranks_value /= 2) return

    call oh_context_create(context, int(MPI_COMM_WORLD, c_int), ierr)
    if (ierr /= 0_c_int) stop 33
    call configure_context(context, nranks_value, 1, .false.)
    call oh_context_set_particle_adapter(context, adapter)

    particles = pic_particle(0.0_c_double, 0.0_c_double, 0.0_c_double, &
                             0_c_int, 1_c_int)
    particles(1)%x = (real(rank_value, c_double) + 0.5_c_double) / &
                     real(nranks_value, c_double)
    particles(1)%y = 0.5_c_double
    particles(1)%z = 0.5_c_double
    particles(1)%region = int(rank_value, c_int)
    particles(1)%species = 1_c_int

    sdid = [int(rank_value, c_int), -1_c_int]
    nphgram_store = 0_c_int
    totalp_store = 0_c_int
    pbase_store = 0_c_int
    nphgram_store(rank_value + 1) = 1_c_int
    weights = [4.0_c_double, 1.0_c_double]

    particles_ptr = c_loc(particles(1))
    sdid_ptr = c_loc(sdid(1))
    nphgram_ptr = c_loc(nphgram_store(1))
    totalp_ptr = c_loc(totalp_store(1))
    pbase_ptr = c_loc(pbase_store(1))

    call oh_context_bind_region_ids(context, sdid_ptr, OH_PARTICLES_BORROWED)
    call oh_context_bind_particles(context, particles_ptr, 16_c_int, &
                                   OH_PARTICLES_BORROWED)
    call oh_context_bind_particle_accounting(context, nphgram_ptr, totalp_ptr, &
                                             pbase_ptr, OH_PARTICLES_BORROWED)
    call oh_context_set_region_weights(context, weights)
    call oh_context_set_total_particles(context)

    mode_value = oh_context_transbound3(context, OH_MODE_NORMAL_PRIMARY, &
                                        0_c_int)
    if (mode_value /= OH_MODE_REBALANCE_SECONDARY) stop 34
    call oh_context_get_region_ids(context, c_loc(copied_sdid(1)))
    call c_f_pointer(pbase_ptr, pbase_values, [3])
    if (rank_value == 0) then
      if (copied_sdid(2) /= -1_c_int) stop 35
    else
      if (copied_sdid(2) /= 0_c_int) stop 36
      if (pbase_values(1) /= 0_c_int) stop 37
      if (pbase_values(2) /= 1_c_int) stop 38
      if (pbase_values(3) /= 2_c_int) stop 39
    end if

    call oh_context_unbind_region_ids(context)
    call oh_context_unbind_particle_accounting(context)
    call oh_context_unbind_particles(context)
    call oh_context_destroy(context)
  end subroutine

  subroutine run_region_weight_reset_behavior_test(adapter, rank_value, &
                                                   nranks_value)
    type(oh_particle_adapter_handle), intent(in) :: adapter
    integer, intent(in) :: rank_value
    integer, intent(in) :: nranks_value
    type(oh_context_handle) :: context
    type(pic_particle), target :: particles(4)
    integer(c_int), target :: sdid(2)
    integer(c_int), target :: copied_sdid(2)
    integer(c_int), target :: nphgram_store(4)
    integer(c_int), target :: totalp_store(2)
    integer(c_int), target :: pbase_store(3)
    integer(c_int), pointer :: pbase_values(:)
    real(c_double), target :: weights(2)
    type(c_ptr) :: particles_ptr
    type(c_ptr) :: sdid_ptr
    type(c_ptr) :: nphgram_ptr
    type(c_ptr) :: totalp_ptr
    type(c_ptr) :: pbase_ptr
    integer(c_int) :: ierr
    integer(c_int) :: mode_value

    if (nranks_value /= 2) return

    call oh_context_create(context, int(MPI_COMM_WORLD, c_int), ierr)
    if (ierr /= 0_c_int) stop 61
    call configure_context(context, nranks_value, 1, .false.)
    call oh_context_set_particle_adapter(context, adapter)

    particles = pic_particle(0.0_c_double, 0.0_c_double, 0.0_c_double, &
                             0_c_int, 1_c_int)
    particles(1)%x = (real(rank_value, c_double) + 0.5_c_double) / &
                     real(nranks_value, c_double)
    particles(1)%y = 0.5_c_double
    particles(1)%z = 0.5_c_double
    particles(1)%region = int(rank_value, c_int)
    particles(1)%species = 1_c_int

    sdid = [int(rank_value, c_int), -1_c_int]
    nphgram_store = 0_c_int
    totalp_store = 0_c_int
    pbase_store = 0_c_int
    nphgram_store(rank_value + 1) = 1_c_int
    weights = [4.0_c_double, 1.0_c_double]

    particles_ptr = c_loc(particles(1))
    sdid_ptr = c_loc(sdid(1))
    nphgram_ptr = c_loc(nphgram_store(1))
    totalp_ptr = c_loc(totalp_store(1))
    pbase_ptr = c_loc(pbase_store(1))

    call oh_context_bind_region_ids(context, sdid_ptr, OH_PARTICLES_BORROWED)
    call oh_context_bind_particles(context, particles_ptr, 4_c_int, &
                                   OH_PARTICLES_BORROWED)
    call oh_context_bind_particle_accounting(context, nphgram_ptr, totalp_ptr, &
                                             pbase_ptr, OH_PARTICLES_BORROWED)
    call oh_context_set_region_weights(context, weights)
    call oh_context_set_region_weights(context)
    call oh_context_set_total_particles(context)

    mode_value = oh_context_transbound3(context, OH_MODE_NORMAL_PRIMARY, &
                                        0_c_int)
    if (mode_value /= OH_MODE_NORMAL_PRIMARY) stop 62
    call oh_context_get_region_ids(context, c_loc(copied_sdid(1)))
    if (any(copied_sdid /= sdid)) stop 63
    call c_f_pointer(pbase_ptr, pbase_values, [3])
    if (pbase_values(1) /= 0_c_int) stop 64
    if (pbase_values(2) /= 1_c_int) stop 65
    if (pbase_values(3) /= 1_c_int) stop 66

    call oh_context_unbind_region_ids(context)
    call oh_context_unbind_particle_accounting(context)
    call oh_context_unbind_particles(context)
    call oh_context_destroy(context)
  end subroutine
end program
