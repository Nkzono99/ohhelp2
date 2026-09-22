/* Same assertions exercise AoS and native SoA in separate MPI invocations. */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ohhelp_c.h"
#if defined(OH_LIB_LEVEL_4P)
#include "ohhelp4p.h"
#define API(name) oh4p_##name
#else
#include "ohhelp4s.h"
#define API(name) oh4s_##name
#endif
struct particle { long long region, id; double x,y,z; int species; unsigned char opaque[3]; };
static int soa,capacity;
static oh_context *ctx;
static struct particle *aos;
static long long *regions,*ids;
static double *xs,*ys,*zs;
static int *species;
static unsigned char (*opaque)[3];
static void *borrowed[7];
/* Check that the bridge restores the legacy default-context AoS configuration. */
extern oh_particle_adapter CustomParticleAdapter;
extern MPI_Datatype CustomTParticle;
extern int useCustomParticleAdapter, useCustomTParticle;
static void bind_borrowed(const oh_particle_field *fields) {
  for (int f=0;f<7;++f) {borrowed[f]=calloc((size_t)capacity,fields[f].size);assert(borrowed[f]);}
  assert(oh_context_bind_soa_particles(ctx,borrowed,7,capacity,OH_PARTICLES_BORROWED)==MPI_SUCCESS);
}
static void put(int i, struct particle p) {
  if (!soa) { aos[i]=p; return; }
  regions[i]=p.region; ids[i]=p.id; xs[i]=p.x; ys[i]=p.y; zs[i]=p.z;
  species[i]=p.species; memcpy(opaque[i],p.opaque,3);
}
static struct particle get(int i) {
  struct particle p;
  if (!soa) return aos[i];
  p.region=regions[i]; p.id=ids[i]; p.x=xs[i]; p.y=ys[i]; p.z=zs[i]; p.species=species[i];
  memcpy(p.opaque,opaque[i],3); return p;
}
static int map(int i,int ps,int sp,int neighbor) {
  if (soa) return neighbor ? API(map_soa_particle_to_neighbor)(i,ps,sp) : API(map_soa_particle_to_subdomain)(i,ps,sp);
  return neighbor ? API(map_particle_to_neighbor)(aos+i,ps,sp) : API(map_particle_to_subdomain)(aos+i,ps,sp);
}
static int transfer(int mode) {
  mode=API(transbound)(mode,0);
  if (!soa) { extern void *Particles; aos=Particles; }
  return mode;
}
int main(int argc,char **argv) {
  int full_receive=argc>2 && !strcmp(argv[2],"full-receive");
  int saved_type=argc>2 && !strcmp(argv[2],"saved-type");
  int rank,n,mode,weighted=argc>2 && !strcmp(argv[2],"weighted");
  int *sdid=NULL,*total=NULL,*base=NULL,*nbor=NULL,*sdoms=NULL,*bounds=NULL,*fsizes=NULL,*hgram=NULL;
  void *buffer=NULL;
  int pc[3]={1,1,1},sc[6]={0,1,0,1,0,1},bc[6]={0};
  int ft[1][OH_FTYPE_N]={{-1,0,0,0,0,0,0}},cf[]={-1},ct[2][2][OH_CTYPE_N]={{{0}}};
  oh_particle_field fields[]={
    {0,OH_PARTICLE_FIELD_LONG_LONG,OH_PARTICLE_ROLE_REGION,sizeof(long long)},
    {1,OH_PARTICLE_FIELD_LONG_LONG,OH_PARTICLE_ROLE_PAYLOAD,sizeof(long long)},
    {2,OH_PARTICLE_FIELD_DOUBLE,OH_PARTICLE_ROLE_X,sizeof(double)},
    {3,OH_PARTICLE_FIELD_DOUBLE,OH_PARTICLE_ROLE_Y,sizeof(double)},
    {4,OH_PARTICLE_FIELD_DOUBLE,OH_PARTICLE_ROLE_Z,sizeof(double)},
    {5,OH_PARTICLE_FIELD_INT,OH_PARTICLE_ROLE_SPECIES,sizeof(int)},
    {6,OH_PARTICLE_FIELD_BYTES,OH_PARTICLE_ROLE_PAYLOAD,3}
  };
  MPI_Init(&argc,&argv); MPI_Comm_rank(MPI_COMM_WORLD,&rank);MPI_Comm_size(MPI_COMM_WORLD,&n);
  soa=argc>1 && strcmp(argv[1],"aos");ctx=oh_default_context();
  MPI_Datatype previous_type=MPI_DATATYPE_NULL;
  if (saved_type) {
    assert(oh_particle_adapter_make_byte_type(sizeof(struct S_particle),&previous_type)==MPI_SUCCESS);
    oh2_set_particle_mpi_type(previous_type);
  } else {
    oh_particle_adapter a;MPI_Datatype type;
    assert(oh_particle_adapter_make_byte_type(sizeof(struct particle),&type)==MPI_SUCCESS);
    a=oh_default_particle_adapter(type); a.stride=sizeof(struct particle);
    oh_particle_adapter_use_integer_fields(&a,offsetof(struct particle,region),sizeof(long long),offsetof(struct particle,species),sizeof(int));
    oh_particle_adapter_use_position_fields(&a,offsetof(struct particle,x),offsetof(struct particle,y),offsetof(struct particle,z));
    a.map_to_neighbor=NULL; a.map_to_subdomain=NULL;
    a.species_base=1;oh2_set_particle_adapter(&a);MPI_Type_free(&type);
  }
  if (soa) {
    assert(oh_context_set_particle_storage(ctx,OH_PARTICLE_STORAGE_SOA)==MPI_SUCCESS);
    assert(oh_context_configure_soa(ctx,fields,7,1)==MPI_SUCCESS);
  }
  pc[0]=n; sc[1]=n;
#if defined(OH_LIB_LEVEL_4P)
  capacity=oh4p_max_local_particles(n*4,100,8,2);
  if (soa && !strcmp(argv[1],"borrowed")) bind_borrowed(fields);
  oh4p_init(&sdid,2,full_receive?100:20,&total,&buffer,&base,capacity,NULL,&nbor,pc,&sdoms,sc,1,bc,&bounds,&ft[0][0],cf,&ct[0][0][0],&fsizes,0,0,0);
  oh4p_per_grid_histogram(&hgram);
#else
  int cbufsize,*zbound=NULL,*pgindex=NULL;
  for (int i=0;i<6;++i) bc[i]=1;
  oh4s_init(&sdid,2,20,n*4,8,4,&total,&base,&capacity,&cbufsize,NULL,&nbor,pc,&sdoms,sc,2,bc,&bounds,&ft[0][0],cf,&ct[0][0][0],&fsizes,&zbound,0,0,0);
  if (soa && !strcmp(argv[1],"borrowed")) bind_borrowed(fields);
  oh4s_particle_buffer(capacity,&buffer);oh4s_per_grid_histogram(&hgram,&pgindex);
#endif
  if (soa) {
    assert(buffer==NULL);
    regions=oh_context_soa_field(ctx,0); ids=oh_context_soa_field(ctx,1);
    xs=oh_context_soa_field(ctx,2);ys=oh_context_soa_field(ctx,3);zs=oh_context_soa_field(ctx,4);
    species=oh_context_soa_field(ctx,5);opaque=oh_context_soa_field(ctx,6);
    assert(regions && ids && xs && ys && zs && species && opaque);
  } else aos=buffer;
  if (argc>2 && !strcmp(argv[2],"pointer-api")) {
    struct particle invalid={0}; API(map_particle_to_neighbor)(&invalid,0,0);
    MPI_Abort(MPI_COMM_WORLD,99);
  }
  if (argc>2 && !strcmp(argv[2],"bad-index")) {
    API(map_soa_particle_to_neighbor)(capacity,0,0);MPI_Abort(MPI_COMM_WORLD,99);
  }
#ifdef OH_LIB_LEVEL_4P
  if (full_receive) {
    assert(n==2);
    if (rank==1) for(int i=0;i<capacity;++i) {
      int sp=i>=capacity/2;
      struct particle p={0,i,0.5,0.5,0.5,sp+1,{3,7,29}};
      put(i,p);assert(map(i,0,sp,0)==0);
    }
    mode=transfer(OH_MODE_NORMAL_PRIMARY);
    assert(mode==OH_MODE_NORMAL_PRIMARY && base[2]==(rank==0?capacity:0));
    for(int i=0;i<base[2];++i) {struct particle p=get(i);assert(p.id==i && p.opaque[2]==29);}
    if(soa) oh_context_unbind_particles(ctx);
    MPI_Finalize();return 0;
  }
#endif
  for(int sp=0;sp<2;++sp) {
    struct particle p={0,rank*2+sp,rank+0.5,0.5,0.5,sp+1,{(unsigned char)rank,(unsigned char)sp,231}};
    put(sp,p);assert(map(sp,0,sp,0)==rank);
  }
  if (weighted) {double *w=malloc((size_t)n*sizeof(double));for(int i=0;i<n;++i)w[i]=i?1:16;oh1_set_region_weights(w);free(w);}
  mode=transfer(OH_MODE_NORMAL_PRIMARY);
  if (weighted && n>1) assert(mode==OH_MODE_REBALANCE_SECONDARY);
  /* Boundary ghosts have region -2. Count each physical particle only once. */
  for (int step=0;step<3;++step) {
#ifdef OH_LIB_LEVEL_4S
    long long *field=calloc((size_t)capacity,sizeof(long long));
    long long *send=calloc((size_t)capacity*4,sizeof(long long));
    long long *recv=calloc((size_t)capacity*4,sizeof(long long));
    assert(field && send && recv);
    for(int i=0;i<base[2];++i) {struct particle p=get(i);field[i]=p.region<0 ? -999 : p.id+500;}
    oh4s_exchange_border_data(field,send,recv,MPI_LONG_LONG);
    for(int i=0;i<base[2];++i) assert(field[i]==get(i).id+500);
    free(field);free(send);free(recv);
#endif
    int physical=0;long long sum=0;
    for(int i=0;i<base[2];++i) {
      struct particle p=get(i);assert(p.id>=0 && p.id<2*n);
      if (!(p.opaque[0]==p.id/2 && p.opaque[1]==p.id%2 && p.opaque[2]==231))
        fprintf(stderr,"bad payload: rank=%d soa=%d step=%d slot=%d id=%lld region=%lld xyz=%g,%g,%g species=%d bytes=%d,%d,%d total=%d\n",rank,soa,step,i,p.id,p.region,p.x,p.y,p.z,p.species,p.opaque[0],p.opaque[1],p.opaque[2],base[2]);
      assert(p.opaque[0]==p.id/2 && p.opaque[1]==p.id%2 && p.opaque[2]==231);
      assert(p.species==p.id%2+1);assert(p.y==0.5 && p.z==0.5);
      assert(p.x>=0 && p.x<n);
      if(p.region<0) continue;
      ++physical;sum+=p.id;
      int ps=i>=base[1],sp=p.species-1;
      if (step==1) {
#ifdef OH_LIB_LEVEL_4P
        p.x=p.id/2+1.5; /* Last rank crosses the periodic upper boundary. */
#else
        p.x=(p.id/2+1)%n+0.5;
#endif
        put(i,p);
      }
      assert(map(i,ps,sp,step!=1)>=0);
    }
    int global;long long totalid;
    MPI_Allreduce(&physical,&global,1,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
    MPI_Allreduce(&sum,&totalid,1,MPI_LONG_LONG,MPI_SUM,MPI_COMM_WORLD);
    if(global!=2*n) fprintf(stderr,"bad count: rank=%d soa=%d step=%d local=%d global=%d mode=%d\n",rank,soa,step,physical,global,mode);
    assert(global==2*n);assert(totalid==(long long)n*(2*n-1));
    mode=transfer(mode);
  }
  for (int i=0;i<base[2];++i) {
    struct particle old=get(i);
    if (old.region>=0) assert(map(i,i>=base[1],old.species-1,1)>=0);
  }
  /* Injection owns a copied slot; remove/remap leaves exactly one contribution. */
  struct particle p={0,100+rank,rank+0.5,0.5,0.5,1,{17,19,23}};
  int injected_index=base[2],dst; oh_particle_ref ref={0};
  if (soa) {
    const void *values[]={&p.region,&p.id,&p.x,&p.y,&p.z,&p.species,p.opaque};
    assert(API(inject_soa_particle)(values,7,0,&ref,&dst)==MPI_SUCCESS);
    assert(ref.index==injected_index && dst==rank);
    assert(oh_context_particle_ref_valid(ctx,ref));
    API(remove_mapped_soa_particle)(ref.index,0,0);assert(regions[ref.index]==-1);
    assert(API(remap_soa_particle_to_neighbor)(ref.index,0,0)==rank);
  } else {
    assert(API(inject_particle)(&p,0)==rank);
    API(remove_mapped_particle)(aos+injected_index,0,0);
    assert(API(remap_particle_to_neighbor)(aos+injected_index,0,0)==rank);
  }
  /* Every rank added one physical particle with its opaque payload intact. */
  mode=transfer(mode);
  if(soa) assert(!oh_context_particle_ref_valid(ctx,ref));
  int physical=0;
  for(int i=0;i<base[2];++i) {p=get(i);if(p.id>=100) {assert(p.id<100+n);assert(p.opaque[2]==23);} if(p.region>=0)++physical; p.region=-1;put(i,p);}
  int all;MPI_Allreduce(&physical,&all,1,MPI_INT,MPI_SUM,MPI_COMM_WORLD);assert(all==3*n);
  mode=transfer(mode);assert(base[2]==0);
#ifdef OH_LIB_LEVEL_4S
  /* Neighbor injection crosses the physical boundary from its adjacent rank. */
  if(rank==0) {
    p.x=-0.5;p.y=p.z=0.5;p.species=1;
    if(soa) {
      const void *values[]={&p.region,&p.id,&p.x,&p.y,&p.z,&p.species,p.opaque};
      assert(API(inject_soa_particle)(values,7,0,&ref,&dst)==MPI_SUCCESS);
      assert(dst==-1 && !oh_context_particle_ref_valid(ctx,ref));
    } else assert(API(inject_particle)(&p,0)==-1);
  }
  mode=transfer(mode);assert(base[2]==0);
#endif
  if (soa) {
    if(borrowed[0]) assert(regions==borrowed[0]);
    oh_context_unbind_particles(ctx);
    for(int f=0;f<7;++f) free(borrowed[f]);
    assert(oh_context_set_particle_storage(ctx,OH_PARTICLE_STORAGE_AOS)==MPI_SUCCESS);
    assert(useCustomTParticle && useCustomParticleAdapter==!saved_type);
    MPI_Aint lower,extent;
    assert(MPI_Type_get_extent(CustomTParticle,&lower,&extent)==MPI_SUCCESS);
    assert(lower==0 && extent==(MPI_Aint)(saved_type?sizeof(struct S_particle):sizeof(struct particle)));
    if (!saved_type) {
      assert(CustomParticleAdapter.stride==sizeof(struct particle));
      assert(CustomParticleAdapter.species_base==1);
      assert(CustomParticleAdapter.get_region(&CustomParticleAdapter,&p,0)==p.region);
    }
    oh2_set_particle_adapter(NULL);
  }
  if(previous_type!=MPI_DATATYPE_NULL) MPI_Type_free(&previous_type);
  MPI_Finalize();return 0;
}
