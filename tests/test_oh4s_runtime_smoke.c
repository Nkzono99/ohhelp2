#include <assert.h>
#include <stddef.h>
#include <string.h>

#include <mpi.h>

#include "oh_particle_adapter.h"
#include "oh_part.h"
#include "ohhelp1.h"
#include "ohhelp2.h"
#include "ohhelp3.h"
#include "ohhelp4s.h"

extern int *InjectedParticles;
extern int *NOfPLocal;
extern int nOfInjections;

struct level4_custom_particle {
  double x, y, z;
  long long region;
  int species;
  int pid;
  long long trace_id;
};

static void
configure_custom_particle_adapter(oh_particle_adapter *adapter,
                                  MPI_Datatype *custom_type) {
  assert(oh_particle_adapter_make_byte_type(
           sizeof(struct level4_custom_particle), custom_type) ==
         MPI_SUCCESS);
  *adapter = oh_default_particle_adapter(*custom_type);
  adapter->stride = sizeof(struct level4_custom_particle);
  oh_particle_adapter_use_integer_fields(
    adapter,
    offsetof(struct level4_custom_particle, region),
    sizeof(((struct level4_custom_particle*)0)->region),
    offsetof(struct level4_custom_particle, species),
    sizeof(((struct level4_custom_particle*)0)->species));
  oh_particle_adapter_use_position_fields(
    adapter,
    offsetof(struct level4_custom_particle, x),
    offsetof(struct level4_custom_particle, y),
    offsetof(struct level4_custom_particle, z));
  oh2_set_particle_adapter(adapter);
}

static void
run_default_particle_path(void *pbuf, int maxlocalp, int rank, int *pbase,
                          int *totalp) {
  struct S_particle *particles = (struct S_particle*)pbuf;
  struct S_particle *active_particles;
  struct S_particle injected;
  int injected_index = -1;
  int injected_before;
  int local_before;
  int mapped;
  int mode;
  int pending_before;

  particles[0].x = (double)rank + 0.5;
  particles[0].y = 0.5;
  particles[0].z = 0.5;
  particles[0].vx = particles[0].vy = particles[0].vz = 0.0;
  particles[0].pid = rank;
  particles[0].preside = 0;
  particles[0].trace_id = rank;
  particles[0].nid = rank;
  particles[0].spec = 0;

  mapped = oh4s_map_particle_to_subdomain(&particles[0], 0, 0);
  assert(mapped == rank);
  mode = oh4s_transbound(OH_MODE_NORMAL_PRIMARY, 0);
  assert(mode == OH_MODE_NORMAL_PRIMARY);
  assert(pbase[0] == 0);
  assert(pbase[1] == 2);
  assert(pbase[2] == 2);
  assert(totalp[0] == 2);
  assert(totalp[1] == 0);

  active_particles = particles + maxlocalp;
  local_before = NOfPLocal[rank];
  injected_before = InjectedParticles[0];
  pending_before = nOfInjections;
  injected.x = (double)rank + 0.5;
  injected.y = 0.5;
  injected.z = 0.5;
  injected.vx = injected.vy = injected.vz = 0.0;
  injected.pid = 100 + rank;
  injected.preside = 0;
  injected.trace_id = 1000 + rank;
  injected.nid = rank;
  injected.spec = 0;
  mapped = oh4s_inject_particle(&injected, 0);
  assert(mapped == rank);
  assert(nOfInjections == pending_before + 1);
  assert(NOfPLocal[rank] == local_before + 1);
  assert(InjectedParticles[0] == injected_before + 1);
  for (int i = 0; i < maxlocalp; i++) {
    if (active_particles[i].pid == 100 + rank &&
        active_particles[i].trace_id == 1000 + rank) {
      injected_index = i;
      break;
    }
  }
  assert(injected_index >= 0);
  assert(active_particles[injected_index].pid == 100 + rank);
  assert(active_particles[injected_index].trace_id == 1000 + rank);
  assert(active_particles[injected_index].nid >= 0);

  mapped = oh4s_remap_particle_to_subdomain(
    &active_particles[injected_index], 0, 0);
  assert(mapped == rank);
  assert(active_particles[injected_index].nid >= 0);
  assert(nOfInjections == pending_before + 1);
  assert(NOfPLocal[rank] == local_before + 1);
  assert(InjectedParticles[0] == injected_before + 1);

  oh4s_remove_mapped_particle(&active_particles[injected_index], 0, 0);
  assert(active_particles[injected_index].nid == -1);
  assert(nOfInjections == pending_before + 1);
  assert(NOfPLocal[rank] == local_before);
  assert(InjectedParticles[0] == injected_before);
  mapped = oh4s_remap_particle_to_subdomain(
    &active_particles[injected_index], 0, 0);
  assert(mapped == rank);
  assert(active_particles[injected_index].nid >= 0);
  assert(nOfInjections == pending_before + 1);
  assert(NOfPLocal[rank] == local_before + 1);
  assert(InjectedParticles[0] == injected_before + 1);

  mode = oh4s_transbound(OH_MODE_NORMAL_PRIMARY, 0);
  assert(mode == OH_MODE_NORMAL_PRIMARY);
  assert(pbase[0] == 0);
  assert(pbase[1] == 2);
  assert(pbase[2] == 2);
  assert(totalp[0] == 2);
  assert(totalp[1] == 0);
  assert(nOfInjections == 0);
  assert(InjectedParticles[0] == 0);
}

static void
run_weighted_load_path(void *pbuf, int rank, int *sdid, int *pbase,
                       int *totalp, int run_secondary_transbound) {
  struct S_particle *particles = (struct S_particle*)pbuf;
  double weights[2] = {4.0, 1.0};
  int mapped;
  int mode;

  particles[0].x = (double)rank + 0.5;
  particles[0].y = 0.5;
  particles[0].z = 0.5;
  particles[0].vx = particles[0].vy = particles[0].vz = 0.0;
  particles[0].pid = rank;
  particles[0].preside = 0;
  particles[0].trace_id = rank;
  particles[0].nid = rank;
  particles[0].spec = 0;

  mapped = oh4s_map_particle_to_subdomain(&particles[0], 0, 0);
  assert(mapped == rank);
  oh1_set_region_weights(weights);
  mode = oh4s_transbound(OH_MODE_NORMAL_PRIMARY, 0);
  assert(mode == OH_MODE_REBALANCE_SECONDARY);
  if (rank == 0) {
    assert(sdid[1] == -1);
    assert(pbase[0] == 0);
    assert(pbase[1] == 0);
    assert(pbase[2] == 0);
    assert(totalp[0] == 0);
    assert(totalp[1] == 0);
  } else {
    assert(sdid[1] == 0);
    assert(pbase[0] == 0);
    assert(pbase[1] == 2);
    assert(pbase[2] == 4);
    assert(totalp[0] == 2);
    assert(totalp[1] == 2);
  }
  if (run_secondary_transbound) {
    mode = oh4s_transbound(OH_MODE_REBALANCE_SECONDARY, 0);
    assert(mode == OH_MODE_NORMAL_PRIMARY);
    assert(sdid[1] == -1);
  }
  oh1_set_region_weights(NULL);
}

static void
run_custom_particle_path(void *pbuf, int maxlocalp, int rank, int *pbase,
                         int *totalp) {
  struct level4_custom_particle *particles =
    (struct level4_custom_particle*)pbuf;
  struct level4_custom_particle *active_particles;
  struct level4_custom_particle injected;
  int injected_index = -1;
  int injected_before;
  int local_before;
  int mapped;
  int mode;
  int pending_before;

  particles[0].x = (double)rank + 0.5;
  particles[0].y = 0.5;
  particles[0].z = 0.5;
  particles[0].pid = rank;
  particles[0].trace_id = rank;
  particles[0].region = rank;
  particles[0].species = 0;

  mapped = oh4s_map_particle_to_subdomain(&particles[0], 0, 0);
  assert(mapped == rank);
  mode = oh4s_transbound(OH_MODE_NORMAL_PRIMARY, 0);
  assert(mode == OH_MODE_NORMAL_PRIMARY);
  assert(pbase[0] == 0);
  assert(pbase[1] == 2);
  assert(pbase[2] == 2);
  assert(totalp[0] == 2);
  assert(totalp[1] == 0);

  active_particles = particles + maxlocalp;
  local_before = NOfPLocal[rank];
  injected_before = InjectedParticles[0];
  pending_before = nOfInjections;
  injected.x = (double)rank + 0.5;
  injected.y = 0.5;
  injected.z = 0.5;
  injected.pid = 100 + rank;
  injected.trace_id = 1000 + rank;
  injected.region = rank;
  injected.species = 0;
  mapped = oh4s_inject_particle(&injected, 0);
  assert(mapped == rank);
  assert(nOfInjections == pending_before + 1);
  assert(NOfPLocal[rank] == local_before + 1);
  assert(InjectedParticles[0] == injected_before + 1);
  for (int i = 0; i < maxlocalp; i++) {
    if (active_particles[i].pid == 100 + rank &&
        active_particles[i].trace_id == 1000 + rank) {
      injected_index = i;
      break;
    }
  }
  assert(injected_index >= 0);
  assert(active_particles[injected_index].pid == 100 + rank);
  assert(active_particles[injected_index].trace_id == 1000 + rank);
  assert(active_particles[injected_index].region >= 0);

  mapped = oh4s_remap_particle_to_subdomain(
    &active_particles[injected_index], 0, 0);
  assert(mapped == rank);
  assert(active_particles[injected_index].region >= 0);
  assert(nOfInjections == pending_before + 1);
  assert(NOfPLocal[rank] == local_before + 1);
  assert(InjectedParticles[0] == injected_before + 1);

  oh4s_remove_mapped_particle(&active_particles[injected_index], 0, 0);
  assert(active_particles[injected_index].region == -1);
  assert(nOfInjections == pending_before + 1);
  assert(NOfPLocal[rank] == local_before);
  assert(InjectedParticles[0] == injected_before);
  mapped = oh4s_remap_particle_to_subdomain(
    &active_particles[injected_index], 0, 0);
  assert(mapped == rank);
  assert(active_particles[injected_index].region >= 0);
  assert(nOfInjections == pending_before + 1);
  assert(NOfPLocal[rank] == local_before + 1);
  assert(InjectedParticles[0] == injected_before + 1);

  mode = oh4s_transbound(OH_MODE_NORMAL_PRIMARY, 0);
  assert(mode == OH_MODE_NORMAL_PRIMARY);
  assert(pbase[0] == 0);
  assert(pbase[1] == 2);
  assert(pbase[2] == 2);
  assert(totalp[0] == 2);
  assert(totalp[1] == 0);
  assert(nOfInjections == 0);
  assert(InjectedParticles[0] == 0);
}

int
main(int argc, char **argv) {
  oh_particle_adapter adapter;
  MPI_Datatype custom_type = MPI_DATATYPE_NULL;
  int rank = 0;
  int nranks = 0;
  int *sdid = NULL;
  int *totalp = NULL;
  int *pbase = NULL;
  void *pbuf = NULL;
  int *pghgram = NULL;
  int *pgindex = NULL;
  int *nbor = NULL;
  int *sdoms = NULL;
  int *bounds = NULL;
  int *fsizes = NULL;
  int *zbound = NULL;
  int pcoord[3] = {2, 1, 1};
  int scoord[6] = {0, 2, 0, 1, 0, 1};
  int bcond[6] = {1, 1, 1, 1, 1, 1};
  int ftypes[1][OH_FTYPE_N] = {{-1, 0, 0, 0, 0, 0, 0}};
  int cfields[1] = {-1};
  int ctypes[1][2][OH_CTYPE_N] = {{{0}}};
  int maxlocalp = 0;
  int cbufsize = 0;
  int use_custom_adapter;
  int use_weighted_load;
  int run_weighted_secondary;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nranks);
  assert(nranks == 2);
  use_custom_adapter = argc > 1 && strcmp(argv[1], "custom-adapter") == 0;
  use_weighted_load = argc > 1 && strcmp(argv[1], "weighted-load") == 0;
  run_weighted_secondary =
    argc > 1 && strcmp(argv[1], "weighted-secondary") == 0;
  if (use_custom_adapter)
    configure_custom_particle_adapter(&adapter, &custom_type);

  oh4s_init(&sdid, 1, 20, 2, 1, 1, &totalp, &pbase, &maxlocalp,
            &cbufsize, NULL, &nbor, pcoord, &sdoms, scoord, 2, bcond,
            &bounds, (int*)ftypes, cfields, (int*)ctypes, &fsizes, &zbound,
            0, 0, 0);
  assert(sdid != NULL);
  assert(sdid[0] == rank);
  assert(sdid[1] == -1);
  assert(totalp != NULL);
  assert(pbase != NULL);
  assert(nbor != NULL);
  assert(sdoms != NULL);
  assert(bounds != NULL);
  assert(fsizes != NULL);
  assert(zbound != NULL);
  assert(maxlocalp == 56);
  assert(cbufsize == 6);
  oh4s_particle_buffer(maxlocalp, &pbuf);
  assert(pbuf != NULL);
  oh4s_per_grid_histogram(&pghgram, &pgindex);
  assert(pghgram != NULL);
  assert(pgindex != NULL);

  if (use_weighted_load || run_weighted_secondary)
    run_weighted_load_path(pbuf, rank, sdid, pbase, totalp,
                           run_weighted_secondary);
  else if (use_custom_adapter)
    run_custom_particle_path(pbuf, maxlocalp, rank, pbase, totalp);
  else
    run_default_particle_path(pbuf, maxlocalp, rank, pbase, totalp);

  if (use_custom_adapter) {
    oh2_set_particle_adapter(NULL);
    MPI_Type_free(&custom_type);
  }
  MPI_Finalize();
  return 0;
}
