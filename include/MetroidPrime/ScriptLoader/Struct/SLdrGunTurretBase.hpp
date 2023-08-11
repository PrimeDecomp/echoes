#ifndef _SLDRGUNTURRETBASE
#define _SLDRGUNTURRETBASE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrGunTurretBase {
    SLdrGunTurretBase();
    ~SLdrGunTurretBase();

    SLdrEditorProperties editorProperties;
    SLdrDamageInfo attackDamage;
    float hurtSleepDelay;
    float gunAimTurnSpeed;
    float unknown_0xc80bc7c5;
    float unknown_0x95e7a2c2;
    float unknown_0x76ba1c18;
    float unknown_0x3eb2de35;
    float unknown_0xe50d8dd2;
    int unknown_0x64d482d5;
    int unknown_0xc3e002ac;
    float unknown_0x5ade66a9;
    float unknown_0x8dd2c329;
    float unknown_0xfc036e93;
    float shotAngleVariance;
    float patrolDelay;
    float withdrawDelay;
    float unknown_0x8a35b1ea;
    float unknown_0xd49bec5a;
    float unknown_0x80ce481a;
    float attackDelay;
    float detectionHeightUp;
    float detectionHeightDown;
    float attackLeashTime;
    bool gunRespawns;
    bool unknown_0x5cf12e9a;
    bool unknown_0x479d8dc4;
    bool isPirateTurret;
    CAssetId crsc;
    CAssetId pirateProjectileEffect;
    int alwaysFf;
    int sound_0x23316032;
    int sound_0xa3b39766;
    int lockOnSound;
    int gunPanSound;
    int sound_0xf57880ec;
    int sound_0x99fe97f6;
    int sound_0xa2714856;
    int sound_0xd58a2fa7;
    int sound_0xb381355a;
    int sound_0x00628c84;
    int sound_0x40533b8d;
    int sound_0x613cafd8;
    int poleSparksSound;
    float maxAudibleDistance;
    float unknown_0xd2986c43;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
};

void LoadTypedefSLdrGunTurretBase(SLdrGunTurretBase&, CInputStream&);

#endif // _SLDRGUNTURRETBASE
