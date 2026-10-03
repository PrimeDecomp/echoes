#ifndef _LOCKEDCACHE
#define _LOCKEDCACHE

#include "types.h"

// Guessed name; returns the mutable base used by the native locked-cache allocator.
void* GetLockedCacheAllocationBase();

// Guessed names. These round the requested size, not the shared allocation cursor.
void* AllocateLockedCache4(uint size);
void* AllocateLockedCache32(uint size);
void FreeLockedCache(void* ptr);

#endif // _LOCKEDCACHE
