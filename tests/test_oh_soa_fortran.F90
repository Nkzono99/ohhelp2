program test_oh_soa_fortran
  use iso_c_binding
  use mpi
  use ohhelp_v2
  implicit none
  type(oh_context_handle) :: ctx
  type(oh_particle_field) :: fields(3)
  type(oh_particle_ref) :: ref
  integer :: mpierr, rank, nranks
  integer(c_int) :: ierr, mode
  integer(c_int), pointer :: hist(:), total(:), base(:)
  type(c_ptr) :: hist_ptr, total_ptr, base_ptr, columns(3), values(3)
  integer(c_long_long), target :: region(8), id(8), new_region, new_id
  real(c_double), target :: x(8), new_x
  integer(c_long_long), pointer :: owned_region(:), region_view(:), id_view(:)
  real(c_double), pointer :: x_view(:)
  call MPI_Init(mpierr)
  call MPI_Comm_rank(MPI_COMM_WORLD,rank,mpierr)
  call MPI_Comm_size(MPI_COMM_WORLD,nranks,mpierr)
  call oh_context_create(ctx,int(MPI_COMM_WORLD,c_int),ierr)
  if (ierr/=MPI_SUCCESS) stop 1
  call oh_context_configure_particles(ctx,1_c_int,10000_c_int)
  call oh_context_set_particle_storage(ctx,OH_PARTICLE_STORAGE_SOA,ierr)
  if (ierr/=MPI_SUCCESS) stop 2
  fields(1)=oh_particle_field(0,OH_PARTICLE_FIELD_LONG_LONG,OH_PARTICLE_ROLE_REGION,c_sizeof(region(1)))
  fields(2)=oh_particle_field(1,OH_PARTICLE_FIELD_DOUBLE,OH_PARTICLE_ROLE_X,c_sizeof(x(1)))
  fields(3)=oh_particle_field(2,OH_PARTICLE_FIELD_LONG_LONG,OH_PARTICLE_ROLE_PAYLOAD,c_sizeof(id(1)))
  call oh_context_configure_soa(ctx,fields,ierr)
  if (ierr/=MPI_SUCCESS) stop 3
  region_view=>region; x_view=>x; id_view=>id
  columns=[oh_soa_column(region_view),oh_soa_column(x_view),oh_soa_column(id_view)]
  call oh_context_bind_soa_particles(ctx,8_c_int,OH_PARTICLES_BORROWED,ierr,columns(:2))
  if (ierr==MPI_SUCCESS) stop 4
  x_view=>x(::2)
  columns(2)=oh_soa_column(x_view)
  if (c_associated(columns(2))) stop 5
  call oh_context_bind_soa_particles(ctx,8_c_int,OH_PARTICLES_BORROWED,ierr,columns)
  if (ierr==MPI_SUCCESS) stop 6
  x_view=>x
  columns(2)=oh_soa_column(x_view)
  call oh_context_bind_soa_particles(ctx,8_c_int,OH_PARTICLES_BORROWED,ierr,columns)
  if (ierr/=MPI_SUCCESS) stop 7
  hist_ptr=c_null_ptr; total_ptr=c_null_ptr; base_ptr=c_null_ptr
  call oh_context_bind_particle_accounting(ctx,hist_ptr,total_ptr,base_ptr,OH_PARTICLES_OWNED)
  call c_f_pointer(hist_ptr,hist,[2*nranks])
  call c_f_pointer(total_ptr,total,[2])
  call c_f_pointer(base_ptr,base,[3])
  region(1)=mod(rank+1,nranks); x(1)=rank+0.5_c_double; id(1)=rank
  hist(mod(rank+1,nranks)+1)=1
  call oh_context_set_total_particles(ctx)
  mode=oh_context_transbound2(ctx,OH_MODE_NORMAL_PRIMARY,0_c_int)
  if (base(3)/=1 .or. region(1)/=rank) stop 8
  if (id(1)/=mod(rank-1+nranks,nranks)) stop 9
  if (x(1)/=id(1)+0.5_c_double) stop 10
  hist(rank+1)=1
  new_region=rank; new_id=100+rank; new_x=new_id+0.5_c_double
  values=[c_loc(new_region),c_loc(new_x),c_loc(new_id)]
  call oh_context_inject_soa_particle(ctx,values,ref,ierr)
  if (ierr/=MPI_SUCCESS .or. ref%index/=1) stop 11
  call oh_context_remove_injected_particle_ref(ctx,ref,ierr)
  if (ierr/=MPI_SUCCESS) stop 12
  region(ref%index+1)=rank
  call oh_context_remap_injected_particle_ref(ctx,ref,ierr)
  if (ierr/=MPI_SUCCESS) stop 13
  mode=oh_context_transbound2(ctx,mode,0_c_int)
  if (base(3)/=2) stop 14
  call oh_context_remove_injected_particle_ref(ctx,ref,ierr)
  if (ierr==MPI_SUCCESS) stop 15
  call oh_context_unbind_particles(ctx)
  call oh_context_bind_soa_particles(ctx,8_c_int,OH_PARTICLES_OWNED,ierr)
  if (ierr/=MPI_SUCCESS) stop 16
  call c_f_pointer(oh_context_soa_field(ctx,0_c_int),owned_region,[8])
  owned_region(1)=123_c_long_long
  if (region(1)==123_c_long_long) stop 17
  call oh_context_destroy(ctx)
  call MPI_Finalize(mpierr)
end program
