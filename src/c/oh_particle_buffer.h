/* File: oh_particle_buffer.h
   Internal helpers for adapter-strided particle storage.
*/
#ifndef OH_PARTICLE_BUFFER_H
#define OH_PARTICLE_BUFFER_H

#include <stddef.h>
#include <string.h>

#include "oh_particle_adapter.h"
#include "oh_part.h"

static inline size_t
oh_particle_buffer_stride(const oh_particle_adapter *adapter) {
  return adapter->stride;
}

static inline struct S_particle *
oh_particle_buffer_at(const oh_particle_adapter *adapter,
                      struct S_particle *base, int index) {
  return (struct S_particle*)((char*)base +
                              (size_t)index*oh_particle_buffer_stride(adapter));
}

static inline const struct S_particle *
oh_particle_buffer_const_at(const oh_particle_adapter *adapter,
                            const struct S_particle *base, int index) {
  return (const struct S_particle*)((const char*)base +
                                    (size_t)index*
                                    oh_particle_buffer_stride(adapter));
}

static inline int
oh_particle_buffer_index(const oh_particle_adapter *adapter,
                         const struct S_particle *base,
                         const struct S_particle *part) {
  ptrdiff_t offset = (const char*)part - (const char*)base;
  size_t stride = oh_particle_buffer_stride(adapter);

  if (offset<0 || (size_t)offset%stride!=0) return -1;
  return (int)((size_t)offset/stride);
}

static inline void
oh_particle_buffer_copy(const oh_particle_adapter *adapter,
                        struct S_particle *dst,
                        const struct S_particle *src) {
  memmove(dst, src, oh_particle_buffer_stride(adapter));
}

static inline void
oh_particle_buffer_copy_n(const oh_particle_adapter *adapter,
                          struct S_particle *dst,
                          const struct S_particle *src, int count) {
  int i;

  for (i=0; i<count; i++)
    oh_particle_buffer_copy(adapter,
                            oh_particle_buffer_at(adapter, dst, i),
                            oh_particle_buffer_const_at(adapter, src, i));
}

#endif
