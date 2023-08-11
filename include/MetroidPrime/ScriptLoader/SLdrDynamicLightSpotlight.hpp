#ifndef _SLDRDYNAMICLIGHTSPOTLIGHT
#define _SLDRDYNAMICLIGHTSPOTLIGHT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrDynamicLightSpotlight {
    SLdrDynamicLightSpotlight();
    ~SLdrDynamicLightSpotlight();

    spline spotlightAngle;
    float spotlightAngleDuration;
    bool spotlightAngleLoops;
};

void LoadTypedefSLdrDynamicLightSpotlight(SLdrDynamicLightSpotlight&, CInputStream&);

#endif // _SLDRDYNAMICLIGHTSPOTLIGHT
