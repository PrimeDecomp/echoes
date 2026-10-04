#ifndef _RSTL_LOCKED_CACHE_ALLOCATOR
#define _RSTL_LOCKED_CACHE_ALLOCATOR

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Alloc/LockedCache.hpp"
#include "types.h"

namespace rstl {
// Guessed name
class locked_cache_allocator {
public:
  locked_cache_allocator() {}
  void Allocate(void*& out, uint size); // Guessed name

  template < typename T >
  void allocate(T*& out, int count) {
    Allocate(reinterpret_cast< void*& >(out), sizeof(T) * count);
  }
  template < typename T >
  void deallocate(T* ptr) {
    if (ptr != 0) {
      if (mHeapAllocation) {
        CMemory::Free(ptr);
      } else {
        FreeLockedCache(ptr);
      }
      --mAllocationCount;
      if (mAllocationCount != 0) {
        mHeapAllocation = mPreviousHeapAllocation;
      }
    }
  }

private:
  // Guessed names; byte positions are recovered from allocation/free consumers.
  bool mUnknown : 1;
  bool mPreferHeap : 1;
  bool mHeapAllocation : 1;
  bool mPreviousHeapAllocation : 1;
  uchar mAllocationCount : 2;
  uchar mUnknownTail : 2;
  uchar x1_unknown[3];
};
CHECK_SIZEOF(locked_cache_allocator, 0x4)
} // namespace rstl

#endif // _RSTL_LOCKED_CACHE_ALLOCATOR
