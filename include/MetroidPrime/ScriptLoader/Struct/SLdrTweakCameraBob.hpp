#ifndef _SLDRTWEAKCAMERABOB
#define _SLDRTWEAKCAMERABOB

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

struct SLdrTweakCameraBob {
    SLdrTweakCameraBob();
    ~SLdrTweakCameraBob();

    rstl::string instanceName;
    float cameraBobExtentX;
    float cameraBobExtentY;
    float cameraBobPeriod;
    float unknown_0xa27bb5a7;
    float unknown_0xe3580b2b;
    float slowSpeedPeriodScale;
    float targetMagnitudeTrackingRate;
    float landingBobSpringConstant;
    float viewWanderRadius;
    float viewWanderSpeedMin;
    float viewWanderSpeedMax;
    float viewWanderRollVariation;
    float gunBobMagnitude;
    float helmetBobMagnitude;
};

void LoadTypedefSLdrTweakCameraBob(SLdrTweakCameraBob&, CInputStream&);

#endif // _SLDRTWEAKCAMERABOB
