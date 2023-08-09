#ifndef _SLDRGUNTURRETBASE
#define _SLDRGUNTURRETBASE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrGunTurretBase {
    SLdrGunTurretBase();
    ~SLdrGunTurretBase();

    SLdrEditorProperties editor_properties;
    SLdrDamageInfo attack_damage;
    float hurt_sleep_delay;
    float gun_aim_turn_speed;
    float unknown_0xc80bc7c5;
    float unknown_0x95e7a2c2;
    float unknown_0x76ba1c18;
    float unknown_0x3eb2de35;
    float unknown_0xe50d8dd2;
    int unknown_0x64d482d5;
    int unknown_0xc3e002ac;
    float unknown_0x5ade66a9;
    float unknown_0x8dd2c329;
    float unknown_0xfc036e93;
    float shot_angle_variance;
    float patrol_delay;
    float withdraw_delay;
    float unknown_0x8a35b1ea;
    float unknown_0xd49bec5a;
    float unknown_0x80ce481a;
    float attack_delay;
    float detection_height_up;
    float detection_height_down;
    float attack_leash_time;
    bool gun_respawns;
    bool unknown_0x5cf12e9a;
    bool unknown_0x479d8dc4;
    bool is_pirate_turret;
    CAssetId crsc;
    CAssetId pirate_projectile_effect;
    int always_ff;
    int sound_0x23316032;
    int sound_0xa3b39766;
    int lock_on_sound;
    int gun_pan_sound;
    int sound_0xf57880ec;
    int sound_0x99fe97f6;
    int sound_0xa2714856;
    int sound_0xd58a2fa7;
    int sound_0xb381355a;
    int sound_0x00628c84;
    int sound_0x40533b8d;
    int sound_0x613cafd8;
    int pole_sparks_sound;
    float max_audible_distance;
    float unknown_0xd2986c43;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
};

void LoadTypedefSLdrGunTurretBase(SLdrGunTurretBase&, CInputStream&);

#endif // _SLDRGUNTURRETBASE
