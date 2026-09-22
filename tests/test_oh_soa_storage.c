#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "oh_particle_storage.h"
#include "oh_particle_ownership.h"
#include "ohhelp1_internal.h"
#include "oh_context_internal.h"

int main(int argc, char **argv) {
  oh_context *ctx = NULL;
  long long region[8] = {0};
  double x[8] = {0};
  unsigned char payload[8][3] = {{0}};
  void *bases[] = {region,x,payload};
  oh_particle_field fields[] = {
    {0,OH_PARTICLE_FIELD_LONG_LONG,OH_PARTICLE_ROLE_REGION,sizeof(long long)},
    {1,OH_PARTICLE_FIELD_DOUBLE,OH_PARTICLE_ROLE_X,sizeof(double)},
    {2,OH_PARTICLE_FIELD_BYTES,OH_PARTICLE_ROLE_PAYLOAD,3}
  };
  oh_particle_field bad[3];
  MPI_Init(&argc,&argv);
  assert(oh_context_create(MPI_COMM_WORLD,&ctx) == MPI_SUCCESS);
  oh_context_configure_particles(ctx,1,20);
  assert(oh_context_set_particle_storage(NULL,OH_PARTICLE_STORAGE_SOA) == MPI_ERR_ARG);
  assert(oh_context_set_particle_storage(ctx,17) == MPI_ERR_ARG);
  assert(oh_context_set_particle_storage(ctx,OH_PARTICLE_STORAGE_SOA) == MPI_SUCCESS);
  assert(oh_context_configure_soa(ctx,fields,3,0) == MPI_SUCCESS);
  for (int which=0;which<6;++which) {
    memcpy(bad,fields,sizeof(bad));
    if (which==0) bad[1].key=bad[0].key;
    if (which==1) bad[1].role=OH_PARTICLE_ROLE_REGION;
    if (which==2) bad[0].role=OH_PARTICLE_ROLE_PAYLOAD;
    if (which==3) bad[0].size=0;
    if (which==4) bad[0].type=OH_PARTICLE_FIELD_DOUBLE;
    if (which==5) bad[2].size=SIZE_MAX;
    assert(oh_context_configure_soa(ctx,bad,3,0) != MPI_SUCCESS);
    assert(ctx->soa->record_size == sizeof(long long)+sizeof(double)+3);
  }
  assert(oh_context_bind_soa_particles(ctx,bases,2,8,OH_PARTICLES_BORROWED) == MPI_ERR_ARG);
  assert(oh_context_bind_soa_particles(ctx,NULL,0,8,OH_PARTICLES_BORROWED) == MPI_ERR_ARG);
  assert(oh_context_bind_soa_particles(ctx,bases,3,8,OH_PARTICLES_OWNED) == MPI_ERR_ARG);
  void *overlap[] = {region,region,payload};
  assert(oh_context_bind_soa_particles(ctx,overlap,3,8,OH_PARTICLES_BORROWED) == MPI_ERR_ARG);
  assert(oh_context_bind_soa_particles(ctx,bases,3,8,OH_PARTICLES_BORROWED) == MPI_SUCCESS);
  assert(oh_context_set_particle_storage(ctx,OH_PARTICLE_STORAGE_AOS) == MPI_ERR_ARG);
  assert(oh_context_configure_soa(ctx,fields,3,0) == MPI_ERR_ARG);
  for (int i=0;i<8;++i) { region[i]=i; x[i]=i+0.5; memset(payload[i],i,3); }
  oh_soa_copy(ctx->soa,1,0,7); /* overlap downward */
  for (int i=1;i<8;++i) assert(region[i]==i-1 && x[i]==i-0.5 && payload[i][2]==i-1);
  oh_soa_copy(ctx->soa,0,1,7); /* overlap upward */
  for (int i=0;i<7;++i) assert(region[i]==i && x[i]==i+0.5 && payload[i][1]==i);
  int indices[] = {1,2,0,6,5,4,3}; /* cycles, requiring preserved sources */
  oh_soa_gather(ctx->soa,indices,7);
  for (int i=0;i<7;++i) assert(region[i]==indices[i] && x[i]==indices[i]+0.5 && payload[i][0]==indices[i]);
  unsigned char record[128];
  oh_soa_pack(ctx->soa,record+1,0,3); /* deliberately unaligned wire buffer */
  oh_soa_unpack(ctx->soa,4,record+1,3);
  for (int i=0;i<3;++i) assert(region[i+4]==indices[i] && x[i+4]==indices[i]+0.5);
  oh_soa_copy(ctx->soa,8,8,0);
  oh_context_unbind_particles(ctx);
  assert(region[4]==1); /* borrowed storage survived */
  assert(oh_context_bind_soa_particles(ctx,NULL,0,8,OH_PARTICLES_OWNED) == MPI_SUCCESS);
  assert(oh_context_soa_field(ctx,0) != region);
  assert(oh_context_soa_field(ctx,99) == NULL);
  oh_context_unbind_particles(ctx);
  assert(oh_context_bind_soa_particles(ctx,NULL,0,0,OH_PARTICLES_BORROWED) == MPI_SUCCESS);
  oh_soa_copy(ctx->soa,0,0,0); oh_soa_gather(ctx->soa,NULL,0);
  oh_context_unbind_particles(ctx);
  assert(oh_context_set_particle_storage(ctx,OH_PARTICLE_STORAGE_AOS) == MPI_SUCCESS);
  assert(oh_context_get_particle_storage(ctx) == OH_PARTICLE_STORAGE_AOS);
  oh_context_destroy(ctx);
  MPI_Finalize();
  return 0;
}
