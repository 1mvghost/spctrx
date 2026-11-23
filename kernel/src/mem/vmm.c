#include <paging.h>
#include <util.h>
#include <vmm.h>

void vmmMap(void* virt, u64 phys, int flags, size_t length) {
  u64 virtPtr = (u64)virt;

  u64 offset = virtPtr % PAGE_SIZE;

  length += offset;
  virtPtr -= offset;

  size_t pages = DIV_ROUND_UP(length, PAGE_SIZE);

  for (size_t i = 0; i < pages; i++) {
    archMapPage((void*)(virtPtr + (i * PAGE_SIZE)), phys + (i * PAGE_SIZE),
                flags);
  }
}