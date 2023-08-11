#ifndef _SLDRING
#define _SLDRING

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlasmaBeamInfo.hpp"

struct SLdrIng {
    SLdrIng();
    ~SLdrIng();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    int unknown_0x092fbad0;
    CAssetId facePlateModel;
    float hearingRadius;
    float ingSpotMaxSpeed;
    float unknown_0x8d42a8d5;
    float unknown_0x84586bfd;
    float unknown_0x50398a06;
    float ingSpotTurnSpeed;
    CAssetId ingSpotBlobEffect;
    CAssetId ingSpotHitNormalDamage;
    CAssetId ingSpotHitHeavyDamage;
    CAssetId ingSpotDeath;
    int soundIngSpotIdle;
    int soundIngSpotMove;
    int sound_0xb392943a;
    int sound_0x24ecc1e9;
    int soundIngSpotDeath;
    CAssetId part_0x3c2d681e;
    CAssetId srsc;
    CAssetId part_0x3da219c7;
    float unknown_0x23271976;
    CAssetId part_0x081e9e6c;
    float unknown_0xcb39eccb;
    float unknown_0x587ca175;
    float unknown_0x0bd7d5a9;
    int soundSwarmMove;
    int sound_0x5650366a;
    int soundBodyProjectileBlasterMiddle;
    int sound_0x0c13c5a8;
    int sound_0x148b81e4;
    float unknown_0x5d0d2c40;
    float unknown_0xc620183a;
    float frustrationTime;
    float tauntChance;
    float aggressiveness;
    SLdrDamageInfo armSwipeDamage;
    SLdrDamageInfo bodyProjectileContactDamage;
    float unknown_0xa0d63374;
    float bodyProjectileSuckTime;
    CAssetId bodyProjectileSplatEffect;
    float bodyProjectileSpeed;
    float bodyProjectileDropTime;
    float unknown_0xe6ddb662;
    float unknown_0xb57bae86;
    int soundBodyProjectile;
    int soundBodyProjectileSplatWall;
    float bodyProjectileOdds;
    float unknown_0xfa6edeb5;
    float unknown_0xa9c8c651;
    CAssetId miniPortalEffect;
    int soundMiniPortal;
    SLdrDamageInfo miniPortalProjectileDamage;
    SLdrPlasmaBeamInfo miniPortalBeamInfo;
    float unknown_0x67f6c10e;
    SLdrDamageInfo exitGrappleDamage;
    float exitGrappleSpitForce;
    int soundGrapple;
    int soundExitGrapple;
    float unknown_0x421651f6;
    float unknown_0x560b4a95;
    float unknown_0x8bdcc614;
    CColor lightColor;
    float lightAttenuation;
    SLdrDamageVulnerability ingSpotVulnerability;
    SLdrDamageVulnerability grappleBallVulnerability;
    SLdrDamageVulnerability triggerVulnerability;
};

void LoadTypedefSLdrIng(SLdrIng&, CInputStream&);

#endif // _SLDRING
