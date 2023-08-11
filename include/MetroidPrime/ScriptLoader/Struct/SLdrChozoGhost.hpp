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

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    float hearingRadius;
    float fadeOutDelay;
    float attackDelay;
    float freezeTime;
    CAssetId unknown_0x54151870;
    SLdrDamageInfo damageInfo_0xffcda1f8;
    CAssetId unknown_0x3a58089c;
    SLdrDamageInfo damageInfo_0x1ff047a9;
    SLdrBehaveChance behaveChance_0xe832241f;
    SLdrBehaveChance behaveChance_0x1e2c8483;
    SLdrBehaveChance behaveChance_0x78d76034;
    int soundImpact;
    float unknown_0xc87d7ec7;
    int rightDisappearCrossfade;
    int sound;
    int unknown_0xec76940c;
    float unknown_0x723542bb;
    int unknown_0xfe9eac26;
    float hurlRecoverTime;
    CAssetId projectileVisorEffect;
    int soundProjectileVisor;
    float unknown_0x61e511b3;
    float unknown_0x2369607a;
    int nearChance;
    int midChance;
};

void LoadTypedefSLdrChozoGhost(SLdrChozoGhost&, CInputStream&);

#endif // _SLDRCHOZOGHOST
