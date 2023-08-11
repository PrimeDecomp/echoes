#ifndef _SLDRSPORBBASE
#define _SLDRSPORBBASE

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPowerBombGuardianStageProperties.hpp"

struct SLdrSporbBase {
    SLdrSporbBase();
    ~SLdrSporbBase();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    float unknown_0x95e7a2c2;
    float unknown_0x76ba1c18;
    float unknown_0x3eb2de35;
    float unknown_0xe50d8dd2;
    int unknown_0x64d482d5;
    int unknown_0xc3e002ac;
    float shotAngleVariance;
    CVector3f attackAimOffset;
    CAssetId tendrilParticleEffect;
    float unknown_0x35557a83;
    float grabberOutAcceleration;
    float grabberInAcceleration;
    float unknown_0xbfddabd4;
    float unknown_0x62bfaa35;
    float grabberAttachTime;
    float unknown_0xed82c56a;
    float unknown_0xe918f440;
    float spitForce;
    float spitDamage;
    float grabDamage;
    float unknown_0x2cfade2c;
    float unknown_0xb68e75cc;
    float unknown_0x6d31262b;
    bool isPowerBombGuardian;
    CAssetId wpsc;
    SLdrDamageInfo powerBombProjectileDamage;
    float unknown_0x03a76d35;
    float unknown_0x6d4e0f5a;
    float unknown_0x3538d49b;
    float unknown_0xe89c7707;
    float unknown_0x738d1f51;
    int sound_0x9480c6d7;
    float unknown_0x48df4182;
    float unknown_0xe39482ad;
    float unknown_0xdd8502cc;
    float unknown_0x4ab8cf7d;
    float unknown_0xf5e28404;
    int grabberFireSound;
    int grabberFlightSound;
    int grabberHitPlayerSound;
    int grabberHitWorldSound;
    int grabberRetractSound;
    int sound_0x64e9152d;
    int morphballSpitSound;
    int grabberExplosionSound;
    int ballEscapeSound;
    int needleTelegraphSound;
    int grabberTelegraphSound;
    SLdrPowerBombGuardianStageProperties powerBombGuardianStageProperties_0x510dba97;
    SLdrPowerBombGuardianStageProperties powerBombGuardianStageProperties_0x0b6c85f7;
    SLdrPowerBombGuardianStageProperties powerBombGuardianStageProperties_0x8b9c92e8;
    SLdrPowerBombGuardianStageProperties powerBombGuardianStageProperties_0xbfaefb37;
};

void LoadTypedefSLdrSporbBase(SLdrSporbBase&, CInputStream&);

#endif // _SLDRSPORBBASE
