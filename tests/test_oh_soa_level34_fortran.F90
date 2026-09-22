#include "oh_config.h"
program soa_level34
  use iso_c_binding
  use mpi
  use ohhelp_v2
  implicit none
  type(oh_context_handle) :: ctx
  type(oh_particle_field) :: fields(5)
  type(oh_particle_ref) :: ref
  integer :: mpierr, rank, nranks
  integer(c_int) :: ierr, capacity, mode, dst, cbufsize
  integer(c_int), target :: pc(3),sc(6),bc(6),ft(7),cf(1),ct(12)
  type(c_ptr) :: sdid,total,base,nbor,sdoms,bounds,fsizes,buffer,hist,indices,zbound
  type(c_ptr) :: values(5)
  integer(c_int), pointer :: counts(:),pbase(:)
  integer(c_long_long), pointer :: region(:),ids(:)
  real(c_double), pointer :: x(:),y(:),z(:)
  integer(c_long_long), target :: rid,identity
  real(c_double), target :: xx,yy,zz
  call MPI_Init(mpierr)
  call MPI_Comm_rank(MPI_COMM_WORLD,rank,mpierr)
  call MPI_Comm_size(MPI_COMM_WORLD,nranks,mpierr)
  fields(1)=oh_particle_field(0,OH_PARTICLE_FIELD_LONG_LONG,OH_PARTICLE_ROLE_REGION,8_c_size_t)
  fields(2)=oh_particle_field(1,OH_PARTICLE_FIELD_DOUBLE,OH_PARTICLE_ROLE_X,8_c_size_t)
  fields(3)=oh_particle_field(2,OH_PARTICLE_FIELD_DOUBLE,OH_PARTICLE_ROLE_Y,8_c_size_t)
  fields(4)=oh_particle_field(3,OH_PARTICLE_FIELD_DOUBLE,OH_PARTICLE_ROLE_Z,8_c_size_t)
  fields(5)=oh_particle_field(4,OH_PARTICLE_FIELD_LONG_LONG,OH_PARTICLE_ROLE_PAYLOAD,8_c_size_t)
#if defined(OH_LIB_LEVEL_4P) || defined(OH_LIB_LEVEL_4S)
  ctx=oh_default_context()
#else
  call oh_context_create(ctx,int(MPI_COMM_WORLD,c_int),ierr)
  if(ierr/=MPI_SUCCESS) stop 1
  call oh_context_configure_particles(ctx,1_c_int,10000_c_int)
#endif
  call oh_context_set_particle_storage(ctx,OH_PARTICLE_STORAGE_SOA,ierr)
  if(ierr/=MPI_SUCCESS) stop 2
  call oh_context_configure_soa(ctx,fields,ierr)
  if(ierr/=MPI_SUCCESS) stop 3
  pc=[nranks,1,1];sc=[0,nranks,0,1,0,1];bc=0;ft=0;ft(1)=-1;cf=-1;ct=0
  sdid=c_null_ptr;total=c_null_ptr;base=c_null_ptr;nbor=c_null_ptr;sdoms=c_null_ptr
  bounds=c_null_ptr;fsizes=c_null_ptr;buffer=c_null_ptr;hist=c_null_ptr
  indices=c_null_ptr;zbound=c_null_ptr
#ifdef OH_LIB_LEVEL_4P
  capacity=oh4p_max_local_particles_raw(int(nranks*2,c_long_long),100_c_int,8_c_int,2_c_int)
  call oh4p_init_raw(sdid,1_c_int,100_c_int,total,buffer,base,capacity,c_null_ptr,nbor,pc, &
    sdoms,sc,1_c_int,bc,bounds,ft,cf,ct,fsizes,0_c_int,0_c_int,0_c_int)
  call oh4p_per_grid_histogram_raw(hist)
#elif defined(OH_LIB_LEVEL_4S)
  bc=1
  call oh4s_init_raw(sdid,1_c_int,100_c_int,int(nranks*2,c_long_long),8_c_int,2_c_int, &
    total,base,capacity,cbufsize,c_null_ptr,nbor,pc,sdoms,sc,2_c_int,bc,bounds,ft,cf,ct,fsizes, &
    zbound,0_c_int,0_c_int,0_c_int)
  call oh4s_particle_buffer_raw(capacity,buffer)
  call oh4s_per_grid_histogram_raw(hist,indices)
#else
  capacity=16
  call oh_context_configure_level3(ctx,c_loc(pc),c_null_ptr,c_loc(sc),1_c_int, &
    c_loc(bc),c_null_ptr,c_loc(ft),c_loc(cf),c_null_ptr,c_null_ptr)
  call oh_context_bind_soa_particles(ctx,capacity,OH_PARTICLES_OWNED,ierr)
  if(ierr/=MPI_SUCCESS) stop 4
  call oh_context_bind_particle_accounting(ctx,hist,total,base,OH_PARTICLES_OWNED)
  call c_f_pointer(hist,counts,[2*nranks])
#endif
  call c_f_pointer(base,pbase,[3])
  call c_f_pointer(oh_context_soa_field(ctx,0_c_int),region,[capacity])
  call c_f_pointer(oh_context_soa_field(ctx,1_c_int),x,[capacity])
  call c_f_pointer(oh_context_soa_field(ctx,2_c_int),y,[capacity])
  call c_f_pointer(oh_context_soa_field(ctx,3_c_int),z,[capacity])
  call c_f_pointer(oh_context_soa_field(ctx,4_c_int),ids,[capacity])
  x(1)=rank+0.5_c_double;y(1)=0.5_c_double;z(1)=0.5_c_double;ids(1)=rank
#ifdef OH_LIB_LEVEL_4P
  dst=oh4p_map_soa_particle_to_subdomain(0_c_int,0_c_int,0_c_int)
  mode=oh4p_transbound_raw(OH_MODE_NORMAL_PRIMARY,0_c_int)
#elif defined(OH_LIB_LEVEL_4S)
  dst=oh4s_map_soa_particle_to_subdomain(0_c_int,0_c_int,0_c_int)
  mode=oh4s_transbound_raw(OH_MODE_NORMAL_PRIMARY,0_c_int)
#else
  dst=oh_context_map_soa_particle_to_neighbor(ctx,0_c_int,0_c_int)
  counts(dst+1)=1
  call oh_context_set_total_particles(ctx)
  mode=oh_context_transbound3(ctx,OH_MODE_NORMAL_PRIMARY,0_c_int)
#endif
  if(dst/=rank .or. pbase(3)<1) stop 5
  rid=rank;identity=100+rank;xx=rank+0.5_c_double;yy=0.5_c_double;zz=0.5_c_double
  values=[c_loc(rid),c_loc(xx),c_loc(yy),c_loc(zz),c_loc(identity)]
#ifdef OH_LIB_LEVEL_4P
  ierr=oh4p_inject_soa_particle(values,5_c_int,0_c_int,ref,dst)
#elif defined(OH_LIB_LEVEL_4S)
  ierr=oh4s_inject_soa_particle(values,5_c_int,0_c_int,ref,dst)
#else
  call oh_context_inject_soa_particle(ctx,values,ref,ierr)
#endif
  if(ierr/=MPI_SUCCESS .or. .not.oh_context_particle_ref_valid(ctx,ref)) stop 6
  if(ids(ref%index+1)/=identity) stop 7
#ifdef OH_LIB_LEVEL_4P
  call oh4p_remove_mapped_soa_particle(ref%index,0_c_int,0_c_int)
  dst=oh4p_remap_soa_particle_to_subdomain(ref%index,0_c_int,0_c_int)
#elif defined(OH_LIB_LEVEL_4S)
  call oh4s_remove_mapped_soa_particle(ref%index,0_c_int,0_c_int)
  dst=oh4s_remap_soa_particle_to_subdomain(ref%index,0_c_int,0_c_int)
#else
  call oh_context_remove_injected_particle_ref(ctx,ref,ierr)
  call oh_context_remap_injected_particle_ref(ctx,ref,ierr)
#endif
  ! Remove old particles before the next transfer (their mapping counts are zero).
  region(1:pbase(3))=-1
#ifdef OH_LIB_LEVEL_4P
  mode=oh4p_transbound_raw(mode,0_c_int)
#elif defined(OH_LIB_LEVEL_4S)
  mode=oh4s_transbound_raw(mode,0_c_int)
#else
  mode=oh_context_transbound3(ctx,mode,0_c_int)
#endif
  if(oh_context_particle_ref_valid(ctx,ref)) stop 8
  if(pbase(3)<1) stop 9
  if(any(ids(1:pbase(3))<100)) stop 10
#if defined(OH_LIB_LEVEL_4P) || defined(OH_LIB_LEVEL_4S)
  call oh_context_unbind_particles(ctx)
#else
  call oh_context_destroy(ctx)
#endif
  call MPI_Finalize(mpierr)
end program
