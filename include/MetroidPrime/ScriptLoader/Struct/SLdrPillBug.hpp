#ifndef _SLDRPILLBUG
#define _SLDRPILLBUG

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrPillBug {
    SLdrPillBug();
    ~SLdrPillBug();

    SLdrEditorProperties editor_properties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actor_information;
    int unknown_0xde7e9f94;
    float floor_turn_speed;
    float stick_radius;
    float waypoint_approach_distance;
    float visible_distance;
    SLdrDamageVulnerability damage_vulnerability;
    SLdrDamageVulnerability wander_vulnerability;
    float crawl_radius;
    float roll_radius;
    float unknown_0x519c7197;
    float unknown_0xa265383c;
    float forward_priority;
    float unknown_0x558c0692;
    float unknown_0x0f991bf1;
    float unknown_0x385a1bed;
    float unknown_0xcf4ea141;
};

void LoadTypedefSLdrPillBug(SLdrPillBug&, CInputStream&);

#endif // _SLDRPILLBUG
