program test_oh_v2_fortran_raw_init_runtime
  use iso_c_binding
  use mpi
  use ohhelp1, only: oh1_fam_comm
  use ohhelp_v2
  implicit none
#include "oh_config.h"

#ifndef TEST_OH_RAW_LEVEL
#define TEST_OH_RAW_LEVEL 3
#endif

#if OH_DIMENSION == 1
  integer(c_int), parameter :: neighbor_count = 3_c_int
#elif OH_DIMENSION == 2
  integer(c_int), parameter :: neighbor_count = 9_c_int
#else
  integer(c_int), parameter :: neighbor_count = 27_c_int
#endif
  integer(c_int), parameter :: ndim = OH_DIMENSION
  integer(c_int), parameter :: oh_lower = 0_c_int
  integer(c_int), parameter :: oh_upper = 1_c_int
  integer(c_int), parameter :: ftype_n = 8_c_int
  integer(c_int), parameter :: ctype_n = 3_c_int

  type, bind(C) :: pic_particle
    real(c_double) :: x
    real(c_double) :: y
    real(c_double) :: z
    integer(c_int) :: region
    integer(c_int) :: species
  end type

  type(oh_context_handle) :: context
  type(oh_particle_adapter_handle) :: adapter
  type(oh_mycomm_v2), target :: mycomm
  type(pic_particle), allocatable, target :: particles(:)
  type(pic_particle), pointer :: active_particles(:)
  integer(c_int), allocatable, target :: nphgram(:)
  integer(c_int), target :: sdid(2)
  integer(c_int), target :: totalp(2)
  integer(c_int), target :: pbase(3)
  integer(c_int), target :: nbor(neighbor_count)
  integer(c_int), target :: pcoord(ndim)
  integer(c_int), target :: scoord(2*ndim)
  integer(c_int), target :: bcond(2*ndim)
  integer(c_int), target :: ftypes(2*ftype_n)
  integer(c_int), target :: cfields(1)
  integer(c_int), target :: ctypes(2*ctype_n)
  integer(c_int), target :: fsizes(2*ndim)
  type(c_ptr) :: pbuf
  type(c_ptr) :: pbase_ptr
  type(c_ptr) :: mycomm_ptr
  type(c_ptr) :: pcoord_ptr
  integer(c_size_t) :: region_offset
  integer(c_size_t) :: species_offset
  integer(c_size_t) :: x_offset
  integer(c_size_t) :: y_offset
  integer(c_size_t) :: z_offset
  integer(c_int) :: ierr
  integer(c_int) :: mode
  integer(c_int) :: mapped
  integer :: mpierr
  integer :: rank
  integer :: nranks
  integer :: d

  call MPI_Init(mpierr)
  call MPI_Comm_rank(MPI_COMM_WORLD, rank, mpierr)
  call MPI_Comm_size(MPI_COMM_WORLD, nranks, mpierr)
  call oh1_fam_comm(MPI_COMM_WORLD)

  allocate(particles(8))
  allocate(nphgram(2*nranks))

  call oh_particle_adapter_create_byte(adapter, c_sizeof(particles(1)), ierr)
  if (ierr /= 0_c_int) stop 1
  region_offset = oh_particle_field_offset(c_loc(particles(1)), &
                                           c_loc(particles(1)%region))
  species_offset = oh_particle_field_offset(c_loc(particles(1)), &
                                            c_loc(particles(1)%species))
  x_offset = oh_particle_field_offset(c_loc(particles(1)), &
                                      c_loc(particles(1)%x))
  y_offset = oh_particle_field_offset(c_loc(particles(1)), &
                                      c_loc(particles(1)%y))
  z_offset = oh_particle_field_offset(c_loc(particles(1)), &
                                      c_loc(particles(1)%z))
  call oh_particle_adapter_use_integer_fields( &
    adapter, region_offset, c_sizeof(particles(1)%region), &
    species_offset, c_sizeof(particles(1)%species))
  call oh_particle_adapter_set_species_base(adapter, 1_c_int)
  call oh_particle_adapter_use_level3_position_fields(adapter, x_offset, &
                                                      y_offset, z_offset)
  call oh_set_particle_adapter(adapter)
  call oh_particle_adapter_destroy(adapter)

  sdid = 0_c_int
  totalp = 0_c_int
  pbase = 0_c_int
  mycomm = oh_mycomm_v2(-777_c_int, -777_c_int, -777_c_int, -777_c_int, &
                        -777_c_int)
  nphgram = 0_c_int
  nbor = -1_c_int
  pcoord = 1_c_int
  pcoord(1) = int(nranks, c_int)
  scoord = 0_c_int
  bcond = 1_c_int
  do d = 0, ndim - 1
    scoord(2*d + 1) = 0_c_int
    scoord(2*d + 2) = pcoord(d + 1)
  end do
  ftypes = 0_c_int
  ftypes(1) = 1_c_int
  cfields(1) = -1_c_int
  ctypes = 0_c_int
  fsizes = 0_c_int

  particles = pic_particle(0.0_c_double, 0.0_c_double, 0.0_c_double, &
                           0_c_int, 1_c_int)
#ifdef TEST_OH_RAW_NULL_PBUF
  pbuf = c_null_ptr
#else
  pbuf = c_loc(particles(1))
#endif
#ifdef TEST_OH_RAW_NULL_PBASE
  pbase_ptr = c_null_ptr
#else
  pbase_ptr = c_loc(pbase(1))
#endif
#ifdef TEST_OH_RAW_MYCOMM
  mycomm_ptr = c_loc(mycomm)
#else
  mycomm_ptr = c_null_ptr
#endif
#ifdef TEST_OH_RAW_NULL_PCOORD
  pcoord_ptr = c_null_ptr
#else
  pcoord_ptr = c_loc(pcoord(1))
#endif

#if TEST_OH_RAW_LEVEL == 2
  call oh2_init_raw(c_loc(sdid(1)), 1_c_int, 20_c_int, &
                    c_loc(nphgram(1)), c_loc(totalp(1)), pbuf, &
                    pbase_ptr, 8_c_int, mycomm_ptr, c_loc(nbor(1)), &
                    pcoord_ptr, 0_c_int, 0_c_int, 0_c_int)
#else
  call oh3_init_raw(c_loc(sdid(1)), 1_c_int, 20_c_int, &
                    c_loc(nphgram(1)), c_loc(totalp(1)), pbuf, &
                    pbase_ptr, 8_c_int, mycomm_ptr, c_loc(nbor(1)), &
                    pcoord_ptr, c_null_ptr, c_loc(scoord(1)), &
                    1_c_int, c_loc(bcond(1)), c_null_ptr, c_loc(ftypes(1)), &
                    c_loc(cfields(1)), c_loc(ctypes(1)), c_loc(fsizes(1)), &
                    0_c_int, 0_c_int, 0_c_int)
#endif
  if (.not. c_associated(pbuf)) stop 12
  call c_f_pointer(pbuf, active_particles, [8])
  active_particles = pic_particle(0.0_c_double, 0.0_c_double, &
                                  0.0_c_double, 0_c_int, 1_c_int)
  active_particles(1)%x = real(rank, c_double) + 0.5_c_double
  active_particles(1)%y = 0.5_c_double
  active_particles(1)%z = 0.5_c_double
  active_particles(1)%region = int(rank, c_int)
  active_particles(1)%species = 1_c_int

#ifdef TEST_OH_RAW_MYCOMM
  if (mycomm%prime /= int(MPI_COMM_NULL, c_int)) stop 7
  if (mycomm%sec /= int(MPI_COMM_NULL, c_int)) stop 8
  if (mycomm%rank /= 0_c_int) stop 9
  if (mycomm%root /= 0_c_int) stop 10
  if (mycomm%black /= 0_c_int) stop 11
#endif

  context = oh_default_context()
  if (.not. oh_context_associated(context)) stop 2
  nphgram = 0_c_int
  nphgram(rank + 1) = 1_c_int
  call oh_context_set_total_particles(context)

#if TEST_OH_RAW_LEVEL == 2
  mode = oh_context_transbound2(context, OH_MODE_NORMAL_PRIMARY, 0_c_int)
#else
  mapped = oh_context_map_particle_to_subdomain( &
    context, active_particles(1)%x, active_particles(1)%y, &
    active_particles(1)%z)
  if (mapped /= int(rank, c_int)) stop 3
  mode = oh_context_transbound3(context, OH_MODE_NORMAL_PRIMARY, 0_c_int)
#endif
  if (mode /= OH_MODE_NORMAL_PRIMARY) stop 4
  if (pbase(2) /= 1_c_int) stop 5
  if (pbase(3) /= 1_c_int) stop 6

  call oh_context_unbind_particles(context)
  call oh_context_unbind_particle_accounting(context)
  call oh_set_particle_adapter()
  deallocate(nphgram)
  deallocate(particles)

  call MPI_Finalize(mpierr)
end program
