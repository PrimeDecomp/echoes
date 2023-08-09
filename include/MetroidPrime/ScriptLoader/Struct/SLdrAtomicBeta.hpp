#ifndef _SLDRATOMICBETA
#define _SLDRATOMICBETA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrAtomicBeta {
    SLdrAtomicBeta();
    ~SLdrAtomicBeta();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    CAssetId beam_effect;
    CAssetId beam;
    SLdrDamageInfo beam_damage;
    CAssetId contact_fx;
    float beam_fade_time;
    float beam_radius;
    float hover_speed;
    SLdrDamageVulnerability frozen_vulnerability;
    float normal_rotate_speed;
    float charging_rotate_speed;
    float speed_change_rate;
    int sound_0x14038b71;
    int sound_0x16a435cb;
    int sound_0x67edefc2;
    float damage_delay;
};

void LoadTypedefSLdrAtomicBeta(SLdrAtomicBeta&, CInputStream&);

#endif // _SLDRATOMICBETA
