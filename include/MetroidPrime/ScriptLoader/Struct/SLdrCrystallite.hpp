#ifndef _SLDRCRYSTALLITE
#define _SLDRCRYSTALLITE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrCrystallite {
    SLdrCrystallite();
    ~SLdrCrystallite();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    float waypoint_approach_distance;
    float wall_turn_speed;
    float floor_turn_speed;
    float down_turn_speed;
    float visible_distance;
    float forward_moving_priority;
    float stun_time;
};

void LoadTypedefSLdrCrystallite(SLdrCrystallite&, CInputStream&);

#endif // _SLDRCRYSTALLITE
