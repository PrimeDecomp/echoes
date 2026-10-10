#include "MetroidPrime/Tweaks/CTweakBall.hpp"

#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall.hpp"

float CTweakBall::GetMaxBallTranslationAcceleration(int surface) const {
  switch (surface) {
  default:
  case 0:
    return mData->movement.forwardAccelNormal;
  case 1:
    return mData->movement.forwardAccelAir;
  case 2:
    return mData->movement.forwardAccelIce;
  case 3:
    return mData->movement.forwardAccelOrganic;
  case 4:
    return mData->movement.forwardAccelWater;
  case 6:
    return mData->movement.forwardAccelLava;
  case 5:
    return mData->movement.forwardAccelPhazon;
  case 7:
    return mData->movement.forwardAccelShrubbery;
  }
}

float CTweakBall::GetBallTranslationFriction(int surface) const {
  switch (surface) {
  default:
  case 0:
    return mData->movement.movementFrictionNormal;
  case 1:
    return mData->movement.movementFrictionAir;
  case 2:
    return mData->movement.movementFrictionIce;
  case 3:
    return mData->movement.movementFrictionOrganic;
  case 4:
    return mData->movement.movementFrictionWater;
  case 6:
    return mData->movement.movementFrictionLava;
  case 5:
    return mData->movement.movementFrictionPhazon;
  case 7:
    return mData->movement.movementFrictionShrubbery;
  }
}

float CTweakBall::GetBallTranslationMaxSpeed(int surface) const {
  switch (surface) {
  default:
  case 0:
    return mData->movement.forwardMaxSpeedNormal;
  case 1:
    return mData->movement.forwardMaxSpeedAir;
  case 2:
    return mData->movement.forwardMaxSpeedIce;
  case 3:
    return mData->movement.forwardMaxSpeedOrganic;
  case 4:
    return mData->movement.forwardMaxSpeedWater;
  case 6:
    return mData->movement.forwardMaxSpeedLava;
  case 5:
    return mData->movement.forwardMaxSpeedPhazon;
  case 7:
    return mData->movement.forwardMaxSpeedShrubbery;
  }
}

float CTweakBall::GetBallForwardBrakingAcceleration(int surface) const {
  switch (surface) {
  default:
  case 0:
    return mData->movement.ballForwardBrakingAccelNormal;
  case 1:
    return mData->movement.ballForwardBrakingAccelAir;
  case 2:
    return mData->movement.ballForwardBrakingAccelIce;
  case 3:
    return mData->movement.ballForwardBrakingAccelOrganic;
  case 4:
    return mData->movement.ballForwardBrakingAccelWater;
  case 6:
    return mData->movement.ballForwardBrakingAccelLava;
  case 5:
    return mData->movement.ballForwardBrakingAccelPhazon;
  case 7:
    return mData->movement.ballForwardBrakingAccelShrubbery;
  }
}

float CTweakBall::GetBallSlipFactor(int surface) const {
  switch (surface) {
  case 0:
  default:
    return 10000.0f;

  case 1:
    return 10000.0f;

  case 2:
    return 1000.0f;

  case 3:
    return 10000.0f;

  case 4:
    return 2000.0f;

  case 5:
    return 2000.0f;

  case 6:
    return 2000.0f;

  case 7:
    return 2000.0f;
  }
}

float CTweakBall::GetBallCameraAnglePerSecond() const {
  return CRelAngle::FromDegrees(mData->camera.ballCameraAnglePerSecond).AsRadians();
}

const CVector3f& CTweakBall::GetBallCameraOffset() const { return mData->camera.ballCameraOffset; }

float CTweakBall::GetBallCameraMinSpeedDistance() const {
  return mData->camera.ballCameraMinSpeedDistance;
}

float CTweakBall::GetBallCameraMaxSpeedDistance() const {
  return mData->camera.ballCameraMaxSpeedDistance;
}

float CTweakBall::GetBallCameraBackwardsDistance() const {
  return mData->camera.ballCameraBackwardsDistance;
}

float CTweakBall::GetBallCameraSpringConstant() const {
  return mData->camera.ballCameraSpringConstant;
}

float CTweakBall::GetBallCameraSpringMax() const { return mData->camera.ballCameraSpringMax; }

float CTweakBall::GetBallCameraSpringTardis() const { return mData->camera.ballCameraSpringTardis; }

float CTweakBall::GetBallCameraCentroidSpringConstant() const {
  return mData->camera.ballCameraCentroidSpringConstant;
}

float CTweakBall::GetBallCameraCentroidSpringMax() const {
  return mData->camera.ballCameraCentroidSpringMax;
}

float CTweakBall::GetBallCameraCentroidSpringTardis() const {
  return mData->camera.ballCameraCentroidSpringTardis;
}

float CTweakBall::GetBallCameraCentroidDistanceSpringConstant() const {
  return mData->camera.ballCameraCentroidDistanceSpringConstant;
}

float CTweakBall::GetBallCameraCentroidDistanceSpringMax() const {
  return mData->camera.ballCameraCentroidDistanceSpringMax;
}

float CTweakBall::GetBallCameraCentroidDistanceSpringTardis() const {
  return mData->camera.ballCameraCentroidDistanceSpringTardis;
}

float CTweakBall::GetBallCameraLookAtSpringConstant() const {
  return mData->camera.ballCameraLookAtSpringConstant;
}

float CTweakBall::GetBallCameraLookAtSpringMax() const {
  return mData->camera.ballCameraLookAtSpringMax;
}

float CTweakBall::GetBallCameraLookAtSpringTardis() const {
  return mData->camera.ballCameraLookAtSpringTardis;
}

float CTweakBall::GetBallCameraFreeLookSpeed() const {
  return CRelAngle::FromDegrees(mData->camera.ballCameraFreeLookSpeed).AsRadians();
}

float CTweakBall::GetBallCameraFreeLookZoomSpeed() const {
  return mData->camera.ballCameraFreeLookZoomSpeed;
}

float CTweakBall::GetBallCameraFreeLookMinDistance() const {
  return mData->camera.ballCameraFreeLookMinDistance;
}

float CTweakBall::GetBallCameraFreeLookMaxDistance() const {
  return mData->camera.ballCameraFreeLookMaxDistance;
}

float CTweakBall::GetBallCameraFreeLookMaxVertAngle() const {
  return CRelAngle::FromDegrees(mData->camera.ballCameraFreeLookMaxVertAngle).AsRadians();
}

float CTweakBall::GetBallCameraConfinedDistance() const {
  return mData->camera.ballCameraConfinedDistance;
}

float CTweakBall::GetBallCameraChaseDistance() const {
  return mData->camera.ballCameraChaseDistance;
}

float CTweakBall::GetBallCameraChaseAnglePerSecond() const {
  return CRelAngle::FromDegrees(mData->camera.ballCameraChaseAnglePerSecond).AsRadians();
}

float CTweakBall::GetBallCameraChaseYawSpeed() const {
  return mData->camera.ballCameraChaseYawSpeed;
}

float CTweakBall::GetBallCameraChaseDampenAngle() const {
  return CRelAngle::FromDegrees(mData->camera.ballCameraChaseDampenAngle).AsRadians();
}

float CTweakBall::GetBallCameraChaseSpringConstant() const {
  return mData->camera.ballCameraChaseSpringConstant;
}

float CTweakBall::GetBallCameraChaseSpringMax() const {
  return mData->camera.ballCameraChaseSpringMax;
}

float CTweakBall::GetBallCameraChaseSpringTardis() const {
  return mData->camera.ballCameraChaseSpringTardis;
}

float CTweakBall::GetBallCameraBoostDistance() const {
  return mData->camera.ballCameraBoostDistance;
}

float CTweakBall::GetBallCameraBoostAnglePerSecond() const {
  return CRelAngle::FromDegrees(mData->camera.ballCameraBoostAnglePerSecond).AsRadians();
}

float CTweakBall::GetBallCameraBoostYawSpeed() const {
  return mData->camera.ballCameraBoostYawSpeed;
}

float CTweakBall::GetBallCameraBoostDampenAngle() const {
  return CRelAngle::FromDegrees(mData->camera.ballCameraBoostDampenAngle).AsRadians();
}

const CVector3f& CTweakBall::GetBallCameraBoostLookAtOffset() const {
  return mData->camera.ballCameraBoostLookAtOffset;
}

float CTweakBall::GetBallCameraBoostSpringConstant() const {
  return mData->camera.ballCameraBoostSpringConstant;
}

float CTweakBall::GetBallCameraBoostSpringMax() const {
  return mData->camera.ballCameraBoostSpringMax;
}

float CTweakBall::GetBallCameraBoostSpringTardis() const {
  return mData->camera.ballCameraBoostSpringTardis;
}

float CTweakBall::GetBallCameraControlDistance() const {
  return mData->camera.ballCameraControlDistance;
}

float CTweakBall::GetBallGravity() const { return -mData->movement.ballGravity; }

float CTweakBall::GetBallWaterGravity() const { return -mData->movement.ballWaterGravity; }

float CTweakBall::GetMinimumAlignmentSpeed() const { return mData->movement.minimumAlignmentSpeed; }

float CTweakBall::GetTireness() const { return mData->movement.tireness; }

float CTweakBall::GetLeftStickDivisor() const { return mData->misc.unknown_0x13cfde23; }

float CTweakBall::GetRightStickDivisor() const { return mData->misc.unknown_0xf3499713; }

CRelAngle CTweakBall::GetMaxLeanAngle() const {
  return CRelAngle::FromDegrees(mData->movement.maxLeanAngle);
}

float CTweakBall::GetTireToMarbleThresholdSpeed() const {
  return mData->movement.tireToMarbleThresholdSpeed;
}

float CTweakBall::GetMarbleToTireThresholdSpeed() const {
  return mData->movement.marbleToTireThresholdSpeed;
}

float CTweakBall::GetForceToLeanGain() const { return mData->movement.forceToLeanGain; }

float CTweakBall::GetLeanTrackingGain() const { return mData->movement.leanTrackingGain; }

float CTweakBall::GetBallTouchRadius() const { return mData->misc.ballTouchRadius; }

float CTweakBall::GetBoostBallDrainTime() const { return mData->boostBall.boostBallDrainTime; }

float CTweakBall::GetBoostBallMaxChargeTime() const {
  return mData->boostBall.boostBallMaxChargeTime;
}

float CTweakBall::GetBoostBallMinChargeTime() const {
  return mData->boostBall.boostBallMinChargeTime;
}

float CTweakBall::GetBoostBallMinRelativeSpeedForDamage() const {
  return mData->boostBall.boostBallMinRelativeSpeedForDamage;
}

float CTweakBall::GetSpiderBallBoostScalar() const {
  return mData->boostBall.spiderBallBoostScalar;
}

CDamageInfo CTweakBall::GetBoostBallDamage() const {
  return LdrToDamageInfo(mData->boostBall.boostBallDamage);
}

CDamageInfo CTweakBall::GetCannonBallDamage() const {
  return LdrToDamageInfo(mData->cannonBall.cannonBallDamage);
}

float CTweakBall::GetBoostBallCollisionKnockBackSpeed() const {
  return mData->boostBall.boostBallCollisionKnockBackSpeed;
}

float CTweakBall::GetBoostBallHitPlayerBallKnockBackSpeed() const {
  return mData->boostBall.boostBallHitPlayerBallKnockBackSpeed;
}

float CTweakBall::GetBoostBallHitPlayerFPKnockBackSpeed() const {
  return mData->boostBall.boostBallHitPlayerFPKnockBackSpeed;
}

float CTweakBall::GetScrewAttackGravity() const { return -mData->screwAttack.screwAttackGravity; }

float CTweakBall::GetScrewAttackInitialDropLimit() const {
  return mData->screwAttack.unknown_0xcb77fb28;
}

float CTweakBall::GetScrewAttackFinalDropLimit() const {
  return mData->screwAttack.unknown_0x3fdeb046;
}

int CTweakBall::GetScrewAttackDropLimitJumpCount() const {
  return mData->screwAttack.unknown_0x691b244d;
}

float CTweakBall::GetScrewAttackVerticalJumpVelocity() const {
  return mData->screwAttack.screwAttackVerticalJumpVelocity;
}

float CTweakBall::GetScrewAttackHorizontalJumpVelocity() const {
  return mData->screwAttack.screwAttackHorizontalJumpVelocity;
}

CRelAngle CTweakBall::GetScrewAttackMaxSteeringAngle() const {
  return CRelAngle::FromDegrees(mData->screwAttack.unknown_0x3d03d8a6);
}

float CTweakBall::GetScrewAttackIntoBallTransitionTime() const {
  return mData->screwAttack.screwAttackIntoBallTransitionTime;
}

float CTweakBall::GetScrewAttackOutOfBallTransitionTime() const {
  return mData->screwAttack.screwAttackOutOfBallTransitionTime;
}

float CTweakBall::GetScrewAttackWallJumpMaxTime() const {
  return mData->screwAttack.screwAttackWallJumpMaxTime;
}

float CTweakBall::GetScrewAttackWallJumpVerticalVelocity() const {
  return mData->screwAttack.screwAttackWallJumpVerticalVelocity;
}

float CTweakBall::GetScrewAttackWallJumpHorizontalVelocity() const {
  return mData->screwAttack.screwAttackWallJumpHorizontalVelocity;
}

float CTweakBall::GetScrewAttackWallJumpGravity() const {
  return -mData->screwAttack.screwAttackWallJumpGravity;
}

CDamageInfo CTweakBall::GetScrewAttackDamage() const {
  return LdrToDamageInfo(mData->screwAttack.screwAttackDamage);
}

float CTweakBall::GetDeathBallDamageDelay() const { return mData->deathBall.deathBallDamageDelay; }

CDamageInfo CTweakBall::GetDeathBallDamage() const {
  return LdrToDamageInfo(mData->deathBall.deathBallDamage);
}

float CTweakBall::GetBoostBallChargeTimeTable(int index) const {
  switch (index) {
  default:
  case 0:
    return mData->boostBall.boostBallChargeTime1;
  case 1:
    return mData->boostBall.boostBallChargeTime2;
  case 2:
    return mData->boostBall.boostBallMaxChargeTime;
  }
}

float CTweakBall::GetBoostBallIncrementalSpeedTable(int index) const {
  switch (index) {
  default:
  case 0:
    return mData->boostBall.boostBallIncrementalSpeed1;
  case 1:
    return mData->boostBall.boostBallIncrementalSpeed2;
  case 2:
    return mData->boostBall.boostBallIncrementalSpeed3;
  }
}
