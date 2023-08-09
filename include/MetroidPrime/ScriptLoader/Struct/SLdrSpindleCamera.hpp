#ifndef _SLDRSPINDLECAMERA
#define _SLDRSPINDLECAMERA

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSpindleCameraStruct.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplineType.hpp"

struct SLdrSpindleCamera {
    SLdrSpindleCamera();
    ~SLdrSpindleCamera();

    SLdrEditorProperties editor_properties;
    int flags_spindle_camera;
    SLdrSpindleCameraStruct spindle_camera_struct_0xe56495fb;
    SLdrSpindleCameraStruct spindle_camera_struct_0x239debfc;
    SLdrSpindleCameraStruct spindle_camera_struct_0x27e8d703;
    SLdrSpindleCameraStruct spindle_camera_struct_0x2f914525;
    SLdrSpindleCameraStruct spindle_camera_struct_0x23aa31b7;
    SLdrSpindleCameraStruct spindle_camera_struct_0xe9e388af;
    SLdrSpindleCameraStruct spindle_camera_struct_0xde5a2c87;
    SLdrSpindleCameraStruct spindle_camera_struct_0x1b3b2394;
    SLdrSpindleCameraStruct spindle_camera_struct_0xf5666b6e;
    SLdrSpindleCameraStruct spindle_camera_struct_0x66c618aa;
    SLdrSpindleCameraStruct spindle_camera_struct_0xb36d0fb6;
    SLdrSpindleCameraStruct spindle_camera_struct_0xcbb013cb;
    SLdrSpindleCameraStruct spindle_camera_struct_0x4abfb789;
    SLdrSpindleCameraStruct spindle_camera_struct_0xfb6a407a;
    SLdrSpindleCameraStruct spindle_camera_struct_0x3ae66f80;
    SLdrSpindleCameraStruct spindle_camera_struct_0x6654ae92;
    SLdrSplineType target_spline_type;
    bool unknown_0x33b4f106;
    spline target_control_spline;
    SLdrSplineType spline_type;
    bool unknown_0x431769c6;
};

void LoadTypedefSLdrSpindleCamera(SLdrSpindleCamera&, CInputStream&);

#endif // _SLDRSPINDLECAMERA
