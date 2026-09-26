#ifndef _CBSJUMP
#define _CBSJUMP

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSJump : public CBodyState {
public:
  CBSJump();

  // CBodyState
  ~CBSJump() override;
  bool IsInAir(const CBodyController& bc) const override;
  bool IsMoving() const override;
  bool ApplyHeadTracking() const override;
  bool ApplyAnimationDeltas() const override;
  bool CanShoot() const override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  pas::EJumpState mState;
  pas::EJumpType mJumpType;
  int xc_;
  int x10_;
  CVector3f mWaypoint1;
  CVector3f mVelocity;
  CVector3f mWaypoint2;
  bool mApplyLaunchVel : 1;
  bool mWallJump : 1;
  bool mWallBounceRight : 1;
  bool mHasWallBounced : 1;
  bool x38_4_ : 1;
};
CHECK_SIZEOF(CBSJump, 0x3c)

#endif // _CBSJUMP
