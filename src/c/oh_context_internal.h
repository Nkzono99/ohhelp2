/* File: oh_context_internal.h
   Internal bridge while v2 migrates process-global state into contexts.
*/
#ifndef OH_CONTEXT_INTERNAL_H
#define OH_CONTEXT_INTERNAL_H

struct oh_state;

void oh1_sync_default_state(void);
void oh1_set_region_weights_state(struct oh_state *state,
                                  const double *weights);

#endif
