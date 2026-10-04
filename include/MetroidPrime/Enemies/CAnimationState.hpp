#ifndef _CANIMATIONSTATE
#define _CANIMATIONSTATE

#include "Kyoto/Animation/CharacterCommon.hpp"
#include "types.h"

class CBodyController;

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
  // Guessed name: permits a command only while ready and not already in that body state.
  bool CanIssueCommand(const CBodyController& controller, pas::EAnimationState state);

  bool IsOver() const { return mState == kAS_Over; }

private:
  EState mState;
};
CHECK_SIZEOF(CAnimationState, 0x4)

#endif // _CANIMATIONSTATE
