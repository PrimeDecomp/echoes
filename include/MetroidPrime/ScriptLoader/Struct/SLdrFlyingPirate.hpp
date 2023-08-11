#ifndef _SLDRFLYINGPIRATE
#define _SLDRFLYINGPIRATE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrFlyingPirate {
    SLdrFlyingPirate();
    ~SLdrFlyingPirate();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    float searchRadius;
    float hearingRadius;
    int unknown_0x20daf45e;
    CAssetId projectile;
    SLdrDamageInfo projectileDamage;
    int soundProjectile;
    CAssetId missile;
    SLdrDamageInfo missileDamage;
    CAssetId wpsc;
    float hurlRecoverTime;
    float hoverHeight;
    CAssetId part_0x6475fc6f;
    SLdrDamageInfo rocketPackExplosionDamage;
    float spiralChance;
    float minimumMissileTime;
    float unknown_0xb9bb2f64;
    float flightThrust;
    int soundImpact;
    int soundSpiral;
    float landChance;
    float unknown_0x71587b45;
    float unknown_0x7903312e;
    CAssetId part_0x317212ab;
    CAssetId part_0xbc113d7b;
    CAssetId part_0x738bbbaa;
    int soundHurled;
    int soundDeath;
    float doubleAttackChance;
    float unknown_0x3427d27f;
    float stopHomingRange;
    float unknown_0xccf05648;
    float unknown_0x2a90f9a9;
    float unknown_0x9ca8f357;
    float unknown_0x7ac85cb6;
};

void LoadTypedefSLdrFlyingPirate(SLdrFlyingPirate&, CInputStream&);

#endif // _SLDRFLYINGPIRATE
