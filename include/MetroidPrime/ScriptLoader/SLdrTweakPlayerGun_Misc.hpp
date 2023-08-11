#ifndef _SLDRTWEAKPLAYERGUN_MISC
#define _SLDRTWEAKPLAYERGUN_MISC

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayerGun_Misc {
    SLdrTweakPlayerGun_Misc();
    ~SLdrTweakPlayerGun_Misc();

    float upLookAngle;
    float downLookAngle;
    float verticalSpread;
    float horizontalSpread;
    float highVerticalSpread;
    float highHorizontalSpread;
    float lowVerticalSpread;
    float lowHorizontalSpread;
    float aimVerticalSpeed;
    float aimHorizontalSpeed;
    float hologramDisplayTime;
    float gunTransformTime;
};

void LoadTypedefSLdrTweakPlayerGun_Misc(SLdrTweakPlayerGun_Misc&, CInputStream&);

#endif // _SLDRTWEAKPLAYERGUN_MISC
