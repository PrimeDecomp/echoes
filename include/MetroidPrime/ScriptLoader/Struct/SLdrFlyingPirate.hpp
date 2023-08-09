#ifndef _SLDRFLYINGPIRATE
#define _SLDRFLYINGPIRATE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrFlyingPirate {
    SLdrFlyingPirate();
    ~SLdrFlyingPirate();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    float search_radius;
    float hearing_radius;
    int unknown_0x20daf45e;
    CAssetId projectile;
    SLdrDamageInfo projectile_damage;
    int sound_projectile;
    CAssetId missile;
    SLdrDamageInfo missile_damage;
    CAssetId wpsc;
    float hurl_recover_time;
    float hover_height;
    CAssetId part_0x6475fc6f;
    SLdrDamageInfo rocket_pack_explosion_damage;
    float spiral_chance;
    float minimum_missile_time;
    float unknown_0xb9bb2f64;
    float flight_thrust;
    int sound_impact;
    int sound_spiral;
    float land_chance;
    float unknown_0x71587b45;
    float unknown_0x7903312e;
    CAssetId part_0x317212ab;
    CAssetId part_0xbc113d7b;
    CAssetId part_0x738bbbaa;
    int sound_hurled;
    int sound_death;
    float double_attack_chance;
    float unknown_0x3427d27f;
    float stop_homing_range;
    float unknown_0xccf05648;
    float unknown_0x2a90f9a9;
    float unknown_0x9ca8f357;
    float unknown_0x7ac85cb6;
};

void LoadTypedefSLdrFlyingPirate(SLdrFlyingPirate&, CInputStream&);

#endif // _SLDRFLYINGPIRATE
