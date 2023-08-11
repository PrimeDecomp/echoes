#ifndef _SLDRBLOGG
#define _SLDRBLOGG

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrBloggStruct.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrBlogg {
    SLdrBlogg();
    ~SLdrBlogg();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    float minAttackAngle;
    float maxAttackAngle;
    float unknown_0x32455774;
    float unknown_0xc1c8d862;
    int unknown_0xa19d5f62;
    CAssetId projectileParticleEffect;
    SLdrDamageInfo projectileDamage;
    float bodyDamageMultiplier;
    float mouthDamageMultiplier;
    float mouthDamageAngle;
    SLdrDamageVulnerability armorVulnerability;
    float chargeDamageRadius;
    float chargeDamage;
    float biteDamage;
    float ballSpitDamage;
    float chargeTurnSpeed;
    float fishAttractionRadius;
    float fishAttractionPriority;
    float aggressiveness;
    float unknown_0x479ccc37;
    float unknown_0x689a803f;
    float unknown_0x800a2b0d;
    float chargeSpeedMultiplier;
    float maxMeleeRange;
    float unknown_0x6a78c607;
    float unknown_0x2c8d9fc4;
    float unknown_0xd1f82f92;
    float unknown_0x5109fb4e;
    float maxCollisionTime;
    int mouthOpenSound;
    SLdrIngPossessionData ingPossessionData;
    SLdrDamageVulnerability ingPossessedArmorVulnerability;
    bool isMegaBlogg;
    float projectileBlurRadius;
    float projectileBlurTime;
    SLdrBloggStruct bloggStruct_0x3874576d;
    SLdrBloggStruct bloggStruct_0x97dd1aa7;
    SLdrBloggStruct bloggStruct_0xf2ba21e1;
};

void LoadTypedefSLdrBlogg(SLdrBlogg&, CInputStream&);

#endif // _SLDRBLOGG
