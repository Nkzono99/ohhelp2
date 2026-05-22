/* Minimal Level-3 custom particle layout setup for v2.0.
   This sample is compile-checked by scripts/docker-build-test.sh. */
#include <stddef.h>
#include <stdlib.h>

#define OH_LIB_LEVEL 3
#include "ohhelp_c.h"

struct pic_particle {
  double x, y, z;
  double vx, vy, vz;
  long long region;
  int species;
};

static int
configure_particle_adapter(oh_particle_adapter *adapter,
                           MPI_Datatype *particle_type) {
  int err = oh_particle_adapter_make_byte_type(sizeof(struct pic_particle),
                                               particle_type);
  if (err != MPI_SUCCESS) return err;

  *adapter = oh_default_particle_adapter(*particle_type);
  adapter->stride = sizeof(struct pic_particle);
  oh_particle_adapter_use_integer_fields(
    adapter,
    offsetof(struct pic_particle, region),
    sizeof(((struct pic_particle*)0)->region),
    offsetof(struct pic_particle, species),
    sizeof(((struct pic_particle*)0)->species));
  oh_set_particle_position_fields(adapter,
                                  offsetof(struct pic_particle, x),
                                  offsetof(struct pic_particle, y),
                                  offsetof(struct pic_particle, z));
  oh_set_particle_adapter(adapter);
  return MPI_SUCCESS;
}

static void
initialize_level3(int **sdid, int nspec, int maxfrac, int **nphgram,
                  int **totalp, struct pic_particle **particles,
                  int **pbase, int maxlocalp, void *mycomm,
                  int **nbor, int pcoord[OH_DIMENSION],
                  int **sdoms, int scoord[OH_DIMENSION][2],
                  int nbound, int bcond[OH_DIMENSION][2],
                  int **bounds, int *ftypes, int *cfields,
                  int *ctypes, int **fsizes) {
  void *raw_particles = *particles;

  oh_init(sdid, nspec, maxfrac, nphgram, totalp, &raw_particles, pbase,
          maxlocalp, mycomm, nbor, pcoord, sdoms, &scoord[0][0],
          nbound, &bcond[0][0], bounds, ftypes, cfields, ctypes, fsizes,
          0, 0, 0);
  *particles = raw_particles;
}

static void
push_one_particle(oh_particle_adapter *adapter, struct pic_particle *particle,
                  int primary_or_secondary, int species, int self_region,
                  int **nphgram) {
  if (particle->region == self_region) return;

  {
    int dst = (int)adapter->map_to_neighbor(adapter, particle,
                                            primary_or_secondary);
    adapter->set_region(adapter, particle, dst, primary_or_secondary);
    nphgram[species][self_region]--;
    nphgram[species][dst]++;
  }
}

int
main(int argc, char **argv) {
  oh_particle_adapter adapter;
  MPI_Datatype particle_type = MPI_DATATYPE_NULL;
  struct pic_particle *particles = NULL;

  (void)argv;
  MPI_Init(&argc, &argv);
  if (configure_particle_adapter(&adapter, &particle_type) != MPI_SUCCESS) {
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  /* Real applications allocate histograms, fields, and geometry before
     calling initialize_level3(). */
  (void)initialize_level3;
  (void)push_one_particle;
  (void)particles;

  oh_set_particle_adapter(NULL);
  MPI_Type_free(&particle_type);
  MPI_Finalize();
  return 0;
}
