#ifndef _SLDRDYNAMICLIGHTFALLOFF
#define _SLDRDYNAMICLIGHTFALLOFF

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrDynamicLightFalloff {
    SLdrDynamicLightFalloff();
    ~SLdrDynamicLightFalloff();

    int falloff_type;
    spline falloff_rate;
    float falloff_rate_duration;
    bool falloff_rate_loops;
};

void LoadTypedefSLdrDynamicLightFalloff(SLdrDynamicLightFalloff&, CInputStream&);

#endif // _SLDRDYNAMICLIGHTFALLOFF
