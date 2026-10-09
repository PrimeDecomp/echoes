#ifndef _DOLPHINGPUMEMORY
#define _DOLPHINGPUMEMORY

#include "types.h"

// Guessed name: shared GX draw-sync-managed scratch storage, separated from the model TUs.
// Method names describe the native operations; no original Echoes export is known.
class GPUMemory {
public:
  static void SetBuffer(void* buffer, uint size);
  static void* EnsureAllocation(int size);
  static void ReleaseAllocation();
  static void TickAllocations();
};

#endif // _DOLPHINGPUMEMORY
