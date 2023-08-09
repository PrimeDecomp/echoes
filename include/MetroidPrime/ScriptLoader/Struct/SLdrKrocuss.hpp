#ifndef _SLDRKROCUSS
#define _SLDRKROCUSS

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrKrocuss {
    SLdrKrocuss();
    ~SLdrKrocuss();

    SLdrEditorProperties editor_properties;
    int flavor;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    float waypoint_approach_distance;
    float visible_distance;
    float wall_turn_speed;
    float floor_turn_speed;
    float down_turn_speed;
    float unknown_0xd5c25506;
    float projectile_bounds_multiplier;
    float collision_look_ahead;
    float anim_speed_scalar;
    bool initially_paused;
    float unknown_0xf04cadca;
    float unknown_0x497d54e8;
    float unknown_0x3371c963;
    float unknown_0x22d37771;
    float unknown_0xbbebed9e;
    SLdrDamageVulnerability shell_closed_vulnerability;
    CColor wing_light_color;
    CAssetId dpsc;
    int shell_open_sound;
    int shell_close_sound;
    float max_audible_distance;
};

void LoadTypedefSLdrKrocuss(SLdrKrocuss&, CInputStream&);

#endif // _SLDRKROCUSS
