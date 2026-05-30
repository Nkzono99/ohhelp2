#include <assert.h>
#include <mpi.h>

#include "oh_context.h"
#include "oh_particle_ownership.h"
#include "ohhelp1_internal.h"
#include "oh_context_internal.h"

int
main(int argc, char **argv) {
  oh_context *context;
  int borrowed[2];
  int copied[2];
  int *bound;
  int rank;
  int nranks;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nranks);

  myRank = rank;
  nOfNodes = nranks;
  RegionId[0] = rank;
  RegionId[1] = -1;
  SubdomainId = RegionId;
  oh1_sync_default_state();

  context = oh_default_context();
  bound = oh_context_bind_region_ids(context, 0, OH_PARTICLES_OWNED);
  assert(bound == RegionId);
  assert(context->region_id == RegionId);
  assert(context->subdomain_id == RegionId);
  assert(context->owns_region_id == 0);
  oh_default_context();
  assert(context->region_id == RegionId);
  assert(context->owns_region_id == 0);
  oh_context_unbind_region_ids(context);
  assert(SubdomainId == RegionId);
  assert(context->owns_region_id == 0);

  borrowed[0] = rank + 10;
  borrowed[1] = -1;
  bound = oh_context_bind_region_ids(context, borrowed,
                                     OH_PARTICLES_BORROWED);
  assert(bound == borrowed);
  assert(context->region_id == RegionId);
  assert(context->subdomain_id == borrowed);
  assert(context->owns_region_id == 0);
  oh_default_context();
  assert(context->region_id == RegionId);
  assert(context->subdomain_id == borrowed);
  assert(context->owns_region_id == 0);
  oh_context_get_region_ids(context, copied);
  assert(copied[0] == borrowed[0]);
  assert(copied[1] == borrowed[1]);

  oh_context_unbind_region_ids(context);
  assert(SubdomainId == RegionId);
  assert(context->region_id == RegionId);
  assert(context->subdomain_id == RegionId);
  assert(context->owns_region_id == 0);

  MPI_Finalize();
  return 0;
}
