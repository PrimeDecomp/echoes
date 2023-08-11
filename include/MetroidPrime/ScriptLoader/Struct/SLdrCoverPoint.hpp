#ifndef _SLDRCOVERPOINT
#define _SLDRCOVERPOINT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrCoverPoint {
    SLdrCoverPoint();
    ~SLdrCoverPoint();

    SLdrEditorProperties editorProperties;
    int unknown;
    bool shouldCrouch;
    float horizontalSafeAngle;
    float verticalSafeAngle;
    float lockTime;
};

void LoadTypedefSLdrCoverPoint(SLdrCoverPoint&, CInputStream&);

#endif // _SLDRCOVERPOINT
