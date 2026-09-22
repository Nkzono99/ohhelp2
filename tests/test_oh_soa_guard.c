#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "oh_particle_storage.h"
#include "oh_particle_ownership.h"

int main(int argc, char **argv) {
  oh_context *ctx = NULL;
  int rank,n,capacity=4;
  long long regions[4] = {0};
  double value[4] = {0};
  void *bases[] = {regions,value};
  int *hist=NULL,*total=NULL,*pbase=NULL;
  oh_particle_field fields[] = {
    {0,OH_PARTICLE_FIELD_LONG_LONG,OH_PARTICLE_ROLE_REGION,sizeof(long long)},
    {1,OH_PARTICLE_FIELD_DOUBLE,OH_PARTICLE_ROLE_PAYLOAD,sizeof(double)}
  };
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank); MPI_Comm_size(MPI_COMM_WORLD,&n);
  assert(argc==2);
  assert(oh_context_create(MPI_COMM_WORLD,&ctx) == MPI_SUCCESS);
  oh_context_configure_particles(ctx,1,10000);
  if (!strcmp(argv[1],"backend") && rank) {
    oh_context_bind_particles(ctx,NULL,capacity,OH_PARTICLES_OWNED);
  } else {
    assert(oh_context_set_particle_storage(ctx,OH_PARTICLE_STORAGE_SOA) == MPI_SUCCESS);
    if (!strcmp(argv[1],"schema") && rank) fields[1].key=10;
    assert(oh_context_configure_soa(ctx,fields,2,0) == MPI_SUCCESS);
    if (!strcmp(argv[1],"pointer-api"))
      oh_context_bind_particles(ctx,NULL,capacity,OH_PARTICLES_OWNED);
    if (!strcmp(argv[1],"capacity") && rank) capacity=0;
    assert(oh_context_bind_soa_particles(ctx,bases,2,capacity,OH_PARTICLES_BORROWED) == MPI_SUCCESS);
  }
  oh_context_bind_particle_accounting(ctx,&hist,&total,&pbase,OH_PARTICLES_OWNED);
  if (!strcmp(argv[1],"capacity") && !rank) {
    assert(n==2); regions[0]=1; hist[1]=1;
  }
  oh_context_set_total_particles(ctx);
  if (!strcmp(argv[1],"level3"))
    (void)oh_context_transbound3(ctx,OH_MODE_NORMAL_PRIMARY,0);
  (void)oh_context_transbound2(ctx,OH_MODE_NORMAL_PRIMARY,0);
  /* Negative cases must abort with a diagnostic, not silently fall back. */
  MPI_Finalize();
  return 0;
}
