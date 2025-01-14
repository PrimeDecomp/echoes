#ifndef _SLDRDYNAMICLIGHTINTENSITY
#define _SLDRDYNAMICLIGHTINTENSITY

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrDynamicLightIntensity {
    SLdrDynamicLightIntensity();
    ~SLdrDynamicLightIntensity();

    SLdrSpline intensity;
    float intensityDuration;
    bool intensityLoops;
};

void LoadTypedefSLdrDynamicLightIntensity(SLdrDynamicLightIntensity&, CInputStream&);

#endif // _SLDRDYNAMICLIGHTINTENSITY
