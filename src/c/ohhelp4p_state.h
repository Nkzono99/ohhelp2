/* Level-4p-specific state binding for the default-context migration. */
#ifndef OHHELP4P_STATE_H
#define OHHELP4P_STATE_H

#include "ohhelp4_state.h"

static inline void
level4p_bind_state(struct oh_state* state) {
    level4_bind_common_state(state);
    state->level4_particle_grid_index[0] = NULL;
    state->level4_particle_grid_index[1] = NULL;
    state->level4_particle_grid_out_shadow[0] = NULL;
    state->level4_particle_grid_out_shadow[1] = NULL;
    state->level4_particle_grid_index_shadow[0] = NULL;
    state->level4_particle_grid_index_shadow[1] = NULL;
    state->level4_particle_grid_z = NULL;
    state->level4_hotspot_recv = HSRecv;
    state->level4_hotspot_send = HSSend;
    state->level4_hotspot_recv_from_parent = HSRecvFromParent;
    state->level4_hotspot_receiver = HSReceiver;
    state->level4_hotspot_list = HotSpotList;
    state->level4_hotspot_top = HotSpotTop;
    state->level4_hotspots = &HotSpot[0][0];
    state->level4_horizontal_planes = NULL;
    state->level4_vertical_planes = NULL;
    state->level4_vertical_plane_head = NULL;
    state->level4_primary_comm_list = NULL;
    state->level4_alt_sec_rl_index = NULL;
    state->level4_primary_rl_index = NULL;
    state->level4_interior_parts = NULL;
    state->level4_grid_overflow_limit = gridOverflowLimit;
    state->level4_boundary_send_buffer = NULL;
    state->level4_z_bound = NULL;
    state->level4_z_bound_shadow = NULL;
}

#endif
