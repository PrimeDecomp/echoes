#include "MetroidPrime/Enemies/CAnimationState.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"

CAnimationState::CAnimationState() : mState(kAS_NotReady) {}

bool CAnimationState::CanIssueCommand(const CBodyController& controller,
                                      pas::EAnimationState state) {
  if (state == controller.GetCurrentStateId()) {
    mState = kAS_Repeat;
  } else if (mState == kAS_Ready) {
    return true;
  } else {
    mState = kAS_Over;
  }

  return false;
}
