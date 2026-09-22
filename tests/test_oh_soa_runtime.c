/* Compare native SoA movement with the existing AoS path, including payloads
   unknown to OhHelp and primary/secondary weighted scheduling. */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oh_particle_storage.h"
#include "oh_particle_adapter.h"
#include "oh_particle_ownership.h"
#include "ohhelp1_internal.h"
#include "oh_context_internal.h"

#define NS 2
#define CAP 256
struct particle {
  long long region;
  int species;
  double x;
  long long id;
  unsigned char payload[3];
};
static const oh_particle_field fields[] = {
  {0, OH_PARTICLE_FIELD_LONG_LONG, OH_PARTICLE_ROLE_REGION, sizeof(long long)},
  {1, OH_PARTICLE_FIELD_INT, OH_PARTICLE_ROLE_SPECIES, sizeof(int)},
  {2, OH_PARTICLE_FIELD_DOUBLE, OH_PARTICLE_ROLE_X, sizeof(double)},
  {3, OH_PARTICLE_FIELD_LONG_LONG, OH_PARTICLE_ROLE_PAYLOAD, sizeof(long long)},
  {4, OH_PARTICLE_FIELD_BYTES, OH_PARTICLE_ROLE_PAYLOAD, 3}
};
struct fixture {
  oh_context *ctx;
  struct particle aos[CAP];
  long long region[CAP], id[CAP];
  int species[CAP];
  double x[CAP];
  unsigned char payload[CAP][3];
  int *hist, *total, *base;
  int mode;
};

static void setup(struct fixture *f, int soa, int owned, int rank, int n) {
  MPI_Datatype type;
  oh_particle_adapter adapter;
  void *bases[] = {f->region, f->species, f->x, f->id, f->payload};
  memset(f, 0, sizeof(*f));
  assert(oh_context_create(MPI_COMM_WORLD, &f->ctx) == MPI_SUCCESS);
  assert(oh_context_get_particle_storage(f->ctx) == OH_PARTICLE_STORAGE_AOS);
  oh_context_configure_particles(f->ctx, NS, 20);
  if (soa) {
    assert(oh_context_set_particle_storage(f->ctx, OH_PARTICLE_STORAGE_SOA) == MPI_SUCCESS);
    assert(oh_context_configure_soa(f->ctx, fields, 5, 1) == MPI_SUCCESS);
    assert(oh_context_bind_soa_particles(f->ctx, owned ? NULL : bases, owned ? 0 : 5, CAP,
      owned ? OH_PARTICLES_OWNED : OH_PARTICLES_BORROWED) == MPI_SUCCESS);
  } else {
    assert(oh_particle_adapter_make_byte_type(sizeof(struct particle), &type) == MPI_SUCCESS);
    adapter = oh_default_particle_adapter(type);
    adapter.stride = sizeof(struct particle);
    oh_particle_adapter_use_integer_fields(&adapter, offsetof(struct particle, region),
      sizeof(long long), offsetof(struct particle, species), sizeof(int));
    oh_particle_adapter_set_species_base(&adapter, 1);
    oh_context_set_particle_adapter(f->ctx, &adapter);
    MPI_Type_free(&type);
    oh_context_bind_particles(f->ctx, f->aos, CAP, OH_PARTICLES_BORROWED);
  }
  oh_context_bind_particle_accounting(f->ctx, &f->hist, &f->total, &f->base, OH_PARTICLES_OWNED);
  f->mode = OH_MODE_NORMAL_PRIMARY;
  (void)rank; (void)n;
}

static struct particle read_particle(struct fixture *f, int i) {
  struct particle p;
  if (oh_context_get_particle_storage(f->ctx) == OH_PARTICLE_STORAGE_AOS)
    return f->aos[i];
  p.region = ((long long*)oh_context_soa_field(f->ctx, 0))[i];
  p.species = ((int*)oh_context_soa_field(f->ctx, 1))[i];
  p.x = ((double*)oh_context_soa_field(f->ctx, 2))[i];
  p.id = ((long long*)oh_context_soa_field(f->ctx, 3))[i];
  memcpy(p.payload, (char*)oh_context_soa_field(f->ctx, 4) + 3*i, 3);
  return p;
}

static void write_particle(struct fixture *f, int i, struct particle p) {
  if (oh_context_get_particle_storage(f->ctx) == OH_PARTICLE_STORAGE_AOS) {
    f->aos[i] = p;
    return;
  }
  ((long long*)oh_context_soa_field(f->ctx, 0))[i] = p.region;
  ((int*)oh_context_soa_field(f->ctx, 1))[i] = p.species;
  ((double*)oh_context_soa_field(f->ctx, 2))[i] = p.x;
  ((long long*)oh_context_soa_field(f->ctx, 3))[i] = p.id;
  memcpy((char*)oh_context_soa_field(f->ctx, 4) + 3*i, p.payload, 3);
}

static struct particle make_particle(long long id, int region, int species) {
  struct particle p = {0};
  p.id = id; p.region = region; p.species = species;
  p.x = id + 0.25;
  for (int j=0; j<3; ++j) p.payload[j] = (unsigned char)(id + j*17);
  return p;
}

static int by_id(const void *a, const void *b) {
  long long x = ((const struct particle*)a)->id;
  long long y = ((const struct particle*)b)->id;
  return (x > y) - (x < y);
}

static void compare(struct fixture *a, struct fixture *b, int expected_global) {
  struct particle x[CAP], y[CAP];
  int ids_a[2], ids_b[2], global;
  assert(a->mode == b->mode);
  for (int i=0; i<3; ++i) assert(a->base[i] == b->base[i]);
  for (int i=0; i<2*NS; ++i) assert(a->total[i] == b->total[i]);
  oh_context_get_region_ids(a->ctx, ids_a);
  oh_context_get_region_ids(b->ctx, ids_b);
  assert(ids_a[0] == ids_b[0] && ids_a[1] == ids_b[1]);
  for (int i=0; i<a->base[2]; ++i) {
    x[i] = read_particle(a,i); y[i] = read_particle(b,i);
  }
  qsort(x, a->base[2], sizeof(*x), by_id);
  qsort(y, b->base[2], sizeof(*y), by_id);
  for (int i=0; i<a->base[2]; ++i) {
    assert(x[i].id == y[i].id && x[i].region == y[i].region);
    assert(x[i].species == y[i].species && x[i].x == y[i].x);
    assert(memcmp(x[i].payload, y[i].payload, 3) == 0);
    struct particle expected = make_particle(y[i].id, (int)y[i].region, y[i].species);
    assert(y[i].x == expected.x && !memcmp(y[i].payload, expected.payload, 3));
    if (i) assert(y[i-1].id != y[i].id);
  }
  MPI_Allreduce(&a->base[2], &global, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  assert(global == expected_global);
}

/* Rebuild accounting as applications do after advancing particle positions.
   Existing species/channel order is retained until transbound compacts it. */
static void recount(struct fixture *f, int n, int destination) {
  memset(f->hist, 0, (size_t)2*NS*n*sizeof(int));
  for (int i=0; i<f->base[2]; ++i) {
    struct particle p = read_particle(f,i);
    if (destination >= 0) p.region = destination;
    write_particle(f,i,p);
    if (p.region >= 0)
      ++f->hist[((i < f->base[1] ? 0 : NS) + p.species-1)*n+p.region];
  }
}

static void exchange(struct fixture *a, struct fixture *b, int total) {
  a->mode = oh_context_transbound2(a->ctx, a->mode, 0);
  b->mode = oh_context_transbound2(b->ctx, b->mode, 0);
  compare(a,b,total);
}

static void run_case(int rank, int n, int owned) {
  struct fixture a, b;
  oh_particle_ref ref;
  struct particle *acopy;
  setup(&a,0,0,rank,n); setup(&b,1,owned,rank,n);
  /* One initially empty rank exercises zero-send/zero-receive paths. */
  int initial = (n > 1 && rank == n-1) ? 0 : 8;
  int expected = (n > 1 ? n-1 : 1)*8;
  for (int i=0; i<initial; ++i) {
    struct particle p = make_particle(rank*100+i, (rank+1)%n, i/4+1);
    write_particle(&a,i,p); write_particle(&b,i,p);
    ++a.hist[(p.species-1)*n+p.region];
    ++b.hist[(p.species-1)*n+p.region];
  }
  oh_context_set_total_particles(a.ctx); oh_context_set_total_particles(b.ctx);
  exchange(&a,&b,expected);
  double *weights = calloc((size_t)n, sizeof(double));
  assert(weights);
  for (int r=0; r<n; ++r) weights[r] = r == 0 ? 3.0 : 1.0;
  oh_context_set_region_weights(a.ctx,weights); oh_context_set_region_weights(b.ctx,weights);
  free(weights);
  recount(&a,n,0); recount(&b,n,0);
  exchange(&a,&b,expected);
  if (n>1) assert(b.mode == OH_MODE_REBALANCE_SECONDARY);
  recount(&a,n,-1); recount(&b,n,-1);
  exchange(&a,&b,expected); /* stable secondary */
  assert(b.mode == (n>1 ? OH_MODE_NORMAL_SECONDARY : OH_MODE_NORMAL_PRIMARY));
  recount(&a,n,-1); recount(&b,n,-1);
  struct particle injected = make_particle(10000+rank, 0, 1);
  const void *values[] = {&injected.region,&injected.species,&injected.x,&injected.id,injected.payload};
  acopy = oh_context_inject_particle_get(a.ctx,&injected);
  assert(oh_context_inject_soa_particle(b.ctx,values,5,&ref) == MPI_SUCCESS);
  assert(ref.index == b.base[2]);
  /* Remove old accounting before changing destination and adding it again. */
  oh_context_remove_injected_particle(a.ctx,acopy);
  assert(oh_context_remove_injected_particle_ref(b.ctx,ref) == MPI_SUCCESS);
  acopy->region = (rank+1)%n;
  ((long long*)oh_context_soa_field(b.ctx,0))[ref.index] = acopy->region;
  oh_context_remap_injected_particle(a.ctx,acopy);
  assert(oh_context_remap_injected_particle_ref(b.ctx,ref) == MPI_SUCCESS);
  assert(oh_context_remove_injected_particle_ref(a.ctx,ref) == MPI_ERR_ARG);
  expected += n;
  exchange(&a,&b,expected);
  assert(oh_context_remove_injected_particle_ref(b.ctx,ref) == MPI_ERR_ARG);
  /* Finalizing pending injections must compact all columns consistently. */
  recount(&a,n,-1); recount(&b,n,-1);
  injected = make_particle(20000+rank,rank,2);
  acopy = oh_context_inject_particle_get(a.ctx,&injected);
  assert(oh_context_inject_soa_particle(b.ctx,values,5,&ref) == MPI_SUCCESS);
  (void)acopy;
  oh_context_set_total_particles(a.ctx); oh_context_set_total_particles(b.ctx);
  assert(oh_context_remap_injected_particle_ref(b.ctx,ref) == MPI_ERR_ARG);
  expected += n;
  exchange(&a,&b,expected);
  /* Force another secondary assignment, then an all-empty exchange. */
  recount(&a,n,n-1); recount(&b,n,n-1); exchange(&a,&b,expected);
  for (int i=0;i<a.base[2];++i) { a.aos[i].region=-1; ((long long*)oh_context_soa_field(b.ctx,0))[i]=-1; }
  recount(&a,n,-1); recount(&b,n,-1); exchange(&a,&b,0);
  oh_context_unbind_particles(b.ctx);
  assert(oh_context_soa_field(b.ctx,0) == NULL);
  assert(oh_context_get_particle_storage(b.ctx) == OH_PARTICLE_STORAGE_SOA);
  assert(oh_context_bind_soa_particles(b.ctx,NULL,0,0,OH_PARTICLES_BORROWED) == MPI_SUCCESS);
  oh_context_set_total_particles(b.ctx);
  b.mode = oh_context_transbound2(b.ctx,b.mode,0);
  assert(b.base[2] == 0);
  oh_context_destroy(a.ctx); oh_context_destroy(b.ctx);
}

int main(int argc, char **argv) {
  int rank,n;
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank); MPI_Comm_size(MPI_COMM_WORLD,&n);
  run_case(rank,n,0); run_case(rank,n,1);
  if (!rank) puts("AoS/SoA Level-2 equivalence passed");
  MPI_Finalize();
  return 0;
}
