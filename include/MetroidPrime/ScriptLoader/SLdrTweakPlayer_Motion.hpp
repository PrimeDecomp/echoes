#ifndef _SLDRTWEAKPLAYER_MOTION
#define _SLDRTWEAKPLAYER_MOTION

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_Motion {
    SLdrTweakPlayer_Motion();
    ~SLdrTweakPlayer_Motion();

    float forwardAccelNormal;
    float forwardAccelAir;
    float forwardAccelIce;
    float forwardAccelOrganic;
    float forwardAccelWater;
    float forwardAccelLava;
    float forwardAccelPhazon;
    float forwardAccelShrubbery;
    float rotationalAccelNormal;
    float rotationalAccelAir;
    float rotationalAccelIce;
    float rotationalAccelOrganic;
    float rotationalAccelWater;
    float rotationalAccelLava;
    float rotationalAccelPhazon;
    float rotationalAccelShrubbery;
    float movementFrictionNormal;
    float movementFrictionAir;
    float movementFrictionIce;
    float movementFrictionOrganic;
    float movementFrictionWater;
    float movementFrictionLava;
    float movementFrictionPhazon;
    float movementFrictionShrubbery;
    float rotationFrictionNormal;
    float rotationFrictionAir;
    float rotationFrictionIce;
    float rotationFrictionOrganic;
    float rotationFrictionWater;
    float rotationFrictionLava;
    float rotationFrictionPhazon;
    float rotationFrictionShrubbery;
    float rotationMaxSpeedNormal;
    float rotationMaxSpeedAir;
    float rotationMaxSpeedIce;
    float rotationMaxSpeedOrganic;
    float rotationMaxSpeedWater;
    float rotationMaxSpeedLava;
    float rotationMaxSpeedPhazon;
    float rotationMaxSpeedShrubbery;
    float forwardMaxSpeedNormal;
    float forwardMaxSpeedAir;
    float forwardMaxSpeedIce;
    float forwardMaxSpeedOrganic;
    float forwardMaxSpeedWater;
    float forwardMaxSpeedLava;
    float forwardMaxSpeedPhazon;
    float forwardMaxSpeedShrubbery;
    float gravitationalAccel;
    float fluidGravitationalAccel;
    float verticalJumpAccel;
    float horizontalJumpAccel;
    float verticalDoubleJumpAccel;
    float horizontalDoubleJumpAccel;
    float waterJumpFactor;
    float waterBallJumpFactor;
    float lavaJumpFactor;
    float lavaBallJumpFactor;
    float phazonJumpFactor;
    float phazonBallJumpFactor;
    float allowedJumpTime;
    float allowedDoubleJumpTime;
    float minDoubleJumpWindow;
    float maxDoubleJumpWindow;
    float unknown;
    float minJumpTime;
    float minDoubleJumpTime;
    float ledgeFallTime;
    float doubleJumpImpulse;
    float backwardsForceMultiplier;
    float bombJumpHeight;
    float bombJumpRadius;
    float gravityBoostTime;
    float gravityBoostForce;
    float gravityBoostCancelDampening;
    bool gravityBoostMultipleAllowed;
};

void LoadTypedefSLdrTweakPlayer_Motion(SLdrTweakPlayer_Motion&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_MOTION
