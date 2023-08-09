#ifndef _SLDRELITEPIRATE
#define _SLDRELITEPIRATE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrShockWaveInfo.hpp"

struct SLdrElitePirate {
    SLdrElitePirate();
    ~SLdrElitePirate();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrDamageInfo melee_damage;
    float max_melee_range;
    float min_shockwave_range;
    float max_shockwave_range;
    float min_rocket_range;
    float max_rocket_range;
    float unknown_0x5236c2b6;
    float unknown_0x01eaab17;
    CAssetId shielded_model;
    CAssetId shielded_skin_rules;
    CAssetId dark_shield;
    int dark_shield_sound;
    CAssetId dark_shield_pop;
    CAssetId light_shield;
    int light_shield_sound;
    CAssetId light_shield_pop;
    float taunt_interval;
    float taunt_variance;
    SLdrShockWaveInfo single_shock_wave_info;
    SLdrShockWaveInfo double_shock_wave_info;
    float unknown_0x28b39197;
    float unknown_0xe27de71b;
    float unknown_0x665e7ace;
    float unknown_0xacd4d06d;
    CAssetId rocket;
    SLdrDamageInfo rocket_damage;
    int unknown_0x624222f8;
    int unknown_0x31e43a1c;
    float repeated_attack_chance;
    float energy_absorb_duration;
    float unknown_0xe47334ae;
    float unknown_0x3dad897b;
    int always_ff_0x06cf4324;
    int always_ff_0x23f5e1ee;
    SLdrActorParameters rocket_launcher_actor_info;
    SLdrAnimationParameters rocket_launcher_anim_info;
    SLdrAnimationParameters unknown_0x7e6e0d38;
    CAssetId visor_electric_effect;
    int sound_visor_electric;
    SLdrIngPossessionData ing_possession_data;
};

void LoadTypedefSLdrElitePirate(SLdrElitePirate&, CInputStream&);

#endif // _SLDRELITEPIRATE
