#ifndef _SLDRSPACEPIRATE
#define _SLDRSPACEPIRATE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSpacePirateWeaponData.hpp"

struct SLdrSpacePirate {
    SLdrSpacePirate();
    ~SLdrSpacePirate();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrIngPossessionData ingPossessionData;
    float aggressiveness;
    float coverCheck;
    float searchRadius;
    float fallBackCheck;
    float fallBackRadius;
    float hearingRadius;
    int sound;
    bool unknown_0xce670970;
    CAssetId projectile;
    SLdrDamageInfo projectileDamage;
    int soundProjectile;
    SLdrDamageInfo bladeDamage;
    float kneelAttackChance;
    CAssetId kneelAttackShot;
    SLdrDamageInfo kneelAttackDamage;
    float dodgeCheck;
    int soundImpact;
    float unknown_0x71587b45;
    float unknown_0x7903312e;
    float unknown_0x5080162a;
    float unknown_0xc78b40e0;
    int soundAlert;
    float gunTrackDelay;
    int unknown_0x1b454a27;
    float cloakOpacity;
    float maxCloakOpacity;
    float unknown_0x61e801d4;
    float unknown_0xf19b113e;
    int soundHurled;
    int soundDeath;
    float unknown_0x8708b7d3;
    float avoidDistance;
    SLdrSpacePirateWeaponData weaponData;
};

void LoadTypedefSLdrSpacePirate(SLdrSpacePirate&, CInputStream&);

#endif // _SLDRSPACEPIRATE
