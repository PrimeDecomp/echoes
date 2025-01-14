#ifndef _SLDRDYNAMICLIGHTSPOTLIGHT
#define _SLDRDYNAMICLIGHTSPOTLIGHT

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrDynamicLightSpotlight {
    SLdrDynamicLightSpotlight();
    ~SLdrDynamicLightSpotlight();

    SLdrSpline spotlightAngle;
    float spotlightAngleDuration;
    bool spotlightAngleLoops;
};

void LoadTypedefSLdrDynamicLightSpotlight(SLdrDynamicLightSpotlight&, CInputStream&);

#endif // _SLDRDYNAMICLIGHTSPOTLIGHT
