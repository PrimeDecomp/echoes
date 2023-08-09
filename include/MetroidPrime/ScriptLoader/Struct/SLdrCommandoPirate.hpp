#ifndef _SLDRCOMMANDOPIRATE
#define _SLDRCOMMANDOPIRATE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct10.hpp"
#include "MetroidPrime/ScriptLoader/SLdrUnknownStruct11.hpp"

struct SLdrCommandoPirate {
    SLdrCommandoPirate();
    ~SLdrCommandoPirate();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    SLdrIngPossessionData ing_possession_data;
    int sound;
    float aggressiveness;
    float cover_check;
    float search_radius;
    float dodge_check;
    int sound_impact;
    int sound_hurled;
    int sound_death;
    int always_ff_0xfca76593;
    int always_ff_0x467c3d94;
    SLdrDamageInfo blade_damage;
    CAssetId projectile;
    SLdrDamageInfo projectile_damage;
    int sound_projectile;
    float hearing_radius;
    SLdrUnknownStruct10 unknown_struct10;
    SLdrUnknownStruct11 unknown_struct11;
    float unknown_0x71587b45;
    float unknown_0x7903312e;
};

void LoadTypedefSLdrCommandoPirate(SLdrCommandoPirate&, CInputStream&);

#endif // _SLDRCOMMANDOPIRATE
