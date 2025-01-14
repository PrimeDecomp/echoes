#ifndef _SLDRCAMERAPITCH
#define _SLDRCAMERAPITCH

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplineType.hpp"

struct SLdrCameraPitch {
    SLdrCameraPitch();
    ~SLdrCameraPitch();

    SLdrEditorProperties editorProperties;
    SLdrSpline forwardsPitch;
    SLdrSpline backwardsPitch;
    SLdrSplineType splineType;
    bool unknown;
};

void LoadTypedefSLdrCameraPitch(SLdrCameraPitch&, CInputStream&);

#endif // _SLDRCAMERAPITCH
