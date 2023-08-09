#ifndef _SLDRDYNAMICLIGHTSPOTLIGHT
#define _SLDRDYNAMICLIGHTSPOTLIGHT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrDynamicLightSpotlight {
    SLdrDynamicLightSpotlight();
    ~SLdrDynamicLightSpotlight();

    spline spotlight_angle;
    float spotlight_angle_duration;
    bool spotlight_angle_loops;
};

void LoadTypedefSLdrDynamicLightSpotlight(SLdrDynamicLightSpotlight&, CInputStream&);

#endif // _SLDRDYNAMICLIGHTSPOTLIGHT
