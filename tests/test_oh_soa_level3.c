#include <assert.h>
#include <string.h>
#include "ohhelp_c.h"

int main(int argc, char **argv) {
  oh_context *ctx;
  int rank,n, *hist=NULL,*total=NULL,*base=NULL;
  int pc[OH_DIMENSION],sc[OH_DIMENSION][2],bc[OH_DIMENSION][2]={{0}};
  int ft[1][OH_FTYPE_N]={{-1,0,0,0,0,0,0}},cf[]={-1};
  oh_particle_field fields[]={
    {0,OH_PARTICLE_FIELD_LONG_LONG,OH_PARTICLE_ROLE_REGION,sizeof(long long)},
    {1,OH_PARTICLE_FIELD_DOUBLE,OH_PARTICLE_ROLE_X,sizeof(double)},
    {2,OH_PARTICLE_FIELD_DOUBLE,OH_PARTICLE_ROLE_Y,sizeof(double)},
    {3,OH_PARTICLE_FIELD_DOUBLE,OH_PARTICLE_ROLE_Z,sizeof(double)},
    {4,OH_PARTICLE_FIELD_LONG_LONG,OH_PARTICLE_ROLE_PAYLOAD,sizeof(long long)}
  };
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank); MPI_Comm_size(MPI_COMM_WORLD,&n);
  assert(oh_context_create(MPI_COMM_WORLD,&ctx)==MPI_SUCCESS);
  oh_context_configure_particles(ctx,1,10000);
  assert(oh_context_set_particle_storage(ctx,OH_PARTICLE_STORAGE_SOA)==MPI_SUCCESS);
  assert(oh_context_configure_soa(ctx,fields,5,0)==MPI_SUCCESS);
  for(int d=0;d<OH_DIMENSION;++d) {pc[d]=d==0?n:1;sc[d][0]=0;sc[d][1]=pc[d];}
  oh_context_configure_level3(ctx,pc,NULL,&sc[0][0],1,&bc[0][0],NULL,&ft[0][0],cf,NULL,NULL);
  assert(oh_context_bind_soa_particles(ctx,NULL,0,16,OH_PARTICLES_OWNED)==MPI_SUCCESS);
  oh_context_bind_particle_accounting(ctx,&hist,&total,&base,OH_PARTICLES_OWNED);
  long long *region=oh_context_soa_field(ctx,0),*id=oh_context_soa_field(ctx,4);
  double *x=oh_context_soa_field(ctx,1),*y=oh_context_soa_field(ctx,2),*z=oh_context_soa_field(ctx,3);
  x[0]=rank+1.25; y[0]=z[0]=0.5; id[0]=rank;
  int dst=oh_context_map_soa_particle_to_neighbor(ctx,0,0);
  assert(dst==(rank+1)%n); assert(x[0]==dst+0.25); assert(region[0]==dst);
  hist[dst]=1; oh_context_set_total_particles(ctx);
  int mode=oh_context_transbound3(ctx,OH_MODE_NORMAL_PRIMARY,0);
  assert(base[2]==1 && region[0]==rank && id[0]==(rank+n-1)%n);
  assert(x[0]==rank+0.25 && y[0]==0.5 && z[0]==0.5);
  hist[rank]=1;
  long long r=1234,identity=100+rank; double xx=rank+0.75,yy=0.5,zz=0.5;
  const void *values[]={&r,&xx,&yy,&zz,&identity}; oh_particle_ref ref;
  assert(oh_context_inject_soa_particle(ctx,values,5,&ref)==MPI_SUCCESS);
  assert(region[ref.index]==rank);
  assert(oh_context_remove_injected_particle_ref(ctx,ref)==MPI_SUCCESS);
  x[ref.index]=(rank+1)%n+0.75;
  assert(oh_context_remap_injected_particle_ref(ctx,ref)==MPI_SUCCESS);
  mode=oh_context_transbound3(ctx,mode,0);
  assert(base[2]==2);
  assert(oh_context_remove_injected_particle_ref(ctx,ref)!=MPI_SUCCESS);
  oh_context_destroy(ctx); MPI_Finalize(); return 0;
}
