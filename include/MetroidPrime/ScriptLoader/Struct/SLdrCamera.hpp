#ifndef _SLDRCAMERA
#define _SLDRCAMERA

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplineType.hpp"

struct SLdrCamera {
    SLdrCamera();
    ~SLdrCamera();

    SLdrEditorProperties editor_properties;
    float animation_time;
    int unknown_0x05c5fc6e;
    int unknown_0xd4b29446;
    SLdrSplineType motion_spline_type;
    SLdrSplineType target_spline_type;
    spline motion_control_spline;
    spline target_control_spline;
    spline fov_spline;
    spline roll_spline;
    spline slowmo_control_spline;
};

void LoadTypedefSLdrCamera(SLdrCamera&, CInputStream&);

#endif // _SLDRCAMERA
