/* File: oh_load_balance.c
   v2 load-balance helpers independent of MPI and particle layout.
*/
#include <limits.h>
#include <float.h>
#include <math.h>

#include "oh_load_balance.h"

double
oh_region_load(long long particle_count, double region_weight) {
  double load;

  if (particle_count <= 0 || region_weight <= 0.0) return 0.0;
  if (!isfinite(region_weight)) return region_weight > 0.0 ? DBL_MAX : 0.0;
  load = (double)particle_count * region_weight;
  return isfinite(load) ? load : DBL_MAX;
}

double
oh_load_limit(double total_load, int max_fraction, int node_count) {
  double limit;

  if (total_load <= 0.0 || node_count <= 0) return 0.0;
  if (!isfinite(total_load)) return total_load > 0.0 ? DBL_MAX : 0.0;
  limit = total_load * (100.0 + (double)max_fraction) / 100.0
        / (double)node_count;
  return isfinite(limit) ? limit : DBL_MAX;
}

int
oh_particles_for_load(double load, double region_weight, long long max_available) {
  long long count;
  int cap;
  double ratio;

  if (max_available <= 0) return 0;
  cap = max_available > INT_MAX ? INT_MAX : (int)max_available;
  if (!(load > 0.0) || !(region_weight > 0.0) || !isfinite(region_weight))
    return 0;
  if (!isfinite(load)) return cap;
  ratio = load / region_weight;
  if (!isfinite(ratio) || ratio >= (double)cap) return cap;
  count = (long long)ratio;
  if ((double)count * region_weight < load) count++;
  if (count < 1) count = 1;
  if (count > cap) count = cap;
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

int
oh_region_weight_is_valid(double region_weight) {
  return region_weight > 0.0 && region_weight <= DBL_MAX;
}

int
oh_region_weights_use_weighted_mode(const double *region_weights,
                                    int region_count) {
  int i;

  if (!region_weights || region_count <= 0) return 0;
  for (i=0; i<region_count; i++)
    if (region_weights[i] != 1.0) return 1;
  return 0;
}
