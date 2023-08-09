#ifndef _SLDRDYNAMICLIGHTMOTIONSPLINE
#define _SLDRDYNAMICLIGHTMOTIONSPLINE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplineType.hpp"

struct SLdrDynamicLightMotionSpline {
    SLdrDynamicLightMotionSpline();
    ~SLdrDynamicLightMotionSpline();

    bool motion_spline_path_loops;
    SLdrSplineType motion_spline_type;
    spline motion_control_spline;
    float motion_spline_duration;
};

void LoadTypedefSLdrDynamicLightMotionSpline(SLdrDynamicLightMotionSpline&, CInputStream&);

#endif // _SLDRDYNAMICLIGHTMOTIONSPLINE
