program test_oh_context_lifecycle_fortran
  use iso_c_binding
  use mpi
  use ohhelp_v2
  implicit none

  type(oh_context_handle) :: context
  type(c_ptr) :: particles
  type(c_ptr) :: nphgram
  type(c_ptr) :: totalp
  type(c_ptr) :: pbase
  integer(c_int), target :: pcoord(3)
  integer(c_int), target :: scoord(2,3)
  integer(c_int), target :: bcond(2,3)
  integer(c_int), target :: ftypes(7,2)
  integer(c_int), target :: cfields(1)
  integer(c_int), target :: ctypes(3,2,1,1)
  integer(c_int), target :: fsizes(2,3,1)
  real(c_double), target :: gsize(3)
  real(c_double), target :: field(8)
  real(c_double), target :: x
  real(c_double), target :: y
  real(c_double), target :: z
  integer :: mpierr
  integer :: nranks
  integer(c_int) :: ierr
  integer(c_int) :: mode
  integer :: i

  call MPI_Init(mpierr)
  call MPI_Comm_size(MPI_COMM_WORLD, nranks, mpierr)

  call oh_context_create(context, int(MPI_COMM_WORLD, c_int), ierr)
  if (ierr /= 0_c_int) stop 1

  call oh_context_configure_particles(context, 1_c_int, 20_c_int)
  pcoord = 1_c_int
  pcoord(1) = int(nranks, c_int)
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
  field = 0.0_c_double
  x = 0.5_c_double
  y = 0.5_c_double
  z = 0.5_c_double
  if (oh_context_map_particle_to_subdomain(context, x, y, z) /= 0_c_int) &
    stop 9
  if (oh_context_map_particle_to_neighbor(context, x, y, z, 0_c_int) &
      /= 0_c_int) stop 10
  call oh_context_bcast_field(context, c_loc(field(1)), c_loc(field(1)), &
                              0_c_int)
  call oh_context_reduce_field(context, c_loc(field(1)), c_loc(field(1)), &
                               0_c_int)
  call oh_context_allreduce_field(context, c_loc(field(1)), c_loc(field(1)), &
                                  0_c_int)

  particles = c_null_ptr
  call oh_context_bind_particles(context, particles, 4_c_int, &
                                 OH_PARTICLES_OWNED)
  if (.not. c_associated(particles)) stop 2

  nphgram = c_null_ptr
  totalp = c_null_ptr
  pbase = c_null_ptr
  call oh_context_bind_particle_accounting(context, nphgram, totalp, pbase, &
                                           OH_PARTICLES_OWNED)
  if (.not. c_associated(nphgram)) stop 3
  if (.not. c_associated(totalp)) stop 4
  if (.not. c_associated(pbase)) stop 5

  mode = oh_context_transbound3(context, OH_MODE_NORMAL_PRIMARY, 0_c_int)
  if (mode /= OH_MODE_NORMAL_PRIMARY) stop 8

  call oh_context_unbind_particle_accounting(context)
  call oh_context_unbind_particles(context)
  call oh_context_destroy(context)

  call MPI_Finalize(mpierr)
end program
