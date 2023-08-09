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

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrIngPossessionData ing_possession_data;
    bool flotsam;
    bool avoid_down_frames;
    float melee_attack_min_range;
    float melee_attack_max_range;
    SLdrDamageInfo melee_attack_damage;
    float unknown;
    float ranged_attack_min_range;
    float ranged_attack_max_range;
    SLdrDamageInfo ranged_attack_damage;
    CAssetId ranged_attack_projectile;
    int ragdoll_impact_sound;
    bool fires_missiles;
    CAssetId missile_projectile;
    SLdrDamageInfo missile_damage;
    CAssetId scannable_info_when_attacking;
};

void LoadTypedefSLdrDarkTrooper(SLdrDarkTrooper&, CInputStream&);

#endif // _SLDRDARKTROOPER
