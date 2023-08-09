#ifndef _SLDRPATTERNEDAITYPEDEF
#define _SLDRPATTERNEDAITYPEDEF

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrPatternedAITypedef {
    SLdrPatternedAITypedef();
    ~SLdrPatternedAITypedef();

    float mass;
    float speed;
    float turn_speed;
    float detection_range;
    float detection_height_range;
    float detection_angle;
    float min_attack_range;
    float max_attack_range;
    float average_attack_time;
    float attack_time_variation;
    float leash_radius;
    float player_leash_radius;
    float player_leash_time;
    SLdrDamageInfo contact_damage;
    float damage_wait_time;
    SLdrHealthInfo health;
    SLdrDamageVulnerability vulnerability;
    float collision_radius;
    float collision_height;
    CVector3f collision_offset;
    float step_up_height;
    float unknown_0xe287d8dd;
    float unknown_0x66cdc6e8;
    float x_damage_delay;
    int sound_x_damage;
    SLdrAnimationParameters animation_information;
    CAssetId state_machine;
    CAssetId state_machine2;
    float unknown_0x87d22d43;
    float unknown_0xf0790c1b;
    float freeze_duration;
    int path_mesh_index;
    CVector3f gib_particles_offset;
    CAssetId gib_particles;
    CAssetId unknown_0xf35f5164;
    CVector3f ice_gib_particles_offset;
    CAssetId ice_gib_particles;
    int sound_0x7344d6cd;
    int sound_0x562cf323;
    int sound_frozen;
    CAssetId knockback_rules;
    int creature_size;
    SLdrEchoParameters echo_parameters;
};

void LoadTypedefSLdrPatternedAITypedef(SLdrPatternedAITypedef&, CInputStream&);

#endif // _SLDRPATTERNEDAITYPEDEF
