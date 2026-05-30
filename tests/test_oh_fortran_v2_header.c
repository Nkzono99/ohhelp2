#include "oh_fortran_v2.h"
#include "oh_fortran_v2.h"

int
main(int argc, char **argv) {
  oh_context *context = 0;
  oh_fortran_particle_adapter *adapter = 0;
  struct S_mycommf mycomm = {0, 0, 0, 0, 0};
  int sdid[2] = {0, -1};
  int nphgram[2] = {0, 0};
  int totalp[2] = {0, 0};
  int pbase[3] = {0, 0, 0};
  int nbor[3] = {0, 0, 0};
  int pcoord[1] = {1};
  int scoord[2] = {0, 1};
  int bcond[2] = {0, 0};
  int ftypes[16] = {0};
  int cfields[1] = {-1};
  int ctypes[6] = {0};
  int fsizes[2] = {0, 0};
  void *pbuf = 0;
  int (*create_adapter)(size_t, oh_fortran_particle_adapter**) =
    oh_fortran_particle_adapter_create_byte;
  void (*destroy_context)(oh_context*) = oh_fortran_context_destroy;

  if (argc == 12345) {
    (void)create_adapter(sizeof(int), &adapter);
    oh_fortran_context_configure_particles(context, 1, 20);
    oh_fortran_oh2_init_raw(sdid, 1, 20, nphgram, totalp, &pbuf, pbase, 4,
                            &mycomm, nbor, pcoord, 0, 0, 0);
    oh_fortran_oh3_init_raw(sdid, 1, 20, nphgram, totalp, &pbuf, pbase, 4,
                            &mycomm, nbor, pcoord, 0, scoord, 1, bcond, 0,
                            ftypes, cfields, ctypes, fsizes, 0, 0, 0);
    destroy_context(context);
    oh_fortran_particle_adapter_destroy(adapter);
  }

  (void)argv;
  return 0;
}
