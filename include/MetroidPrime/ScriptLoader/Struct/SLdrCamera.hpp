#ifndef _SLDRCAMERA
#define _SLDRCAMERA

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplineType.hpp"

struct SLdrCamera {
    SLdrCamera();
    ~SLdrCamera();

    SLdrEditorProperties editorProperties;
    float animationTime;
    int unknown_0x05c5fc6e;
    int unknown_0xd4b29446;
    SLdrSplineType motionSplineType;
    SLdrSplineType targetSplineType;
    spline motionControlSpline;
    spline targetControlSpline;
    spline fovSpline;
    spline rollSpline;
    spline slowmoControlSpline;
};

void LoadTypedefSLdrCamera(SLdrCamera&, CInputStream&);

#endif // _SLDRCAMERA
