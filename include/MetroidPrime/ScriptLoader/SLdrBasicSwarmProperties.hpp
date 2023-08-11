#ifndef _SLDRBASICSWARMPROPERTIES
#define _SLDRBASICSWARMPROPERTIES

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"

struct SLdrBasicSwarmProperties {
    SLdrBasicSwarmProperties();
    ~SLdrBasicSwarmProperties();

    SLdrDamageInfo contactDamage;
    float damageWaitTime;
    float collisionRadius;
    float touchRadius;
    float damageRadius;
    float speed;
    int count;
    int maxCount;
    float influenceRadius;
    float unknown_0x61959f0d;
    float alignmentPriority;
    float separationPriority;
    float pathFollowingPriority;
    float playerAttractPriority;
    float playerAttractDistance;
    float spawnSpeed;
    int attackerCount;
    float attackProximity;
    float attackTimer;
    SLdrHealthInfo health;
    SLdrDamageVulnerability damageVulnerability;
    CAssetId deathParticleEffect;
    int unknown_0x84f81f55;
    float unknown_0xe1bd61d8;
    float turnRate;
    bool unknown_0xc2208b0f;
    bool unknown_0x7eb5d9e8;
    bool isOrbitable;
    bool unknown_0xbc01a28e;
    int locomotionLoopedSound;
    int attackLoopedSound;
    float unknown_0xd2986c43;
    float maxAudibleDistance;
    int minVolume;
    int maxVolume;
    float freezeDuration;
    float lifeTime;
};

void LoadTypedefSLdrBasicSwarmProperties(SLdrBasicSwarmProperties&, CInputStream&);

#endif // _SLDRBASICSWARMPROPERTIES
