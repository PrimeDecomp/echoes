#ifndef _SLDRDYNAMICLIGHTMOTIONSPLINE
#define _SLDRDYNAMICLIGHTMOTIONSPLINE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplineType.hpp"

struct SLdrDynamicLightMotionSpline {
    SLdrDynamicLightMotionSpline();
    ~SLdrDynamicLightMotionSpline();

    bool motionSplinePathLoops;
    SLdrSplineType motionSplineType;
    spline motionControlSpline;
    float motionSplineDuration;
};

void LoadTypedefSLdrDynamicLightMotionSpline(SLdrDynamicLightMotionSpline&, CInputStream&);

#endif // _SLDRDYNAMICLIGHTMOTIONSPLINE
