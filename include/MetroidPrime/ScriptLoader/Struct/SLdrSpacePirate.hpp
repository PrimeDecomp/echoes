#ifndef _SLDRSPACEPIRATE
#define _SLDRSPACEPIRATE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSpacePirateWeaponData.hpp"

struct SLdrSpacePirate {
    SLdrSpacePirate();
    ~SLdrSpacePirate();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrIngPossessionData ing_possession_data;
    float aggressiveness;
    float cover_check;
    float search_radius;
    float fall_back_check;
    float fall_back_radius;
    float hearing_radius;
    int sound;
    bool unknown_0xce670970;
    CAssetId projectile;
    SLdrDamageInfo projectile_damage;
    int sound_projectile;
    SLdrDamageInfo blade_damage;
    float kneel_attack_chance;
    CAssetId kneel_attack_shot;
    SLdrDamageInfo kneel_attack_damage;
    float dodge_check;
    int sound_impact;
    float unknown_0x71587b45;
    float unknown_0x7903312e;
    float unknown_0x5080162a;
    float unknown_0xc78b40e0;
    int sound_alert;
    float gun_track_delay;
    int unknown_0x1b454a27;
    float cloak_opacity;
    float max_cloak_opacity;
    float unknown_0x61e801d4;
    float unknown_0xf19b113e;
    int sound_hurled;
    int sound_death;
    float unknown_0x8708b7d3;
    float avoid_distance;
    SLdrSpacePirateWeaponData weapon_data;
};

void LoadTypedefSLdrSpacePirate(SLdrSpacePirate&, CInputStream&);

#endif // _SLDRSPACEPIRATE
