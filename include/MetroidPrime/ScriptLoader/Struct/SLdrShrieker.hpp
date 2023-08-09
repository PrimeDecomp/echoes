#ifndef _SLDRSHRIEKER
#define _SLDRSHRIEKER

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrShrieker {
    SLdrShrieker();
    ~SLdrShrieker();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrDamageVulnerability buried_vulnerability;
    float hostile_accumulate_priority;
    SLdrDamageInfo damage_info;
    CAssetId part;
    SLdrDamageInfo projectile_damage;
    CAssetId projectile;
    int combat_visor_max_volume;
    int echo_visor_max_volume;
    SLdrDamageInfo melee_damage;
    CAssetId melee_effect;
    float unknown_0x9b6a4437;
    float melee_attack_time_variation;
    float melee_range;
    float hover_height;
    CVector3f missile_deflection_offset;
    float missile_deflection_radius;
    float unknown_0xe70ef8a3;
    int sound_missile_deflection;
    float dodge_time;
    float dodge_percentage;
    float detection_height;
    float unknown_0x4753beb1;
    float pop_detection_radius;
    float morphball_detection_radius;
    float visibility_change_time;
};

void LoadTypedefSLdrShrieker(SLdrShrieker&, CInputStream&);

#endif // _SLDRSHRIEKER
