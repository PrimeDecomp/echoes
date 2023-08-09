#ifndef _SLDRTWEAKPLAYERGUN_MISC
#define _SLDRTWEAKPLAYERGUN_MISC

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayerGun_Misc {
    SLdrTweakPlayerGun_Misc();
    ~SLdrTweakPlayerGun_Misc();

    float up_look_angle;
    float down_look_angle;
    float vertical_spread;
    float horizontal_spread;
    float high_vertical_spread;
    float high_horizontal_spread;
    float low_vertical_spread;
    float low_horizontal_spread;
    float aim_vertical_speed;
    float aim_horizontal_speed;
    float hologram_display_time;
    float gun_transform_time;
};

void LoadTypedefSLdrTweakPlayerGun_Misc(SLdrTweakPlayerGun_Misc&, CInputStream&);

#endif // _SLDRTWEAKPLAYERGUN_MISC
