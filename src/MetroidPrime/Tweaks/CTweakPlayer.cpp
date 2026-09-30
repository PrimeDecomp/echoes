#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer.hpp"

float CTweakPlayer::GetNormalGravAccel() const { return mData->motion.gravitationalAccel; }

float CTweakPlayer::GetFluidGravAccel() const { return mData->motion.fluidGravitationalAccel; }

float CTweakPlayer::GetVerticalJumpAccel() const { return mData->motion.verticalJumpAccel; }

float CTweakPlayer::GetHorizontalJumpAccel() const { return mData->motion.horizontalJumpAccel; }

float CTweakPlayer::GetVerticalDoubleJumpAccel() const {
  return mData->motion.verticalDoubleJumpAccel;
}

float CTweakPlayer::GetHorizontalDoubleJumpAccel() const {
  return mData->motion.horizontalDoubleJumpAccel;
}

float CTweakPlayer::GetWaterJumpFactor() const { return mData->motion.waterJumpFactor; }

float CTweakPlayer::GetLavaJumpFactor() const { return mData->motion.lavaJumpFactor; }

float CTweakPlayer::GetPhazonJumpFactor() const { return mData->motion.phazonJumpFactor; }

float CTweakPlayer::GetBombJumpHeight() const { return mData->motion.bombJumpHeight; }

float CTweakPlayer::GetBombJumpRadius() const { return mData->motion.bombJumpRadius; }

float CTweakPlayer::GetAllowedJumpTime() const { return mData->motion.allowedJumpTime; }

float CTweakPlayer::GetAllowedDoubleJumpTime() const { return mData->motion.allowedDoubleJumpTime; }

float CTweakPlayer::GetMinDoubleJumpWindow() const { return mData->motion.minDoubleJumpWindow; }

float CTweakPlayer::GetMaxDoubleJumpWindow() const { return mData->motion.maxDoubleJumpWindow; }

float CTweakPlayer::GetMinJumpTime() const { return mData->motion.minJumpTime; }

float CTweakPlayer::GetMinDoubleJumpTime() const { return mData->motion.minDoubleJumpTime; }

float CTweakPlayer::GetAllowedLedgeTime() const { return mData->motion.ledgeFallTime; }

float CTweakPlayer::GetDoubleJumpImpulse() const { return mData->motion.doubleJumpImpulse; }

float CTweakPlayer::GetBackwardsForceMultiplier() const {
  return mData->motion.backwardsForceMultiplier;
}

float CTweakPlayer::GetGravityBoostTime() const { return mData->motion.gravityBoostTime; }

float CTweakPlayer::GetGravityBoostForce() const { return mData->motion.gravityBoostForce; }

float CTweakPlayer::GetGravityBoostCancelDampening() const {
  return mData->motion.gravityBoostCancelDampening;
}

bool CTweakPlayer::GetGravityBoostMultipleAllowed() const {
  return mData->motion.gravityBoostMultipleAllowed;
}

float CTweakPlayer::GetEyeOffset() const { return mData->misc.eyeOffset; }

float CTweakPlayer::GetHorizontalFreeLookAngleVel() const {
  return CRelAngle::FromDegrees(mData->misc.freeLookMaxX).AsRadians();
}

float CTweakPlayer::GetVerticalFreeLookAngleVel() const {
  return CRelAngle::FromDegrees(mData->misc.freeLookMaxZ).AsRadians();
}

float CTweakPlayer::GetFreeLookSpeed() const {
  return CRelAngle::FromDegrees(mData->misc.freeLookSpeed).AsRadians();
}

float CTweakPlayer::GetFreeLookSnapSpeed() const {
  return CRelAngle::FromDegrees(mData->misc.freeLookSnapSpeed).AsRadians();
}

float CTweakPlayer::GetFreeLookCenteredThresholdAngle() const {
  return CRelAngle::FromDegrees(mData->misc.freeLookMinAngle).AsRadians();
}

float CTweakPlayer::GetFreeLookCenteredTime() const { return mData->misc.freeLookCenteredTime; }

bool CTweakPlayer::GetVelocityFreeLookEnabled() const { return mData->misc.nullAnalogScales; }

float CTweakPlayer::GetVelocityFreeLookThreshold() const { return mData->misc.unknown_0xfb909bc3; }

float CTweakPlayer::GetLeftAnalogMax() { return mData->misc.leftAnalogMax; }

float CTweakPlayer::GetRightAnalogMax() { return mData->misc.rightAnalogMax; }

float CTweakPlayer::GetTurnSpeedMultiplier() const { return mData->misc.normalTurnFactor; }

float CTweakPlayer::GetFreeLookTurnSpeedMultiplier() const {
  return mData->misc.freeLookTurnFactor;
}

float CTweakPlayer::GetOrbitMinDistance(CPlayer::EPlayerOrbitType type) const {
  switch (type) {
  default:
  case CPlayer::kOT_Close:
    return mData->orbit.orbitCloseMinDistance;
  case CPlayer::kOT_Far:
    return mData->orbit.orbitFarMinDistance;
  case CPlayer::kOT_Default:
    return mData->orbit.orbitCarcassMinDistance;
  }
}

float CTweakPlayer::GetOrbitNormalDistance(CPlayer::EPlayerOrbitType type) const {
  switch (type) {
  default:
  case CPlayer::kOT_Close:
    return mData->orbit.orbitCloseNormalDistance;
  case CPlayer::kOT_Far:
    return mData->orbit.orbitFarNormalDistance;
  case CPlayer::kOT_Default:
    return mData->orbit.orbitCarcassNormalDistance;
  }
}

float CTweakPlayer::GetOrbitMaxDistance(CPlayer::EPlayerOrbitType type) const {
  switch (type) {
  default:
  case CPlayer::kOT_Close:
    return mData->orbit.orbitCloseMaxDistance;
  case CPlayer::kOT_Far:
    return mData->orbit.orbitFarMaxDistance;
  case CPlayer::kOT_Default:
    return mData->orbit.orbitCarcassMaxDistance;
  }
}

float CTweakPlayer::GetOrbitModeTimer() const { return mData->orbit.orbitModeTimer; }

float CTweakPlayer::GetOrbitCameraSpeed() const {
  return CRelAngle::FromDegrees(mData->orbit.orbitCameraSpeed).AsRadians();
}

float CTweakPlayer::GetOrbitUpperAngle() const {
  return CRelAngle::FromDegrees(mData->orbit.orbitUpperAngle).AsRadians();
}

float CTweakPlayer::GetOrbitLowerAngle() const {
  return CRelAngle::FromDegrees(mData->orbit.orbitLowerAngle).AsRadians();
}

float CTweakPlayer::GetOrbitHorizAngle() const {
  return CRelAngle::FromDegrees(mData->orbit.orbitHorizAngle).AsRadians();
}

float CTweakPlayer::GetOrbitMaxLockDistance() const { return mData->orbit.orbitMaxLockDistance; }

float CTweakPlayer::GetOrbitInvalidTargetTime() const { return mData->orbit.unknown_0x55f7d145; }

float CTweakPlayer::GetOrbitMaxTargetDistance() const {
  return mData->orbit.orbitMaxTargetDistance;
}

float CTweakPlayer::GetOrbitDistanceThreshold() const {
  return mData->orbit.orbitDistanceThreshold;
}

int CTweakPlayer::GetOrbitZoneWidth(CPlayer::EPlayerZoneInfo zone) const {
  const float scale = CGraphics::GetViewport().mWidth / 640.f;
  const int extent =
      zone == CPlayer::kZI_Scan ? mData->orbit.orbitScanZoneWidth : mData->orbit.orbitZoneWidth;
  return static_cast< int >(extent * scale);
}

int CTweakPlayer::GetOrbitZoneHeight(CPlayer::EPlayerZoneInfo zone) const {
  const float scale = CGraphics::GetViewport().mHeight / 448.f;
  const int extent =
      zone == CPlayer::kZI_Scan ? mData->orbit.orbitScanZoneHeight : mData->orbit.orbitZoneHeight;
  return static_cast< int >(extent * scale);
}

int CTweakPlayer::GetOrbitZoneCentreX(CPlayer::EPlayerZoneInfo zone) const {
  const float scale = CGraphics::GetViewport().mWidth / 640.f;
  const int extent =
      zone == CPlayer::kZI_Scan ? mData->orbit.orbitScanZoneCentreX : mData->orbit.orbitZoneCentreX;
  return static_cast< int >(extent * scale);
}

int CTweakPlayer::GetOrbitZoneCentreY(CPlayer::EPlayerZoneInfo zone) const {
  const float scale = CGraphics::GetViewport().mHeight / 448.f;
  const int extent =
      zone == CPlayer::kZI_Scan ? mData->orbit.orbitScanZoneCentreY : mData->orbit.orbitZoneCentreY;
  return static_cast< int >(extent * scale);
}

int CTweakPlayer::GetOrbitZoneIdealX(CPlayer::EPlayerZoneInfo zone) const {
  const float scale = CGraphics::GetViewport().mWidth / 640.f;
  const int extent =
      zone == CPlayer::kZI_Scan ? mData->orbit.orbitScanZoneIdealX : mData->orbit.orbitZoneIdealX;
  return static_cast< int >(extent * scale);
}

int CTweakPlayer::GetOrbitZoneIdealY(CPlayer::EPlayerZoneInfo zone) const {
  const float scale = CGraphics::GetViewport().mHeight / 448.f;
  const int extent =
      zone == CPlayer::kZI_Scan ? mData->orbit.orbitScanZoneIdealY : mData->orbit.orbitZoneIdealY;
  return static_cast< int >(extent * scale);
}

float CTweakPlayer::GetOrbitBoxWidth() const { return mData->orbit.orbitBoxWidth; }

float CTweakPlayer::GetOrbitBoxHeight() const { return mData->orbit.orbitBoxHeight; }

float CTweakPlayer::GetOrbitFixedOffsetZDiff() const { return mData->orbit.unknown_0x478c15f9; }

float CTweakPlayer::GetOrbitZRange() const { return mData->orbit.orbitZRange; }

float CTweakPlayer::GetOrbitPreventionTime() const { return mData->orbit.orbitPreventionTime; }

bool CTweakPlayer::GetDashEnabled() const { return mData->orbit.orbitDash; }

bool CTweakPlayer::GetDashOnButtonRelease() const { return mData->orbit.orbitDashUsesTap; }

float CTweakPlayer::GetDashButtonHoldCancelTime() const { return mData->orbit.orbitDashTapTime; }

float CTweakPlayer::GetDashStrafeInputThreshold() const {
  return mData->orbit.orbitDashStickThreshold;
}

float CTweakPlayer::GetSidewaysDoubleJumpImpulse() const {
  return mData->orbit.orbitDashDoubleJumpImpulse;
}

float CTweakPlayer::GetSidewaysVerticalDoubleJumpAccel() const {
  return mData->orbit.orbitDashVerticalDoubleJumpAccel;
}

float CTweakPlayer::GetSidewaysHorizontalDoubleJumpAccel() const {
  return mData->orbit.orbitDashHorizontalDoubleJumpAccel;
}

float CTweakPlayer::GetScanningRange() const { return mData->scanVisor.scanDistance; }

bool CTweakPlayer::GetScanRetention() const { return mData->scanVisor.scanRetention; }

bool CTweakPlayer::GetScanFreezesGame() const { return mData->scanVisor.scanFreezesGame; }

float CTweakPlayer::GetScanMaxLockDistance() const { return mData->scanVisor.scanMaxLockDistance; }

float CTweakPlayer::GetScanMaxTargetDistance() const {
  return mData->scanVisor.scanMaxTargetDistance;
}

float CTweakPlayer::GetScanCameraSpeed() const { return mData->scanVisor.scanCameraSpeed; }

float CTweakPlayer::GetGrappleDistance() const { return mData->grapple.grappleDistance; }

float CTweakPlayer::GetGrappleSwingLength() const { return mData->grapple.grappleBeamLength; }

float CTweakPlayer::GetGrappleSwingPeriod() const { return mData->grapple.grappleSwingTime; }

float CTweakPlayer::GetGrappleMaxVelocity() const { return mData->grapple.grappleMaxVelocity; }

float CTweakPlayer::GetGrappleCameraSpeed() const {
  return CRelAngle::FromDegrees(mData->grapple.grappleCameraSpeed).AsRadians();
}

float CTweakPlayer::GetGrapplePullCloseDistance() const {
  return mData->grapple.grapplePullCloseDistance;
}

float CTweakPlayer::GetGrapplePullDampenDistance() const {
  return mData->grapple.grapplePullDampenDistance;
}

float CTweakPlayer::GetGrapplePullSpeedMax() const { return mData->grapple.grapplePullVelocity; }

float CTweakPlayer::GetGrapplePullCameraSpeed() const {
  return CRelAngle::FromDegrees(mData->grapple.grapplePullCameraSpeed).AsRadians();
}

float CTweakPlayer::GetMaxGrappleTurnSpeed() const { return mData->grapple.grappleTurnRate; }

float CTweakPlayer::GetGrappleJumpForce() const { return mData->grapple.grappleJumpForce; }

float CTweakPlayer::GetGrappleReleaseTime() const { return mData->grapple.grappleReleaseTime; }

int CTweakPlayer::GetGrappleJumpMode() const { return mData->grapple.grappleControlScheme; }

bool CTweakPlayer::GetOrbitReleaseBreaksGrapple() const {
  return mData->grapple.grappleHoldOrbitButton;
}

bool CTweakPlayer::GetInvertGrappleTurn() const {
  return mData->grapple.grappleTurnControlsReversed;
}

float CTweakPlayer::GetGrappleBeamSpeed() const { return mData->grapple.beam.travel_Speed; }

float CTweakPlayer::GetGrappleBeamXWaveAmplitude() const {
  return mData->grapple.beam.x_Wave_Amplitude;
}

float CTweakPlayer::GetGrappleBeamZWaveAmplitude() const {
  return mData->grapple.beam.z_Wave_Amplitude;
}

float CTweakPlayer::GetGrappleBeamAnglePhaseDelta() const {
  return mData->grapple.beam.angle_Phase_Delta;
}

float CTweakPlayer::GetAimMaxDistance() const { return mData->aimStuff.aimMaxDistance; }

float CTweakPlayer::GetAimThresholdDistance() const { return mData->aimStuff.aimThresholdDistance; }

float CTweakPlayer::GetAimBoxWidth() const { return mData->aimStuff.aimBoxWidth; }

float CTweakPlayer::GetAimBoxHeight() const { return mData->aimStuff.aimBoxHeight; }

float CTweakPlayer::GetAimTargetTimer() const { return mData->aimStuff.aimTargetTimer; }

float CTweakPlayer::GetAimAssistHorizontalAngle() const {
  return CRelAngle::FromDegrees(mData->aimStuff.aimAssistHorizontalAngle).AsRadians();
}

float CTweakPlayer::GetAimAssistVerticalAngle() const {
  return CRelAngle::FromDegrees(mData->aimStuff.aimAssistVerticalAngle).AsRadians();
}

float CTweakPlayer::GetPlayerHeight() const { return mData->collision.playerHeight; }

float CTweakPlayer::GetPlayerRadius() const { return mData->collision.playerRadius; }

float CTweakPlayer::GetStepUpHeight() const { return mData->collision.stepUpHeight; }

float CTweakPlayer::GetStepDownHeight() const { return mData->collision.stepDownHeight; }

float CTweakPlayer::GetBallRadius() const { return mData->collision.ballRadius; }

float CTweakPlayer::GetFirstPersonCameraSpeed() const {
  return CRelAngle::FromDegrees(mData->firstPersonCamera.firstPersonCameraSpeed).AsRadians();
}

float CTweakPlayer::GetJumpCameraPitchDownStart() const {
  return mData->firstPersonCamera.unknown_0xb400ebd6;
}

float CTweakPlayer::GetJumpCameraPitchDownDuration() const {
  return mData->firstPersonCamera.unknown_0xfd26b7b9;
}

float CTweakPlayer::GetJumpCameraPitchDownAngle() const {
  return CRelAngle::FromDegrees(mData->firstPersonCamera.unknown_0x97b14dc6).AsRadians();
}

float CTweakPlayer::GetFallCameraPitchDownStart() const {
  return mData->firstPersonCamera.unknown_0xeb59925a;
}

float CTweakPlayer::GetFallCameraPitchDownDuration() const {
  return mData->firstPersonCamera.unknown_0xa1d73380;
}

float CTweakPlayer::GetFallCameraPitchDownAngle() const {
  return CRelAngle::FromDegrees(mData->firstPersonCamera.unknown_0xc8e8344a).AsRadians();
}

float CTweakPlayer::GetFrozenTimeout() const { return mData->frozen.frozenTimer; }

int CTweakPlayer::GetIceBreakJumpCount() const { return mData->frozen.frozenJumpCounter; }

float CTweakPlayer::GetFrozenDamageThreshold() const { return mData->frozen.frozenDamageThreshold; }

float CTweakPlayer::GetVariaSuitDamageReduction() { return mData->suitDamageReduction.varia; }

float CTweakPlayer::GetDarkSuitDamageReduction() { return mData->suitDamageReduction.dark; }

float CTweakPlayer::GetLightSuitDamageReduction() { return mData->suitDamageReduction.light; }

float CTweakPlayer::GetMaxTranslationalAcceleration(CPlayer::ESurfaceRestraints surface) const {
  switch (surface) {
  default:
  case CPlayer::kSR_Normal:
    return mData->motion.forwardAccelNormal;
  case CPlayer::kSR_Air:
    return mData->motion.forwardAccelAir;
  case CPlayer::kSR_Ice:
    return mData->motion.forwardAccelIce;
  case CPlayer::kSR_Organic:
    return mData->motion.forwardAccelOrganic;
  case CPlayer::kSR_Water:
    return mData->motion.forwardAccelWater;
  case CPlayer::kSR_Lava:
    return mData->motion.forwardAccelLava;
  case CPlayer::kSR_Phazon:
    return mData->motion.forwardAccelPhazon;
  case CPlayer::kSR_Shrubbery:
    return mData->motion.forwardAccelShrubbery;
  }
}

float CTweakPlayer::GetMaxRotationalAcceleration(CPlayer::ESurfaceRestraints surface) const {
  switch (surface) {
  default:
  case CPlayer::kSR_Normal:
    return mData->motion.rotationalAccelNormal;
  case CPlayer::kSR_Air:
    return mData->motion.rotationalAccelAir;
  case CPlayer::kSR_Ice:
    return mData->motion.rotationalAccelIce;
  case CPlayer::kSR_Organic:
    return mData->motion.rotationalAccelOrganic;
  case CPlayer::kSR_Water:
    return mData->motion.rotationalAccelWater;
  case CPlayer::kSR_Lava:
    return mData->motion.rotationalAccelLava;
  case CPlayer::kSR_Phazon:
    return mData->motion.rotationalAccelPhazon;
  case CPlayer::kSR_Shrubbery:
    return mData->motion.rotationalAccelShrubbery;
  }
}

float CTweakPlayer::GetPlayerTranslationFriction(CPlayer::ESurfaceRestraints surface) const {
  switch (surface) {
  default:
  case CPlayer::kSR_Normal:
    return mData->motion.movementFrictionNormal;
  case CPlayer::kSR_Air:
    return mData->motion.movementFrictionAir;
  case CPlayer::kSR_Ice:
    return mData->motion.movementFrictionIce;
  case CPlayer::kSR_Organic:
    return mData->motion.movementFrictionOrganic;
  case CPlayer::kSR_Water:
    return mData->motion.movementFrictionWater;
  case CPlayer::kSR_Lava:
    return mData->motion.movementFrictionLava;
  case CPlayer::kSR_Phazon:
    return mData->motion.movementFrictionPhazon;
  case CPlayer::kSR_Shrubbery:
    return mData->motion.movementFrictionShrubbery;
  }
}

float CTweakPlayer::GetPlayerRotationFriction(CPlayer::ESurfaceRestraints surface) const {
  switch (surface) {
  default:
  case CPlayer::kSR_Normal:
    return mData->motion.rotationFrictionNormal;
  case CPlayer::kSR_Air:
    return mData->motion.rotationFrictionAir;
  case CPlayer::kSR_Ice:
    return mData->motion.rotationFrictionIce;
  case CPlayer::kSR_Organic:
    return mData->motion.rotationFrictionOrganic;
  case CPlayer::kSR_Water:
    return mData->motion.rotationFrictionWater;
  case CPlayer::kSR_Lava:
    return mData->motion.rotationFrictionLava;
  case CPlayer::kSR_Phazon:
    return mData->motion.rotationFrictionPhazon;
  case CPlayer::kSR_Shrubbery:
    return mData->motion.rotationFrictionShrubbery;
  }
}

float CTweakPlayer::GetPlayerRotationMaxSpeed(CPlayer::ESurfaceRestraints surface) const {
  switch (surface) {
  default:
  case CPlayer::kSR_Normal:
    return mData->motion.rotationMaxSpeedNormal;
  case CPlayer::kSR_Air:
    return mData->motion.rotationMaxSpeedAir;
  case CPlayer::kSR_Ice:
    return mData->motion.rotationMaxSpeedIce;
  case CPlayer::kSR_Organic:
    return mData->motion.rotationMaxSpeedOrganic;
  case CPlayer::kSR_Water:
    return mData->motion.rotationMaxSpeedWater;
  case CPlayer::kSR_Lava:
    return mData->motion.rotationMaxSpeedLava;
  case CPlayer::kSR_Phazon:
    return mData->motion.rotationMaxSpeedPhazon;
  case CPlayer::kSR_Shrubbery:
    return mData->motion.rotationMaxSpeedShrubbery;
  }
}

float CTweakPlayer::GetPlayerTranslationMaxSpeed(CPlayer::ESurfaceRestraints surface) const {
  switch (surface) {
  default:
  case CPlayer::kSR_Normal:
    return mData->motion.forwardMaxSpeedNormal;
  case CPlayer::kSR_Air:
    return mData->motion.forwardMaxSpeedAir;
  case CPlayer::kSR_Ice:
    return mData->motion.forwardMaxSpeedIce;
  case CPlayer::kSR_Organic:
    return mData->motion.forwardMaxSpeedOrganic;
  case CPlayer::kSR_Water:
    return mData->motion.forwardMaxSpeedWater;
  case CPlayer::kSR_Lava:
    return mData->motion.forwardMaxSpeedLava;
  case CPlayer::kSR_Phazon:
    return mData->motion.forwardMaxSpeedPhazon;
  case CPlayer::kSR_Shrubbery:
    return mData->motion.forwardMaxSpeedShrubbery;
  }
}

float CTweakPlayer::GetDarkWorldDamageGracePeriod() const {
  return mData->darkWorld.damageGracePeriod;
}

float CTweakPlayer::GetDarkWorldDamageRecoveryRate() const {
  return mData->darkWorld.unknown_0xa4e33ef0;
}

CDamageInfo CTweakPlayer::GetDarkWorldDamageInfo() const {
  return LdrToDamageInfo(mData->darkWorld.damagePerSecond);
}

float CTweakPlayer::GetDarkWorldDamageReduction() const {
  return mData->darkWorld.darkSuitDamageReduction;
}

float CTweakPlayer::GetDarkSuitEffectGenerationScale() const {
  return mData->darkWorld.darkSuitEffectGenerationScale;
}

float CTweakPlayer::GetDarkSuitEffectColorScale() const {
  return mData->darkWorld.darkSuitEffectColorScale;
}
