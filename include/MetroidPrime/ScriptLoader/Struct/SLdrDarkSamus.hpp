#ifndef _SLDRDARKSAMUS
#define _SLDRDARKSAMUS

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrDarkSamus {
    SLdrDarkSamus();
    ~SLdrDarkSamus();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    float unknown_0x72edeb7d;
    float unknown_0x74fa22f0;
    int glideSound;
    int missileRicochetSound;
    bool unknown_0x6689925b;
    CAssetId txtr_0x3863160b;
    float unknown_0x2c6a3344;
    SLdrDamageInfo meleeAttackDamage;
    CAssetId part_0x6d40aa56;
    CAssetId part_0x9603a544;
    SLdrDamageInfo diveAttackDamage;
    float unknown_0x4aa5dd62;
    float unknown_0x1ad7dc21;
    CAssetId diveAttackEffect;
    CAssetId scatterShotProjectile;
    CAssetId scatterShotProjectile2;
    SLdrDamageInfo scatterShotDamage;
    float unknown_0x378285b9;
    float unknown_0xd242e25f;
    CAssetId normalMissileProjectile;
    SLdrDamageInfo normalMissileDamage;
    int unknown_0x1f1ef7a9;
    int unknown_0xc4a1a44e;
    CAssetId superMissileProjectile;
    SLdrDamageInfo superMissileDamage;
    CAssetId freezeBeamProjectile;
    SLdrDamageInfo freezeBeamDamage;
    CAssetId txtr_0x7ffeb33d;
    float damageInterruptThreshold;
    float unknown_0xf317f4d5;
    CAssetId sweepSwoosh;
    SLdrDamageInfo sweepBeamDamage;
    CAssetId crsc;
    int sweepBeamSound;
    int unknown_0x0ef8dc15;
    CAssetId invulnerableModel;
    CAssetId invulnerableSkinRules;
    SLdrAnimationParameters boostBallModel;
    SLdrDamageInfo boostBallDamage;
    CAssetId boostBallGlow;
    CAssetId swhc_0x449aa4aa;
    CAssetId swhc_0x0345fa17;
    int sound_0x2c72576b;
    int boostBallHitPlayerSound;
    CAssetId boostBallCollision;
    SLdrAudioPlaybackParms audioPlaybackParms;
    CAssetId part_0xa6c42023;
    int iceSpreadSound;
    CAssetId part_0x908b06e9;
    CAssetId part_0x494de4a4;
    int sound_0xa861649f;
    SLdrDamageInfo damageInfo_0x18402aa9;
    CAssetId part_0xe701daea;
    CAssetId phazonProjectile;
    CAssetId wpsc;
    SLdrDamageInfo damageInfo_0x58769eb2;
    SLdrDamageInfo phazonProjectileDamage;
    CAssetId phazonEnrageSphere;
    SLdrDamageInfo damageInfo_0x8f3af226;
    CAssetId alternateScannableInfo;
};

void LoadTypedefSLdrDarkSamus(SLdrDarkSamus&, CInputStream&);

#endif // _SLDRDARKSAMUS
