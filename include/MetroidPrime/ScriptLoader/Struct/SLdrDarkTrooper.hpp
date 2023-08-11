#ifndef _SLDRDARKTROOPER
#define _SLDRDARKTROOPER

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrDarkTrooper {
    SLdrDarkTrooper();
    ~SLdrDarkTrooper();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    SLdrIngPossessionData ingPossessionData;
    bool flotsam;
    bool avoidDownFrames;
    float meleeAttackMinRange;
    float meleeAttackMaxRange;
    SLdrDamageInfo meleeAttackDamage;
    float unknown;
    float rangedAttackMinRange;
    float rangedAttackMaxRange;
    SLdrDamageInfo rangedAttackDamage;
    CAssetId rangedAttackProjectile;
    int ragdollImpactSound;
    bool firesMissiles;
    CAssetId missileProjectile;
    SLdrDamageInfo missileDamage;
    CAssetId scannableInfoWhenAttacking;
};

void LoadTypedefSLdrDarkTrooper(SLdrDarkTrooper&, CInputStream&);

#endif // _SLDRDARKTROOPER
