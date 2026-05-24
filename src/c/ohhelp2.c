/* File: ohhelp2.c
   Version 1.1.1 (2015/10/23)
   Copyright (C) 2009-2015  Hiroshi Nakashima <h.nakashima@media.kyoto-u.ac.jp>
                            (ACCMS, Kyoto University)
   This program can be freely used, redistributed and modified for non-
   commercial purpose providing that the copyright notice above remains
   unchanged.
*/
#include <stdlib.h>

#define EXTERN extern
#include "ohhelp1.h"
#include "ohhelp1_internal.h"
#undef  EXTERN
#define EXTERN
#include "ohhelp2_internal.h"
#include "oh_context_internal.h"
#include "oh_particle_buffer.h"

/* Prototypes for private functions. */
static int   try_primary2_state(struct oh_state *state, int currmode,
                                int level, int stats);
static int   try_stable2_state(struct oh_state *state, int currmode,
                               int level, int stats);
static void  rebalance2_state(struct oh_state *state, int currmode, int level,
                              int stats);
static void  init_particle_adapter(void);
static void  allocate_particle_storage(struct oh_state *state,
                                       struct S_particle **pbuf,
                                       int maxlocalp);
static int   particle_buffer_ownership_is_valid(int ownership);
static void  allocate_particle_base(struct oh_state *state, int **pbase);
static void  allocate_level2_work_buffers(struct oh_state *state, int ns,
                                          int nn, int nnns,
                                          int maxlocalp);
int          transbound2_state(struct oh_state *state, int currmode, int stats,
                               int level);
static int   finish_transbound2_state(struct oh_state *state, int ret);
static void  move_to_sendbuf_secondary_state(struct oh_state *state,
                                             int secondary, int stats);
static void  move_to_sendbuf_uw(struct oh_state *state, int ps, int me,
                                int *putmes, int cbase, int *ctp, int nbase,
                                int *ntp, void **rbb);
static void  move_to_sendbuf_dw(struct oh_state *state, int ps, int me,
                                int *putmes, int ctail, int *ctp, int ntail,
                                int *ntp);
static void  move_injected_to_sendbuf(struct oh_state *state);
static void  move_injected_from_sendbuf(struct oh_state *state,
                                        int *injected, int mysd,
                                        void **rbb);
static void  receive_particles(struct oh_state *state,
                               struct S_commlist *rlist, int rlsize,
                               int *req);
static void  send_particles(struct oh_state *state,
                            struct S_commlist *slist, int slsize,
                            int myregion, int parentregion, int *req);
void        *oh2_inject_particle_state(struct oh_state *state, void *part);
void         oh2_remap_injected_particle_state(struct oh_state *state,
                                               void *part);
void         oh2_remove_injected_particle_state(struct oh_state *state,
                                                void *part);
static void  state_update_injected_particle_count(struct oh_state *state,
                                                  struct S_particle *part,
                                                  int delta);
static OH_nid_t state_particle_region(struct oh_state *state,
                                      const struct S_particle *part,
                                      int primary_or_secondary);
static void  state_set_particle_region(struct oh_state *state,
                                       struct S_particle *part,
                                       OH_nid_t region,
                                       int primary_or_secondary);
static void  state_mark_particle_removed(struct oh_state *state,
                                         struct S_particle *part,
                                         int primary_or_secondary);
static int   state_particle_species(struct oh_state *state,
                                    const struct S_particle *part);
static int   state_particle_subdomain(struct oh_state *state,
                                      const struct S_particle *part,
                                      int primary_or_secondary);
static int   state_map_injected_particle_to_subdomain(struct oh_state *state,
                                                      struct S_particle *part);
static int   state_region_subdomain(struct oh_state *state, OH_nid_t region,
                                    int primary_or_secondary);
static int   state_primarize_particle(struct oh_state *state,
                                      struct S_particle *part);
static size_t particle_stride_state(struct oh_state *state);
static struct S_particle *state_particle_at(struct oh_state *state,
                                            void *base,
                                            int index);
static int   state_particle_buffer_index(struct oh_state *state,
                                         const struct S_particle *part);
static void  state_copy_particle(struct oh_state *state,
                                 struct S_particle *dst,
                                 const struct S_particle *src);
static void  state_copy_particles(struct oh_state *state,
                                  struct S_particle *dst,
                                  const struct S_particle *src, int count);
static void  finalize_injected_particles_state(struct oh_state *state);
static int   state_injected_particle_region_kind(struct oh_state *state,
                                                 struct S_particle *part);

void
oh2_init_(int *sdid, int *nspec, int *maxfrac, int *nphgram,
          int *totalp, struct S_particle *pbuf, int *pbase, int *maxlocalp,
          struct S_mycommf *mycomm, int *nbor, int *pcoord,
          int *stats, int *repiter, int *verbose) {
  specBase = 1;
  init2(&sdid, *nspec, *maxfrac, &nphgram, &totalp,
        &pbuf, &pbase, *maxlocalp, NULL, mycomm, &nbor, pcoord,
        *stats, *repiter, *verbose);
}
void
oh2_set_particle_mpi_type_(int *type) {
  oh2_set_particle_mpi_type(MPI_Type_f2c(*type));
}
void
oh2_set_particle_mpi_type(MPI_Datatype type) {
  oh2_set_particle_mpi_type_state(&OhDefaultState, type);
}
void
oh2_set_particle_mpi_type_state(struct oh_state *state, MPI_Datatype type) {
  if (!state) state = &OhDefaultState;
  if (state != &OhDefaultState) {
    int err;

    if (state->owns_particle_mpi_type &&
        state->owned_particle_adapter.mpi_type != MPI_DATATYPE_NULL)
      MPI_Type_free(&state->owned_particle_adapter.mpi_type);
    if (type == MPI_DATATYPE_NULL) {
      err = oh_particle_adapter_make_byte_type(sizeof(struct S_particle),
                                               &type);
      if (err != MPI_SUCCESS)
        local_errstop("failed to create default particle MPI datatype");
      state->owns_particle_mpi_type = 1;
    } else {
      state->owns_particle_mpi_type = 0;
    }
    state->owned_particle_adapter = oh_default_particle_adapter(type);
    state->particle_adapter = &state->owned_particle_adapter;
    state->particle_mpi_type = type;
    state->custom_particle_mpi_type = MPI_DATATYPE_NULL;
    state->use_custom_particle_mpi_type = 0;
    state->use_custom_particle_adapter = 0;
    return;
  }
  if (type == MPI_DATATYPE_NULL) {
    state->use_custom_particle_mpi_type = 0;
    state->custom_particle_mpi_type = MPI_DATATYPE_NULL;
  } else {
    state->custom_particle_mpi_type = type;
    state->use_custom_particle_mpi_type = 1;
  }
  CustomTParticle = state->custom_particle_mpi_type;
  useCustomTParticle = state->use_custom_particle_mpi_type;
}
void
oh2_set_particle_adapter(const oh_particle_adapter *adapter) {
  oh2_set_particle_adapter_state(&OhDefaultState, adapter);
}
void
oh2_set_particle_adapter_state(struct oh_state *state,
                               const oh_particle_adapter *adapter) {
  if (!state) state = &OhDefaultState;
  if (state != &OhDefaultState) {
    int err;

    if (state->owns_particle_mpi_type &&
        state->owned_particle_adapter.mpi_type != MPI_DATATYPE_NULL) {
      MPI_Type_free(&state->owned_particle_adapter.mpi_type);
      state->owned_particle_adapter.mpi_type = MPI_DATATYPE_NULL;
      state->owns_particle_mpi_type = 0;
    }
    if (!adapter) {
      MPI_Datatype type = MPI_DATATYPE_NULL;
      err = oh_particle_adapter_make_byte_type(sizeof(struct S_particle),
                                               &type);
      if (err != MPI_SUCCESS)
        local_errstop("failed to create default particle MPI datatype");
      state->owned_particle_adapter = oh_default_particle_adapter(type);
      state->particle_adapter = &state->owned_particle_adapter;
      state->particle_mpi_type = type;
      state->custom_particle_mpi_type = MPI_DATATYPE_NULL;
      state->use_custom_particle_mpi_type = 0;
      state->use_custom_particle_adapter = 0;
      state->owns_particle_mpi_type = 1;
      return;
    }
    if (!oh_particle_adapter_validate(adapter))
      local_errstop("invalid oh_particle_adapter");
    state->owned_custom_particle_adapter = *adapter;
    state->custom_particle_adapter = &state->owned_custom_particle_adapter;
    state->particle_adapter = &state->owned_custom_particle_adapter;
    state->particle_mpi_type = adapter->mpi_type;
    state->custom_particle_mpi_type = adapter->mpi_type;
    state->use_custom_particle_mpi_type = 1;
    state->use_custom_particle_adapter = 1;
    return;
  }
  if (!adapter) {
    state->use_custom_particle_adapter = 0;
    state->custom_particle_mpi_type = MPI_DATATYPE_NULL;
    state->use_custom_particle_mpi_type = 0;
    useCustomParticleAdapter = state->use_custom_particle_adapter;
    CustomTParticle = state->custom_particle_mpi_type;
    useCustomTParticle = state->use_custom_particle_mpi_type;
    return;
  }
  if (!oh_particle_adapter_validate(adapter))
    local_errstop("invalid oh_particle_adapter");
  CustomParticleAdapter = *adapter;
  state->custom_particle_adapter = &CustomParticleAdapter;
  state->use_custom_particle_adapter = 1;
  state->custom_particle_mpi_type = adapter->mpi_type;
  state->use_custom_particle_mpi_type = 1;
  useCustomParticleAdapter = state->use_custom_particle_adapter;
  CustomTParticle = state->custom_particle_mpi_type;
  useCustomTParticle = state->use_custom_particle_mpi_type;
}
void
oh2_init(int **sdid, int nspec, int maxfrac, int **nphgram,
         int **totalp, void **pbuf, int **pbase, int maxlocalp,
         void *mycomm, int **nbor, int *pcoord,
         int stats, int repiter, int verbose) {
  specBase = 0;
  init2(sdid, nspec, maxfrac, nphgram, totalp,
        (struct S_particle**)pbuf, pbase, maxlocalp,
        (struct S_mycommc*)mycomm, NULL, nbor, pcoord, stats, repiter,
        verbose);
}
void
init2(int **sdid, int nspec, int maxfrac, int **nphgram,
      int **totalp, struct S_particle **pbuf, int **pbase, int maxlocalp,
      struct S_mycommc *mycommc, struct S_mycommf *mycommf,
      int **nbor, int *pcoord, int stats, int repiter, int verbose) {
  struct oh_state *state;
  int ns, nn, nnns;

  init1(sdid, nspec, maxfrac, nphgram, totalp, NULL, NULL,
        mycommc, mycommf, nbor, pcoord, stats, repiter, verbose);

  init_particle_adapter();
  oh1_sync_default_state();
  state = &OhDefaultState;

  ns = state->n_of_species;  nn = state->n_of_nodes;  nnns = nn * ns;
  nOfLocalPLimit = totalParts = maxlocalp;
  state->n_of_local_particles_limit = nOfLocalPLimit;
  state->total_parts = totalParts;
  allocate_particle_storage(state, pbuf, maxlocalp);
  allocate_particle_base(state, pbase);
  allocate_level2_work_buffers(state, ns, nn, nnns, maxlocalp);
  oh1_sync_default_state();
}
static void
init_particle_adapter(void) {
  if (useCustomParticleAdapter) {
    ParticleAdapter = CustomParticleAdapter;
    T_Particle = ParticleAdapter.mpi_type;
  } else if (useCustomTParticle) {
    T_Particle = CustomTParticle;
    ParticleAdapter = oh_default_particle_adapter(T_Particle);
  } else {
    if (oh_particle_adapter_make_byte_type(sizeof(struct S_particle),
                                           &T_Particle) != MPI_SUCCESS)
      local_errstop("failed to create default particle MPI datatype");
    ParticleAdapter = oh_default_particle_adapter(T_Particle);
  }
  if (!useCustomParticleAdapter)
    ParticleAdapter.species_base = specBase;
  if (!oh_particle_adapter_validate(&ParticleAdapter))
    local_errstop("particle MPI datatype extent must match particle stride");
}
static void
allocate_particle_storage(struct oh_state *state, struct S_particle **pbuf,
                          int maxlocalp) {
  const int ownership = *pbuf ? OH_PARTICLES_BORROWED : OH_PARTICLES_OWNED;

  *pbuf = (struct S_particle*)oh2_bind_particle_buffer_state(
    state, *pbuf, maxlocalp, ownership);
}
void *
oh2_bind_particle_buffer_state(struct oh_state *state, void *particles,
                               int maxlocalp, int ownership) {
  if (!state) state = &OhDefaultState;
  if (maxlocalp<0)
    local_errstop("negative maxlocalp for particle buffer binding");
  if (!particle_buffer_ownership_is_valid(ownership))
    local_errstop("invalid particle buffer ownership flag");
  if (ownership==OH_PARTICLES_BORROWED && !particles && maxlocalp>0)
    local_errstop("borrowed particle buffer binding requires a non-NULL buffer");
  if (ownership==OH_PARTICLES_OWNED && particles)
    local_errstop("owned particle buffer binding requires a NULL buffer");
  if (state->particle_buffer_bound &&
      state->particle_buffer_ownership==OH_PARTICLES_OWNED &&
      state->particles)
    free(state->particles);
  if (ownership==OH_PARTICLES_OWNED && maxlocalp>0)
    particles = mem_alloc(particle_stride_state(state), maxlocalp,
                          "Particles");

  if (state == &OhDefaultState) {
    Particles = (struct S_particle*)particles;
    nOfLocalPLimit = maxlocalp;
    totalParts = maxlocalp;
  }
  state->particles = particles;
  state->n_of_local_particles_limit = maxlocalp;
  state->total_parts = maxlocalp;
  state->particle_buffer_bound = particles!=NULL || maxlocalp==0;
  state->particle_buffer_ownership = ownership;
  return particles;
}
void
oh2_unbind_particle_buffer_state(struct oh_state *state) {
  if (!state) state = &OhDefaultState;
  if (state->particle_buffer_ownership==OH_PARTICLES_OWNED &&
      state->particles)
    free(state->particles);
  if (state == &OhDefaultState) {
    Particles = NULL;
    nOfLocalPLimit = 0;
    totalParts = 0;
  }
  state->particles = NULL;
  state->n_of_local_particles_limit = 0;
  state->total_parts = 0;
  state->particle_buffer_bound = 0;
  state->particle_buffer_ownership = OH_PARTICLES_BORROWED;
}
static int
particle_buffer_ownership_is_valid(int ownership) {
  return ownership==OH_PARTICLES_BORROWED || ownership==OH_PARTICLES_OWNED;
}
static void
allocate_particle_base(struct oh_state *state, int **pbase) {
  const int ownership = *pbase ? OH_PARTICLES_BORROWED : OH_PARTICLES_OWNED;

  if (!*pbase)  *pbase = (int*)mem_alloc(sizeof(int), 3, "ParticleBase");
  (*pbase)[0] = (*pbase)[1] = (*pbase)[2] = 0;
  secondaryBase = *pbase + 1;  totalLocalParticles = *pbase + 2;
  state->secondary_base = secondaryBase;
  state->total_local_particles = totalLocalParticles;
  state->particle_base_bound = 1;
  state->particle_base_ownership = ownership;
}
static void
allocate_level2_work_buffers(struct oh_state *state, int ns, int nn, int nnns,
                             int maxlocalp) {
#ifndef OH_POS_AWARE
  SendBuf = (struct S_particle*)mem_alloc(particle_stride_state(state),
                                          maxlocalp, "SendBuf");
  state->send_buffer = SendBuf;
#endif
  RecvBufBases = (struct S_particle**)mem_alloc(sizeof(struct S_particle*),
                                                2*ns+1, "RecvBufBases");
  SendBufDisps = (int*)mem_alloc(sizeof(int),  nnns, "SendBufDisps");
  RecvBufDisps = (int*)mem_alloc(sizeof(int),  nn, "RecvBufDisps");
  nOfInjections = 0;
  state->recv_buffer_bases = (void**)RecvBufBases;
  state->send_buffer_disps = SendBufDisps;
  state->recv_buffer_disps = RecvBufDisps;
  state->n_of_injections = nOfInjections;

  Requests = (MPI_Request*)mem_alloc(sizeof(MPI_Request),
                                     nnns*4+OH_NEIGHBORS*2, "Requests");
  Statuses = (MPI_Status*) mem_alloc(sizeof(MPI_Status),
                                     nnns*4+OH_NEIGHBORS*2, "Statuses");
  state->requests = Requests;
  state->statuses = Statuses;
}
int
oh2_transbound_(int *currmode, int *stats) {
  return(transbound2_state(oh1_state(), *currmode, *stats, 2));
}
int
oh2_transbound(int currmode, int stats) {
  return transbound2_state(oh1_state(), currmode, stats, 2);
}
int
transbound2_state(struct oh_state *state, int currmode, int stats, int level) {
  int ret=MODE_NORM_SEC;

  if (!state->particle_buffer_bound)
    local_errstop("particle buffer is not bound");
  if (!state->particle_accounting_bound || !state->particle_base_bound)
    local_errstop("particle accounting is not bound");

  stats = stats && state->stats_mode;
  currmode = transbound1_state(state, currmode, stats, level);

  if (try_primary2_state(state, currmode, level, stats)) {
    ret = MODE_NORM_PRI;
  } else if (!Mode_PS(currmode) ||
           !try_stable2_state(state, currmode, level, stats)) {
    rebalance2_state(state, currmode, level, stats);  ret = MODE_REB_SEC;
  }
  return finish_transbound2_state(state, ret);
}
static int
finish_transbound2_state(struct oh_state *state, int ret) {
  int ns=state->n_of_species, nn=state->n_of_nodes, nnns2=2*nn*ns;
  int *n_of_particles_local=state->n_of_particles_local;
  int *total_particles=state->total_particles;
  int *total_particles_next=state->total_particles_next;
  int *injected_particles=state->injected_particles;
  int i, s, tp;

  for (i=0; i<nnns2; i++) n_of_particles_local[i] = 0;
  for (s=0,tp=0; s<ns*2; s++) {
    total_particles[s] = total_particles_next[s];
    tp += total_particles_next[s];
  }
  for (s=0; s<ns*2; s++)  injected_particles[s] = 0;
  state->total_parts = tp;
  *state->total_local_particles = tp;
  state->n_of_injections = 0;
  state->curr_mode = ret;
  if (oh_context_is_default_state(state)) {
    totalParts = tp;
    nOfInjections = 0;
    currMode = ret;
  }
  return ret;
}
static int
try_primary2_state(struct oh_state *state, int currmode, int level, int stats) {

  if (!try_primary1_state(state, currmode, level, stats))  return(FALSE);
  move_to_sendbuf_primary_state(state, Mode_PS(currmode), stats);
  exchange_primary_particles_state(state, currmode, stats);
  state->primary_parts = state->total_particles_global[state->my_rank];
  if (oh_context_is_default_state(state)) primaryParts = state->primary_parts;
  *state->secondary_base = state->primary_parts;
  return(TRUE);
}
void
exchange_primary_particles_state(struct oh_state *state, int currmode,
                                 int stats) {
  int i, s, nn=state->n_of_nodes, ns=state->n_of_species, nnns=nn*ns;
  int me=state->my_rank;
  int *np, *rnp, *sbd;
  MPI_Comm comm=state->comm;
  MPI_Datatype particle_type=state->particle_mpi_type;
  struct S_particle *sendbuf=state->send_buffer;
  void **recvbuf_bases=state->recv_buffer_bases;
  int *recvbuf_disps=state->recv_buffer_disps;

  if (stats) oh1_stats_time(STATS_TB_COMM, 0);
  np = state->n_of_particles_local;     /* &NOfPLocal[0][0][0] */
  rnp = state->n_of_primaries;          /* &NOfPrimaries[0][0][0] */
  sbd = state->send_buffer_disps;       /* SendBufDisps[0][0] */
  if (currmode==MODE_NORM_PRI) {
    for (s=0; s<ns; s++,np+=nn,rnp+=nn,sbd+=nn) {
                                        /* np=&NOfPLocal[0][s][0] */
                                        /* rnp=&NOfPrimaries[0][s][0] */
                                        /* sbd=&SendBufDisps[s][0] */
      struct S_particle *rb;
      rb = recvbuf_bases[s];            /* RecvBufBases[0][s] */
      for (i=0; i<OH_NEIGHBORS; i++) {
        int dst=state->dst_neighbors[i];
        int src=state->src_neighbors[i];
        int rc;
        MPI_Status st;
        if (dst==me) continue;
        if (src>=0) {
          rc = rnp[src];                /* NOfPrimaries[0][s][src] */
          if (dst>=0)
            MPI_Sendrecv(state_particle_at(state, sendbuf, sbd[dst]), np[dst],
                         particle_type, dst, 0,
                         rb, rc, particle_type, src, 0, comm, &st);
          else
            MPI_Recv(rb, rc, particle_type, src, 0, comm, &st);
          rb = state_particle_at(state, rb, rc);
        } else if (dst>=0)
          MPI_Send(state_particle_at(state, sendbuf, sbd[dst]), np[dst],
                   particle_type, dst, 0, comm);
      }
    }
  } else {
    for (s=0; s<ns; s++,np+=nn,rnp+=nn,sbd+=nn) {
                                        /* np=&NOfPLocal[0][s][0] */
                                        /* sbd=&SendBufDisps[s][0] */
                                        /* rnp=&NOfPrimaries[0][s][0] */
      int rdisp=0;
      rnp[me] = 0;                      /* &NOfPrimaries[0][s][me] */
      for (i=0; i<nn; i++) {
        int rc = rnp[i] + rnp[i+nnns];
                        /* NOfPrimaries[0][s][i]+ NofPrimaries[1][s][i] */
        state->temp_array[i] = rc;
        recvbuf_disps[i] = rdisp;
        rdisp += rc;
        np[i] += np[i+nnns];            /* += NOfPLocal[1][s][i] */
      }
      MPI_Alltoallv(sendbuf, np, sbd, particle_type,
                    recvbuf_bases[s], state->temp_array, recvbuf_disps,
                    particle_type, comm);
    }
  }
}
static int
try_stable2_state(struct oh_state *state, int currmode, int level, int stats) {

  if (!try_stable1_state(state, currmode, level, stats)) return(FALSE);
  exchange_particles_state(state, state->comm_list+state->sl_head_tail[1],
                           state->sec_sl_head_tail[0],
                           state->nodes[state->my_rank].parentid,
                           currmode==MODE_NORM_SEC, currmode, stats);
  return(TRUE);
}
static void
rebalance2_state(struct oh_state *state, int currmode, int level, int stats) {
  int me, ns, s, oldp, newp;

  rebalance1_state(state, currmode, level, stats);
  me=state->my_rank;  ns=state->n_of_species;
  oldp = state->nodes_next[me].parentid;
  newp = state->nodes[me].parentid;
  if (state->n_of_injections && oldp>=0 && oldp!=newp)
    for (s=0; s<ns; s++)  state->injected_particles[ns+s] = 0;
  if (Mode_Is_Norm(currmode))
    exchange_particles_state(state, state->sec_recv_list, *state->sec_rl_size,
                             Mode_PS(currmode) ? oldp : -1,
                             1, currmode, stats);
  else
    exchange_particles_state(state, state->sec_recv_list, *state->sec_rl_size,
                             -1, 0, currmode, stats);
}
void
move_to_sendbuf_primary_state(struct oh_state *state, int secondary,
                              int stats) {
  int me=state->my_rank, ns=state->n_of_species, nn=state->n_of_nodes;
  int nnns=nn*ns;
  int *n_of_particles_local=state->n_of_particles_local;
  int *n_of_primaries=state->n_of_primaries;
  int *total_particles=state->total_particles;
  int *total_particles_next=state->total_particles_next;
  int s, i, j, *pp;

  if (stats) oh1_stats_time(STATS_TB_MOVE, 0);
  for (s=0,i=me,pp=n_of_primaries; s<ns; s++,i+=nn,pp+=nn) {
    int t = 0;
    n_of_particles_local[i] = 0;                /* NOfPLocal[0][s][me] */
    for (j=0; j<nn; j++) t += pp[j] + pp[j+nnns];
                            /* NOfPrimaries[0][s][j] + NOfPrimaries[1][s][j] */
    total_particles_next[s] = t;                /* TotalPNext[0][s] */
    total_particles_next[ns+s] = 0;             /* TotalPNext[1][s] */
  }
  set_sendbuf_disps_state(state, secondary, -1);
  if (state->n_of_injections)  move_injected_to_sendbuf(state);

  move_to_sendbuf_uw(state, 0, me, n_of_particles_local+me, 0,
                     total_particles, 0, total_particles_next,
                     state->recv_buffer_bases);

  if (secondary) move_to_sendbuf_uw(state, 1, -1, NULL, state->primary_parts,
                                    total_particles+ns, 0,
                                    total_particles_next+ns,
                                    state->recv_buffer_bases+ns);

  move_to_sendbuf_dw(state, 0, me, n_of_particles_local+me,
                     state->primary_parts, total_particles,
                     state->total_particles_global[me], total_particles_next);

  set_sendbuf_disps_state(state, secondary, -1);
  if (state->n_of_injections)
    move_injected_from_sendbuf(state, state->injected_particles, me,
                               state->recv_buffer_bases);
}
static void
move_to_sendbuf_secondary_state(struct oh_state *state, int secondary,
                                int stats) {
  int me=state->my_rank, ns=state->n_of_species, ns2=ns<<1;
  int nn=state->n_of_nodes;
  struct S_node *node = state->nodes+me;
  int put[2]={-node->get.prime, -node->get.sec}, pnext[2];
  int sec=node->parentid;
  int nnns=nn*ns;
  int *n_of_particles_local=state->n_of_particles_local;
  int *total_particles=state->total_particles;
  int *total_particles_next=state->total_particles_next;
  int *injected_particles=state->injected_particles;
  int *mynp[2]={n_of_particles_local+me, /* &NOfPLocal[0][0][me] */
                sec<0 ? NULL : n_of_particles_local+nnns+sec};
                                        /* &NOfPLocal[1][0][sec] */
  int *mynps;
  int ps, s, i;

  if (stats) oh1_stats_time(STATS_TB_MOVE, 1);
  for (ps=0,i=0; ps<2; ps++) {
    int putme=put[ps], npnext=0;
    mynps=mynp[ps];
    if (mynps==NULL) {
      pnext[ps] = 0;  break;
    }
    if (putme<0) putme = 0;
    for (s=0; s<ns; s++,i++,mynps+=nn) {        /* i=ps*ns+s */
      int stay=*mynps;                          /* NofPLocal[ps][s][me/sec] */
      int tpni=total_particles_next[i];         /* TotalPNext[ps][s] */
      int inj=injected_particles[i];            /* InjectedParticles[ps][s] */
      if (putme<stay) {
        total_particles_next[i] = tpni = tpni + stay - putme;
        stay = putme;
        putme = 0;
      } else
        putme -= stay;
      if (stay>inj) {
        injected_particles[ns2+i] = 0;  *mynps = stay - inj;
      } else {
        injected_particles[ns2+i] = inj - stay;  *mynps = 0;
      }
      npnext += tpni;
    }
    pnext[ps] = npnext;
  }
  set_sendbuf_disps_state(state, secondary, sec);
  if (state->n_of_injections)  move_injected_to_sendbuf(state);

  move_to_sendbuf_uw(state, 0, me, mynp[0],     /* &NOfPLocal[0][0][me] */
                     0, total_particles, 0, total_particles_next,
                     state->recv_buffer_bases);

  if (secondary) {
    move_to_sendbuf_uw(state, 1, sec, mynp[1],  /* &NOfPLocal[1][0][sec] */
                       state->primary_parts, total_particles+ns, pnext[0],
                       total_particles_next+ns,
                       state->recv_buffer_bases+ns);
    move_to_sendbuf_dw(state, 1, sec, mynp[1], state->total_parts,
                       total_particles+ns, pnext[0]+pnext[1],
                       total_particles_next+ns);
  } else {
    struct S_particle *rbb=state_particle_at(state, state->particles,
                                             pnext[0]);
    for (s=0; s<ns; s++) {
      state->recv_buffer_bases[ns+s] = rbb;     /* RecvBufBases[1][s] */
      rbb = state_particle_at(state, rbb, total_particles_next[ns+s]);
                                                /* TotalPNext[1][s] */
    }
  }
  move_to_sendbuf_dw(state, 0, me, mynp[0], state->primary_parts,
                     total_particles, pnext[0], total_particles_next);

  set_sendbuf_disps_state(state, secondary, sec);
  if (state->n_of_injections) {
    move_injected_from_sendbuf(state, injected_particles+ns2, me,
                               state->recv_buffer_bases);
    if (sec>=0)
      move_injected_from_sendbuf(state, injected_particles+ns2+ns, sec,
                                 state->recv_buffer_bases+ns);
  }
  state->primary_parts = pnext[0];
  if (oh_context_is_default_state(state)) primaryParts = pnext[0];
  *state->secondary_base = pnext[0];
}
void
set_sendbuf_disps_state(struct oh_state *state, int secondary, int parent) {
  int nn=state->n_of_nodes, ns=state->n_of_species, me=state->my_rank;
  int *sendbuf_disps=state->send_buffer_disps;
  int *n_of_particles_local=state->n_of_particles_local;
  int *injected_particles=state->injected_particles;
  int i, j, k, s, disp;

  for (s=0,i=0,disp=0; s<ns; s++) {
    for (k=0; k<nn; k++,i++) {
      sendbuf_disps[i] = disp;                  /* SendBufDisps[s][k] */
      disp += n_of_particles_local[i];          /* NOfPLocal[0][s][k] */
      if (k==me)  disp += injected_particles[s];/* InjectedParticles[0][s] */
    }
  }
  if (secondary) {
    for (s=0,j=0,disp=0; s<ns; s++) {
      for (k=0; k<nn; k++,i++,j++) {
        sendbuf_disps[j] += disp;               /* SendBufDisps[s][k] */
        disp += n_of_particles_local[i];        /* NOfPLocal[1][s][k] */
        if (k==parent)  disp += injected_particles[ns+s];
      }                                         /* InjectedParticles[1][s] */
    }
  }
}
void
exchange_particles_state(struct oh_state *state, struct S_commlist *secrlist,
                         int secrlsize, int oldparent, int neighboring,
                         int currmode, int stats) {
  int me, nn, ns, nnns;
  int newparent;
  int s, i, req;

  move_to_sendbuf_secondary_state(state, Mode_PS(currmode), stats);
  me=state->my_rank;  nn=state->n_of_nodes;  ns=state->n_of_species;
  nnns=nn*ns;
  newparent=state->nodes[me].parentid;
  if (stats) oh1_stats_time(STATS_TB_COMM, 1);
  if (neighboring) {
    req = 0;
    receive_particles(state, state->comm_list, state->sl_head_tail[0], &req);
    if (newparent>=0)
      receive_particles(state, secrlist, secrlsize, &req);
    if (oldparent!=newparent && oldparent>=0)
      receive_particles(state, state->comm_list+state->sl_head_tail[1],
                        state->sec_sl_head_tail[0], &req);
    send_particles(state, state->comm_list+state->sl_head_tail[0],
                   state->sl_head_tail[1]-state->sl_head_tail[0], newparent,
                   oldparent, &req);
    if (oldparent>=0)
      send_particles(state,
                     state->comm_list+state->sl_head_tail[1]+
                     state->sec_sl_head_tail[0],
                     state->sec_sl_head_tail[1]-state->sec_sl_head_tail[0],
                     me, newparent, &req);
    MPI_Waitall(req, state->requests, state->statuses);
  }
  else {
    int ps;
    int *rcount=state->n_of_recv;
    int *scount=state->n_of_send;
    void **rbb=state->recv_buffer_bases;
    for (ps=0; ps<2; ps++,rbb+=ns) {            /* rbb=&RecvBufBases[p][0] */
      int *sbd0=state->send_buffer_disps, *sbd;
      for (s=0; s<ns; s++,rcount+=nn,scount+=nn,sbd0+=nn) {
                                        /* rcount=&NOfRecv[ps][s][0] */
                                        /* sbd0=&SendBufDisps[s][0] */
        int rdisp=0;
        for (i=0; i<nn; i++) {
          state->recv_buffer_disps[i] = rdisp;  rdisp += rcount[i];
        }
        if (ps==0) sbd = sbd0;                  /* &SendBufDisps[s][0] */
        else {
          sbd = state->temp_array;
          for (i=0; i<nn; i++) {
            int r=state->nodes[i].parentid;
            if (r>=0) {
              sbd[i] = sbd0[r];
              sbd0[r] += scount[i];
            }
            else sbd[i] = 0;            /* not necessary becasuse scount[i]=0
                                           but ... */
          }
        }
        MPI_Alltoallv(state->send_buffer, scount, sbd,
                      state->particle_mpi_type, rbb[s], rcount,
                      state->recv_buffer_disps, state->particle_mpi_type,
                      state->comm);
        if (ps==0)
          for (i=0; i<nn; i++) sbd0[i] += scount[i];
      }
    }
  }
}
static void
move_to_sendbuf_uw(struct oh_state *state, int ps, int me, int *putmes,
                   int cbase, int *ctp, int nbase, int *ntp,
                   void **rbb) {
  int i, in, j, jn, k, s;
  int ns=state->n_of_species, nn=state->n_of_nodes;
  int *sbd=state->send_buffer_disps;
  struct S_particle *particles=state->particles;
  struct S_particle *sendbuf=state->send_buffer;

  for (s=0,i=cbase,j=nbase,k=0; s<ns; s++,i=in,j=jn,sbd+=nn,k+=nn) {
    int putme = putmes ? putmes[k] : 0; /* NOfPLocal[0/1][s][me/sec] */
    in = i + ctp[s];  jn = j + ntp[s];
    if (j<=i) {                         /* upward move only */
      for (; putme>0; i++) {            /* throw my particles to send buf */
        struct S_particle *part=state_particle_at(state, particles, i);
        int dst=state_particle_subdomain(state, part, ps);
        if (dst<0) continue;
        state_copy_particle(state, state_particle_at(state, sendbuf,
                                                     sbd[dst]++), part);
        if (dst==me) putme--;
      }
      for (; i<in; i++) {               /* move upward */
        struct S_particle *part=state_particle_at(state, particles, i);
        int dst=state_particle_subdomain(state, part, ps);
        if (dst<0) continue;
        if (dst==me)
          state_copy_particle(state, state_particle_at(state, particles, j++),
                              part);
        else
          state_copy_particle(state, state_particle_at(state, sendbuf,
                                                       sbd[dst]++), part);
      }
      rbb[s] = state_particle_at(state, particles, j); /* receive to bottom */
    } else if (jn>in) {                 /* downward only and thus skip */
      rbb[s] = state_particle_at(state, particles, j); /* receive to top */
    } else {                            /* downward and upward */
      int ib, im, jm;
      for (; putme>0; i++) {            /* throw my particles to send buf */
        struct S_particle *part=state_particle_at(state, particles, i);
        int dst=state_particle_subdomain(state, part, ps);
        if (dst<0) continue;
        state_copy_particle(state, state_particle_at(state, sendbuf,
                                                     sbd[dst]++), part);
        if (dst==me) putme--;
      }
      ib = i;
      for (; i<j; i++) {                 /* skip downward movers if any */
        int dst=state_particle_subdomain(
          state, state_particle_at(state, particles, i), ps);
        if (dst==me && dst>=0)  j++;
      }
      im = i-1; jm = j-1;
      for (; i<in; i++) {               /* move remainders upward */
        struct S_particle *part=state_particle_at(state, particles, i);
        int dst=state_particle_subdomain(state, part, ps);
        if (dst<0) continue;
        if (dst==me)
          state_copy_particle(state, state_particle_at(state, particles, j++),
                              part);
        else
          state_copy_particle(state, state_particle_at(state, sendbuf,
                                                       sbd[dst]++), part);
      }
      rbb[s] = state_particle_at(state, particles, j); /* receive to bottom */
      for (i=im,j=jm; i>=ib; i--) {     /* move first half downward if any */
        struct S_particle *part=state_particle_at(state, particles, i);
        int dst=state_particle_subdomain(state, part, ps);
        if (dst<0) continue;
        if (dst==me)
          state_copy_particle(state, state_particle_at(state, particles, j--),
                              part);
        else
          state_copy_particle(state, state_particle_at(state, sendbuf,
                                                       sbd[dst]++), part);
      }
    }
  }
}
static void
move_to_sendbuf_dw(struct oh_state *state, int ps, int me, int *putmes,
                   int ctail, int *ctp, int ntail, int *ntp) {
  int i, in, j, jn, k, s;
  int ns=state->n_of_species, nn=state->n_of_nodes, nnnsm1=nn*(ns-1);
  int *sbd=state->send_buffer_disps+nnnsm1;
  struct S_particle *particles=state->particles;
  struct S_particle *sendbuf=state->send_buffer;

  in = ctail;  jn = ntail;
  for (s=ns-1,i=in-1,j=jn-1,k=nnnsm1; s>=0; s--,i=in-1,j=jn-1,sbd-=nn,k-=nn) {
    int putme = putmes ? putmes[k] : 0; /* NOfPLocal[0/1][s][me/sec] */
    in -= ctp[s];  jn -= ntp[s];
    if (i>=j || in>=jn) continue;       /* not downward only and thus skip */
    for (; putme>0; i--) {              /* throw my particles to send buf */
      struct S_particle *part=state_particle_at(state, particles, i);
      int dst=state_particle_subdomain(state, part, ps);
      if (dst<0) continue;
      state_copy_particle(state,
                          state_particle_at(state, sendbuf, sbd[dst]++),
                          part);
      if (dst==me) putme--;
    }
    for (; i>=in; i--) {                /* move downward */
      struct S_particle *part=state_particle_at(state, particles, i);
      int dst=state_particle_subdomain(state, part, ps);
      if (dst<0) continue;
      if (dst==me)
        state_copy_particle(state, state_particle_at(state, particles, j--),
                            part);
      else
        state_copy_particle(state,
                            state_particle_at(state, sendbuf, sbd[dst]++),
                            part);
    }
  }
}
static void
move_injected_to_sendbuf(struct oh_state *state) {
  struct S_particle *pbuf=state_particle_at(state, state->particles,
                                            state->total_parts);
  int ninj=state->n_of_injections, nn=state->n_of_nodes;
  int i;

  for (i=0; i<ninj; i++) {
    struct S_particle *part=state_particle_at(state, pbuf, i);
    int dst = state_map_injected_particle_to_subdomain(state, part);
    int s = state_particle_species(state, part);
    if (dst<0) continue;
    state_copy_particle(state,
                        state_particle_at(
                          state, state->send_buffer,
                          state->send_buffer_disps[dst+s*nn]++), part);
  }
}
static void
move_injected_from_sendbuf(struct oh_state *state, int *injected, int mysd,
                           void **rbb) {
  int nn=state->n_of_nodes, ns=state->n_of_species;
  int *sdisp=state->send_buffer_disps+mysd;
  int s, i;

  for (s=0; s<ns; s++,sdisp+=nn) {
    struct S_particle *rbuf=rbb[s];
    struct S_particle *sbuf=state_particle_at(state, state->send_buffer,
                                              *sdisp);
    int inj=injected[s];
    state_copy_particles(state, rbuf, sbuf, inj);
    rbb[s] = state_particle_at(state, rbb[s], inj);  *sdisp += inj;
  }
}
static void
receive_particles(struct oh_state *state, struct S_commlist *rlist,
                  int rlsize, int *req) {
  int me=state->my_rank, i, r=*req, nn=state->n_of_nodes;
  int ns=state->n_of_species, sdisp;
  struct S_particle *rbuf;

  for (i=0; i<rlsize; i++) {
    if (rlist[i].rid==me) {
      int count=rlist[i].count, tag=rlist[i].tag;
      rbuf = state->recv_buffer_bases[tag];
      state->recv_buffer_bases[tag] = state_particle_at(state, rbuf, count);
      MPI_Irecv(rbuf, count, state->particle_mpi_type, rlist[i].sid, tag,
                state->comm, state->requests+(r++));
    }
    if (rlist[i].sid==me) {
      int count=rlist[i].count, tag=rlist[i].tag, region=rlist[i].region;
      region += nn * (tag<ns ? tag : tag-ns);
      sdisp = state->send_buffer_disps[region];
      state->send_buffer_disps[region] = sdisp + count;
                                                /* SendBufDisps[s][region] */
      MPI_Isend(state_particle_at(state, state->send_buffer, sdisp), count,
                state->particle_mpi_type, rlist[i].rid, tag, state->comm,
                state->requests+(r++));
    }
  }
  *req = r;
}
static void
send_particles(struct oh_state *state, struct S_commlist *slist, int slsize,
               int myregion, int parentregion, int *req) {
  int me=state->my_rank, i, r=*req, nn=state->n_of_nodes;
  int ns=state->n_of_species, sdisp, region;

  for (i=0; i<slsize; i++) {
    if (slist[i].sid==me && (region=slist[i].region)!=myregion &&
                            region != parentregion) {
      int count=slist[i].count, tag=slist[i].tag;
      region += nn * (tag<ns ? tag : tag-ns);
      sdisp = state->send_buffer_disps[region];
      state->send_buffer_disps[region] = sdisp + count;
                                                /* SendBufDisps[s][region] */
      MPI_Isend(state_particle_at(state, state->send_buffer, sdisp), count,
                state->particle_mpi_type, slist[i].rid, tag, state->comm,
                state->requests+(r++));
    }
  }
  *req = r;
}
void
oh2_inject_particle_(struct S_particle *part) {
  oh2_inject_particle(part);
}
void
oh2_inject_particle(void *part) {
  (void)oh2_inject_particle_state(oh1_state(), part);
}
void *
oh2_inject_particle_get(void *part) {
  return oh2_inject_particle_state(oh1_state(), part);
}
void *
oh2_inject_particle_state(struct oh_state *state, void *part) {
  int inj = state->total_parts + state->n_of_injections++;
  void *copy;
  if (oh_context_is_default_state(state))
    nOfInjections = state->n_of_injections;

#ifndef OH_HAS_SPEC
  if (!state->use_custom_particle_adapter && state->n_of_species!=1)
    local_errstop("particles cannot be injected when S_particle does not "
                  "have 'spec' element and you have two or more species");
#endif
  if (inj>=state->n_of_local_particles_limit)
    local_errstop("injection causes local particle buffer overflow");
  copy = state_particle_at(state, state->particles, inj);
  state_copy_particle(state, copy, part);
  state_update_injected_particle_count(state, copy, 1);
  return copy;
}
void
oh2_remap_injected_particle_(struct S_particle *part) {
  oh2_remap_injected_particle(part);
}
void
oh2_remap_injected_particle(void *part) {
  oh2_remap_injected_particle_state(oh1_state(), part);
}
void
oh2_remap_injected_particle_state(struct oh_state *state, void *part) {
  const int pidx = state_particle_buffer_index(state, part);

  if (pidx<state->total_parts ||
      pidx>=state->total_parts+state->n_of_injections)
    local_errstop("'part' argument pointing %c%d%c of the particle buffer is "\
                  "not for injected particles",
                  state->spec_base?'(':'[', pidx+state->spec_base,
                  state->spec_base?')':']');
#ifndef OH_HAS_SPEC
  if (!state->use_custom_particle_adapter && state->n_of_species!=1)
    local_errstop("particles cannot be injected when S_particle does not "
                  "have 'spec' element and you have two or more species");
#endif
  state_update_injected_particle_count(state, part, 1);
}
void
oh2_remove_injected_particle_(struct S_particle *part) {
  oh2_remove_injected_particle(part);
}
void
oh2_remove_injected_particle(void *part) {
  oh2_remove_injected_particle_state(oh1_state(), part);
}
void
oh2_remove_injected_particle_state(struct oh_state *state, void *part) {
  const int pidx = state_particle_buffer_index(state, part);

  if (pidx<state->total_parts ||
      pidx>=state->total_parts+state->n_of_injections)
    local_errstop("'part' argument pointing %c%d%c of the particle buffer is "\
                  "not for injected particles",
                  state->spec_base?'(':'[', pidx+state->spec_base,
                  state->spec_base?')':']');
#ifndef OH_HAS_SPEC
  if (!state->use_custom_particle_adapter && state->n_of_species!=1)
    local_errstop("particles cannot be injected when S_particle does not "
                  "have 'spec' element and you have two or more species");
#endif
  state_update_injected_particle_count(state, part, -1);
  state_mark_particle_removed(state, part, 0);
}
static void
state_update_injected_particle_count(struct oh_state *state,
                                     struct S_particle *part, int delta) {
  const int ns=state->n_of_species, nn=state->n_of_nodes;
  int s, n;

  s = state_particle_species(state, part);
  n = state_map_injected_particle_to_subdomain(state, part);
  if (n<0)  return;
  if (n==state->region_id[1]) {
    state->n_of_particles_local[(ns+s)*nn+n] += delta;
    state->injected_particles[ns+s] += delta;
  } else {
    state->n_of_particles_local[nn*s+n] += delta;
    if (n==state->my_rank)  state->injected_particles[s] += delta;
  }
}
static OH_nid_t
state_particle_region(struct oh_state *state, const struct S_particle *part,
                      int primary_or_secondary) {
  oh_particle_region_t region =
    state->particle_adapter->get_region(state->particle_adapter, part,
                                        primary_or_secondary);

  return (OH_nid_t)region;
}
static void
state_set_particle_region(struct oh_state *state, struct S_particle *part,
                          OH_nid_t region, int primary_or_secondary) {
  state->particle_adapter->set_region(state->particle_adapter, part, region,
                                      primary_or_secondary);
}
static void
state_mark_particle_removed(struct oh_state *state, struct S_particle *part,
                            int primary_or_secondary) {
  state_set_particle_region(state, part, (OH_nid_t)-1, primary_or_secondary);
}
static int
state_particle_species(struct oh_state *state, const struct S_particle *part) {
  int raw_species = state->particle_adapter->get_species(
    state->particle_adapter, part);
  int species = raw_species;
  int species_base = state->particle_adapter->species_base;

  if (!state->particle_adapter->single_species)
    species -= species_base;

#ifdef OH_HAS_SPEC
  species = Particle_Spec(species);
#else
  if (!state->use_custom_particle_adapter) return 0;
#endif
  if (species<0 || species>=state->n_of_species)
    local_errstop("particle species %d is outside configured range [%d,%d)",
                  raw_species, species_base,
                  species_base + state->n_of_species);
  return species;
}
static int
state_particle_subdomain(struct oh_state *state, const struct S_particle *part,
                         int primary_or_secondary) {
  if (state->particle_adapter->map_to_subdomain)
    return state->particle_adapter->map_to_subdomain(
      state->particle_adapter, (void*)part, primary_or_secondary);
  return state_region_subdomain(
    state, state_particle_region(state, part, primary_or_secondary),
    primary_or_secondary);
}
static int
state_map_injected_particle_to_subdomain(struct oh_state *state,
                                         struct S_particle *part) {
  int dst;

  if (state->particle_adapter->map_to_subdomain)
    return state->particle_adapter->map_to_subdomain(state->particle_adapter,
                                                     part, 0);
  dst = state_region_subdomain(state, state_particle_region(state, part, 0),
                               0);
#ifdef OH_POS_AWARE
  if (dst>=state->n_of_nodes)  dst = state_primarize_particle(state, part);
#endif
  return dst;
}
static int
state_region_subdomain(struct oh_state *state, OH_nid_t region,
                       int primary_or_secondary) {
#ifdef OH_POS_AWARE
  OH_nid_t subdomain;

  if (region<0) return -1;
  subdomain = region >> state->log_grid;
  if (subdomain<OH_NEIGHBORS)
    return state->abs_neighbors[primary_or_secondary][(int)subdomain];
  return (int)(subdomain - OH_NEIGHBORS);
#else
  (void)state;
  (void)primary_or_secondary;
  return region;
#endif
}
static int
state_primarize_particle(struct oh_state *state, struct S_particle *part) {
#ifdef OH_POS_AWARE
  const OH_nid_t region =
    (OH_nid_t)state->particle_adapter->get_region(
      state->particle_adapter, part, 1) -
    ((OH_nid_t)(state->n_of_nodes + OH_NEIGHBORS) << state->log_grid);

  state->particle_adapter->set_region(state->particle_adapter, part, region,
                                      1);
  return state_region_subdomain(state, region, 1);
#else
  (void)state;
  (void)part;
  return -1;
#endif
}
static size_t
particle_stride_state(struct oh_state *state) {
  return oh_particle_buffer_stride(state->particle_adapter);
}
static struct S_particle *
state_particle_at(struct oh_state *state, void *base, int index) {
  return oh_particle_buffer_at(state->particle_adapter, base, index);
}
static int
state_particle_buffer_index(struct oh_state *state,
                            const struct S_particle *part) {
  return oh_particle_buffer_index(state->particle_adapter, state->particles,
                                  part);
}
static void
state_copy_particle(struct oh_state *state, struct S_particle *dst,
                    const struct S_particle *src) {
  oh_particle_buffer_copy(state->particle_adapter, dst, src);
}
static void
state_copy_particles(struct oh_state *state, struct S_particle *dst,
                     const struct S_particle *src, int count) {
  oh_particle_buffer_copy_n(state->particle_adapter, dst, src, count);
}
void
oh2_set_total_particles_() {
  oh2_set_total_particles();
}
void
oh2_set_total_particles() {
  oh2_set_total_particles_state(oh1_state());
}
void
oh2_set_total_particles_state(struct oh_state *state) {
  if (!state) state = oh1_state();
  if (state->n_of_injections) finalize_injected_particles_state(state);
  set_total_particles_state(state);
  if (state->particle_base_bound) {
    *state->secondary_base = state->primary_parts;
    *state->total_local_particles = state->total_parts;
  }
  if (oh_context_is_default_state(state)) {
    TotalP = state->total_particles;
    primaryParts = state->primary_parts;
    totalParts = state->total_parts;
    nOfInjections = state->n_of_injections;
    oh1_sync_default_state();
  }
}
static void
finalize_injected_particles_state(struct oh_state *state) {
  const int ns=state->n_of_species, nn=state->n_of_nodes;
  const int old_primary=state->primary_parts;
  const int old_total=state->total_parts;
  const int ninj=state->n_of_injections;
  const int scan_total=old_total+ninj;
  const int nclass=ns*2, nhist=nclass*nn;
  int *counts, *hist, *cursor;
  int i, s, t, ps, dst, total;

  if (!state->particle_buffer_bound)
    local_errstop("particle buffer is not bound");
  if (!state->particle_accounting_bound || !state->particle_base_bound)
    local_errstop("particle accounting is not bound");
  if (!state->send_buffer)
    local_errstop("particle work buffer is not allocated");
  if (scan_total>state->n_of_local_particles_limit)
    local_errstop("injection causes local particle buffer overflow");

  counts = (int*)mem_alloc(sizeof(int), nclass, "FinalizeCounts");
  hist = (int*)mem_alloc(sizeof(int), nhist, "FinalizeNOfPLocal");
  cursor = (int*)mem_alloc(sizeof(int), nclass, "FinalizeCursor");
  for (i=0; i<nclass; i++) counts[i] = cursor[i] = 0;
  for (i=0; i<nhist; i++) hist[i] = 0;

  for (i=0; i<scan_total; i++) {
    struct S_particle *part=state_particle_at(state, state->particles, i);
    if (i<old_total) {
      ps = i<old_primary ? 0 : 1;
      dst = state_particle_subdomain(state, part, ps);
    } else {
      ps = state_injected_particle_region_kind(state, part);
      dst = state_map_injected_particle_to_subdomain(state, part);
    }
    if (dst<0) continue;
    s = state_particle_species(state, part);
    t = ps*ns+s;
    counts[t]++;
    hist[t*nn+dst]++;
  }

  total = 0;
  for (s=0; s<ns; s++) {
    cursor[s] = total;
    total += counts[s];
  }
  state->primary_parts = total;
  for (s=0; s<ns; s++) {
    t = ns+s;
    cursor[t] = total;
    total += counts[t];
  }
  if (total>state->n_of_local_particles_limit)
    local_errstop("injection causes local particle buffer overflow");

  for (i=0; i<scan_total; i++) {
    struct S_particle *part=state_particle_at(state, state->particles, i);
    if (i<old_total) {
      ps = i<old_primary ? 0 : 1;
      dst = state_particle_subdomain(state, part, ps);
    } else {
      ps = state_injected_particle_region_kind(state, part);
      dst = state_map_injected_particle_to_subdomain(state, part);
    }
    if (dst<0) continue;
    s = state_particle_species(state, part);
    t = ps*ns+s;
    state_copy_particle(state,
                        state_particle_at(state, state->send_buffer,
                                          cursor[t]++), part);
  }

  state_copy_particles(state, state->particles, state->send_buffer, total);
  for (i=0; i<nhist; i++) state->n_of_particles_local[i] = hist[i];
  for (i=0; i<nclass; i++) state->injected_particles[i] = 0;
  state->total_parts = total;
  state->n_of_injections = 0;
  *state->secondary_base = state->primary_parts;
  *state->total_local_particles = total;

  free(cursor);
  free(hist);
  free(counts);
}
static int
state_injected_particle_region_kind(struct oh_state *state,
                                    struct S_particle *part) {
  int dst = state_map_injected_particle_to_subdomain(state, part);
  return dst>=0 && dst==state->region_id[1] ? 1 : 0;
}
int
oh2_max_local_particles_(dint *npmax, int *maxfrac, int *minmargin) {
  return(oh2_max_local_particles(*npmax, *maxfrac, *minmargin));
}
int
oh2_max_local_particles(dint npmax, int maxfrac, int minmargin) {
  int nn, nplint;
  dint npl, npmargin;

  MPI_Comm_size(MCW, &nn);
  if (npmax<=0) errstop("max # of particles should be greater than 0");
  if (maxfrac<=0 || maxfrac>100)
    errstop("load imbalance factor (%d) should be in the range [1..100]",
            maxfrac);
  npl = (npmax-1)/nn + 1; /* ceil(npmax/nn) */
  npmargin = (npl*maxfrac-1)/100 + 1;
  npl += (npmargin<minmargin) ? minmargin : npmargin;
  if (npl>INT_MAX) mem_alloc_error("Particles", 0);
  nplint = npl;
  return(nplint);
}
