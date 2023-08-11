#ifndef _SLDRTWEAKTARGETING_OUTERBEAMICON
#define _SLDRTWEAKTARGETING_OUTERBEAMICON

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakTargeting_OuterBeamIcon {
    SLdrTweakTargeting_OuterBeamIcon();
    ~SLdrTweakTargeting_OuterBeamIcon();

    float unknown_0x383e2b2d;
    float unknown_0xeaac42d0;
    float butSettingsColor;
    CColor butSettingsScale;
    int unknown_0xe7d57d6a;
};

void LoadTypedefSLdrTweakTargeting_OuterBeamIcon(SLdrTweakTargeting_OuterBeamIcon&, CInputStream&);

#endif // _SLDRTWEAKTARGETING_OUTERBEAMICON
