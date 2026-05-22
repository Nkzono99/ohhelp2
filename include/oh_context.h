/* File: oh_context.h
   v2 context-oriented API draft.
*/
#ifndef OH_CONTEXT_H
#define OH_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct oh_state oh_context;
typedef struct oh_particle_adapter oh_particle_adapter;

oh_context *oh_default_context(void);
/* Passing NULL resets all region weights to 1.0 on the default context. */
void oh_context_set_region_weights(oh_context *context, const double *weights);
/* Passing NULL resets particle movement to the default S_particle adapter. */
void oh_context_set_particle_adapter(oh_context *context,
                                     const oh_particle_adapter *adapter);

#ifdef __cplusplus
}
#endif

#endif
