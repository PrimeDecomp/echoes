#ifndef _SLDRCHOZOGHOST
#define _SLDRCHOZOGHOST

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrBehaveChance.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrChozoGhost {
    SLdrChozoGhost();
    ~SLdrChozoGhost();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    float hearing_radius;
    float fade_out_delay;
    float attack_delay;
    float freeze_time;
    CAssetId unknown_0x54151870;
    SLdrDamageInfo damage_info_0xffcda1f8;
    CAssetId unknown_0x3a58089c;
    SLdrDamageInfo damage_info_0x1ff047a9;
    SLdrBehaveChance behave_chance_0xe832241f;
    SLdrBehaveChance behave_chance_0x1e2c8483;
    SLdrBehaveChance behave_chance_0x78d76034;
    int sound_impact;
    float unknown_0xc87d7ec7;
    int right_disappear_crossfade;
    int sound;
    int unknown_0xec76940c;
    float unknown_0x723542bb;
    int unknown_0xfe9eac26;
    float hurl_recover_time;
    CAssetId projectile_visor_effect;
    int sound_projectile_visor;
    float unknown_0x61e511b3;
    float unknown_0x2369607a;
    int near_chance;
    int mid_chance;
};

void LoadTypedefSLdrChozoGhost(SLdrChozoGhost&, CInputStream&);

#endif // _SLDRCHOZOGHOST
