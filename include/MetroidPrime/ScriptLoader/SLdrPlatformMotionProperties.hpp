#ifndef _SLDRPLATFORMMOTIONPROPERTIES
#define _SLDRPLATFORMMOTIONPROPERTIES

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplineType.hpp"

struct SLdrPlatformMotionProperties {
    SLdrPlatformMotionProperties();
    ~SLdrPlatformMotionProperties();

    SLdrSplineType motion_spline_type;
    spline motion_control_spline;
    float motion_spline_duration;
    float initial_time;
    int unknown;
    spline roll_control_spline;
    spline yaw_control_spline;
    spline pitch_control_spline;
};

void LoadTypedefSLdrPlatformMotionProperties(SLdrPlatformMotionProperties&, CInputStream&);

#endif // _SLDRPLATFORMMOTIONPROPERTIES
