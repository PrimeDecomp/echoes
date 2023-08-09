#ifndef _SLDRGLOWBUG
#define _SLDRGLOWBUG

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrGlowbug {
    SLdrGlowbug();
    ~SLdrGlowbug();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    CAssetId death_flash_effect;
    CAssetId part;
    CAssetId attack_effect;
    CAssetId attack_telegraph_effect;
    CAssetId attack_echo_effect;
    float attack_duration;
    float attack_telegraph_duration;
    CVector3f attack_aim_offset;
    int attack_telegraph_sound;
    int attack_sound;
    CAssetId scan_model;
    bool is_in_light_world;
};

void LoadTypedefSLdrGlowbug(SLdrGlowbug&, CInputStream&);

#endif // _SLDRGLOWBUG
