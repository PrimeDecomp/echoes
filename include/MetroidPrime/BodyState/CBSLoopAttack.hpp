#ifndef _CBSLOOPATTACK
#define _CBSLOOPATTACK

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSLoopAttack : public CBodyState {
public:
  CBSLoopAttack();

  // CBodyState
  ~CBSLoopAttack() override {}
  bool CanShoot() const override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

  bool GetAdvance() const { return mAdvance; }

private:
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);

  pas::ELoopState mState;
  pas::ELoopAttackType mLoopAttackType;
  float mElapsedTime;
  bool mWaitForAnimOver : 1;
  bool mAdvance : 1;
};
CHECK_SIZEOF(CBSLoopAttack, 0x14)

#endif // _CBSLOOPATTACK
