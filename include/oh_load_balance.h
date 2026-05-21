/* File: oh_load_balance.h
   v2 load-balance helpers independent of MPI and particle layout.
*/
#ifndef OH_LOAD_BALANCE_H
#define OH_LOAD_BALANCE_H

#ifdef __cplusplus
extern "C" {
#endif

double oh_region_load(long long particle_count, double region_weight);
double oh_load_limit(double total_load, int max_fraction, int node_count);
int oh_particles_for_load(double load, double region_weight,
                          long long max_available);

#ifdef __cplusplus
}
#endif

#endif
