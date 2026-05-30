/* File: oh_particle_buffer.h
   Internal helpers for adapter-strided particle storage.
*/
#ifndef OH_PARTICLE_BUFFER_H
#define OH_PARTICLE_BUFFER_H

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
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
  uintptr_t base_addr;
  uintptr_t part_addr;
  uintptr_t offset;
  uintptr_t index;
  size_t stride;

  if (!adapter || !base || !part) return -1;
  stride = oh_particle_buffer_stride(adapter);
  if (stride == 0) return -1;
  base_addr = (uintptr_t)base;
  part_addr = (uintptr_t)part;
  if (part_addr < base_addr) return -1;
  offset = part_addr - base_addr;
  if (offset % stride != 0) return -1;
  index = offset / stride;
  if (index > (uintptr_t)INT_MAX) return -1;
  return (int)index;
}

static inline int
oh_particle_buffer_index_bounded(const oh_particle_adapter *adapter,
                                 const void *base, int count,
                                 const void *part) {
  uintptr_t base_addr;
  uintptr_t part_addr;
  uintptr_t offset;
  uintptr_t span;
  uintptr_t index;
  size_t stride;

  if (!adapter || !base || !part || count < 0) return -1;
  stride = oh_particle_buffer_stride(adapter);
  if (stride == 0) return -1;
  if ((uintptr_t)count > UINTPTR_MAX / (uintptr_t)stride) return -1;
  span = (uintptr_t)count * (uintptr_t)stride;
  base_addr = (uintptr_t)base;
  if (span > UINTPTR_MAX - base_addr) return -1;
  part_addr = (uintptr_t)part;
  if (part_addr < base_addr || part_addr >= base_addr + span) return -1;
  offset = part_addr - base_addr;
  if (offset % stride != 0) return -1;
  index = offset / stride;
  if (index > (uintptr_t)INT_MAX) return -1;
  return (int)index;
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
