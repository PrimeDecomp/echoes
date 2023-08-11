#ifndef _SLDRPARASITE
#define _SLDRPARASITE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrParasite {
    SLdrParasite();
    ~SLdrParasite();

    SLdrEditorProperties editorProperties;
    int flavor;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    float telegraphDistance;
    float waypointApproachDistance;
    float wallTurnSpeed;
    float floorTurnSpeed;
    float downTurnSpeed;
    float stuckTime;
    float unknown_0xd5c25506;
    float behaviorInfluenceRadius;
    float separationDistance;
    float separationPriority;
    float alignmentPriority;
    float unknown_0x61959f0d;
    float pathFollowingPriority;
    float forwardMovingPriority;
    float playerAvoidanceDistance;
    float playerAvoidancePriority;
    float parasiteVisibleDistance;
    bool initiallyPaused;
};

void LoadTypedefSLdrParasite(SLdrParasite&, CInputStream&);

#endif // _SLDRPARASITE
