#ifndef _SLDRDYNAMICLIGHTINTENSITY
#define _SLDRDYNAMICLIGHTINTENSITY

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrDynamicLightIntensity {
    SLdrDynamicLightIntensity();
    ~SLdrDynamicLightIntensity();

    spline intensity;
    float intensityDuration;
    bool intensityLoops;
};

void LoadTypedefSLdrDynamicLightIntensity(SLdrDynamicLightIntensity&, CInputStream&);

#endif // _SLDRDYNAMICLIGHTINTENSITY
