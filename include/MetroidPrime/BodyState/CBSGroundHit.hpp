#ifndef _CBSGROUNDHIT
#define _CBSGROUNDHIT

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSGroundHit : public CBodyState {
public:
  CBSGroundHit();

  // CBodyState
  ~CBSGroundHit() override {}
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);

  float mRotateSpeed;
  float mRemTime;
  pas::EFallState mFallState;
};
CHECK_SIZEOF(CBSGroundHit, 0x10)

#endif // _CBSGROUNDHIT
