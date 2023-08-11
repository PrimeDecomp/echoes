#ifndef _SLDRGRENCHLER
#define _SLDRGRENCHLER

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrGrenchler {
    SLdrGrenchler();
    ~SLdrGrenchler();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    float unknown_0x04d51e3a;
    bool isGrappleGuardian;
    bool hasHealthBar;
    SLdrDamageVulnerability damageVulnerability;
    SLdrAnimationParameters tail;
    SLdrAnimationParameters unknown_0x0abef809;
    CAssetId taillessModel;
    CAssetId taillessSkinRules;
    SLdrAnimationParameters unknown_0x9b193ae8;
    SLdrAnimationParameters unknown_0xc24cf580;
    CAssetId cmdl;
    CAssetId cskr;
    int tailHitSound;
    int tailDestroyedSound;
    float unknown_0x5d8f2bee;
    float unknown_0x7bd1a35f;
    float unknown_0xea4b88c8;
    float unknown_0xaa04f0be;
    float unknown_0x98d5d373;
    float unknown_0xd89aab05;
    float unknown_0x2e6096ee;
    float unknown_0x6e2fee98;
    float unknown_0x49632b31;
    SLdrDamageInfo biteDamage;
    float unknown_0x262b2508;
    float unknown_0x66645d7e;
    float unknown_0x909e6095;
    float unknown_0xd0d118e3;
    CAssetId electricEffect;
    SLdrDamageInfo beamDamage;
    float unknown_0x680ce795;
    SLdrAudioPlaybackParms audioPlaybackParms_0xad47febe;
    float unknown_0x06f7ceed;
    float unknown_0x46b8b69b;
    float unknown_0xb0428b70;
    float unknown_0xf00df306;
    float unknown_0xb00775e6;
    CAssetId burstProjectile;
    SLdrDamageInfo burstDamage;
    CAssetId surfaceRingsEffect;
    CAssetId part_0xbf4daae6;
    CAssetId partAlwaysFf;
    CAssetId part_0xffcee1a9;
    CAssetId grappleSwoosh;
    CAssetId grappleBeamPart;
    CAssetId grappleHitFx;
    SLdrDamageInfo grappleDamage;
    SLdrAudioPlaybackParms audioPlaybackParms_0xb6b9074b;
    CAssetId beamEffect;
    int unknown_0xd4753ff4;
    float unknown_0x05fc6001;
    float unknown_0x13e5b580;
    float unknown_0xfc6f199d;
    CAssetId grappleVisorEffect;
    SLdrDamageInfo damageInfo;
    CAssetId part_0x54b6bfa1;
    SLdrAudioPlaybackParms audioPlaybackParms_0x5cf705f2;
    CAssetId part_0xb9f9f4f2;
    CAssetId alternateScannableInfo;
    SLdrIngPossessionData ingPossessionData;
};

void LoadTypedefSLdrGrenchler(SLdrGrenchler&, CInputStream&);

#endif // _SLDRGRENCHLER
