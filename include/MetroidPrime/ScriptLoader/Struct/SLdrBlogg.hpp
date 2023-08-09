#ifndef _SLDRBLOGG
#define _SLDRBLOGG

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrBloggStruct.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrBlogg {
    SLdrBlogg();
    ~SLdrBlogg();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    float min_attack_angle;
    float max_attack_angle;
    float unknown_0x32455774;
    float unknown_0xc1c8d862;
    int unknown_0xa19d5f62;
    CAssetId projectile_particle_effect;
    SLdrDamageInfo projectile_damage;
    float body_damage_multiplier;
    float mouth_damage_multiplier;
    float mouth_damage_angle;
    SLdrDamageVulnerability armor_vulnerability;
    float charge_damage_radius;
    float charge_damage;
    float bite_damage;
    float ball_spit_damage;
    float charge_turn_speed;
    float fish_attraction_radius;
    float fish_attraction_priority;
    float aggressiveness;
    float unknown_0x479ccc37;
    float unknown_0x689a803f;
    float unknown_0x800a2b0d;
    float charge_speed_multiplier;
    float max_melee_range;
    float unknown_0x6a78c607;
    float unknown_0x2c8d9fc4;
    float unknown_0xd1f82f92;
    float unknown_0x5109fb4e;
    float max_collision_time;
    int mouth_open_sound;
    SLdrIngPossessionData ing_possession_data;
    SLdrDamageVulnerability ing_possessed_armor_vulnerability;
    bool is_mega_blogg;
    float projectile_blur_radius;
    float projectile_blur_time;
    SLdrBloggStruct blogg_struct_0x3874576d;
    SLdrBloggStruct blogg_struct_0x97dd1aa7;
    SLdrBloggStruct blogg_struct_0xf2ba21e1;
};

void LoadTypedefSLdrBlogg(SLdrBlogg&, CInputStream&);

#endif // _SLDRBLOGG
