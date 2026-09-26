#ifndef _CBSHURLED
#define _CBSHURLED

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSHurled : public CBodyState {
public:
  CBSHurled();

  // CBodyState
  ~CBSHurled() override;
  bool IsInAir(const CBodyController& bc) const override;
  bool IsMoving() const override;
  bool ApplyHeadTracking() const override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  pas::EHurledState mState;
  float mKnockAngle;
  int mAnimSeries;
  float mRotateSpeed;
  float mRemTime;
  float mCurTime;
  mutable CVector3f mLastTranslation;
  mutable float mLandedDur;
  bool mNeedsRecover : 1;
};
CHECK_SIZEOF(CBSHurled, 0x30)

#endif // _CBSHURLED
