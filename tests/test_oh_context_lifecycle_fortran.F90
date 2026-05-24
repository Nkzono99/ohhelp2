program test_oh_context_lifecycle_fortran
  use iso_c_binding
  use mpi
  use ohhelp_v2
  implicit none

  type(oh_context_handle) :: context_x
  type(oh_context_handle) :: context_y
  type(c_ptr) :: particles_x
  type(c_ptr) :: particles_y
  type(c_ptr) :: nphgram_x
  type(c_ptr) :: nphgram_y
  type(c_ptr) :: totalp_x
  type(c_ptr) :: totalp_y
  type(c_ptr) :: pbase_x
  type(c_ptr) :: pbase_y
  real(c_double), target :: field(8)
  real(c_double), target :: coord_x(3)
  real(c_double), target :: coord_y(3)
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

  call configure_context(context_x, nranks, 1)
  call configure_context(context_y, nranks, 2)

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

  particles_x = c_null_ptr
  particles_y = c_null_ptr
  call oh_context_bind_particles(context_x, particles_x, 4_c_int, &
                                 OH_PARTICLES_OWNED)
  call oh_context_bind_particles(context_y, particles_y, 4_c_int, &
                                 OH_PARTICLES_OWNED)
  if (.not. c_associated(particles_x)) stop 3
  if (.not. c_associated(particles_y)) stop 4
  if (c_associated(particles_x, particles_y)) stop 5

  nphgram_x = c_null_ptr
  totalp_x = c_null_ptr
  pbase_x = c_null_ptr
  nphgram_y = c_null_ptr
  totalp_y = c_null_ptr
  pbase_y = c_null_ptr
  call oh_context_bind_particle_accounting(context_x, nphgram_x, totalp_x, &
                                           pbase_x, OH_PARTICLES_OWNED)
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

  call oh_context_unbind_particle_accounting(context_x)
  call oh_context_unbind_particles(context_x)
  call oh_context_unbind_particle_accounting(context_y)
  call oh_context_unbind_particles(context_y)
  call oh_context_destroy(context_x)
  call oh_context_destroy(context_y)

  call MPI_Finalize(mpierr)

contains
  subroutine configure_context(context, nranks_value, axis)
    type(oh_context_handle), intent(in) :: context
    integer, intent(in) :: nranks_value
    integer, intent(in) :: axis
    integer(c_int), target :: pcoord(3)
    integer(c_int), target :: scoord(2,3)
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
    do i = 1, 3
      scoord(2, i) = pcoord(i)
      gsize(i) = 1.0_c_double
    end do
    ftypes = 0_c_int
    ftypes(1, 1) = 1_c_int
    cfields(1) = -1_c_int
    ctypes = 0_c_int
    fsizes = 0_c_int
    call oh_context_configure_level3(context, c_loc(pcoord(1)), c_null_ptr, &
                                     c_loc(scoord(1,1)), 1_c_int, &
                                     c_loc(bcond(1,1)), c_null_ptr, &
                                     c_loc(ftypes(1,1)), c_loc(cfields(1)), &
                                     c_loc(ctypes(1,1,1,1)), &
                                     c_loc(fsizes(1,1,1)))
    call oh_context_grid_size(context, gsize)
  end subroutine
end program
