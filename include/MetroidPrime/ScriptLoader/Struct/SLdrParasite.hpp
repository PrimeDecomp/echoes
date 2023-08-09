#ifndef _SLDRPARASITE
#define _SLDRPARASITE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrParasite {
    SLdrParasite();
    ~SLdrParasite();

    SLdrEditorProperties editor_properties;
    int flavor;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    float telegraph_distance;
    float waypoint_approach_distance;
    float wall_turn_speed;
    float floor_turn_speed;
    float down_turn_speed;
    float stuck_time;
    float unknown_0xd5c25506;
    float behavior_influence_radius;
    float separation_distance;
    float separation_priority;
    float alignment_priority;
    float unknown_0x61959f0d;
    float path_following_priority;
    float forward_moving_priority;
    float player_avoidance_distance;
    float player_avoidance_priority;
    float parasite_visible_distance;
    bool initially_paused;
};

void LoadTypedefSLdrParasite(SLdrParasite&, CInputStream&);

#endif // _SLDRPARASITE
