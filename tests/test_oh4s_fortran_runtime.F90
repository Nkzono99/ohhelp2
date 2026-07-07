#include "oh_config.h"
program test_oh4s_fortran_runtime
  use mpi
  use oh_type
  use ohhelp4s
  implicit none

  integer, parameter :: nspec = 1
  integer, parameter :: maxfrac = 20
  integer :: ierr
  integer :: rank
  integer :: nranks
  integer :: maxlocalp
  integer :: cbufsize
  integer :: mode
  integer :: mapped
  integer :: sdid(2)
  integer :: totalp(nspec, 2)
  integer :: pbase(3)
  integer :: pcoord(OH_DIMENSION)
  integer :: nbor(3, 3, 3)
  integer :: sdoms(2, OH_DIMENSION, 2)
  integer :: scoord(2, OH_DIMENSION)
  integer :: bcond(2, OH_DIMENSION)
  integer :: bounds(2, OH_DIMENSION, 2)
  integer :: ftypes(7, 1)
  integer :: cfields(1)
  integer :: ctypes(3, 2, 2, 1)
  integer :: fsizes(2, OH_DIMENSION, 1)
  integer :: zbound(2, 2)
  integer :: pghgram
  integer :: pgindex
  type(oh_particle), allocatable :: pbuf(:)
  type(oh_mycomm) :: mycomm
  real*8 :: weights(2)
  character(len=64) :: arg
  logical :: run_weighted_secondary

  call MPI_Init(ierr)
  call MPI_Comm_rank(MPI_COMM_WORLD, rank, ierr)
  call MPI_Comm_size(MPI_COMM_WORLD, nranks, ierr)
  if (nranks /= 2) stop 1
  call oh1_fam_comm(MPI_COMM_WORLD)

  call get_command_argument(1, arg)
  run_weighted_secondary = trim(arg) == "weighted-secondary"

  pcoord = (/2, 1, 1/)
  scoord(:,1) = (/0, 2/)
  scoord(:,2) = (/0, 1/)
  scoord(:,3) = (/0, 1/)
  bcond = 2
  nbor = -1
  sdoms = 0
  sdoms(1,1,1) = 0
  sdoms(2,1,1) = -1
  bounds = 0
  ftypes = 0
  ftypes(1,1) = -1
  cfields(1) = 0
  ctypes = 0
  totalp = 0
  pbase = 0
  maxlocalp = 0
  cbufsize = 0
  fsizes = 0
  zbound = 0

  call oh4s_init(sdid, nspec, maxfrac, 2_8, 1, 1, totalp, pbase, &
                 maxlocalp, cbufsize, mycomm, nbor, pcoord, sdoms, &
                 scoord, 2, bcond, bounds, ftypes, cfields, ctypes, &
                 fsizes, zbound, 0, 0, 0)
  if (sdid(1) /= rank) stop 2
  if (sdid(2) /= -1) stop 3
  if (maxlocalp <= 0) stop 4
  allocate(pbuf(maxlocalp * 2))
  call clear_particles(pbuf)
  call oh4s_particle_buffer(maxlocalp, pbuf)
  pghgram = 0
  pgindex = 0
  call oh4s_per_grid_histogram(pghgram, pgindex)

  pbuf(1)%x = real(rank, 8) + 0.5d0
  pbuf(1)%y = 0.5d0
  pbuf(1)%z = 0.5d0
  pbuf(1)%pid = rank
  pbuf(1)%preside = OH_PCL_ALIVE
  pbuf(1)%trace_id = int(rank, 8)
  pbuf(1)%nid = rank
  pbuf(1)%spec = 1

  mapped = oh4s_map_particle_to_subdomain(pbuf(1), 0, 1)
  if (mapped /= rank) stop 5

  if (run_weighted_secondary) then
    weights = (/4.0d0, 1.0d0/)
    call oh1_set_region_weights(weights)
  end if

  mode = oh4s_transbound(OH_MODE_NORMAL_PRIMARY, 0)
  if (run_weighted_secondary) then
    if (mode /= OH_MODE_REBALANCE_SECONDARY) stop 6
    if (rank == 0) then
      if (sdid(2) /= -1) stop 7
      if (pbase(1) /= 0 .or. pbase(2) /= 0 .or. pbase(3) /= 0) stop 8
      if (totalp(1,1) /= 0 .or. totalp(1,2) /= 0) stop 9
    else
      if (sdid(2) /= 0) stop 10
      if (pbase(1) /= 0 .or. pbase(2) /= 2 .or. pbase(3) /= 4) stop 11
      if (totalp(1,1) /= 2 .or. totalp(1,2) /= 2) stop 12
    end if
    mode = oh4s_transbound(OH_MODE_REBALANCE_SECONDARY, 0)
    if (mode /= OH_MODE_NORMAL_PRIMARY) stop 13
    if (sdid(2) /= -1) stop 14
  else
    if (mode /= OH_MODE_NORMAL_PRIMARY) stop 15
    if (pbase(1) /= 0 .or. pbase(2) /= 2 .or. pbase(3) /= 2) stop 16
    if (totalp(1,1) /= 2 .or. totalp(1,2) /= 0) stop 17
    call run_particle_mutation_path(pbuf, maxlocalp, rank)
  end if

  call MPI_Finalize(ierr)
contains
  subroutine run_particle_mutation_path(parts, maxlocalp_value, rank_value)
    type(oh_particle), intent(inout) :: parts(:)
    integer, intent(in) :: maxlocalp_value
    integer, intent(in) :: rank_value
    type(oh_particle) :: injected
    integer :: injected_slot
    integer :: i
    integer :: mapped_value

    injected%x = real(rank_value, 8) + 0.5d0
    injected%y = 0.5d0
    injected%z = 0.5d0
    injected%vx = 0.0d0
    injected%vy = 0.0d0
    injected%vz = 0.0d0
    injected%pid = 100 + rank_value
    injected%preside = OH_PCL_ALIVE
    injected%trace_id = int(1000 + rank_value, 8)
    injected%nid = rank_value
    injected%spec = 1

    mapped_value = oh4s_inject_particle(injected, 0)
    if (mapped_value /= rank_value) stop 18
    injected_slot = 0
    do i = maxlocalp_value + 1, maxlocalp_value * 2
      if (parts(i)%pid == 100 + rank_value .and. &
          parts(i)%trace_id == int(1000 + rank_value, 8)) then
        injected_slot = i
        exit
      end if
    end do
    if (injected_slot == 0) stop 19
    if (parts(injected_slot)%nid < 0) stop 20

    call oh4s_remove_mapped_particle(parts(injected_slot), 0, 1)
    if (parts(injected_slot)%nid /= -1) stop 21
    mapped_value = oh4s_remap_particle_to_subdomain(parts(injected_slot), &
                                                    0, 1)
    if (mapped_value /= rank_value) stop 22
    if (parts(injected_slot)%nid < 0) stop 23
  end subroutine

  subroutine clear_particles(parts)
    type(oh_particle), intent(inout) :: parts(:)

    parts(:)%x = 0.0d0
    parts(:)%y = 0.0d0
    parts(:)%z = 0.0d0
    parts(:)%vx = 0.0d0
    parts(:)%vy = 0.0d0
    parts(:)%vz = 0.0d0
    parts(:)%pid = 0
    parts(:)%preside = OH_PCL_ALIVE
    parts(:)%trace_id = 0_8
    parts(:)%nid = -1
    parts(:)%spec = 1
  end subroutine
end program
