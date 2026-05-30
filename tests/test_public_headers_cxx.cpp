#include "oh_context.h"
#include "oh_load_balance.h"
#include "oh_particle_adapter.h"
#include "ohhelp_c.h"

#include <cassert>
#include <cstddef>
#include <vector>

struct CustomParticle {
  int region;
  int species;
  double x;
  double y;
  double z;
};

OH_DEFINE_PARTICLE_ADAPTER_ACCESSORS(custom_particle, CustomParticle, region,
                                     species)
OH_DEFINE_PARTICLE_ADAPTER_REGION_MAPPING(custom_particle, CustomParticle,
                                          region)

int
main(int argc, char **argv) {
  oh_context *context = nullptr;
  MPI_Datatype particle_type = MPI_DATATYPE_NULL;
  oh_particle_adapter adapter = {};
  int rank = 0;
  int nprocs = 1;
  const int nspec = 1;
  MPI_Comm comm = MCW;
  MPI_Comm app_comm = MPI_COMM_NULL;
  double load = oh_region_load(2, 3.0);

  MPI_Init(&argc, &argv);
  comm = MCW;
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &nprocs);

  MPI_Comm_dup(comm, &app_comm);
  assert(oh_context_create(app_comm, &context) == MPI_SUCCESS);
  assert(context != nullptr);
  MPI_Comm_free(&app_comm);
  oh_context_configure_particles(context, nspec, 0);
  assert(oh_context_region_count(context) == nprocs);
  assert(!oh_context_is_level3_configured(context));

  std::vector<double> weights(static_cast<std::size_t>(nprocs), 1.0);
  if (!weights.empty()) weights[0] = 2.0;
  oh_context_set_region_weights(context, weights.data());
  oh_context_set_region_weights_n(context, weights.data(),
                                  static_cast<int>(weights.size()));
  oh_context_set_region_weights(context, nullptr);

  assert(oh_particle_adapter_make_byte_type(sizeof(CustomParticle),
                                            &particle_type) == MPI_SUCCESS);
  adapter.stride = sizeof(CustomParticle);
  adapter.mpi_type = particle_type;
  oh_particle_adapter_use_int_fields(&adapter,
                                     offsetof(CustomParticle, region),
                                     offsetof(CustomParticle, species));
  oh3_particle_adapter_use_position_fields(&adapter,
                                           offsetof(CustomParticle, x),
                                           offsetof(CustomParticle, y),
                                           offsetof(CustomParticle, z));
  oh3_particle_adapter_use_neighbor_position_fields(
    &adapter, offsetof(CustomParticle, x), offsetof(CustomParticle, y),
    offsetof(CustomParticle, z));
  oh3_particle_adapter_use_subdomain_position_fields(
    &adapter, offsetof(CustomParticle, x), offsetof(CustomParticle, y),
    offsetof(CustomParticle, z));
  adapter.get_region = custom_particle_get_region;
  adapter.set_region = custom_particle_set_region;
  adapter.get_species = custom_particle_get_species;
  adapter.map_to_neighbor = custom_particle_map_to_neighbor;
  adapter.map_to_subdomain = custom_particle_map_to_subdomain;
  oh_context_set_particle_adapter(context, &adapter);

  std::vector<CustomParticle> particles(4);
  int sdid[2] = {rank, -1};
  std::vector<int> nphgram(static_cast<std::size_t>(2 * nspec * nprocs), 0);
  std::vector<int> totalp(static_cast<std::size_t>(2 * nspec), 0);
  int pbase_storage[3] = {0, 0, 0};
  int *nphgram_ptr = nphgram.data();
  int *totalp_ptr = totalp.data();
  int *pbase_ptr = pbase_storage;

  oh_context_bind_region_ids(context, sdid, OH_PARTICLES_BORROWED);
  oh_context_bind_particles(context, particles.data(), 4, OH_PARTICLES_BORROWED);
  oh_context_bind_particle_accounting(context, &nphgram_ptr, &totalp_ptr,
                                      &pbase_ptr, OH_PARTICLES_BORROWED);
  assert(oh_context_max_local_particles_for_capacity(context, 100, 10, 1) > 0);
  assert(oh_context_transbound1(context, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_NORMAL_PRIMARY);
  assert(oh_context_transbound2(context, OH_MODE_NORMAL_PRIMARY, 0) ==
         OH_MODE_NORMAL_PRIMARY);
  assert(oh_weighted_transfer_count(10.0, load, 2.0, 8) == 2);

  oh_context_destroy(context);
  MPI_Type_free(&particle_type);
  MPI_Finalize();
  return 0;
}
