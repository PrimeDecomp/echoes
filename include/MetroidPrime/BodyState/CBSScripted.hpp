#ifndef _CBSSCRIPTED
#define _CBSSCRIPTED

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSScripted : public CBodyState {
public:
  CBSScripted();

  // CBodyState
  ~CBSScripted() override;
  bool ApplyHeadTracking() const override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  bool mLoopAnim : 1;
  bool mTimedLoop : 1;
  float mRemTime;
};
CHECK_SIZEOF(CBSScripted, 0xc)

#endif // _CBSSCRIPTED
