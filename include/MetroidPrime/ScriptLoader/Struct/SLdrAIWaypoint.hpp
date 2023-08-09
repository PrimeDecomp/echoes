#ifndef _SLDRAIWAYPOINT
#define _SLDRAIWAYPOINT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrAIWaypoint {
    SLdrAIWaypoint();
    ~SLdrAIWaypoint();

    SLdrEditorProperties editor_properties;
    float speed;
    float pause;
    int unknown_0xc6705a00;
    int locator_index;
    int unknown_0x166979d4;
};

void LoadTypedefSLdrAIWaypoint(SLdrAIWaypoint&, CInputStream&);

#endif // _SLDRAIWAYPOINT
