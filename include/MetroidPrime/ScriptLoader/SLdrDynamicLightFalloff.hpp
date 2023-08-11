#ifndef _SLDRDYNAMICLIGHTFALLOFF
#define _SLDRDYNAMICLIGHTFALLOFF

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrDynamicLightFalloff {
    SLdrDynamicLightFalloff();
    ~SLdrDynamicLightFalloff();

    int falloffType;
    spline falloffRate;
    float falloffRateDuration;
    bool falloffRateLoops;
};

void LoadTypedefSLdrDynamicLightFalloff(SLdrDynamicLightFalloff&, CInputStream&);

#endif // _SLDRDYNAMICLIGHTFALLOFF
