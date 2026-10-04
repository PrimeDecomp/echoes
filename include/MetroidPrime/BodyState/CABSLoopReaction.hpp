#ifndef _CABSLOOPREACTION
#define _CABSLOOPREACTION

#include "MetroidPrime/BodyState/CAdditiveBodyState.hpp"

// Guessed name
class CABSLoopReaction : public CAdditiveBodyState {
public:
  CABSLoopReaction();

  // CBodyState
  ~CABSLoopReaction() override {}
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  // Guessed names
  void UpdateWeight(CBodyController& bc);
  bool SelectAnimation(CBodyController& bc, CStateManager& mgr, pas::ELoopState state);
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);

  float mWeight;
  int mType;
  pas::ELoopState mState;
  int mAnimationId;
};
CHECK_SIZEOF(CABSLoopReaction, 0x14)

#endif // _CABSLOOPREACTION
