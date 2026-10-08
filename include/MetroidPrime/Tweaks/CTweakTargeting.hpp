#ifndef _CTWEAKTARGETING
#define _CTWEAKTARGETING

#include "Kyoto/Graphics/CColor.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "rstl/single_ptr.hpp"

struct SLdrTweakTargeting;

class CTweakTargeting {
public:
  explicit CTweakTargeting(const SLdrTweakTargeting& data) : mData(&data) {}

  // Guessed name, correlated with Prime.
  int GetTargetRadiusMode() const;
  // Guessed name, correlated with Prime.
  float GetCurrLockOnExitDuration() const;
  // Guessed name, correlated with Prime.
  float GetCurrLockOnEnterDuration() const;
  // Guessed name, correlated with Prime.
  float GetCurrLockOnSwitchDuration() const;
  float GetLockOnConfirmReticleScale() const;
  // Guessed name, correlated with Prime.
  float GetNextLockOnEnterDuration() const;
  // Guessed name, correlated with Prime.
  float GetNextLockOnExitDuration() const;
  // Guessed name, correlated with Prime.
  float GetNextLockOnSwitchDuration() const;
  float GetSeekerTargetReticleScale() const;
  float GetSeekerZRotationRate() const;
  // Guessed name, correlated with Prime.
  float GetXRayReticleRotationRate() const;
  float GetOrbitPointZOffset() const;
  float GetOrbitPointInterpolateInTime() const;
  float GetOrbitPointInterpolateOutTime() const;
  float GetFlowerReticleScale() const;
  CColor GetFlowerReticleColor() const;
  // Guessed name, correlated with Prime.
  float GetMissileBracketOpenHolsterTime() const;
  // Guessed name, correlated with Prime.
  float GetMissileBracketScaleStart() const;
  // Guessed name, correlated with Prime.
  float GetMissileBracketScaleEnd() const;
  // Guessed name, correlated with Prime.
  float GetMissileBracketMissileFireAnimTime() const;
  CColor GetMissileBracketColor() const;
  float GetInnerBeamIconOpenTime() const;
  float GetInnerBeamIconScale() const;
  // Guessed name, correlated with Prime.
  float GetChargeGaugeOvershootOffset() const;
  float GetOuterBeamIconSwitchTime() const;
  float GetOuterBeamIconScale() const;
  CColor GetOuterBeamIconColor() const;
  float GetChargeGaugeScale() const;
  CColor GetChargeGaugeColor() const;
  // Guessed name, correlated with Prime.
  int GetChargeTickCount() const;
  float GetChargeGaugeTickDeltaAngle() const;
  float GetLockFireReticleScale() const;
  float GetLockFireAnimTime() const;
  CColor GetLockFireColor() const;
  float GetLockDaggerNormalScale() const;
  // Guessed name, correlated with Prime.
  float GetLockDaggerScaleEnd() const;
  CColor GetLockDaggerColor() const;
  float GetLockDagger0Angle() const;
  float GetLockDagger1Angle() const;
  float GetLockDagger2Angle() const;
  CColor GetLockOnConfirmReticleColor() const;
  CColor GetSeekerReticleColor() const;
  float GetLockOnConfirmMinRadiusViewport() const;
  float GetLockOnConfirmMaxRadiusViewport() const;
  float GetFlowerMinRadiusViewport() const;
  float GetFlowerMaxRadiusViewport() const;
  float GetSeekerMinRadiusViewport() const;
  float GetSeekerMaxRadiusViewport() const;
  float GetMissileBracketMinRadiusViewport() const;
  float GetMissileBracketMaxRadiusViewport() const;
  float GetInnerIconMinRadiusViewport() const;
  float GetInnerIconMaxRadiusViewport() const;
  float GetOuterIconMinRadiusViewport() const;
  float GetOuterIconMaxRadiusViewport() const;
  float GetLockFireMinRadiusViewport() const;
  float GetLockFireMaxRadiusViewport() const;
  float GetLockDaggerMinRadiusViewport() const;
  float GetLockDaggerMaxRadiusViewport() const;
  float GetGrappleIconScale() const;
  float GetGrappleIconScaleInactive() const;
  float GetGrappleIconMinRadiusViewport() const;
  float GetGrappleIconMaxRadiusViewport() const;
  CColor GetGrappleIconColor() const;
  CColor GetGrappleIconColorInactive() const;
  // Guessed name, correlated with Prime.
  CColor GetLockedGrapplePointColor() const;
  // Guessed name, correlated with Prime.
  float GetGrappleMinClampScale() const;
  CColor GetChargeGaugeGlowColor() const;
  float GetChargeGaugeGlowTime() const;
  CColor GetOrbitPointModelColor() const;
  CColor GetCrosshairsColor() const;
  float GetCrosshairsFadeInOutTime() const;
  // Guessed name, correlated with Prime.
  bool GetDrawOrbitPoint() const;
  CColor GetChargeGaugeGlowColorB() const;
  // Guessed name, correlated with Prime.
  float GetChargeGaugePulsePeriod() const;
  float GetAngularLagSpeed() const;
  float GetHealthMeterRadius() const;
  float GetHealthMeterLagFillTime() const;
  float GetHealthMeterLagDrainTime() const;
  float GetHealthMeterShadowDrainTime() const;
  CColor GetHealthColor() const;
  float GetScanLockScale() const;
  float GetScanLockTransitionTime() const;
  float GetScanLockTranslation() const;
  const CColor& GetScanLockCrossHairColor() const;
  const CColor& GetScanLockLockedColor() const;
  const CColor& GetScanLockUnlockedColor() const;

  float GetOuterBeamIconAngle(CPlayerState::EBeamId beam, int index) const;
  float GetChargeGaugeAngle(CPlayerState::EBeamId beam) const;

private:
  const SLdrTweakTargeting* mData;
};
CHECK_SIZEOF(CTweakTargeting, 0x4)

extern rstl::single_ptr< CTweakTargeting > gpTweakTargeting;

#endif // _CTWEAKTARGETING
