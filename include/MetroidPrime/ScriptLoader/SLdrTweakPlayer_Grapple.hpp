#ifndef _SLDRTWEAKPLAYER_GRAPPLE
#define _SLDRTWEAKPLAYER_GRAPPLE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_GrappleBeam.hpp"

struct SLdrTweakPlayer_Grapple {
    SLdrTweakPlayer_Grapple();
    ~SLdrTweakPlayer_Grapple();

    float grappleDistance;
    float grappleBeamLength;
    float grappleSwingTime;
    float grappleMaxVelocity;
    float grappleCameraSpeed;
    float grapplePullCloseDistance;
    float grapplePullDampenDistance;
    float grapplePullVelocity;
    float grapplePullCameraSpeed;
    float grappleTurnRate;
    float grappleJumpForce;
    float grappleReleaseTime;
    int grappleControlScheme;
    bool grappleHoldOrbitButton;
    bool grappleTurnControlsReversed;
    SLdrTweakPlayer_GrappleBeam beam;
};

void LoadTypedefSLdrTweakPlayer_Grapple(SLdrTweakPlayer_Grapple&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_GRAPPLE
