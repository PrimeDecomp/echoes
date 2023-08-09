#ifndef _SLDRPLASMABEAMINFO
#define _SLDRPLASMABEAMINFO

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrPlasmaBeamInfo {
    SLdrPlasmaBeamInfo();
    ~SLdrPlasmaBeamInfo();

    int unknown;
    CAssetId weapon_system;
    CAssetId contact_effect;
    CAssetId pulse_effect;
    CAssetId beam_texture;
    CAssetId glow_texture;
    float length;
    float radius;
    float expansion_speed;
    float life_time;
    float pulse_speed;
    float shutdown_time;
    float contact_effect_scale;
    float pulse_effect_scale;
    float travel_speed;
    CColor inner_color;
    CColor outer_color;
    CAssetId beam_streaks;
};

void LoadTypedefSLdrPlasmaBeamInfo(SLdrPlasmaBeamInfo&, CInputStream&);

#endif // _SLDRPLASMABEAMINFO
