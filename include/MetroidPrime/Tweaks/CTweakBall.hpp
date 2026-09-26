#ifndef _CTWEAKBALL
#define _CTWEAKBALL

#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/single_ptr.hpp"

struct SLdrTweakBall;

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

private:
  const SLdrTweakBall* mData;
};
CHECK_SIZEOF(CTweakBall, 0x4)

extern rstl::single_ptr< CTweakBall > gpTweakBall;

#endif // _CTWEAKBALL
