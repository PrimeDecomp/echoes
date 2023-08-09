#ifndef _SLDRMETROIDALPHA
#define _SLDRMETROIDALPHA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrMetroidAlpha {
    SLdrMetroidAlpha();
    ~SLdrMetroidAlpha();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrDamageVulnerability frozen_vulnerability;
    SLdrDamageVulnerability energy_drain_vulnerability;
    SLdrDamageVulnerability damage_vulnerability;
    float unknown_0x72439b39;
    float unknown_0x3af75fcc;
    float telegraph_attack_time;
    float baby_metroid_scale;
    float unknown_0x03362858;
    float unknown_0x852d3bb0;
    float unknown_0x1c783744;
    CAssetId part;
    float unknown_0x2fe164c4;
    float unknown_0x4c1f78f3;
    float unknown_0x5f3f294c;
    float unknown_0x06b460f4;
    float chance_to_dodge;
    int unknown_0xfa51d735;
    SLdrIngPossessionData ing_possession_data;
};

void LoadTypedefSLdrMetroidAlpha(SLdrMetroidAlpha&, CInputStream&);

#endif // _SLDRMETROIDALPHA
