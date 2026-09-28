#ifndef _CABSREACTION
#define _CABSREACTION

#include "MetroidPrime/BodyState/CAdditiveBodyState.hpp"

class CABSReaction : public CAdditiveBodyState {
public:
  CABSReaction();

  // CBodyState
  ~CABSReaction() override {}
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
  void StopAnimation(CBodyController& bc);
  void UpdateWeight(CBodyController& bc); // Guessed name

  float mWeight;
  int mAnim;
  pas::EAdditiveReactionType mType;
  bool mActive;
};
CHECK_SIZEOF(CABSReaction, 0x14)

#endif // _CABSREACTION
