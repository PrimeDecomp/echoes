#ifndef _SLDRPLATFORMMOTIONPROPERTIES
#define _SLDRPLATFORMMOTIONPROPERTIES

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSplineType.hpp"

struct SLdrPlatformMotionProperties {
    SLdrPlatformMotionProperties();
    ~SLdrPlatformMotionProperties();

    SLdrSplineType motionSplineType;
    SLdrSpline motionControlSpline;
    float motionSplineDuration;
    float initialTime;
    int unknown;
    SLdrSpline rollControlSpline;
    SLdrSpline yawControlSpline;
    SLdrSpline pitchControlSpline;
};

void LoadTypedefSLdrPlatformMotionProperties(SLdrPlatformMotionProperties&, CInputStream&);

#endif // _SLDRPLATFORMMOTIONPROPERTIES
