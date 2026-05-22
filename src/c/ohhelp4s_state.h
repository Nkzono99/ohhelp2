/* Level-4s-specific state binding for the default-context migration. */
#ifndef OHHELP4S_STATE_H
#define OHHELP4S_STATE_H

#include "ohhelp4_state.h"

static inline void
level4s_bind_state(struct oh_state* state) {
    level4_bind_common_state(state);
    state->level4_particle_grid_index[0] = NOfPGridIndex[0];
    state->level4_particle_grid_index[1] = NOfPGridIndex[1];
    state->level4_particle_grid_out_shadow[0] = NOfPGridOutShadow[0];
    state->level4_particle_grid_out_shadow[1] = NOfPGridOutShadow[1];
    state->level4_particle_grid_index_shadow[0] = NOfPGridIndexShadow[0];
    state->level4_particle_grid_index_shadow[1] = NOfPGridIndexShadow[1];
    state->level4_particle_grid_z = NOfPGridZ;
    state->level4_hotspot_recv = NULL;
    state->level4_hotspot_send = NULL;
    state->level4_hotspot_recv_from_parent = NULL;
    state->level4_hotspot_receiver = NULL;
    state->level4_hotspot_list = NULL;
    state->level4_hotspot_top = NULL;
    state->level4_hotspots = NULL;
    state->level4_horizontal_planes = &HPlane[0][0];
    state->level4_vertical_planes = VPlane;
    state->level4_vertical_plane_head = VPlaneHead;
    state->level4_primary_comm_list = &PrimaryCommList[0][0];
    state->level4_alt_sec_rl_index = AltSecRLIndex;
    state->level4_primary_rl_index = PrimaryRLIndex;
    state->level4_interior_parts = InteriorParts;
    state->level4_grid_overflow_limit = 0;
    state->level4_boundary_send_buffer = BoundarySendBuf;
    state->level4_z_bound = &ZBound[0][0];
    state->level4_z_bound_shadow = ZBoundShadow ? &ZBoundShadow[0][0] : NULL;
}

#endif
