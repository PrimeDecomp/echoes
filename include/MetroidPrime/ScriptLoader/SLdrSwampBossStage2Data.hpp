#ifndef _SLDRSWAMPBOSSSTAGE2DATA
#define _SLDRSWAMPBOSSSTAGE2DATA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrShockWaveInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSwampBossStage2Struct.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct38.hpp"

struct SLdrSwampBossStage2Data {
    SLdrSwampBossStage2Data();
    ~SLdrSwampBossStage2Data();

    float hoverSpeed;
    CAssetId upperLeftWingTarget;
    CAssetId lowerLeftWingTarget;
    CAssetId upperRightWingTarget;
    CAssetId lowerRightWingTarget;
    float unknown_0xcabe6b96;
    SLdrSwampBossStage2Struct swampBossStage2Struct_0x7fa9256a;
    SLdrSwampBossStage2Struct swampBossStage2Struct_0x8b884b8e;
    SLdrSwampBossStage2Struct swampBossStage2Struct_0x04b7a789;
    SLdrSwampBossStage2Struct swampBossStage2Struct_0xf096c96d;
    float stunTime;
    int unknown_0x96ce7897;
    CAssetId spitProjectile;
    SLdrDamageInfo spitDamage;
    CAssetId spitVisorEffect;
    int soundSpitVisor;
    float spitProjectileRadius;
    SLdrDamageInfo swoopDamage;
    float swoopPush;
    float swoopDamageTime;
    CAssetId splash;
    float unknown_0x7fc50ac2;
    float unknown_0x13448a4a;
    float unknown_0xf55924da;
    float unknown_0x83bc1de7;
    float unknown_0x5a844633;
    float unknown_0x78e22d1b;
    float unknown_0x9e116385;
    float radarRange;
    float unknown_0xfe97e835;
    SLdrShockWaveInfo splashShockWave;
    float unknown_0x9807497c;
    float unknown_0xe57ca27c;
    CAssetId scanInfoLight;
    CAssetId scanInfoDark;
    CAssetId bubbleTelegraphEffect;
    CAssetId wingDamageEffect;
    SLdrUnknownStruct38 unknownStruct38;
    CAssetId blowEffect;
    SLdrDamageInfo blowDamage;
    float unknown_0x6d89649b;
    float blowPush;
    float breakStunDamage;
    SLdrAudioPlaybackParms stunnedSound;
    SLdrAudioPlaybackParms audioPlaybackParms_0x427a116a;
    SLdrAudioPlaybackParms audioPlaybackParms_0xc05d5c7a;
    SLdrAudioPlaybackParms audioPlaybackParms_0x2b3c923a;
    float unknown_0x8fe0bf01;
    SLdrAudioPlaybackParms flinchSound;
    float flinchSoundChance;
    SLdrAudioPlaybackParms stunnedFlinchSound;
    SLdrAudioPlaybackParms audioPlaybackParms_0x878a6522;
    float unknown_0x19849710;
    SLdrAudioPlaybackParms audioPlaybackParms_0x692fa63c;
    SLdrAudioPlaybackParms audioPlaybackParms_0xbe3d39aa;
};

void LoadTypedefSLdrSwampBossStage2Data(SLdrSwampBossStage2Data&, CInputStream&);

#endif // _SLDRSWAMPBOSSSTAGE2DATA
