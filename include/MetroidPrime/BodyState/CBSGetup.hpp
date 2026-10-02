#ifndef _CBSGETUP
#define _CBSGETUP

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSGetup : public CBodyState {
public:
  CBSGetup();

  // CBodyState
  ~CBSGetup() override {}
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);

  pas::EFallState mFallState;
};
CHECK_SIZEOF(CBSGetup, 0x8)

#endif // _CBSGETUP
