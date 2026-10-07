#ifndef _CBSJUMP
#define _CBSJUMP

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSJump : public CBodyState {
public:
  CBSJump();

  // CBodyState
  ~CBSJump() override {}

  bool IsInAir(const CBodyController& bc) const override;
  bool IsMoving() const override;
  bool ApplyHeadTracking() const override;
  bool ApplyAnimationDeltas() const override;
  bool CanShoot() const override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
  uchar CheckForWallJump(CBodyController& bc, CStateManager& mgr);
  void CheckForLand(CBodyController& bc, CStateManager& mgr);
  void PlayJumpLoop(CStateManager& mgr, CBodyController& bc);
  // Guessed names
  void ForceLand(CBodyController& bc, CStateManager& mgr);
  void UpdateAnimationVariant(CBodyController& bc);
  pas::EAnimationState UpdateExitJump(float dt, CBodyController& bc, CStateManager& mgr);

  pas::EJumpState mState;
  pas::EJumpType mJumpType;
  int mAnimationVariant; // Guessed name; third Jump PAS parameter.
  int mFacingFlags;      // Guessed name; CBCJumpCmd::EFacingFlags.
  CVector3f mWaypoint1;
  CVector3f mVelocity;
  CVector3f mWaypoint2;
  bool mApplyLaunchVel : 1;
  bool mWallJump : 1;
  bool mWallBounceRight : 1;
  bool mHasWallBounced : 1;
  bool mExitJumpRequested : 1; // Guessed name
};
CHECK_SIZEOF(CBSJump, 0x3c)

#endif // _CBSJUMP
