#ifndef _SLDRTWEAKBALL_MOVEMENT
#define _SLDRTWEAKBALL_MOVEMENT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakBall_Movement {
    SLdrTweakBall_Movement();
    ~SLdrTweakBall_Movement();

    float forwardAccelNormal;
    float forwardAccelAir;
    float forwardAccelIce;
    float forwardAccelOrganic;
    float forwardAccelWater;
    float forwardAccelLava;
    float forwardAccelPhazon;
    float forwardAccelShrubbery;
    float movementFrictionNormal;
    float movementFrictionAir;
    float movementFrictionIce;
    float movementFrictionOrganic;
    float movementFrictionWater;
    float movementFrictionLava;
    float movementFrictionPhazon;
    float movementFrictionShrubbery;
    float forwardMaxSpeedNormal;
    float forwardMaxSpeedAir;
    float forwardMaxSpeedIce;
    float forwardMaxSpeedOrganic;
    float forwardMaxSpeedWater;
    float forwardMaxSpeedLava;
    float forwardMaxSpeedPhazon;
    float forwardMaxSpeedShrubbery;
    float unknown_0x85ee51ed;
    float unknown_0x6d7811f5;
    float ballUpGravity;
    float ballDownGravity;
    float ballForwardBrakingAccelNormal;
    float ballForwardBrakingAccelAir;
    float ballForwardBrakingAccelIce;
    float ballForwardBrakingAccelOrganic;
    float ballForwardBrakingAccelWater;
    float ballForwardBrakingAccelLava;
    float ballForwardBrakingAccelPhazon;
    float ballForwardBrakingAccelShrubbery;
    float ballGravity;
    float ballWaterGravity;
    float unknown_0xeeb74968;
    float ballBoostHeight;
    float unknown_0xe2bb0298;
    float minimumAlignmentSpeed;
    float tireness;
    float maxLeanAngle;
    float tireToMarbleThresholdSpeed;
    float marbleToTireThresholdSpeed;
    float forceToLeanGain;
    float leanTrackingGain;
};

void LoadTypedefSLdrTweakBall_Movement(SLdrTweakBall_Movement&, CInputStream&);

#endif // _SLDRTWEAKBALL_MOVEMENT
