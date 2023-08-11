#ifndef _SLDRTWEAKPLAYER_AIMSTUFF
#define _SLDRTWEAKPLAYER_AIMSTUFF

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_AimStuff {
    SLdrTweakPlayer_AimStuff();
    ~SLdrTweakPlayer_AimStuff();

    float aimMinTime;
    float aimMaxTime;
    float aimMaxDistance;
    float aimMaxAngleLeft;
    float aimMaxAngleRight;
    float aimMaxAngleUp;
    float aimMaxAngleDown;
    float aimAnglePerSecond;
    float aimThresholdDistance;
    float aimTurnAnglePerSecond;
    float unknown;
    float aimBoxWidth;
    float aimBoxHeight;
    float aimTargetTimer;
    float aimAssistHorizontalAngle;
    float aimAssistVerticalAngle;
};

void LoadTypedefSLdrTweakPlayer_AimStuff(SLdrTweakPlayer_AimStuff&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_AIMSTUFF
