#ifndef _SLDRTWEAKPLAYERGUN_HOLSTERING
#define _SLDRTWEAKPLAYERGUN_HOLSTERING

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayerGun_Holstering {
    SLdrTweakPlayerGun_Holstering();
    ~SLdrTweakPlayerGun_Holstering();

    float gun_holster_time;
    float gun_not_firing_time;
    float gun_holstered_angle;
};

void LoadTypedefSLdrTweakPlayerGun_Holstering(SLdrTweakPlayerGun_Holstering&, CInputStream&);

#endif // _SLDRTWEAKPLAYERGUN_HOLSTERING
