#include "oh_config.h"
program test_oh_legacy_fortran_runtime
  use mpi
  use oh_type
  use ohhelp1
  use ohhelp2
  use ohhelp3
  implicit none

  integer, parameter :: nspec = 1
  integer, parameter :: maxfrac = 20
  integer, parameter :: npmax = 4
  integer :: ierr
  integer :: nranks
  integer :: maxlocalp
  integer :: mode
  integer :: sdid(2)
  integer :: pbase(3)
  integer :: pcoord(OH_DIMENSION)
  integer :: nphgram(1, nspec, 2)
  integer :: totalp(nspec, 2)
  type(oh_particle) :: pbuf(npmax)
  type(oh_mycomm) :: mycomm
  integer :: nbor(3, 3, 3)
  integer :: sdoms(2, OH_DIMENSION, 1)
  integer :: scoord(2, OH_DIMENSION)
  integer :: bcond(2, OH_DIMENSION)
  integer :: bounds(2, OH_DIMENSION, 1)
  integer :: ftypes(7, 2)
  integer :: cfields(1)
  integer :: ctypes(3, 2, 1, 1)
  integer :: fsizes(2, OH_DIMENSION, 1)
  real*8 :: grid_size(OH_DIMENSION)
  real*8 :: x
  real*8 :: y
  real*8 :: z

  call MPI_Init(ierr)
  call MPI_Comm_size(MPI_COMM_WORLD, nranks, ierr)
  if (nranks /= 1) stop 1
  call oh1_fam_comm(MPI_COMM_WORLD)

  pcoord = 1
  scoord(1,:) = 0
  scoord(2,:) = 1
  bcond = 1
  nbor = -1
  sdoms = 0
  sdoms(1,1,1) = 0
  sdoms(2,1,1) = -1
  bounds = 0
  nphgram = 0
  totalp = 0
  pbase = 0

  pbuf(:)%x = 0.0d0
  pbuf(:)%y = 0.0d0
  pbuf(:)%z = 0.0d0
  pbuf(:)%vx = 0.0d0
  pbuf(:)%vy = 0.0d0
  pbuf(:)%vz = 0.0d0
  pbuf(:)%pid = 0
  pbuf(:)%preside = OH_PCL_ALIVE
  pbuf(:)%trace_id = 0_8
  pbuf(:)%nid = 0
  pbuf(:)%spec = 1

  ftypes = 0
  ftypes(:,1) = (/1, 0, 0, 0, 0, 0, 0/)
  ftypes(1,2) = -1
  cfields(1) = 0
  ctypes = 0

  maxlocalp = oh2_max_local_particles(4_8, maxfrac, 0)
  if (maxlocalp < npmax) stop 2

  call oh3_init(sdid, nspec, maxfrac, nphgram, totalp, pbuf, pbase, &
                npmax, mycomm, nbor, pcoord, sdoms, scoord, 1, bcond, &
                bounds, ftypes, cfields, ctypes, fsizes, 0, 0, 0)

  nphgram(1,1,1) = 1
  pbuf(1)%x = 0.5d0
  pbuf(1)%y = 0.5d0
  pbuf(1)%z = 0.5d0
  pbuf(1)%nid = 0
  pbuf(1)%spec = 1

  grid_size = 1.0d0
  call oh3_grid_size(grid_size)

  x = 0.5d0
  y = 0.5d0
  z = 0.5d0
  if (oh3_map_particle_to_subdomain(x, y, z) /= 0) stop 3
  if (oh3_map_particle_to_neighbor(x, y, z, 0) /= 0) stop 4

  mode = oh3_transbound(OH_MODE_NORMAL_PRIMARY, 0)
  if (mode /= OH_MODE_NORMAL_PRIMARY) stop 5
  if (pbase(2) /= 1) stop 6
  if (pbase(3) /= 1) stop 7

  call MPI_Finalize(ierr)
end program
