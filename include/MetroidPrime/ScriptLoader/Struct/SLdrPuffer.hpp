#ifndef _SLDRPUFFER
#define _SLDRPUFFER

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrPuffer {
    SLdrPuffer();
    ~SLdrPuffer();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    float hover_speed;
    CAssetId cloud_effect;
    SLdrDamageInfo cloud_damage;
    CAssetId cloud_steam;
    float cloud_steam_alpha;
    bool orbit_interpolant_followed;
    bool cloud_in_dark;
    bool cloud_in_echo;
    SLdrDamageInfo explosion_damage;
    int sound_turn;
};

void LoadTypedefSLdrPuffer(SLdrPuffer&, CInputStream&);

#endif // _SLDRPUFFER
