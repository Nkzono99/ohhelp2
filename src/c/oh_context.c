/* File: oh_context.c
   v2 context facade for the current default OhHelp instance.
*/
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

static int
storage_ownership_is_valid(int ownership) {
  return ownership==OH_PARTICLES_BORROWED || ownership==OH_PARTICLES_OWNED;
}

static struct oh_state*
default_context_or_stop(struct oh_state *context) {
  if (!context) context = oh1_state();
  if (context != &OhDefaultState)
    local_errstop("only the default oh_context is implemented yet");
  return context;
}

void
oh1_sync_default_state(void) {
  OhDefaultState.comm = MCW;
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
  OhDefaultState.primary_parts = primaryParts;
  OhDefaultState.total_parts = totalParts;
  OhDefaultState.nodes = Nodes;
  OhDefaultState.nodes_next = NodesNext;
  OhDefaultState.node_queue = NodeQueue;
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
  OhDefaultState.n_of_local_particles_limit = nOfLocalPLimit;
  OhDefaultState.particles = Particles;
  OhDefaultState.send_buffer = SendBuf;
  OhDefaultState.recv_buffer_bases = (void**)RecvBufBases;
  OhDefaultState.secondary_base = secondaryBase;
  OhDefaultState.total_local_particles = totalLocalParticles;
  OhDefaultState.send_buffer_disps = SendBufDisps;
  OhDefaultState.recv_buffer_disps = RecvBufDisps;
  OhDefaultState.n_of_injections = nOfInjections;
  OhDefaultState.spec_base = specBase;
  OhDefaultState.particle_mpi_type = T_Particle;
  OhDefaultState.custom_particle_mpi_type = CustomTParticle;
  OhDefaultState.use_custom_particle_mpi_type = useCustomTParticle;
  OhDefaultState.particle_adapter = &ParticleAdapter;
  OhDefaultState.custom_particle_adapter = &CustomParticleAdapter;
  OhDefaultState.use_custom_particle_adapter = useCustomParticleAdapter;
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
oh_context_set_region_weights(struct oh_state *context, const double *weights) {
  context = default_context_or_stop(context);
  oh1_set_region_weights_state(context, weights);
  weightedLoadBalancing = context->weighted_load_balancing;
  oh1_sync_default_state();
}

void
oh_context_set_particle_adapter(struct oh_state *context,
                                const oh_particle_adapter *adapter) {
  context = default_context_or_stop(context);
  oh2_set_particle_adapter_state(context, adapter);
  oh1_sync_default_state();
}

void
oh_context_set_particle_mpi_type(struct oh_state *context, MPI_Datatype type) {
  context = default_context_or_stop(context);
  oh2_set_particle_mpi_type_state(context, type);
  oh1_sync_default_state();
}

void *
oh_context_bind_particles(struct oh_state *context, void *particles,
                          int maxlocalp, int ownership) {
  void *bound;
  context = default_context_or_stop(context);
  bound = oh2_bind_particle_buffer_state(context, particles, maxlocalp,
                                         ownership);
  oh1_sync_default_state();
  return bound;
}

void
oh_context_unbind_particles(struct oh_state *context) {
  context = default_context_or_stop(context);
  oh2_unbind_particle_buffer_state(context);
  oh1_sync_default_state();
}

void
oh_context_bind_particle_accounting_state(struct oh_state *state,
                                          int **nphgram, int **totalp,
                                          int **pbase, int ownership) {
  int ns, nn;
  int i;

  if (!state) state = &OhDefaultState;
  if (state != &OhDefaultState)
    local_errstop("only the default oh_context is implemented yet");
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

  NOfPLocal = *nphgram;
  TotalPNext = *totalp;
  TotalP = NULL;
  state->n_of_particles_local = NOfPLocal;
  state->total_particles_next = TotalPNext;
  state->total_particles = NULL;
  state->particle_accounting_bound = 1;
  state->n_of_particles_local_ownership = ownership;
  state->total_particles_next_ownership = ownership;

  if (pbase) {
    secondaryBase = *pbase + 1;
    totalLocalParticles = *pbase + 2;
    state->secondary_base = secondaryBase;
    state->total_local_particles = totalLocalParticles;
    state->particle_base_bound = 1;
    state->particle_base_ownership = ownership;
  }
}

void
oh_context_unbind_particle_accounting_state(struct oh_state *state) {
  if (!state) state = &OhDefaultState;
  if (state != &OhDefaultState)
    local_errstop("only the default oh_context is implemented yet");

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

  NOfPLocal = NULL;
  TotalP = NULL;
  TotalPNext = NULL;
  secondaryBase = NULL;
  totalLocalParticles = NULL;
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
  context = default_context_or_stop(context);
  oh_context_bind_particle_accounting_state(context, nphgram, totalp, pbase,
                                            ownership);
  oh1_sync_default_state();
}

void
oh_context_unbind_particle_accounting(struct oh_state *context) {
  context = default_context_or_stop(context);
  oh_context_unbind_particle_accounting_state(context);
  oh1_sync_default_state();
}

int
oh_context_transbound1(struct oh_state *context, int currmode, int stats) {
  int ret;
  context = default_context_or_stop(context);
  ret = transbound1_state(context, currmode, stats, 1);
  oh1_sync_default_state();
  return ret;
}

int
oh_context_transbound2(struct oh_state *context, int currmode, int stats) {
  int ret;
  context = default_context_or_stop(context);
  ret = transbound2_state(context, currmode, stats, 2);
  oh1_sync_default_state();
  return ret;
}

int
oh_context_transbound3(struct oh_state *context, int currmode, int stats) {
  int ret;
  context = default_context_or_stop(context);
  ret = oh3_transbound_state(context, currmode, stats);
  oh1_sync_default_state();
  return ret;
}

void
oh_context_broadcast(struct oh_state *context, void *pbuf, void *sbuf,
                     int pcount, int scount, MPI_Datatype ptype,
                     MPI_Datatype stype) {
  context = default_context_or_stop(context);
  oh1_broadcast_state(context, pbuf, sbuf, pcount, scount, ptype, stype);
}

void
oh_context_all_reduce(struct oh_state *context, void *pbuf, void *sbuf,
                      int pcount, int scount, MPI_Datatype ptype,
                      MPI_Datatype stype, MPI_Op pop, MPI_Op sop) {
  context = default_context_or_stop(context);
  oh1_all_reduce_state(context, pbuf, sbuf, pcount, scount, ptype, stype,
                       pop, sop);
}

void
oh_context_reduce(struct oh_state *context, void *pbuf, void *sbuf,
                  int pcount, int scount, MPI_Datatype ptype,
                  MPI_Datatype stype, MPI_Op pop, MPI_Op sop) {
  context = default_context_or_stop(context);
  oh1_reduce_state(context, pbuf, sbuf, pcount, scount, ptype, stype,
                   pop, sop);
}

void
oh_context_set_total_particles(struct oh_state *context) {
  context = default_context_or_stop(context);
  set_total_particles_state(context);
  TotalP = context->total_particles;
  primaryParts = context->primary_parts;
  totalParts = context->total_parts;
  oh1_sync_default_state();
}

void
oh_context_inject_particle(struct oh_state *context, void *part) {
  context = default_context_or_stop(context);
  (void)oh2_inject_particle_state(context, (struct S_particle*)part);
  oh1_sync_default_state();
}

void *
oh_context_inject_particle_get(struct oh_state *context, void *part) {
  void *copy;
  context = default_context_or_stop(context);
  copy = oh2_inject_particle_state(context, (struct S_particle*)part);
  oh1_sync_default_state();
  return copy;
}

void
oh_context_remap_injected_particle(struct oh_state *context, void *part) {
  context = default_context_or_stop(context);
  oh2_remap_injected_particle_state(context, (struct S_particle*)part);
  oh1_sync_default_state();
}

void
oh_context_remove_injected_particle(struct oh_state *context, void *part) {
  context = default_context_or_stop(context);
  oh2_remove_injected_particle_state(context, (struct S_particle*)part);
  oh1_sync_default_state();
}

void
oh_context_grid_size(struct oh_state *context, double *size) {
  context = default_context_or_stop(context);
  oh3_grid_size_state(context, size);
  oh1_sync_default_state();
}

int
oh_context_map_particle_to_neighbor(struct oh_state *context, double *x,
                                    double *y, double *z, int ps) {
  context = default_context_or_stop(context);
  return oh3_map_particle_to_neighbor_state(context, x, y, z, ps);
}

int
oh_context_map_particle_to_subdomain(struct oh_state *context, double x,
                                     double y, double z) {
  context = default_context_or_stop(context);
  return oh3_map_particle_to_subdomain_state(context, x, y, z);
}

void
oh_context_bcast_field(struct oh_state *context, void *pfld, void *sfld,
                       int ftype) {
  context = default_context_or_stop(context);
  oh3_bcast_field_state(context, pfld, sfld, ftype);
}

void
oh_context_reduce_field(struct oh_state *context, void *pfld, void *sfld,
                        int ftype) {
  context = default_context_or_stop(context);
  oh3_reduce_field_state(context, pfld, sfld, ftype);
}

void
oh_context_allreduce_field(struct oh_state *context, void *pfld, void *sfld,
                           int ftype) {
  context = default_context_or_stop(context);
  oh3_allreduce_field_state(context, pfld, sfld, ftype);
}

void
oh_context_exchange_borders(struct oh_state *context, void *pfld, void *sfld,
                            int ctype, int bcast) {
  context = default_context_or_stop(context);
  oh3_exchange_borders_state(context, pfld, sfld, ctype, bcast);
}

void
oh1_state_(struct oh_state **state) {
  *state = oh1_state();
}
