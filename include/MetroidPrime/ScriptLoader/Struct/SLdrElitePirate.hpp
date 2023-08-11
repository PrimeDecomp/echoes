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

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrDamageInfo meleeDamage;
    float maxMeleeRange;
    float minShockwaveRange;
    float maxShockwaveRange;
    float minRocketRange;
    float maxRocketRange;
    float unknown_0x5236c2b6;
    float unknown_0x01eaab17;
    CAssetId shieldedModel;
    CAssetId shieldedSkinRules;
    CAssetId darkShield;
    int darkShieldSound;
    CAssetId darkShieldPop;
    CAssetId lightShield;
    int lightShieldSound;
    CAssetId lightShieldPop;
    float tauntInterval;
    float tauntVariance;
    SLdrShockWaveInfo singleShockWaveInfo;
    SLdrShockWaveInfo doubleShockWaveInfo;
    float unknown_0x28b39197;
    float unknown_0xe27de71b;
    float unknown_0x665e7ace;
    float unknown_0xacd4d06d;
    CAssetId rocket;
    SLdrDamageInfo rocketDamage;
    int unknown_0x624222f8;
    int unknown_0x31e43a1c;
    float repeatedAttackChance;
    float energyAbsorbDuration;
    float unknown_0xe47334ae;
    float unknown_0x3dad897b;
    int alwaysFf_0x06cf4324;
    int alwaysFf_0x23f5e1ee;
    SLdrActorParameters rocketLauncherActorInfo;
    SLdrAnimationParameters rocketLauncherAnimInfo;
    SLdrAnimationParameters unknown_0x7e6e0d38;
    CAssetId visorElectricEffect;
    int soundVisorElectric;
    SLdrIngPossessionData ingPossessionData;
};

void LoadTypedefSLdrElitePirate(SLdrElitePirate&, CInputStream&);

#endif // _SLDRELITEPIRATE
