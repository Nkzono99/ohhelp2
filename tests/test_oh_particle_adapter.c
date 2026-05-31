#include <assert.h>
#include <limits.h>
#include <string.h>

#include <mpi.h>

#include "oh_particle_adapter.h"
#include "oh_part.h"

int
main(int argc, char **argv) {
  MPI_Datatype particle_type;
  MPI_Datatype padded_type;
  MPI_Datatype wide_type;
  oh_particle_adapter adapter;
  oh_particle_adapter default_adapter;
  struct S_particle particle;
  struct wide_particle {
    double x, y, z;
    long long region;
    long species;
  } wide_particle;

  if (argc > 1 && strcmp(argv[1], "before-init") == 0) {
    adapter = oh_default_particle_adapter(MPI_BYTE);
    adapter.stride = 1;
    assert(!oh_particle_adapter_validate(&adapter));
    assert(oh_particle_adapter_make_byte_type(sizeof(struct S_particle),
                                              &particle_type) != MPI_SUCCESS);
    assert(particle_type == MPI_DATATYPE_NULL);
    return 0;
  }

  MPI_Init(&argc, &argv);
  assert(oh_particle_adapter_make_byte_type(sizeof(struct S_particle),
                                            0) == MPI_ERR_ARG);
  assert(oh_particle_adapter_make_byte_type((size_t)INT_MAX + 1,
                                            &particle_type) != MPI_SUCCESS);
  assert(particle_type == MPI_DATATYPE_NULL);
  assert(oh_particle_adapter_make_byte_type(0, &particle_type) != MPI_SUCCESS);
  assert(particle_type == MPI_DATATYPE_NULL);
  assert(oh_particle_adapter_make_byte_type(sizeof(struct S_particle),
                                            &particle_type) == MPI_SUCCESS);

  adapter = oh_default_particle_adapter(particle_type);
  assert(oh_particle_adapter_validate(&adapter));
  {
    oh_particle_adapter invalid = adapter;

    invalid.get_region = 0;
    assert(!oh_particle_adapter_validate(&invalid));
    invalid = adapter;
    invalid.set_region = 0;
    assert(!oh_particle_adapter_validate(&invalid));
    invalid = adapter;
    invalid.get_species = 0;
    assert(!oh_particle_adapter_validate(&invalid));
  }
  assert(adapter.stride == sizeof(struct S_particle));
  assert(adapter.position_offset[0] == offsetof(struct S_particle, x));
  assert(adapter.position_offset[1] == offsetof(struct S_particle, y));
  assert(adapter.position_offset[2] == offsetof(struct S_particle, z));
  assert(adapter.species_base == 0);
  assert(oh_particle_adapter_region_access(&adapter) ==
         OH_PARTICLE_ADAPTER_ACCESS_CALLBACK);
  assert(oh_particle_adapter_species_access(&adapter) ==
         OH_PARTICLE_ADAPTER_ACCESS_CALLBACK);
  assert(oh_particle_adapter_map_to_subdomain_access(&adapter) ==
         OH_PARTICLE_ADAPTER_MAP_CALLBACK);
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
  assert(oh_particle_adapter_validate(&adapter));
  oh_particle_adapter_use_position_fields(&adapter, (size_t)-1,
                                          offsetof(struct S_particle, x),
                                          offsetof(struct S_particle, y));
  assert(!oh_particle_adapter_validate(&adapter));
  oh_particle_adapter_use_position_fields(&adapter,
                                          offsetof(struct S_particle, z),
                                          sizeof(struct S_particle),
                                          offsetof(struct S_particle, y));
  assert(!oh_particle_adapter_validate(&adapter));
  assert(oh_particle_adapter_position(&adapter, &particle, 1) == 0);
  assert(oh_particle_adapter_const_position(&adapter, &particle, 1) == 0);
  oh_particle_adapter_use_position_fields(&adapter,
                                          offsetof(struct S_particle, z),
                                          offsetof(struct S_particle, x),
                                          offsetof(struct S_particle, y));

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
  default_adapter = adapter;
  adapter.stride = sizeof(wide_particle);
  oh_particle_adapter_use_integer_fields(
    &adapter, offsetof(struct wide_particle, region),
    sizeof(wide_particle.region), offsetof(struct wide_particle, species),
    sizeof(wide_particle.species));
  wide_particle.region = 4000000000LL;
  wide_particle.species = 2;
  assert(oh_particle_adapter_validate(&adapter));
  assert(oh_particle_adapter_region_access(&adapter) ==
         OH_PARTICLE_ADAPTER_ACCESS_INTEGER_FIELD);
  assert(oh_particle_adapter_species_access(&adapter) ==
         OH_PARTICLE_ADAPTER_ACCESS_INTEGER_FIELD);
  assert(oh_particle_adapter_map_to_subdomain_access(&adapter) ==
         OH_PARTICLE_ADAPTER_MAP_REGION_FIELD);
  assert(adapter.get_region(&adapter, &wide_particle, 0) == 4000000000LL);
  assert(adapter.get_species(&adapter, &wide_particle) == 2);
  assert(oh_particle_adapter_read_region_field(&adapter, &wide_particle) ==
         4000000000LL);
  assert(oh_particle_adapter_read_species_field(&adapter, &wide_particle) ==
         2);
  oh_particle_adapter_set_species_base(&adapter, 1);
  assert(adapter.species_base == 1);
  adapter.set_region(&adapter, &wide_particle, 5000000000LL, 0);
  assert(wide_particle.region == 5000000000LL);
  adapter.set_region(&adapter, &wide_particle, -1, 0);
  assert(wide_particle.region == -1);
  assert(adapter.get_region(&adapter, &wide_particle, 0) == -1);
  adapter.set_region(&adapter, &wide_particle, -2, 0);
  assert(wide_particle.region == -2);
  assert(adapter.get_region(&adapter, &wide_particle, 0) == -2);

  oh_particle_adapter_use_single_species_integer_region(
    &adapter, offsetof(struct wide_particle, region),
    sizeof(wide_particle.region));
  assert(oh_particle_adapter_validate(&adapter));
  assert(oh_particle_adapter_region_access(&adapter) ==
         OH_PARTICLE_ADAPTER_ACCESS_INTEGER_FIELD);
  assert(oh_particle_adapter_species_access(&adapter) ==
         OH_PARTICLE_ADAPTER_ACCESS_SINGLE_SPECIES);
  assert(adapter.get_species(&adapter, &wide_particle) == 0);
  adapter.set_region(&adapter, &wide_particle, 6000000000LL, 0);
  assert(wide_particle.region == 6000000000LL);
  oh_particle_adapter_write_region_field(&adapter, &wide_particle,
                                         7000000000LL);
  assert(wide_particle.region == 7000000000LL);

  oh_particle_adapter_use_integer_fields(
    &adapter, offsetof(struct wide_particle, region), 3,
    offsetof(struct wide_particle, species), sizeof(wide_particle.species));
  assert(!oh_particle_adapter_validate(&adapter));
  adapter.get_region = default_adapter.get_region;
  adapter.set_region = default_adapter.set_region;
  adapter.get_species = default_adapter.get_species;
  assert(!oh_particle_adapter_validate(&adapter));
  oh_particle_adapter_use_integer_fields(
    &adapter, sizeof(wide_particle), sizeof(wide_particle.region),
    offsetof(struct wide_particle, species), sizeof(wide_particle.species));
  assert(!oh_particle_adapter_validate(&adapter));
  oh_particle_adapter_use_integer_fields(
    &adapter, offsetof(struct wide_particle, region),
    sizeof(wide_particle.region), sizeof(wide_particle),
    sizeof(wide_particle.species));
  assert(!oh_particle_adapter_validate(&adapter));
  oh_particle_adapter_use_integer_fields(
    &adapter, (size_t)-1, sizeof(wide_particle.region),
    offsetof(struct wide_particle, species), sizeof(wide_particle.species));
  assert(!oh_particle_adapter_validate(&adapter));

  if (argc > 1 && strcmp(argv[1], "after-finalize") == 0) {
    oh_particle_adapter live_adapter = oh_default_particle_adapter(particle_type);

    assert(oh_particle_adapter_validate(&live_adapter));
    MPI_Type_free(&wide_type);
    MPI_Type_free(&padded_type);
    MPI_Finalize();
    assert(!oh_particle_adapter_validate(&live_adapter));
    assert(oh_particle_adapter_make_byte_type(sizeof(struct S_particle),
                                              &wide_type) != MPI_SUCCESS);
    assert(wide_type == MPI_DATATYPE_NULL);
    return 0;
  }

  MPI_Type_free(&wide_type);
  MPI_Type_free(&padded_type);
  MPI_Type_free(&particle_type);
  MPI_Finalize();
  return 0;
}
