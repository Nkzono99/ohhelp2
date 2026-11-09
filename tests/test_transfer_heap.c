/* Exercise the actual scheduler heap, including arbitrary-parent removal. */
#ifndef OHHELP1_SOURCE
#define OHHELP1_SOURCE "../src/c/ohhelp1.c"
#endif
#include OHHELP1_SOURCE
#include <assert.h>

static void check_heap(struct S_heap *h, int greater,
                       const struct S_heap_key *key) {
  int i;
  for (i=1; i<=h->n; i++) {
    assert(h->index[h->node[i]]==i);
    if (i>1) {
      int child=h->node[i], parent=h->node[i>>1];
      double c=key->weighted ? key->loads[child] : key->particles[child];
      double p=key->weighted ? key->loads[parent] : key->particles[parent];
      if (greater) assert(c<=p);
      else assert(c>=p);
    }
  }
}

int main(void) {
  dint particles[128]={100,60,90,50,55,80,85};
  double loads[128];
  int storage[257]={0};
  struct S_heap h={0};
  struct S_heap_key key={particles,loads,0};
  int i, greater, trial, weighted;
  unsigned rng=1739;
  h.node=storage; h.index=storage+129;
  for (i=0; i<7; i++) push_heap(i,&h,1,&key);
  remove_heap(&h,1,h.index[3],&key); /* last 85 must sift above parent 60 */
  check_heap(&h,1,&key);
  {
    const int expected[]={100,90,85,80,60,55};
    for (i=0; i<6; i++) assert(particles[pop_heap(&h,1,&key)]==expected[i]);
  }
  for (weighted=0;weighted<=1;weighted++)
  for (greater=0; greater<=1; greater++) for (trial=0; trial<2000; trial++) {
    key.weighted=weighted;
    h.n=0;
    for (i=0; i<128; i++) {
      rng=rng*1664525u+1013904223u;particles[i]=rng%20001u;
      rng=rng*1664525u+1013904223u;loads[i]=(rng%40001u)*0.25;
      push_heap(i,&h,greater,&key);
    }
    while (h.n) {
      rng=rng*1664525u+1013904223u;
      remove_heap(&h,greater,1+(rng%(unsigned)h.n),&key);
      check_heap(&h,greater,&key);
    }
  }
  puts("PASS: donor heap regression and 1024000 count/weighted min/max removals");
  return 0;
}
