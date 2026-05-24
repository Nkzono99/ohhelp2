#define OH_LIB_LEVEL 3
#include "ohhelp3.h"
#include "ohhelp_c.h"

#include <stddef.h>

struct my_particle {
  double x, y, z;
  long long region;
  int species;
};

int
main(void) {
  oh_particle_adapter *adapter = 0;
  MPI_Datatype type = MPI_DATATYPE_NULL;
  struct my_particle particle = {0};
  double size[OH_DIMENSION] = {0};
  double *weights = 0;
  void *raw_particles = 0;
  void *primary_field = 0;
  void *secondary_field = 0;
  int *sdid = 0;
  int **nphgram = 0;
  int *totalp[2] = {0, 0};
  int *pbase = 0;
  int *nbor = 0;
  int pcoord[OH_DIMENSION] = {0};
  int *sdoms = 0;
  int scoord[OH_DIMENSION][2] = {{0}};
  int bcond[OH_DIMENSION][2] = {{0}};
  int *bounds = 0;
  int ftypes[OH_FTYPE_N] = {0};
  int cfields[1] = {-1};
  int ctypes[2][OH_CTYPE_N] = {{0}};
  int *fsizes = 0;

  oh_set_region_weights(weights);
  oh_set_particle_mpi_type(type);
  oh_set_particle_position_fields(adapter,
                                  offsetof(struct my_particle, x),
                                  offsetof(struct my_particle, y),
                                  offsetof(struct my_particle, z));
  oh_set_particle_adapter(adapter);
  (void)oh_max_local_particles(0, 0, 0);

  oh_init(&sdid, 1, 0, nphgram, totalp, &raw_particles, &pbase, 0,
          0, &nbor, pcoord, &sdoms, &scoord[0][0], 0, &bcond[0][0],
          &bounds, ftypes, cfields, &ctypes[0][0], &fsizes, 0, 0, 0);

  oh_grid_size(size);
  oh_bcast_field(primary_field, secondary_field, 0);
  oh_reduce_field(primary_field, secondary_field, 0);
  oh_allreduce_field(primary_field, secondary_field, 0);
  oh_exchange_borders(primary_field, secondary_field, 0,
                      OH_MODE_NORMAL_PRIMARY);

#if OH_DIMENSION == 1
  (void)oh_map_particle_to_neighbor(&particle.x, 0);
  (void)oh_map_particle_to_subdomain(particle.x);
#elif OH_DIMENSION == 2
  (void)oh_map_particle_to_neighbor(&particle.x, &particle.y, 0);
  (void)oh_map_particle_to_subdomain(particle.x, particle.y);
#else
  (void)oh_map_particle_to_neighbor(&particle.x, &particle.y, &particle.z, 0);
  (void)oh_map_particle_to_subdomain(particle.x, particle.y, particle.z);
#endif

  (void)oh_transbound(OH_MODE_NORMAL_PRIMARY, 0);
  (void)OH_MODE_NORMAL_SECONDARY;
  (void)OH_MODE_REBALANCE_SECONDARY;
  (void)OH_MODE_ANY_PRIMARY;
  (void)OH_MODE_ANY_SECONDARY;
  return 0;
}
