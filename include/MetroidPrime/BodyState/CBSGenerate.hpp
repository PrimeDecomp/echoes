#ifndef _CBSGENERATE
#define _CBSGENERATE

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSGenerate : public CBodyState {
public:
  CBSGenerate();

  // CBodyState
  ~CBSGenerate() override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;
};
CHECK_SIZEOF(CBSGenerate, 0x4)

#endif // _CBSGENERATE
