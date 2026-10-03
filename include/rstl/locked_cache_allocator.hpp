#ifndef _RSTL_LOCKED_CACHE_ALLOCATOR
#define _RSTL_LOCKED_CACHE_ALLOCATOR

#include "types.h"

namespace rstl {
// Guessed name
class locked_cache_allocator {
public:
  locked_cache_allocator() {}
  void Allocate(void*& out, uint size); // Guessed name

  template < typename T >
  void allocate(T*& out, int count) {}
  template < typename T >
  void deallocate(T* ptr) {}

private:
  uchar mFlags; // Guessed name
  uchar x1_unknown[3];
};
CHECK_SIZEOF(locked_cache_allocator, 0x4)
} // namespace rstl

#endif // _RSTL_LOCKED_CACHE_ALLOCATOR
