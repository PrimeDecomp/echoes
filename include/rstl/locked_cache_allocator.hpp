#ifndef _RSTL_LOCKED_CACHE_ALLOCATOR
#define _RSTL_LOCKED_CACHE_ALLOCATOR

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Alloc/LockedCache.hpp"
#include "types.h"

namespace rstl {
// Guessed name
class locked_cache_allocator {
public:
  explicit locked_cache_allocator(int mode = 0)
  : mHeapAfterCacheAttempt(mode == 0)
  , mPreferHeap(mode == 2)
  , mHeapAllocation(false)
  , mPreviousHeapAllocation(false)
  , mAllocationCount(0) {}
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
  bool mHeapAfterCacheAttempt : 1;
  bool mPreferHeap : 1;
  bool mHeapAllocation : 1;
  bool mPreviousHeapAllocation : 1;
  uint mAllocationCount : 2;
  uint mUnknownTail : 2;
};
CHECK_SIZEOF(locked_cache_allocator, 0x4)
} // namespace rstl

#endif // _RSTL_LOCKED_CACHE_ALLOCATOR
