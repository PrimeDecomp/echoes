#ifndef _SLDRKRALEE
#define _SLDRKRALEE

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrKralee {
    SLdrKralee();
    ~SLdrKralee();

    SLdrEditorProperties editor_properties;
    Choice flavor;
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
    float warp_in_time;
    float warp_out_time;
    float visible_time;
    float unknown_0x7bba36ff;
    float invisible_time;
    float unknown_0x4e4ae0e4;
    float warp_attack_radius;
    float warp_attack_knockback;
    float warp_attack_damage;
    float anim_speed_scalar;
    float max_audible_distance;
    CAssetId warp_in_particle_effect;
    CAssetId warp_out_particle_effect;
    int warp_in_sound;
    int warp_out_sound;
    bool initially_paused;
    bool initially_invisible;
};

void LoadTypedefSLdrKralee(SLdrKralee&, CInputStream&);

#endif // _SLDRKRALEE
