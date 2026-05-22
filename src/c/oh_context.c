/* File: oh_context.c
   v2 context facade for the current default OhHelp instance.
*/
#include "ohhelp1.h"
#include "ohhelp1_internal.h"
#include "ohhelp2.h"
#include "ohhelp2_internal.h"
#include "ohhelp3.h"
#include "ohhelp3_internal.h"
#include "oh_context_internal.h"
#include "oh_context.h"

struct oh_state OhDefaultState;

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

void
oh1_state_(struct oh_state **state) {
  *state = oh1_state();
}
