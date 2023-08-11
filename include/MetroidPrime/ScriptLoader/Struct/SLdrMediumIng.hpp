#ifndef _SLDRMEDIUMING
#define _SLDRMEDIUMING

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrCameraShakerData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrMediumIng {
    SLdrMediumIng();
    ~SLdrMediumIng();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    int spawnMode;
    SLdrActorParameters actorInformation;
    float aggressiveness;
    float unknown_0x4d1d840d;
    float minMeleeAttackInterval;
    float maxMeleeAttackRange;
    SLdrDamageInfo meleeDamage;
    float unknown_0x636f11e5;
    SLdrDamageInfo mistDamage;
    float minMistAttackInterval;
    SLdrDamageVulnerability mistingVulnerability;
    float minArmAttackInterval;
    float unknown_0x9d3cfeb0;
    float unknown_0xdc2bc136;
    float minTentacleLength;
    float maxTentacleLength;
    float armAttackTime;
    float unknown_0x8f1d597c;
    SLdrAnimationParameters attackTentacle;
    SLdrActorParameters actorParameters;
    spline attackMotion;
    SLdrCameraShakerData cameraShakerData;
    SLdrDamageInfo attackTentacleDamage;
    float tauntChance;
    float doubleDashChance;
    CColor lightColor;
    float lightAttenuation;
    spline unknown_0xb459c3e9;
    spline dashSpeed;
    CAssetId ingSpotBlobFx;
    int ingSpotSound;
    float unknown_0x0e3d3708;
};

void LoadTypedefSLdrMediumIng(SLdrMediumIng&, CInputStream&);

#endif // _SLDRMEDIUMING
