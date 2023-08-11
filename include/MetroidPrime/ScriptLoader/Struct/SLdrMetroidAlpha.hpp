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

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrDamageVulnerability frozenVulnerability;
    SLdrDamageVulnerability energyDrainVulnerability;
    SLdrDamageVulnerability damageVulnerability;
    float unknown_0x72439b39;
    float unknown_0x3af75fcc;
    float telegraphAttackTime;
    float babyMetroidScale;
    float unknown_0x03362858;
    float unknown_0x852d3bb0;
    float unknown_0x1c783744;
    CAssetId part;
    float unknown_0x2fe164c4;
    float unknown_0x4c1f78f3;
    float unknown_0x5f3f294c;
    float unknown_0x06b460f4;
    float chanceToDodge;
    int unknown_0xfa51d735;
    SLdrIngPossessionData ingPossessionData;
};

void LoadTypedefSLdrMetroidAlpha(SLdrMetroidAlpha&, CInputStream&);

#endif // _SLDRMETROIDALPHA
