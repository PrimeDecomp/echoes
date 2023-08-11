#ifndef _SLDRPLANTSCARABSWARM
#define _SLDRPLANTSCARABSWARM

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrBasicSwarmProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrPlantScarabSwarm {
    SLdrPlantScarabSwarm();
    ~SLdrPlantScarabSwarm();

    SLdrEditorProperties editorProperties;
    SLdrActorParameters actorInformation;
    SLdrAnimationParameters animationInformation;
    bool active;
    SLdrBasicSwarmProperties basicSwarmProperties;
    int unknown_0x7399abbb;
    int unknown_0x734d923b;
    float maxAttackAngle;
    float intoAttackSpeed;
    float attackSpeed;
    float grenadeMass;
    float grenadeLaunchSpeed;
    float unknown_0xed086ce0;
    int unknown_0x454f16b1;
    SLdrDamageInfo grenadeDamage;
    float grenadeExplosionProximity;
    CAssetId grenadeExplosionEffect;
    CAssetId part;
    CAssetId grenadeTrailEffect;
    CAssetId grenadeEffect;
    int grenadeBounceSound;
    float grenadeBounceSoundFallOff;
    float unknown_0x15e0c159;
    int grenadeExplosionSound;
    float grenadeExplosionSoundFallOff;
    float unknown_0xab84892e;
};

void LoadTypedefSLdrPlantScarabSwarm(SLdrPlantScarabSwarm&, CInputStream&);

#endif // _SLDRPLANTSCARABSWARM
