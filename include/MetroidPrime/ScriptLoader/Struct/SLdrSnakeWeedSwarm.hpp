#ifndef _SLDRSNAKEWEEDSWARM
#define _SLDRSNAKEWEEDSWARM

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrSnakeWeedSwarm {
    SLdrSnakeWeedSwarm();
    ~SLdrSnakeWeedSwarm();

    SLdrEditorProperties editor_properties;
    SLdrAnimationParameters animation_information;
    SLdrActorParameters actor_information;
    float density;
    float max_depth;
    float location_variance;
    float detection_radius;
    float grab_radius;
    float unknown_0x723737bc;
    float unknown_0x57452dd9;
    float retreat_depth;
    float move_speed;
    float unknown_0x11f854e2;
    float max_slope;
    float min_size;
    float max_size;
    float height_offset;
    SLdrDamageInfo contact_damage;
    float damage_wait_time;
    int sound_looped;
    int sound_into_ground;
    int sound_outof_ground;
};

void LoadTypedefSLdrSnakeWeedSwarm(SLdrSnakeWeedSwarm&, CInputStream&);

#endif // _SLDRSNAKEWEEDSWARM
