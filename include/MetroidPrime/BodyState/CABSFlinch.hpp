#ifndef _CABSFLINCH
#define _CABSFLINCH

#include "MetroidPrime/BodyState/CAdditiveBodyState.hpp"

class CABSFlinch : public CAdditiveBodyState {
public:
  CABSFlinch();

  // CBodyState
  ~CABSFlinch() override {}
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc) const;
  void UpdateWeight(CBodyController& bc); // Guessed name

  float mWeight;
  int mAnim;
};
CHECK_SIZEOF(CABSFlinch, 0xc)

#endif // _CABSFLINCH
