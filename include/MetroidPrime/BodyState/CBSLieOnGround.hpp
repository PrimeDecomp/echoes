#ifndef _CBSLIEONGROUND
#define _CBSLIEONGROUND

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CActor;

class CBSLieOnGround : public CBodyState {
public:
  CBSLieOnGround(const CActor& actor);

  // CBodyState
  ~CBSLieOnGround() override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  uint mHasGroundHit : 1;
};
CHECK_SIZEOF(CBSLieOnGround, 0x8)

#endif // _CBSLIEONGROUND
