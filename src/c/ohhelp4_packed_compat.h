/* Internal Level-4 packed-region compatibility macros.
   These keep legacy call sites compiling while packed-id semantics are moved
   behind ohhelp4_particle.h helpers.  Call sites must have a local variable
   named `state` that points to the active oh_state. */
#ifndef OHHELP4_PACKED_COMPAT_H
#define OHHELP4_PACKED_COMPAT_H

#ifdef OH_POS_AWARE
#undef Decl_Grid_Info
#undef Grid_Position
#undef Combine_Subdom_Pos
#undef Subdomain_Id
#undef Primarize_Id
#undef Primarize_Id_Only
#undef Secondarize_Id
#undef Secondary_Injected
#undef Neighbor_Subdomain_Id
#define Decl_Grid_Info()
#define Grid_Position(ID) level4_grid_position(state, ID)
#define Combine_Subdom_Pos(ID, G) \
  level4_combine_subdomain_position(state, ID, G)
#define Subdomain_Id(ID, PS) \
  level4_subdomain_id(state, ID, PS)
#define Primarize_Id(P, SD) \
  do { SD = level4_primarize_particle(state, P); } while (0)
#define Primarize_Id_Only(P) \
  level4_primarize_particle_only(state, P)
#define Secondarize_Id(P) \
  level4_secondarize_particle(state, P)
#define Secondary_Injected(ID) \
  level4_secondary_injected(state, ID)
#define Neighbor_Subdomain_Id(ID, PS) \
  level4_neighbor_subdomain_id(state, ID, PS)
#endif

#endif
