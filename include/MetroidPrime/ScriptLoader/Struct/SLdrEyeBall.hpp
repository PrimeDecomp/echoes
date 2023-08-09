#ifndef _SLDREYEBALL
#define _SLDREYEBALL

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrEyeBall {
    SLdrEyeBall();
    ~SLdrEyeBall();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    float close_time;
    float fire_wait_time;
    CAssetId projectile;
    SLdrDamageInfo ray_damage;
    CAssetId plasma_burn;
    CAssetId plasma_pulse;
    CAssetId plasma_texture;
    CAssetId plasma_glow;
    CColor laser_inner_color;
    CColor laser_outer_color;
    int unknown_0x81d14be8;
    int unknown_0x6e1320d6;
    int unknown_0x85249bd5;
    int unknown_0x6ae6f0eb;
    int laser_sound;
    bool should_be_triggered;
    float max_audible_distance;
    float drop_off;
};

void LoadTypedefSLdrEyeBall(SLdrEyeBall&, CInputStream&);

#endif // _SLDREYEBALL
