#ifndef _SLDRBASICSWARMPROPERTIES
#define _SLDRBASICSWARMPROPERTIES

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrBasicSwarmProperties {
    SLdrBasicSwarmProperties();
    ~SLdrBasicSwarmProperties();

    SLdrDamageInfo contact_damage;
    float damage_wait_time;
    float collision_radius;
    float touch_radius;
    float damage_radius;
    float speed;
    int count;
    int max_count;
    float influence_radius;
    float unknown_0x61959f0d;
    float alignment_priority;
    float separation_priority;
    float path_following_priority;
    float player_attract_priority;
    float player_attract_distance;
    float spawn_speed;
    int attacker_count;
    float attack_proximity;
    float attack_timer;
    SLdrHealthInfo health;
    SLdrDamageVulnerability damage_vulnerability;
    CAssetId death_particle_effect;
    int unknown_0x84f81f55;
    float unknown_0xe1bd61d8;
    float turn_rate;
    bool unknown_0xc2208b0f;
    bool unknown_0x7eb5d9e8;
    bool is_orbitable;
    bool unknown_0xbc01a28e;
    int locomotion_looped_sound;
    int attack_looped_sound;
    float unknown_0xd2986c43;
    float max_audible_distance;
    int min_volume;
    int max_volume;
    float freeze_duration;
    float life_time;
};

void LoadTypedefSLdrBasicSwarmProperties(SLdrBasicSwarmProperties&, CInputStream&);

#endif // _SLDRBASICSWARMPROPERTIES
