#ifndef _CGXTRANSIENTBUFFER
#define _CGXTRANSIENTBUFFER

#include "types.h"

// Guessed name: shared GX draw-sync-managed scratch storage, separated from the model TUs.
// Method names describe the native operations; no original Echoes export is known.
class CGXTransientBuffer {
public:
  static void SetBuffer(void* buffer, uint size);
  static void* EnsureAllocation(int size);
  static void ReleaseAllocation();
  static void TickAllocations();
};

#endif // _CGXTRANSIENTBUFFER
