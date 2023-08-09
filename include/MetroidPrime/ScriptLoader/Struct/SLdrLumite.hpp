#ifndef _SLDRLUMITE
#define _SLDRLUMITE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrLumite {
    SLdrLumite();
    ~SLdrLumite();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    float unknown_0x2d9ebd7f;
    float unknown_0x6dd1c509;
    CAssetId small_shot_projectile;
    SLdrDamageInfo small_shot_damage;
    float unknown_0x6d5356bb;
    float unknown_0x2d1c2ecd;
    CAssetId big_shot_projectile;
    SLdrDamageInfo big_shot_damage;
    CAssetId trail_effect;
    CAssetId sunlight_enter_exit_effect;
    float unknown_0xe05d93ef;
    float unknown_0x47691396;
    int phase_in_sound;
    int phase_out_sound;
};

void LoadTypedefSLdrLumite(SLdrLumite&, CInputStream&);

#endif // _SLDRLUMITE
