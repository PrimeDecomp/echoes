#ifndef _CBSATTACK
#define _CBSATTACK

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/BodyState/CBodyState.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"

class CBSAttack : public CBodyState {
public:
  CBSAttack();

  // CBodyState
  ~CBSAttack() override {}
  bool CanShoot() const override { return false; }
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
  void UpdatePhysicsActor(CBodyController& bc, float dt);

  pas::EAnimationState mNextState;
  CBCSlideCmd mSlide;
  CVector3f mTargetPos;
  float mAlignTargetPosStartTime;
  float mAlignTargetPosTime;
  float mCurTime;
};
CHECK_SIZEOF(CBSAttack, 0x38)

#endif // _CBSATTACK
