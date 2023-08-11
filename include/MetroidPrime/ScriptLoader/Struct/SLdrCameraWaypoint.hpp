#ifndef _SLDRCAMERAWAYPOINT
#define _SLDRCAMERAWAYPOINT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrCameraWaypoint {
    SLdrCameraWaypoint();
    ~SLdrCameraWaypoint();

    SLdrEditorProperties editorProperties;
};

void LoadTypedefSLdrCameraWaypoint(SLdrCameraWaypoint&, CInputStream&);

#endif // _SLDRCAMERAWAYPOINT
