#ifndef _CINTERRUPTGUARD
#define _CINTERRUPTGUARD

#include "dolphin/os.h"

class CInterruptGuard {
  bool mEnabled;

public:
  CInterruptGuard() : mEnabled(OSDisableInterrupts()) {}
  ~CInterruptGuard() { OSRestoreInterrupts(mEnabled); }
};

#endif
