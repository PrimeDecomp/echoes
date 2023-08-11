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

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    CAssetId beamEffect;
    CAssetId beam;
    SLdrDamageInfo beamDamage;
    CAssetId contactFx;
    float beamFadeTime;
    float beamRadius;
    float hoverSpeed;
    SLdrDamageVulnerability frozenVulnerability;
    float normalRotateSpeed;
    float chargingRotateSpeed;
    float speedChangeRate;
    int sound_0x14038b71;
    int sound_0x16a435cb;
    int sound_0x67edefc2;
    float damageDelay;
};

void LoadTypedefSLdrAtomicBeta(SLdrAtomicBeta&, CInputStream&);

#endif // _SLDRATOMICBETA
