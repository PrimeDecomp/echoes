#include "MetroidPrime/Player/CPlayerCameraBob.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakCameraBob.hpp"

void CPlayerCameraBob::BindTweaks(SLdrTweakCameraBob& data) {
  kCameraBobExtentX = data.cameraBobExtentX;
  kCameraBobExtentY = data.cameraBobExtentY;
  kCameraBobPeriod = data.cameraBobPeriod;
  kOrbitBobScale = data.orbitBobScale;
  kMaxOrbitBobScale = data.maxOrbitBobScale;
  kSlowSpeedPeriodScale = data.slowSpeedPeriodScale;
  kTargetMagnitudeTrackingRate = data.targetMagnitudeTrackingRate;
  kLandingBobSpringConstant = data.landingBobSpringConstant;
  kViewWanderRadius = data.viewWanderRadius;
  kViewWanderSpeedMin = data.viewWanderSpeedMin;
  kViewWanderSpeedMax = data.viewWanderSpeedMax;
  kViewWanderRollVariation = data.viewWanderRollVariation;
  kGunBobMagnitude = data.gunBobMagnitude;
  kHelmetBobMagnitude = data.helmetBobMagnitude;
}
