/* Native column storage and transport helpers for Levels 2-4. */
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ohhelp1_internal.h"
#include "oh_context_internal.h"
#include "oh_particle_buffer.h"
#include "ohhelp2_internal.h"

static void storage_error(struct oh_state *state, const char *message) {
  fprintf(stderr, "OhHelp particle storage: %s\n", message);
  MPI_Abort(state->comm, 1);
  abort();
}

static int valid_range(const struct oh_soa_storage *s, int first, int count) {
  return first >= 0 && count >= 0 && first <= s->capacity &&
         count <= s->capacity - first;
}

static void require_range(const struct oh_soa_storage *s, int first, int count) {
  if (!valid_range(s, first, count)) {
    fprintf(stderr, "OhHelp SoA index/count exceeds particle capacity\n");
    MPI_Abort(s->comm, 1);
    abort();
  }
}

static void *field_at(struct oh_soa_storage *s, int field, int index) {
  return (char*)s->bases[field] + (size_t)index * s->fields[field].size;
}

static long long read_integer(const void *value, int type) {
  if (type == OH_PARTICLE_FIELD_INT) {
    int v;
    memcpy(&v, value, sizeof(v));
    return v;
  } else {
    long long v;
    memcpy(&v, value, sizeof(v));
    return v;
  }
}

void oh_storage_require_aos(struct oh_state *state, const char *api) {
  if (state->particle_storage != OH_PARTICLE_STORAGE_AOS) {
    fprintf(stderr, "%s requires AoS particle storage\n", api);
    MPI_Abort(state->comm, 1);
    abort();
  }
}

void oh_storage_advance_generation(struct oh_state *state) {
  if (state->particle_generation == LLONG_MAX)
    storage_error(state, "particle reference generation exhausted");
  ++state->particle_generation;
}

int oh_context_set_particle_storage(oh_context *ctx, int backend) {
  if (!ctx || ctx->particle_buffer_bound || (ctx->soa && ctx->soa->level4) ||
      (backend != OH_PARTICLE_STORAGE_AOS && backend != OH_PARTICLE_STORAGE_SOA))
    return MPI_ERR_ARG;
  ctx->particle_storage = backend;
  ctx->storage_schema_dirty = 1;
  return MPI_SUCCESS;
}

int oh_context_get_particle_storage(const oh_context *ctx) {
  return ctx ? ctx->particle_storage : OH_PARTICLE_STORAGE_AOS;
}

int oh_context_configure_soa(oh_context *ctx, const oh_particle_field *fields,
                            int nfields, int species_base) {
  struct oh_soa_storage *s;
  size_t size = 0;
  int i, j, roles = 0;
  if (!ctx || ctx->particle_buffer_bound || (ctx->soa && ctx->soa->level4) ||
      ctx->particle_storage != OH_PARTICLE_STORAGE_SOA || !fields ||
      nfields <= 0 || nfields > INT_MAX / 4 || (ctx->n_of_species <= 0 && !oh_context_is_default_state(ctx)))
    return MPI_ERR_ARG;
  for (i = 0; i < nfields; ++i) {
    const oh_particle_field *f = fields + i;
    if (f->key < 0 || f->size == 0 || f->size > INT_MAX ||
        size > (size_t)INT_MAX - f->size || f->role < 0 || f->role > 16 ||
        (f->role && (f->role & (f->role - 1))) || (roles & f->role))
      return MPI_ERR_ARG;
    for (j = 0; j < i; ++j)
      if (fields[j].key == f->key) return MPI_ERR_ARG;
    if (!((f->type == OH_PARTICLE_FIELD_INT && f->size == sizeof(int)) ||
          (f->type == OH_PARTICLE_FIELD_LONG_LONG && f->size == sizeof(long long)) ||
          (f->type == OH_PARTICLE_FIELD_DOUBLE && f->size == sizeof(double)) ||
          f->type == OH_PARTICLE_FIELD_BYTES))
      return MPI_ERR_TYPE;
    if ((f->role == OH_PARTICLE_ROLE_REGION ||
         f->role == OH_PARTICLE_ROLE_SPECIES) &&
        f->type != OH_PARTICLE_FIELD_INT && f->type != OH_PARTICLE_FIELD_LONG_LONG)
      return MPI_ERR_TYPE;
    if (f->role >= OH_PARTICLE_ROLE_X && f->type != OH_PARTICLE_FIELD_DOUBLE)
      return MPI_ERR_TYPE;
    roles |= f->role;
    size += f->size;
  }
  if (!(roles & OH_PARTICLE_ROLE_REGION) ||
      (!(roles & OH_PARTICLE_ROLE_SPECIES) && ctx->n_of_species > 1))
    return MPI_ERR_ARG;
  s = calloc(1, sizeof(*s));
  if (!s) return MPI_ERR_NO_MEM;
  s->comm = ctx->comm;
  s->wire_type = MPI_DATATYPE_NULL;
  s->cursor_type = MPI_DATATYPE_NULL;
  s->saved_aos_type = MPI_DATATYPE_NULL;
  s->fields = calloc((size_t)nfields, sizeof(*s->fields));
  s->offsets = calloc((size_t)nfields, sizeof(*s->offsets));
  if (!s->fields || !s->offsets) {
    oh_soa_destroy(s);
    return MPI_ERR_NO_MEM;
  }
  s->nfields = nfields;
  s->species_base = species_base;
  s->species_field = -1;
  s->record_size = size;
  size = 0;
  for (i = 0; i < nfields; ++i) {
    s->fields[i] = fields[i];
    s->offsets[i] = size;
    size += fields[i].size;
    if (fields[i].role == OH_PARTICLE_ROLE_REGION) s->region_field = i;
    if (fields[i].role == OH_PARTICLE_ROLE_SPECIES) s->species_field = i;
  }
  oh_soa_destroy(ctx->soa);
  ctx->soa = s;
  ctx->storage_schema_dirty = 1;
  return MPI_SUCCESS;
}

void oh_soa_unbind(struct oh_soa_storage *s) {
  int initialized = 0, finalized = 0;
  if (!s) return;
  oh_soa_level4_release(s);
  if (s->bases && s->ownership == OH_PARTICLES_OWNED)
    for (int i = 0; i < s->nfields; ++i) free(s->bases[i]);
  free(s->bases);
  free(s->receive_buffer);
  free(s->receive_starts);
  s->bases = NULL;
  s->receive_buffer = NULL;
  s->receive_starts = NULL;
  s->capacity = 0;
  if (s->wire_type != MPI_DATATYPE_NULL) {
    MPI_Initialized(&initialized);
    if (initialized) MPI_Finalized(&finalized);
    if (initialized && !finalized) MPI_Type_free(&s->wire_type);
    s->wire_type = MPI_DATATYPE_NULL;
  }
}

void oh_soa_destroy(struct oh_soa_storage *s) {
  if (!s) return;
  oh_soa_unbind(s);
  if (s->saved_aos_type != MPI_DATATYPE_NULL) {
    int initialized=0, finalized=0;
    MPI_Initialized(&initialized);
    if (initialized) MPI_Finalized(&finalized);
    if (initialized && !finalized) MPI_Type_free(&s->saved_aos_type);
  }
  free(s->fields);
  free(s->offsets);
  free(s);
}

int oh_soa_bind(struct oh_soa_storage *s, void *const *bases,
               int capacity, int ownership) {
  int i, j, err;
  size_t slots = capacity > 0 ? (size_t)capacity : 1;
  if (!s || s->bases || capacity < 0 ||
      (ownership != OH_PARTICLES_BORROWED && ownership != OH_PARTICLES_OWNED) ||
      (ownership == OH_PARTICLES_OWNED && bases) ||
      (ownership == OH_PARTICLES_BORROWED && capacity > 0 && !bases) ||
      slots > (size_t)PTRDIFF_MAX / s->record_size)
    return MPI_ERR_ARG;
  if (ownership == OH_PARTICLES_BORROWED && capacity > 0) {
    for (i = 0; i < s->nfields; ++i) {
      uintptr_t begin = (uintptr_t)bases[i];
      size_t span = (size_t)capacity * s->fields[i].size;
      if (!begin || span > UINTPTR_MAX - begin) return MPI_ERR_ARG;
      for (j = 0; j < i; ++j) {
        uintptr_t other = (uintptr_t)bases[j];
        size_t other_span = (size_t)capacity * s->fields[j].size;
        if (begin < other + other_span && other < begin + span)
          return MPI_ERR_ARG;
      }
    }
  }
  s->ownership = ownership;
  s->capacity = capacity;
  s->bases = calloc((size_t)s->nfields, sizeof(*s->bases));
  if (!s->level4) s->receive_buffer = malloc(slots * s->record_size);
  if (!s->bases || (!s->level4 && !s->receive_buffer)) {
    oh_soa_unbind(s);
    return MPI_ERR_NO_MEM;
  }
  for (i = 0; i < s->nfields; ++i) {
    if (ownership == OH_PARTICLES_OWNED) {
      s->bases[i] = calloc(slots, s->fields[i].size);
      if (!s->bases[i]) {
        oh_soa_unbind(s);
        return MPI_ERR_NO_MEM;
      }
    } else s->bases[i] = bases ? bases[i] : NULL;
  }
  err = oh_particle_adapter_make_byte_type(s->record_size, &s->wire_type);
  if (err != MPI_SUCCESS) oh_soa_unbind(s);
  return err;
}

void *oh_context_soa_field(oh_context *ctx, int key) {
  if (!ctx || ctx->particle_storage != OH_PARTICLE_STORAGE_SOA ||
      !ctx->particle_buffer_bound || !ctx->soa) return NULL;
  for (int i = 0; i < ctx->soa->nfields; ++i)
    if (ctx->soa->fields[i].key == key) return ctx->soa->bases[i];
  return NULL;
}

long long oh_soa_region(struct oh_soa_storage *s, int index) {
  require_range(s, index, 1);
  return read_integer(field_at(s, s->region_field, index),
                      s->fields[s->region_field].type);
}

int oh_soa_species(struct oh_state *state, int index) {
  struct oh_soa_storage *s = state->soa;
  long long raw;
  require_range(s, index, 1);
  if (s->species_field < 0) return 0;
  raw = read_integer(field_at(s, s->species_field, index),
                     s->fields[s->species_field].type);
  if (raw < s->species_base || raw >= (long long)s->species_base + state->n_of_species)
    storage_error(state, "SoA particle species outside configured range");
  return (int)(raw - s->species_base);
}

void oh_soa_set_region(struct oh_soa_storage *s, int index, long long region) {
  require_range(s, index, 1);
  if (s->fields[s->region_field].type == OH_PARTICLE_FIELD_INT) {
    int v = (int)region; /* Internal callers write only validated ids or -1. */
    memcpy(field_at(s, s->region_field, index), &v, sizeof(v));
  } else memcpy(field_at(s, s->region_field, index), &region, sizeof(region));
}

void oh_soa_copy(struct oh_soa_storage *s, int dst, int src, int count) {
  require_range(s, dst, count);
  require_range(s, src, count);
  if (!count) return;
  for (int f = 0; f < s->nfields; ++f)
    memmove(field_at(s, f, dst), field_at(s, f, src),
            (size_t)count * s->fields[f].size);
}

void oh_soa_pack(struct oh_soa_storage *s, void *records, int src, int count) {
  require_range(s, src, count);
  for (int f = 0; f < s->nfields; ++f)
    for (int i = 0; i < count; ++i)
      memcpy((char*)records + (size_t)i * s->record_size + s->offsets[f],
             field_at(s, f, src + i), s->fields[f].size);
}

void oh_soa_unpack(struct oh_soa_storage *s, int dst, const void *records, int count) {
  require_range(s, dst, count);
  for (int f = 0; f < s->nfields; ++f)
    for (int i = 0; i < count; ++i)
      memcpy(field_at(s, f, dst + i),
             (const char*)records + (size_t)i * s->record_size + s->offsets[f],
             s->fields[f].size);
}

void oh_soa_gather(struct oh_soa_storage *s, const int *indices, int count) {
  size_t largest = 0;
  void *scratch;
  require_range(s, 0, count);
  if (!count) return;
  for (int i = 0; i < count; ++i) require_range(s, indices[i], 1);
  for (int f = 0; f < s->nfields; ++f)
    if (s->fields[f].size > largest) largest = s->fields[f].size;
  scratch = malloc((size_t)count * largest);
  if (!scratch) {
    fprintf(stderr, "out of memory for SoA gather scratch\n");
    MPI_Abort(s->comm, 1);
    abort();
  }
  for (int f = 0; f < s->nfields; ++f) {
    size_t size = s->fields[f].size;
    for (int i = 0; i < count; ++i)
      memcpy((char*)scratch + (size_t)i * size, field_at(s, f, indices[i]), size);
    memcpy(s->bases[f], scratch, (size_t)count * size);
  }
  free(scratch);
}

void oh_soa_check_capacity(struct oh_state *state, long long count) {
  if (state->particle_storage == OH_PARTICLE_STORAGE_SOA &&
      (count < 0 || count > state->n_of_local_particles_limit))
    storage_error(state, "SoA transfer exceeds particle capacity");
}

void *oh_storage_receive_at(struct oh_state *state, int index) {
  oh_soa_check_capacity(state, index);
  if (state->particle_storage == OH_PARTICLE_STORAGE_SOA && !oh_soa_is_level4(state))
    return (char*)state->soa->receive_buffer + (size_t)index * state->soa->record_size;
  return oh_particle_buffer_at(state->particle_adapter, state->particles, index);
}

int oh_storage_receive_index(struct oh_state *state, void *records, int count) {
  struct oh_soa_storage *s;
  size_t offset;
  if (state->particle_storage != OH_PARTICLE_STORAGE_SOA || oh_soa_is_level4(state)) return 0;
  s = state->soa;
  offset = (size_t)((char*)records - (char*)s->receive_buffer);
  if (offset % s->record_size || offset / s->record_size > (size_t)s->capacity)
    storage_error(state, "invalid SoA receive cursor");
  require_range(s, (int)(offset / s->record_size), count);
  return (int)(offset / s->record_size);
}

void oh_storage_unpack_received(struct oh_state *state, void *records, int count) {
  if (state->particle_storage == OH_PARTICLE_STORAGE_SOA && !oh_soa_is_level4(state))
    oh_soa_unpack(state->soa, oh_storage_receive_index(state, records, count),
                   records, count);
}

/* All ranks vote on revalidation, even when only one rank was reconfigured.
   One small reduction is the steady-state cost for both backends. */
void oh_storage_preflight(struct oh_state *state, int level) {
  int local[8] = {state->particle_storage, -state->particle_storage,
                  state->n_of_species, -state->n_of_species, 0,
                  state->storage_schema_dirty, level, -level};
  int shared[8];
  struct oh_soa_storage *s = state->soa;
  if (state->particle_storage == OH_PARTICLE_STORAGE_SOA)
    local[4] = !s || !state->particle_buffer_bound ||
               (level == 3 && !state->grid) ||
               (level >= 4 && !s->level4) || (level < 4 && s->level4) ||
               (oh_context_is_default_state(state) && !s->level4) ||
               !state->particle_accounting_bound || !state->particle_base_bound;
  if (local[0] == OH_PARTICLE_STORAGE_SOA && s && level>=3) {
    int roles=0;
    for (int f=0; f<s->nfields; ++f) roles|=s->fields[f].role;
    for (int d=0; d<OH_DIMENSION; ++d)
      if (!(roles&(OH_PARTICLE_ROLE_X<<d))) local[4]=1;
  }
  MPI_Allreduce(local, shared, 8, MPI_INT, MPI_MAX, state->comm);
  if (shared[0] != -shared[1] || shared[2] != -shared[3] || shared[6] != -shared[7] || shared[4])
    storage_error(state, "backend/species mismatch or unsupported SoA level/binding");
  if (local[0] == OH_PARTICLE_STORAGE_AOS) {
    state->storage_schema_dirty = 0;
    return;
  }
  if (shared[5]) {
    int config[2] = {s->nfields, s->species_base}, min_config[2], max_config[2];
    MPI_Allreduce(config, min_config, 2, MPI_INT, MPI_MIN, state->comm);
    MPI_Allreduce(config, max_config, 2, MPI_INT, MPI_MAX, state->comm);
    if (min_config[0] != max_config[0] || min_config[1] != max_config[1])
      storage_error(state, "SoA schema mismatch");
    /* Exact descriptors, not a hash. Local capacities/addresses may differ. */
    for (int i = 0; i < s->nfields; ++i) {
      long long v[4] = {s->fields[i].key, s->fields[i].type,
                       s->fields[i].role, (long long)s->fields[i].size};
      long long min[4], max[4];
      MPI_Allreduce(v, min, 4, MPI_LONG_LONG_INT, MPI_MIN, state->comm);
      MPI_Allreduce(v, max, 4, MPI_LONG_LONG_INT, MPI_MAX, state->comm);
      for (int j = 0; j < 4; ++j)
        if (min[j] != max[j]) storage_error(state, "SoA schema mismatch");
    }
    state->storage_schema_dirty = 0;
  }
  oh_soa_check_capacity(state, (long long)state->total_parts + state->n_of_injections);
  oh_storage_advance_generation(state);
}

/* Native columns behind the legacy Level 4 cursor arithmetic. A cursor contains
   only a bank and an index: it is never copied or sent as particle payload. */
struct oh_soa_cursor { struct oh_soa_bank *bank; int index; };
struct oh_soa_bank {
  struct oh_soa_storage *schema;
  int capacity, owned;
  void **bases;
  struct oh_soa_cursor *slots;
};

int oh_soa_is_level4(const struct oh_state *state) {
  return state->particle_storage == OH_PARTICLE_STORAGE_SOA &&
         state->soa && state->soa->level4;
}

static struct oh_soa_cursor *checked_cursor(struct oh_state *state,
                                           const void *ptr, int count) {
  struct oh_soa_storage *s = state->soa;
  uintptr_t address = (uintptr_t)ptr;
  for (int b=0; b<3; ++b) {
    struct oh_soa_bank *bank=s->banks[b];
    if (bank) {
      uintptr_t start=(uintptr_t)bank->slots;
      size_t bytes=(size_t)bank->capacity*sizeof(*bank->slots);
      if (address>=start && address-start<=bytes &&
          (address-start)%sizeof(*bank->slots)==0) {
        int index=(int)((address-start)/sizeof(*bank->slots));
        if (count>=0 && count<=bank->capacity-index) return bank->slots+index;
      }
    }
  }
  storage_error(state,"Level 4 SoA cursor/count is outside its column bank");
  return NULL;
}

static void *cursor_field(struct oh_soa_cursor *p, int f) {
  return p->bank->bases[f] ? (char*)p->bank->bases[f] +
         (size_t)p->index*p->bank->schema->fields[f].size : NULL;
}

static long long cursor_region(const oh_particle_adapter *a, const void *p, int ps) {
  struct oh_state *state=a->user_data;
  struct oh_soa_cursor *c=checked_cursor(state,p,1);
  int f=state->soa->region_field;
  (void)ps;
  return read_integer(cursor_field(c,f),state->soa->fields[f].type);
}
static void cursor_set_region(const oh_particle_adapter *a, void *p, long long v, int ps) {
  struct oh_state *state=a->user_data;
  struct oh_soa_cursor *c=checked_cursor(state,p,1);
  int f=state->soa->region_field;
  (void)ps;
  if (state->soa->fields[f].type==OH_PARTICLE_FIELD_INT && (v<INT_MIN || v>INT_MAX))
    storage_error(state,"packed region does not fit SoA region column");
  oh_particle_adapter_write_integer_value(cursor_field(c,f),state->soa->fields[f].size,v);
}
static int cursor_species(const oh_particle_adapter *a, const void *p) {
  struct oh_state *state=a->user_data;
  struct oh_soa_cursor *c=checked_cursor(state,p,1);
  int f=state->soa->species_field;
  long long value;
  if (f<0) return 0;
  value=read_integer(cursor_field(c,f),state->soa->fields[f].type);
  if (value<state->soa->species_base ||
      value >= (long long)state->soa->species_base+state->n_of_species ||
      value<INT_MIN || value>INT_MAX)
    storage_error(state,"invalid SoA species");
  return (int)value;
}

double *oh_soa_position(struct oh_state *state, int index, int dim) {
  struct oh_soa_storage *s=state->soa;
  require_range(s,index,1);
  for (int f=0; f<s->nfields; ++f)
    if (s->fields[f].role==(OH_PARTICLE_ROLE_X<<dim)) return field_at(s,f,index);
  storage_error(state,"SoA coordinate column missing for active dimension");
  return NULL;
}
double *oh_soa_cursor_position(struct oh_state *state, void *ptr, int dim) {
  struct oh_soa_cursor *p=checked_cursor(state,ptr,1);
  if (dim>=OH_DIMENSION) return NULL;
  for (int f=0; f<state->soa->nfields; ++f)
    if (state->soa->fields[f].role==(OH_PARTICLE_ROLE_X<<dim)) return cursor_field(p,f);
  storage_error(state,"SoA coordinate column missing for active dimension");
  return NULL;
}

void oh_soa_level4_init(struct oh_state *state, int kind) {
  struct oh_soa_storage *s=state->soa;
  oh_particle_adapter a;
  int roles=0;
  if (!s || s->level4) storage_error(state,"configure SoA schema before Level 4 init");
  for (int f=0; f<s->nfields; ++f) roles|=s->fields[f].role;
  for (int d=0; d<OH_DIMENSION; ++d)
    if (!(roles&(OH_PARTICLE_ROLE_X<<d))) storage_error(state,"missing Level 4 coordinate column");
  s->saved_aos_kind = state->use_custom_particle_adapter ? 2 :
                       state->use_custom_particle_mpi_type ? 1 : 0;
  if (s->saved_aos_kind) {
    MPI_Datatype previous = state->custom_particle_mpi_type;
    if (s->saved_aos_kind == 2) {
      s->saved_aos_adapter = *state->particle_adapter;
      previous = s->saved_aos_adapter.mpi_type;
    }
    if (MPI_Type_dup(previous,&s->saved_aos_type)!=MPI_SUCCESS)
      storage_error(state,"cannot preserve AoS particle datatype");
    s->saved_aos_adapter.mpi_type = s->saved_aos_type;
  }
  if (oh_particle_adapter_make_byte_type(sizeof(struct oh_soa_cursor),&s->cursor_type)!=MPI_SUCCESS)
    storage_error(state,"cannot create cursor extent");
  memset(&a,0,sizeof(a));
  a.stride=sizeof(struct oh_soa_cursor); a.mpi_type=s->cursor_type;
  a.user_data=state; a.get_region=cursor_region; a.set_region=cursor_set_region;
  a.get_species=cursor_species; a.single_species=s->species_field<0;
  a.species_base=s->species_base;
  /* NULL map callbacks preserve Level 4 packed-id decoding in Level 2. */
  oh2_set_particle_adapter_state(state,&a);
  s->level4=kind;
  free(s->receive_buffer); s->receive_buffer=NULL;
}

void oh_soa_level4_restore_adapter(struct oh_state *state) {
  struct oh_soa_storage *s=state->soa;
  int initialized=0, finalized=0;
  MPI_Initialized(&initialized);
  if (initialized) MPI_Finalized(&finalized);
  if (initialized && !finalized && s->saved_aos_kind==2) {
    /* The setter duplicates the saved handle before it is released. */
    oh2_set_particle_adapter_state(state,&s->saved_aos_adapter);
  } else if (initialized && !finalized && s->saved_aos_kind==1) {
    oh2_set_particle_mpi_type_state(state,s->saved_aos_type);
    /* The default-context datatype setter borrows its input. Transfer our
       duplicate to the existing default-context ownership mechanism. */
    ownsCustomTParticle=1;
    s->saved_aos_type=MPI_DATATYPE_NULL;
  } else oh2_set_particle_adapter_state(state,NULL);
  if (s->saved_aos_type!=MPI_DATATYPE_NULL && initialized && !finalized)
    MPI_Type_free(&s->saved_aos_type);
  s->saved_aos_type=MPI_DATATYPE_NULL;
  s->saved_aos_kind=0;
}

void *oh_soa_level4_bank(struct oh_state *state, int which, int capacity) {
  struct oh_soa_storage *s=state->soa;
  struct oh_soa_bank *bank;
  if (which<0 || which>=3 || capacity<0 || capacity>INT_MAX/2 || s->banks[which])
    storage_error(state,"invalid Level 4 column bank allocation");
  if (which==0) {
    if (!s->bases && oh_soa_bind(s,NULL,capacity,OH_PARTICLES_OWNED)!=MPI_SUCCESS)
      storage_error(state,"cannot allocate Level 4 columns");
    if (s->capacity!=capacity) storage_error(state,"Level 4 bound capacity mismatch");
  }
  bank=calloc(1,sizeof(*bank));
  if (!bank) storage_error(state,"cannot allocate column bank");
  s->banks[which]=bank; bank->schema=s; bank->capacity=capacity;
  bank->owned=which!=0;
  bank->slots=calloc((size_t)capacity+1,sizeof(*bank->slots));
  bank->bases=which==0 ? s->bases : calloc((size_t)s->nfields,sizeof(void*));
  if (!bank->slots || !bank->bases) storage_error(state,"cannot allocate column bank");
  if (which!=0) for (int f=0; f<s->nfields; ++f) {
    if ((size_t)capacity>(size_t)PTRDIFF_MAX/s->fields[f].size)
      storage_error(state,"column bank size overflow");
    bank->bases[f]=calloc(capacity ? (size_t)capacity : 1,s->fields[f].size);
    if (!bank->bases[f]) storage_error(state,"cannot allocate scratch column");
  }
  for (int i=0; i<=capacity; ++i) { bank->slots[i].bank=bank; bank->slots[i].index=i; }
  if (which==0) {
    state->particle_buffer_bound=1;
    state->particle_buffer_ownership=OH_PARTICLES_BORROWED;
  }
  return bank->slots;
}

void oh_soa_cursor_copy(struct oh_state *state, void *dst, const void *src, int count) {
  struct oh_soa_cursor *d=checked_cursor(state,dst,count), *p=checked_cursor(state,src,count);
  if (!count) return;
  for (int f=0; f<state->soa->nfields; ++f)
    memmove(cursor_field(d,f),cursor_field(p,f),(size_t)count*state->soa->fields[f].size);
}
void oh_soa_level4_finish(struct oh_state *state) {
  /* Bound columns keep their addresses, including borrowed application views. */
  oh_soa_check_capacity(state,state->total_parts);
  oh_soa_cursor_copy(state,state->particles,state->send_buffer,state->total_parts);
}
void *oh_soa_level4_slot(struct oh_state *state, int index) {
  if (!oh_soa_is_level4(state) || !state->particle_buffer_bound ||
      index<0 || index>=state->soa->capacity)
    storage_error(state,"invalid Level 4 SoA particle index");
  return state->soa->banks[0]->slots+index;
}
void oh_soa_level4_release(struct oh_soa_storage *s) {
  int initialized=0,finalized=0;
  for (int b=0; b<3; ++b) {
    struct oh_soa_bank *bank=s->banks[b];
    if (!bank) continue;
    if (bank->owned) { for (int f=0; f<s->nfields; ++f) free(bank->bases[f]); free(bank->bases); }
    free(bank->slots); free(bank); s->banks[b]=NULL;
  }
  if (s->cursor_type!=MPI_DATATYPE_NULL) {
    MPI_Initialized(&initialized); if (initialized) MPI_Finalized(&finalized);
    if (initialized && !finalized) MPI_Type_free(&s->cursor_type);
    s->cursor_type=MPI_DATATYPE_NULL;
  }
  s->level4=0;
}

/* A transfer's datatype selects exactly the participating column slices.
   MPI retains a committed datatype used by a pending request after Type_free.
   All fields use byte signatures, preserving fixed-size opaque payloads. */
static MPI_Datatype cursor_type(struct oh_state *state, const void *ptr, int count) {
  struct oh_soa_cursor *p=checked_cursor(state,ptr,count);
  int nf=state->soa->nfields;
  int *lengths=malloc((size_t)nf*sizeof(int));
  MPI_Aint *disps=malloc((size_t)nf*sizeof(MPI_Aint));
  MPI_Datatype type;
  if (!lengths || !disps) storage_error(state,"cannot allocate transfer descriptor");
  for (int f=0; f<nf; ++f) {
    size_t bytes=(size_t)count*state->soa->fields[f].size;
    if (bytes>INT_MAX) storage_error(state,"SoA MPI column slice exceeds INT_MAX bytes");
    lengths[f]=(int)bytes;
    MPI_Get_address(cursor_field(p,f),disps+f);
  }
  if (MPI_Type_create_hindexed(nf,lengths,disps,MPI_BYTE,&type)!=MPI_SUCCESS ||
      MPI_Type_commit(&type)!=MPI_SUCCESS) storage_error(state,"cannot create column transfer type");
  free(lengths); free(disps);
  return type;
}
int oh_storage_isend(struct oh_state *state, const void *buf, int count,
                     MPI_Datatype type, int peer, int tag, MPI_Comm comm, MPI_Request *req) {
  int result;
  if (!oh_soa_is_level4(state)) return MPI_Isend(buf,count,type,peer,tag,comm,req);
  type=cursor_type(state,buf,count);
  result=MPI_Isend(MPI_BOTTOM,1,type,peer,tag,comm,req); MPI_Type_free(&type); return result;
}
int oh_storage_irecv(struct oh_state *state, void *buf, int count,
                     MPI_Datatype type, int peer, int tag, MPI_Comm comm, MPI_Request *req) {
  int result;
  if (!oh_soa_is_level4(state)) return MPI_Irecv(buf,count,type,peer,tag,comm,req);
  type=cursor_type(state,buf,count);
  result=MPI_Irecv(MPI_BOTTOM,1,type,peer,tag,comm,req); MPI_Type_free(&type); return result;
}
int oh_storage_send(struct oh_state *state, const void *buf, int count,
                    MPI_Datatype type, int peer, int tag, MPI_Comm comm) {
  int result;
  if (!oh_soa_is_level4(state)) return MPI_Send(buf,count,type,peer,tag,comm);
  type=cursor_type(state,buf,count);
  result=MPI_Send(MPI_BOTTOM,1,type,peer,tag,comm); MPI_Type_free(&type); return result;
}
int oh_storage_recv(struct oh_state *state, void *buf, int count,
                    MPI_Datatype type, int peer, int tag, MPI_Comm comm, MPI_Status *status) {
  int result;
  if (!oh_soa_is_level4(state)) return MPI_Recv(buf,count,type,peer,tag,comm,status);
  type=cursor_type(state,buf,count);
  result=MPI_Recv(MPI_BOTTOM,1,type,peer,tag,comm,status); MPI_Type_free(&type); return result;
}
int oh_storage_sendrecv(struct oh_state *state, const void *sbuf, int scount,
                        MPI_Datatype stype, int dst, int stag, void *rbuf, int rcount,
                        MPI_Datatype rtype, int src, int rtag, MPI_Comm comm, MPI_Status *status) {
  int result;
  if (!oh_soa_is_level4(state))
    return MPI_Sendrecv(sbuf,scount,stype,dst,stag,rbuf,rcount,rtype,src,rtag,comm,status);
  stype=cursor_type(state,sbuf,scount); rtype=cursor_type(state,rbuf,rcount);
  result=MPI_Sendrecv(MPI_BOTTOM,1,stype,dst,stag,MPI_BOTTOM,1,rtype,src,rtag,comm,status);
  MPI_Type_free(&stype); MPI_Type_free(&rtype); return result;
}
int oh_storage_alltoallv(struct oh_state *state, const void *sbuf, const int *scount,
                         const int *sdisp, MPI_Datatype stype, void *rbuf,
                         const int *rcount, const int *rdisp, MPI_Datatype rtype, MPI_Comm comm) {
  int n=state->n_of_nodes, result;
  MPI_Request *req;
  if (!oh_soa_is_level4(state))
    return MPI_Alltoallv(sbuf,scount,sdisp,stype,rbuf,rcount,rdisp,rtype,comm);
  req=malloc((size_t)2*n*sizeof(*req));
  if (!req) storage_error(state,"cannot allocate all-to-all requests");
  for (int r=0; r<n; ++r) {
    oh_storage_irecv(state,oh_particle_buffer_at(state->particle_adapter,rbuf,rdisp[r]),
                     rcount[r],rtype,r,0,comm,req+r);
    oh_storage_isend(state,oh_particle_buffer_const_at(state->particle_adapter,sbuf,sdisp[r]),
                     scount[r],stype,r,0,comm,req+n+r);
  }
  result=MPI_Waitall(2*n,req,MPI_STATUSES_IGNORE); free(req); return result;
}

int oh_soa_append(struct oh_state *state, const void *const *values, int nfields,
                  oh_particle_ref *ref) {
  struct oh_soa_storage *s=state->soa;
  long long index=(long long)state->total_parts+state->n_of_injections;
  void *record;
  if (!s || !values || !ref || nfields!=s->nfields) return MPI_ERR_ARG;
  if (index<0 || index>=s->capacity) return MPI_ERR_COUNT;
  for (int f=0; f<nfields; ++f) if (!values[f]) return MPI_ERR_ARG;
  if (s->species_field>=0) {
    int f=s->species_field;
    long long sp=read_integer(values[f],s->fields[f].type);
    if (sp<s->species_base || sp>=(long long)s->species_base+state->n_of_species) return MPI_ERR_ARG;
  }
  record=malloc(s->record_size);
  if (!record) return MPI_ERR_NO_MEM;
  for (int f=0; f<nfields; ++f) memcpy((char*)record+s->offsets[f],values[f],s->fields[f].size);
  oh_soa_unpack(s,(int)index,record,1); free(record);
  ++state->n_of_injections;
  if (oh_context_is_default_state(state)) nOfInjections=state->n_of_injections;
  ref->context=state; ref->generation=state->particle_generation; ref->index=(int)index;
  return MPI_SUCCESS;
}

int oh_context_particle_ref_valid(const oh_context *ctx, oh_particle_ref ref) {
  return ctx && ctx->particle_storage==OH_PARTICLE_STORAGE_SOA &&
    ctx->particle_buffer_bound && ref.context==ctx &&
    ref.generation==ctx->particle_generation && ref.index>=ctx->total_parts &&
    (long long)ref.index<(long long)ctx->total_parts+ctx->n_of_injections;
}
