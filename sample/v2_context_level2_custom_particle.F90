program v2_context_level2_custom_particle
  use iso_c_binding
  use mpi
  use ohhelp_v2
  implicit none

  type, bind(C) :: pic_particle
    real(c_double) :: x
    real(c_double) :: y
    real(c_double) :: z
    integer(c_long_long) :: region
    integer(c_int) :: species
  end type

  type(oh_context_handle) :: context
  type(oh_particle_adapter_handle) :: adapter
  type(pic_particle), target :: particle
  type(pic_particle), target :: particles(4)
  real(c_double), allocatable, target :: weights(:)
  integer(c_int), allocatable, target :: nphgram(:)
  integer(c_int), target :: sdid(2)
  integer(c_int), target :: totalp(2)
  integer(c_int), target :: pbase(3)
  type(c_ptr) :: particles_ptr
  type(c_ptr) :: sdid_ptr
  type(c_ptr) :: nphgram_ptr
  type(c_ptr) :: totalp_ptr
  type(c_ptr) :: pbase_ptr
  integer(c_size_t) :: region_offset
  integer(c_size_t) :: species_offset
  integer(c_int) :: ierr
  integer(c_int) :: mode
  integer :: mpierr
  integer :: rank
  integer :: nranks

  call MPI_Init(mpierr)
  call MPI_Comm_rank(MPI_COMM_WORLD, rank, mpierr)
  call MPI_Comm_size(MPI_COMM_WORLD, nranks, mpierr)

  call oh_context_create(context, int(MPI_COMM_WORLD, c_int), ierr)
  if (ierr /= 0_c_int) stop 1
  call oh_context_configure_particles(context, 1_c_int, 20_c_int)

  particle = pic_particle(0.0_c_double, 0.0_c_double, 0.0_c_double, &
                          int(rank, c_long_long), 1_c_int)
  call oh_particle_adapter_create_byte(adapter, c_sizeof(particle), ierr)
  if (ierr /= 0_c_int) stop 2
  region_offset = oh_particle_field_offset(c_loc(particle), &
                                           c_loc(particle%region))
  species_offset = oh_particle_field_offset(c_loc(particle), &
                                            c_loc(particle%species))
  call oh_particle_adapter_use_integer_fields( &
    adapter, region_offset, c_sizeof(particle%region), &
    species_offset, c_sizeof(particle%species))
  call oh_particle_adapter_set_species_base(adapter, 1_c_int)
  call oh_context_set_particle_adapter(context, adapter)
  call oh_particle_adapter_destroy(adapter)

  allocate(weights(nranks))
  allocate(nphgram(2 * nranks))
  weights = 1.0_c_double
  nphgram = 0_c_int
  nphgram(rank + 1) = 1_c_int
  sdid = [int(rank, c_int), -1_c_int]
  totalp = 0_c_int
  pbase = 0_c_int
  particles = pic_particle(0.5_c_double, 0.5_c_double, 0.5_c_double, &
                           int(rank, c_long_long), 1_c_int)

  call oh_context_set_region_weights(context, weights)
  particles_ptr = c_loc(particles(1))
  sdid_ptr = c_loc(sdid(1))
  nphgram_ptr = c_loc(nphgram(1))
  totalp_ptr = c_loc(totalp(1))
  pbase_ptr = c_loc(pbase(1))
  call oh_context_bind_region_ids(context, sdid_ptr, OH_PARTICLES_BORROWED)
  call oh_context_bind_particles(context, particles_ptr, 4_c_int, &
                                 OH_PARTICLES_BORROWED)
  call oh_context_bind_particle_accounting(context, nphgram_ptr, totalp_ptr, &
                                           pbase_ptr, OH_PARTICLES_BORROWED)
  call oh_context_set_total_particles(context)

  mode = oh_context_transbound2(context, OH_MODE_NORMAL_PRIMARY, 0_c_int)
  if (mode /= OH_MODE_NORMAL_PRIMARY) stop 3
  if (pbase(1) /= 0_c_int) stop 4
  if (pbase(2) /= 1_c_int) stop 5
  if (pbase(3) /= 1_c_int) stop 6
  if (particles(1)%region /= int(rank, c_long_long)) stop 7
  if (particles(1)%species /= 1_c_int) stop 8

  call oh_context_unbind_particle_accounting(context)
  call oh_context_unbind_particles(context)
  call oh_context_unbind_region_ids(context)
  call oh_context_destroy(context)
  deallocate(nphgram)
  deallocate(weights)

  call MPI_Finalize(mpierr)
end program
