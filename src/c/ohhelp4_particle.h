/* Internal Level-4 particle adapter helpers.
   These helpers keep Level-4p/4s particle movement layout-agnostic while the
   packed region-id contract is still being migrated. */
#ifndef OHHELP4_PARTICLE_H
#define OHHELP4_PARTICLE_H

#include "oh_particle_buffer.h"

static inline struct S_particle*
level4_particle_at(struct oh_state* state, int index) {
    return oh_particle_buffer_at(state->particle_adapter,
                                 state->particles, index);
}

static inline int
level4_particle_index(struct oh_state* state, const struct S_particle* part) {
    return oh_particle_buffer_index(state->particle_adapter,
                                    state->particles, part);
}

static inline int
level4_particle_is_injected(struct oh_state* state,
                            const struct S_particle* part) {
    return level4_particle_index(state, part) >= state->total_parts;
}

static inline OH_nid_t
level4_particle_region(struct oh_state* state, const struct S_particle* part,
                       int primary_or_secondary) {
    return (OH_nid_t)state->particle_adapter->get_region(
        state->particle_adapter, part, primary_or_secondary);
}

static inline double*
level4_particle_position(struct oh_state* state, struct S_particle* part,
                         int dim) {
    return oh_particle_adapter_position(state->particle_adapter, part, dim);
}

static inline int
level4_particle_species(struct oh_state* state, const struct S_particle* part) {
    return Particle_Spec(
        state->particle_adapter->get_species(state->particle_adapter, part) -
        state->spec_base);
}

static inline void
level4_set_particle_region(struct oh_state* state, struct S_particle* part,
                           OH_nid_t region, int primary_or_secondary) {
    state->particle_adapter->set_region(state->particle_adapter, part, region,
                                        primary_or_secondary);
}

static inline void
level4_copy_particle(struct oh_state* state, struct S_particle* dst,
                     const struct S_particle* src) {
    oh_particle_buffer_copy(state->particle_adapter, dst, src);
}

static inline void
level4_push_particle(struct oh_state* state, struct S_particle** cursor,
                     const struct S_particle* src) {
    level4_copy_particle(state, *cursor, src);
    *cursor = oh_particle_buffer_at(state->particle_adapter, *cursor, 1);
}

static inline void
level4_copy_particle_to_buffer(struct oh_state* state, struct S_particle* base,
                               int index, const struct S_particle* src) {
    level4_copy_particle(state,
                         oh_particle_buffer_at(state->particle_adapter,
                                               base, index),
                         src);
}

static inline size_t
level4_init_particle_stride(void) {
    if (useCustomParticleAdapter)
        return oh_particle_buffer_stride(&CustomParticleAdapter);
    return sizeof(struct S_particle);
}

static inline struct S_particle*
level4_init_particle_at(struct S_particle* base, int index) {
    if (useCustomParticleAdapter)
        return oh_particle_buffer_at(&CustomParticleAdapter, base, index);
    return base + index;
}

static inline int
level4_grid_position(struct oh_state* state, OH_nid_t region) {
    return (int)(region & state->grid_mask);
}

static inline OH_nid_t
level4_combine_subdomain_position(struct oh_state* state, int subdomain,
                                  int grid_position) {
    return (((OH_nid_t)subdomain << state->log_grid) + grid_position);
}

static inline int
level4_subdomain_id(struct oh_state* state, OH_nid_t region,
                    int primary_or_secondary) {
    int subdomain;

    if (region < 0) return -1;
    subdomain = (int)(region >> state->log_grid);
    if (subdomain < OH_NEIGHBORS)
        return state->abs_neighbors[primary_or_secondary][subdomain];
    return subdomain - OH_NEIGHBORS;
}

static inline int
level4_neighbor_subdomain_id(struct oh_state* state, OH_nid_t region,
                             int primary_or_secondary) {
    return state->abs_neighbors[primary_or_secondary]
                               [(int)(region >> state->log_grid)];
}

static inline int
level4_primarize_particle(struct oh_state* state, struct S_particle* part) {
    const OH_nid_t offset =
        ((OH_nid_t)(state->n_of_nodes + OH_NEIGHBORS) << state->log_grid);
    const OH_nid_t region = level4_particle_region(state, part, 1) - offset;

    level4_set_particle_region(state, part, region, 1);
    return level4_subdomain_id(state, region, 1);
}

static inline void
level4_primarize_particle_only(struct oh_state* state,
                               struct S_particle* part) {
    const OH_nid_t offset =
        ((OH_nid_t)(state->n_of_nodes + OH_NEIGHBORS) << state->log_grid);

    level4_set_particle_region(
        state, part, level4_particle_region(state, part, 1) - offset, 1);
}

static inline void
level4_secondarize_particle(struct oh_state* state, struct S_particle* part) {
    const OH_nid_t offset =
        ((OH_nid_t)(state->n_of_nodes + OH_NEIGHBORS) << state->log_grid);

    level4_set_particle_region(
        state, part, level4_particle_region(state, part, 1) + offset, 1);
}

static inline int
level4_secondary_injected(struct oh_state* state, OH_nid_t region) {
    return (int)((region >> state->log_grid) >=
                 state->n_of_nodes + OH_NEIGHBORS);
}

static inline int
level4_local_grid_position(struct oh_state* state, int grid_position,
                           OH_nid_t region, int primary_or_secondary) {
    const int neighbor = (int)(region >> state->log_grid);
    return grid_position +
           state->level4_grid_offset[primary_or_secondary * OH_NEIGHBORS +
                                     neighbor];
}

#endif
