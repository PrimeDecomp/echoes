#include "Kyoto/Alloc/LockedCache.hpp"

// Guessed descriptive names for the target's temporary locked-cache allocator state.
static uchar* sAllocationBase = reinterpret_cast< uchar* >(0xe0000000);
static void* sAllocations[16];
static uint sAllocatedBytes;
static uint sAllocationCount;
static bool sOutOfOrder;

void* AllocateLockedCache4(uint size) {
  const uint next = sAllocatedBytes + ((size + 3) & ~3u);
  if (next <= 0x4000) {
    void* ptr = sAllocationBase + sAllocatedBytes;
    sAllocatedBytes = next;
    if (sAllocationCount < 16) {
      sAllocations[sAllocationCount] = ptr;
    } else {
      sOutOfOrder = true;
    }
    ++sAllocationCount;
    return ptr;
  }
  return nullptr;
}

void* AllocateLockedCache32(uint size) {
  const uint next = sAllocatedBytes + ((size + 31) & ~31u);
  if (next <= 0x4000) {
    void* ptr = sAllocationBase + sAllocatedBytes;
    sAllocatedBytes = next;
    if (sAllocationCount < 16) {
      sAllocations[sAllocationCount] = ptr;
    } else {
      sOutOfOrder = true;
    }
    ++sAllocationCount;
    return ptr;
  }
  return nullptr;
}

void FreeLockedCache(void* ptr) {
  --sAllocationCount;
  if (!sOutOfOrder) {
    if (sAllocations[sAllocationCount] == ptr) {
      sAllocatedBytes = static_cast< uchar* >(ptr) - sAllocationBase;
    } else {
      sOutOfOrder = true;
    }
  }

  if (sAllocationCount == 0) {
    sAllocatedBytes = 0;
    sOutOfOrder = false;
  }
}

void* GetLockedCacheAllocationBase() { return sAllocationBase; }
