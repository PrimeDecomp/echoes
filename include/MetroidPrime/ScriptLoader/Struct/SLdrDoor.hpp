#ifndef _SLDRDOOR
#define _SLDRDOOR

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrHealthInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScannableParameters.hpp"

struct SLdrDoor {
    SLdrDoor();
    ~SLdrDoor();

    SLdrEditorProperties editor_properties;
    CVector3f collision_box;
    CVector3f collision_offset;
    SLdrHealthInfo health;
    SLdrDamageVulnerability vulnerability;
    SLdrAnimationParameters animation_information;
    CAssetId shell_model;
    CAssetId blue_shell_model;
    CColor shell_color;
    CAssetId burn_texture;
    SLdrActorParameters actor_information;
    CVector3f orbit_offset;
    bool is_open;
    bool is_locked;
    float open_animation_time;
    float close_animation_time;
    float close_delay;
    float shield_fade_out_time;
    float shield_fade_in_time;
    bool morph_ball_tunnel;
    bool horizontal;
    SLdrScannableParameters alt_scannable;
};

void LoadTypedefSLdrDoor(SLdrDoor&, CInputStream&);

#endif // _SLDRDOOR
