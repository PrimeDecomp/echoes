#ifndef _CBSTAUNT
#define _CBSTAUNT

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSTaunt : public CBodyState {
public:
  CBSTaunt();

  // CBodyState
  ~CBSTaunt() override {}
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
};
CHECK_SIZEOF(CBSTaunt, 0x4)

#endif // _CBSTAUNT
