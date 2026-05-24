#include <assert.h>

#include <mpi.h>

#include "oh_particle_adapter.h"
#include "oh_part.h"

int
main(int argc, char **argv) {
  MPI_Datatype particle_type;
  MPI_Datatype padded_type;
  MPI_Datatype wide_type;
  oh_particle_adapter adapter;
  struct S_particle particle;
  struct wide_particle {
    double x, y, z;
    long long region;
    long species;
  } wide_particle;

  MPI_Init(&argc, &argv);
  assert(oh_particle_adapter_make_byte_type(0, &particle_type) != MPI_SUCCESS);
  assert(particle_type == MPI_DATATYPE_NULL);
  assert(oh_particle_adapter_make_byte_type(sizeof(struct S_particle),
                                            &particle_type) == MPI_SUCCESS);

  adapter = oh_default_particle_adapter(particle_type);
  assert(oh_particle_adapter_validate(&adapter));
  assert(adapter.stride == sizeof(struct S_particle));
  assert(adapter.position_offset[0] == offsetof(struct S_particle, x));
  assert(adapter.position_offset[1] == offsetof(struct S_particle, y));
  assert(adapter.position_offset[2] == offsetof(struct S_particle, z));
  assert(adapter.species_base == 0);
  assert(oh_particle_adapter_position(&adapter, &particle, 0) == &particle.x);
  assert(oh_particle_adapter_position(&adapter, &particle, 1) == &particle.y);
  assert(oh_particle_adapter_position(&adapter, &particle, 2) == &particle.z);
  assert(oh_particle_adapter_position(&adapter, &particle, -1) == 0);
  assert(oh_particle_adapter_position(&adapter, &particle, 3) == 0);
  assert(oh_particle_adapter_const_position(&adapter, &particle, 0) ==
         &particle.x);
  oh_particle_adapter_use_position_fields(&adapter,
                                          offsetof(struct S_particle, z),
                                          offsetof(struct S_particle, x),
                                          offsetof(struct S_particle, y));
  assert(oh_particle_adapter_position(&adapter, &particle, 0) == &particle.z);
  assert(oh_particle_adapter_position(&adapter, &particle, 1) == &particle.x);
  assert(oh_particle_adapter_position(&adapter, &particle, 2) == &particle.y);

  particle.nid = 7;
  particle.spec = 3;
  assert(adapter.get_region(&adapter, &particle, 0) == 7);
  assert(adapter.get_species(&adapter, &particle) == 3);
  adapter.set_region(&adapter, &particle, 5, 0);
  assert(particle.nid == 5);
  assert(sizeof(oh_particle_region_t) >= sizeof(particle.nid));

  assert(oh_particle_adapter_make_byte_type(sizeof(struct S_particle)+8,
                                            &padded_type) == MPI_SUCCESS);
  adapter.mpi_type = padded_type;
  assert(!oh_particle_adapter_validate(&adapter));
  adapter.stride = sizeof(struct S_particle)+8;
  assert(oh_particle_adapter_validate(&adapter));

  assert(oh_particle_adapter_make_byte_type(sizeof(wide_particle),
                                            &wide_type) == MPI_SUCCESS);
  adapter = oh_default_particle_adapter(wide_type);
  adapter.stride = sizeof(wide_particle);
  oh_particle_adapter_use_integer_fields(
    &adapter, offsetof(struct wide_particle, region),
    sizeof(wide_particle.region), offsetof(struct wide_particle, species),
    sizeof(wide_particle.species));
  wide_particle.region = 4000000000LL;
  wide_particle.species = 2;
  assert(oh_particle_adapter_validate(&adapter));
  assert(adapter.get_region(&adapter, &wide_particle, 0) == 4000000000LL);
  assert(adapter.get_species(&adapter, &wide_particle) == 2);
  oh_particle_adapter_set_species_base(&adapter, 1);
  assert(adapter.species_base == 1);
  adapter.set_region(&adapter, &wide_particle, 5000000000LL, 0);
  assert(wide_particle.region == 5000000000LL);

  oh_particle_adapter_use_integer_fields(
    &adapter, offsetof(struct wide_particle, region), 3,
    offsetof(struct wide_particle, species), sizeof(wide_particle.species));
  assert(!oh_particle_adapter_validate(&adapter));

  MPI_Type_free(&wide_type);
  MPI_Type_free(&padded_type);
  MPI_Type_free(&particle_type);
  MPI_Finalize();
  return 0;
}
