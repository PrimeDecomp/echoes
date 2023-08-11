#ifndef _SLDRTWEAKPLAYERGUN_HOLSTERING
#define _SLDRTWEAKPLAYERGUN_HOLSTERING

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayerGun_Holstering {
    SLdrTweakPlayerGun_Holstering();
    ~SLdrTweakPlayerGun_Holstering();

    float gunHolsterTime;
    float gunNotFiringTime;
    float gunHolsteredAngle;
};

void LoadTypedefSLdrTweakPlayerGun_Holstering(SLdrTweakPlayerGun_Holstering&, CInputStream&);

#endif // _SLDRTWEAKPLAYERGUN_HOLSTERING
