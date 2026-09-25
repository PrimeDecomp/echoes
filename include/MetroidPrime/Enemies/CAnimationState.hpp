#ifndef _CANIMATIONSTATE
#define _CANIMATIONSTATE

#include "types.h"

// Guessed name. Tracks readiness and completion independently of the body controller.
class CAnimationState {
public:
  enum EState {
    kAS_NotReady,
    kAS_Ready,
    kAS_Repeat,
    kAS_Over,
  };

  CAnimationState();
  bool IsOver() const { return mState == kAS_Over; }

private:
  EState mState;
};
CHECK_SIZEOF(CAnimationState, 0x4)

#endif
