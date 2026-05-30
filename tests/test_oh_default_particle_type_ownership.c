#include <assert.h>
#include <mpi.h>

#include "ohhelp2.h"
#include "ohhelp1_internal.h"
#include "ohhelp2_internal.h"
#include "oh_context_internal.h"
#include "oh_particle_adapter.h"

static void
run_default_oh2_init(int nranks) {
  int *sdid = 0;
  int *nphgram = 0;
  int *totalp = 0;
  void *pbuf = 0;
  int *pbase = 0;
  int *nbor = 0;
  int pcoord[3] = {nranks, 1, 1};

  oh2_init(&sdid, 1, 0, &nphgram, &totalp, &pbuf, &pbase, 4,
           0, &nbor, pcoord, 0, 0, 0);
  assert(sdid);
  assert(nphgram);
  assert(totalp);
  assert(pbuf);
  assert(pbase);
  assert(T_Particle != MPI_DATATYPE_NULL);
  assert(ownsTParticle);
  assert(ParticleAdapter.mpi_type == T_Particle);
  assert(OhDefaultState.particle_mpi_type == T_Particle);
  assert(OhDefaultState.owns_particle_mpi_type);
}

static void
run_default_oh2_init_with_custom_adapter(int nranks) {
  int *sdid = 0;
  int *nphgram = 0;
  int *totalp = 0;
  void *pbuf = 0;
  int *pbase = 0;
  int *nbor = 0;
  int pcoord[3] = {nranks, 1, 1};

  oh2_init(&sdid, 1, 0, &nphgram, &totalp, &pbuf, &pbase, 4,
           0, &nbor, pcoord, 0, 0, 0);
  assert(T_Particle != MPI_DATATYPE_NULL);
  assert(!ownsTParticle);
  assert(ownsCustomTParticle);
  assert(useCustomParticleAdapter);
  assert(useCustomTParticle);
  assert(T_Particle == CustomTParticle);
  assert(ParticleAdapter.mpi_type == CustomTParticle);
  assert(CustomParticleAdapter.mpi_type == CustomTParticle);
}

int
main(int argc, char **argv) {
  MPI_Datatype custom_type = MPI_DATATYPE_NULL;
  MPI_Datatype custom_adapter_type = MPI_DATATYPE_NULL;
  MPI_Datatype duplicated_adapter_type = MPI_DATATYPE_NULL;
  oh_particle_adapter adapter;
  int nranks;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nranks);

  run_default_oh2_init(nranks);

  oh2_set_particle_mpi_type(MPI_DATATYPE_NULL);
  assert(!ownsTParticle);
  assert(T_Particle == MPI_DATATYPE_NULL);
  assert(!OhDefaultState.owns_particle_mpi_type);

  run_default_oh2_init(nranks);
  assert(ownsTParticle);

  assert(oh_particle_adapter_make_byte_type(sizeof(struct S_particle),
                                            &custom_type) == MPI_SUCCESS);
  oh2_set_particle_mpi_type(custom_type);
  assert(!ownsTParticle);
  assert(useCustomTParticle);
  assert(CustomTParticle == custom_type);
  assert(!OhDefaultState.owns_particle_mpi_type);

  oh2_set_particle_mpi_type(MPI_DATATYPE_NULL);
  assert(!ownsTParticle);
  assert(!useCustomTParticle);
  MPI_Type_free(&custom_type);

  assert(oh_particle_adapter_make_byte_type(sizeof(struct S_particle),
                                            &custom_adapter_type) ==
         MPI_SUCCESS);
  adapter = oh_default_particle_adapter(custom_adapter_type);
  oh2_set_particle_adapter(&adapter);
  duplicated_adapter_type = CustomTParticle;
  assert(ownsCustomTParticle);
  assert(useCustomParticleAdapter);
  assert(useCustomTParticle);
  assert(duplicated_adapter_type != MPI_DATATYPE_NULL);
  assert(duplicated_adapter_type != custom_adapter_type);
  assert(CustomParticleAdapter.mpi_type == duplicated_adapter_type);
  MPI_Type_free(&custom_adapter_type);

  run_default_oh2_init_with_custom_adapter(nranks);
  assert(CustomTParticle == duplicated_adapter_type);
  oh2_set_particle_adapter(NULL);
  assert(!ownsCustomTParticle);
  assert(!useCustomParticleAdapter);
  assert(!useCustomTParticle);
  assert(CustomTParticle == MPI_DATATYPE_NULL);

  run_default_oh2_init(nranks);
  oh2_set_particle_mpi_type(MPI_DATATYPE_NULL);
  assert(!ownsTParticle);

  MPI_Finalize();
  return 0;
}
