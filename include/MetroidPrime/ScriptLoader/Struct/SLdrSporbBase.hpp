#ifndef _SLDRSPORBBASE
#define _SLDRSPORBBASE

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPowerBombGuardianStageProperties.hpp"

struct SLdrSporbBase {
    SLdrSporbBase();
    ~SLdrSporbBase();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    float unknown_0x95e7a2c2;
    float unknown_0x76ba1c18;
    float unknown_0x3eb2de35;
    float unknown_0xe50d8dd2;
    int unknown_0x64d482d5;
    int unknown_0xc3e002ac;
    float shot_angle_variance;
    CVector3f attack_aim_offset;
    CAssetId tendril_particle_effect;
    float unknown_0x35557a83;
    float grabber_out_acceleration;
    float grabber_in_acceleration;
    float unknown_0xbfddabd4;
    float unknown_0x62bfaa35;
    float grabber_attach_time;
    float unknown_0xed82c56a;
    float unknown_0xe918f440;
    float spit_force;
    float spit_damage;
    float grab_damage;
    float unknown_0x2cfade2c;
    float unknown_0xb68e75cc;
    float unknown_0x6d31262b;
    bool is_power_bomb_guardian;
    CAssetId wpsc;
    SLdrDamageInfo power_bomb_projectile_damage;
    float unknown_0x03a76d35;
    float unknown_0x6d4e0f5a;
    float unknown_0x3538d49b;
    float unknown_0xe89c7707;
    float unknown_0x738d1f51;
    int sound_0x9480c6d7;
    float unknown_0x48df4182;
    float unknown_0xe39482ad;
    float unknown_0xdd8502cc;
    float unknown_0x4ab8cf7d;
    float unknown_0xf5e28404;
    int grabber_fire_sound;
    int grabber_flight_sound;
    int grabber_hit_player_sound;
    int grabber_hit_world_sound;
    int grabber_retract_sound;
    int sound_0x64e9152d;
    int morphball_spit_sound;
    int grabber_explosion_sound;
    int ball_escape_sound;
    int needle_telegraph_sound;
    int grabber_telegraph_sound;
    SLdrPowerBombGuardianStageProperties power_bomb_guardian_stage_properties_0x510dba97;
    SLdrPowerBombGuardianStageProperties power_bomb_guardian_stage_properties_0x0b6c85f7;
    SLdrPowerBombGuardianStageProperties power_bomb_guardian_stage_properties_0x8b9c92e8;
    SLdrPowerBombGuardianStageProperties power_bomb_guardian_stage_properties_0xbfaefb37;
};

void LoadTypedefSLdrSporbBase(SLdrSporbBase&, CInputStream&);

#endif // _SLDRSPORBBASE
