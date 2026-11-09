/* Exercise the actual count-based rebalance tree before MPI scheduling. */
#ifndef OHHELP1_SOURCE
#define OHHELP1_SOURCE "../src/c/ohhelp1.c"
#endif
#include OHHELP1_SOURCE
#include <assert.h>

static void trial(const dint *initial, int nn, int oldparent) {
  struct oh_state state={0};
  struct S_node nodes[256]={0}, next[256]={0}, *queue[256]={0};
  dint counts[256], total=0;
  int local[512]={0}, less[514]={0}, greater[514]={0}, i;
  double loads[256], weights[256];
  state.n_of_nodes=nn; state.n_of_species=1; state.my_rank=0;
  state.nodes=nodes; state.nodes_next=next; state.node_queue=queue;
  state.total_particles_global=counts; state.total_load_global=loads;
  state.region_weights=weights; state.n_of_particles_local=local;
  state.less_heap.node=less; state.less_heap.index=less+257;
  state.greater_heap.node=greater; state.greater_heap.index=greater+257;
  for(i=0;i<nn;i++) {
    counts[i]=initial[i]; total+=counts[i]; loads[i]=counts[i]; weights[i]=1;
    nodes[i].id=i; nodes[i].parentid=(i==oldparent ? -1 : oldparent);
  }
  state.n_of_particles=total;
  rebalance1_state(&state,MODE_NORM_SEC,-1,0);
  {
    dint accounted=0;
    for(i=0;i<nn;i++) {
      struct S_node *ch;
      dint region=counts[i], process=counts[i]+next[i].get.sec;
      assert(counts[i]>=0);
      assert(process>=total/nn && process<=(total+nn-1)/nn);
      for(ch=next[i].child;ch;ch=ch->sibling) region+=ch->get.sec;
      assert(region==initial[i]);
      accounted+=process;
    }
    assert(accounted==total);
  }
}
int main(int argc,char **argv) {
  dint p[256]={0,2,2,1}; unsigned rng=20261109; int i,t;
  MPI_Init(&argc,&argv);
  trial(p,4,3); /* old parent has 1 but ceil-average receiver asks for 2 */
  p[1]=p[2]=p[3]=0;trial(p,4,3); /* empty system */
  p[3]=1;trial(p,4,3); /* fewer particles than processes */
  for(t=0;t<10000;t++) {
    int nn=4+(t%253);
    for(i=0;i<nn;i++) {rng=rng*1664525u+1013904223u;p[i]=rng%1000;}
    trial(p,nn,t%nn);
  }
  puts("PASS: actual rebalance old-parent rounding regression and 10000 conserved trees");
  MPI_Finalize();return 0;
}
