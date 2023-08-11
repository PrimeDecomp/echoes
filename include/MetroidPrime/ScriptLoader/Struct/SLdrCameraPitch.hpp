#ifndef _SLDRCAMERAPITCH
#define _SLDRCAMERAPITCH

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplineType.hpp"

struct SLdrCameraPitch {
    SLdrCameraPitch();
    ~SLdrCameraPitch();

    SLdrEditorProperties editorProperties;
    spline forwardsPitch;
    spline backwardsPitch;
    SLdrSplineType splineType;
    bool unknown;
};

void LoadTypedefSLdrCameraPitch(SLdrCameraPitch&, CInputStream&);

#endif // _SLDRCAMERAPITCH
