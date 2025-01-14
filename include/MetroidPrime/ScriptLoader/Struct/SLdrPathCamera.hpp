#ifndef _SLDRPATHCAMERA
#define _SLDRPATHCAMERA

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplineType.hpp"

struct SLdrPathCamera {
    SLdrPathCamera();
    ~SLdrPathCamera();

    SLdrEditorProperties editorProperties;
    int flagsPathCamera;
    int unknown_0xd4b29446;
    SLdrSplineType motionSplineType;
    SLdrSpline motionControlSpline;
    SLdrSplineType targetSplineType;
    SLdrSpline targetControlSpline;
    SLdrSpline fovSpline;
    SLdrSpline speedControlSpline;
    SLdrSplineType splineType;
    bool unknown_0x431769c6;
    float distance;
    float speed;
    float dampenDistance;
    int initialPosition;
    float angularSpeed;
    SLdrSpline unknown_0x12861f7d;
    SLdrSpline unknown_0x96ac52b0;
};

void LoadTypedefSLdrPathCamera(SLdrPathCamera&, CInputStream&);

#endif // _SLDRPATHCAMERA
