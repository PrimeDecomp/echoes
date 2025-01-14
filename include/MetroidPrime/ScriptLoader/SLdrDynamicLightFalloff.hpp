#ifndef _SLDRDYNAMICLIGHTFALLOFF
#define _SLDRDYNAMICLIGHTFALLOFF

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrDynamicLightFalloff {
    SLdrDynamicLightFalloff();
    ~SLdrDynamicLightFalloff();

    int falloffType;
    SLdrSpline falloffRate;
    float falloffRateDuration;
    bool falloffRateLoops;
};

void LoadTypedefSLdrDynamicLightFalloff(SLdrDynamicLightFalloff&, CInputStream&);

#endif // _SLDRDYNAMICLIGHTFALLOFF
