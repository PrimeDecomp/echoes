#ifndef _SLDRSHRIEKER
#define _SLDRSHRIEKER

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrShrieker {
    SLdrShrieker();
    ~SLdrShrieker();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrDamageVulnerability buriedVulnerability;
    float hostileAccumulatePriority;
    SLdrDamageInfo damageInfo;
    CAssetId part;
    SLdrDamageInfo projectileDamage;
    CAssetId projectile;
    int combatVisorMaxVolume;
    int echoVisorMaxVolume;
    SLdrDamageInfo meleeDamage;
    CAssetId meleeEffect;
    float unknown_0x9b6a4437;
    float meleeAttackTimeVariation;
    float meleeRange;
    float hoverHeight;
    CVector3f missileDeflectionOffset;
    float missileDeflectionRadius;
    float unknown_0xe70ef8a3;
    int soundMissileDeflection;
    float dodgeTime;
    float dodgePercentage;
    float detectionHeight;
    float unknown_0x4753beb1;
    float popDetectionRadius;
    float morphballDetectionRadius;
    float visibilityChangeTime;
};

void LoadTypedefSLdrShrieker(SLdrShrieker&, CInputStream&);

#endif // _SLDRSHRIEKER
