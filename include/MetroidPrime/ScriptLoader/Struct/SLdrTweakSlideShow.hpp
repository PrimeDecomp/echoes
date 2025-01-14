#ifndef _SLDRTWEAKSLIDESHOW
#define _SLDRTWEAKSLIDESHOW

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

struct SLdrTweakSlideShow {
    SLdrTweakSlideShow();
    ~SLdrTweakSlideShow();

    rstl::string instanceName;
    rstl::string pakFile;
    rstl::string font;
    CColor fontColor;
    CColor fontOutlineColor;
    float unknown_0xd398dac2;
    float unknown_0x03757d08;
    float translationMultiplier;
    float scaleMultiplier;
    float slideShowDelay;
    CColor helpFrameColor;
    float helpTransitionTime;
    float slideBlendTime;
    float unknown_0x029d2082;
    float unknown_0xb187cd9b;
    float fadeInTime;
    float fadeOutTime;
    rstl::string unknown_0xc0544bc1;
};

void LoadTypedefSLdrTweakSlideShow(SLdrTweakSlideShow&, CInputStream&);

#endif // _SLDRTWEAKSLIDESHOW
