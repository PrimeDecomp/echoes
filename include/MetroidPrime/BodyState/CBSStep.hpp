#ifndef _CBSSTEP
#define _CBSSTEP

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSStep : public CBodyState {
public:
  CBSStep();

  // CBodyState
  ~CBSStep() override {}
  bool IsMoving() const override;
  bool CanShoot() const override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
};
CHECK_SIZEOF(CBSStep, 0x4)

#endif // _CBSSTEP
