/* File: oh_particle_buffer.h
   Internal helpers for adapter-strided particle storage.
*/
#ifndef OH_PARTICLE_BUFFER_H
#define OH_PARTICLE_BUFFER_H

#include <stddef.h>
#include <string.h>

#include "oh_particle_adapter.h"
static inline size_t
oh_particle_buffer_stride(const oh_particle_adapter *adapter) {
  return adapter->stride;
}

static inline void *
oh_particle_buffer_at(const oh_particle_adapter *adapter, void *base,
                      int index) {
  return (char*)base + (size_t)index*oh_particle_buffer_stride(adapter);
}

static inline const void *
oh_particle_buffer_const_at(const oh_particle_adapter *adapter,
                            const void *base, int index) {
  return (const char*)base +
         (size_t)index*oh_particle_buffer_stride(adapter);
}

static inline int
oh_particle_buffer_index(const oh_particle_adapter *adapter, const void *base,
                         const void *part) {
  ptrdiff_t offset = (const char*)part - (const char*)base;
  size_t stride = oh_particle_buffer_stride(adapter);

  if (offset<0 || (size_t)offset%stride!=0) return -1;
  return (int)((size_t)offset/stride);
}

static inline void
oh_particle_buffer_copy(const oh_particle_adapter *adapter, void *dst,
                        const void *src) {
  memmove(dst, src, oh_particle_buffer_stride(adapter));
}

static inline void
oh_particle_buffer_copy_n(const oh_particle_adapter *adapter, void *dst,
                          const void *src, int count) {
  int i;

  for (i=0; i<count; i++)
    oh_particle_buffer_copy(adapter,
                            oh_particle_buffer_at(adapter, dst, i),
                            oh_particle_buffer_const_at(adapter, src, i));
}

#endif
