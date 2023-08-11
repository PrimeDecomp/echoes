#ifndef _SLDRPLATFORMMOTIONPROPERTIES
#define _SLDRPLATFORMMOTIONPROPERTIES

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplineType.hpp"

struct SLdrPlatformMotionProperties {
    SLdrPlatformMotionProperties();
    ~SLdrPlatformMotionProperties();

    SLdrSplineType motionSplineType;
    spline motionControlSpline;
    float motionSplineDuration;
    float initialTime;
    int unknown;
    spline rollControlSpline;
    spline yawControlSpline;
    spline pitchControlSpline;
};

void LoadTypedefSLdrPlatformMotionProperties(SLdrPlatformMotionProperties&, CInputStream&);

#endif // _SLDRPLATFORMMOTIONPROPERTIES
