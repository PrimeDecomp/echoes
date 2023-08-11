#ifndef _SLDRPATTERNEDAITYPEDEF
#define _SLDRPATTERNEDAITYPEDEF

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrPatternedAITypedef {
    SLdrPatternedAITypedef();
    ~SLdrPatternedAITypedef();

    float mass;
    float speed;
    float turnSpeed;
    float detectionRange;
    float detectionHeightRange;
    float detectionAngle;
    float minAttackRange;
    float maxAttackRange;
    float averageAttackTime;
    float attackTimeVariation;
    float leashRadius;
    float playerLeashRadius;
    float playerLeashTime;
    SLdrDamageInfo contactDamage;
    float damageWaitTime;
    SLdrHealthInfo health;
    SLdrDamageVulnerability vulnerability;
    float collisionRadius;
    float collisionHeight;
    CVector3f collisionOffset;
    float stepUpHeight;
    float unknown_0xe287d8dd;
    float unknown_0x66cdc6e8;
    float xDamageDelay;
    int soundXDamage;
    SLdrAnimationParameters animationInformation;
    CAssetId stateMachine;
    CAssetId stateMachine2;
    float unknown_0x87d22d43;
    float unknown_0xf0790c1b;
    float freezeDuration;
    int pathMeshIndex;
    CVector3f gibParticlesOffset;
    CAssetId gibParticles;
    CAssetId unknown_0xf35f5164;
    CVector3f iceGibParticlesOffset;
    CAssetId iceGibParticles;
    int sound_0x7344d6cd;
    int sound_0x562cf323;
    int soundFrozen;
    CAssetId knockbackRules;
    int creatureSize;
    SLdrEchoParameters echoParameters;
};

void LoadTypedefSLdrPatternedAITypedef(SLdrPatternedAITypedef&, CInputStream&);

#endif // _SLDRPATTERNEDAITYPEDEF
