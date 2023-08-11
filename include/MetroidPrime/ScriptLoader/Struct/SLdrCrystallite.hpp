#ifndef _SLDRCRYSTALLITE
#define _SLDRCRYSTALLITE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPatternedAITypedef.hpp"

struct SLdrCrystallite {
    SLdrCrystallite();
    ~SLdrCrystallite();

    SLdrEditorProperties editorProperties;
    SLdrPatternedAITypedef patterned;
    SLdrActorParameters actorInformation;
    float waypointApproachDistance;
    float wallTurnSpeed;
    float floorTurnSpeed;
    float downTurnSpeed;
    float visibleDistance;
    float forwardMovingPriority;
    float stunTime;
};

void LoadTypedefSLdrCrystallite(SLdrCrystallite&, CInputStream&);

#endif // _SLDRCRYSTALLITE
