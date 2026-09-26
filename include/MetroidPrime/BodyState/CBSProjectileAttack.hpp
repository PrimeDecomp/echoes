#ifndef _CBSPROJECTILEATTACK
#define _CBSPROJECTILEATTACK

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSProjectileAttack : public CBodyState {
public:
  CBSProjectileAttack();

  // CBodyState
  ~CBSProjectileAttack() override;
  bool CanShoot() const override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;
};
CHECK_SIZEOF(CBSProjectileAttack, 0x4)

#endif // _CBSPROJECTILEATTACK
