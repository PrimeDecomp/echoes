#ifndef _CTWEAKPLAYER
#define _CTWEAKPLAYER

#include "MetroidPrime/Player/CPlayer.hpp"
#include "rstl/single_ptr.hpp"

struct SLdrTweakPlayer;
class CDamageInfo;

class CTweakPlayer {
public:
  explicit CTweakPlayer(const SLdrTweakPlayer& data) : mData(&data) {}

  float GetNormalGravAccel() const;
  float GetFluidGravAccel() const;
  float GetVerticalJumpAccel() const;
  float GetHorizontalJumpAccel() const;
  float GetVerticalDoubleJumpAccel() const;
  float GetHorizontalDoubleJumpAccel() const;
  float GetWaterJumpFactor() const;
  float GetLavaJumpFactor() const;
  float GetPhazonJumpFactor() const;
  float GetBombJumpHeight() const;
  float GetBombJumpRadius() const;
  float GetAllowedJumpTime() const;
  float GetAllowedDoubleJumpTime() const;
  float GetMinDoubleJumpWindow() const;
  float GetMaxDoubleJumpWindow() const;
  float GetMinJumpTime() const;
  float GetMinDoubleJumpTime() const;
  float GetAllowedLedgeTime() const;
  float GetDoubleJumpImpulse() const;
  float GetBackwardsForceMultiplier() const;
  float GetGravityBoostTime() const;
  float GetGravityBoostForce() const;
  float GetGravityBoostCancelDampening() const;
  bool GetGravityBoostMultipleAllowed() const;
  float GetEyeOffset() const;
  float GetHorizontalFreeLookAngleVel() const;
  float GetVerticalFreeLookAngleVel() const;
  float GetFreeLookSpeed() const;
  float GetFreeLookSnapSpeed() const;
  float GetFreeLookCenteredThresholdAngle() const;
  float GetFreeLookCenteredTime() const;
  bool GetVelocityFreeLookEnabled() const;    // Guessed name.
  float GetVelocityFreeLookThreshold() const; // Guessed name.
  float GetLeftAnalogMax();
  float GetRightAnalogMax();
  float GetTurnSpeedMultiplier() const;
  float GetFreeLookTurnSpeedMultiplier() const;
  float GetOrbitMinDistance(CPlayer::EPlayerOrbitType type) const;
  float GetOrbitNormalDistance(CPlayer::EPlayerOrbitType type) const;
  float GetOrbitMaxDistance(CPlayer::EPlayerOrbitType type) const;
  float GetOrbitModeTimer() const;
  float GetOrbitCameraSpeed() const;
  float GetOrbitUpperAngle() const;
  float GetOrbitLowerAngle() const;
  float GetOrbitHorizAngle() const;
  float GetOrbitMaxLockDistance() const;
  float GetOrbitInvalidTargetTime() const; // Guessed name.
  float GetOrbitMaxTargetDistance() const;
  float GetOrbitDistanceThreshold() const;
  int GetOrbitZoneWidth(CPlayer::EPlayerZoneInfo zone) const;
  int GetOrbitZoneHeight(CPlayer::EPlayerZoneInfo zone) const;
  int GetOrbitZoneCentreX(CPlayer::EPlayerZoneInfo zone) const;
  int GetOrbitZoneCentreY(CPlayer::EPlayerZoneInfo zone) const;
  int GetOrbitZoneIdealX(CPlayer::EPlayerZoneInfo zone) const;
  int GetOrbitZoneIdealY(CPlayer::EPlayerZoneInfo zone) const;
  float GetOrbitBoxWidth() const;
  float GetOrbitBoxHeight() const;
  float GetOrbitFixedOffsetZDiff() const; // Guessed name.
  float GetOrbitZRange() const;
  float GetOrbitPreventionTime() const;
  bool GetDashEnabled() const;
  bool GetDashOnButtonRelease() const;
  float GetDashButtonHoldCancelTime() const;
  float GetDashStrafeInputThreshold() const;
  float GetSidewaysDoubleJumpImpulse() const;
  float GetSidewaysVerticalDoubleJumpAccel() const;
  float GetSidewaysHorizontalDoubleJumpAccel() const;
  float GetScanningRange() const;
  bool GetScanRetention() const;
  bool GetScanFreezesGame() const;
  float GetScanMaxLockDistance() const;
  float GetScanMaxTargetDistance() const;
  float GetScanCameraSpeed() const;
  float GetGrappleDistance() const;
  float GetGrappleSwingLength() const;
  float GetGrappleSwingPeriod() const;
  float GetGrappleMaxVelocity() const;
  float GetGrappleCameraSpeed() const;
  float GetGrapplePullCloseDistance() const;
  float GetGrapplePullDampenDistance() const;
  float GetGrapplePullSpeedMax() const;
  float GetGrapplePullCameraSpeed() const;
  float GetMaxGrappleTurnSpeed() const;
  float GetGrappleJumpForce() const;
  float GetGrappleReleaseTime() const;
  int GetGrappleJumpMode() const;
  bool GetOrbitReleaseBreaksGrapple() const;
  bool GetInvertGrappleTurn() const;
  float GetGrappleBeamSpeed() const;
  float GetGrappleBeamXWaveAmplitude() const;
  float GetGrappleBeamZWaveAmplitude() const;
  float GetGrappleBeamAnglePhaseDelta() const;
  float GetAimMaxDistance() const;
  float GetAimThresholdDistance() const;
  float GetAimBoxWidth() const;
  float GetAimBoxHeight() const;
  float GetAimTargetTimer() const;
  float GetAimAssistHorizontalAngle() const;
  float GetAimAssistVerticalAngle() const;
  float GetPlayerHeight() const;
  float GetPlayerRadius() const;
  float GetStepUpHeight() const;
  float GetStepDownHeight() const;
  float GetBallRadius() const;
  float GetFirstPersonCameraSpeed() const;
  float GetJumpCameraPitchDownStart() const;    // Guessed name.
  float GetJumpCameraPitchDownDuration() const; // Guessed name.
  float GetJumpCameraPitchDownAngle() const;    // Guessed name.
  float GetFallCameraPitchDownStart() const;    // Guessed name.
  float GetFallCameraPitchDownDuration() const; // Guessed name.
  float GetFallCameraPitchDownAngle() const;    // Guessed name.
  float GetFrozenTimeout() const;
  int GetIceBreakJumpCount() const;
  float GetFrozenDamageThreshold() const;
  float GetVariaSuitDamageReduction();
  float GetDarkSuitDamageReduction();
  float GetLightSuitDamageReduction();
  float GetMaxTranslationalAcceleration(CPlayer::ESurfaceRestraints surface) const;
  float GetMaxRotationalAcceleration(CPlayer::ESurfaceRestraints surface) const;
  float GetPlayerTranslationFriction(CPlayer::ESurfaceRestraints surface) const;
  float GetPlayerRotationFriction(CPlayer::ESurfaceRestraints surface) const;
  float GetPlayerRotationMaxSpeed(CPlayer::ESurfaceRestraints surface) const;
  float GetPlayerTranslationMaxSpeed(CPlayer::ESurfaceRestraints surface) const;
  float GetDarkWorldDamageGracePeriod() const;
  float GetDarkWorldDamageRecoveryRate() const; // Guessed name.
  CDamageInfo GetDarkWorldDamageInfo() const;
  float GetDarkWorldDamageReduction() const;
  float GetDarkSuitEffectGenerationScale() const;
  float GetDarkSuitEffectColorScale() const;

private:
  const SLdrTweakPlayer* mData;
};
CHECK_SIZEOF(CTweakPlayer, 0x4)

extern rstl::single_ptr< CTweakPlayer > gpTweakPlayerA;
extern rstl::single_ptr< CTweakPlayer > gpTweakPlayerB;

#endif // _CTWEAKPLAYER
