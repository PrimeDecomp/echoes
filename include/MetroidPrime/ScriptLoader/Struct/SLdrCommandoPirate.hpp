#ifndef _SLDRCOMMANDOPIRATE
#define _SLDRCOMMANDOPIRATE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct10.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct11.hpp"

struct SLdrCommandoPirate {
    SLdrCommandoPirate();
    ~SLdrCommandoPirate();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrIngPossessionData ingPossessionData;
    int sound;
    float aggressiveness;
    float coverCheck;
    float searchRadius;
    float dodgeCheck;
    int soundImpact;
    int soundHurled;
    int soundDeath;
    int alwaysFf_0xfca76593;
    int alwaysFf_0x467c3d94;
    SLdrDamageInfo bladeDamage;
    CAssetId projectile;
    SLdrDamageInfo projectileDamage;
    int soundProjectile;
    float hearingRadius;
    SLdrUnknownStruct10 unknownStruct10;
    SLdrUnknownStruct11 unknownStruct11;
    float unknown_0x71587b45;
    float unknown_0x7903312e;
};

void LoadTypedefSLdrCommandoPirate(SLdrCommandoPirate&, CInputStream&);

#endif // _SLDRCOMMANDOPIRATE
