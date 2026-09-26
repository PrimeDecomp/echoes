#ifndef _CABSAIM
#define _CABSAIM

#include "MetroidPrime/BodyState/CAdditiveBodyState.hpp"

class CABSAim : public CAdditiveBodyState {
public:
  CABSAim();

  // CBodyState
  ~CABSAim() override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  bool mNeedsIdle;
  int mAnims[4];
  float mAngles[4];
  int x28_;
  float mHWeight;
  float mHWeightVel;
  float mVWeight;
  float mVWeightVel;
};
CHECK_SIZEOF(CABSAim, 0x3c)

#endif // _CABSAIM
