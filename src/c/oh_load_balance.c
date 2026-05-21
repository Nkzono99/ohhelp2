/* File: oh_load_balance.c
   v2 load-balance helpers independent of MPI and particle layout.
*/
#include <limits.h>

#include "oh_load_balance.h"

double
oh_region_load(long long particle_count, double region_weight) {
  if (particle_count <= 0 || region_weight <= 0.0) return 0.0;
  return (double)particle_count * region_weight;
}

double
oh_load_limit(double total_load, int max_fraction, int node_count) {
  if (total_load <= 0.0 || node_count <= 0) return 0.0;
  return total_load * (100.0 + (double)max_fraction) / 100.0
       / (double)node_count;
}

int
oh_particles_for_load(double load, double region_weight, long long max_available) {
  long long count;

  if (load <= 0.0 || region_weight <= 0.0 || max_available <= 0) return 0;
  count = (long long)(load / region_weight);
  if ((double)count * region_weight < load) count++;
  if (count < 1) count = 1;
  if (count > max_available) count = max_available;
  if (count > INT_MAX) count = INT_MAX;
  return (int)count;
}

double
oh_load_after_transfer(double current_load, long long particle_count,
                       double region_weight) {
  double moved_load = oh_region_load(particle_count, region_weight);

  if (moved_load >= current_load) return 0.0;
  return current_load - moved_load;
}

int
oh_weighted_transfer_count(double target_load, double receiver_load,
                           double donor_region_weight,
                           long long donor_particles) {
  double deficit = target_load - receiver_load;

  return oh_particles_for_load(deficit, donor_region_weight, donor_particles);
}
