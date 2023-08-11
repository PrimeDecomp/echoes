#ifndef _SLDRUNKNOWNSTRUCT37
#define _SLDRUNKNOWNSTRUCT37

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrShockWaveInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSwampBossStage1Struct.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct36.hpp"

struct SLdrUnknownStruct37 {
    SLdrUnknownStruct37();
    ~SLdrUnknownStruct37();

    CAssetId darkWaterRingEffect;
    float unknown_0x27a06f6a;
    float unknown_0x233a5e40;
    CAssetId preJumpTelegraphEffect;
    SLdrShockWaveInfo splashShockWave;
    CAssetId tongueParticleEffect;
    CAssetId tongueParticleModel;
    CAssetId tongueTipModel;
    SLdrDamageInfo damageInfo;
    CAssetId part;
    float unknown_0x78755da3;
    float unknown_0x74e1a041;
    float unknown_0x1f4e7c2c;
    float unknown_0xee6b6f47;
    float unknown_0x3ce96c9d;
    SLdrDamageVulnerability weakSpotVulnerability;
    float weakSpotDamageMultiplier;
    CAssetId spitProjectile;
    SLdrDamageInfo spitDamage;
    CAssetId spitVisorEffect;
    int soundSpitVisor;
    float spitProjectileRadius;
    SLdrUnknownStruct36 unknownStruct36;
    SLdrSwampBossStage1Struct swampBossStage1Struct_0x4500f774;
    SLdrSwampBossStage1Struct swampBossStage1Struct_0x3e1e7597;
    SLdrSwampBossStage1Struct swampBossStage1Struct_0xa1c4f609;
};

void LoadTypedefSLdrUnknownStruct37(SLdrUnknownStruct37&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT37
