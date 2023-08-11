#ifndef _SLDRTWEAKBALL_CAMERA
#define _SLDRTWEAKBALL_CAMERA

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakBall_Camera {
    SLdrTweakBall_Camera();
    ~SLdrTweakBall_Camera();

    float ballCameraAnglePerSecond;
    CVector3f ballCameraOffset;
    float ballCameraMinSpeedDistance;
    float ballCameraMaxSpeedDistance;
    float ballCameraBackwardsDistance;
    float ballCameraElevation;
    float ballCameraSpringConstant;
    float ballCameraSpringMax;
    float ballCameraSpringTardis;
    float ballCameraCentroidSpringConstant;
    float ballCameraCentroidSpringMax;
    float ballCameraCentroidSpringTardis;
    float ballCameraCentroidDistanceSpringConstant;
    float ballCameraCentroidDistanceSpringMax;
    float ballCameraCentroidDistanceSpringTardis;
    float ballCameraLookAtSpringConstant;
    float ballCameraLookAtSpringMax;
    float ballCameraLookAtSpringTardis;
    float ballCameraTransitionTime;
    float ballCameraFreeLookSpeed;
    float ballCameraFreeLookZoomSpeed;
    float ballCameraFreeLookMinDistance;
    float ballCameraFreeLookMaxDistance;
    float ballCameraFreeLookMaxVertAngle;
    float unknown_0x144db504;
    float unknown_0xee5bea64;
    float ballCameraChaseDistance;
    float ballCameraChaseElevation;
    float ballCameraChaseYawSpeed;
    float ballCameraChaseDampenAngle;
    float ballCameraChaseAnglePerSecond;
    CVector3f ballCameraChaseLookAtOffset;
    float ballCameraChaseSpringConstant;
    float ballCameraChaseSpringMax;
    float ballCameraChaseSpringTardis;
    float ballCameraBoostDistance;
    float ballCameraBoostElevation;
    float ballCameraBoostYawSpeed;
    float ballCameraBoostDampenAngle;
    float ballCameraBoostAnglePerSecond;
    CVector3f ballCameraBoostLookAtOffset;
    float ballCameraBoostSpringConstant;
    float ballCameraBoostSpringMax;
    float ballCameraBoostSpringTardis;
    float ballCameraControlDistance;
    float ballCameraLookAtMinHeight;
    float unknown_0x50f77df0;
};

void LoadTypedefSLdrTweakBall_Camera(SLdrTweakBall_Camera&, CInputStream&);

#endif // _SLDRTWEAKBALL_CAMERA
