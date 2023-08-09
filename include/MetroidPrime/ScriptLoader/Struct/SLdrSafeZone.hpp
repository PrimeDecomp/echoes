#ifndef _SLDRSAFEZONE
#define _SLDRSAFEZONE

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSafeZoneStructA.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSafeZoneStructB.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTriggerInfo.hpp"

struct SLdrSafeZone {
    SLdrSafeZone();
    ~SLdrSafeZone();

    SLdrEditorProperties editor_properties;
    SLdrTriggerInfo trigger;
    bool deactivate_on_enter;
    bool deactivate_on_exit;
    float activation_time;
    float deactivation_time;
    float lifetime;
    float random_lifetime_offset;
    CAssetId impact_effect;
    bool filter_sound_effects;
    int unknown_0x414379ea;
    bool ignore_cinematic_camera;
    SLdrSafeZoneStructB normal_safe_zone_struct;
    SLdrSafeZoneStructB energized_safe_zone_struct;
    SLdrSafeZoneStructB supercharged_safe_zone_struct;
    SLdrDamageInfo normal_damage;
    SLdrDamageInfo damage_info;
    float inside_fade_start;
    float inside_fade_time;
    float unknown_0x6c14904c;
    float flash_time;
    float flash_brightness;
    int flash_sound;
    int safezone_shape;
    bool mobile;
    bool generate_mobile_light;
    CVector3f mobile_light_offset;
    CColor unknown_0xe71b43e1;
    float unknown_0x9f638987;
    SLdrSafeZoneStructA safe_zone_struct_a_0x8a09f99a;
    SLdrSafeZoneStructA safe_zone_struct_a_0xafb855b8;
    SLdrEchoParameters echo_parameters;
};

void LoadTypedefSLdrSafeZone(SLdrSafeZone&, CInputStream&);

#endif // _SLDRSAFEZONE
