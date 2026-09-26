#ifndef _CABSLOOPREACTION
#define _CABSLOOPREACTION

#include "MetroidPrime/BodyState/CAdditiveBodyState.hpp"

// Guessed name
class CABSLoopReaction : public CAdditiveBodyState {
public:
  CABSLoopReaction();

  // CBodyState
  ~CABSLoopReaction() override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  float mWeight;
  int mType;
  pas::ELoopState mState;
  int mAnim;
};
CHECK_SIZEOF(CABSLoopReaction, 0x14)

#endif // _CABSLOOPREACTION
