#ifndef _SLDRKRALEE
#define _SLDRKRALEE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrKralee {
    SLdrKralee();
    ~SLdrKralee();

    SLdrEditorProperties editorProperties;
    int flavor;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    float waypointApproachDistance;
    float visibleDistance;
    float wallTurnSpeed;
    float floorTurnSpeed;
    float downTurnSpeed;
    float unknown_0xd5c25506;
    float projectileBoundsMultiplier;
    float collisionLookAhead;
    float warpInTime;
    float warpOutTime;
    float visibleTime;
    float unknown_0x7bba36ff;
    float invisibleTime;
    float unknown_0x4e4ae0e4;
    float warpAttackRadius;
    float warpAttackKnockback;
    float warpAttackDamage;
    float animSpeedScalar;
    float maxAudibleDistance;
    CAssetId warpInParticleEffect;
    CAssetId warpOutParticleEffect;
    int warpInSound;
    int warpOutSound;
    bool initiallyPaused;
    bool initiallyInvisible;
};

void LoadTypedefSLdrKralee(SLdrKralee&, CInputStream&);

#endif // _SLDRKRALEE
