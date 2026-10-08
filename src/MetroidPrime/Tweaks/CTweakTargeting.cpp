#include "MetroidPrime/Tweaks/CTweakTargeting.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting.hpp"

int CTweakTargeting::GetTargetRadiusMode() const { return mData->unknown_0xc3410560; }

float CTweakTargeting::GetCurrLockOnExitDuration() const { return mData->unknown_0x3eb13041; }

float CTweakTargeting::GetCurrLockOnEnterDuration() const { return mData->unknown_0x5e67cab0; }

float CTweakTargeting::GetCurrLockOnSwitchDuration() const { return mData->unknown_0xe0ca98ac; }

float CTweakTargeting::GetLockOnConfirmReticleScale() const {
  return mData->lockOnConfirmReticleScale;
}

float CTweakTargeting::GetNextLockOnEnterDuration() const { return mData->unknown_0x9f8d62c1; }

float CTweakTargeting::GetNextLockOnExitDuration() const { return mData->unknown_0xff5eeeb9; }

float CTweakTargeting::GetNextLockOnSwitchDuration() const { return mData->unknown_0x0d0b660d; }

float CTweakTargeting::GetSeekerTargetReticleScale() const {
  return mData->seekerTargetReticleScale;
}

float CTweakTargeting::GetSeekerZRotationRate() const { return mData->seekerZRotationRate; }

float CTweakTargeting::GetXRayReticleRotationRate() const { return mData->unknown_0xdcbd7bf8; }

float CTweakTargeting::GetOrbitPointZOffset() const { return mData->orbitPointZOffset; }

float CTweakTargeting::GetOrbitPointInterpolateInTime() const {
  return mData->orbitPointInterpolateInTime;
}

float CTweakTargeting::GetOrbitPointInterpolateOutTime() const {
  return mData->orbitPointInterpolateOutTime;
}

float CTweakTargeting::GetFlowerReticleScale() const { return mData->flowerReticleScale; }

CColor CTweakTargeting::GetFlowerReticleColor() const { return mData->flowerReticleColor; }

float CTweakTargeting::GetMissileBracketOpenHolsterTime() const {
  return mData->missileBracketOpenHolsterTime;
}

float CTweakTargeting::GetMissileBracketScaleStart() const { return mData->unknown_0x4c73a43d; }

float CTweakTargeting::GetMissileBracketScaleEnd() const { return mData->unknown_0x6543d31b; }

float CTweakTargeting::GetMissileBracketMissileFireAnimTime() const {
  return mData->missileBracketMissileFireAnimTime;
}

CColor CTweakTargeting::GetMissileBracketColor() const { return mData->missileBracketColor; }

float CTweakTargeting::GetInnerBeamIconOpenTime() const { return mData->innerBeamIconOpenTime; }

float CTweakTargeting::GetInnerBeamIconScale() const { return mData->innerBeamIconScale; }

float CTweakTargeting::GetChargeGaugeOvershootOffset() const {
  return mData->tweakTargeting_OuterBeamIcon.unknown_0x383e2b2d;
}

float CTweakTargeting::GetOuterBeamIconSwitchTime() const {
  return mData->tweakTargeting_OuterBeamIcon.outerBeamIconSwitchTime;
}

float CTweakTargeting::GetOuterBeamIconScale() const {
  return mData->tweakTargeting_OuterBeamIcon.outerBeamIconScale;
}

CColor CTweakTargeting::GetOuterBeamIconColor() const {
  return mData->tweakTargeting_OuterBeamIcon.outerBeamIconColor;
}

float CTweakTargeting::GetChargeGaugeScale() const { return mData->charge_Gauge.chargeGaugeScale; }

CColor CTweakTargeting::GetChargeGaugeColor() const { return mData->charge_Gauge.chargeGaugeColor; }

int CTweakTargeting::GetChargeTickCount() const { return mData->charge_Gauge.unknown_0xed78e6eb; }

float CTweakTargeting::GetChargeGaugeTickDeltaAngle() const {
  return -1.f * CMath::Deg2Rad(mData->charge_Gauge.chargeGaugeTickDeltaAngle);
}

float CTweakTargeting::GetLockFireReticleScale() const {
  return mData->lockFire.lockFireReticleScale;
}

float CTweakTargeting::GetLockFireAnimTime() const { return mData->lockFire.lockFireAnimTime; }

CColor CTweakTargeting::GetLockFireColor() const { return mData->lockFire.lockFireColor; }

float CTweakTargeting::GetLockDaggerNormalScale() const {
  return mData->lockDagger.lockDaggerNormalScale;
}

float CTweakTargeting::GetLockDaggerScaleEnd() const {
  return mData->lockDagger.unknown_0x7b48e6f9;
}

CColor CTweakTargeting::GetLockDaggerColor() const { return mData->lockDagger.lockDaggerColor; }

float CTweakTargeting::GetLockDagger0Angle() const {
  return CMath::Deg2Rad(mData->lockDagger.lockDagger0Angle);
}

float CTweakTargeting::GetLockDagger1Angle() const {
  return CMath::Deg2Rad(mData->lockDagger.lockDagger1Angle);
}

float CTweakTargeting::GetLockDagger2Angle() const {
  return CMath::Deg2Rad(mData->lockDagger.lockDagger2Angle);
}

CColor CTweakTargeting::GetLockOnConfirmReticleColor() const {
  return mData->lockOnConfirmReticleColor;
}

CColor CTweakTargeting::GetSeekerReticleColor() const { return mData->seekerReticleColor; }

float CTweakTargeting::GetLockOnConfirmMinRadiusViewport() const {
  return mData->lockOnConfirmMinRadiusViewport;
}

float CTweakTargeting::GetLockOnConfirmMaxRadiusViewport() const {
  return mData->lockOnConfirmMaxRadiusViewport;
}

float CTweakTargeting::GetFlowerMinRadiusViewport() const { return mData->flowerMinRadiusViewport; }

float CTweakTargeting::GetFlowerMaxRadiusViewport() const { return mData->flowerMaxRadiusViewport; }

float CTweakTargeting::GetSeekerMinRadiusViewport() const { return mData->seekerMinRadiusViewport; }

float CTweakTargeting::GetSeekerMaxRadiusViewport() const { return mData->seekerMaxRadiusViewport; }

float CTweakTargeting::GetMissileBracketMinRadiusViewport() const {
  return mData->missileBracketMinRadiusViewport;
}

float CTweakTargeting::GetMissileBracketMaxRadiusViewport() const {
  return mData->missileBracketMaxRadiusViewport;
}

float CTweakTargeting::GetInnerIconMinRadiusViewport() const {
  return mData->innerIconMinRadiusViewport;
}

float CTweakTargeting::GetInnerIconMaxRadiusViewport() const {
  return mData->innerIconMaxRadiusViewport;
}

float CTweakTargeting::GetOuterIconMinRadiusViewport() const {
  return mData->outerIconMinRadiusViewport;
}

float CTweakTargeting::GetOuterIconMaxRadiusViewport() const {
  return mData->outerIconMaxRadiusViewport;
}

float CTweakTargeting::GetLockFireMinRadiusViewport() const {
  return mData->lockFireMinRadiusViewport;
}

float CTweakTargeting::GetLockFireMaxRadiusViewport() const {
  return mData->lockFireMaxRadiusViewport;
}

float CTweakTargeting::GetLockDaggerMinRadiusViewport() const {
  return mData->lockDaggerMinRadiusViewport;
}

float CTweakTargeting::GetLockDaggerMaxRadiusViewport() const {
  return mData->lockDaggerMaxRadiusViewport;
}

float CTweakTargeting::GetGrappleIconScale() const { return mData->grappleIconScale; }

float CTweakTargeting::GetGrappleIconScaleInactive() const {
  return mData->grappleIconScaleInactive;
}

float CTweakTargeting::GetGrappleIconMinRadiusViewport() const {
  return mData->grappleIconMinRadiusViewport;
}

float CTweakTargeting::GetGrappleIconMaxRadiusViewport() const {
  return mData->grappleIconMaxRadiusViewport;
}

CColor CTweakTargeting::GetGrappleIconColor() const { return mData->grappleIconColor; }

CColor CTweakTargeting::GetGrappleIconColorInactive() const {
  return mData->grappleIconColorInactive;
}

CColor CTweakTargeting::GetLockedGrapplePointColor() const { return mData->unknown_0x083b1cc8; }

float CTweakTargeting::GetGrappleMinClampScale() const { return mData->unknown_0x966982b1; }

CColor CTweakTargeting::GetChargeGaugeGlowColor() const { return mData->chargeGaugeGlowColor; }

float CTweakTargeting::GetChargeGaugeGlowTime() const { return mData->chargeGaugeGlowTime; }

CColor CTweakTargeting::GetOrbitPointModelColor() const { return mData->orbitPointModelColor; }

CColor CTweakTargeting::GetCrosshairsColor() const { return mData->crosshairsColor; }

float CTweakTargeting::GetCrosshairsFadeInOutTime() const { return mData->crosshairsFadeInOutTime; }

bool CTweakTargeting::GetDrawOrbitPoint() const { return mData->unknown_0x8a548cc9; }

CColor CTweakTargeting::GetChargeGaugeGlowColorB() const { return mData->chargeGaugeGlowColorB; }

float CTweakTargeting::GetChargeGaugePulsePeriod() const { return mData->unknown_0x42c7fbe4; }

float CTweakTargeting::GetAngularLagSpeed() const { return mData->unknown_0xcd1e0e91; }

float CTweakTargeting::GetHealthMeterRadius() const { return mData->healthMeterRadius; }

float CTweakTargeting::GetHealthMeterLagFillTime() const { return mData->healthMeterLagFillTime; }

float CTweakTargeting::GetHealthMeterLagDrainTime() const { return mData->healthMeterLagDrainTime; }

float CTweakTargeting::GetHealthMeterShadowDrainTime() const {
  return mData->healthMeterShadowDrainTime;
}

CColor CTweakTargeting::GetHealthColor() const { return mData->healthColor; }

float CTweakTargeting::GetScanLockScale() const { return mData->scan.scanLockScale; }

float CTweakTargeting::GetScanLockTransitionTime() const {
  return mData->scan.scanLockTransitionTime;
}

float CTweakTargeting::GetScanLockTranslation() const { return mData->scan.scanLockTranslation; }

const CColor& CTweakTargeting::GetScanLockCrossHairColor() const {
  return mData->scan.scanLockCrossHairColor;
}

const CColor& CTweakTargeting::GetScanLockLockedColor() const {
  return mData->scan.scanLockLockedColor;
}

const CColor& CTweakTargeting::GetScanLockUnlockedColor() const {
  return mData->scan.scanLockUnlockedColor;
}

static float GetIconConfigurationAngle(const SLdrTIcon_Configurations& configuration, int index) {
  switch (index) {
  default:
  case 0:
    return configuration.something0Angle;
  case 1:
    return configuration.something1Angle;
  case 2:
    return configuration.something2Angle;
  case 3:
    return configuration.something3Angle;
  case 4:
    return configuration.something4Angle;
  case 5:
    return configuration.something5Angle;
  case 6:
    return configuration.something6Angle;
  case 7:
    return configuration.something7Angle;
  case 8:
    return configuration.something8Angle;
  }
}

float CTweakTargeting::GetOuterBeamIconAngle(CPlayerState::EBeamId beam, int index) const {
  switch (beam) {
  default:
  case CPlayerState::kBI_Power:
    return GetIconConfigurationAngle(mData->power_Beam_Icon_Configurations, index);
  case CPlayerState::kBI_Dark:
    return GetIconConfigurationAngle(mData->ice_Beam_Icon_Configurations, index);
  case CPlayerState::kBI_Light:
    return GetIconConfigurationAngle(mData->wave_Beam_Icon_Configurations, index);
  case CPlayerState::kBI_Annihilator:
    return GetIconConfigurationAngle(mData->plasma_Beam_Icon_Configurations, index);
  }
}

float CTweakTargeting::GetChargeGaugeAngle(CPlayerState::EBeamId beam) const {
  switch (beam) {
  default:
  case CPlayerState::kBI_Power:
    return mData->charge_Gauge.chargeGaugePowerBeamAngle;
  case CPlayerState::kBI_Dark:
    return mData->charge_Gauge.chargeGaugeIceBeamAngle;
  case CPlayerState::kBI_Light:
    return mData->charge_Gauge.chargeGaugeWaveBeamAngle;
  case CPlayerState::kBI_Annihilator:
    return mData->charge_Gauge.chargeGaugePlasmaBeamAngle;
  }
}
