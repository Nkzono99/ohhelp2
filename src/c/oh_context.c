/* File: oh_context.c
   v2 context facade for the current default OhHelp instance.
*/
#include <limits.h>
#include <stdlib.h>

#include "ohhelp1.h"
#include "ohhelp1_internal.h"
#include "ohhelp2.h"
#include "ohhelp2_internal.h"
#include "ohhelp3.h"
#include "ohhelp3_internal.h"
#include "oh_context_internal.h"
#include "oh_context.h"

struct oh_state OhDefaultState;

static void free_context_particle_type(struct oh_state *context);
static int init_context_particle_adapter(struct oh_state *context,
                                         MPI_Datatype type, int owns_type);
static void free_context_comm(struct oh_state *context);
static void free_context_level1_storage(struct oh_state *context);
static void ensure_context_level1_storage(struct oh_state *context);
static void free_context_level2_storage(struct oh_state *context);
static void ensure_context_level2_storage(struct oh_state *context,
                                          int maxlocalp);
static void free_context_region_id_storage(struct oh_state *context);
static void configure_context_level1_neighbors(struct oh_state *context,
                                               int **nbor, int *pcoord);

static int
storage_ownership_is_valid(int ownership) {
  return ownership==OH_PARTICLES_BORROWED || ownership==OH_PARTICLES_OWNED;
}

static struct oh_state*
context_or_default(struct oh_state *context) {
  if (!context) context = oh1_state();
  return context;
}

static void
require_particle_buffer_unbound(struct oh_state *context, const char *api) {
  if (context->particle_buffer_bound)
    local_errstop("%s cannot change particle layout while particles are bound",
                  (char*)api);
}

static void
require_particle_configuration_unbound(struct oh_state *context,
                                       const char *api) {
  if (context->particle_buffer_bound)
    local_errstop("%s cannot change species count while particles are bound",
                  (char*)api);
  if (context->particle_accounting_bound)
    local_errstop("%s cannot change species count while accounting is bound",
                  (char*)api);
  if (context->owns_level3_storage)
    local_errstop("%s cannot change species count while Level 3 is configured",
                  (char*)api);
}

void
oh_context_validate_species_node_capacity(int n_of_nodes, int nspec,
                                          const char *api) {
  long long nnns;
  long long hist_slots;
  long long species_classes;
  long long request_slots;
  long long comm_list_a;
  long long comm_list_b;

  if (n_of_nodes<=0)
    local_errstop("%s requires initialized communicator", (char*)api);
  if (nspec > INT_MAX / n_of_nodes)
    local_errstop("%s species/node count exceeds Level 2 particle accounting capacity",
                  (char*)api);

  nnns = (long long)n_of_nodes * nspec;
  hist_slots = 2LL * nnns;
  species_classes = 4LL * nspec;
  request_slots = 4LL * nnns + 2LL * OH_NEIGHBORS;
  comm_list_a = 2LL * OH_NEIGHBORS * (nnns + 1LL) +
                (long long)n_of_nodes * ((long long)nspec + 3LL);
  comm_list_b = (14LL + 4LL * nspec) * n_of_nodes;

  if (hist_slots > INT_MAX || species_classes > INT_MAX ||
      request_slots > INT_MAX ||
      comm_list_a > INT_MAX || comm_list_b > INT_MAX)
    local_errstop("%s species/node count exceeds Level 2 particle accounting capacity",
                  (char*)api);
}

static int
context_is_default(const struct oh_state *context) {
  return context == &OhDefaultState;
}

int
oh_context_is_default_state(const struct oh_state *state) {
  return context_is_default(state);
}

static void*
context_calloc(size_t count, size_t size, const char *name) {
  void *ptr;

  if (size != 0 && count > ((size_t)-1) / size)
    local_errstop("invalid allocation size for %s", (char*)name);
  ptr = calloc(count, size);
  if (!ptr) local_errstop("out of memory for %s", (char*)name);
  return ptr;
}

static void
free_context_particle_type(struct oh_state *context) {
  int initialized = 0;
  int finalized = 0;

  if (!context || !context->owns_particle_mpi_type ||
      context->particle_mpi_type == MPI_DATATYPE_NULL)
    return;
  MPI_Initialized(&initialized);
  if (initialized) MPI_Finalized(&finalized);
  if (initialized && !finalized)
    MPI_Type_free(&context->particle_mpi_type);
  context->particle_mpi_type = MPI_DATATYPE_NULL;
  context->owned_particle_adapter.mpi_type = MPI_DATATYPE_NULL;
  context->owned_custom_particle_adapter.mpi_type = MPI_DATATYPE_NULL;
  context->custom_particle_mpi_type = MPI_DATATYPE_NULL;
  context->owns_particle_mpi_type = 0;
}

static void
free_context_comm(struct oh_state *context) {
  int initialized = 0;
  int finalized = 0;

  if (!context || !context->owns_comm || context->comm == MPI_COMM_NULL)
    return;
  MPI_Initialized(&initialized);
  if (initialized) MPI_Finalized(&finalized);
  if (initialized && !finalized) MPI_Comm_free(&context->comm);
  context->comm = MPI_COMM_NULL;
  context->owns_comm = 0;
}

static void
free_context_region_id_storage(struct oh_state *context) {
  int *freed = NULL;

  if (!context) return;
  if (context_is_default(context)) {
    context->region_id = RegionId;
    context->subdomain_id = SubdomainId;
    context->owns_region_id = 0;
    context->owns_subdomain_id = 0;
    return;
  }
  if (context->owns_region_id && context->region_id) {
    freed = context->region_id;
    free(context->region_id);
  }
  if (context->owns_subdomain_id && context->subdomain_id &&
      context->subdomain_id != freed)
    free(context->subdomain_id);
  context->region_id = NULL;
  context->subdomain_id = NULL;
  context->owns_region_id = 0;
  context->owns_subdomain_id = 0;
}

static int
init_context_particle_adapter(struct oh_state *context, MPI_Datatype type,
                              int owns_type) {
  context->owned_particle_adapter = oh_default_particle_adapter(type);
  context->particle_adapter = &context->owned_particle_adapter;
  context->custom_particle_adapter = &context->owned_custom_particle_adapter;
  context->particle_mpi_type = type;
  context->custom_particle_mpi_type = MPI_DATATYPE_NULL;
  context->use_custom_particle_mpi_type = 0;
  context->use_custom_particle_adapter = 0;
  context->owns_particle_mpi_type = owns_type;
  oh2_refresh_particle_adapter_fast_path(context);
  return MPI_SUCCESS;
}

static void
free_context_level1_storage(struct oh_state *context) {
  int initialized = 0;
  int finalized = 0;
  int i;

  if (!context || context_is_default(context) || !context->owns_level1_storage)
    return;

  MPI_Initialized(&initialized);
  if (initialized) MPI_Finalized(&finalized);
  if (initialized && !finalized) {
    if (context->communicators && context->communicators->body) {
      for (i=0; i<context->communicators->n; i++)
        if (context->communicators->body[i] != MPI_COMM_NULL)
          MPI_Comm_free(context->communicators->body+i);
    }
    if (context->world_group != MPI_GROUP_NULL)
      MPI_Group_free(&context->world_group);
    if (context->histogram_type != MPI_DATATYPE_NULL)
      MPI_Type_free(&context->histogram_type);
    if (context->comm_list_type != MPI_DATATYPE_NULL)
      MPI_Type_free(&context->comm_list_type);
  }

  free(context->n_of_primaries);
  free(context->total_particles_global);
  free(context->n_of_particles_to_stay);
  free(context->injected_particles);
  free(context->n_of_recv);
  free(context->n_of_send);
  free(context->recv_counts);
  free(context->send_counts);
  free(context->level4_send_counts);
  free(context->nodes);
  free(context->nodes_next);
  free(context->node_queue);
  free(context->less_heap.node);
  free(context->less_heap.index);
  free(context->greater_heap.node);
  free(context->greater_heap.index);
  free(context->temp_array);
  free(context->neighbors);
  free(context->src_neighbors);
  free(context->comm_list);
  free(context->rl_index);
  free(context->sl_head_tail);
  free(context->sec_sl_head_tail);
  free(context->sec_rl_size);
  if (context->communicators) free(context->communicators->body);
  free(context->communicators);
  free(context->my_comm);

  context->n_of_primaries = NULL;
  context->total_particles_global = NULL;
  context->n_of_particles_to_stay = NULL;
  context->injected_particles = NULL;
  context->n_of_recv = NULL;
  context->n_of_send = NULL;
  context->recv_counts = NULL;
  context->send_counts = NULL;
  context->level4_send_counts = NULL;
  context->level4_send_counts_size = 0;
  context->nodes = NULL;
  context->nodes_next = NULL;
  context->node_queue = NULL;
  context->less_heap.n = context->greater_heap.n = 0;
  context->less_heap.node = context->less_heap.index = NULL;
  context->greater_heap.node = context->greater_heap.index = NULL;
  context->temp_array = NULL;
  context->neighbors = NULL;
  context->dst_neighbors = NULL;
  context->src_neighbors = NULL;
  context->comm_list = NULL;
  context->sec_recv_list = NULL;
  context->rl_index = NULL;
  context->sl_head_tail = NULL;
  context->sec_sl_head_tail = NULL;
  context->sec_rl_size = NULL;
  context->world_group = MPI_GROUP_NULL;
  context->communicators = NULL;
  context->my_comm = NULL;
  context->histogram_type = MPI_DATATYPE_NULL;
  context->comm_list_type = MPI_DATATYPE_NULL;
  context->owns_level1_storage = 0;
}

static void
free_context_level2_storage(struct oh_state *context) {
  if (!context || context_is_default(context) || !context->owns_level2_storage)
    return;

  free(context->send_buffer);
  free(context->recv_buffer_bases);
  free(context->send_buffer_disps);
  free(context->recv_buffer_disps);
  free(context->requests);
  free(context->statuses);

  context->send_buffer = NULL;
  context->recv_buffer_bases = NULL;
  context->send_buffer_disps = NULL;
  context->recv_buffer_disps = NULL;
  context->requests = NULL;
  context->statuses = NULL;
  context->n_of_injections = 0;
  context->owns_level2_storage = 0;
}

static void
ensure_context_level2_storage(struct oh_state *context, int maxlocalp) {
  int nn = context->n_of_nodes;
  int ns = context->n_of_species;
  int nnns = nn * ns;
  size_t stride;
  size_t slots;

  if (context_is_default(context)) return;
  if (context->owns_level2_storage) return;
  if (!context->particle_adapter)
    local_errstop("particle adapter is not configured");

  stride = context->particle_adapter->stride;
  slots = maxlocalp > 0 ? (size_t)maxlocalp : 1;
  context->send_buffer = context_calloc(slots, stride, "SendBuf");
  context->recv_buffer_bases =
    context_calloc((size_t)2*ns+1, sizeof(void*), "RecvBufBases");
  context->send_buffer_disps =
    (int*)context_calloc((size_t)nnns, sizeof(int), "SendBufDisps");
  context->recv_buffer_disps =
    (int*)context_calloc((size_t)nn, sizeof(int), "RecvBufDisps");
  context->requests =
    (MPI_Request*)context_calloc((size_t)nnns*4+OH_NEIGHBORS*2,
                                 sizeof(MPI_Request), "Requests");
  context->statuses =
    (MPI_Status*)context_calloc((size_t)nnns*4+OH_NEIGHBORS*2,
                                sizeof(MPI_Status), "Statuses");
  context->n_of_injections = 0;
  context->owns_level2_storage = 1;
}

static void
ensure_context_level1_storage(struct oh_state *context) {
  int nn = context->n_of_nodes;
  int ns = context->n_of_species;
  int nnns = nn * ns;
  int clsize;
  int i;
  MPI_Datatype vector_type = MPI_DATATYPE_NULL;
  MPI_Aint int_extent = (MPI_Aint)sizeof(int);

  if (context_is_default(context)) return;
  if (context->owns_level1_storage) return;

  context->n_of_primaries =
    (int*)context_calloc((size_t)2*nnns, sizeof(int), "NOfPrimaries");
  context->total_particles_global =
    (dint*)context_calloc((size_t)nn+1, sizeof(dint), "TotalPGlobal");
  context->n_of_particles_to_stay =
    (dint*)context_calloc((size_t)nn, sizeof(dint), "NOfPToStay");
  context->injected_particles =
    (int*)context_calloc((size_t)4*ns, sizeof(int), "InjectedParticles");
  context->n_of_recv =
    (int*)context_calloc((size_t)2*nnns, sizeof(int), "NOfRecv");
  context->n_of_send =
    (int*)context_calloc((size_t)2*nnns, sizeof(int), "NOfSend");
  context->recv_counts =
    (int*)context_calloc((size_t)2*nnns, sizeof(int), "RecvCounts");
  context->send_counts =
    (int*)context_calloc((size_t)2*nnns, sizeof(int), "SendCounts");
  context->temp_array =
    (int*)context_calloc((size_t)nn, sizeof(int), "TempArray");
  context->nodes =
    (struct S_node*)context_calloc((size_t)nn, sizeof(struct S_node),
                                   "Nodes");
  context->nodes_next =
    (struct S_node*)context_calloc((size_t)nn, sizeof(struct S_node),
                                   "NodesNext");
  context->node_queue =
    (struct S_node**)context_calloc((size_t)nn, sizeof(struct S_node*),
                                    "NodeQueue");
  context->less_heap.node =
    (int*)context_calloc((size_t)nn+1, sizeof(int), "LessHeap.node");
  context->less_heap.index =
    (int*)context_calloc((size_t)nn, sizeof(int), "LessHeap.index");
  context->greater_heap.node =
    (int*)context_calloc((size_t)nn+1, sizeof(int), "GreaterHeap.node");
  context->greater_heap.index =
    (int*)context_calloc((size_t)nn, sizeof(int), "GreaterHeap.index");
  context->neighbors =
    (int(*)[OH_NEIGHBORS])context_calloc(3, sizeof(*context->neighbors),
                                         "Neighbors");
  context->src_neighbors =
    (int*)context_calloc(OH_NEIGHBORS, sizeof(int), "SrcNeighbors");
  context->dst_neighbors = context->neighbors[0];
  context->rl_index =
    (int*)context_calloc(OH_NEIGHBORS+1, sizeof(int), "RLIndex");
  context->sl_head_tail =
    (int*)context_calloc(2, sizeof(int), "SLHeadTail");
  context->sec_sl_head_tail =
    (int*)context_calloc(2, sizeof(int), "SecSLHeadTail");
  context->sec_rl_size =
    (int*)context_calloc(1, sizeof(int), "SecRLSize");
  context->communicators =
    (struct S_comms*)context_calloc(1, sizeof(struct S_comms), "Comms");
  context->communicators->body =
    (MPI_Comm*)context_calloc((size_t)nn, sizeof(MPI_Comm), "Comms.body");
  context->my_comm =
    (struct S_mycommc*)context_calloc(1, sizeof(struct S_mycommc), "MyComm");

  clsize = 2*OH_NEIGHBORS*(nn*ns+1)+nn*(ns+3);
  if (clsize<(14+4*ns)*nn) clsize = (14+4*ns)*nn;
  context->comm_list =
    (struct S_commlist*)context_calloc((size_t)clsize,
                                       sizeof(struct S_commlist), "CommList");

  for (i=0; i<nn; i++) {
    context->nodes[i].id = i;
    context->nodes_next[i].id = i;
    context->communicators->body[i] = MPI_COMM_NULL;
  }
  for (i=0; i<OH_NEIGHBORS; i++) {
    context->neighbors[0][i] =
      (i==(OH_NEIGHBORS>>1)) ? context->my_rank : -(nn+1);
    context->neighbors[1][i] = context->neighbors[0][i];
    context->neighbors[2][i] = context->neighbors[0][i];
    context->src_neighbors[i] = context->neighbors[0][i];
  }

  context->my_comm->prime = MPI_COMM_NULL;
  context->my_comm->sec = MPI_COMM_NULL;
  context->my_comm->rank = 0;
  context->my_comm->root = 0;
  context->my_comm->black = 0;
  context->world_group = MPI_GROUP_NULL;
  MPI_Comm_group(context->comm, &context->world_group);

  MPI_Type_vector(2*ns, 1, nn, MPI_INT, &vector_type);
  MPI_Type_create_resized(vector_type, 0, int_extent,
                          &context->histogram_type);
  MPI_Type_commit(&context->histogram_type);
  MPI_Type_free(&vector_type);

  MPI_Type_contiguous(sizeof(struct S_commlist), MPI_BYTE,
                      &context->comm_list_type);
  MPI_Type_commit(&context->comm_list_type);

  context->owns_level1_storage = 1;
}

void
oh_context_build_grid_neighbors(struct oh_state *state, const int *pcoord,
                                int raw[OH_NEIGHBORS]) {
  int nn = state->n_of_nodes;
  int me = state->my_rank;
  int p, q, r, i, j, k, l;
  int yplus = (OH_DIMENSION>1) ? 2 : 0;
  int zplus = (OH_DIMENSION>2) ? 2 : 0;
  int xoff, yoff, zoff;

  if (!pcoord)
    local_errstop("context grid neighbor build requires pcoord");
  p = pcoord[0];
  q = (OH_DIMENSION>1) ? pcoord[1] : 1;
  r = (OH_DIMENSION>2) ? pcoord[2] : 1;
  if (p<=0 || q<=0 || r<=0 || p*q*r!=nn)
    local_errstop("context pcoord product must match communicator size");

  i = me % p;
  j = (me/p) % q;
  k = me / (p*q);
  for (l=0,zoff=-1; zoff<zplus; zoff++) {
    for (yoff=-1; yoff<yplus; yoff++) {
      for (xoff=-1; xoff<2; xoff++,l++) {
        raw[l] = (i+xoff+p)%p + (((j+yoff+q)%q) + ((k+zoff+r)%r)*q)*p;
      }
    }
  }
}

void
oh_context_apply_neighbors(struct oh_state *state,
                           const int raw[OH_NEIGHBORS]) {
  int nn = state->n_of_nodes;
  int *temp = state->temp_array;
  int i;

  memcpy(state->neighbors[1], raw, sizeof(int)*OH_NEIGHBORS);
  memcpy(state->neighbors[2], raw, sizeof(int)*OH_NEIGHBORS);
  state->dst_neighbors = state->neighbors[0];
  for (i=0; i<nn; i++) temp[i] = 0;
  for (i=0; i<OH_NEIGHBORS; i++) {
    int dst = raw[i];
    int src = raw[(OH_NEIGHBORS-1)-i];
    if (dst<0) {
      state->dst_neighbors[i] = -(nn+1);
    } else {
      state->dst_neighbors[i] = (temp[dst]&1) ? -(dst+1) : dst;
      temp[dst] |= 1;
    }
    if (src<0) {
      state->src_neighbors[i] = -(nn+1);
    } else {
      state->src_neighbors[i] = (temp[src]&2) ? -(src+1) : src;
      temp[src] |= 2;
    }
  }
}

static void
configure_context_level1_neighbors(struct oh_state *context, int **nbor,
                                   int *pcoord) {
  int me = context->my_rank;
  int *raw;
  int i;

  if (!nbor)
    local_errstop("non-default init1_state() requires a neighbor output");

  raw = *nbor;
  if (!raw) {
    raw = context->neighbors[1];
    *nbor = raw;
    oh_context_build_grid_neighbors(context, pcoord, raw);
  } else {
    for (i=0; i<OH_NEIGHBORS; i++) {
      int n = raw[i];
      int m = raw[(OH_NEIGHBORS-1)-i];
      int reply;
      MPI_Status st;

      if (m>=0) {
        if (n>=0)
          MPI_Sendrecv(&i, 1, MPI_INT, n, 0, &reply, 1, MPI_INT, m, 0,
                       context->comm, &st);
        else
          MPI_Recv(&reply, 1, MPI_INT, m, 0, context->comm, &st);
        if (reply!=i)
          local_errstop("rank-%d's %d-th neighbor rank-%d says "
                        "rank-%d is not %d-th neighbor but %d-th",
                        me, (OH_NEIGHBORS-1)-i, m, me, i, reply);
      } else if (n>=0) {
        MPI_Send(&i, 1, MPI_INT, n, 0, context->comm);
      }
    }
  }

  oh_context_apply_neighbors(context, raw);
}

void
oh_context_init_level1_state(struct oh_state *state, int **sdid, int nspec,
                             int maxfrac, int **nphgram, int **totalp,
                             int **rcounts, int **scounts,
                             struct S_mycommc *mycommc,
                             struct S_mycommf *mycommf, int **nbor,
                             int *pcoord, int stats, int repiter,
                             int verbose) {
  int i;
  int accounting_ownership;
  int *requested_sdid;
  int old_owned_sdid;

  if (!state || context_is_default(state))
    local_errstop("oh_context_init_level1_state() requires a non-default context");
  if (stats || verbose)
    local_errstop("non-default init1_state() does not support stats or verbose yet");
  if (nspec<=0)
    local_errstop("non-default init1_state() requires nspec > 0");
  if (maxfrac<0)
    local_errstop("non-default init1_state() requires maxfrac >= 0");
  oh_context_validate_species_node_capacity(state->n_of_nodes, nspec,
                                            "non-default init1_state()");
  if (!sdid || !nphgram || !totalp)
    local_errstop("non-default init1_state() requires sdid, nphgram, and totalp");

  old_owned_sdid = state->owns_region_id && *sdid == state->region_id;
  if (old_owned_sdid) *sdid = NULL;
  if (state->n_of_particles_local_ownership==OH_PARTICLES_OWNED &&
      *nphgram == state->n_of_particles_local)
    *nphgram = NULL;
  if (state->total_particles_next_ownership==OH_PARTICLES_OWNED &&
      *totalp == state->total_particles_next)
    *totalp = NULL;
  if (nbor && state->owns_level1_storage && *nbor == state->neighbors[1])
    *nbor = NULL;
  if (rcounts && state->owns_level1_storage && *rcounts == state->recv_counts)
    *rcounts = NULL;
  if (scounts && state->owns_level1_storage && *scounts == state->send_counts)
    *scounts = NULL;

  if ((*nphgram && !*totalp) || (!*nphgram && *totalp))
    local_errstop("non-default init1_state() requires nphgram and totalp "
                  "to be both NULL or both borrowed");
  if (rcounts && *rcounts)
    local_errstop("non-default init1_state() cannot borrow recv counts yet");
  if (scounts && *scounts)
    local_errstop("non-default init1_state() cannot borrow send counts yet");
  requested_sdid = *sdid;

  oh_context_unbind_particle_accounting_state(state);
  free_context_level2_storage(state);
  free_context_level1_storage(state);
  free_context_region_id_storage(state);

  state->curr_mode = MODE_NORM_PRI;
  state->acc_mode = 0;
  state->n_of_species = nspec;
  state->max_fraction = maxfrac;
  state->stats_mode = stats;
  state->report_iteration = repiter;
  state->weighted_load_balancing = FALSE;
  state->n_of_particles = 0;
  state->total_load = 0.0;
  state->n_of_local_particles_max = 0;
  state->n_of_local_load_max = 0.0;

  state->region_id = requested_sdid
                       ? requested_sdid
                       : (int*)mem_alloc(sizeof(int), 2, "SubdomainID");
  state->subdomain_id = state->region_id;
  state->owns_region_id = requested_sdid ? 0 : 1;
  state->owns_subdomain_id = 0;
  state->region_id[0] = state->my_rank;
  state->region_id[1] = -1;
  *sdid = state->region_id;

  ensure_context_level1_storage(state);
  configure_context_level1_neighbors(state, nbor, pcoord);

  accounting_ownership = (*nphgram || *totalp)
                           ? OH_PARTICLES_BORROWED
                           : OH_PARTICLES_OWNED;
  oh_context_bind_particle_accounting_state(state, nphgram, totalp, NULL,
                                            accounting_ownership);
  if (rcounts) *rcounts = state->recv_counts;
  if (scounts) *scounts = state->send_counts;
  for (i=0; i<2*state->n_of_species*state->n_of_nodes; i++) {
    state->n_of_particles_local[i] = 0;
    state->n_of_recv[i] = 0;
    state->n_of_send[i] = 0;
    state->recv_counts[i] = 0;
    state->send_counts[i] = 0;
  }
  for (i=0; i<2*state->n_of_species; i++)
    state->total_particles_next[i] = 0;
  for (i=0; i<state->n_of_nodes; i++) {
    state->region_weights[i] = 1.0;
    state->total_load_global[i] = 0.0;
  }
  for (i=0; i<4*state->n_of_species; i++) state->injected_particles[i] = 0;

  state->my_comm_c = mycommc;
  state->my_comm_f = mycommf;
  if (state->my_comm) {
    state->my_comm->prime = MPI_COMM_NULL;
    state->my_comm->sec = MPI_COMM_NULL;
    state->my_comm->rank = 0;
    state->my_comm->root = 0;
    state->my_comm->black = 0;
    if (mycommc) *mycommc = *state->my_comm;
  }
  if (mycommf) {
    mycommf->prime = MPI_Comm_c2f(MPI_COMM_NULL);
    mycommf->sec = MPI_Comm_c2f(MPI_COMM_NULL);
    mycommf->rank = 0;
    mycommf->root = 0;
    mycommf->black = 0;
  }
}

int
oh_context_create(MPI_Comm comm, struct oh_state **context) {
  MPI_Datatype particle_type = MPI_DATATYPE_NULL;
  struct oh_state *created;
  int mpi_initialized = 0;
  int mpi_finalized = 0;
  int err, i;

  if (!context) return MPI_ERR_ARG;
  *context = NULL;
  if (comm == MPI_COMM_NULL) return MPI_ERR_COMM;
  MPI_Initialized(&mpi_initialized);
  if (mpi_initialized) MPI_Finalized(&mpi_finalized);
  if (!mpi_initialized || mpi_finalized) return MPI_ERR_OTHER;

  created = (struct oh_state*)calloc(1, sizeof(*created));
  if (!created) return MPI_ERR_NO_MEM;

  created->comm = MPI_COMM_NULL;
  err = MPI_Comm_dup(comm, &created->comm);
  if (err != MPI_SUCCESS) {
    free(created);
    return err;
  }
  created->owns_comm = 1;
  MPI_Comm_size(created->comm, &created->n_of_nodes);
  MPI_Comm_rank(created->comm, &created->my_rank);
  created->curr_mode = MODE_NORM_PRI;
  created->acc_mode = 0;
  created->world_group = MPI_GROUP_NULL;
  created->histogram_type = MPI_DATATYPE_NULL;
  created->comm_list_type = MPI_DATATYPE_NULL;

  err = oh_particle_adapter_make_byte_type(sizeof(struct S_particle),
                                           &particle_type);
  if (err != MPI_SUCCESS) {
    free_context_comm(created);
    free(created);
    return err;
  }
  init_context_particle_adapter(created, particle_type, 1);

  created->region_weights =
    (double*)calloc((size_t)created->n_of_nodes, sizeof(double));
  created->total_load_global =
    (double*)calloc((size_t)created->n_of_nodes, sizeof(double));
  if (!created->region_weights || !created->total_load_global) {
    free_context_particle_type(created);
    free_context_comm(created);
    free(created->region_weights);
    free(created->total_load_global);
    free(created);
    return MPI_ERR_NO_MEM;
  }
  for (i=0; i<created->n_of_nodes; i++) created->region_weights[i] = 1.0;

  created->n_of_particles_local_ownership = OH_PARTICLES_BORROWED;
  created->total_particles_next_ownership = OH_PARTICLES_BORROWED;
  created->particle_buffer_ownership = OH_PARTICLES_BORROWED;
  created->particle_base_ownership = OH_PARTICLES_BORROWED;
  *context = created;
  return MPI_SUCCESS;
}

void
oh_context_destroy(struct oh_state *context) {
  if (!context || context_is_default(context)) return;
  oh_context_unbind_particles(context);
  oh_context_unbind_particle_accounting_state(context);
  oh3_free_context_state(context);
  free_context_level2_storage(context);
  free_context_level1_storage(context);
  free_context_particle_type(context);
  free_context_region_id_storage(context);
  free(context->region_weights);
  free(context->total_load_global);
  free_context_comm(context);
  free(context);
}

void
oh1_sync_default_state(void) {
  OhDefaultState.comm = MCW;
  OhDefaultState.owns_comm = 0;
  OhDefaultState.n_of_nodes = nOfNodes;
  OhDefaultState.my_rank = myRank;
  OhDefaultState.region_id = RegionId;
  OhDefaultState.subdomain_id = SubdomainId;
  OhDefaultState.curr_mode = currMode;
  OhDefaultState.acc_mode = accMode;
  OhDefaultState.n_of_species = nOfSpecies;
  OhDefaultState.max_fraction = maxFraction;
  OhDefaultState.n_of_particles_local = NOfPLocal;
  OhDefaultState.n_of_primaries = NOfPrimaries;
  OhDefaultState.total_particles_global = TotalPGlobal;
  OhDefaultState.region_weights = RegionWeights;
  OhDefaultState.total_load_global = TotalLoadGlobal;
  OhDefaultState.n_of_particles = nOfParticles;
  OhDefaultState.total_load = nOfLoad;
  OhDefaultState.n_of_local_particles_max = nOfLocalPMax;
  OhDefaultState.n_of_local_load_max = nOfLocalLoadMax;
  OhDefaultState.weighted_load_balancing = weightedLoadBalancing;
  OhDefaultState.n_of_particles_to_stay = NOfPToStay;
  OhDefaultState.total_particles = TotalP;
  OhDefaultState.total_particles_next = TotalPNext;
  OhDefaultState.injected_particles = InjectedParticles;
  OhDefaultState.n_of_recv = NOfRecv;
  OhDefaultState.n_of_send = NOfSend;
  OhDefaultState.recv_counts = RecvCounts;
  OhDefaultState.send_counts = SendCounts;
  OhDefaultState.owns_level1_storage = 0;
  OhDefaultState.primary_parts = primaryParts;
  OhDefaultState.total_parts = totalParts;
  OhDefaultState.nodes = Nodes;
  OhDefaultState.nodes_next = NodesNext;
  OhDefaultState.node_queue = NodeQueue;
  OhDefaultState.less_heap = LessHeap;
  OhDefaultState.greater_heap = GreaterHeap;
  OhDefaultState.temp_array = TempArray;
  OhDefaultState.neighbors = Neighbors;
  OhDefaultState.dst_neighbors = DstNeighbors;
  OhDefaultState.src_neighbors = SrcNeighbors;
  OhDefaultState.comm_list = CommList;
  OhDefaultState.sec_recv_list = SecRList;
  OhDefaultState.rl_index = RLIndex;
  OhDefaultState.sl_head_tail = SLHeadTail;
  OhDefaultState.sec_sl_head_tail = SecSLHeadTail;
  OhDefaultState.sec_rl_size = &SecRLSize;
  OhDefaultState.world_group = GroupWorld;
  OhDefaultState.communicators = &Comms;
  OhDefaultState.my_comm = MyComm;
  OhDefaultState.my_comm_c = MyCommC;
  OhDefaultState.my_comm_f = MyCommF;
  OhDefaultState.histogram_type = T_Histogram;
  OhDefaultState.comm_list_type = T_Commlist;
  OhDefaultState.n_of_local_particles_limit = nOfLocalPLimit;
  OhDefaultState.particles = Particles;
  OhDefaultState.send_buffer = SendBuf;
  OhDefaultState.recv_buffer_bases = RecvBufBases;
  OhDefaultState.owns_level2_storage = 0;
  OhDefaultState.secondary_base = secondaryBase;
  OhDefaultState.total_local_particles = totalLocalParticles;
  OhDefaultState.send_buffer_disps = SendBufDisps;
  OhDefaultState.recv_buffer_disps = RecvBufDisps;
  OhDefaultState.n_of_injections = nOfInjections;
  OhDefaultState.spec_base = specBase;
  OhDefaultState.particle_mpi_type =
    useCustomParticleAdapter ? CustomTParticle : T_Particle;
  OhDefaultState.owns_particle_mpi_type = ownsTParticle;
  OhDefaultState.custom_particle_mpi_type = CustomTParticle;
  OhDefaultState.use_custom_particle_mpi_type = useCustomTParticle;
  OhDefaultState.particle_adapter =
    useCustomParticleAdapter ? &CustomParticleAdapter : &ParticleAdapter;
  OhDefaultState.custom_particle_adapter = &CustomParticleAdapter;
  OhDefaultState.use_custom_particle_adapter = useCustomParticleAdapter;
  OhDefaultState.particle_region_access =
    oh_particle_adapter_region_access(OhDefaultState.particle_adapter);
  OhDefaultState.particle_species_access =
    oh_particle_adapter_species_access(OhDefaultState.particle_adapter);
  OhDefaultState.particle_map_to_neighbor_access =
    oh_particle_adapter_map_to_neighbor_access(OhDefaultState.particle_adapter);
  OhDefaultState.particle_map_to_subdomain_access =
    oh_particle_adapter_map_to_subdomain_access(OhDefaultState.particle_adapter);
  OhDefaultState.requests = Requests;
  OhDefaultState.statuses = Statuses;
  OhDefaultState.exclude_level2 = excludeLevel2;
#ifdef OH_POS_AWARE
  OhDefaultState.abs_neighbors = AbsNeighbors;
#else
  OhDefaultState.abs_neighbors = NULL;
#endif
  OhDefaultState.subdomains = SubDomains;
  OhDefaultState.subdomains_float = SubDomainsFloat;
  OhDefaultState.grid = Grid;
#ifdef OH_POS_AWARE
  OhDefaultState.grid_mask = gridMask;
  OhDefaultState.log_grid = logGrid;
#else
  OhDefaultState.grid_mask = 0;
  OhDefaultState.log_grid = 0;
#endif
  OhDefaultState.subdomain_desc = SubDomainDesc;
  OhDefaultState.n_of_boundaries = nOfBoundaries;
  OhDefaultState.boundaries = Boundaries;
  OhDefaultState.adjacent = &Adjacent[0][0];
  OhDefaultState.n_of_fields = nOfFields;
  OhDefaultState.field_types = (int*)FieldTypes;
  OhDefaultState.field_desc = FieldDesc;
  OhDefaultState.n_of_exchanges = nOfExc;
  OhDefaultState.boundary_comm_fields = BoundaryCommFields;
  OhDefaultState.boundary_comm_types = (int*)BoundaryCommTypes;
  OhDefaultState.border_exchange = (struct S_borderexc*)BorderExc;
  OhDefaultState.stats = &Stats;
  OhDefaultState.stats_mode = statsMode;
  OhDefaultState.report_iteration = reportIteration;
  OhDefaultState.stats_time_type = &T_StatsTime;
  OhDefaultState.stats_time_op = &Op_StatsTime;
  OhDefaultState.stats_part_op = &Op_StatsPart;
}

struct oh_state*
oh1_state(void) {
  oh1_sync_default_state();
  return &OhDefaultState;
}

struct oh_state*
oh_default_context(void) {
  return oh1_state();
}

void
oh_context_configure_particles(struct oh_state *context, int nspec,
                               int maxfrac) {
  context = context_or_default(context);
  if (nspec<=0)
    local_errstop("oh_context_configure_particles() requires nspec > 0");
  if (maxfrac<0)
    local_errstop("oh_context_configure_particles() requires maxfrac >= 0");
  oh_context_validate_species_node_capacity(
    context->n_of_nodes, nspec, "oh_context_configure_particles()");

  if (!context_is_default(context) && context->n_of_species>0) {
    require_particle_configuration_unbound(
      context, "oh_context_configure_particles()");
    if (context->n_of_species!=nspec) {
      free_context_level2_storage(context);
      free_context_level1_storage(context);
    }
  }

  if (!context->region_id) {
    context->region_id = (int*)calloc(2, sizeof(int));
    if (!context->region_id) local_errstop("out of memory for RegionId");
    context->owns_region_id = 1;
  }
  if (!context->subdomain_id) {
    context->subdomain_id = (int*)calloc(2, sizeof(int));
    if (!context->subdomain_id) local_errstop("out of memory for SubdomainID");
    context->owns_subdomain_id = 1;
  }

  context->n_of_species = nspec;
  context->max_fraction = maxfrac;
  context->region_id[0] = context->my_rank;
  context->region_id[1] = -1;
  context->subdomain_id[0] = context->my_rank;
  context->subdomain_id[1] = -1;
  ensure_context_level1_storage(context);

  if (context_is_default(context)) {
    nOfSpecies = nspec;
    maxFraction = maxfrac;
    RegionId[0] = context->region_id[0];
    RegionId[1] = context->region_id[1];
    SubdomainId = context->subdomain_id;
    oh1_sync_default_state();
  }
}

void
oh_context_set_region_weights(struct oh_state *context, const double *weights) {
  context = context_or_default(context);
  oh1_set_region_weights_state(context, weights);
  if (context_is_default(context)) {
    weightedLoadBalancing = context->weighted_load_balancing;
    oh1_sync_default_state();
  }
}

void
oh_context_set_region_weights_n(struct oh_state *context,
                                const double *weights, int weight_count) {
  context = context_or_default(context);
  if (weights && weight_count != context->n_of_nodes)
    local_errstop("oh_context_set_region_weights_n() requires %d weights for "
                  "this context, got %d", context->n_of_nodes,
                  weight_count);
  oh_context_set_region_weights(context, weights);
}

int
oh_context_region_count(const struct oh_state *context) {
  if (!context) context = oh1_state();
  return context->n_of_nodes;
}

int
oh_context_is_level3_configured(const struct oh_state *context) {
  if (!context) context = oh1_state();
  return context->owns_level3_storage;
}

void
oh_context_set_particle_adapter(struct oh_state *context,
                                const oh_particle_adapter *adapter) {
  context = context_or_default(context);
  require_particle_buffer_unbound(context, "oh_context_set_particle_adapter()");
  oh2_set_particle_adapter_state(context, adapter);
  oh3_bind_context_particle_adapter(context);
  if (context_is_default(context)) oh1_sync_default_state();
}

void
oh_context_set_particle_mpi_type(struct oh_state *context, MPI_Datatype type) {
  context = context_or_default(context);
  require_particle_buffer_unbound(context, "oh_context_set_particle_mpi_type()");
  oh2_set_particle_mpi_type_state(context, type);
  oh3_bind_context_particle_adapter(context);
  if (context_is_default(context)) oh1_sync_default_state();
}

void *
oh_context_bind_particles(struct oh_state *context, void *particles,
                          int maxlocalp, int ownership) {
  void *bound;
  context = context_or_default(context);
  if (!context_is_default(context) && context->n_of_species<=0)
    local_errstop("particle binding requires configured particles");
  free_context_level2_storage(context);
  bound = oh2_bind_particle_buffer_state(context, particles, maxlocalp,
                                         ownership);
  ensure_context_level2_storage(context, maxlocalp);
  if (context_is_default(context)) oh1_sync_default_state();
  return bound;
}

void
oh_context_unbind_particles(struct oh_state *context) {
  context = context_or_default(context);
  free_context_level2_storage(context);
  oh2_unbind_particle_buffer_state(context);
  if (context_is_default(context)) oh1_sync_default_state();
}

int *
oh_context_bind_region_ids(struct oh_state *context, int *sdid,
                           int ownership) {
  int previous[2];

  context = context_or_default(context);
  if (!storage_ownership_is_valid(ownership))
    local_errstop("invalid region id ownership flag");
  if (ownership==OH_PARTICLES_BORROWED && !sdid)
    local_errstop("borrowed region id binding requires a non-NULL array");
  if (ownership==OH_PARTICLES_OWNED && sdid)
    local_errstop("owned region id binding requires a NULL array");

  previous[0] = context->region_id ? context->region_id[0] : context->my_rank;
  previous[1] = context->region_id ? context->region_id[1] : -1;
  free_context_region_id_storage(context);

  if (context_is_default(context)) {
    if (ownership==OH_PARTICLES_OWNED) sdid = RegionId;
    sdid[0] = previous[0];
    sdid[1] = previous[1];
    RegionId[0] = sdid[0];
    RegionId[1] = sdid[1];
    SubdomainId = sdid;
    context->region_id = RegionId;
    context->subdomain_id = SubdomainId;
    context->owns_region_id = 0;
    context->owns_subdomain_id = 0;
    return sdid;
  }

  if (ownership==OH_PARTICLES_OWNED && !sdid)
    sdid = (int*)mem_alloc(sizeof(int), 2, "SubdomainID");

  sdid[0] = previous[0];
  sdid[1] = previous[1];
  context->region_id = sdid;
  context->subdomain_id = sdid;
  context->owns_region_id = ownership==OH_PARTICLES_OWNED;
  context->owns_subdomain_id = 0;

  if (context_is_default(context)) {
    RegionId[0] = sdid[0];
    RegionId[1] = sdid[1];
    SubdomainId = sdid;
  }
  return sdid;
}

void
oh_context_unbind_region_ids(struct oh_state *context) {
  int previous[2];
  int *owned_ids;

  context = context_or_default(context);
  previous[0] = context->region_id ? context->region_id[0] : context->my_rank;
  previous[1] = context->region_id ? context->region_id[1] : -1;
  free_context_region_id_storage(context);

  if (context_is_default(context)) {
    RegionId[0] = previous[0];
    RegionId[1] = previous[1];
    SubdomainId = RegionId;
    context->region_id = RegionId;
    context->subdomain_id = SubdomainId;
    context->owns_region_id = 0;
    context->owns_subdomain_id = 0;
    return;
  }

  owned_ids = (int*)mem_alloc(sizeof(int), 2, "SubdomainID");
  owned_ids[0] = previous[0];
  owned_ids[1] = previous[1];
  context->region_id = owned_ids;
  context->subdomain_id = owned_ids;
  context->owns_region_id = 1;
  context->owns_subdomain_id = 0;

  if (context_is_default(context)) {
    RegionId[0] = owned_ids[0];
    RegionId[1] = owned_ids[1];
    SubdomainId = owned_ids;
  }
}

void
oh_context_get_region_ids(struct oh_state *context, int sdid[2]) {
  context = context_or_default(context);
  if (!sdid) local_errstop("region id getter requires a non-NULL array");
  sdid[0] = context->region_id ? context->region_id[0] : context->my_rank;
  sdid[1] = context->region_id ? context->region_id[1] : -1;
}

void
oh_context_bind_particle_accounting_state(struct oh_state *state,
                                          int **nphgram, int **totalp,
                                          int **pbase, int ownership) {
  int ns, nn;
  int i;

  state = context_or_default(state);
  if (!storage_ownership_is_valid(ownership))
    local_errstop("invalid particle accounting ownership flag");
  if (state->n_of_species<=0 || state->n_of_nodes<=0)
    local_errstop("particle accounting binding requires initialized context");
  if (!nphgram || !totalp)
    local_errstop("particle accounting binding requires nphgram and totalp");
  if (ownership==OH_PARTICLES_BORROWED && (!*nphgram || !*totalp))
    local_errstop("borrowed particle accounting requires non-NULL arrays");
  if (ownership==OH_PARTICLES_OWNED && (*nphgram || *totalp))
    local_errstop("owned particle accounting requires NULL nphgram/totalp");
  if (pbase && ownership==OH_PARTICLES_BORROWED && !*pbase)
    local_errstop("borrowed particle accounting requires non-NULL pbase");
  if (pbase && ownership==OH_PARTICLES_OWNED && *pbase)
    local_errstop("owned particle accounting requires NULL pbase");

  ns = state->n_of_species;
  nn = state->n_of_nodes;
  oh_context_validate_species_node_capacity(
    nn, ns, "oh_context_bind_particle_accounting()");

  if (state->n_of_particles_local_ownership==OH_PARTICLES_OWNED &&
      state->n_of_particles_local)
    free(state->n_of_particles_local);
  if (state->total_particles_next_ownership==OH_PARTICLES_OWNED &&
      state->total_particles_next)
    free(state->total_particles_next);
  if (state->particle_base_bound &&
      state->particle_base_ownership==OH_PARTICLES_OWNED &&
      state->secondary_base)
    free(state->secondary_base - 1);
  if (state->total_particles &&
      state->total_particles != state->total_particles_next)
    free(state->total_particles);

  if (ownership==OH_PARTICLES_OWNED) {
    *nphgram = (int*)mem_alloc(sizeof(int), 2*ns*nn, "NOfPLocal");
    *totalp = (int*)mem_alloc(sizeof(int), 2*ns, "TotalPNext");
    if (pbase) *pbase = (int*)mem_alloc(sizeof(int), 3, "ParticleBase");
    for (i=0; i<2*ns*nn; i++) (*nphgram)[i] = 0;
    for (i=0; i<2*ns; i++) (*totalp)[i] = 0;
    if (pbase) (*pbase)[0] = (*pbase)[1] = (*pbase)[2] = 0;
  }

  if (context_is_default(state)) {
    NOfPLocal = *nphgram;
    TotalPNext = *totalp;
    TotalP = NULL;
  }
  state->n_of_particles_local = *nphgram;
  state->total_particles_next = *totalp;
  state->total_particles = NULL;
  state->particle_accounting_bound = 1;
  state->n_of_particles_local_ownership = ownership;
  state->total_particles_next_ownership = ownership;

  if (pbase) {
    if (context_is_default(state)) {
      secondaryBase = *pbase + 1;
      totalLocalParticles = *pbase + 2;
    }
    state->secondary_base = *pbase + 1;
    state->total_local_particles = *pbase + 2;
    state->particle_base_bound = 1;
    state->particle_base_ownership = ownership;
  }
}

void
oh_context_unbind_particle_accounting_state(struct oh_state *state) {
  state = context_or_default(state);

  if (state->n_of_particles_local_ownership==OH_PARTICLES_OWNED &&
      state->n_of_particles_local)
    free(state->n_of_particles_local);
  if (state->total_particles_next_ownership==OH_PARTICLES_OWNED &&
      state->total_particles_next)
    free(state->total_particles_next);
  if (state->particle_base_bound &&
      state->particle_base_ownership==OH_PARTICLES_OWNED &&
      state->secondary_base)
    free(state->secondary_base - 1);
  if (state->total_particles &&
      state->total_particles != state->total_particles_next)
    free(state->total_particles);

  if (context_is_default(state)) {
    NOfPLocal = NULL;
    TotalP = NULL;
    TotalPNext = NULL;
    secondaryBase = NULL;
    totalLocalParticles = NULL;
  }
  state->n_of_particles_local = NULL;
  state->total_particles = NULL;
  state->total_particles_next = NULL;
  state->particle_accounting_bound = 0;
  state->n_of_particles_local_ownership = OH_PARTICLES_BORROWED;
  state->total_particles_next_ownership = OH_PARTICLES_BORROWED;
  state->secondary_base = NULL;
  state->total_local_particles = NULL;
  state->particle_base_bound = 0;
  state->particle_base_ownership = OH_PARTICLES_BORROWED;
}

void
oh_context_bind_particle_accounting(struct oh_state *context, int **nphgram,
                                    int **totalp, int **pbase,
                                    int ownership) {
  context = context_or_default(context);
  if (!pbase)
    local_errstop("particle accounting binding requires a pbase slot");
  oh_context_bind_particle_accounting_state(context, nphgram, totalp, pbase,
                                            ownership);
  if (context_is_default(context)) oh1_sync_default_state();
}

void
oh_context_unbind_particle_accounting(struct oh_state *context) {
  context = context_or_default(context);
  oh_context_unbind_particle_accounting_state(context);
  if (context_is_default(context)) oh1_sync_default_state();
}

int
oh_context_max_local_particles_for_capacity(
    struct oh_state *context, long long global_particle_limit,
    int capacity_percent, int min_margin) {
  long long per_rank;
  long long margin;
  long long capacity;
  int nn;

  context = context_or_default(context);
  nn = context->n_of_nodes;
  if (nn<=0)
    local_errstop("capacity calculation requires an initialized context");
  if (global_particle_limit<=0)
    local_errstop("global particle limit should be greater than 0");
  if (capacity_percent<0)
    local_errstop("capacity headroom percent should be non-negative");
  if (min_margin<0)
    local_errstop("capacity minimum margin should be non-negative");

  per_rank = (global_particle_limit - 1) / nn + 1;
  if (capacity_percent>0 &&
      per_rank > (LLONG_MAX - 99) / (long long)capacity_percent)
    mem_alloc_error("Particles", 0);
  margin = capacity_percent==0
             ? 0
             : (per_rank * (long long)capacity_percent - 1) / 100 + 1;
  if (margin < min_margin) margin = min_margin;
  if (margin>INT_MAX || per_rank>(long long)INT_MAX-margin)
    mem_alloc_error("Particles", 0);
  capacity = per_rank + margin;
  return (int)capacity;
}

void
oh_context_configure_level3(struct oh_state *context, const int *pcoord,
                            const int *sdoms, const int *scoord, int nbound,
                            const int *bcond, const int *bounds,
                            const int *ftypes, const int *cfields,
                            const int *ctypes, int *fsizes) {
  context = context_or_default(context);
  oh3_configure_context_state(context, pcoord, sdoms, scoord, nbound, bcond,
                              bounds, ftypes, cfields, ctypes, fsizes);
  if (context_is_default(context)) oh1_sync_default_state();
}

int
oh_context_transbound1(struct oh_state *context, int currmode, int stats) {
  int ret;
  context = context_or_default(context);
  ret = transbound1_state(context, currmode, stats, 1);
  if (context_is_default(context)) oh1_sync_default_state();
  return ret;
}

int
oh_context_transbound2(struct oh_state *context, int currmode, int stats) {
  int ret;
  context = context_or_default(context);
  ret = transbound2_state(context, currmode, stats, 2);
  if (context_is_default(context)) oh1_sync_default_state();
  return ret;
}

int
oh_context_transbound3(struct oh_state *context, int currmode, int stats) {
  int ret;
  context = context_or_default(context);
  ret = oh3_transbound_state(context, currmode, stats);
  if (context_is_default(context)) oh1_sync_default_state();
  return ret;
}

void
oh_context_broadcast(struct oh_state *context, void *pbuf, void *sbuf,
                     int pcount, int scount, MPI_Datatype ptype,
                     MPI_Datatype stype) {
  context = context_or_default(context);
  oh1_broadcast_state(context, pbuf, sbuf, pcount, scount, ptype, stype);
}

void
oh_context_all_reduce(struct oh_state *context, void *pbuf, void *sbuf,
                           int pcount, int scount, MPI_Datatype ptype,
                           MPI_Datatype stype, MPI_Op pop, MPI_Op sop) {
  context = context_or_default(context);
  oh1_all_reduce_state(context, pbuf, sbuf, pcount, scount, ptype, stype,
                       pop, sop);
}

void
oh_context_reduce(struct oh_state *context, void *pbuf, void *sbuf,
                  int pcount, int scount, MPI_Datatype ptype,
                  MPI_Datatype stype, MPI_Op pop, MPI_Op sop) {
  context = context_or_default(context);
  oh1_reduce_state(context, pbuf, sbuf, pcount, scount, ptype, stype,
                   pop, sop);
}

void
oh_context_set_total_particles(struct oh_state *context) {
  context = context_or_default(context);
  oh2_set_total_particles_state(context);
  if (context_is_default(context)) {
    TotalP = context->total_particles;
    primaryParts = context->primary_parts;
    totalParts = context->total_parts;
    oh1_sync_default_state();
  }
}

void
oh_context_inject_particle(struct oh_state *context, void *part) {
  context = context_or_default(context);
  (void)oh2_inject_particle_state(context, part);
  if (context_is_default(context)) oh1_sync_default_state();
}

void *
oh_context_inject_particle_get(struct oh_state *context, void *part) {
  void *copy;
  context = context_or_default(context);
  copy = oh2_inject_particle_state(context, part);
  if (context_is_default(context)) oh1_sync_default_state();
  return copy;
}

void
oh_context_remap_injected_particle(struct oh_state *context, void *part) {
  context = context_or_default(context);
  oh2_remap_injected_particle_state(context, part);
  if (context_is_default(context)) oh1_sync_default_state();
}

void
oh_context_remove_injected_particle(struct oh_state *context, void *part) {
  context = context_or_default(context);
  oh2_remove_injected_particle_state(context, part);
  if (context_is_default(context)) oh1_sync_default_state();
}

void
oh_context_grid_size(struct oh_state *context, double *size) {
  context = context_or_default(context);
  if (!size) local_errstop("oh_context_grid_size() requires a size array");
  oh3_grid_size_state(context, size);
  if (context_is_default(context)) oh1_sync_default_state();
}

int
oh_context_map_particle_to_neighbor(struct oh_state *context, double *x,
                                    double *y, double *z, int ps) {
  context = context_or_default(context);
  return oh3_map_particle_to_neighbor_state(context, x, y, z, ps);
}

int
oh_context_map_particle_to_subdomain(struct oh_state *context, double x,
                                     double y, double z) {
  context = context_or_default(context);
  return oh3_map_particle_to_subdomain_state(context, x, y, z);
}

void
oh_context_bcast_field(struct oh_state *context, void *pfld, void *sfld,
                       int ftype) {
  context = context_or_default(context);
  oh3_bcast_field_state(context, pfld, sfld, ftype);
}

void
oh_context_reduce_field(struct oh_state *context, void *pfld, void *sfld,
                        int ftype) {
  context = context_or_default(context);
  oh3_reduce_field_state(context, pfld, sfld, ftype);
}

void
oh_context_allreduce_field(struct oh_state *context, void *pfld, void *sfld,
                           int ftype) {
  context = context_or_default(context);
  oh3_allreduce_field_state(context, pfld, sfld, ftype);
}

void
oh_context_exchange_borders(struct oh_state *context, void *pfld, void *sfld,
                            int ctype, int bcast) {
  context = context_or_default(context);
  oh3_exchange_borders_state(context, pfld, sfld, ctype, bcast);
}

void
oh1_state_(struct oh_state **state) {
  *state = oh1_state();
}
