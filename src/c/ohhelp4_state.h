/* Shared Level-4 state binding for the default-context migration. */
#ifndef OHHELP4_STATE_H
#define OHHELP4_STATE_H

static inline void
level4_bind_common_state(struct oh_state* state) {
    state->level4_grid_desc = GridDesc;
    state->level4_pbuf_index = PbufIndex;
    state->level4_particle_grid[0] = NOfPGrid[0];
    state->level4_particle_grid[1] = NOfPGrid[1];
    state->level4_particle_grid_total[0] = NOfPGridTotal[0];
    state->level4_particle_grid_total[1] = NOfPGridTotal[1];
    state->level4_particle_grid_out[0] = NOfPGridOut[0];
    state->level4_particle_grid_out[1] = NOfPGridOut[1];
    state->level4_alt_sec_recv_list = AltSecRList;
    state->level4_sec_rl_index = SecRLIndex;
    state->level4_histogram_half_type = T_Hgramhalf;
    state->level4_first_neighbor = FirstNeighbor;
    state->level4_grid_offset = &GridOffset[0][0];
    state->level4_real_dst_neighbors = RealDstNeighbors;
    state->level4_real_src_neighbors = RealSrcNeighbors;
    state->level4_boundary_condition = &BoundaryCondition[0][0];
}

#endif
