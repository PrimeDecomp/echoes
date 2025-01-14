#ifndef _SLDRCAMERA
#define _SLDRCAMERA

#include "Kyoto/Math/CMayaSpline.hpp"
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
    SLdrSpline motionControlSpline;
    SLdrSpline targetControlSpline;
    SLdrSpline fovSpline;
    SLdrSpline rollSpline;
    SLdrSpline slowmoControlSpline;
};

void LoadTypedefSLdrCamera(SLdrCamera&, CInputStream&);

#endif // _SLDRCAMERA
