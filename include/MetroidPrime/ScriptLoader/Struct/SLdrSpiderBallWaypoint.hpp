#ifndef _SLDRSPIDERBALLWAYPOINT
#define _SLDRSPIDERBALLWAYPOINT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrSpiderBallWaypoint {
    SLdrSpiderBallWaypoint();
    ~SLdrSpiderBallWaypoint();

    SLdrEditorProperties editor_properties;
    int flags;
};

void LoadTypedefSLdrSpiderBallWaypoint(SLdrSpiderBallWaypoint&, CInputStream&);

#endif // _SLDRSPIDERBALLWAYPOINT
