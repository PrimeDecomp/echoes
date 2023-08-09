#ifndef _SLDRATOMICALPHA
#define _SLDRATOMICALPHA

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrAtomicAlpha {
    SLdrAtomicAlpha();
    ~SLdrAtomicAlpha();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    CAssetId bomb_weapon;
    CAssetId bomb_model;
    SLdrDamageInfo bomb_damage;
    float bomb_drop_delay;
    float bomb_reappear_delay;
    float bomb_reappear_time;
    bool invisible;
    bool home_while_charging;
};

void LoadTypedefSLdrAtomicAlpha(SLdrAtomicAlpha&, CInputStream&);

#endif // _SLDRATOMICALPHA
