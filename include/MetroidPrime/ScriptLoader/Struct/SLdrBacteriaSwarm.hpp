#ifndef _SLDRBACTERIASWARM
#define _SLDRBACTERIASWARM

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAnimationParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrBasicSwarmProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrBacteriaSwarm {
    SLdrBacteriaSwarm();
    ~SLdrBacteriaSwarm();

    SLdrEditorProperties editor_properties;
    SLdrActorParameters actor_information;
    SLdrAnimationParameters animation_information;
    bool active;
    SLdrBasicSwarmProperties basic_swarm_properties;
    float unknown_0x4a85a2da;
    float containment_priority;
    float bacteria_patrol_speed;
    float unknown_0x7de56d56;
    float unknown_0x39098c47;
    float bacteria_acceleration;
    float bacteria_deceleration;
    float patrol_turn_speed;
    float unknown_0xbdcdb9c0;
    CAssetId bacteria_particle_effect;
    CColor bacteria_patrol_color;
    CColor bacteria_player_pursuit_color;
    float color_change_time;
    int patrol_sound;
    int pursuit_sound;
    float unknown_0xad4ce8f3;
    float unknown_0xa9d6d9d9;
    float patrol_sound_weight;
    float unknown_0x90f8e29f;
    float unknown_0x4b47b178;
    float pursuit_sound_weight;
    float unknown_0xd2986c43;
    float max_audible_distance;
    int min_volume;
    int max_volume;
    CAssetId bacteria_scan_model;
    bool spawn_instantly;
};

void LoadTypedefSLdrBacteriaSwarm(SLdrBacteriaSwarm&, CInputStream&);

#endif // _SLDRBACTERIASWARM
