#ifndef _SLDRDYNAMICLIGHTINTENSITY
#define _SLDRDYNAMICLIGHTINTENSITY

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrDynamicLightIntensity {
    SLdrDynamicLightIntensity();
    ~SLdrDynamicLightIntensity();

    spline intensity;
    float intensity_duration;
    bool intensity_loops;
};

void LoadTypedefSLdrDynamicLightIntensity(SLdrDynamicLightIntensity&, CInputStream&);

#endif // _SLDRDYNAMICLIGHTINTENSITY
