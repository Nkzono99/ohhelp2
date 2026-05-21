/* File: ohhelp2.c
   Version 1.1.1 (2015/10/23)
   Copyright (C) 2009-2015  Hiroshi Nakashima <h.nakashima@media.kyoto-u.ac.jp>
                            (ACCMS, Kyoto University)
   This program can be freely used, redistributed and modified for non-
   commercial purpose providing that the copyright notice above remains
   unchanged.
*/
#define EXTERN extern
#include "ohhelp1.h"
#undef  EXTERN
#define EXTERN
#include "ohhelp2.h"
#include "oh_particle_buffer.h"

/* Prototypes for private functions. */
static int   try_primary2(int currmode, int level, int stats);
static int   try_stable2(int currmode, int level, int stats);
static void  rebalance2(int currmode, int level, int stats);
static void  exchange_primary_particles_state(struct oh_state *state,
                                              int currmode, int stats);
static void  set_sendbuf_disps_state(struct oh_state *state, int secondary,
                                     int parent);
static void  move_to_sendbuf_secondary(int secondary, int stats);
static void  move_to_sendbuf_uw(struct oh_state *state, int ps, int me,
                                int *putmes, int cbase, int *ctp, int nbase,
                                int *ntp, struct S_particle **rbb);
static void  move_to_sendbuf_dw(struct oh_state *state, int ps, int me,
                                int *putmes, int ctail, int *ctp, int ntail,
                                int *ntp);
static void  move_injected_to_sendbuf(struct oh_state *state);
static void  move_injected_from_sendbuf(struct oh_state *state,
                                        int *injected, int mysd,
                                        struct S_particle **rbb);
static void  receive_particles(struct oh_state *state,
                               struct S_commlist *rlist, int rlsize,
                               int *req);
static void  send_particles(struct oh_state *state,
                            struct S_commlist *slist, int slsize,
                            int myregion, int parentregion, int *req);
static int   particle_region(const struct S_particle *part,
                             int primary_or_secondary);
static void  set_particle_region(struct S_particle *part, int region,
                                 int primary_or_secondary);
static int   particle_species(const struct S_particle *part);
static int   particle_subdomain(const struct S_particle *part,
                                int primary_or_secondary);
static int   map_injected_particle_to_subdomain(struct S_particle *part);
static size_t particle_stride(void);
static struct S_particle *particle_at(struct S_particle *base, int index);
static int   particle_buffer_index(const struct S_particle *part);
static void  copy_particle(struct S_particle *dst,
                           const struct S_particle *src);
static void  copy_particles(struct S_particle *dst,
                            const struct S_particle *src, int count);

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
  CustomTParticle = type;
  useCustomTParticle = 1;
}
void
oh2_set_particle_adapter(const oh_particle_adapter *adapter) {
  if (!oh_particle_adapter_validate(adapter))
    local_errstop("invalid oh_particle_adapter");
  CustomParticleAdapter = *adapter;
  useCustomParticleAdapter = 1;
  CustomTParticle = adapter->mpi_type;
  useCustomTParticle = 1;
}
void
oh2_init(int **sdid, int nspec, int maxfrac, int **nphgram,
         int **totalp, struct S_particle **pbuf, int **pbase, int maxlocalp,
         void *mycomm, int **nbor, int *pcoord,
         int stats, int repiter, int verbose) {
  specBase = 0;
  init2(sdid, nspec, maxfrac, nphgram, totalp,
        pbuf, pbase, maxlocalp, (struct S_mycommc*)mycomm, NULL, nbor, pcoord,
        stats, repiter, verbose);
}
void
init2(int **sdid, int nspec, int maxfrac, int **nphgram,
      int **totalp, struct S_particle **pbuf, int **pbase, int maxlocalp,
      struct S_mycommc *mycommc, struct S_mycommf *mycommf,
      int **nbor, int *pcoord, int stats, int repiter, int verbose) {
  int ns, nn, nnns, s;

  init1(sdid, nspec, maxfrac, nphgram, totalp, NULL, NULL,
        mycommc, mycommf, nbor, pcoord, stats, repiter, verbose);

  ns = nOfSpecies;  nn = nOfNodes;  nnns = nn * ns;

  if (useCustomParticleAdapter) {
    ParticleAdapter = CustomParticleAdapter;
    T_Particle = ParticleAdapter.mpi_type;
  } else if (useCustomTParticle) {
    T_Particle = CustomTParticle;
    ParticleAdapter = oh_default_particle_adapter(T_Particle);
  } else {
    MPI_Type_contiguous(sizeof(struct S_particle), MPI_BYTE, &T_Particle);
    MPI_Type_commit(&T_Particle);
    ParticleAdapter = oh_default_particle_adapter(T_Particle);
  }
  if (!oh_particle_adapter_validate(&ParticleAdapter))
    local_errstop("particle MPI datatype extent must match particle stride");

  nOfLocalPLimit = totalParts = maxlocalp;
  if (*pbuf)
    Particles = *pbuf;
  else
    Particles = *pbuf =
      (struct S_particle*)mem_alloc(particle_stride(), maxlocalp, "Particles");

  if (!*pbase)  *pbase = (int*)mem_alloc(sizeof(int), 3, "ParticleBase");
  (*pbase)[0] = (*pbase)[1] = (*pbase)[2] = 0;
  secondaryBase = *pbase + 1;  totalLocalParticles = *pbase + 2;

#ifndef OH_POS_AWARE
  SendBuf = (struct S_particle*)mem_alloc(particle_stride(), maxlocalp,
                                          "SendBuf");
#endif
  RecvBufBases = (struct S_particle**)mem_alloc(sizeof(struct S_particle*),
                                                2*ns+1, "RecvBufBases");
  SendBufDisps = (int*)mem_alloc(sizeof(int),  nnns, "SendBufDisps");
  RecvBufDisps = (int*)mem_alloc(sizeof(int),  nn, "RecvBufDisps");
  nOfInjections = 0;

  Requests = (MPI_Request*)mem_alloc(sizeof(MPI_Request),
                                     nnns*4+OH_NEIGHBORS*2, "Requests");
  Statuses = (MPI_Status*) mem_alloc(sizeof(MPI_Status),
                                     nnns*4+OH_NEIGHBORS*2, "Statuses");
}
int
oh2_transbound_(int *currmode, int *stats) {
  return(transbound2(*currmode, *stats, 2));
}
int
oh2_transbound(int currmode, int stats) {
  return(transbound2(currmode, stats, 2));
}
int
transbound2(int currmode, int stats, int level) {
  int ret=MODE_NORM_SEC, nn=nOfNodes, ns=nOfSpecies, nnns2=2*nn*ns;
  int i, s, tp;

  stats = stats && statsMode;
  currmode = transbound1(currmode, stats, level);

  if (try_primary2(currmode, level, stats))  ret = MODE_NORM_PRI;
  else if (!Mode_PS(currmode) || !try_stable2(currmode, level, stats)) {
    rebalance2(currmode, level, stats);  ret = MODE_REB_SEC;
  }
  for (i=0; i<nnns2; i++) NOfPLocal[i] = 0;
  for (s=0,tp=0; s<ns*2; s++) {
    TotalP[s] = TotalPNext[s];  tp += TotalPNext[s];
  }
  for (s=0; s<ns*2; s++)  InjectedParticles[s] = 0;
  totalParts = *totalLocalParticles = tp;  nOfInjections = 0;
  return((currMode=ret));
}
static int
try_primary2(int currmode, int level, int stats) {

  if (!try_primary1(currmode, level, stats))  return(FALSE);
  move_to_sendbuf_primary(Mode_PS(currmode), stats);
  exchange_primary_particles(currmode, stats);
  primaryParts = *secondaryBase = TotalPGlobal[myRank];
  return(TRUE);
}
void
exchange_primary_particles(int currmode, int stats) {
  exchange_primary_particles_state(oh1_state(), currmode, stats);
}
static void
exchange_primary_particles_state(struct oh_state *state, int currmode,
                                 int stats) {
  int i, s, nn=state->n_of_nodes, ns=state->n_of_species, nnns=nn*ns;
  int me=state->my_rank;
  int *np, *rnp, *sbd;
  MPI_Comm comm=state->comm;
  MPI_Datatype particle_type=state->particle_mpi_type;
  struct S_particle *sendbuf=state->send_buffer;
  struct S_particle **recvbuf_bases=state->recv_buffer_bases;
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
            MPI_Sendrecv(particle_at(sendbuf, sbd[dst]), np[dst],
                         particle_type, dst, 0,
                         rb, rc, particle_type, src, 0, comm, &st);
          else
            MPI_Recv(rb, rc, particle_type, src, 0, comm, &st);
          rb = particle_at(rb, rc);
        } else if (dst>=0)
          MPI_Send(particle_at(sendbuf, sbd[dst]), np[dst], particle_type,
                   dst, 0, comm);
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
try_stable2(int currmode, int level, int stats) {

  if (!try_stable1(currmode, level, stats)) return(FALSE);
  exchange_particles(CommList+SLHeadTail[1], SecSLHeadTail[0],
                     Nodes[myRank].parentid, currmode==MODE_NORM_SEC,
                     currmode, stats);
  return(TRUE);
}
static void
rebalance2(int currmode, int level, int stats) {
  int me=myRank, ns=nOfSpecies, s, oldp, newp;

  rebalance1(currmode, level, stats);
  oldp = NodesNext[me].parentid;  newp=Nodes[me].parentid;
  if (nOfInjections && oldp>=0 && oldp!=newp)
    for (s=0; s<ns; s++)  InjectedParticles[ns+s] = 0;
  if (Mode_Is_Norm(currmode))
    exchange_particles(SecRList, SecRLSize,
                       Mode_PS(currmode) ? oldp : -1,
                       1, currmode, stats);
  else
    exchange_particles(SecRList, SecRLSize, -1, 0, currmode, stats);
}
void
move_to_sendbuf_primary(int secondary, int stats) {
  int me=myRank, ns=nOfSpecies, nn=nOfNodes, nnns=nn*ns;
  struct oh_state *state;
  int s, i, j, *pp;

  if (stats) oh1_stats_time(STATS_TB_MOVE, 0);
  for (s=0,i=me,pp=NOfPrimaries; s<ns; s++,i+=nn,pp+=nn) {
    int t = 0;
    NOfPLocal[i] = 0;                           /* NOfPLocal[0][s][me] */
    for (j=0; j<nn; j++) t += pp[j] + pp[j+nnns];
                            /* NOfPrimaries[0][s][j] + NOfPrimaries[1][s][j] */
    TotalPNext[s] = t;                          /* TotalPNext[0][s] */
    TotalPNext[ns+s] = 0;                       /* TotalPNext[1][s] */
  }
  set_sendbuf_disps(secondary, -1);
  state = oh1_state();
  if (nOfInjections)  move_injected_to_sendbuf(state);

  move_to_sendbuf_uw(state, 0, me, NOfPLocal+me, 0, TotalP, 0, TotalPNext,
                     state->recv_buffer_bases);

  if (secondary) move_to_sendbuf_uw(state, 1, -1, NULL, primaryParts,
                                    TotalP+ns, 0, TotalPNext+ns,
                                    state->recv_buffer_bases+ns);

  move_to_sendbuf_dw(state, 0, me, NOfPLocal+me, primaryParts, TotalP,
                     TotalPGlobal[me], TotalPNext);

  set_sendbuf_disps(secondary, -1);
  if (nOfInjections)
    move_injected_from_sendbuf(state, InjectedParticles, me,
                               state->recv_buffer_bases);
}
static void
move_to_sendbuf_secondary(int secondary, int stats) {
  int me=myRank, ns=nOfSpecies, ns2=ns<<1, nn=nOfNodes;
  struct oh_state *state;
  struct S_node *node = Nodes+me;
  int put[2]={-node->get.prime, -node->get.sec}, pnext[2];
  int sec=node->parentid;
  int nnns=nn*ns;
  int *mynp[2]={NOfPLocal+me,           /* &NOfPLocal[0][0][me] */
                sec<0 ? NULL : NOfPLocal+nnns+sec};
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
      int tpni=TotalPNext[i];                   /* TotalPNext[ps][s] */
      int inj=InjectedParticles[i];             /* InjectedParticles[ps][s] */
      if (putme<stay) {
        TotalPNext[i] = tpni = tpni + stay - putme;
        stay = putme;
        putme = 0;
      } else
        putme -= stay;
      if (stay>inj) {
        InjectedParticles[ns2+i] = 0;  *mynps = stay - inj;
      } else {
        InjectedParticles[ns2+i] = inj - stay;  *mynps = 0;
      }
      npnext += tpni;
    }
    pnext[ps] = npnext;
  }
  set_sendbuf_disps(secondary, sec);
  state = oh1_state();
  if (nOfInjections)  move_injected_to_sendbuf(state);

  move_to_sendbuf_uw(state, 0, me, mynp[0],     /* &NOfPLocal[0][0][me] */
                     0, TotalP, 0, TotalPNext, state->recv_buffer_bases);

  if (secondary) {
    move_to_sendbuf_uw(state, 1, sec, mynp[1],  /* &NOfPLocal[1][0][sec] */
                       primaryParts, TotalP+ns, pnext[0], TotalPNext+ns,
                       state->recv_buffer_bases+ns);
    move_to_sendbuf_dw(state, 1, sec, mynp[1], totalParts, TotalP+ns,
                       pnext[0]+pnext[1], TotalPNext+ns);
  } else {
    struct S_particle *rbb=particle_at(state->particles, pnext[0]);
    for (s=0; s<ns; s++) {
      state->recv_buffer_bases[ns+s] = rbb;     /* RecvBufBases[1][s] */
      rbb = particle_at(rbb, TotalPNext[ns+s]); /* TotalPNext[1][s] */
    }
  }
  move_to_sendbuf_dw(state, 0, me, mynp[0], primaryParts, TotalP, pnext[0],
                     TotalPNext);

  set_sendbuf_disps(secondary, sec);
  if (nOfInjections) {
    move_injected_from_sendbuf(state, InjectedParticles+ns2, me,
                               state->recv_buffer_bases);
    if (sec>=0)
      move_injected_from_sendbuf(state, InjectedParticles+ns2+ns, sec,
                                 state->recv_buffer_bases+ns);
  }
  primaryParts = *secondaryBase = pnext[0];
}
void
set_sendbuf_disps(int secondary, int parent) {
  set_sendbuf_disps_state(oh1_state(), secondary, parent);
}
static void
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
exchange_particles(struct S_commlist *secrlist, int secrlsize, int oldparent,
                   int neighboring, int currmode, int stats) {
  struct oh_state *state;
  int me, nn, ns, nnns;
  int newparent;
  int s, i, req;

  move_to_sendbuf_secondary(Mode_PS(currmode), stats);
  state = oh1_state();
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
    struct S_particle **rbb=state->recv_buffer_bases;
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
                   struct S_particle **rbb) {
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
        struct S_particle *part=particle_at(particles, i);
        int dst=particle_subdomain(part, ps);
        if (dst<0) continue;
        copy_particle(particle_at(sendbuf, sbd[dst]++), part);
        if (dst==me) putme--;
      }
      for (; i<in; i++) {               /* move upward */
        struct S_particle *part=particle_at(particles, i);
        int dst=particle_subdomain(part, ps);
        if (dst<0) continue;
        if (dst==me) copy_particle(particle_at(particles, j++), part);
        else         copy_particle(particle_at(sendbuf, sbd[dst]++), part);
      }
      rbb[s] = particle_at(particles, j);        /* receive to bottom */
    } else if (jn>in) {                 /* downward only and thus skip */
      rbb[s] = particle_at(particles, j);        /* receive to top */
    } else {                            /* downward and upward */
      int ib, im, jm;
      for (; putme>0; i++) {            /* throw my particles to send buf */
        struct S_particle *part=particle_at(particles, i);
        int dst=particle_subdomain(part, ps);
        if (dst<0) continue;
        copy_particle(particle_at(sendbuf, sbd[dst]++), part);
        if (dst==me) putme--;
      }
      ib = i;
      for (; i<j; i++) {                 /* skip downward movers if any */
        int dst=particle_subdomain(particle_at(particles, i), ps);
        if (dst==me && dst>=0)  j++;
      }
      im = i-1; jm = j-1;
      for (; i<in; i++) {               /* move remainders upward */
        struct S_particle *part=particle_at(particles, i);
        int dst=particle_subdomain(part, ps);
        if (dst<0) continue;
        if (dst==me) copy_particle(particle_at(particles, j++), part);
        else         copy_particle(particle_at(sendbuf, sbd[dst]++), part);
      }
      rbb[s] = particle_at(particles, j);        /* receive to bottom */
      for (i=im,j=jm; i>=ib; i--) {     /* move first half downward if any */
        struct S_particle *part=particle_at(particles, i);
        int dst=particle_subdomain(part, ps);
        if (dst<0) continue;
        if (dst==me) copy_particle(particle_at(particles, j--), part);
        else         copy_particle(particle_at(sendbuf, sbd[dst]++), part);
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
      struct S_particle *part=particle_at(particles, i);
      int dst=particle_subdomain(part, ps);
      if (dst<0) continue;
      copy_particle(particle_at(sendbuf, sbd[dst]++), part);
      if (dst==me) putme--;
    }
    for (; i>=in; i--) {                /* move downward */
      struct S_particle *part=particle_at(particles, i);
      int dst=particle_subdomain(part, ps);
      if (dst<0) continue;
      if (dst==me) copy_particle(particle_at(particles, j--), part);
      else         copy_particle(particle_at(sendbuf, sbd[dst]++), part);
    }
  }
}
static void
move_injected_to_sendbuf(struct oh_state *state) {
  struct S_particle *pbuf=particle_at(state->particles, state->total_parts);
  int ninj=state->n_of_injections, nn=state->n_of_nodes;
  int i;

  for (i=0; i<ninj; i++) {
    struct S_particle *part=particle_at(pbuf, i);
    int dst = map_injected_particle_to_subdomain(part);
    int s = particle_species(part);
    if (dst<0) continue;
    copy_particle(particle_at(state->send_buffer,
                              state->send_buffer_disps[dst+s*nn]++), part);
  }
}
static void
move_injected_from_sendbuf(struct oh_state *state, int *injected, int mysd,
                           struct S_particle **rbb) {
  int nn=state->n_of_nodes, ns=state->n_of_species;
  int *sdisp=state->send_buffer_disps+mysd;
  int s, i;

  for (s=0; s<ns; s++,sdisp+=nn) {
    struct S_particle *rbuf=rbb[s];
    struct S_particle *sbuf=particle_at(state->send_buffer, *sdisp);
    int inj=injected[s];
    copy_particles(rbuf, sbuf, inj);
    rbb[s] = particle_at(rbb[s], inj);  *sdisp += inj;
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
      state->recv_buffer_bases[tag] = particle_at(rbuf, count);
      MPI_Irecv(rbuf, count, state->particle_mpi_type, rlist[i].sid, tag,
                state->comm, state->requests+(r++));
    }
    if (rlist[i].sid==me) {
      int count=rlist[i].count, tag=rlist[i].tag, region=rlist[i].region;
      region += nn * (tag<ns ? tag : tag-ns);
      sdisp = state->send_buffer_disps[region];
      state->send_buffer_disps[region] = sdisp + count;
                                                /* SendBufDisps[s][region] */
      MPI_Isend(particle_at(state->send_buffer, sdisp), count,
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
      MPI_Isend(particle_at(state->send_buffer, sdisp), count,
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
oh2_inject_particle(struct S_particle *part) {
  const int ns=nOfSpecies, nn=nOfNodes;
  int inj = totalParts + nOfInjections++;
  int s = particle_species(part);
  int n = particle_region(part, 0);

#ifndef OH_HAS_SPEC
  if (!useCustomParticleAdapter && ns!=1)
    local_errstop("particles cannot be injected when S_particle does not "
                  "have 'spec' element and you have two or more species");
#endif
  if (inj>=nOfLocalPLimit)
    local_errstop("injection causes local particle buffer overflow");
  copy_particle(particle_at(Particles, inj), part);
  if (n<0)  return;
  if (n==RegionId[1]) {
    NOfPLocal[(ns+s)*nn+n]++;
    InjectedParticles[ns+s]++;
  } else {
    NOfPLocal[nn*s+n]++;
    if (n==myRank)  InjectedParticles[s]++;
  }
}
void
oh2_remap_injected_particle_(struct S_particle *part) {
  oh2_remap_injected_particle(part);
}
void
oh2_remap_injected_particle(struct S_particle *part) {
  const int pidx = particle_buffer_index(part), ns=nOfSpecies, nn=nOfNodes;
  int s, n;

  if (pidx<totalParts || pidx>=totalParts+nOfInjections)
    local_errstop("'part' argument pointing %c%d%c of the particle buffer is "\
                  "not for injected particles",
                  specBase?'(':'[', pidx+specBase, specBase?')':']');
#ifndef OH_HAS_SPEC
  if (!useCustomParticleAdapter && ns!=1)
    local_errstop("particles cannot be injected when S_particle does not "
                  "have 'spec' element and you have two or more species");
#endif
  s = particle_species(part);
  n = particle_region(part, 0);
  if (n<0)  return;
  if (n==RegionId[1]) {
    NOfPLocal[(ns+s)*nn+n]++;
    InjectedParticles[ns+s]++;
  } else {
    NOfPLocal[nn*s+n]++;
    if (n==myRank)  InjectedParticles[s]++;
  }
}
void
oh2_remove_injected_particle_(struct S_particle *part) {
  oh2_remove_injected_particle(part);
}
void
oh2_remove_injected_particle(struct S_particle *part) {
  const int pidx = particle_buffer_index(part), ns=nOfSpecies, nn=nOfNodes;
  int s, n;

  if (pidx<totalParts || pidx>=totalParts+nOfInjections)
    local_errstop("'part' argument pointing %c%d%c of the particle buffer is "\
                  "not for injected particles",
                  specBase?'(':'[', pidx+specBase, specBase?')':']');
#ifndef OH_HAS_SPEC
  if (!useCustomParticleAdapter && ns!=1)
    local_errstop("particles cannot be injected when S_particle does not "
                  "have 'spec' element and you have two or more species");
#endif
  s = particle_species(part);
  n = particle_region(part, 0);
  if (n<0)  return;
  if (n==RegionId[1]) {
    NOfPLocal[(ns+s)*nn+n]--;
    InjectedParticles[ns+s]--;
  } else {
    NOfPLocal[nn*s+n]--;
    if (n==myRank)  InjectedParticles[s]--;
  }
  set_particle_region(part, -1, 0);
}
static int
particle_region(const struct S_particle *part, int primary_or_secondary) {
  return ParticleAdapter.get_region(part, primary_or_secondary);
}
static void
set_particle_region(struct S_particle *part, int region,
                    int primary_or_secondary) {
  ParticleAdapter.set_region(part, region, primary_or_secondary);
}
static int
particle_species(const struct S_particle *part) {
  int species = ParticleAdapter.get_species(part) - specBase;

#ifdef OH_HAS_SPEC
  return Particle_Spec(species);
#else
  return useCustomParticleAdapter ? species : 0;
#endif
}
static int
particle_subdomain(const struct S_particle *part, int primary_or_secondary) {
  Decl_Grid_Info();

  if (ParticleAdapter.map_to_subdomain)
    return ParticleAdapter.map_to_subdomain((void*)part, primary_or_secondary);
  return Subdomain_Id(particle_region(part, primary_or_secondary),
                      primary_or_secondary);
}
static int
map_injected_particle_to_subdomain(struct S_particle *part) {
  int dst;
  Decl_Grid_Info();

  if (ParticleAdapter.map_to_subdomain)
    return ParticleAdapter.map_to_subdomain(part, 0);
  dst = Subdomain_Id(particle_region(part, 0), 0);
#ifdef OH_POS_AWARE
  if (dst>=nOfNodes)  Primarize_Id(part, dst);
#endif
  return dst;
}
static size_t
particle_stride(void) {
  return oh_particle_buffer_stride(&ParticleAdapter);
}
static struct S_particle *
particle_at(struct S_particle *base, int index) {
  return oh_particle_buffer_at(&ParticleAdapter, base, index);
}
static int
particle_buffer_index(const struct S_particle *part) {
  return oh_particle_buffer_index(&ParticleAdapter, Particles, part);
}
static void
copy_particle(struct S_particle *dst, const struct S_particle *src) {
  oh_particle_buffer_copy(&ParticleAdapter, dst, src);
}
static void
copy_particles(struct S_particle *dst, const struct S_particle *src, int count) {
  oh_particle_buffer_copy_n(&ParticleAdapter, dst, src, count);
}
void
oh2_set_total_particles_() {
  set_total_particles();
}
void
oh2_set_total_particles() {
  set_total_particles();
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
