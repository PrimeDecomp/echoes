#ifndef _CTWEAKBALL
#define _CTWEAKBALL

#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/single_ptr.hpp"

struct SLdrTweakBall;
class CDamageInfo;
class CRelAngle;

class CTweakBall {
public:
  explicit CTweakBall(const SLdrTweakBall& data) : mData(&data) {}

  float GetBallTranslationMaxSpeed(int surface) const;
  float GetBallCameraAnglePerSecond() const;
  const CVector3f& GetBallCameraOffset() const;
  float GetBallCameraMinSpeedDistance() const;
  float GetBallCameraMaxSpeedDistance() const;
  float GetBallCameraBackwardsDistance() const;
  float GetBallCameraSpringConstant() const;
  float GetBallCameraSpringMax() const;
  float GetBallCameraSpringTardis() const;
  float GetBallCameraCentroidSpringConstant() const;
  float GetBallCameraCentroidSpringMax() const;
  float GetBallCameraCentroidSpringTardis() const;
  float GetBallCameraCentroidDistanceSpringConstant() const;
  float GetBallCameraCentroidDistanceSpringMax() const;
  float GetBallCameraCentroidDistanceSpringTardis() const;
  float GetBallCameraLookAtSpringConstant() const;
  float GetBallCameraLookAtSpringMax() const;
  float GetBallCameraLookAtSpringTardis() const;
  float GetBallCameraFreeLookSpeed() const;
  float GetBallCameraFreeLookZoomSpeed() const;
  float GetBallCameraFreeLookMinDistance() const;
  float GetBallCameraFreeLookMaxDistance() const;
  float GetBallCameraFreeLookMaxVertAngle() const;
  float GetBallCameraConfinedDistance() const;
  float GetBallCameraChaseDistance() const;
  float GetBallCameraChaseYawSpeed() const;
  float GetBallCameraChaseDampenAngle() const;
  float GetBallCameraChaseAnglePerSecond() const;
  float GetBallCameraChaseSpringConstant() const;
  float GetBallCameraChaseSpringMax() const;
  float GetBallCameraChaseSpringTardis() const;
  float GetBallCameraBoostDistance() const;
  float GetBallCameraBoostYawSpeed() const;
  float GetBallCameraBoostDampenAngle() const;
  float GetBallCameraBoostAnglePerSecond() const;
  const CVector3f& GetBallCameraBoostLookAtOffset() const;
  float GetBallCameraBoostSpringConstant() const;
  float GetBallCameraBoostSpringMax() const;
  float GetBallCameraBoostSpringTardis() const;

  // Guessed names for newly recovered accessors.
  float GetMaxBallTranslationAcceleration(int surface) const;
  float GetBallTranslationFriction(int surface) const;
  float GetBallForwardBrakingAcceleration(int surface) const;
  float GetBallSlipFactor(int surface) const;
  float GetBallCameraControlDistance() const;
  float GetBallGravity() const;
  float GetBallWaterGravity() const;
  float GetMinimumAlignmentSpeed() const;
  float GetTireness() const;
  float GetLeftStickDivisor() const;
  float GetRightStickDivisor() const;
  CRelAngle GetMaxLeanAngle() const;
  float GetTireToMarbleThresholdSpeed() const;
  float GetMarbleToTireThresholdSpeed() const;
  float GetForceToLeanGain() const;
  float GetLeanTrackingGain() const;
  float GetBallTouchRadius() const;
  float GetBoostBallDrainTime() const;
  float GetBoostBallMaxChargeTime() const;
  float GetBoostBallMinChargeTime() const;
  float GetBoostBallMinRelativeSpeedForDamage() const;
  float GetSpiderBallBoostScalar() const;
  CDamageInfo GetBoostBallDamage() const;
  CDamageInfo GetCannonBallDamage() const;
  float GetBoostBallCollisionKnockBackSpeed() const;
  float GetBoostBallHitPlayerBallKnockBackSpeed() const;
  float GetBoostBallHitPlayerFPKnockBackSpeed() const;
  float GetScrewAttackGravity() const;
  float GetScrewAttackInitialDropLimit() const;
  float GetScrewAttackFinalDropLimit() const;
  int GetScrewAttackDropLimitJumpCount() const;
  float GetScrewAttackVerticalJumpVelocity() const;
  float GetScrewAttackHorizontalJumpVelocity() const;
  CRelAngle GetScrewAttackMaxSteeringAngle() const;
  float GetScrewAttackIntoBallTransitionTime() const;
  float GetScrewAttackOutOfBallTransitionTime() const;
  float GetScrewAttackWallJumpMaxTime() const;
  float GetScrewAttackWallJumpVerticalVelocity() const;
  float GetScrewAttackWallJumpHorizontalVelocity() const;
  float GetScrewAttackWallJumpGravity() const;
  CDamageInfo GetScrewAttackDamage() const;
  float GetDeathBallDamageDelay() const;
  CDamageInfo GetDeathBallDamage() const;
  float GetBoostBallChargeTimeTable(int index) const;
  float GetBoostBallIncrementalSpeedTable(int index) const;

private:
  // Borrowed settings record.
  const SLdrTweakBall* mData;
};
CHECK_SIZEOF(CTweakBall, 0x4)

extern rstl::single_ptr< CTweakBall > gpTweakBall;

#endif // _CTWEAKBALL
