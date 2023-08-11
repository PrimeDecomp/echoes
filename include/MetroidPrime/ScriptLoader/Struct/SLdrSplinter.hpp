#ifndef _SLDRSPLINTER
#define _SLDRSPLINTER

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrSplinter {
    SLdrSplinter();
    ~SLdrSplinter();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    float unknown_0x72edeb7d;
    float unknown_0xb8ed9ffa;
    float unknown_0x5e8d301b;
    float unknown_0xb98bb88f;
    float unknown_0x5feb176e;
    int unknown_0x726cd31d;
    int unknown_0x376e909f;
    SLdrDamageInfo attackDamage;
    int unknown_0xb63b810c;
    SLdrAnimationParameters unknown_0x6d752efc;
    SLdrAnimationParameters unknown_0x0d6ab7b5;
    CAssetId part_0x630d93a1;
    SLdrDamageInfo damageInfo_0x4436a388;
    SLdrIngPossessionData ingPossessionData;
    bool isMegaSplinter;
    CAssetId wpsc;
    SLdrDamageInfo damageInfo_0x02fd0913;
    CAssetId part_0x496f191b;
    float unknown_0x51be00d3;
    float unknown_0xb7deaf32;
};

void LoadTypedefSLdrSplinter(SLdrSplinter&, CInputStream&);

#endif // _SLDRSPLINTER
