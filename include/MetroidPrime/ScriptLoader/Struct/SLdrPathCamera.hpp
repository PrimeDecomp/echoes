#ifndef _SLDRPATHCAMERA
#define _SLDRPATHCAMERA

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplineType.hpp"

struct SLdrPathCamera {
    SLdrPathCamera();
    ~SLdrPathCamera();

    SLdrEditorProperties editor_properties;
    int flags_path_camera;
    int unknown_0xd4b29446;
    SLdrSplineType motion_spline_type;
    spline motion_control_spline;
    SLdrSplineType target_spline_type;
    spline target_control_spline;
    spline fov_spline;
    spline speed_control_spline;
    SLdrSplineType spline_type;
    bool unknown_0x431769c6;
    float distance;
    float speed;
    float dampen_distance;
    int initial_position;
    float angular_speed;
    spline unknown_0x12861f7d;
    spline unknown_0x96ac52b0;
};

void LoadTypedefSLdrPathCamera(SLdrPathCamera&, CInputStream&);

#endif // _SLDRPATHCAMERA
