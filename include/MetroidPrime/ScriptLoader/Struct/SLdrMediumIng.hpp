#ifndef _SLDRMEDIUMING
#define _SLDRMEDIUMING

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrCameraShakerData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrMediumIng {
    SLdrMediumIng();
    ~SLdrMediumIng();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    int spawn_mode;
    SLdrActorParameters actor_information;
    float aggressiveness;
    float unknown_0x4d1d840d;
    float min_melee_attack_interval;
    float max_melee_attack_range;
    SLdrDamageInfo melee_damage;
    float unknown_0x636f11e5;
    SLdrDamageInfo mist_damage;
    float min_mist_attack_interval;
    SLdrDamageVulnerability misting_vulnerability;
    float min_arm_attack_interval;
    float unknown_0x9d3cfeb0;
    float unknown_0xdc2bc136;
    float min_tentacle_length;
    float max_tentacle_length;
    float arm_attack_time;
    float unknown_0x8f1d597c;
    SLdrAnimationParameters attack_tentacle;
    SLdrActorParameters actor_parameters;
    spline attack_motion;
    SLdrCameraShakerData camera_shaker_data;
    SLdrDamageInfo attack_tentacle_damage;
    float taunt_chance;
    float double_dash_chance;
    CColor light_color;
    float light_attenuation;
    spline unknown_0xb459c3e9;
    spline dash_speed;
    CAssetId ing_spot_blob_fx;
    int ing_spot_sound;
    float unknown_0x0e3d3708;
};

void LoadTypedefSLdrMediumIng(SLdrMediumIng&, CInputStream&);

#endif // _SLDRMEDIUMING
