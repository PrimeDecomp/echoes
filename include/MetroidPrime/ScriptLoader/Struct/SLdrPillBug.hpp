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

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    int unknown_0xde7e9f94;
    float floorTurnSpeed;
    float stickRadius;
    float waypointApproachDistance;
    float visibleDistance;
    SLdrDamageVulnerability damageVulnerability;
    SLdrDamageVulnerability wanderVulnerability;
    float crawlRadius;
    float rollRadius;
    float unknown_0x519c7197;
    float unknown_0xa265383c;
    float forwardPriority;
    float unknown_0x558c0692;
    float unknown_0x0f991bf1;
    float unknown_0x385a1bed;
    float unknown_0xcf4ea141;
};

void LoadTypedefSLdrPillBug(SLdrPillBug&, CInputStream&);

#endif // _SLDRPILLBUG
