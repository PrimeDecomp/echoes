#ifndef _SLDROCTAPEDESEGMENT
#define _SLDROCTAPEDESEGMENT

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrOctapedeSegment {
    SLdrOctapedeSegment();
    ~SLdrOctapedeSegment();

    SLdrEditorProperties editor_properties;
    int flavor;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    float waypoint_approach_distance;
    float visible_distance;
    float wall_turn_speed;
    float floor_turn_speed;
    float down_turn_speed;
    float unknown_0xd5c25506;
    float projectile_bounds_multiplier;
    float collision_look_ahead;
    float anim_speed_scalar;
    float max_audible_distance;
    bool initially_paused;
    float unknown_0x4fb8747e;
    CAssetId between_segments_effect;
    float unknown_0x9b9c46fc;
    float unknown_0x9f0677d6;
    float unknown_0xc0241fc1;
    float unknown_0xc4be2eeb;
    float unknown_0x99778599;
    float unknown_0xff92e3ed;
    float unknown_0xb8a1f0d5;
    float unknown_0xabe4167e;
    float unknown_0x2caddcbe;
    float unknown_0x4d320455;
    int unknown_0xd6f71bb3;
    int unknown_0x96b863c5;
    float unknown_0x417f4a91;
    SLdrDamageInfo explosion_damage;
    int walk_sound;
    int idle_sound;
    int seperate_sound;
    int bounce_sound;
    int explode_sound;
    float unknown_0x0c4763d7;
};

void LoadTypedefSLdrOctapedeSegment(SLdrOctapedeSegment&, CInputStream&);

#endif // _SLDROCTAPEDESEGMENT
