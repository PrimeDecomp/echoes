#ifndef _SLDRSPINDLECAMERA
#define _SLDRSPINDLECAMERA

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSpindleCameraStruct.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplineType.hpp"

struct SLdrSpindleCamera {
    SLdrSpindleCamera();
    ~SLdrSpindleCamera();

    SLdrEditorProperties editorProperties;
    int flagsSpindleCamera;
    SLdrSpindleCameraStruct spindleCameraStruct_0xe56495fb;
    SLdrSpindleCameraStruct spindleCameraStruct_0x239debfc;
    SLdrSpindleCameraStruct spindleCameraStruct_0x27e8d703;
    SLdrSpindleCameraStruct spindleCameraStruct_0x2f914525;
    SLdrSpindleCameraStruct spindleCameraStruct_0x23aa31b7;
    SLdrSpindleCameraStruct spindleCameraStruct_0xe9e388af;
    SLdrSpindleCameraStruct spindleCameraStruct_0xde5a2c87;
    SLdrSpindleCameraStruct spindleCameraStruct_0x1b3b2394;
    SLdrSpindleCameraStruct spindleCameraStruct_0xf5666b6e;
    SLdrSpindleCameraStruct spindleCameraStruct_0x66c618aa;
    SLdrSpindleCameraStruct spindleCameraStruct_0xb36d0fb6;
    SLdrSpindleCameraStruct spindleCameraStruct_0xcbb013cb;
    SLdrSpindleCameraStruct spindleCameraStruct_0x4abfb789;
    SLdrSpindleCameraStruct spindleCameraStruct_0xfb6a407a;
    SLdrSpindleCameraStruct spindleCameraStruct_0x3ae66f80;
    SLdrSpindleCameraStruct spindleCameraStruct_0x6654ae92;
    SLdrSplineType targetSplineType;
    bool unknown_0x33b4f106;
    SLdrSpline targetControlSpline;
    SLdrSplineType splineType;
    bool unknown_0x431769c6;
};

void LoadTypedefSLdrSpindleCamera(SLdrSpindleCamera&, CInputStream&);

#endif // _SLDRSPINDLECAMERA
