/* File: oh_context.h
   v2 context-oriented API draft.
*/
#ifndef OH_CONTEXT_H
#define OH_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct oh_state oh_context;

oh_context *oh_default_context(void);
void oh_context_set_region_weights(oh_context *context, const double *weights);

#ifdef __cplusplus
}
#endif

#endif
