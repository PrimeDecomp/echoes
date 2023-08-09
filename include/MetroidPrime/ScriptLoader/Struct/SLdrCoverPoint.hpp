#ifndef _SLDRCOVERPOINT
#define _SLDRCOVERPOINT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrCoverPoint {
    SLdrCoverPoint();
    ~SLdrCoverPoint();

    SLdrEditorProperties editor_properties;
    int unknown;
    bool should_crouch;
    float horizontal_safe_angle;
    float vertical_safe_angle;
    float lock_time;
};

void LoadTypedefSLdrCoverPoint(SLdrCoverPoint&, CInputStream&);

#endif // _SLDRCOVERPOINT
