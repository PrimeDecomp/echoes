#ifndef _CABSIDLE
#define _CABSIDLE

#include "MetroidPrime/BodyState/CAdditiveBodyState.hpp"

class CABSIdle : public CAdditiveBodyState {
public:
  // CBodyState
  ~CABSIdle() override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;
};
CHECK_SIZEOF(CABSIdle, 0x4)

#endif // _CABSIDLE
