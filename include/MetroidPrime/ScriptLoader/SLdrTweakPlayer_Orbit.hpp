#ifndef _SLDRTWEAKPLAYER_ORBIT
#define _SLDRTWEAKPLAYER_ORBIT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_Orbit {
    SLdrTweakPlayer_Orbit();
    ~SLdrTweakPlayer_Orbit();

    float orbitCloseMinDistance;
    float orbitCloseNormalDistance;
    float orbitCloseMaxDistance;
    float orbitFarMinDistance;
    float orbitFarNormalDistance;
    float orbitFarMaxDistance;
    float orbitCarcassMinDistance;
    float orbitCarcassNormalDistance;
    float orbitCarcassMaxDistance;
    float orbitMaxAngularChange;
    float orbitModeTimer;
    float orbitCameraSpeed;
    float orbitUpperAngle;
    float orbitLowerAngle;
    float orbitHorizAngle;
    float orbitUpperCameraAngle;
    float orbitLowerCameraAngle;
    float orbitMaxTargetDistance;
    float orbitMaxLockDistance;
    float unknown_0x55f7d145;
    float orbitDistanceThreshold;
    int orbitZoneWidth;
    int orbitZoneHeight;
    int unknown_0x58ee9d03;
    int unknown_0xe052fa66;
    int unknown_0xc452b61e;
    int unknown_0x7ceed17b;
    int orbitScanZoneWidth;
    int orbitScanZoneHeight;
    int unknown_0xec529a5e;
    int unknown_0x54eefd3b;
    int unknown_0x73ebdce2;
    int unknown_0xcb57bb87;
    float orbitBoxWidth;
    float orbitBoxHeight;
    float orbitMinCameraPitchDistance;
    float orbitMaxCameraPitchDistance;
    float unknown_0x478c15f9;
    float orbitZRange;
    float orbitSelectionCloseAngle;
    float orbitSelectionMaxAngle;
    float unknown_0x90b71b2e;
    float orbitPreventionTime;
    bool orbitDash;
    bool orbitDashUsesTap;
    float orbitDashTapTime;
    float orbitDashStickThreshold;
    float orbitDashDoubleJumpImpulse;
    float unknown_0x75a00cfb;
    float unknown_0xc4775e5f;
};

void LoadTypedefSLdrTweakPlayer_Orbit(SLdrTweakPlayer_Orbit&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_ORBIT
