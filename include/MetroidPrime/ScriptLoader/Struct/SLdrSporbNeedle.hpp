#ifndef _SLDRSPORBNEEDLE
#define _SLDRSPORBNEEDLE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrSporbNeedle {
    SLdrSporbNeedle();
    ~SLdrSporbNeedle();

    SLdrEditorProperties editor_properties;
    SLdrActorParameters actor_information;
    CAssetId model;
    float initial_speed;
    float mass;
    SLdrDamageInfo attack_damage;
    float fuse_time;
    CAssetId trail_effect;
    CAssetId explosion_effect;
    int launch_sound;
    int flight_sound;
    int hit_player_sound;
    int collision_sound;
    int explosion_sound;
};

void LoadTypedefSLdrSporbNeedle(SLdrSporbNeedle&, CInputStream&);

#endif // _SLDRSPORBNEEDLE
