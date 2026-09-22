#ifndef _CFRAMEDELAYEDKILLER
#define _CFRAMEDELAYEDKILLER

#include "types.h"

class CFrameDelayedKiller {
public:
  enum EWhichFrame {
    kWhichFrame_ThisFrame,
    kWhichFrame_NextFrame,
  };

  static void Initialize();
  static void ShutDown();
  static void FlushAllocationsForFrame();
  static void ScheduleDeletion(EWhichFrame whichFrame, void* victim);
  static void FlushAllAllocations();
  static void StallAndFlushAllAllocations();
};

#endif // _CFRAMEDELAYEDKILLER
